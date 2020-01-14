#include "videoframe.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <QPainter>

VideoFrame::VideoFrame(QWidget *parent) : QGLWidget(parent)
//  , m_camCapture(0)
{
    qRegisterMetaType<PImage>("P1Image");

	setAutoFillBackground(false);
	m_is_tex_update = false;
	m_is_update = false;
	m_scale = 1;
	m_rotateBy90 = false;

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	m_timer.start(1);

	m_counter_fps = 0;
	m_fps = 0;
	m_time_fps.start();
}

VideoFrame::~VideoFrame()
{
//    if(m_camCapture)
//        delete m_camCapture;
}

void VideoFrame::setRedBrightness(float v)
{
    m_Rgb[0] = v;
    m_is_update = true;
}

void VideoFrame::setGreenBrightness(float v)
{
    m_Rgb[1] = v;
    m_is_update = true;
}

void VideoFrame::setBlueBrightness(float v)
{
    m_Rgb[2] = v;
    m_is_update = true;
}

void VideoFrame::setEV(float ev)
{
    m_eV = ev;
    m_is_update = true;
}

void VideoFrame::setImage(const QImage &image)
{
    //m_image = std::make_shared<QImage>(image);
	m_is_tex_update = true;
	m_is_update = true;
}

void VideoFrame::setScale(float v)
{
	if(v < 1){
		v = 1;
		m_mouseMove = QVector2D(0, 0);
	}

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
		generateTextureQ();
		update();
	}

	if(m_time_fps.elapsed() > FPS_TIMEWAIT){
		m_fps = (float)m_counter_fps/m_time_fps.elapsed() * 1000;
		m_counter_fps = 0;
		m_time_fps.restart();
	}
}

void VideoFrame::onReceiveImage(const P1Image &image)
{
	m_isYUV = true;
    m_image = image;
	m_counter_fps++;

	//m_image.save("1.bmp");

	m_is_tex_update = true;
	m_is_update = true;
}

void VideoFrame::onReceiveImage(const QImage &image)
{
	m_isYUV = false;
	m_qimage = image;
	m_counter_fps++;

	//m_image.save("1.bmp");

	m_is_tex_update_q = true;
	m_is_update = true;
}

void VideoFrame::setViewport(float w, float h)
{
	glViewport(0, 0, w, h);

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
	if(!m_is_tex_update || !m_image.get() || m_image->isNull())
		return;
	m_is_tex_update = false;

//	int t = GL_RGB;
//	if(m_image->format() == QImage::Format_ARGB32 || m_image->format() == QImage::Format_RGB32)
//		t = GL_RGBA;

    if(m_prev_width != m_image->width() || m_prev_height != m_image->height()){
		m_prev_width = m_image->width();
		m_prev_height = m_image->height();
		glBindTexture(GL_TEXTURE_2D, m_bindTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_image->width(), m_image->height(),
                     0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_image->Y.data());
		glBindTexture(GL_TEXTURE_2D, m_bindTexU);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_image->width()/2, m_image->height()/2,
					 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_image->U.data());

		glBindTexture(GL_TEXTURE_2D, m_bindTexV);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_image->width()/2, m_image->height()/2,
					 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_image->V.data());
	}else{
		glBindTexture(GL_TEXTURE_2D, m_bindTex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_image->width(), m_image->height(),
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, m_image->Y.data());
		glBindTexture(GL_TEXTURE_2D, m_bindTexU);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_image->width()/2, m_image->height()/2,
						GL_LUMINANCE, GL_UNSIGNED_BYTE, m_image->U.data());
		glBindTexture(GL_TEXTURE_2D, m_bindTexV);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_image->width()/2, m_image->height()/2,
						GL_LUMINANCE, GL_UNSIGNED_BYTE, m_image->V.data());
	}

}

