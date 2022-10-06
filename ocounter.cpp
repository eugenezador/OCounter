
#include <QFileDialog>
#include "ocounter.h"
#include "ui_ocounter.h"

Ocounter::Ocounter(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Ocounter)
    , window(new InfoWindow(this))
    , com_port(new ComPort(this))
    , shared_memory(new MShare(this))
//    , shared_memory("QSharedMemoryExample")
{
    ui->setupUi(this);

//чтение доступных портов при запуске
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        ui->portName->addItem(serialPortInfo.portName());
    }

    connect(this, &Ocounter::open_serial_port, com_port, &ComPort::open_serial_port);
    connect(this, &Ocounter::sent_data_to_com_port, com_port, &ComPort::writeData);
    connect(this, &Ocounter::close_serial_port, com_port, &ComPort::close_serial_port);
    connect(com_port, &ComPort::received_data, this, &Ocounter::parse_received_data);
    connect(com_port, &ComPort::received_data, this, &Ocounter::update_data);

    connect(window, &InfoWindow::info_enable, this, &Ocounter::info_bottom_enable);

    connect(this, &Ocounter::write_to_shared_memory, shared_memory, &MShare::write_to_shared_memory);
    connect(this, &Ocounter::read_from_shared_memory, shared_memory, &MShare::read_from_shared_memory);
//    connect(shared_memory, &MShare::read_data_from_shared_memory, this, &Ocounter::update_shared_memory_data);

    device_start = QDateTime::currentDateTimeUtc().toTime_t();
    plot_settings();
}

Ocounter::~Ocounter()
{
    delete shared_memory;
    delete com_port;
    delete window;
    delete ui;
}


void Ocounter::parse_received_data(const QByteArray &data)
{
    result.clear();
    QVector<double> values;
    double time = 0;
    QString tmp;
    int k = 0;
    int flag = 0;

    if(strstr(data.constData(),"none")) {
        ui->read_log->append("No targets detected");
    }

    else if(strstr(data.constData(),"time")) {

    for(int i = 0; i < data.size() ; i++) {

        if(data[i]  == ',' && i == data.size() - 1) break;

        if(data[i] == 'e') {
            flag = 1;
            i++;
        }

        if (data[i] == ' ' && tmp.size() != 0) {
            flag = 0;
            time = device_start + tmp.toDouble();
            result.push_back(tmp.toDouble());
            k =0;
            for(int j = 0; tmp[j] != '\0'; j++) {
                tmp[j] = 0;
            }
        }

        if((data[i] == ':' || data[i] == ',') && tmp.size() != 0) {
            flag = 1;
            i++;
        }

        if (data[i] == '(' && tmp.size() != 0) {
            flag = 0;
            values.append(tmp.toDouble());
            result.push_back(tmp.toDouble());
            k =0;
            for(int j = 0; j < tmp.size(); j++) {
                tmp[j] = 0;
            }
        }

        if(flag) {
            tmp[k] = data[i];
            k++;
        }
    }

    graph_value[time] = values;

    real_plot();

//    emit write_to_shared_memory(result);

    qDebug() << "parse in: " << graph_value;
    }
}

void Ocounter::info_bottom_enable()
{
    ui->info->setEnabled(true);
}

void Ocounter::update_data(QByteArray &read_data)
{
    data.clear();
    for(int i = 0; i < read_data.size(); i++) {
        data[i] = read_data[i];
    }
    qDebug() << "update data" << data;

}

void Ocounter::update_shared_memory_data(QVector<double> vector)
{
    result.clear();
    for(int i = 0; i < vector.size(); i++) {
        result[i] = vector[i];
    }
    qDebug() << "update memory_data" << result;
}

void Ocounter::plot_settings()
{
    ui->plot->setInteraction(QCP::iRangeDrag, true);// взаимодействие удаления/приближения графика
    ui->plot->setInteraction(QCP::iRangeZoom, true);// взвимодействие перетаскивания графика

    ui->plot->addGraph();
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::green, Qt::green, 4));
    ui->plot->graph(0)->setLineStyle(QCPGraph::lsNone);

    ui->plot->xAxis->setLabel("t");
    ui->plot->yAxis->setLabel("L");


    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("h:m:s");
    ui->plot->xAxis->setTicker(dateTicker);

}

void Ocounter::real_plot()
{
    if(lazer_on) {
//    if( !q_x.empty() && !q_y.empty() )
//        {
//            q_x.clear();
//            q_y.clear();
//        }
//    ui->plot->graph(0)->setData(q_x, q_y);

//    ui->plot->replot();
//    ui->plot->update();


    for (const auto& item : graph_value) {
        q_x.append(item.first);
        for (const double& point : item.second) {
            if(point < min_L) {
                min_L = point;
            }
            if(point > max_L) {
                max_L = point;
            }
            ui->plot->graph(0)->addData(item.first, point);
        }

        ui->plot->xAxis->setRange(q_x.first() - 4, q_x.last() + 4);
        ui->plot->yAxis->setRange(min_L - 140, max_L + 140);
    }

    ui->plot->replot();
    ui->plot->update();
    }
}

