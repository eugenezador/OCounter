#ifndef OBJECTCOUNTER_H
#define OBJECTCOUNTER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTime>


class ObjectCounter : public QObject
{
    Q_OBJECT
public:
    explicit ObjectCounter(QObject *parent = nullptr);

protected:
    QByteArray data;

    QSerialPort *serial;// указатель на область памяти для экземпляра порта
    QString currentPortName;// для записи предыдущего значения порта

    std::map<double, QVector<double>> graph_value;

    QVector<double> values;

    void serial_port_properties(const QString &text);

    void parse(const QByteArray &data, std::map<double, QVector<double>> &graph_value);

    void writeData(const QByteArray &data);

    void readData();

};

#endif // OBJECTCOUNTER_H
