#include "ocounter.h"
#include "ui_ocounter.h"
//#include "infowindow.h"

Ocounter::Ocounter(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Ocounter)
{
    ui->setupUi(this);

    serial = new QSerialPort(this);

//чтение доступных портов при запуске
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        ui->portName->addItem(serialPortInfo.portName());
    }

// параметры порта
    //
    currentPortName = ui->portName->currentText();
    serial->setPortName(currentPortName);
    serial->open(QIODevice::ReadWrite);
    serial->setBaudRate(QSerialPort::Baud38400);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    connect(ui->portName, &QComboBox::currentTextChanged, this, &Ocounter::serial_port_properties);
    //

    plot_settings();

    //connect();

}

Ocounter::~Ocounter()
{
    serial->close();
    delete serial;
    delete ui;
}

void Ocounter::serial_port_properties(const QString &text)
{
    bool currentPortNameChanged = false;
    qDebug() << __FUNCTION__;
    if (currentPortName != text) {
        currentPortName = text;
        currentPortNameChanged = true;
       } else {
           currentPortNameChanged = false;
       }

    if (currentPortNameChanged) {

        serial->close();

        serial->setPortName(currentPortName);

        serial->open(QIODevice::ReadWrite);
        serial->setBaudRate(QSerialPort::Baud38400);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
    }
}

