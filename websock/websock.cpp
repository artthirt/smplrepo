#include "websock.h"

#include <stdio.h>
#include <iostream>

#include <QDataStream>
#include <QImageReader>

#include <QFile>

#ifdef _MSC_VER
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

#include <omp.h>

#include "common.h"

#define MAX_FRAMES			8

#define MAX_BUFFERS			3000

const uchar JPEG_STRING[] = {0xff, 0xd8, 0xff};

bool check_string(const char* data1, const char* data2, int len)
{
	if(!data1 || !data2){
		return false;
	}
	int i = 0;
	const char* b1 = data1, *b2 = data2;
	while(*b1++ == *b2++ && i++ < len){}
	return 	i == len;
}


inline int clamp(int x){
	return x > 255? 255 : (x < 0? 0 : x);
}

inline int minFast(int a, int b)
{
	int z = a - b;
	int i = (~(z >> 31)) & 0x1;
	return a - i * z;
}

inline int maxFast(int a, int b)
{
	int c = minFast(-a, -b);
	return -c;
}

inline int clampFast(int x)
{
	return minFast(255, maxFast(0, x));
}

inline int _Rgb(uchar r, uchar g, uchar b)
{
	return (int)(r | (g << 8) | (b << 16));
}

inline int conv_yuv_to_rgb(int Y, int U, int V)
{
    int R, G, B, C, D, E;

	C = Y - 16;
	D = U - 128;
	E = V - 128;

    R = clamp((298 * C + 409 * E + 128) >> 8);
    G = clamp((298 * C - 100 * D - 208 * E + 128) >> 8);
    B = clamp((298 * C + 516 * D + 128) >> 8);

    return qRgb(R, G, B);
}

#ifdef USE_MMX

#include <immintrin.h>

union SHI64{
    short s[4];
    long long ll;
};

inline void conv_yuv_to_rgb(uchar Y0, uchar Y1, int U, int V, QRgb &rgb0, QRgb &rgb1)
{
    short C0, C1, D, E;

    SHI64 RGB0, RGB1;

    C0 = Y0 - 16;
    C1 = Y1 - 16;
    D = U - 128;
    E = V - 128;

    int T0 = 409 * E + 128;
    int T1 = -100 * D - 208 * E + 128;
    int T2 = 516 * D + 128;

    __m64 mRGB0, mRGB1, r1, r2, t1;

    RGB0.s[2] = ((298 * C0 + T0) >> 8);
    RGB0.s[1] = ((298 * C0 + T1) >> 8);
    RGB0.s[0] = ((298 * C0 + T2) >> 8);

    RGB1.s[2] = ((298 * C1 + T0) >> 8);
    RGB1.s[1] = ((298 * C1 + T1) >> 8);
    RGB1.s[0] = ((298 * C1 + T2) >> 8);

    mRGB0 = _mm_cvtsi64x_si64(RGB0.ll);
    mRGB1 = _mm_cvtsi64x_si64(RGB1.ll);

    r1 = _m_packuswb(mRGB0, t1);
    r2 = _m_packuswb(mRGB1, t1);

    rgb0 = (QRgb)_m_to_int(r1);
    rgb1 = (QRgb)_m_to_int(r2);
    //rgb0 = qRgb(RGB0[0], RGB0[1], RGB0[2]);
    //rgb1 = qRgb(RGB1[0], RGB1[1], RGB1[2]);
}

#else

inline void conv_yuv_to_rgb(uchar Y0, uchar Y1, int U, int V, QRgb &rgb0, QRgb &rgb1)
{
    int R0, G0, B0, R1, G1, B1, C0, C1, D, E;

	C0 = Y0 - 16;
	C1 = Y1 - 16;
	D = U - 128;
	E = V - 128;

	int T0 = 409 * E + 128;
	int T1 = -100 * D - 208 * E + 128;
    int T2 = 516 * D + 128;

    R0 = clamp((298 * C0 + T0) >> 8);
    G0 = clamp((298 * C0 + T1) >> 8);
    B0 = clamp((298 * C0 + T2) >> 8);

    R1 = clamp((298 * C1 + T0) >> 8);
    G1 = clamp((298 * C1 + T1) >> 8);
    B1 = clamp((298 * C1 + T2) >> 8);

    rgb0 = qRgb(R0, G0, B0);
    rgb1 = qRgb(R1, G1, B1);
}

