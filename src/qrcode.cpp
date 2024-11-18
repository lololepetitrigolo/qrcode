#include <cstdint>

#include "qrcode.hpp"

using std::string;
using std::vector;

const uint32_t max_characters[40][4][4] = {
    {
        {34, 20, 14, 8},
        {41, 25, 17, 10},
        {17, 10, 7, 4},
        {27, 16, 11, 7},
    },
    {
        {63, 38, 26, 16},
        {77, 47, 32, 20},
        {34, 20, 14, 8},
        {48, 29, 20, 12},
    },
    {
        {101, 61, 42, 26},
        {127, 77, 53, 32},
        {58, 35, 24, 15},
        {77, 47, 32, 20},
    },
    {
        {149, 90, 62, 38},
        {187, 114, 78, 48},
        {82, 50, 34, 21},
        {111, 67, 46, 28},
    },
    {
        {202, 122, 84, 52},
        {255, 154, 106, 65},
        {106, 64, 44, 27},
        {144, 87, 60, 37},
    },
    {
        {255, 154, 106, 65},
        {322, 195, 134, 82},
        {139, 84, 58, 36},
        {178, 108, 74, 45},
    },
    {
        {293, 178, 122, 75},
        {370, 224, 154, 95},
        {154, 93, 64, 39},
        {207, 125, 86, 53},
    },
    {
        {365, 221, 152, 93},
        {461, 279, 192, 118},
        {202, 122, 84, 52},
        {259, 157, 108, 66},
    },
    {
        {432, 262, 180, 111},
        {552, 335, 230, 141},
        {235, 143, 98, 60},
        {312, 189, 130, 80},
    },
    {
        {513, 311, 213, 131},
        {652, 395, 271, 167},
        {288, 174, 119, 74},
        {364, 221, 151, 93},
    },
    {
        {604, 366, 251, 155},
        {772, 468, 321, 198},
        {331, 200, 137, 85},
        {427, 259, 177, 109},
    },
    {
        {691, 419, 287, 177},
        {883, 535, 367, 226},
        {374, 227, 155, 96},
        {489, 296, 203, 125},
    },
    {
        {796, 483, 331, 204},
        {1022, 619, 425, 262},
        {427, 259, 177, 109},
        {580, 352, 241, 149},
    },
    {
        {871, 528, 362, 223},
        {1101, 667, 458, 282},
        {468, 283, 194, 120},
        {621, 376, 258, 159},
    },
    {
        {991, 600, 412, 254},
        {1250, 758, 520, 320},
        {530, 321, 220, 136},
        {703, 426, 292, 180},
    },
    {
        {1082, 656, 450, 277},
        {1408, 854, 586, 361},
        {602, 365, 250, 154},
        {775, 470, 322, 198},
    },
    {
        {1212, 734, 504, 310},
        {1548, 938, 644, 397},
        {674, 408, 280, 173},
        {876, 531, 364, 224},
    },
    {
        {1346, 816, 560, 345},
        {1725, 1046, 718, 442},
        {746, 452, 310, 191},
        {948, 574, 394, 243},
    },
    {
        {1500, 909, 624, 384},
        {1903, 1153, 792, 488},
        {813, 493, 338, 208},
        {1063, 644, 442, 272},
    },
    {
        {1600, 970, 666, 410},
        {2061, 1249, 858, 528},
        {919, 557, 382, 235},
        {1159, 702, 482, 297},
    },
    {
        {1708, 1035, 711, 438},
        {2232, 1352, 929, 572},
        {969, 587, 403, 248},
        {1224, 742, 509, 314},
    },
    {
        {1872, 1134, 779, 480},
        {2409, 1460, 1003, 618},
        {1056, 640, 439, 270},
        {1358, 823, 565, 348},
    },
    {
        {2059, 1248, 857, 528},
        {2620, 1588, 1091, 672},
        {1108, 672, 461, 284},
        {1468, 890, 611, 376},
    },
    {
        {2188, 1326, 911, 561},
        {2812, 1704, 1171, 721},
        {1228, 744, 511, 315},
        {1588, 963, 661, 407},
    },
    {
        {2395, 1451, 997, 614},
        {3057, 1853, 1273, 784},
        {1286, 779, 535, 330},
        {1718, 1041, 715, 440},
    },
    {
        {2544, 1542, 1059, 652},
        {3283, 1990, 1367, 842},
        {1425, 864, 593, 365},
        {1804, 1094, 751, 462},
    },
    {
        {2701, 1637, 1125, 692},
        {3517, 2132, 1465, 902},
        {1501, 910, 625, 385},
        {1933, 1172, 805, 496},
    },
    {
        {2857, 1732, 1190, 732},
        {3669, 2223, 1528, 940},
        {1581, 958, 658, 405},
        {2085, 1263, 868, 534},
    },
    {
        {3035, 1839, 1264, 778},
        {3909, 2369, 1628, 1002},
        {1677, 1016, 698, 430},
        {2181, 1322, 908, 559},
    },
    {
        {3289, 1994, 1370, 843},
        {4158, 2520, 1732, 1066},
        {1782, 1080, 742, 457},
        {2358, 1429, 982, 604},
    },
    {
        {3486, 2113, 1452, 894},
        {4417, 2677, 1840, 1132},
        {1897, 1150, 790, 486},
        {2473, 1499, 1030, 634},
    },
    {
        {3693, 2238, 1538, 947},
        {4686, 2840, 1952, 1201},
        {2022, 1226, 842, 518},
        {2670, 1618, 1112, 684},
    },
    {
        {3909, 2369, 1628, 1002},
        {4965, 3009, 2068, 1273},
        {2157, 1307, 898, 553},
        {2805, 1700, 1168, 719},
    },
    {
        {4134, 2506, 1722, 1060},
        {5253, 3183, 2188, 1347},
        {2301, 1394, 958, 590},
        {2949, 1787, 1228, 756},
    },
    {
        {4343, 2632, 1809, 1113},
        {5529, 3351, 2303, 1417},
        {2361, 1431, 983, 605},
        {3081, 1867, 1283, 790},
    },
    {
        {4588, 2780, 1911, 1176},
        {5836, 3537, 2431, 1496},
        {2524, 1530, 1051, 647},
        {3244, 1966, 1351, 832},
    },
    {
        {4775, 2894, 1989, 1224},
        {6153, 3729, 2563, 1577},
        {2625, 1591, 1093, 673},
        {3417, 2071, 1423, 876},
    },
    {
        {5039, 3054, 2099, 1292},
        {6479, 3927, 2699, 1661},
        {2735, 1658, 1139, 701},
        {3599, 2181, 1499, 923},
    },
    {
        {5313, 3220, 2213, 1362},
        {6743, 4087, 2809, 1729},
        {2927, 1774, 1219, 750},
        {3791, 2298, 1579, 972},
    },
    {
        {5596, 3391, 2331, 1435},
        {7089, 4296, 2953, 1817},
        {3057, 1852, 1273, 784},
        {3993, 2420, 1663, 1024},
    },
};

