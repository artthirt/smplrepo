#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTimer>

#include "websock.h"
#include "tcpsocket.h"
#include "testsender.h"

#include "camerastream.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QProgressBar;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_actionRotate_by_90_triggered();

	void on_actionRotate_by_90_triggered(bool checked);

    void on_actionStart_test_triggered(bool checked);

	void onTimeout();

	void on_actionSet_Record_FileName_triggered();

	void on_actionStart_record_triggered(bool checked);

	void on_actionSet_Host_for_Web_Camera_triggered();

	void on_actionOpen_Local_WebCamera_triggered();

	void on_actionClose_Local_Web_Camera_triggered();

    void on_hs_red_valueChanged(int value);

    void on_hs_green_valueChanged(int value);

    void on_hs_blue_valueChanged(int value);

    void on_hs_eV_valueChanged(int value);

private:
	Ui::MainWindow *ui;
	QProgressBar *m_progressBar;
	QTimer m_timer;

	std::unique_ptr<WebSock> m_websock;
	std::unique_ptr<tcpsocket> m_tcpsocket;
    std::unique_ptr<TestSender> m_testSender;
	std::unique_ptr<CameraStream> m_camerastream;
};
#endif // MAINWINDOW_H
