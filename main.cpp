#include <QApplication>
#include "otrs.h"
#include "login_config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    otrs w;
    w.show();


    return a.exec();
}
