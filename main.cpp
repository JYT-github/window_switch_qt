#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{


    QApplication app(argc, argv);

    QFile file(":/res/qss/qss.qss");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString styleSheet = filetext.readAll();
    app.setStyleSheet(styleSheet);
    file.close();
    MainWindow w;
    w.show();
    return app.exec();
}

