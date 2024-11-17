#include <cassert>
#include <cstdint>
#include <vector>

#include "reedsolomon.hpp"

using std::int8_t;
using std::size_t;
using std::uint8_t;
using std::vector;

int getNumRawDataModules(int version) {
  int result = (16 * version + 128) * version + 64;
  if (version >= 2) {
    int numAlign = version / 7 + 2;
    result -= (25 * numAlign - 10) * numAlign - 55;
    if (version >= 7)
      result -= 36;
  }
  assert(208 <= result && result <= 29648);
  return result;
}

uint8_t reedSolomonMultiply(uint8_t x, uint8_t y) {
  // Russian peasant multiplication
  int z = 0;
  for (int i = 7; i >= 0; i--) {
    z = (z << 1) ^ ((z >> 7) * 0x11D);
    z ^= ((y >> i) & 1) * x;
  }
  assert(z >> 8 == 0);
  return static_cast<uint8_t>(z);
}

vector<uint8_t> reedSolomonComputeDivisor(int degree) {
  if (degree < 1 || degree > 255)
    throw std::domain_error("Degree out of range");
  // Polynomial coefficients are stored from highest to lowest power, excluding
  // the leading term which is always 1. For example the polynomial x^3 + 255x^2
  // + 8x + 93 is stored as the uint8 array {255, 8, 93}.
  vector<uint8_t> result(static_cast<size_t>(degree));
  result.at(result.size() - 1) = 1; // Start off with the monomial x^0

  // Compute the product polynomial (x - r^0) * (x - r^1) * (x - r^2) * ... * (x
  // - r^{degree-1}), and drop the highest monomial term which is always
  // 1x^degree. Note that r = 0x02, which is a generator element of this field
  // GF(2^8/0x11D).
  uint8_t root = 1;
  for (int i = 0; i < degree; i++) {
    // Multiply the current product by (x - r^i)
    for (size_t j = 0; j < result.size(); j++) {
      result.at(j) = reedSolomonMultiply(result.at(j), root);
      if (j + 1 < result.size())
        result.at(j) ^= result.at(j + 1);
    }
    root = reedSolomonMultiply(root, 0x02);
  }
  return result;
}

vector<uint8_t> reedSolomonComputeRemainder(const vector<uint8_t> &data,
                                            const vector<uint8_t> &divisor) {
  vector<uint8_t> result(divisor.size());
  for (uint8_t b : data) { // Polynomial division
    uint8_t factor = b ^ result.at(0);
    result.erase(result.begin());
    result.push_back(0);
    for (size_t i = 0; i < result.size(); i++)
      result.at(i) ^= reedSolomonMultiply(divisor.at(i), factor);
  }
  return result;
}

vector<uint8_t>
addEccAndInterleave(vector<uint8_t> &data, uint8_t version,
                    ERROR_CORRECTION_LEVEL errorCorrectionLevel) {
  // Calculate parameter numbers
  int numBlocks = NUM_ERROR_CORRECTION_BLOCKS[errorCorrectionLevel][version];
  int blockEccLen = ECC_CODEWORDS_PER_BLOCK[errorCorrectionLevel][version];
  int rawCodewords = getNumRawDataModules(version) / 8;
  int numShortBlocks = numBlocks - rawCodewords % numBlocks;
  int shortBlockLen = rawCodewords / numBlocks;

  // Split data into blocks and append ECC to each block
  vector<vector<uint8_t>> blocks;
  const vector<uint8_t> rsDiv = reedSolomonComputeDivisor(blockEccLen);
  for (int i = 0, k = 0; i < numBlocks; i++) {
    vector<uint8_t> dat(data.cbegin() + k,
                        data.cbegin() + (k + shortBlockLen - blockEccLen +
                                         (i < numShortBlocks ? 0 : 1)));
    k += (int)dat.size();
    const vector<uint8_t> ecc = reedSolomonComputeRemainder(dat, rsDiv);
    if (i < numShortBlocks)
      dat.push_back(0);
    dat.insert(dat.end(), ecc.cbegin(), ecc.cend());
    blocks.push_back(std::move(dat));
  }

  // Interleave (not concatenate) the bytes from every block into a single
  // sequence
  vector<uint8_t> result;
  for (size_t i = 0; i < blocks.at(0).size(); i++) {
    for (size_t j = 0; j < blocks.size(); j++) {
      // Skip the padding byte in short blocks
      if (i != static_cast<unsigned int>(shortBlockLen - blockEccLen) ||
          j >= static_cast<unsigned int>(numShortBlocks))
        result.push_back(blocks.at(j).at(i));
    }
  }
  assert(result.size() == static_cast<unsigned int>(rawCodewords));
  return result;
}
