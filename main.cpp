#include <QGuiApplication>
#include "Application.h"

/**
 * @brief qMain
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    qputenv("QT_FORCE_STDERR_LOGGING", QByteArray("1"));
    Application application;
    application.run();

    return app.exec();
}
