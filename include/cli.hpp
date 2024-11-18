#ifndef CLI_HPP
#define CLI_HPP

#include "qrcode.hpp"
#include "reedsolomon.hpp"

using std::string;
using std::vector;

void read_user_input(string &data, uint8_t *qrcode_version,
                     ERROR_CORRECTION_LEVEL *error_code_correction_level,
                     MODE_INDICATOR *mode, uint8_t *mask_code);

void read_user_file(string &data, uint8_t *qrcode_version,
                    ERROR_CORRECTION_LEVEL *error_code_correction_level,
                    MODE_INDICATOR *mode, uint8_t *mask_code, char *filename);

#endif // CLI_HPP
