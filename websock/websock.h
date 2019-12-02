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

extern "C"{
#	include "libavcodec/avcodec.h"
#	include "libavformat/avformat.h"
}

#include "videodata.h"

class WebSock : public QThread
{
	Q_OBJECT
public:
	explicit WebSock(QObject *parent = nullptr);
	~WebSock();

signals:
	void sendImage(QImage);

public slots:
	void onReadyRead();
	void onTimeout();

protected:
	virtual void run();

private:
	std::unique_ptr<QUdpSocket> m_sock;
	VideoData m_vData;

	struct Frame{
		QByteArray data;
		bool done = false;
		bool h264 = false;
		QImage image;

		std::unique_ptr<std::thread> thr;

		Frame();
		Frame(const QByteArray& data);
		Frame(const Frame &frame);
		~Frame();

		void start();

		void encode();
	};

	std::queue<Frame> m_frames;
	std::queue<QByteArray> m_framesH264;
	std::mutex m_mutex;
	std::mutex m_mutexh;

	std::unique_ptr<QTimer> m_timer;

	std::unique_ptr<std::thread> m_decodeThread;

	void tryParseData(const QByteArray &data);

	AVCodec* m_codec		= nullptr;
	AVCodecContext *m_ctx	= nullptr;
	bool m_done				= false;
	uint m_numpack			= 0;

	void initH264();
	void doDecodeH264();
	void decodeH264(const QByteArray& frame);
	void parseH264(const QByteArray &data, AVFrame *picture);
	void createImage(AVFrame *picture);
};

#endif // WEBSOCK_H
