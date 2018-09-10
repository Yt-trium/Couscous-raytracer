// coucous includes.
#include "gui/mainwindow.h"

// Qt includes.
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