void Ocounter::parse(const QByteArray &data, std::map<double, QVector<double>> &graph_value)
{
    QVector<double> values;
    double time = 0;
    QString tmp;
    int k = 0;
    int flag = 0;

    for(int i = 0; i < data.size() ; i++) {

        if(data[i]  == ',' && i == data.size() - 1) break;

        if(data[i] == 'e') {
            flag = 1;
            i++;
        }

        if (data[i] == ' ' && tmp.size() != 0) {
            flag = 0;
            time = tmp.toDouble();
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

void Ocounter::aim_parser(const QByteArray &data, QVector<QVector<double> > &lazer_value)
{
    QVector<double> value;
    QString tmp;
    int k = 0;
    int flag = 0;

    for(int i = 0; i < data.size() ; i++) {

        if(data[i]  == ',' && i == data.size() - 1) break;

        if(data[i] == 'e') {
            flag = 1;
            i++;
        }

        if (data[i] == ' ' && tmp.size() != 0) {
            flag = 0;
            value.push_back(tmp.toDouble());
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
            value.append(tmp.toDouble());
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

    lazer_value.push_back(value);
    qDebug() << "lazer: " << lazer_value;
}

void Ocounter::plot_settings()
{
    ui->plot->setInteraction(QCP::iRangeDrag, true);// взаимодействие удаления/приближения графика
    ui->plot->setInteraction(QCP::iRangeZoom, true);// взвимодействие перетаскивания графика

    ui->plot->addGraph();
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plot->graph(0)->setLineStyle(QCPGraph::lsNone);

    ui->plot->xAxis->setLabel("t");
    ui->plot->yAxis->setLabel("L");


    double now = QDateTime::currentDateTime().toTime_t();
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("h:m:s");
    ui->plot->xAxis->setTicker(dateTicker);

    ui->plot->xAxis->setRange(now, now + 100);

}

void Ocounter::real_plot()
{
    if( !q_x.empty() && !q_y.empty() )
        {
            q_x.clear();
            q_y.clear();
        }
    ui->plot->graph(0)->setData(q_x, q_y);

    ui->plot->replot();
    ui->plot->update();

    for (const auto& item : graph_value) {
        q_x.append(item.first);
        q_y = {};
        for (const double& point : item.second) {
            q_y.append(point);
            ui->plot->graph(0)->addData(q_x.last(), q_y.last());
        }

        ui->plot->xAxis->setRange(q_x.first(), q_x.last());
        ui->plot->yAxis->setRange(q_y.first(), q_y.last());
    }

    ui->plot->replot();
    ui->plot->update();
}

void Ocounter::plot()
{
    for (int i = 0; i < lazer_value.size(); i++) {
        for (int j = 1; j < lazer_value[i].size(); j++) {
            ui->plot->graph(0)->addData(lazer_value[i][0], lazer_value[i][j]);
        }

//        ui->plot->xAxis->setRange(q_x.first(), q_x.last());
//        ui->plot->yAxis->setRange(q_y.first(), q_y.last());
    }

    ui->plot->replot();
    ui->plot->update();
}

void Ocounter::create_shared_memory()
{
    if(!share_memory.create(graph_value.size())){
        qDebug() << "no memory";
        return;
    }

//    for (const auto& item : graph_value) {

//        for (const double& point : item.second) {
//            double *dist = (double*)share_memory.data();
//            const double *source = item.second.data();
//        }
//    }

//    share_memory.lock();
//    double *dist = (double*)share_memory.data();
//    const double *source = result.data();
//    memcpy(dist, source, result.size() + 1);
//    share_memory.unlock();
}

void Ocounter::read_shared_memory()
{


}

void Ocounter::detach_shared_memory()
{
    if (!share_memory.detach()) {
        qDebug() << "can't detach";
    }
}


void Ocounter::writeData(const QByteArray &data)
{
    if(serial->isOpen() == true){
        serial->write(data);
        serial->write("\r");
        serial->waitForBytesWritten();

        qDebug() << "write: " << data;
        ui->write_log->append(data);

        readData();
    }
    else
    {
        qDebug() << "not open";
        ui->write_log->append("No Connection");
    }
}

void Ocounter::readData()
{
    //QByteArray data;
    if(serial->isReadable())
    {
       while ( serial->waitForReadyRead(90) )
       {
           // результат чтения накапливается в переменную data
           data.append(serial->readAll());
           if(strcmp(data.end() - 1 , "\\") == 1 && strcmp(data.end(), "r")) break;
       }
    }

    if(strstr(data.constData(),"time") && strstr(data.constData(),"none") == 0) {
        parse(data, graph_value);
        real_plot();
    }
    if(strstr(data.constData(),"none")) {
        qDebug() << "parse: none";
    }
    ui->read_log->append(data);
}

void Ocounter::on_lon_clicked()
{
//    if(key_pressed) {
//        writeData("$LON");
//        key_pressed = false;
//    }

   aim_parser("#Opt ch1 time81 pnts:203.7(1745),1258.4(810),1329.7(155),1393.6(179),1451.1(111),1469.4(35),", lazer_value);
   aim_parser("#Opt ch1 time104 pnts:203.7(1745),1258.4(810),1329.7(155),1393.6(179),", lazer_value);
   aim_parser("#Opt ch1 time144 pnts:203.7(1745),", lazer_value);
   //real_plot();
}


void Ocounter::on_lof_clicked()
{
    if(key_pressed) {
        writeData("$LOF");
        key_pressed = false;
    }

//    parse("#Opt ch1 time41414 #pnts:418.14(321)", graph_value);
}

void Ocounter::on_ver_clicked()
{
    if(key_pressed) {
        writeData("$VER");
        key_pressed = false;
    }
}

void Ocounter::on_vlt_clicked()
{
    if(key_pressed) {
        writeData("$VLT");
        key_pressed = false;
    }
}

void Ocounter::on_css_clicked()
{
    if(key_pressed) {
        writeData("$CSS");
        key_pressed = false;
    }
}

void Ocounter::on_tm1_clicked()
{
    if(key_pressed) {
        writeData("$TM1");
        key_pressed = false;
    }
}

void Ocounter::on_rst_clicked()
{
   if(key_pressed) {
       writeData("$RST");
       key_pressed = false;
   }
}

void Ocounter::on_syn1_clicked()
{
    if(key_pressed) {
        writeData("$SYN1");
        key_pressed = false;
    }
}

void Ocounter::on_syn2_clicked()
{
    if(key_pressed) {
        writeData("$SYN2");
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
        writeData(data);
        data.resize(0);
        key_pressed = false;
    }
}

void Ocounter::on_nim_clicked()
{
    if(key_pressed) {
//        data.resize(0);
//        data.resize(5);
//        data[0] = '$';
//        data[1] = 'N';
//        data[2] = 'I';
//        data[3] = 'M';
//        data[4] = ' ';
//        data[5] = ui->nim_spinBox->value();
        for(int i = 0; i < ui->nim_spinBox->value(); i++) {
            writeData("$NIM 1");
        }
//        writeData(data);
//        data.resize(0);
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
        writeData("$NIM 1");
    }

    if(event->key() == Qt::Key_V) {
        writeData("$VER");
    }

    if(event->key() == Qt::Key_F2) {
        writeData("$LON");
    }

    if(event->key() == Qt::Key_F3) {
        writeData("$LOF");
    }

    if(event->key() == Qt::Key_F4) {
        writeData("$CSS");
    }

    if(event->key() == Qt::Key_F5) {
        writeData("$VLT");
    }

    if(event->key() == Qt::Key_T) {
        writeData("$TM1");
    }

    if(event->key() == Qt::Key_R) {
        writeData("$RST");
    }

    if(event->key() == Qt::Key_F6) {
        writeData("$SYN1");
    }

    if(event->key() == Qt::Key_F7) {
        writeData("$SYN2");
    }


}

void Ocounter::on_connected_clicked()
{
    if(is_connect) {
        serial->close();
        ui->connected->setText("Connect");
        ui->connected->setStyleSheet("*{ background-color: rgb(0, 153, 0); color:  rgb(255, 255, 255)}");

        is_connect = false;
        qDebug() << "serial close";
    }
    else if(!is_connect) {
        serial->open(QIODevice::ReadWrite);
        serial->setBaudRate(QSerialPort::Baud38400);

        data.resize(0);
        writeData("$VER");

        if(data.size() != 0) {
            ui->connected->setText("Disconnect");
            ui->connected->setStyleSheet("*{ background-color: rgb(255,0,0); color:  rgb(255, 255, 255)}");

            is_connect = true;
            qDebug() << "serial open";
        } else {
            ui->read_log->append("Connection error");
        }
    }
}

void Ocounter::on_nim1_clicked()
{
    if(key_pressed) {
        writeData("$NIM 1");
        key_pressed = false;
    }
}

void Ocounter::on_info_clicked()
{
    window = new InfoWindow(this);
    window->show();
}
