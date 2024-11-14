#include <cassert>
#include <cstdint>
#include <vector>

#include "reedsolomon.hpp"

using std::int8_t;
using std::size_t;
using std::uint8_t;
using std::vector;

const int8_t ECC_CODEWORDS_PER_BLOCK[4][41] = {
    {-1, 7,  10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26,
     30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30,
     30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30}, // Low
    {-1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22,
     24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28,
     28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28}, // Medium
    {-1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24,
     20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30,
     30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30}, // Quartile
    {-1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22,
     24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30,
     30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30}, // High
};

const int8_t NUM_ERROR_CORRECTION_BLOCKS[4][41] = {
    {-1, 1,  1,  1,  1,  1,  2,  2,  2,  2,  4,  4,  4,  4,
     4,  6,  6,  6,  6,  7,  8,  8,  9,  9,  10, 12, 12, 12,
     13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25}, // Low
    {-1, 1,  1,  1,  2,  2,  4,  4,  4,  5,  5,  5,  8,  9,
     9,  10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25,
     26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49}, // Medium
    {-1, 1,  1,  2,  2,  4,  4,  6,  6,  8,  8,  8,  10, 12,
     16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34,
     35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68}, // Quartile
    {-1, 1,  1,  2,  4,  4,  4,  5,  6,  8,  8,  11, 11, 16,
     16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40,
     42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81}, // High
};

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
addEccAndInterleave(vector<uint8_t> &data, unsigned short version,
                    ERROR_CORRECTION_LEVEL errorCorrectionLevel) {
  // Calculate parameter numbers
  int numBlocks =
      NUM_ERROR_CORRECTION_BLOCKS[static_cast<int>(errorCorrectionLevel)]
                                 [version];
  int blockEccLen =
      ECC_CODEWORDS_PER_BLOCK[static_cast<int>(errorCorrectionLevel)][version];
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
    k += static_cast<int>(dat.size());
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
