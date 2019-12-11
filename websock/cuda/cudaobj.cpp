#include "cudaobj.h"

#include <cuda_runtime.h>
#include <cuda_runtime_api.h>

#include "convert_cuda.h"

CudaObj::CudaObj()
{

}

CudaObj::~CudaObj()
{
	clear();
}

size_t CudaObj::size() const
{
	return m_size;
}

void *CudaObj::mem() const
{
	return m_mem;
}

bool CudaObj::init(size_t size)
{
	if(!size)
		return false;
	if(!empty() && size && size != m_size){
		clear();
	}
	m_size = size;

	cudaError_t err = cudaMalloc(&m_mem, m_size);

	return err == cudaSuccess;
}

void CudaObj::clear()
{
	if(!m_size || !m_mem)
		return;

	cudaFree(m_mem);
	m_mem = nullptr;
	m_size = 0;
}

bool CudaObj::empty() const
{
	return m_mem == nullptr;
}

bool CudaObj::copyTo(void *data, size_t len)
{
	if(!data || !m_mem || len > m_size)
		return false;

	cudaError_t err = cudaMemcpy(data, m_mem, len, cudaMemcpyHostToDevice);
	return err == cudaSuccess;
}

bool CudaObj::copyTo(void *data)
{
	if(!data || !m_mem )
		return false;

	cudaError_t err = cudaMemcpy(m_mem, data, m_size, cudaMemcpyHostToDevice);
	return err == cudaSuccess;
}

bool CudaObj::copyFrom(void *data, size_t len)
{
	if(!data || !m_mem || len > m_size)
		return false;

	cudaError_t err = cudaMemcpy(data, m_mem, len, cudaMemcpyDeviceToHost);
	return err == cudaSuccess;
}

bool CudaObj::copyFrom(void *data)
{
	if(!data || !m_mem)
		return false;

	cudaError_t err = cudaMemcpy(data, m_mem, m_size, cudaMemcpyDeviceToHost);
	return err == cudaSuccess;
}

/////////////////////////////////////////////
/////////////////////////////////////////////


QImage ConvertImageCu::createImage(AVFrame *picture)
{
	if(picture->width != m_output.width() || picture->height != m_output.height()){
		m_output = QImage(picture->width, picture->height, QImage::Format_RGB888);

		size_t Ysize = picture->linesize[0] * picture->height;
		size_t Usize = picture->linesize[1] * picture->height/2;
		size_t Vsize = picture->linesize[2] * picture->height/2;

		size_t RGBsize = picture->width * picture->height * 3;

		m_Y.init(Ysize);
		m_U.init(Usize);
		m_V.init(Vsize);
		m_Rgb.init(RGBsize);
	}

	m_Y.copyTo(picture->data[0]);
	m_U.copyTo(picture->data[1]);
	m_V.copyTo(picture->data[2]);

	int lsRgb = m_output.width() * 3;

	cudaStreamQuery(0);
	convert_yuv((const uint8_t*)m_Y.mem(), picture->linesize[0],
			(const uint8_t*)m_U.mem(), picture->linesize[1],
			(const uint8_t*)m_V.mem(), picture->linesize[2],
			(uint8_t*)m_Rgb.mem(), lsRgb, picture->width, picture->height);
	cudaStreamQuery(0);

	m_Rgb.copyFrom(m_output.bits());
	cudaStreamQuery(0);

	return m_output;
}
