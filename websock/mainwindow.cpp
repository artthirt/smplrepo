#include "mainwindow.h"
#include "ui_mainwindow.h"

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

	connect(m_websock.get(), SIGNAL(sendImage(QImage)), ui->widgetFrame, SLOT(onReceiveImage(QImage)), Qt::QueuedConnection);
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
        m_testSender->startPlay();
    }else{
        m_testSender->stopPlay();
    }
}
