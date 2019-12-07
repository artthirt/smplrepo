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

//////////////////////////////

WebSock::WebSock(QObject *parent) : QThread(parent)
{
	av_register_all();
	avcodec_register_all();

	m_codec = nullptr;
	m_ctx = nullptr;

	m_done = false;

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
//			QFile f("test.jpg");
//			f.open(QIODevice::WriteOnly);
//			uint size = pkt.data.size();
//			//f.write((char*)&size, sizeof(size));
//			f.write(pkt.data);
//			f.close();
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
    while(!m_done){
        if(m_is_update_frame){
            decodeH264();
        }else{
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
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
        m_mutexh.lock();
        res = avcodec_send_packet(m_ctx, &pkt);
        m_mutexh.unlock();
    }while(res == AVERROR(EAGAIN));

    m_is_update_frame = true;
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
        m_mutexh.lock();
        res = avcodec_receive_frame(m_ctx, picture);
        m_mutexh.unlock();

        if(res == 0){
            m_is_update_frame = false;
            createImage(picture);
        }
    }while(res != AVERROR(EAGAIN));
    m_is_update_frame = false;
    return res >= 0;
}

void WebSock::createImage(AVFrame *picture)
{
#if 1
	QImage image(picture->width, picture->height, QImage::Format_ARGB32);

//#pragma omp parallel for
	for(int y = 0; y < picture->height; ++y){
		uint8_t* il = &picture->data[0][y * picture->linesize[0]];
		QRgb* sc = (QRgb*)image.scanLine(y);
		for(int x = 0; x < picture->width; ++x){
			uchar r = 0;
			r = il[x];
			sc[x] = r;
		}
	}

#if 1
//#pragma omp parallel for
	for(int y = 0; y < picture->height/2; ++y){
		uint8_t* il1 = &picture->data[1][y * picture->linesize[1]];
		uint8_t* il2 = &picture->data[2][y * picture->linesize[2]];
		QRgb* sc1 = (QRgb*)image.scanLine(2 * y + 0);
		QRgb* sc2 = (QRgb*)image.scanLine(2 * y + 1);
		for(int x = 0; x < picture->width/2; ++x){
			uchar g = 0, b = 0;
			g = il1[x];
			b = il2[x];

			int col1 = conv_yuv_to_rgb(sc1[2 * x], g, b);
			int col2 = conv_yuv_to_rgb(sc1[2 * x + 1], g, b);
			int col3 = conv_yuv_to_rgb(sc2[2 * x], g, b);
			int col4 = conv_yuv_to_rgb(sc2[2 * x + 1], g, b);

			sc1[2 * x] = col1;
			sc1[2 * x + 1] = col2;
			sc2[2 * x] = col3;
			sc2[2 * x + 1] = col4;
		}
	}

#endif

#else

#endif

	if(m_frames.size() < MAX_BUFFERS){
		m_mutex.lock();
		m_frames.push(Frame());
		m_frames.back().image = image;
		m_frames.back().done = true;
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