#endif

//////////////////////////////

//////////////////////////////

WebSock::WebSock(QObject *parent) : QThread(parent)
{
	av_register_all();
	avcodec_register_all();

	m_codec = nullptr;
	m_ctx = nullptr;

	m_done = false;

	m_fileName = "test.bin";

    memset(&m_frame, 0, sizeof(m_frame));

	initH264();

    m_decodeThread.reset(new std::thread(std::bind(&WebSock::doSendPktToCodec, this)));
    //m_decodeThread2.reset(new std::thread(std::bind(&WebSock::doGetDecodedFrame, this)));
}

WebSock::~WebSock()
{
	m_done = true;

	quit();
	wait();

    if(m_decodeThread.get()){
		m_decodeThread->join();
	}

    if(m_decodeThread2.get()){
        m_decodeThread2->join();
    }

    if(m_ctx){
#if LIBAVCODEC_VERSION_MAJOR > 56
		avcodec_free_context(&m_ctx);
#else
		av_freep(&m_ctx);
#endif
	}
}

void WebSock::setRecord(bool val)
{
	m_isRecord = val;
}

bool WebSock::isRecord() const
{
	return m_isRecord;
}

void WebSock::setFileName(const QString &fn)
{
	m_fileName = fn;
}

QString WebSock::fileName() const
{
	return m_fileName;
}

void WebSock::onReadyRead()
{
	while(m_sock->hasPendingDatagrams()){
		QByteArray data;
		data.resize(m_sock->pendingDatagramSize());
		m_sock->readDatagram(data.data(), data.size());
		tryParseData(data);
	}
}

void WebSock::onTimeout()
{
	while(!m_frames.empty() && m_frames.front().done){
		m_mutex.lock();
		Frame frame = m_frames.front();
		m_frames.pop();
		m_mutex.unlock();

		emit sendImage(frame.image);
	}
}

void WebSock::run()
{
	m_timer.reset(new QTimer);
	m_timer->moveToThread(this);
	connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(onTimeout()));
	m_timer->start(10);

	m_sock.reset(new QUdpSocket);
	m_sock->moveToThread(this);
	m_sock->bind(8000);

	int buf = 7 * 1024 * 1024;
	setsockopt(m_sock->socketDescriptor(), SOL_SOCKET, SO_RCVBUF, (char*)&buf, sizeof(buf));

	connect(m_sock.get(), SIGNAL(readyRead()), this, SLOT(onReadyRead()));

	exec();

	m_sock.reset();
}

void WebSock::tryParseData(const QByteArray &data)
{
	//printf("data size %d\n", data.size());

	m_vData.addData(data, false);

	while(m_vData.frameCount()){
		Packet pkt = m_vData.getFrame();
		m_vData.removeFrame();

		if(check_string(pkt.data, (char*)JPEG_STRING, 3)){
			if(m_frames.size() < MAX_FRAMES){
					m_mutex.lock();
					m_frames.push(Frame(pkt.data));
					m_mutex.unlock();
					m_frames.back().start();
				}
		}else{
			if(m_framesH264.size() < MAX_BUFFERS){
				//m_mutexh.lock();
				m_framesH264.push(pkt.data);
				//m_mutexh.unlock();
			}
			if(m_isRecord){
				QFile f(m_fileName);
				f.open(QIODevice::WriteOnly | QIODevice::Append);
				uint size = pkt.data.size();
				f.write((char*)&size, sizeof(size));
				f.write(pkt.data);
				f.close();
			}
		}
//		QFile f("1.jpg");
//		f.open(QIODevice::WriteOnly);
//		f.write(pkt.data);
//		f.close();

	}
}

void WebSock::initH264()
{
	m_codec = avcodec_find_decoder(AV_CODEC_ID_H264);

	if(!m_codec){
		qDebug("codec h.264 not found");
		return;
	}else{
		m_ctx = avcodec_alloc_context3(m_codec);

        AVDictionary *dict = nullptr;
        av_dict_set(&dict, "threads", "auto", 0);
        av_dict_set(&dict, "framerate", "30", 0);
        av_dict_set(&dict, "refcounted_frames", "1", 0);
        av_dict_set_int(&dict, "lowres", m_codec->max_lowres, 0);

        if(avcodec_open2(m_ctx, m_codec, &dict) < 0) {
			printf("Error: could not open codec.\n");
			return;
		}
        m_ctx->pkt_timebase.den = 1200000;
        m_ctx->pkt_timebase.num = 1;
    }
}

