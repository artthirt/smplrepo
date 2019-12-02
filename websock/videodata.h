#ifndef VIDEODATA_H
#define VIDEODATA_H

#include <QObject>
#include <QVector>
#include <QByteArray>

//#include "bsidata.h"

/// \brief структура для фрейма с заголовком субкадра
struct Packet{
	Packet(){
		subframe = number = flag = format = source = 0;
	}
	Packet(const Packet& packet)
	{
		this->data = packet.data;
		this->subframe = packet.subframe;
		this->_sev = packet._sev;
		this->_cn = packet._cn;
		this->format = packet.format;
		this->flag = packet.flag;
		this->number = packet.number;
		this->source = packet.source;
	}
	/**
		@param data - image frame
		@param subframe - config frame
		@param sev - SEV byte array
		@oaram cn - CN byte array
	*/
	Packet(const QByteArray& data, int subframe, QByteArray& sev, QByteArray& cn){
		this->data = data;
		this->subframe = subframe;
		this->_sev = sev;
		this->_cn = cn;
	}
	/**
	 * @brief clear
	 * очистка данных
	 */
	void clear(){
		data.clear();
		_sev.clear();
		_cn.clear();
		subframe = number = flag = format = source = 0;
	}

	QByteArray data;
	/// \brief СЕВ
	QByteArray _sev;
	/// \brief ЦН
	QByteArray _cn;
	int subframe;
	qint64 number;
	int flag;
	int format;
	int source;
};

///////////////////////////////////////////////////
/// \brief The VideoData class
/// класс для обработки данных по протоколу ASF
class VideoData: public QObject
{
	Q_OBJECT
public:
	enum {ASF_SOURCE = 0xF0};

	VideoData();
	~VideoData();

	/// \brief Добавить данные в парсер.
	virtual bool addData(const QByteArray& data, bool param, bool *first_packet = 0);
	/// \brief Получить количество найденных изображений.
	virtual int frameCount();
	/// \brief Получить данные изображения с номером index.
	virtual const Packet& getFrame();
	/// \brief получить структуру с кадром и заголовком
	Packet& getFullPacket();
	/// \brief Удалить изображение с номером index.
	virtual bool removeFrame();
	/// \brief номер текущего субкадра
	virtual int n_subframe();
	/// \brief источник данных
	virtual uchar source();
	/// \brief Формат видеокадра
	virtual uchar format();
	/// \brief массив с данными СЕВ
	virtual void get_SEV(QByteArray& ba);
	/// \brief массив с данными ЦН
	virtual void get_CN(QByteArray& ba);
	/// \brief номер кадра
	virtual unsigned int number();
	/// \brief numbers of lost packets
	virtual qint64 lost_packets();
	/// \brief
	virtual bool parse(const QByteArray &data);

	/**
	 * @brief get_udp_packets
	 * разбить входные данные на пакеты для отправки
	 * @param data
	 * @return
	 */
	QVector< QByteArray > get_udp_packets(const QByteArray& data);

signals:
	void s_add_packet(uchar source, const QByteArray);

private:
	QVector< Packet > m_frames;
	QByteArray m_packet;
	uint m_id;
	bool m_creating;
	qint64 m_current_time_snapshot;
	qint64 m_lost_packet;
	qint64 m_receive_frames;

	/// \brief Номер принимаемого в данный момент изображения.
	int m_currentID;
	/// \brief Номер предыдущего принимаемого в данный момент изображения.
	int m_prevID;
	/// \brief размер фрейма
	quint32 m_FrameSize;
	/// \brief теущий размер буяера
	quint32 m_currentFrameSize;

	uint m_frame_number;
};

#endif // VIDEODATA_H
