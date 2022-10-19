#include <QFileDialog>
#include "ocounter.h"
#include "ui_ocounter.h"

Ocounter::Ocounter(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Ocounter)
    , window(new InfoWindow(this))
    , com_port(new ComPort(this))
    , shared_memory(new MShare(this))
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
//    connect(shared_memory, &MShare::read_data_from_shared_memory, this, &Ocounter::update_shared_memory_data, Qt::QueuedConnection);


//    device_start = QDateTime::currentDateTimeUtc().toTime_t();

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
        QString str = QString(data);
    QVector<double> values;
    double time = 0;
    QString tmp;
    int k = 0;
    int flag = 0;

    // если нет целей сразу выодим соответствующее сообщение
    if(strstr(data.constData(),"none")) {
        ui->read_log->append("No targets detected");
    }

    else if(strstr(data.constData(),"time")) {

    for(int i = 0; i < str.size() ; i++) {

        //выходим из цикла если последний элемент запятая
        if(str[i]  == ',' && i == str.size() - 1) break;

        // ловим начало числа авремени
        if(str[i] == 'e') {
            qDebug() << "flag 1 e";
            flag = 1;
            i++;
        }

        // ловим конец числа времени
        if (str[i] == ' ' && str.size() != 0) {
            flag = 0;
            qDebug() << "time : "<< tmp;
            time = lazer_start + tmp.toDouble() * 0.001;
//            result.push_back(tmp.toDouble());
            k =0;

            tmp.clear();
        }

        // ловим начало первой цели
        if(str[i] == ':' /*&& tmp.size() != 0*/) {
            qDebug() << "flag 1 :";
            flag = 1;
            i++;
        }

        // ловим начало второй цели
        if(str[i] == ',' /*&& tmp.size() != 0*/ && i != data.size() - 1 ) {
            qDebug() << "flag 1 ,";
            flag = 1;
            i++;
        }

        // ловим конец числа
        if (str[i] == '(' && data.size() != 0) {
            qDebug() << "point: " << tmp;
            flag = 0;
            values.append(tmp.toDouble());
//            result.push_back(tmp.toDouble());
            k =0;

            tmp.clear();
        }

        // если мы внутри нужного числа, то записываем i-й элемент в буферный массив
        if(flag) {
//            qDebug() << "inside need data: "<< str.at(i);
            tmp[k] = str[i];
//            qDebug() << "inside need tmp: "<< tmp.at(k);
            k++;
        }
    }

    graph_value[time] = values;

    real_plot();

//    emit write_to_shared_memory(result);

    ui->read_log->append(QString(data));

    qDebug() << "parse : " << graph_value;
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
    ui->plot->xAxis->setTicker(dateTicker);

    ui->plot->xAxis->setRange(QDateTime::currentDateTimeUtc().toTime_t(), QDateTime::currentDateTimeUtc().toTime_t()  + 200 );
    dateTicker->setDateTimeFormat("h:m:s");

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

void Ocounter::com_port_permission()
{
//    QString fileName = QFileDialog::getOpenFileName(this,
//    tr("Open Script"), "/", tr("Script Files (*permission.sh)"));

//    if (QProcess::execute(QString("/bin/sh ") + fileName) < 0)
//        qDebug() << "Failed to run";

}

void Ocounter::on_lon_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$LON\r");
        lazer_on = true;
        key_pressed = false;
        lazer_start = QDateTime::currentDateTimeUtc().toTime_t();
    }

//    parse_received_data("Opt ch time255405 pnts:292.5(4383),331.4(1077),");

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

//    parse_received_data("Opt ch1 time33840 pnts:none");
//    parse_received_data("Opt ch1 time34840 pnts:1185.3(436),1489.8(26),1636.6(25),1747.8(46),1785.6(519),1802.9(78),");
//    parse_received_data("#Opt ch1 time35840 pnts:1470.0(748),1634.2(1032),1745.1(904),1784.7(686),1802.3(108),");
//    parse_received_data("#Opt ch1 time36839 pnts:1503.8(615),");
//    parse_received_data("#Opt ch1 time37839 pnts:176.2(1197),225.6(1242),1191.3(34),1503.8(548),");
//    parse_received_data("#Opt ch1 time38839 pnts:203.7(1745),1258.4(810),1329.7(155),1393.6(179),1451.1(111),1469.4(35),");
//    parse_received_data("#Opt ch1 time39839 pnts:1193.7(94),1331.8(800),1449.6(630),1495.2(24),1512.8(26),");
//    parse_received_data("#Opt ch1 time40839 pnts:1193.1(120),1331.2(102),1437.9(507),1451.4(402),");
//    parse_received_data("#Opt ch1 time41839 pnts:1194.0(57),1331.2(24),1437.6(479),1450.2(183),");
//    parse_received_data("#Opt ch1 time42839 pnts:1193.1(143),1334.2(520),1437.6(647),1451.4(407),");
//    parse_received_data("#Opt ch1 time43839 pnts:1193.7(99),1334.2(485),1437.9(596),1451.1(363),");
//    parse_received_data("#Opt ch1 time44839 pnts:1194.3(61),1334.5(574),1438.5(591),1451.1(301),");


}

void Ocounter::on_ver_clicked()
{
    if(key_pressed) {
        emit sent_data_to_com_port("$VER\r");
        key_pressed = false;
    }

//    emit read_from_shared_memory();
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
//    com_port_permission();
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

        if(com_port->serial->isOpen()) {
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
//    ui->info->setEnabled(false);
//    connect(window, &InfoWindow::info_enable, this, &Ocounter::info_bottom_enable);
}
