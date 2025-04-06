#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

using std::string;

#include "cli.hpp"

void read_user_input(string &data, uint8_t *qrcode_version,
                     ERROR_CORRECTION_LEVEL *error_code_correction_level,
                     MODE_INDICATOR *mode, uint8_t *mask_code) {
  std::cout << "Enter the text to encode : ";
  std::getline(std::cin, data);

  std::cout << "\nChoose the recovery level (press enter to use auto mode "
               "selection) :\n\t- L for LOW 7%\n\t- M for "
               "MEDIUM 15%\n\t- Q for QUARTILE 25%\n\t- H for HIGH 30%\n";
  string error_code_correction_level_str;
  std::getline(std::cin, error_code_correction_level_str);
  if (error_code_correction_level_str == "L")
    *error_code_correction_level = L;
  else if (error_code_correction_level_str == "M")
    *error_code_correction_level = M;
  else if (error_code_correction_level_str == "Q")
    *error_code_correction_level = Q;
  else if (error_code_correction_level_str == "H")
    *error_code_correction_level = H;
  else if (error_code_correction_level_str == "")
    *error_code_correction_level = L;
  else
    throw std::runtime_error(
        "Invalid recovery level input enter L, M, Q, H or level it empty");

  std::cout << "\nChoose the mode to encode input (press enter for auto "
               "selection): "
               "\n\t- N for "
               "NUMERIC (text should only have digits)\n\t- A for ALPHANUMERIC "
               "(text can have uppercase digits or some special character)"
               "\n\t- B for BYTE (every utf-8 text)\n";
  string mode_str;
  std::getline(std::cin, mode_str);
  if (mode_str == "N")
    *mode = NUMERIC;
  else if (mode_str == "A")
    *mode = ALPHANUMERIC;
  else if (mode_str == "B")
    *mode = BYTE;
  else if (mode_str == "") {
    *mode = is_numeric(data)        ? NUMERIC
            : is_alphanumeric(data) ? ALPHANUMERIC
                                    : BYTE;
  } else
    throw std::runtime_error("Invalid encoding mode N, A, B or level it empty");

  std::cout << "\nEnter the version of QR code to use (1 to 40 or press "
               "enter if you have no specific need) : ";
  string qrcode_version_str;
  std::getline(std::cin, qrcode_version_str);

  try {
    *qrcode_version =
        qrcode_version_str == "" ? 7 : std::stoi(qrcode_version_str);
  } catch (...) {
    throw std::runtime_error("Enter a number please");
  }
  if (*qrcode_version > 40 || *qrcode_version == 0)
    throw std::runtime_error("Enter a valide QR code version");

  std::cout << "\nEnter the mask you want to use (0 to 7 or press "
               "enter if you have no specific need(RECOMMANDED)) : ";
  string mask_str;
  std::getline(std::cin, mask_str);

  try {
    *mask_code = mask_str == "" ? 8 : std::stoi(mask_str);
  } catch (...) {
    throw std::runtime_error("Enter a number please");
  }

  if (*qrcode_version > 7)
    throw std::runtime_error("Enter a valide mask");

  if (qrcode_version_str == "") {
    *qrcode_version = determine_version(data.length(), *mode,
                                        error_code_correction_level_str == ""
                                            ? L
                                            : *error_code_correction_level);
    std::cout << "sssssssssssssssss";
  }

  if (error_code_correction_level_str == "")
    *error_code_correction_level =
        upgrade_err_level(data.size(), *qrcode_version, *mode);

  if (get_max_capacitie(*qrcode_version, *error_code_correction_level, *mode) <
      data.size())
    throw std::runtime_error("Given parameters aren't compatible with data");
}

void read_user_file(string &data, uint8_t *qrcode_version,
                    ERROR_CORRECTION_LEVEL *error_code_correction_level,
                    MODE_INDICATOR *mode, uint8_t *mask_code, char *filename) {
  std::ifstream fichier(filename);

  if (!fichier) {
    std::cerr << "Can not open the file " << filename << std::endl;
    throw std::runtime_error("Error when reading the input file");
  }

  // Use a stringstream to read the file contents into a string
  std::stringstream buffer;
  buffer << fichier.rdbuf();

  data = buffer.str();

  std::cout << data << std::endl;

  fichier.close();

  *mode = is_numeric(data)        ? NUMERIC
          : is_alphanumeric(data) ? ALPHANUMERIC
                                  : BYTE;

  *qrcode_version = determine_version(data.length(), *mode, L);
  *error_code_correction_level =
      upgrade_err_level(data.size(), *qrcode_version, *mode);
  *mask_code = 8;
}
