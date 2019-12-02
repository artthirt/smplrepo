#include "videodata.h"

#include <QDataStream>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>

//////////////////////////////////////////////////////////////////////////////////

//значение ASFDataHeader::ASF_ID
#define PRE_ASF_ID	0x82

//заголовок пакета передачи
struct ASFDataHeader{
	ASFDataHeader(){
		ASF_ID = PRE_ASF_ID;
		Reserved = 0;
		Length_type_flags = 0x01;
		Property_flags = 0x7D;
		Send_time = QDateTime::currentDateTime().toTime_t();
		Duration = 0;
		Segment_properties = 0;
	}

	quint8 ASF_ID;
	quint16 Reserved;
	quint8 Length_type_flags;
	quint8 Property_flags;
	quint32 Send_time;
	quint16 Duration;
	quint8 Segment_properties;

	uchar paddingLengthType(){
		return (Length_type_flags >> 3) & 0x3;
	}
};

#define PRE_STREAM_ID	0x81

//заголовок пакета данных
struct ASFPayloadHeader{
	ASFPayloadHeader(){
		Stream_ID = PRE_STREAM_ID;
		Frame_number = 0;
		Fragment_offset = 0;
		Replicated_data_length = 0;
		Frame_size = 0;
		Send_time = QDateTime::currentDateTime().toTime_t();
		Payload_length = 0;
	}

	quint8 Stream_ID;// = 0x81
	quint32 Frame_number;
	quint32 Fragment_offset;
	quint8 Replicated_data_length;
	quint32 Frame_size;
	quint32 Send_time;
	quint16 Payload_length;
};


/////////////////////////////////////////////////////////////////////////////////
/// \brief VideoData::VideoData
///

VideoData::VideoData()
{
	m_id = 0;
	m_creating = false;
	m_FrameSize = 0;
	m_prevID = -1;
	m_frame_number = 0;
	m_lost_packet = 0;
	m_receive_frames = 0;
}

VideoData::~VideoData()
{

}

/**
 * @brief VideoData::addData
 * @param data
 * @param param
 * @return
 * packet:
 * uint id - rand identification
 * uint max_cnt - count packets
 * uint index - current packets
 * char* data - image data
 */
bool VideoData::addData(const QByteArray &data, bool param, bool *first_packet)
{
	Q_UNUSED(param);
	Q_UNUSED(first_packet);

	ASFDataHeader dheader;
	ASFPayloadHeader pheader;

	QDataStream stream(data);
	stream.setByteOrder(QDataStream::LittleEndian);

	while(!stream.atEnd()){

		stream >> dheader.ASF_ID;
		if(dheader.ASF_ID != PRE_ASF_ID){
			m_creating = false;
			m_lost_packet++;
			return false;
		}
		m_current_time_snapshot = QDateTime::currentMSecsSinceEpoch();

		stream >> dheader.Reserved;
		stream >> dheader.Length_type_flags;
		stream >> dheader.Property_flags;
		stream >> dheader.Send_time;
		stream >> dheader.Duration;
		stream >> dheader.Segment_properties;

		if(dheader.Length_type_flags != 0x01 && dheader.Property_flags != 0x7D){
			m_creating = false;
			m_lost_packet++;
			return false;
		}

		stream >> pheader.Stream_ID;
		if(pheader.Stream_ID != PRE_STREAM_ID){
			m_creating = false;
			m_lost_packet++;
			return false;
		}

		stream >> pheader.Frame_number;
		stream >> pheader.Fragment_offset;
		stream >> pheader.Replicated_data_length;

		stream >> pheader.Frame_size;
		stream >> pheader.Send_time;
		stream >> pheader.Payload_length;

		if(m_currentID != pheader.Frame_number && pheader.Fragment_offset){
			m_creating = false;
			m_lost_packet++;
			return false;
		}

		if(!pheader.Fragment_offset){
			m_prevID = m_currentID;
			m_packet.clear();
			m_FrameSize = pheader.Frame_size;
			m_currentFrameSize = 0;
			m_creating = true;
		}
		m_currentID = pheader.Frame_number;

		if(m_FrameSize != pheader.Frame_size){
			m_creating = false;
			m_lost_packet++;
			return false;
		}

		QByteArray ba(pheader.Payload_length, Qt::Uninitialized);
		int size = stream.readRawData(ba.data(), pheader.Payload_length);
		size = qMin<int>(m_FrameSize - m_currentFrameSize, size);
		ba.resize(size);
		if(m_packet.size() != pheader.Fragment_offset){
			qDebug() << "packet lost";
			m_currentID = -1;
			m_creating = false;
			m_lost_packet++;
			return false;
		}
		m_packet.append(ba);
		m_currentFrameSize += size;

		if(m_currentFrameSize >= m_FrameSize){
			Packet pack;
			pack.data = m_packet;
			pack.source = ASF_SOURCE;
			pack.subframe = 0;
			pack.number = m_receive_frames++;
			m_frames.push_back(pack);

			//qDebug("frame number %d", pheader.Frame_number);

			emit s_add_packet(1, m_packet);

			m_packet.clear();
			m_creating = false;
// 			QFile f("f.jp2");
// 			if(f.open(QIODevice::WriteOnly)){
// 				f.write(m_packet);
// 				f.close();
// 			}
		}
	}
	return true;

/** \comment custom protocols
	uint id, max_cnt, index;

	stream >> id >> max_cnt >> index;

	if(!max_cnt || index >= max_cnt){
		m_creating = false;
		return false;
	}

	if(m_id != id){
		m_id = id;
		if(index){
			m_creating = false;
			return false;
		}
		m_packet.clear();
		m_creating = true;
	}

	int size = stream.device()->size() - stream.device()->pos();

	QByteArray d;
	d.resize(size);
	stream.readRawData(d.data(), d.size());

	m_packet += d;

	if(m_creating && index + 1 >= max_cnt && m_packet.size()){
		m_frames.push_back(m_packet);
		m_packet.clear();
		m_creating = false;
	}
	return true;
	*/

}

