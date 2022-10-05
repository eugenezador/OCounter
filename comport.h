#ifndef COMPORT_H
#define COMPORT_H

#include <QObject>

class ComPort : public QObject
{
    Q_OBJECT
public:
    explicit ComPort(QObject *parent = nullptr);

signals:

};

#endif // COMPORT_H
