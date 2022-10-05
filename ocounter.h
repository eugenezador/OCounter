#ifndef OCOUNTER_H
#define OCOUNTER_H

#include <QMainWindow>
//#include <sys/mman.h>
#include "qcustomplot.h"
#include "comport.h"
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

    void parse_received_data(const QByteArray &data);

    void update_data(QByteArray &read_data);

    // Методы графика
    void plot_settings();

    void real_plot();


signals:
    void sent_data_to_com_port(const QByteArray &data);
    void recive_data_from_com_port(QByteArray &data);
    void open_serial_port(const QString &text);
    void close_serial_port();

private slots:
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
    bool is_connect = false; // для отображения кнопик соединения с портом
    bool lazer_on = false;
/////////////////////
    QByteArray data = "14"; // при первом запуске кнопки connect контейнер data не обновляется
/////////////////////
    ComPort *com_port;

//    контейнер для хранения данных о целях
//    (время обнаружения - массив расстояний до обнаруженных объектов)
    std::map<double, QVector<double>> graph_value;

    QVector<double> result; // массив для записи команды распознавания в распределяемую память

    QVector<double> q_x; // массив для записи коодинат

    QVector<double> values; // вектор(массив) расстояний до обнаруженных объектов за 1 выстрел
};
#endif // OCOUNTER_H
