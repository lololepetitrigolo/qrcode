#ifndef QRCODE_HPP
#define QRCODE_HPP

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

size_t get_max_capacitie(uint8_t qrcode_version,
                         ERROR_CORRECTION_LEVEL err_level, MODE_INDICATOR mode);

uint8_t compute_character_count_indicator(uint8_t qrcode_version,
                                          MODE_INDICATOR mode);

unsigned int determine_number_of_bits(ERROR_CORRECTION_LEVEL correction_level,
                                      uint8_t qrcode_version);

size_t determine_version(size_t data_length, MODE_INDICATOR mode,
                         ERROR_CORRECTION_LEVEL err_level);

ERROR_CORRECTION_LEVEL upgrade_err_level(size_t data_length,
                                         uint8_t qrcode_version,
                                         MODE_INDICATOR mode);

void add_character_count_indicator(string data, uint8_t qrcode_version,
                                   MODE_INDICATOR mode,
                                   vector<uint8_t> &encoded_data);

void add_mode_indicator(MODE_INDICATOR mode, vector<uint8_t> &encoded_data);

bool is_numeric(string data);

bool is_alphanumeric(string data);

vector<uint8_t> encode_numeric(string data, vector<uint8_t> &encoded_data);

vector<uint8_t> encode_alphanumeric(string data, vector<uint8_t> &encoded_data);

vector<uint8_t> encode_byte(string data, vector<uint8_t> &encoded_data);

void add_terminator(vector<uint8_t> &bits, unsigned long number_of_bits);

void add_padding(vector<uint8_t> &bits);

void add_extra_padding(vector<uint8_t> &bits, unsigned int number_of_bits);

void draw_finder_pattern(int x, int y, vector<vector<uint8_t>> &qrcode,
                         vector<vector<uint8_t>> &is_module_reserved);

void draw_alignment_pattern(uint8_t qrcode_version,
                            vector<vector<uint8_t>> &qrcode,
                            vector<vector<uint8_t>> &is_module_reserved);

void draw_timing_pattern(vector<vector<uint8_t>> &qrcode,
                         vector<vector<uint8_t>> &is_module_reserved);

void draw_dark_module(uint8_t qrcode_version, vector<vector<uint8_t>> &qrcode,
                      vector<vector<uint8_t>> &is_module_reserved);

void mark_reserved_information_module(
    uint8_t qrcode_version, vector<vector<uint8_t>> &is_module_reserved);

void draw_data(vector<vector<uint8_t>> &qrcode,
               vector<vector<uint8_t>> &is_module_reserved,
               vector<uint8_t> &data);

void apply_mask(uint8_t mask_number, vector<vector<uint8_t>> &qrcode,
                vector<vector<uint8_t>> &is_module_reserved);

void draw_format_information(uint8_t qrcode_version,
                             ERROR_CORRECTION_LEVEL error_code_correction_level,
                             uint8_t mask_code,
                             vector<vector<uint8_t>> &qrcode);

void add_padding_arround_qrcode(vector<vector<uint8_t>> &qrcode);

#endif // QRCODE_HPP
