#include "convert_cuda.h"

#include <cuda.h>
#include <cuda_runtime.h>

#define BLOCKSIZE	32

namespace internal{

__device__ float clip(float v)
{
	if(v > 255)
		return 255;
	if(v < 0)
		return 0;
	return v;
}

__device__ float3 getRgb(float3 yuv)
{
	float3 vec;

	vec.x = yuv.x + 1.402 * (yuv.z - 128);
	vec.y = yuv.x - 0.344 * (yuv.y - 128) - 0.714 * (yuv.z - 128);
	vec.z = yuv.x + 1.772 * (yuv.y - 128);

	vec.x = clip(vec.x);
	vec.y = clip(vec.y);
	vec.z = clip(vec.z);
	return vec;
}


__global__ void convert(const uint8_t* Y, int lsY, const uint8_t* U, int lsU, const uint8_t* V, int lsV,
							uint8_t *Rgb, int lsRgb, int width, int height)
{
	int row = threadIdx.y + blockIdx.y * blockDim.y;
	int col = threadIdx.x + blockIdx.x * blockDim.x;

	if(row >= height || col >= width)
		return;


	int y2 = (int)(row/2.);
	int x2 = (int)(col/2.);

	float3 _rgb, yuv;

	yuv.x = Y[lsY * row + col];
	yuv.y = U[lsU * y2 + x2];
	yuv.z = V[lsV * y2 + x2];

	_rgb = getRgb(yuv);

	Rgb[lsRgb * row + col * 3 + 2] = _rgb.x;
	Rgb[lsRgb * row + col * 3 + 1] = _rgb.y;
	Rgb[lsRgb * row + col * 3 + 0] = _rgb.z;
}

}

extern "C"
void convert_yuv(const uint8_t* Y, int lsY, const uint8_t* U, int lsU, const uint8_t* V, int lsV,
				 uint8_t *Rgb, int lsRgb, int width, int height)
{
	int x1 = width / BLOCKSIZE + 1;
	int x2 = height / BLOCKSIZE + 1;

	dim3 dimGrid(x1, x2), dimBlock(BLOCKSIZE, BLOCKSIZE);

	internal::convert<<<dimGrid, dimBlock>>>(Y, lsY, U, lsU, V, lsV, Rgb, lsRgb, width, height);
}

