#include "objectcounter.h"

ObjectCounter::ObjectCounter(QObject *parent) : QObject(parent),
    serial(new QSerialPort(this))
{
    connect(serial, &QSerialPort::readyRead, this, &ObjectCounter::readData);
}

ObjectCounter::~ObjectCounter()
{
    serial->close();
    delete serial;
}

void ObjectCounter::open_serial_port(const QString &text)
{
    serial->setPortName(text);

    serial->open(QIODevice::ReadWrite);
    serial->setBaudRate(QSerialPort::Baud38400);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
}

void ObjectCounter::close_serial_port()
{
    if (serial->isOpen())
        serial->close();

}

void ObjectCounter::writeData(const QByteArray &data)
{
    if(serial->isOpen() == true){
        serial->write(data);
        serial->write("\r");
        serial->waitForBytesWritten();

        qDebug() << "write: " << data;
    }
    else
    {
        qDebug() << "not open";
    }
}

void ObjectCounter::readData()
{
//    QByteArray read_data;
    QByteArray read_data = serial->readAll();
//    if(serial->isReadable())
//    {
//      //   результат чтения накапливается в переменную data
//        read_data.append(serial->readAll());
//        if(strcmp(read_data.end() - 1 , "\\") == 1 && strcmp(read_data.end(), "r")) break;
//    }
    qDebug() << "read: " << read_data;

    emit received_data(read_data);
}
