#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>

#include "export.hpp"

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
