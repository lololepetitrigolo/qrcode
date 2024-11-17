#include <algorithm>
#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ostream>
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

uint8_t compute_character_count_indicator(uint8_t qrcode_version,
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

void int_to_bits_N(unsigned long data, vector<uint8_t> &bits, int N) {
  for (int i = N - 1; i >= 0; --i)
    bits.push_back((data >> i) & 1);
}

unsigned int determine_number_of_bits(ERROR_CORRECTION_LEVEL correction_level,
                                      uint8_t qrcode_version) {
  int result = (16 * qrcode_version + 128) * qrcode_version + 64;
  if (qrcode_version >= 2) {
    int numAlign = qrcode_version / 7 + 2;
    result -= (25 * numAlign - 10) * numAlign - 55;
    if (qrcode_version >= 7)
      result -= 36;
  }
  assert(208 <= result && result <= 29648);
  return (result / 8 -
          ECC_CODEWORDS_PER_BLOCK[correction_level][qrcode_version] *
              NUM_ERROR_CORRECTION_BLOCKS[correction_level][qrcode_version]) *
         8;
}

void add_character_count_indicator(string data, uint8_t qrcode_version,
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
  for (size_t i = 0; i < data.length(); i += 3) {
    string sub_data = data.substr(i, std::min(3UL, data.length() - i));
    int num = std::stoi(sub_data);
    int size = 3;
    if(sub_data.length() ==  1)
      size = 4;
    else if(sub_data.length() == 2)
      size = 7;
    else{
      if(sub_data[0] == '0' && sub_data[1] == '0')
        size = 4;
      else if(sub_data[0] == '0')
          size = 7;
      else 
        size = 10;
          
    }
    int_to_bits_N(num, encoded_data, size);
  }
  return encoded_data;
}

int get_alphanumeric_code(char c) {
  if ('0' <= c && c <= '9')
    return (int)c - 48;

  if ('A' <= c && c <= 'Z')
    return (int)c - 55;

  switch (c) {
  case ' ':
    return 36;
  case '$':
    return 37;
  case '%':
    return 38;
  case '*':
    return 39;
  case '+':
    return 40;
  case '-':
    return 41;
  case '.':
    return 42;
  case '/':
    return 43;
  case ':':
    return 44;
  }
  return 0;
}

vector<uint8_t> encode_alphanumeric(string data,
                                    vector<uint8_t> &encoded_data) {
  for (size_t i = 0; i < data.length(); i += 2) {
    int char_code1 = get_alphanumeric_code(data[i]);
    int char_code2 =
        i + 1 < data.size() ? get_alphanumeric_code(data[i + 1]) : char_code1;
    int_to_bits_N((45 * (i + 1 < data.size() ? char_code1 : 0)) + char_code2,
                  encoded_data, i + 1 < data.size() ? 11 : 6);
  }
  return encoded_data;
}

vector<uint8_t> encode_byte(string data, vector<uint8_t> &encoded_data) {
  for (size_t i = 0; i < data.length(); i++) {
    int char_code = (int)data[i];
    int_to_bits_N(char_code, encoded_data, 8);
  }
  return encoded_data;
}

void add_terminator(vector<uint8_t> &bits, unsigned long number_of_bits) {
  unsigned int number_of_0 = std::min(number_of_bits - bits.size(), 4UL);

  for (size_t i = 0; i < number_of_0; i++) {
    bits.push_back(0);
  }
}

void add_padding(vector<uint8_t> &bits) {
  if (!(bits.size() % 8))
    return;

  size_t size = bits.size();
  for (size_t i = 0; i < 8 - (size % 8); i++) {
    bits.push_back(0);
  }
}

void add_extra_padding(vector<uint8_t> &bits, unsigned int number_of_bits) {
  uint8_t padding[16] = {1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1};
  size_t i = 0;

  while (bits.size() != number_of_bits) {
    bits.push_back(padding[i % 16]);
    i++;
  }
}

void draw_finder_pattern(int x, int y, vector<vector<uint8_t>> &qrcode,
                         vector<vector<uint8_t>> &is_module_reserved) {
  for (size_t i = 0; i < 7; i++) {
    for (size_t j = 0; j < 7; j++) {
      if (i == 0 || j == 0 || i == 6 || j == 6 ||
          (i >= 2 && i <= 4 && j >= 2 && j <= 4))
        qrcode[y + i][x + j] = 1;
      else
        qrcode[y + i][x + j] = 0;
      is_module_reserved[y + i][x + j] = 1;
    }
  }

  int size = qrcode.size();
  for (int i = 0; i < 9; i++) {
    // Top line
    if (x - 1 + i >= 0 && x - 1 + i < size && y - 1 >= 0)
      is_module_reserved[y - 1][x - 1 + i] = 1;
    // Bottom line
    if (x - 1 + i >= 0 && x - 1 + i < size && y + 7 < size)
      is_module_reserved[y + 7][x - 1 + i] = 1;
    // Left line
    if (x - 1 >= 0 && y - 1 + i >= 0 && y - 1 + i < size)
      is_module_reserved[y - 1 + i][x - 1] = 1;
    // Right line
    if (x + 7 < size && y - 1 + i >= 0 && y - 1 + i < size)
      is_module_reserved[y - 1 + i][x + 7] = 1;
  }
}

void draw_alignment_pattern(uint8_t qrcode_version,
                            vector<vector<uint8_t>> &qrcode,
                            vector<vector<uint8_t>> &is_module_reserved) {
  if (qrcode_version == 1)
    return;

  vector<uint8_t> position_alignment_pattern;

  int numAlign = qrcode_version / 7 + 2;
  int step = (qrcode_version == 32) ? 26
                                    : (qrcode_version * 4 + numAlign * 2 + 1) /
                                          (numAlign * 2 - 2) * 2;
  uint8_t size = qrcode.size();
  for (int i = 0, pos = size - 7; i < numAlign - 1; i++, pos -= step)
    position_alignment_pattern.insert(position_alignment_pattern.begin(), pos);
  position_alignment_pattern.insert(position_alignment_pattern.begin(), 6);

  for (int i = 0; i < (int)position_alignment_pattern.size(); i++) {
    for (int j = 0; j < (int)position_alignment_pattern.size(); j++) {
      if (((j == 0 && i == 0) || (i == 0 && j == numAlign - 1) ||
           (i == numAlign - 1 && j == 0)))
        continue;

      int x = position_alignment_pattern[i] - 2;
      int y = position_alignment_pattern[j] - 2;

      for (int k = 0; k < 5; k++) {
        for (int l = 0; l < 5; l++) {
          if (k == 0 || l == 0 || k == 4 || l == 4 || (k == 2 && l == 2))
            qrcode[y + k][x + l] = 1;
          else
            qrcode[y + k][x + l] = 0;
          is_module_reserved[y + k][x + l] = 1;
        }
      }
    }
  }
}

void draw_timing_pattern(vector<vector<uint8_t>> &qrcode,
                         vector<vector<uint8_t>> &is_module_reserved) {
  for (size_t i = 8; i < qrcode.size() - 7; i++) {
    qrcode[6][i] = !(i % 2);
    qrcode[i][6] = !(i % 2);
    is_module_reserved[i][6] = 1;
    is_module_reserved[6][i] = 1;
  }
}

void draw_dark_module(uint8_t qrcode_version, vector<vector<uint8_t>> &qrcode,
                      vector<vector<uint8_t>> &is_module_reserved) {
  qrcode[qrcode_version * 4 + 9][8] = 1;
  is_module_reserved[qrcode_version * 4 + 9][8] = 1;
}

void mark_reserved_information_module(
    uint8_t qrcode_version, vector<vector<uint8_t>> &is_module_reserved) {

  // For top left parttern
  for (size_t i = 0; i < 9; i++) {
    is_module_reserved[8][i] = 1;
    is_module_reserved[i][8] = 1;
  }
  for (size_t i = 0; i < 8; i++) {
    // For top right pattern
    is_module_reserved[8][is_module_reserved.size() - 1 - i] = 1;
    // For bottom left pattern
    is_module_reserved[qrcode_version * 4 + 9 + i][8] = 1;
  }

  if (qrcode_version >= 7) {
    for (size_t i = 0; i < 6; i++) {
      for (size_t j = 0; j < 3; j++) {
        // For top right pattern
        is_module_reserved[i][is_module_reserved.size() - 11 + j] = 1;
        // For bottom left pattern
        is_module_reserved[is_module_reserved.size() - 11 + j][i] = 1;
      }
    }
  }
}

void draw_data(vector<vector<uint8_t>> &qrcode,
               vector<vector<uint8_t>> &is_module_reserved,
               vector<uint8_t> &data) {
  int size = qrcode.size();
  int i = 0;
  for (int right = size - 1; right >= 1;
       right -= 2) {
    if (right == 6)
      right = 5;
    for (int vert = 0; vert < size; vert++) {
      for (int j = 0; j < 2; j++) {
        size_t x = static_cast<size_t>(right - j);
        bool upward = ((right + 1) & 2) == 0;
        size_t y = static_cast<size_t>(upward ? size - 1 - vert : vert);
        if (!is_module_reserved.at(y).at(x) && i < (int)data.size() * 8) {
          qrcode.at(y).at(x) = data.at(i >> 3) >> (7 - (i & 7)) & 1;
          i++;
        }
      }
    }
  }
}

void apply_mask(uint8_t mask_number, vector<vector<uint8_t>> &qrcode,
                vector<vector<uint8_t>> &is_module_reserved) {
  assert(mask_number < 8);
  for (size_t y = 0; y < qrcode.size(); y++) {
    for (size_t x = 0; x < qrcode.size(); x++) {
      bool invert;
      switch (mask_number) {
      case 0:
        invert = (x + y) % 2 == 0;
        break;
      case 1:
        invert = y % 2 == 0;
        break;
      case 2:
        invert = x % 3 == 0;
        break;
      case 3:
        invert = (x + y) % 3 == 0;
        break;
      case 4:
        invert = (x / 3 + y / 2) % 2 == 0;
        break;
      case 5:
        invert = x * y % 2 + x * y % 3 == 0;
        break;
      case 6:
        invert = (x * y % 2 + x * y % 3) % 2 == 0;
        break;
      case 7:
        invert = ((x + y) % 2 + x * y % 3) % 2 == 0;
        break;
      }
      if (!is_module_reserved[y][x])
        qrcode[y][x] = qrcode[y][x] ^ invert;
    }
  }
}

void draw_format_information(uint8_t qrcode_version,
                             ERROR_CORRECTION_LEVEL error_code_correction_level,
                             uint8_t mask_code,
                             vector<vector<uint8_t>> &qrcode) {
  int size = qrcode.size();
  int data = error_code_correction_level << 3 | mask_code;
  int rem = data;
  for (int i = 0; i < 10; i++)
    rem = (rem << 1) ^ ((rem >> 9) * 0x537);
  int bits = (data << 10 | rem) ^ 0x5412;
  assert(bits >> 15 == 0);

  // Draw first copy
  for (int i = 0; i <= 5; i++)
    qrcode[i][8] = (bits >> i) & 1;
  qrcode[7][8] = (bits >> 6) & 1;
  qrcode[8][8] = (bits >> 7) & 1;
  qrcode[8][7] = (bits >> 8) & 1;
  for (int i = 9; i < 15; i++)
    qrcode[8][14 - i] = (bits >> i) & 1;

  // Draw second copy
  for (int i = 0; i < 8; i++)
    qrcode[8][size - 1 - i] = (bits >> i) & 1;
  for (int i = 8; i < 15; i++)
    qrcode[size - 15 + i][8] = (bits >> i) & 1;

  if (qrcode_version >= 7) {
    int rem = qrcode_version;
    for (int i = 0; i < 12; i++)
      rem = (rem << 1) ^ ((rem >> 11) * 0x1F25);
    long bits = static_cast<long>(qrcode_version) << 12 | rem;
    assert(bits >> 18 == 0);

    // Draw two copies
    for (int i = 0; i < 18; i++) {
      bool bit = (bits >> i) & 1;
      int a = size - 11 + i % 3;
      int b = i / 3;
      qrcode[a][b] = bit;
      qrcode[b][a] = bit;
    }
  }
}

void add_padding_arround_qrcode(vector<vector<uint8_t>> &qrcode) {
  vector<uint8_t> empty_row(qrcode.size() + 4, 0);

  for (vector<uint8_t> &row : qrcode) {
    std::reverse(row.begin(), row.end());
    row.push_back(0);
    row.push_back(0);
    std::reverse(row.begin(), row.end());
    row.push_back(0);
    row.push_back(0);
  }

  std::reverse(qrcode.begin(), qrcode.end());
  qrcode.push_back(empty_row);
  qrcode.push_back(empty_row);
  std::reverse(qrcode.begin(), qrcode.end());
  qrcode.push_back(empty_row);
  qrcode.push_back(empty_row);
}

vector<vector<uint8_t>> scaleUpQrcode(vector<vector<uint8_t>> &qrcode) {
  uint8_t scale_factor = 1024 / qrcode.size();

  if (scale_factor <= 1)
    return qrcode;

  vector<vector<uint8_t>> qrcode_scaled(
      qrcode.size() * scale_factor,
      vector<uint8_t>(qrcode.size() * scale_factor, 0));

  for (size_t i = 0; i < qrcode.size() * scale_factor; i++) {
    for (size_t j = 0; j < qrcode.size() * scale_factor; j++) {
      qrcode_scaled[i][j] = qrcode[i / scale_factor][j / scale_factor];
    }
  }

  return qrcode_scaled;
}

void saveQRCodeToPGM(vector<std::vector<uint8_t>> &qrcode,
                     const string &filename) {

  // Ouvrir un fichier en mode binaire
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Erreur when openning the file" << std::endl;
    return;
  }

  file << "P2" << std::endl;
  file << qrcode.size() << " " << qrcode.size() << std::endl;
  file << "1" << std::endl;

  for (int y = 0; y < (int)qrcode.size(); y++) {
    for (int x = 0; x < (int)qrcode.size(); x++) {
      if (qrcode[y][x])
        file << "0 ";
      else
        file << "1 ";
    }
  }

  file.close();
  std::cout << "QR code saved " << filename << std::endl;
}