void VideoFrame::generateTextureQ()
{
	if(!m_is_tex_update_q || m_qimage.isNull())
		return;
	m_is_tex_update_q = false;

    int t = GL_RGB;
    int tt = 3;
    if(m_qimage.format() == QImage::Format_ARGB32 || m_qimage.format() == QImage::Format_RGB32){
        t = GL_RGBA;
        tt = 4;
    }

	if(m_prev_width != m_qimage.width() || m_prev_height != m_qimage.height()){
		m_prev_width = m_qimage.width();
		m_prev_height = m_qimage.height();
		glBindTexture(GL_TEXTURE_2D, m_bindTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, tt, m_qimage.width(), m_qimage.height(),
                     0, t, GL_UNSIGNED_BYTE, m_qimage.bits());
	}else{
		glBindTexture(GL_TEXTURE_2D, m_bindTex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_qimage.width(), m_qimage.height(),
                        t, GL_UNSIGNED_BYTE, m_qimage.bits());
	}
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
    //glLoadIdentity();
    //glTranslatef(0, 0, -2);

	if(m_isYUV)
		m_shpr.bind();
	else
		m_shpr_rgb.bind();

	float mvp[16];

	m_model.setToIdentity();
	m_model.translate(m_mouseMove.x(), -m_mouseMove.y(), -2);

	if(m_rotateBy90)
		m_model.rotate(-90, 0, 0, 1);

	m_model.scale(m_scale, m_scale, 1);

    if((m_isYUV && (!m_image.get() || m_image->isNull())) && (!m_isYUV && m_qimage.isNull()))
		return;

    float arim = m_isYUV && m_image.get()? (float)m_image->width()/m_image->height() : (float)m_qimage.width()/m_qimage.height();
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

	glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_bindTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_bindTexU);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_bindTexV);

	if(m_isYUV){
		glUniformMatrix4fv(m_mvpInt, 1, false, mvp);

		glUniform1i(m_utexInt, 0);
		glUniform1i(m_texUInt, 1);
		glUniform1i(m_texVInt, 2);

		glEnableVertexAttribArray(m_vecInt);
		glEnableVertexAttribArray(m_texInt);
		glVertexAttribPointer(m_texInt, 2, GL_FLOAT, false, 0, m_textureBuffer.data());
		glVertexAttribPointer(m_vecInt, 3, GL_FLOAT, false, 0, m_vertexBuffer.data());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexBuffer.size() / 3);
		glDisableVertexAttribArray(m_vecInt);
		glDisableVertexAttribArray(m_texInt);
	}else{
		glUniformMatrix4fv(m_mvpRgbInt, 1, false, mvp);

		glUniform1i(m_utexRgbInt, 0);

        float rgb[3] = {
            m_Rgb[0] * m_eV,
            m_Rgb[1] * m_eV,
            m_Rgb[2] * m_eV,
        };

        glUniform3fv(m_rgbInt, 1, rgb);

		glEnableVertexAttribArray(m_vecRgbInt);
		glEnableVertexAttribArray(m_texRgbInt);
		glVertexAttribPointer(m_texRgbInt, 2, GL_FLOAT, false, 0, m_textureBuffer.data());
		glVertexAttribPointer(m_vecRgbInt, 3, GL_FLOAT, false, 0, m_vertexBuffer.data());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexBuffer.size() / 3);
		glDisableVertexAttribArray(m_vecRgbInt);
		glDisableVertexAttribArray(m_texRgbInt);

	}

	glDisable(GL_TEXTURE_2D);
}

QString loadFile(const QString &fn)
{
	QFile f(fn);
	if(f.open(QIODevice::ReadOnly)){
		QByteArray str = f.readAll();
		f.close();

		return str;
	}
	return "";
}

void VideoFrame::initializeGL()
{
	QGLWidget::initializeGL();

	QOpenGLFunctions::initializeOpenGLFunctions();

	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenTextures(1, &m_bindTex);
    glGenTextures(1, &m_bindTexU);
    glGenTextures(1, &m_bindTexV);
    //m_bindTex = 1;

	QString vertex		 = loadFile(":/gl/frame.vert");
	QString fragment	 = loadFile(":/gl/frame.frag");
	QString fragment_rgb = loadFile(":/gl/frame_rgb.frag");

	m_shpr.addShaderFromSourceCode(QGLShader::Vertex, vertex);
	m_shpr.addShaderFromSourceCode(QGLShader::Fragment, fragment);
    if(!m_shpr.link()){
        qDebug("Link error: %s", m_shpr.log().toLatin1().data());
    }
    if(!m_shpr.bind()){
        qDebug("Bind error: %s", m_shpr.log().toLatin1().data());
    }

	m_shpr_rgb.addShaderFromSourceCode(QGLShader::Vertex, vertex);
	m_shpr_rgb.addShaderFromSourceCode(QGLShader::Fragment, fragment_rgb);
	if(!m_shpr_rgb.link()){
		qDebug("Link error: %s", m_shpr.log().toLatin1().data());
	}
	if(!m_shpr_rgb.bind()){
		qDebug("Bind error: %s", m_shpr.log().toLatin1().data());
	}

	m_vecInt = m_shpr.attributeLocation("aVec");
	m_texInt = m_shpr.attributeLocation("aTex");
	m_mvpInt = m_shpr.uniformLocation("uMvp");
	m_utexInt = m_shpr.uniformLocation("uTex");
    m_texUInt = m_shpr.uniformLocation("uUTex");
    m_texVInt = m_shpr.uniformLocation("uVTex");

	m_vecRgbInt = m_shpr_rgb.attributeLocation("aVec");
	m_texRgbInt = m_shpr_rgb.attributeLocation("aTex");
	m_mvpRgbInt = m_shpr_rgb.uniformLocation("uMvp");
	m_utexRgbInt = m_shpr_rgb.uniformLocation("uTex");
    m_rgbInt = m_shpr_rgb.uniformLocation("rgb");

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

	setViewport(w, h);
}

void VideoFrame::paintGL()
{
	QGLWidget::paintGL();
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

void VideoFrame::paintEvent(QPaintEvent *event)
{
	makeCurrent();
	drawImage();
	doneCurrent();

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	painter.setPen(Qt::white);

	painter.drawText(10, 30, "fps " + QString::number(m_fps, 'f', 0));

	if(m_image.get())
		painter.drawText(10, height() - 20, QString::number(m_image->width()) + "x" + QString::number(m_image->height()));

	painter.end();

//	QGLWidget::paintEvent(event);
}


void VideoFrame::mousePressEvent(QMouseEvent *event)
{
	m_mouseDown = true;
	m_mousePos = QVector2D(event->pos().x(), event->pos().y());
}

void VideoFrame::mouseReleaseEvent(QMouseEvent *event)
{
	m_mouseDown = false;
	m_mousePos = QVector2D(event->pos().x(), event->pos().y());
}

void VideoFrame::mouseMoveEvent(QMouseEvent *event)
{
	if(m_mouseDown){
		QVector2D mousePos = QVector2D(event->pos().x(), event->pos().y());
		m_mouseMove += (mousePos - m_mousePos)/height() * 2;
		m_mousePos = mousePos;
		m_is_update = true;
	}
}
