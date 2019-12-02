#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QGLWidget>
#include <QTimer>
#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>

//#include "camcapture.h"

class VideoFrame : public QGLWidget, private QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit VideoFrame(QWidget *parent = nullptr);
    ~VideoFrame();

    void setImage(const QImage& image);

    void setScale(float v);

    void startCVCamera();

signals:

public slots:
    void onTimeout();
    void onReceiveImage(const QImage& image);

private:
    QImage m_image;
    bool m_is_update;
    bool m_is_tex_update;
    QTimer m_timer;

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
};

#endif // GLWIDGET_H
