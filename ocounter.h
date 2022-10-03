#ifndef OCOUNTER_H
#define OCOUNTER_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <sys/mman.h>
#include <qcustomplot.h>
//#include "objectcounter.h"
#include "infowindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Ocounter; }
QT_END_NAMESPACE

class Ocounter : public QMainWindow
{
    Q_OBJECT

public:
    Ocounter(QWidget *parent = nullptr);
    ~Ocounter();

    void serial_port_properties(const QString &text);

    void parse(const QByteArray &data, std::map<double, QVector<double>> &graph_value);

    void aim_parser(const QByteArray &data, QVector<QVector<double>> &lazer_value);

    // Методы графика
    void plot_settings();

    void real_plot();

    void plot();

    //shared memory
    void create_shared_memory();

    void read_shared_memory();

    void detach_shared_memory();


private slots:

 //Чтение и запись
    void writeData(const QByteArray &data);

    void readData();

    void on_lon_clicked();

    void on_lof_clicked();

    void on_ver_clicked();

    void on_vlt_clicked();

    void on_css_clicked();

    void on_tm1_clicked();

    void on_rst_clicked();

    void on_syn1_clicked();

    void on_syn2_clicked();

    void on_srr_clicked();

    void on_nim_clicked();

    void on_connected_clicked();

    void on_nim1_clicked();

    void on_info_clicked();

private:
    Ui::Ocounter *ui;
    InfoWindow *window;

    void  keyPressEvent(QKeyEvent *event);

    bool key_pressed = false;
    bool is_connect = false;
    bool lazer_on = false;

    QByteArray data;

    QSerialPort *serial;// указатель на область памяти для экземпляра порта
    QString currentPortName;// для записи предыдущего значения порта

    std::map<double, QVector<double>> graph_value;

    QVector<QVector<double>> lazer_value;

    QVector<double> q_x, q_y;

    QVector<double> values;

    //shared memory
    QSharedMemory share_memory;

};
#endif // OCOUNTER_H
