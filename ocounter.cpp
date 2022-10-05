
#include "ocounter.h"
#include "ui_ocounter.h"

Ocounter::Ocounter(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Ocounter)
    , com_port(new ObjectCounter(this))
{
    ui->setupUi(this);

//чтение доступных портов при запуске
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        ui->portName->addItem(serialPortInfo.portName());
    }



    connect(this, &Ocounter::open_serial_port, com_port, &ObjectCounter::open_serial_port);
    connect(this, &Ocounter::sent_data_to_com_port, com_port, &ObjectCounter::writeData);
    connect(this, &Ocounter::close_serial_port, com_port, &ObjectCounter::close_serial_port);

    connect(com_port, &ObjectCounter::received_data, this, &Ocounter::parse_received_data);
    connect(com_port, &ObjectCounter::received_data, this, &Ocounter::update_data);

    device_start = QDateTime::currentDateTime().toTime_t();
    plot_settings();

}

Ocounter::~Ocounter()
{
    delete com_port;
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

    if(strstr(data.constData(),"time")) {

    for(int i = 0; i < data.size() ; i++) {

        if(data[i]  == ',' && i == data.size() - 1) break;

        if(data[i] == 'e') {
            flag = 1;
            i++;
        }

        if (data[i] == ' ' && tmp.size() != 0) {
            flag = 0;
            time = /*device_start + */tmp.toDouble();
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
    qDebug() << "parse in: " << graph_value;
    }
}

void Ocounter::update_data(QByteArray &read_data)
{
    data.clear();
    for(int i = 0; i < read_data.size(); i++) {
        data[i] = read_data[i];
    }
    qDebug() << "update data" << data;

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
        emit sent_data_to_com_port("$LON");
        lazer_on = true;
        key_pressed = false;
    }
}


void Ocounter::on_lof_clicked()
{
    if(key_pressed) {
    emit sent_data_to_com_port("$LOF");
        lazer_on = false;
        key_pressed = false;
    }
}

void Ocounter::on_ver_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$VER");
        key_pressed = false;
    }
}

void Ocounter::on_vlt_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$VLT");
        key_pressed = false;
    }
}

void Ocounter::on_css_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$CSS");
        key_pressed = false;
    }
}

void Ocounter::on_tm1_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$TM1");
        key_pressed = false;
    }
}

void Ocounter::on_rst_clicked()
{
   if(key_pressed) {
       emit sent_data_to_com_port("$RST");
       key_pressed = false;
   }
}

void Ocounter::on_syn1_clicked()
{
    if(key_pressed) {
//        writeData("$SYN1");
        emit sent_data_to_com_port("$SYN1");
        key_pressed = false;
    }
}

void Ocounter::on_syn2_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$SYN2");
        key_pressed = false;
    }
}

void Ocounter::on_srr_clicked()
{
    if(key_pressed) {
        data.resize(0);
        data.resize(5);
        data[0] = '$';
        data[1] = 'S';
        data[2] = 'R';
        data[3] = 'R';
        data[4] = ' ';
        data[5] = ui->srr_spinBox->value();
        emit sent_data_to_com_port(data);
        data.resize(0);
        key_pressed = false;
    }
}

void Ocounter::on_nim_clicked()
{
    if(key_pressed) {
        data.resize(0);
        data.resize(5);
        data[0] = '$';
        data[1] = 'N';
        data[2] = 'I';
        data[3] = 'M';
        data[4] = ' ';
        data[5] = ui->nim_spinBox->value();
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
        emit sent_data_to_com_port("$NIM 1");
    }

    if(event->key() == Qt::Key_V) {
        emit sent_data_to_com_port("$VER");
    }

    if(event->key() == Qt::Key_F2) {
        emit sent_data_to_com_port("$LON");
    }

    if(event->key() == Qt::Key_F3) {
        emit sent_data_to_com_port("$LOF");
    }

    if(event->key() == Qt::Key_F4) {
        emit sent_data_to_com_port("$CSS");
    }

    if(event->key() == Qt::Key_F5) {
        emit sent_data_to_com_port("$VLT");
    }

    if(event->key() == Qt::Key_T) {
        emit sent_data_to_com_port("$TM1");
    }

    if(event->key() == Qt::Key_R) {
        emit sent_data_to_com_port("$RST");
    }

    if(event->key() == Qt::Key_F6) {
        emit sent_data_to_com_port("$SYN1");
    }

    if(event->key() == Qt::Key_F7) {
        emit sent_data_to_com_port("$SYN2");
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
        emit sent_data_to_com_port("$VER");


        if(data[0]) {
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
//        writeData("$NIM 1");
        key_pressed = false;
    }
}

void Ocounter::on_info_clicked()
{
    window = new InfoWindow(this);
    window->show();
}
