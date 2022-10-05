#ifndef OBJECTCOUNTER_H
#define OBJECTCOUNTER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>


class ObjectCounter : public QObject
{
    Q_OBJECT
public:
    explicit ObjectCounter(QObject *parent = nullptr);
    ~ObjectCounter();

public:
    //QByteArray data;

    QSerialPort *serial = nullptr;// указатель на область памяти для экземпляра порта
    QString currentPortName;// для записи предыдущего значения порта

    void open_serial_port(const QString &text);
    void close_serial_port();

signals:
    void received_data(QByteArray &data);

public slots:

    void writeData(const QByteArray &data);

    void readData();

};

#endif // OBJECTCOUNTER_H
