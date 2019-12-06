#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "websock.h"
#include "testsender.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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

private:
	Ui::MainWindow *ui;

	std::unique_ptr<WebSock> m_websock;
    std::unique_ptr<TestSender> m_testSender;
};
#endif // MAINWINDOW_H
