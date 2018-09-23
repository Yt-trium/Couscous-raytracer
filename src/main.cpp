// coucous includes.
#include "gui/mainwindow.h"
#include "test/test.h"

// Qt includes.
#include <QApplication>

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "--run-unit-tests") == 0)
    {
        return run_tests();
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
