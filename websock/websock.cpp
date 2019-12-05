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

#define MAX_FRAMES			8

#define MAX_BUFFERS			30

const uchar JPEG_STRING[] = {0xff, 0xd8, 0xff, 0xe0};

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

	initH264();

	m_decodeThread.reset(new std::thread(std::bind(&WebSock::doDecodeH264, this)));
}

WebSock::~WebSock()
{
	m_done = true;

	quit();
	wait();

	if(m_decodeThread){
		m_decodeThread->join();
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

	int buf = 5 * 1024 * 1024;
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

		if(check_string(pkt.data, (char*)JPEG_STRING, 4)){
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

        if(m_codec->capabilities & AV_CODEC_CAP_TRUNCATED) {
            m_ctx->flags |= AV_CODEC_FLAG_TRUNCATED;
		}
//		if(m_codec->capabilities & CODEC_FLAG2_CHUNKS)
        m_ctx->flags2 |= AV_CODEC_FLAG2_NO_OUTPUT;

        if( (m_codec->capabilities & AV_CODEC_CAP_TRUNCATED) != 0)
            m_ctx->flags |= AV_CODEC_FLAG_TRUNCATED;
        m_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY | AV_CODEC_FLAG_LOOP_FILTER;
		//m_ctx->flags |= CODEC_FLAG_LOOP_FILTER;

//		m_ctx->flags2 |= CODEC_FLAG2_FAST | CODEC_FLAG2_NO_OUTPUT
//				| CODEC_FLAG2_DROP_FRAME_TIMECODE | CODEC_FLAG2_IGNORE_CROP
//				| CODEC_FLAG2_SHOW_ALL;
        m_ctx->flags2 |= AV_CODEC_FLAG2_NO_OUTPUT | AV_CODEC_FLAG2_FAST;
        m_ctx->flags2 |= AV_CODEC_FLAG2_DROP_FRAME_TIMECODE | AV_CODEC_FLAG2_IGNORE_CROP | AV_CODEC_FLAG2_SHOW_ALL;
		m_ctx->bit_rate = 0;
		m_ctx->refcounted_frames = 0;
		m_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
		m_ctx->field_order = AV_FIELD_UNKNOWN;
		m_ctx->request_sample_fmt = AV_SAMPLE_FMT_NONE;
		m_ctx->workaround_bugs = FF_BUG_AUTODETECT;
		m_ctx->strict_std_compliance = FF_COMPLIANCE_NORMAL;
		m_ctx->error_concealment = FF_EC_DEBLOCK;
		m_ctx->thread_type = FF_THREAD_FRAME;
		m_ctx->pkt_timebase.num = 1;
		m_ctx->pkt_timebase.den = -1;
		m_ctx->extradata = 0;
		m_ctx->idct_algo = FF_IDCT_AUTO;
		m_ctx->error_concealment = FF_EC_DEBLOCK;
		m_ctx->thread_count = 0;
		m_ctx->thread_type = FF_THREAD_FRAME;
		m_ctx->thread_safe_callbacks = 0;
		m_ctx->skip_loop_filter = AVDISCARD_DEFAULT;
		m_ctx->skip_idct = AVDISCARD_DEFAULT;
		m_ctx->skip_frame = AVDISCARD_DEFAULT;

		const int CWidth = 640;
		const int CHeight = 480;

		m_ctx->width = CWidth;
		m_ctx->height = CHeight;
		m_ctx->coded_width = CWidth;
		m_ctx->coded_height = CHeight;

		if(avcodec_open2(m_ctx, m_codec, NULL) < 0) {
			printf("Error: could not open codec.\n");
			return;
		}
	}
}

void WebSock::doDecodeH264()
{
	while(!m_done){
		if(!m_framesH264.empty()){
			//m_mutexh.lock();
			QByteArray data = m_framesH264.front();
			m_framesH264.pop();
			//m_mutexh.unlock();

			decodeH264(data);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
}

void WebSock::decodeH264(const QByteArray &frame)
{
	AVFrame *picture = av_frame_alloc();
	memset(picture, 0, sizeof(picture));

	parseH264(frame, picture);

	av_frame_free(&picture);
}

void WebSock::parseH264(const QByteArray &data, AVFrame *picture)
{
	if(data.isEmpty())
		return;

//	{
//		uint sm = get_sum(data);
//		QString s = QString("bins/tmp_%1_%2.264")
//				.arg(numpack, 6, 10, QLatin1Char('0'))
//				.arg(sm, 6, 3, QLatin1Char('0'));
//		QFile file(s);
//		file.open(QIODevice::WriteOnly);
//		file.write(data);
//		file.close();
//	}

	AVPacket pkt;
	av_init_packet(&pkt);

	pkt.dts = AV_NOPTS_VALUE;
	pkt.stream_index = 0;
	pkt.flags = 0;
	pkt.side_data = 0;
	pkt.side_data_elems = 0;
	pkt.duration = 0;
	pkt.pos = -1;
	pkt.convergence_duration = AV_NOPTS_VALUE;

	pkt.pts = m_numpack;
	pkt.data = (uint8_t*)data.data();
	pkt.size = data.size();

	int got_picture = 0;

//	qDebug("packet size %d", data.size());

	int res = avcodec_decode_video2(m_ctx, picture, &got_picture, &pkt);

	if(res < 0){
//		QFile f("error.bin");
//		f.open(QIODevice::WriteOnly | QIODevice::Append);
//		f.write(data);
//		f.close();

		char buf[256] = {0};
		av_make_error_string(buf, 256, res);
		qDebug("error %s \n", buf);
	}

	if(!got_picture)
		return;

	if(res > 0){
//		qDebug("got picture %d. %d, [%d %d] %d", got_picture, res, m_picture->width,
//			   m_picture->height, m_picture->pict_type);

		createImage(picture);
	}
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
