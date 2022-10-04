#ifndef OCOUNTER_H
#define OCOUNTER_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSharedMemory>
//#include <sys/mman.h>
#include "qcustomplot.h"
#include "objectcounter.h"
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

    // Методы графика
    void plot_settings();

    void real_plot();

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

    double device_start = 0;

    double min_L = 99999; // минимальное расстоние до цели (для масштабирования графика)
    double max_L = 0; // максимальное расстояние до цели (для масштабирования графика)

    bool key_pressed = false;
    bool is_connect = false;
    bool lazer_on = false;

    QByteArray data;

    ObjectCounter *com;

    QSerialPort *serial;// указатель на область памяти для экземпляра порта
    QString currentPortName;// для записи предыдущего значения порта

//    контейнер для хранения данных о целях
//    (время обнаружения - массив расстояний до обнаруженных объектов)
    std::map<double, QVector<double>> graph_value;

    QVector<double> result; // массив для записи команды распознавания в распределяемую память

    QVector<double> q_x, q_y; // массив для записи коодинат

    QVector<double> values;

    //shared memory
    QSharedMemory shared_memory;

    double sh = 144;

};
#endif // OCOUNTER_H
