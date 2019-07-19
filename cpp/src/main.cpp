#include <QApplication>
#include "cpp/include/demo.h"

int main (int argc, char *argv[])
{
    QApplication app (argc, argv);

    Demo *dem = new Demo();
    dem->show();

    return app.exec();
}