int main() {
  string data = "6445";

  ERROR_CORRECTION_LEVEL error_code_correction_level = L;

  uint8_t qrcode_version = 7;
  assert(qrcode_version > 0 && qrcode_version <= 40);

  MODE_INDICATOR mode = NUMERIC;

  unsigned int number_of_bits =
      determine_number_of_bits(error_code_correction_level, qrcode_version);
  std::cout << number_of_bits << std::endl;

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
  case ALPHANUMERIC:
    encode_alphanumeric(data, encoded_data);
    break;
  case BYTE:
    encode_byte(data, encoded_data);
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
    dataCodewords[i >> 3] |= (encoded_data[i] ? 1 : 0) << (7 - (i & 7));

  for (uint8_t code : dataCodewords) {
    std::cout << (int)code << ",";
  }
  std::cout << std::endl;

  // Code redondance with reedsolomon algorithm
  vector<uint8_t> encoded_data_with_redondances = addEccAndInterleave(
      dataCodewords, qrcode_version, error_code_correction_level);


  /* --- Qrcode agencing info --- */

  // Init data
  uint8_t qrcode_size = ((qrcode_version - 1) * 4) + 21;
  vector<vector<uint8_t>> qrcode(qrcode_size, vector<uint8_t>(qrcode_size, 0));
  vector<vector<uint8_t>> is_module_reserved(qrcode_size,
                                             vector<uint8_t>(qrcode_size, 0));

  // Place all stuff on qrcode
  draw_finder_pattern(0, 0, qrcode, is_module_reserved);
  draw_finder_pattern(qrcode_size - 7, 0, qrcode, is_module_reserved);
  draw_finder_pattern(0, qrcode_size - 7, qrcode, is_module_reserved);

  draw_alignment_pattern(qrcode_version, qrcode, is_module_reserved);

  draw_timing_pattern(qrcode, is_module_reserved);

  mark_reserved_information_module(qrcode_version, is_module_reserved);

  draw_data(qrcode, is_module_reserved, encoded_data_with_redondances);

  uint8_t mask_code = 0;

  apply_mask(mask_code, qrcode, is_module_reserved);

  draw_format_information(qrcode_version, error_code_correction_level,
                          mask_code, qrcode);

  draw_dark_module(qrcode_version, qrcode, is_module_reserved);

  add_padding_arround_qrcode(qrcode);

  /* --- Output the genrated qrcode --- */

  // Scaled it up to make a clear image
  vector<vector<uint8_t>> qrcode_scaled = scaleUpQrcode(qrcode);

  // Save it
  saveQRCodeToPGM(qrcode_scaled, "qrcode.pgm");

  // Show it on kitty
  system("kitten icat --place 30x30@-1x-1 --scale-up --align=left qrcode.pgm");

  for (size_t i = 0; i < 15; i++) {
    std::cout << std::endl;
  }

  return EXIT_SUCCESS;
}
