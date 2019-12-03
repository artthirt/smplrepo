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

	connect(m_websock.get(), SIGNAL(sendImage(QImage)), ui->widgetFrame, SLOT(onReceiveImage(QImage)), Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::on_actionRotate_by_90_triggered()
{
}

void MainWindow::on_actionRotate_by_90_triggered(bool checked)
{
	ui->widgetFrame->setRotateBy90(checked);
}
