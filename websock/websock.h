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
    std::unique_ptr<std::thread> m_decodeThread2;

	void tryParseData(const QByteArray &data);

	AVCodec* m_codec		= nullptr;
	AVCodecContext *m_ctx	= nullptr;
    AVFrame m_frame;
	bool m_done				= false;
	uint m_numpack			= 0;
    bool m_is_update_frame  = false;;

	void initH264();
    void doGetDecodedFrame();
    void doSendPktToCodec();
    bool doSendPkt(const QByteArray &data);
    void decodeH264();
    bool parseH264(AVFrame *picture);
	void createImage(AVFrame *picture);

    // QObject interface
public:
    bool event(QEvent *ev);
};

#endif // WEBSOCK_H
