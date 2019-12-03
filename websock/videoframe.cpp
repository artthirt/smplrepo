#include "videoframe.h"

#include <QMouseEvent>
#include <QWheelEvent>

VideoFrame::VideoFrame(QWidget *parent) : QGLWidget(parent)
//  , m_camCapture(0)
{
    setAutoFillBackground(false);
    m_is_tex_update = false;
    m_is_update = false;
    m_scale = 1;
	m_rotateBy90 = false;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer.start(15);
}

VideoFrame::~VideoFrame()
{
//    if(m_camCapture)
//        delete m_camCapture;
}

void VideoFrame::setImage(const QImage &image)
{
    m_image = image;
    m_is_tex_update = true;
    m_is_update = true;
}

void VideoFrame::setScale(float v)
{
    if(v < 1)
        v = 1;

    m_scale = v;
    m_is_update = true;
}

void VideoFrame::startCVCamera()
{
//    if(m_camCapture != 0)
//        return;
//    m_camCapture = new CamCapture;
//    m_camCapture->moveToThread(m_camCapture);
//    m_camCapture->start();

	//    connect(m_camCapture, SIGNAL(sendImage(QImage)), this, SLOT(onReceiveImage(QImage)), Qt::QueuedConnection);
}

void VideoFrame::setRotateBy90(bool v)
{
	m_rotateBy90 = v;
}

void VideoFrame::onTimeout()
{
    if(m_is_update){
        m_is_update = false;
        generateTexture();
        updateGL();
    }
}

void VideoFrame::onReceiveImage(const QImage &image)
{
    m_image = image;

	m_image.save("1.bmp");

    m_is_tex_update = true;
    m_is_update = true;
}

void VideoFrame::setViewport(float w, float h)
{
    float ar = w / h;
    m_proj.setToIdentity();
    m_proj.ortho(-ar, ar, -1, 1, 1, 10);
}

void addPt(std::vector< float >& buf, float x1, float x2, float x3)
{
    buf.push_back(x1);
    buf.push_back(x2);
    buf.push_back(x3);
}

void addPt(std::vector< float >& buf, float x1, float x2)
{
    buf.push_back(x1);
    buf.push_back(x2);
}

void VideoFrame::generateTexture()
{
    if(!m_is_tex_update || m_image.isNull())
        return;
    m_is_tex_update = false;

    int t = GL_RGB;
    if(m_image.format() == QImage::Format_ARGB32 || m_image.format() == QImage::Format_RGB32)
        t = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, m_bindTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, t, m_image.width(), m_image.height(), 0, t, GL_UNSIGNED_BYTE, m_image.bits());
}

inline void qmat2float(const QMatrix4x4& mat, float* data, int len = 16)
{
    for(int i = 0; i < len; ++i)
        data[i] = mat.constData()[i];
}

void VideoFrame::drawImage()
{
    setViewport(width(), height());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 1);

    m_shpr.bind();

    float mvp[16];

    m_model.setToIdentity();
    m_model.translate(0, 0, -2);

	if(m_rotateBy90)
		m_model.rotate(-90, 0, 0, 1);

    m_model.scale(m_scale, m_scale, 1);

    float arim = (float)m_image.width()/m_image.height();
    float ar = (float)width() / height();

	if(m_rotateBy90){
		arim = 1 / arim;
		if(ar > arim){
			m_model.scale(1, arim, 1);
		}else{
			m_model.scale(ar / arim, ar, 1);
		}
	}else{
		if(ar > arim){
			m_model.scale(arim, 1, 1);
		}else{
			m_model.scale(ar, ar / arim, 1);
		}
	}

    m_mvp = m_proj * m_model;

    qmat2float(m_mvp, mvp);

    glUniformMatrix4fv(m_mvpInt, 1, false, mvp);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_bindTex);

    glEnableVertexAttribArray(m_vecInt);
    glEnableVertexAttribArray(m_texInt);
    glVertexAttribPointer(m_texInt, 2, GL_FLOAT, false, 0, m_textureBuffer.data());
    glVertexAttribPointer(m_vecInt, 3, GL_FLOAT, false, 0, m_vertexBuffer.data());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexBuffer.size() / 3);
    glDisableVertexAttribArray(m_vecInt);
    glDisableVertexAttribArray(m_texInt);

    glDisable(GL_TEXTURE_2D);
}


void VideoFrame::initializeGL()
{
    QGLWidget::initializeGL();

    QOpenGLFunctions::initializeOpenGLFunctions();

    glGenTextures(1, &m_bindTex);
    //m_bindTex = 1;

    m_shpr.addShaderFromSourceCode(QGLShader::Vertex,
                                   "attribute vec3 aVec;\n"
                                   "attribute vec2 aTex;\n"
                                   "uniform mat4 uMvp;\n"
                                   "varying vec2 vTex;\n"
                                   "void main(){\n"
                                   "    gl_Position = uMvp * vec4(aVec, 1);\n"
                                   "    vTex = aTex;\n"
                                   "}");
    m_shpr.addShaderFromSourceCode(QGLShader::Fragment,
								   "//precision highp float;\n"
                                   "varying vec2 vTex;\n"
                                   "uniform sampler2D uTex;\n"
                                   "void main(){\n"
                                   "    gl_FragColor = texture2D(uTex, vTex).bgra;\n"
                                   "}");
    m_shpr.link();
    m_shpr.bind();

    m_vecInt = m_shpr.attributeLocation("aVec");
    m_texInt = m_shpr.attributeLocation("aTex");
    m_mvpInt = m_shpr.uniformLocation("uMvp");
    m_utexInt = m_shpr.uniformLocation("uTex");

    addPt(m_vertexBuffer, -1, -1, 0);
    addPt(m_vertexBuffer, -1, 1, 0);
    addPt(m_vertexBuffer, 1, -1, 0);
    addPt(m_vertexBuffer, 1, 1, 0);

    addPt(m_textureBuffer, 0, 1);
    addPt(m_textureBuffer, 0, 0);
    addPt(m_textureBuffer, 1, 1);
    addPt(m_textureBuffer, 1, 0);

    const uchar *s1 = glGetString(GL_VERSION);
    const uchar *s2 = glGetString(GL_RENDERER);
    const uchar *s3 = glGetString(GL_VENDOR);
    const uchar *s4 = glGetString(GL_EXTENSIONS);

    qDebug() << (char*)s1;
    qDebug() << (char*)s2;
    qDebug() << (char*)s3;
    qDebug() << (char*)s4;
}

void VideoFrame::resizeGL(int w, int h)
{
    QGLWidget::resizeGL(w, h);

    glViewport(0, 0, w, h);
    setViewport(w, h);
}

void VideoFrame::paintGL()
{
    QGLWidget::paintGL();

    drawImage();
}


bool VideoFrame::event(QEvent *event)
{
    if(event->type() == QEvent::MouseMove){

    }
    if(event->type() == QEvent::Wheel){
        QWheelEvent *w = (QWheelEvent*)event;
        setScale(m_scale + (float)w->delta()/20);

    }
    if(event->type() == QEvent::MouseButtonPress){

    }
    if(event->type() == QEvent::MouseButtonRelease){

    }

    return QGLWidget::event(event);
}
