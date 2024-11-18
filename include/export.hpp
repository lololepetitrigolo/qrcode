#ifndef EXPORT_HPP
#define EXPORT_HPP

#include <cstdint>
#include <vector>

using std::string;
using std::vector;

vector<vector<uint8_t>> scaleUpQrcode(vector<vector<uint8_t>> &qrcode);

void saveQRCodeToPGM(vector<std::vector<uint8_t>> &qrcode,
                     const string &filename);
#endif // EXPORT_HPP
