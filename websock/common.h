#ifndef COMMON_H
#define COMMON_H

#include <QEvent>
#include <QByteArray>

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
