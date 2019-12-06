#include "testsender.h"

#include <QCoreApplication>

#include "common.h"

#define ARRAY_SIZE 1024 * 1024

TestSender::TestSender(QObject *parent) : QThread(parent)
{
    m_sender = nullptr;
    m_timeout = 500;
}

TestSender::~TestSender()
{
    quit();
    wait();
}

void TestSender::setFilename(const QString &name)
{
    m_fileName = name;

}

QString TestSender::fileName() const
{
    return m_fileName;
}

void TestSender::setSender(QObject *sender)
{
    m_sender = sender;
}

void TestSender::startThread()
{
    moveToThread(this);
    start();
}

void TestSender::startPlay()
{
    if(m_fileName.isEmpty() || !QFile::exists(m_fileName))
        return;


    m_mutex.lock();
    if(m_file.isOpen())
        m_file.close();
    m_file.setFileName(m_fileName);
    m_file.open(QIODevice::ReadOnly);
    m_file.seek(0);
    m_mutex.unlock();
}

void TestSender::stopPlay()
{
    m_mutex.lock();
    if(m_file.isOpen())
        m_file.close();
    m_mutex.unlock();
}

void TestSender::onTimeout()
{
    if(m_fileName.isEmpty() || !m_file.isOpen() || !m_sender){
        return;
    }

    m_mutex.lock();
    QByteArray data = m_file.read(ARRAY_SIZE);
    if(m_file.atEnd())
        m_file.close();
    m_mutex.unlock();

    if(!data.isEmpty()){
        QCoreApplication::postEvent(m_sender, new EventTest(data));
    }else{
        m_mutex.lock();
        m_file.close();
        m_mutex.unlock();
    }
}

void TestSender::run()
{
    m_timer.reset(new QTimer);
    m_timer->moveToThread(this);
    connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->start(m_timeout);

    exec();

    m_timer.reset();
}