void WebSock::doGetDecodedFrame()
{
//    while(!m_done){
//        if(m_is_update_frame){
//            decodeH264();
//        }else{
//            std::this_thread::sleep_for(std::chrono::milliseconds(2));
//        }
//    }
}

void WebSock::doSendPktToCodec()
{
	while(!m_done){
        decodeH264();

        if(!m_framesH264.empty()){
			//m_mutexh.lock();
			QByteArray data = m_framesH264.front();
			//m_mutexh.unlock();
            m_framesH264.pop();

            bool res = doSendPkt(data);
//            if(res){
//            }else{
//                std::this_thread::sleep_for(std::chrono::milliseconds(2));
//            }
        }else{
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
}

bool WebSock::doSendPkt(const QByteArray& data)
{
    AVPacket pkt;
    av_init_packet(&pkt);

    pkt.dts = AV_NOPTS_VALUE;
    pkt.stream_index = 0;
    pkt.flags = 0;
    pkt.side_data = nullptr;
    pkt.side_data_elems = 0;
    pkt.duration = 0;
    pkt.pos = -1;

    pkt.pts = m_numpack;
    pkt.data = (uint8_t*)data.data();
    pkt.size = data.size();

    int res = 0;

    do{
		//m_mutexh.lock();
        res = avcodec_send_packet(m_ctx, &pkt);
		//m_mutexh.unlock();
    }while(res == AVERROR(EAGAIN));

    if(res < 0){
//        char buf[256] = {0};
//        av_make_error_string(buf, 256, res);
//        qDebug("error %s \n", buf);
    }else{
    }
    return res != AVERROR(EAGAIN);
}

void WebSock::decodeH264()
{
//	AVFrame *picture = av_frame_alloc();
//	memset(picture, 0, sizeof(picture));

    if(parseH264(&m_frame)){
        av_frame_unref(&m_frame);
    }
//	av_frame_free(&picture);
}

bool WebSock::parseH264(AVFrame *picture)
{
    int res = 0;
    do{
		//m_mutexh.lock();
        res = avcodec_receive_frame(m_ctx, picture);
		//m_mutexh.unlock();

        if(res == 0){
            createImage(picture);
        }
    }while(res != AVERROR(EAGAIN));
    return res >= 0;
}

void WebSock::createImage(AVFrame *picture)
{
#ifdef USE_OPENCL
	QImage image = m_convertImage.createImage(picture);
#else

	QImage image(picture->width, picture->height, QImage::Format_ARGB32);

//    saveImage(picture, "test.image");

    int numthr = omp_get_num_procs();

	if(numthr > 6)
		omp_set_num_threads(numthr/2);

#pragma omp parallel for
	for(int y = 0; y < picture->height; ++y){
		uint8_t* il = &picture->data[0][y * picture->linesize[0]];
		QRgb* sc = (QRgb*)image.scanLine(y);
		for(int x = 0; x < picture->width; ++x){
			uchar r = il[x];
			sc[x] = r;
		}
	}

#pragma omp parallel for
	for(int y = 0; y < picture->height >> 1; ++y){
		uint8_t* il1 = &picture->data[1][y * picture->linesize[1]];
		uint8_t* il2 = &picture->data[2][y * picture->linesize[2]];
		QRgb* sc1 = (QRgb*)image.scanLine((y << 1) + 0);
		QRgb* sc2 = (QRgb*)image.scanLine((y << 1) + 1);
		for(int x = 0; x < picture->width >> 1; ++x){
			uchar g = 0, b = 0;
			g = il1[x];
			b = il2[x];


			conv_yuv_to_rgb((uchar)sc1[(x << 1)], (uchar)sc1[(x << 1) + 1], g, b, sc1[(x << 1)], sc1[(x << 1) + 1]);
			conv_yuv_to_rgb((uchar)sc2[(x << 1)], (uchar)sc2[(x << 1) + 1], g, b, sc2[(x << 1)], sc2[(x << 1) + 1]);
		}
	}

#endif

	if(m_frames.size() < MAX_BUFFERS){
		m_mutex.lock();
		m_frames.push(Frame(image));
		m_mutex.unlock();
    }
}

bool WebSock::event(QEvent *ev)
{
    if(ev->type() == EventTest::EVENT){
        EventTest *et = (EventTest*)ev;
        m_framesH264.push(et->data);
        return true;
    };
    return QThread::event(ev);
}

///////////////////////////
///////////////////////////

WebSock::Frame::Frame()
{

}

WebSock::Frame::Frame(const QByteArray &data)
{
	this->data = data;
}

WebSock::Frame::Frame(const WebSock::Frame &frame)
{
	data = frame.data;
	done = frame.done;
	image = frame.image;
}

WebSock::Frame::Frame(const QImage &image)
{
	this->image = image;
	done = true;
}

WebSock::Frame::~Frame()
{
	if(thr.get()){
		thr->join();
	}
}

void WebSock::Frame::start()
{
	thr.reset(new std::thread(std::bind(&Frame::encode, this)));
}

void WebSock::Frame::encode()
{
	QDataStream stream(data);

	QImageReader reader(stream.device(), "jpeg");

	image = reader.read();

	done = true;
}

//////////////////////////////////

void saveImage(AVFrame* picture, const QString& fileName)
{
    QFile f(fileName);
    f.open(QIODevice::WriteOnly);

    f.write((char*)&picture->width, sizeof(int));
    f.write((char*)&picture->height, sizeof(int));
    f.write((char*)picture->linesize, sizeof(picture->linesize));
    f.write((char*)picture->data[0], picture->linesize[0] * picture->height);
    f.write((char*)picture->data[1], picture->linesize[1] * picture->height);
    f.write((char*)picture->data[2], picture->linesize[2] * picture->height);

    f.close();
}

void loadImage(const QString &fileName, Image *picture)
{
    if(!QFile::exists(fileName))
        return;

    QFile f(fileName);

    f.open(QIODevice::ReadOnly);

    f.read((char*)&picture->width, sizeof(int));
    f.read((char*)&picture->height, sizeof(int));
    f.read((char*)picture->linesize, sizeof(picture->linesize));

    picture->data[0].resize(picture->linesize[0] * picture->height);
    picture->data[1].resize(picture->linesize[1] * picture->height);
    picture->data[2].resize(picture->linesize[2] * picture->height);

    f.read((char*)picture->data[0].data(), picture->linesize[0] * picture->height);
    f.read((char*)picture->data[1].data(), picture->linesize[1] * picture->height);
    f.read((char*)picture->data[2].data(), picture->linesize[2] * picture->height);

    f.close();
}

/////////////////////////

QImage createImage(const Image *picture)
{
    if(!picture || picture->empty())
        return QImage();

    QImage image(picture->width, picture->height, QImage::Format_ARGB32);

    int numthr = omp_get_num_procs();

    if(numthr > 6)
        omp_set_num_threads(numthr/2);

#pragma omp parallel for
    for(int y = 0; y < picture->height; ++y){
        uint8_t* il = (uint8_t*)&picture->data[0].data()[y * picture->linesize[0]];
        QRgb* sc = (QRgb*)image.scanLine(y);
        for(int x = 0; x < picture->width; ++x){
            uchar r = il[x];
            sc[x] = r;
        }
    }

#pragma omp parallel for
    for(int y = 0; y < picture->height >> 1; ++y){
        uint8_t* il1 = (uint8_t*)&picture->data[1].data()[y * picture->linesize[1]];
        uint8_t* il2 = (uint8_t*)&picture->data[2].data()[y * picture->linesize[2]];
        QRgb* sc1 = (QRgb*)image.scanLine((y << 1) + 0);
        QRgb* sc2 = (QRgb*)image.scanLine((y << 1) + 1);
        for(int x = 0; x < picture->width >> 1; ++x){
            uchar g = 0, b = 0;
            g = il1[x];
            b = il2[x];


            conv_yuv_to_rgb((uchar)sc1[(x << 1)], (uchar)sc1[(x << 1) + 1], g, b, sc1[(x << 1)], sc1[(x << 1) + 1]);
            conv_yuv_to_rgb((uchar)sc2[(x << 1)], (uchar)sc2[(x << 1) + 1], g, b, sc2[(x << 1)], sc2[(x << 1) + 1]);

        }
    }

    return image;
}

//////////////////////////////////

ConvertImage::ConvertImage(){
	cl_::clMainObject::instance().init();

	m_progname = ":/cl/convert.cl";

	QString str;
	QFile f(QString::fromStdString(m_progname));
	if(f.open(QIODevice::ReadOnly)){
		str = f.readAll();
		f.close();
	}


	m_program = cl_::clMainObject::instance().getProgram(str.toStdString());
	bool res = cl_::clMainObject::instance().buildProgram(m_program);
	std::string s = m_program->getBuildInfoString();
	QString qs = QString::fromStdString(s);
	qDebug("out: %s", s.c_str());

	m_kernel = m_program->createKernel("convert");

}

QImage ConvertImage::createImage(IMAGE *picture)
{
	if(picture->width != m_image.width || picture->height != m_image.height){
		m_program->freeBuffers();
		m_image.width = picture->width;
		m_image.height = picture->height;

		m_output = QImage(picture->width, picture->height, QImage::Format_RGB888);

		size_t Ysize = picture->linesize[0] * picture->height;
		size_t Usize = picture->linesize[1] * picture->height/2;
		size_t Vsize = picture->linesize[2] * picture->height/2;

		size_t RGBsize = picture->width * picture->height * 3;

		m_Y = m_program->createBuffer(Ysize, cl_::clProgram::READWRITE);
		m_U = m_program->createBuffer(Usize, cl_::clProgram::READWRITE);
		m_V = m_program->createBuffer(Vsize, cl_::clProgram::READWRITE);
		m_Rgb = m_program->createBuffer(RGBsize, cl_::clProgram::READ);
	}

	m_program->write(m_Y, picture->data[0]);
	m_program->write(m_U, picture->data[1]);
	m_program->write(m_V, picture->data[2]);

	m_program->setArg(m_kernel, 0, m_Y);
	m_program->setArg(m_kernel, 1, m_U);
	m_program->setArg(m_kernel, 2, m_V);
	m_program->setArg(m_kernel, 3, m_Rgb);
	m_program->setArg(m_kernel, 4, picture->linesize[0]);
	m_program->setArg(m_kernel, 5, picture->linesize[1]);
	m_program->setArg(m_kernel, 6, picture->linesize[2]);
	m_program->setArg(m_kernel, 7, picture->width * 3);
	m_program->setArg(m_kernel, 8, picture->width);
	m_program->setArg(m_kernel, 9, picture->height);

	bool res = cl_::clMainObject::instance().run(m_kernel, m_program, picture->width * picture->height);

	res = m_program->read(m_Rgb, m_output.bits());

	return m_output;
}

QImage ConvertImage::createImage(Image *picture)
{
	if(picture->width != m_image.width || picture->height != m_image.height){
		m_program->freeBuffers();
		m_image.width = picture->width;
		m_image.height = picture->height;

		m_output = QImage(picture->width, picture->height, QImage::Format_RGB888);

		size_t Ysize = picture->linesize[0] * picture->height;
		size_t Usize = picture->linesize[1] * picture->height/2;
		size_t Vsize = picture->linesize[2] * picture->height/2;

		size_t RGBsize = picture->width * picture->height * 3;

		m_Y = m_program->createBuffer(Ysize, cl_::clProgram::WRITE);
		m_U = m_program->createBuffer(Usize, cl_::clProgram::WRITE);
		m_V = m_program->createBuffer(Vsize, cl_::clProgram::WRITE);
		m_Rgb = m_program->createBuffer(RGBsize, cl_::clProgram::READ);
	}

	m_program->write(m_Y, picture->data[0].data());
	m_program->write(m_U, picture->data[1].data());
	m_program->write(m_V, picture->data[2].data());

	m_program->setArg(m_kernel, 0, m_Y);
	m_program->setArg(m_kernel, 1, m_U);
	m_program->setArg(m_kernel, 2, m_V);
	m_program->setArg(m_kernel, 3, m_Rgb);
	m_program->setArg(m_kernel, 4, picture->linesize[0]);
	m_program->setArg(m_kernel, 5, picture->linesize[1]);
	m_program->setArg(m_kernel, 6, picture->linesize[2]);
	m_program->setArg(m_kernel, 7, picture->width * 3);
	m_program->setArg(m_kernel, 8, picture->width);
	m_program->setArg(m_kernel, 9, picture->height);

	bool res = cl_::clMainObject::instance().run(m_kernel, m_program, picture->width * picture->height);

	res = m_program->read(m_Rgb, m_output.bits());

	return m_output;
}
