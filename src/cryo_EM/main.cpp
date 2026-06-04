
#include <QApplication>
#include <QMovie>
#include <QPixmap>
#include <QSettings>
#include <QSplashScreen>
#include <QStyleFactory>

#include "mainwindow.h"
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/logo/lib/icon.png"));
    // Change default message handler output
    qSetMessagePattern("Message:%{message} File:%{file} Line:%{line} Function:%{function}");

    QPixmap pixmap(":/loading/lib/loading.gif");
    QSplashScreen splash(pixmap);
    splash.show();

    a.processEvents();
    qDebug() << "QStyleFactory::keys() " << QStyleFactory::keys();  // QList("windowsvista", "Windows", "Fusion")
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Normal, QPalette::WindowText, Qt::black);
    palette.setColor(QPalette::Normal, QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::Normal, QPalette::Text, Qt::black);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, Qt::lightGray);  // Seems to not work on Windows
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::lightGray);
    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::lightGray);
    palette.setColor(QPalette::Inactive, QPalette::Text, Qt::black);
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, Qt::black);
    palette.setColor(QPalette::Normal, QPalette::Button, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::Button, Qt::white);
    palette.setColor(QPalette::Inactive, QPalette::Button, Qt::white);

    qApp->setPalette(palette);

    QCoreApplication::setOrganizationName("SDU");
    QCoreApplication::setApplicationName("Autom3D");
    QSettings settings;  // By default stored in registry or ini file
    if (!settings.contains("python_path")) {
        settings.setValue("python_path", "python");
    }
    qDebug() << "python_path: " << settings.value("python_path").toString();

    MainWindow w;
    w.setWindowIcon(QIcon(":/logo/lib/icon.png"));
    w.setWindowTitle("Autom3D");
    // w.show();
    w.showMaximized();

    splash.finish(&w);

    if(argc > 1) w.open_mrc_file(argv[1]);
        

    return a.exec();
}
