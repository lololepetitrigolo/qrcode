#ifndef REEDSOLOMON_HPP
#define REEDSOLOMON_HPP

#include <cassert>
#include <cstdint>
#include <vector>

enum ERROR_CORRECTION_LEVEL { L = 1, M = 0, Q = 3, H = 2 };

std::vector<uint8_t>
addEccAndInterleave(std::vector<uint8_t> &data, unsigned short version,
                    ERROR_CORRECTION_LEVEL errorCorrectionLevel);

#endif // REEDSOLOMON_HPP
