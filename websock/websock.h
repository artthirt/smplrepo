#ifndef WEBSOCK_H
#define WEBSOCK_H

#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QTimer>
#include <QImage>
#include <thread>

#include <memory>
#include <queue>
#include <mutex>

#include "common.h"

extern "C"{
#	include "libavcodec/avcodec.h"
#	include "libavformat/avformat.h"
}

#include "videodata.h"

#ifdef USE_CUDA
#include "cudaobj.h"
#endif

#ifdef USE_OPENCL

#include "cl_main_object.h"

#define IMAGE	AVFrame

class ConvertImage{
public:
	ConvertImage();

	PImage createImage(AVFrame *picture);
	PImage createImage(Image *picture);

private:
	cl_::_clProgram m_program;
	cl_::clKernel m_kernel;
	cl_::clBuffer m_Y;
	cl_::clBuffer m_U;
	cl_::clBuffer m_V;
	cl_::clBuffer m_Rgb;

	Image m_image;

	std::string m_progname;
};

#endif

class WebSock : public QThread
{
	Q_OBJECT
public:
	explicit WebSock(QObject *parent = nullptr);
	~WebSock();

	void setRecord(bool val);
	bool isRecord()  const;

	void setFileName(const QString &fn);
	QString fileName() const;

signals:
    void sendImage(P1Image);

public slots:
	void onReadyRead();
	void onTimeout();

protected:
	virtual void run();

private:
	std::unique_ptr<QUdpSocket> m_sock;
	VideoData m_vData;
	QString m_fileName;

	struct Frame{
		QByteArray data;
		bool done = false;
		bool h264 = false;
        P1Image image;

		std::unique_ptr<std::thread> thr;

		Frame();
		Frame(const QByteArray& data);
		Frame(const Frame &frame);
        Frame(const P1Image& image);
		~Frame();

		void start();

		void encode();
	};

	std::queue<Frame> m_frames;
	std::queue<QByteArray> m_framesH264;
	std::queue<AVFrame*> m_avframes;
	std::mutex m_mutex;
	std::mutex m_mutexh;

	bool m_isRecord;

	std::unique_ptr<QTimer> m_timer;

	std::unique_ptr<std::thread> m_decodeThread;
	std::unique_ptr<std::thread> m_decodeThread2;

	void tryParseData(const QByteArray &data);

	AVCodec* m_codec		= nullptr;
	AVCodecContext *m_ctx	= nullptr;
	AVFrame m_frame;
	bool m_done				= false;
	uint m_numpack			= 0;

#ifdef USE_CUDA
	ConvertImageCu m_convertImageCu;
#endif

#ifdef USE_OPENCL
	ConvertImage m_convertImage;
#endif

	void initH264();
	void doGetDecodedFrame();
	void doSendPktToCodec();
	bool doSendPkt(const QByteArray &data);
	void decodeH264();
	bool parseH264();
	void createImage(AVFrame *picture);

	// QObject interface
public:
	bool event(QEvent *ev);
};

/**
 * @brief saveImage
 * save avframe to custom format
 * @param picture
 * @param fileName
 */
void saveImage(AVFrame* picture, const QString& fileName);
/**
 * @brief loadImage
 * load from custom format to Image
 * @param fileName
 * @param picture
 */
void loadImage(const QString& fileName, Image *picture);
/**
 * @brief createImage
 * create QImage from Image
 * @param picture
 * @return
 */
PImage createImage(const Image *picture);

//////////////////////////////////////

#endif // WEBSOCK_H
