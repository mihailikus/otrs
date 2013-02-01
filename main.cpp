#include <QApplication>
#include "otrs.h"
#include "login_config.h"

int main(int argc, char *argv[])
{
    // Под Windows устанавливаем кодировку cp1251
    #ifdef Q_WS_WIN
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    // Под остальными ОС - utf8
    #else
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    #endif

    bool viewGUI = true;
    QString param;
    for (int i = 0; i<argc; i++) {
        param = codec->toUnicode(argv[i]);

        if (param == "--no_gui") {
            viewGUI = false;
        }
    }



    QApplication a(argc, argv);
    otrs w;
    if (viewGUI) {
        w.show();
    }


    return a.exec();
}
