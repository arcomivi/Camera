#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QQmlApplicationEngine>

class Application : public QObject {
        Q_OBJECT
    public:
        explicit Application(QObject *parent = Q_NULLPTR);
        void run();
    signals:

    public slots:
    private:
        QQmlApplicationEngine *m_engine;
};

#endif // APPLICATION_H
