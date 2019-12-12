#ifndef CUDAOBJ_H
#define CUDAOBJ_H

#include <QImage>

#include "common.h"

extern "C"{
#	include "libavcodec/avcodec.h"
#	include "libavformat/avformat.h"
}

class CudaObj
{
public:
	CudaObj();
	~CudaObj();

	size_t size() const;

	void *mem() const;

	bool init(size_t size);
	void clear();
	bool empty() const;

	bool copyTo(void* mem, size_t len);
	bool copyTo(void* mem);
	bool copyFrom(void* mem, size_t len);
	bool copyFrom(void* mem);

private:
	void *m_mem		= nullptr;
	size_t m_size	= 0;
};

class ConvertImageCu{
public:
	PImage createImage(AVFrame* picture);
private:
	CudaObj m_Y;
	CudaObj m_U;
	CudaObj m_V;
	CudaObj m_Rgb;

	Image m_image;
};


#endif // CUDAOBJ_H