void Ocounter::on_lon_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$LON\r");
        lazer_on = true;
        key_pressed = false;
    }

//    result << 1 << 4 << 4 << 1;
//    emit write_to_shared_memory(result);
}


void Ocounter::on_lof_clicked()
{
    if(key_pressed) {
    emit sent_data_to_com_port("$LOF\r");
        lazer_on = false;
        key_pressed = false;
    }
}

void Ocounter::on_ver_clicked()
{
//    if(key_pressed) {
//        emit sent_data_to_com_port("$VER\r");
//        key_pressed = false;
//    }

    emit read_from_shared_memory();
}

void Ocounter::on_vlt_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$VLT\r");
        key_pressed = false;
    }
}

void Ocounter::on_css_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$CSS\r");
        key_pressed = false;
    }
}

void Ocounter::on_tm1_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$TM1\r");
        key_pressed = false;
    }
}

void Ocounter::on_rst_clicked()
{
   if(key_pressed) {
       emit sent_data_to_com_port("$RST\r");
       key_pressed = false;
   }
}

void Ocounter::on_syn1_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$SYN1\r");
        key_pressed = false;
    }
}

void Ocounter::on_syn2_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$SYN2\r");
        key_pressed = false;
    }
}

void Ocounter::on_srr_clicked()
{
    if(key_pressed) {
        data.clear();
        data.resize(6);
        data[0] = '$';
        data[1] = 'S';
        data[2] = 'R';
        data[3] = 'R';
        data[4] = ' ';
        data[5] = ui->srr_spinBox->value();
        data[6] = '\r';
        emit sent_data_to_com_port(data);
        data.resize(0);
        key_pressed = false;
    }
}

void Ocounter::on_nim_clicked()
{
    if(key_pressed) {
        data.clear();
        data.resize(6);
        data[0] = '$';
        data[1] = 'N';
        data[2] = 'I';
        data[3] = 'M';
        data[4] = ' ';
        data[5] = ui->nim_spinBox->value();
        data[6] = '\r';
        emit sent_data_to_com_port(data);
        key_pressed = false;
    }
}

void Ocounter::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_S) {
        key_pressed = true;
    }
    else {
        key_pressed = false;
    }

    if(event->key() == Qt::Key_F1) {
        emit sent_data_to_com_port("$NIM 1\r");
    }

    if(event->key() == Qt::Key_V) {
        emit sent_data_to_com_port("$VER\r");
    }

    if(event->key() == Qt::Key_F2) {
        emit sent_data_to_com_port("$LON\r");
    }

    if(event->key() == Qt::Key_F3) {
        emit sent_data_to_com_port("$LOF\r");
    }

    if(event->key() == Qt::Key_F4) {
        emit sent_data_to_com_port("$CSS\r");
    }

    if(event->key() == Qt::Key_F5) {
        emit sent_data_to_com_port("$VLT\r");
    }

    if(event->key() == Qt::Key_T) {
        emit sent_data_to_com_port("$TM1\r");
    }

    if(event->key() == Qt::Key_R) {
        emit sent_data_to_com_port("$RST\r");
    }

    if(event->key() == Qt::Key_F6) {
        emit sent_data_to_com_port("$SYN1\r");
    }

    if(event->key() == Qt::Key_F7) {
        emit sent_data_to_com_port("$SYN2\r");
    }
}

void Ocounter::on_connected_clicked()
{
    if(is_connect) {
        ui->connected->setText("Connect");
        ui->connected->setStyleSheet("*{ background-color: rgb(0, 153, 0); color:  rgb(255, 255, 255)}");

        emit close_serial_port();
        is_connect = false;
        qDebug() << "serial close";
    }
    else if(!is_connect) {
        emit open_serial_port(ui->portName->currentText());
        emit sent_data_to_com_port("$VER\r");


        if(data.size() != 0) {
            ui->connected->setText("Disconnect");
            ui->connected->setStyleSheet("*{ background-color: rgb(255,0,0); color:  rgb(255, 255, 255)}");

            is_connect = true;
            qDebug() << "serial open";
            ui->read_log->append(data);
        } else {
            ui->read_log->append("Connection error");
        }
    }
}

void Ocounter::on_nim1_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$NIM 1\r");
        key_pressed = false;
    }
}

void Ocounter::on_info_clicked()
{

    window->show();
    ui->info->setEnabled(false);
    connect(window, &InfoWindow::info_enable, this, &Ocounter::info_bottom_enable);
}
