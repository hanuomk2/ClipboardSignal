#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    // w.show();                        // commnet out 2019.06.05
    w.show_systemtrayicon(true);        // システムトレイアイコン化 2019.06.05

    return a.exec();
}
