#ifndef DIALOGTCPHOST_H
#define DIALOGTCPHOST_H

#include <QDialog>

namespace Ui {
class DialogTcpHost;
}

class DialogTcpHost : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTcpHost(QWidget *parent = nullptr);
	~DialogTcpHost();

    void setHost(const QString &host, ushort port);

    QString host() const;
    ushort port() const;

public slots:
    void on_buttonBox_accepted();

private:
	Ui::DialogTcpHost *ui;

    QString m_host;
    ushort m_port;
};

#endif // DIALOGTCPHOST_H
