#include "Application.h"


Application::Application(QObject *parent) : QObject(parent) {

}

void Application::run() {
    m_engine = new QQmlApplicationEngine(this);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    //    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
    //                     &app, [url](QObject *obj, const QUrl &objUrl) {
    //                         if (!obj && url == objUrl)
    //                             QCoreApplication::exit(-1);
    //                     }, Qt::QueuedConnection);
    m_engine->load(url);
}
