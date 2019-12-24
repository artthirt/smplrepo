#ifndef CAMERASTREAM_H
#define CAMERASTREAM_H

#include <QObject>
#include <QThread>
#include <QCamera>
#include <QTimer>
#include <QImage>
#include <queue>
#include "videosurface.h"

#include <memory>
#include <thread>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class CameraStream : public QThread
{
	Q_OBJECT
public:
	explicit CameraStream(QObject *parent = nullptr);
	~CameraStream();

	void startPlay();
	void stopPlay();

	void setInitAV(bool val);

	bool initCamera();

signals:
    void sendPacket(QByteArray);
	void sendImage(QImage);
	void startCamera(bool);

public slots:
    void imageCaptured(int id, const QImage &preview);
    void onSendImage(const QImage& image);
	void onTimeout();
	void onStartCamera(bool val);

	void stateChanged(QCamera::State state);

protected:
	virtual void run();

private:
	QCamera *m_camera;
	std::shared_ptr<QTimer> m_timer;
	VideoSurface *m_videoSurface;
	uint m_numImage;
	QSize m_imageSize;
    QImage m_image;
	bool m_isPlay = false;
	bool m_InitAV = true;

    uint m_max_frames = 5;
    std::queue< QImage > m_frames;

    int64_t m_numFrame = 0;
	AVCodecContext *m_fmt = nullptr;
	AVCodec *m_codec = nullptr;

	bool m_isInitAV;

	void initContext(int width, int height);

    bool m_done = false;
    std::shared_ptr< std::thread > m_encoderThread;
    void doEncode();
    void encodeFrame(const QImage& image);
};

#endif // CAMERASTREAM_H
