#ifndef COMMON_H
#define COMMON_H

#include <QEvent>
#include <QByteArray>

#include <QImage>

#include <memory>
#include <vector>

typedef std::shared_ptr<QImage> PImage;

struct Image{
	int width   = 0;
	int height  = 0;
	int linesize[8];
	std::vector< uint8_t > data[8];

	bool empty() const{
		return width == 0 || height == 0 || data[0].empty() || data[1].empty() || data[2].empty();
	}
};

class EventTest: public QEvent{
    Q_GADGET
public:
    enum{EVENT = QEvent::User + 100};

    QByteArray data;

    EventTest(const QByteArray& data): QEvent((Type)EVENT){
        this->data = data;
    }


};

#endif // COMMON_H