int VideoData::frameCount()
{
	return m_frames.size();
}

Packet empty_array;

const Packet &VideoData::getFrame()
{
	if(!m_frames.size())
		return empty_array;
	return m_frames.front();
}

Packet &VideoData::getFullPacket()
{
	if(!m_frames.size())
		return empty_array;
	return m_frames.front();
}

bool VideoData::removeFrame()
{
	if(!m_frames.size())
		return true;

	m_frames.pop_front();

	return true;
}

int VideoData::n_subframe()
{
	return 0;
}

uchar VideoData::source()
{
	return ASF_SOURCE;
}

uchar VideoData::format()
{
	return 0;
}

void VideoData::get_SEV(QByteArray &ba)
{
	Q_UNUSED(ba);
}

void VideoData::get_CN(QByteArray &ba)
{
	Q_UNUSED(ba);
}

unsigned int VideoData::number()
{
	return 0;
}

qint64 VideoData::lost_packets()
{
	return m_lost_packet;
}

bool VideoData::parse(const QByteArray &data)
{
	Packet pack;
	pack.data = data;
	pack.source = ASF_SOURCE;
	pack.subframe = 0;
	m_frames.push_back(pack);
	if(m_frames.size())
		return true;
	return false;
}

#define MAXSIZEPKT	60000

QVector<QByteArray> VideoData::get_udp_packets(const QByteArray &data)
{
	QVector<QByteArray> output;

	if(!data.size())
		return output;

	uint max_pkt = qRound((double)data.size() / MAXSIZEPKT + 0.5);
	int size = 0, current = 0;
	if(!max_pkt)max_pkt = 1;

	for(uint i = 0; i < max_pkt; i++){
		ASFDataHeader dheader;
		ASFPayloadHeader pheader;

		QByteArray send;
		QDataStream stream(&send, QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);

		stream << dheader.ASF_ID;
		stream << dheader.Reserved;
		stream << dheader.Length_type_flags;
		stream << dheader.Property_flags;
		stream << dheader.Send_time;
		stream << dheader.Duration;
		stream << dheader.Segment_properties;
		///
		pheader.Frame_number = m_frame_number;
		size = qMin<int>(MAXSIZEPKT - stream.device()->pos() - sizeof(pheader), data.size() - current);
		if(size < 0){
			throw new QString("value less zero");
		}
		pheader.Fragment_offset = current;
		pheader.Frame_size = data.size();
		pheader.Payload_length = size;

		stream << pheader.Stream_ID;
		stream << pheader.Frame_number;
		stream << pheader.Fragment_offset;
		stream << pheader.Replicated_data_length;
		stream << pheader.Frame_size;
		stream << pheader.Send_time;
		stream << pheader.Payload_length;

		stream.writeRawData(&data.data()[current], size);
		output << send;

		current += size;
	}

	m_frame_number++;

	return output;
}

