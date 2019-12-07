#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProgressBar>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_websock.reset(new WebSock);
	m_websock->moveToThread(m_websock.get());
	m_websock->start();

    m_testSender.reset(new TestSender());
    m_testSender->startThread();

    m_testSender->setFilename("test.bin");
    m_testSender->setSender(m_websock.get());

	m_progressBar = new QProgressBar(this);
	m_progressBar->setRange(0, 100);
	m_progressBar->setValue(0);
	ui->statusbar->addWidget(m_progressBar);
	m_progressBar->hide();

	connect(m_websock.get(), SIGNAL(sendImage(QImage)), ui->widgetFrame, SLOT(onReceiveImage(QImage)), Qt::QueuedConnection);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	m_timer.start(500);
}

MainWindow::~MainWindow()
{
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
}
