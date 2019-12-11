#ifndef CONVERT_CUDA_H
#define CONVERT_CUDA_H

#include "inttypes.h"

extern "C"{

void convert_yuv(const uint8_t* Y, int lsY, const uint8_t* U, int lsU, const uint8_t* V, int lsV,
                 uint8_t *Rgb, int lsRgb, int width, int height);

}

#endif // COVERT_CUDA_H
