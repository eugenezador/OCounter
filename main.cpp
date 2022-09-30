#include "ocounter.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Ocounter w;
    w.show();
    return a.exec();
}
