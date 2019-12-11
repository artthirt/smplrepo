#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QGLWidget>
#include <QTimer>
#include <QTime>
#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>

//#include "camcapture.h"

class VideoFrame : public QGLWidget, private QOpenGLFunctions
{
    Q_OBJECT
public:
	enum{FPS_TIMEWAIT = 2000};

    explicit VideoFrame(QWidget *parent = nullptr);
    ~VideoFrame();

    void setImage(const QImage& image);

    void setScale(float v);

    void startCVCamera();

	void setRotateBy90(bool v);

signals:

public slots:
    void onTimeout();
    void onReceiveImage(const QImage& image);

private:
    QImage m_image;
    bool m_is_update;
    bool m_is_tex_update;
    QTimer m_timer;
	QTime m_time_fps;
	float m_fps;
	uint m_counter_fps;

	int m_prev_width	= 0;
	int m_prev_height	= 0;
	int m_prev_type		= 0;

	bool m_rotateBy90;

    uint m_bindTex;
    int m_mvpInt;
    int m_vecInt;
    int m_texInt;
    int m_utexInt;

    float m_scale;
    QVector2D m_offset;

    QGLShaderProgram m_shpr;

    QMatrix4x4 m_mvp;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_model;

    std::vector<float> m_vertexBuffer;
    std::vector<float> m_textureBuffer;

//    CamCapture *m_camCapture;

    void setViewport(float w, float h);
    void generateTexture();
    void drawImage();

    // QGLWidget interface
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    // QObject interface
public:
    bool event(QEvent *event);

	// QWidget interface
protected:
	void paintEvent(QPaintEvent *event);
};

#endif // GLWIDGET_H
