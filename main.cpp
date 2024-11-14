#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "reedsolomon.hpp"

using std::string;
using std::vector;

enum MODE_INDICATOR {
  NUMERIC = 1,
  ALPHANUMERIC = 2,
  BYTE = 4,
  ECI = 7,
  KANJI = 8,
};

unsigned int compute_character_count_indicator(unsigned int qrcode_version,
                                               MODE_INDICATOR mode) {
  if (qrcode_version < 10) {
    switch (mode) {
    case NUMERIC:
      return 10;
    case ALPHANUMERIC:
      return 9;
    default:
      return 8;
    }
  } else if (qrcode_version > 26) {
    switch (mode) {
    case NUMERIC:
      return 14;
    case ALPHANUMERIC:
      return 13;
    case BYTE:
      return 16;
    default:
      return 12;
    }
  } else {
    switch (mode) {
    case NUMERIC:
      return 12;
    case ALPHANUMERIC:
      return 11;
    case BYTE:
      return 16;
    default:
      return 10;
    }
  }
}

void int_to_bits(unsigned long data, vector<uint8_t> &bits) {
  bool is_started = false;

  for (size_t i = sizeof(data) * 8; i > 0; --i) {
    is_started |= (data >> (i - 1)) & 1;
    if (is_started) {
      bits.push_back((data >> (i - 1)) & 1);
    }
  }
}

void add_character_count_indicator(string data, unsigned int qrcode_version,
                                   MODE_INDICATOR mode,
                                   vector<uint8_t> &encoded_data) {
  for (size_t i = compute_character_count_indicator(qrcode_version, mode);
       i > 0; --i) {
    encoded_data.push_back((data.size() >> (i - 1)) & 1);
  }
}

void add_mode_indicator(MODE_INDICATOR mode, vector<uint8_t> &encoded_data) {
  switch (mode) {
  case NUMERIC:
    encoded_data.push_back(0);
    encoded_data.push_back(0);
    encoded_data.push_back(0);
    encoded_data.push_back(1);
    break;
  case ALPHANUMERIC:
    encoded_data.push_back(0);
    encoded_data.push_back(0);
    encoded_data.push_back(1);
    encoded_data.push_back(0);
    break;
  case BYTE:
    encoded_data.push_back(0);
    encoded_data.push_back(1);
    encoded_data.push_back(0);
    encoded_data.push_back(0);
    break;
  default:
    break;
  }
}

vector<uint8_t> encode_numeric(string data, vector<uint8_t> &encoded_data) {
  for (int i = 0; i < data.length(); i += 3) {
    string sub_data = data.substr(i, std::min(3, (int)data.length() - i));
    int_to_bits(std::stoi(sub_data), encoded_data);
  }
  return encoded_data;
}

void add_terminator(vector<uint8_t> &bits, unsigned int number_of_bits) {
  unsigned int number_of_0 =
      std::min(number_of_bits - (unsigned int)bits.size(), 4U);

  for (size_t i = 0; i < number_of_0; i++) {
    bits.push_back(0);
  }
}

void add_padding(vector<uint8_t> &bits) {
  for (size_t i = 0; i < bits.size() % 8; i++) {
    bits.push_back(0);
  }
}

void add_extra_padding(vector<uint8_t> bits, unsigned int number_of_bits) {
  uint8_t padding[16] = {1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1};
  size_t i = 0;

  while (bits.size() != number_of_bits) {
    bits.push_back(padding[i % 16]);
    i++;
  }
}

int main(int argc, char *argv[]) {
  string data = "1";

  ERROR_CORRECTION_LEVEL error_code_correction_level = L;

  unsigned short qrcode_version = 1;
  assert(qrcode_version > 0 && qrcode_version <= 40);

  MODE_INDICATOR mode = NUMERIC;

  unsigned int number_of_bits = 13 * 8;

  vector<uint8_t> encoded_data;

  // Add mode
  add_mode_indicator(mode, encoded_data);

  // Add len on the right size
  add_character_count_indicator(data, qrcode_version, mode, encoded_data);

  // Determine bits from data
  switch (mode) {
  case NUMERIC:
    encode_numeric(data, encoded_data);
    break;
  default:
    return EXIT_FAILURE;
  }

  // Add terminator
  add_terminator(encoded_data, number_of_bits);

  // Add padding
  add_padding(encoded_data);
  add_extra_padding(encoded_data, number_of_bits);

  // pack bits into uint8_t in BigEndian notation
  vector<uint8_t> dataCodewords(encoded_data.size() / 8);
  for (size_t i = 0; i < encoded_data.size(); i++)
    dataCodewords.at(i >> 3) |= (encoded_data.at(i) ? 1 : 0) << (7 - (i & 7));

  // Code redondance with reedsolomon algorithm
  vector<uint8_t> encoded_data_with_redondances = addEccAndInterleave(
      dataCodewords, qrcode_version, error_code_correction_level);

  return 0;
}
