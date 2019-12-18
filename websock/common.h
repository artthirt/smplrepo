#ifndef COMMON_H
#define COMMON_H

#include <QEvent>
#include <QByteArray>

#include <QImage>

#include <memory>
#include <vector>

class YUVImage{
public:
    YUVImage(){}
    YUVImage(int width, int height, uint8_t *data[8], int linesize[8]){
        set(width, height, data, linesize);
    }

    void set(int width, int height, uint8_t *data[8], int linesize[8]){
        m_width = width;
        m_height = height;

        Y.resize(linesize[0] * height);
        U.resize(linesize[1] * height/2);
        V.resize(linesize[2] * height/2);

        std::copy(data[0], data[0] + Y.size(), Y.data());
        std::copy(data[1], data[1] + U.size(), U.data());
        std::copy(data[2], data[2] + V.size(), V.data());
    }

    int width() const { return m_width; }
    int height() const {return m_height; }
    bool isNull() const { return Y.empty() || U.empty() || V.empty(); }

    void save(const QString& fn){}

    std::vector<uchar> Y;
    std::vector<uchar> U;
    std::vector<uchar> V;
private:
    int m_width     = 0;
    int m_height    = 0;
};

typedef std::shared_ptr<YUVImage> P1Image;
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
