#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <vector>

#include "cli.hpp"
#include "export.hpp"
#include "qrcode.hpp"
#include "reedsolomon.hpp"

using std::string;
using std::vector;

void show_information(string &data, uint8_t qrcode_version, MODE_INDICATOR mode,
                      ERROR_CORRECTION_LEVEL error_code_correction_level) {
  std::cout << "\n\n---------- Encoding information section ----------\n";
  std::cout << "Encoding the following data : " << data << std::endl;

  switch (error_code_correction_level) {
  case L:
    std::cout << "Error code level : LOW" << std::endl;
    break;
  case M:
    std::cout << "Error code level : MEDIUM" << std::endl;
    break;
  case Q:
    std::cout << "Error code level : QUARTILE" << std::endl;
    break;
  case H:
    std::cout << "Error code level : HIGH" << std::endl;
    break;
  }

  switch (mode) {

  case NUMERIC:
    std::cout << "Encoding mode : NUMERIC" << std::endl;
    if (!is_numeric(data))
      throw std::runtime_error("Data can not be encoded in numeric mode");
    break;
  case ALPHANUMERIC:
    std::cout << "Encoding mode : ALPHANUMERIC" << std::endl;
    if (!is_alphanumeric(data))
      throw std::runtime_error("Data can not be encoded in alphanumeric mode");
    break;
  case BYTE:
    std::cout << "Encoding mode : BYTE (utf-8)" << std::endl;
    break;
  case ECI:
    throw std::runtime_error("ECI encoding isn't support :(");
  case KANJI:
    throw std::runtime_error("Kanji encoding isn't support :(");
  }

  std::cout << "QR code version : " << (int)qrcode_version << std::endl;
}

void show_qrcode_in_shell(vector<vector<uint8_t>> &qrcode) {
  // Show it on kitty
  int ret_code = system("kitten icat --place 30x30@-1x-1 --scale-up "
                        "--align=left qrcode.pgm 2> /dev/null");

  if (ret_code != 0) {
    if (qrcode.size() > 70) {
      std::cout << "QR code too big to be printed in shell\n";
      return;
    }
    for (vector<uint8_t> row : qrcode) {
      for (uint8_t m : row) {
        if (m)
          std::cout << "  ";
        else
          std::cout << "##";
      }
      std::cout << std::endl;
    }
  } else {
    for (size_t i = 0; i < 15; i++) {
      std::cout << std::endl;
    }
  }
}

int main(int argc, char *argv[]) {
  string data;
  ERROR_CORRECTION_LEVEL error_code_correction_level;
  MODE_INDICATOR mode;
  uint8_t qrcode_version;
  uint8_t mask_code;

  if (argc == 1) {
    read_user_input(data, &qrcode_version, &error_code_correction_level, &mode,
                    &mask_code);
  } else {
    read_user_file(data, &qrcode_version, &error_code_correction_level, &mode,
                   &mask_code, argv[1]);
  }

  show_information(data, qrcode_version, mode, error_code_correction_level);

  unsigned int number_of_bits =
      determine_number_of_bits(error_code_correction_level, qrcode_version);

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

  // Use to not overwrite functional part of the qrcode when writting data
  mark_reserved_information_module(qrcode_version, is_module_reserved);

  // Add data to qrcode
  draw_data(qrcode, is_module_reserved, encoded_data_with_redondances);

  // detect which mask is the best
  // apply the best mask
  if (mask_code == 8)
    mask_code = get_best_mask(qrcode, is_module_reserved);

  std::cout << "Mask : " << (int)mask_code << std::endl;

  apply_mask(mask_code, qrcode, is_module_reserved);

  // Add the last information on qrcode
  draw_format_information(qrcode_version, error_code_correction_level,
                          mask_code, qrcode);

  draw_dark_module(qrcode_version, qrcode, is_module_reserved);

  // For a better dectetion of Qr code with camera
  add_padding_arround_qrcode(qrcode);

  /* --- Output the genrated qrcode --- */

  // Scaled it up to make a good quality image
  vector<vector<uint8_t>> qrcode_scaled = scaleUpQrcode(qrcode);

  std::cout << "\n--------------- Generated QR code ----------------\n";
  // Save it
  saveQRCodeToPGM(qrcode_scaled, "qrcode.pgm");

  // Show a qrcode in ascci art or with kitten icat
  show_qrcode_in_shell(qrcode);

  return EXIT_SUCCESS;
}
