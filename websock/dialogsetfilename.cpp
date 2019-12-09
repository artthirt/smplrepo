#include "dialogsetfilename.h"
#include "ui_dialogsetfilename.h"

DialogSetFileName::DialogSetFileName(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSetFileName)
{
	ui->setupUi(this);
}

DialogSetFileName::~DialogSetFileName()
{
	delete ui;
}

void DialogSetFileName::setFileName(const QString &fn)
{
	m_fileName = fn;
	ui->le_FileName->setText(fn);
}

QString DialogSetFileName::fileName() const
{
	return m_fileName;
}

void DialogSetFileName::on_buttonBox_accepted()
{
	m_fileName = ui->le_FileName->text();
}
