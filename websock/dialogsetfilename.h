#ifndef DIALOGSETFILENAME_H
#define DIALOGSETFILENAME_H

#include <QDialog>

namespace Ui {
class DialogSetFileName;
}

class DialogSetFileName : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSetFileName(QWidget *parent = nullptr);
	~DialogSetFileName();

	void setFileName(const QString& fn);
	QString fileName() const;

private slots:
	void on_buttonBox_accepted();

private:
	Ui::DialogSetFileName *ui;

	QString m_fileName;
};

#endif // DIALOGSETFILENAME_H
