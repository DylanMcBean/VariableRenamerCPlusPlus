#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Load the stylesheet from the .qss file
    QFile file("dark_mode_modern.qss");
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);
    QString styleSheet = stream.readAll();
    file.close();

    // Apply the stylesheet to the QApplication
    a.setStyleSheet(styleSheet);

    MainWindow w;
    w.show();
    return a.exec();
}
