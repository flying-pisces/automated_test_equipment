#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    int status = 0;

    try
    {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        status = a.exec();
    }
    catch(std::exception exp)
    {
        QApplication error(argc, argv);
        QMessageBox msgBox;

        msgBox.setText(QString("Program error: %1").arg(exp.what()));
        msgBox.exec();
    }

    return status;
}
