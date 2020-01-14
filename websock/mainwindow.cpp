#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProgressBar>
#include <QSlider>

#include "dialogsetfilename.h"

#include "dialogtcphost.h"

//////////////////////////////////

double getValue(double val, double min, double max)
{
    return (val - min)/(max - min);
}

double getValue(QSlider *slider)
{
    return getValue(slider->value(), slider->minimum(), slider->maximum());
}

//////////////////////////////////
/// \brief MainWindow::MainWindow
/// \param parent
///

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_websock.reset(new WebSock);
	m_websock->moveToThread(m_websock.get());
	m_websock->start();

	m_tcpsocket.reset(new tcpsocket);
	m_tcpsocket->setOwner(m_websock.get());
	m_tcpsocket->connectToHost(QHostAddress("127.0.0.1"), 1443);

    m_testSender.reset(new TestSender());
	m_testSender->startThread();

    m_testSender->setFilename("test.bin");
    m_testSender->setSender(m_websock.get());

	m_websock->setFileName(m_testSender->fileName());

	m_progressBar = new QProgressBar(this);
	m_progressBar->setRange(0, 100);
	m_progressBar->setValue(0);
	ui->statusbar->addWidget(m_progressBar);
	m_progressBar->hide();

    connect(m_websock.get(), SIGNAL(sendImage(P1Image)), ui->widgetFrame, SLOT(onReceiveImage(P1Image)), Qt::QueuedConnection);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	m_timer.start(500);

//	ConvertImage clImage;
//	Image im;
//	loadImage("test.image", &im);
//	QImage im2 = clImage.createImage(&im);
//	ui->widgetFrame->onReceiveImage(im2);
}

MainWindow::~MainWindow()
{
	m_tcpsocket.reset();
	m_testSender->setSender(nullptr);

	delete ui;
}


void MainWindow::on_actionRotate_by_90_triggered()
{
}

void MainWindow::on_actionRotate_by_90_triggered(bool checked)
{
	ui->widgetFrame->setRotateBy90(checked);
}

void MainWindow::on_actionStart_test_triggered(bool checked)
{
    if(checked){
		if(m_testSender->startPlay()){
			m_progressBar->show();
		}
    }else{
        m_testSender->stopPlay();
	}
}

void MainWindow::onTimeout()
{
	if(!m_testSender->isOpenFile() && ui->actionStart_test->isChecked()){
		ui->actionStart_test->setChecked(false);
	}
	if(m_testSender->isOpenFile()){
		m_progressBar->show();
		m_progressBar->setValue(m_testSender->progress() * 100.f);
	}else{
		m_progressBar->hide();
	}

	if(m_tcpsocket.get()){
		QString str = QString("Connecting: %1; host %2:%3")
				.arg(m_tcpsocket->isConnecting())
				.arg(m_tcpsocket->connectingHost().toString())
				.arg(m_tcpsocket->connectingPort());
		ui->statusbar->showMessage(str);
	}
}

void MainWindow::on_actionSet_Record_FileName_triggered()
{
	DialogSetFileName dlg;
	dlg.setFileName(m_testSender->fileName());

	if(dlg.exec()){
		m_testSender->setFilename(dlg.fileName());
		m_websock->setFileName(dlg.fileName());
	}
}

void MainWindow::on_actionStart_record_triggered(bool checked)
{
	m_websock->setRecord(checked);
}

void MainWindow::on_actionSet_Host_for_Web_Camera_triggered()
{
	DialogTcpHost dlg;

	QHostAddress host = m_tcpsocket->connectingHost();
	ushort port = m_tcpsocket->connectingPort();

	dlg.setHost(host.toString(), port);

	if(dlg.exec()){
		m_tcpsocket->newHost(QHostAddress(dlg.host()), dlg.port());
	}
}

void MainWindow::on_actionOpen_Local_WebCamera_triggered()
{
	if(!m_camerastream.get()){
		m_camerastream.reset(new CameraStream);
		m_camerastream->setInitAV(false);

		m_camerastream->moveToThread(m_camerastream.get());
		m_camerastream->start();

		connect(m_camerastream.get(), SIGNAL(sendImage(QImage)),
				ui->widgetFrame, SLOT(onReceiveImage(QImage)), Qt::QueuedConnection);

	}
	m_camerastream->startPlay();
}

void MainWindow::on_actionClose_Local_Web_Camera_triggered()
{
	m_camerastream->stopPlay();
}

void MainWindow::on_hs_red_valueChanged(int value)
{
    double val = 2 * getValue(ui->hs_red);
    ui->lb_red->setText(QString::number(val, 'f', 1));
    ui->widgetFrame->setRedBrightness(val);
}

void MainWindow::on_hs_green_valueChanged(int value)
{
    double val = 2 * getValue(ui->hs_green);
    ui->lb_green->setText(QString::number(val, 'f', 1));
    ui->widgetFrame->setGreenBrightness(val);
}

void MainWindow::on_hs_blue_valueChanged(int value)
{
    double val = 2 * getValue(ui->hs_blue);
    ui->lb_blue->setText(QString::number(val, 'f', 1));
    ui->widgetFrame->setBlueBrightness(val);
}

void MainWindow::on_hs_eV_valueChanged(int value)
{
    double val =  10 * (2 * getValue(ui->hs_eV) - 1);
    val = pow(2, val);
    ui->lb_eV->setText(QString::number(val, 'f', 1));
    ui->widgetFrame->setEV(val);
}
