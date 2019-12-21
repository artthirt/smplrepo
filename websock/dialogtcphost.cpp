#include "dialogtcphost.h"
#include "ui_dialogtcphost.h"

DialogTcpHost::DialogTcpHost(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogTcpHost)
{
	ui->setupUi(this);
}

DialogTcpHost::~DialogTcpHost()
{
	delete ui;
}

void DialogTcpHost::setHost(const QString &host, ushort port)
{
	m_host = host;
	m_port = port;

	ui->leHost->setText(QString("%1:%2").arg(m_host).arg(m_port));
}

QString DialogTcpHost::host() const
{
	return m_host;
}

ushort DialogTcpHost::port() const
{
	return m_port;
}

void DialogTcpHost::on_buttonBox_accepted()
{
	QString s = ui->leHost->text();
	QStringList sl = s.split(":");
	if(sl.size() > 1){
		m_host = sl[0];
		m_port = sl[1].toUInt();
	}
	close();
}