size_t get_max_capacitie(uint8_t qrcode_version,
                         ERROR_CORRECTION_LEVEL err_level,
                         MODE_INDICATOR mode) {
  int mode_index = mode == NUMERIC ? 0 : mode == ALPHANUMERIC ? 2 : 3;
  return max_characters[qrcode_version - 1][err_level][mode_index];
}

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

size_t determine_version(size_t data_length, MODE_INDICATOR mode,
                         ERROR_CORRECTION_LEVEL err_level) {
  for (size_t version = 1; version <= 40; version++) {
    if (data_length <= get_max_capacitie(version, err_level, mode))
      return version;
  }
  return 41;
}

ERROR_CORRECTION_LEVEL upgrade_err_level(size_t data_length,
                                         uint8_t qrcode_version,
                                         MODE_INDICATOR mode) {
  if (get_max_capacitie(qrcode_version, H, mode) >= data_length)
    return H;
  if (get_max_capacitie(qrcode_version, Q, mode) >= data_length)
    return Q;
  if (get_max_capacitie(qrcode_version, M, mode) >= data_length)
    return M;

  return L;
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

bool is_numeric(string data) {
  for (char c : data) {
    if (c < '0' || c > '9')
      return false;
  }
  return true;
}

static const string alphanumeric_characters =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

bool is_alphanumeric(string data) {
  for (char c : data) {
    if (std::find(alphanumeric_characters.begin(),
                  alphanumeric_characters.end(),
                  c) == alphanumeric_characters.end())
      return false;
  }

  return true;
}

vector<uint8_t> encode_numeric(string data, vector<uint8_t> &encoded_data) {
  for (size_t i = 0; i < data.length(); i += 3) {
    string sub_data = data.substr(i, std::min(3UL, data.length() - i));
    int num = std::stoi(sub_data);
    int size = 3;
    if (sub_data.length() == 1)
      size = 4;
    else if (sub_data.length() == 2)
      size = 7;
    else {
      if (sub_data[0] == '0' && sub_data[1] == '0')
        size = 4;
      else if (sub_data[0] == '0')
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
  for (int right = size - 1; right >= 1; right -= 2) {
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
