#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QVideoSurfaceFormat>
#include <QVideoProbe>
#include <QCamera>
#include <QWebSocket>
#include <QtWebSockets/QWebSocketServer>

#include <QtMultimedia/QAbstractVideoSurface>

class FrameProvider : public QObject {
        Q_OBJECT
        Q_PROPERTY(QAbstractVideoSurface *videoSurface READ videoSurface WRITE setVideoSurface)


    public:
        FrameProvider(QObject *parent = Q_NULLPTR) : QObject(parent) {

        }
        QAbstractVideoSurface *videoSurface() const { return m_surface; }

    private:
        QAbstractVideoSurface *m_surface = Q_NULLPTR;
        QVideoSurfaceFormat m_format;

    public:


        void setVideoSurface(QAbstractVideoSurface *surface) {
            if (m_surface && m_surface != surface  && m_surface->isActive()) {
                m_surface->stop();
            }

            m_surface = surface;

            if (m_surface && m_format.isValid()) {
                m_format = m_surface->nearestFormat(m_format);
                m_surface->start(m_format);

            }
        }

        void setFormat(int width, int heigth) { //}, int format = QVideoFrame::Format_RGB32) {
            QSize size(width, heigth);
            QVideoSurfaceFormat format(size, QVideoFrame::Format_RGB32);
            m_format = format;

            if (m_surface) {
                if (m_surface->isActive()) {
                    m_surface->stop();
                }
                m_format = m_surface->nearestFormat(m_format);
                m_surface->start(m_format);
            }
        }

    public slots:
        void onNewVideoContentReceived(const QVideoFrame &frame) {

            if (m_surface) {
                m_surface->present(frame);
            }
        }
};

class MyVideoSurface : public QAbstractVideoSurface {
        Q_OBJECT
    public:
        MyVideoSurface(QObject *parent = Q_NULLPTR) : QAbstractVideoSurface(parent)  {}
        virtual ~MyVideoSurface() {}

        // QAbstractVideoSurface interface
    public:
        QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const;
        bool present(const QVideoFrame &frame);
    Q_SIGNALS:
        void framePresent(QVideoFrame frame);
        void frameIsPresent(QString present);
};

class Application : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged)
    public:
        explicit Application(QObject *parent = Q_NULLPTR);
        ~Application();
        void run();
        void startQCamera();
        void startCameraQml();
        QString serverUrl() const;
        void setServerUrl(const QString &serverUrl);

    signals:
        void updateText(QString text);
        void serverUrlChanged(QString serverUrl);
    public slots:
        void onError(QAbstractSocket::SocketError error);
        void onNewConnection();
        void processTextMessage(QString message);
        void processBinaryMessage(QByteArray message);
        void socketDisconnected();
        void onNewVideoContentReceived(const QVideoFrame &frame);
    private:
        QString m_serverUrl;
        int m_counter;
        QList<QWebSocket *> m_clients;
        QWebSocketServer *m_webSocketServer;
        QQmlApplicationEngine m_engine;
        QCamera *m_camera;
        QVideoProbe *m_probe;
        MyVideoSurface *m_videoSurface;
        FrameProvider *m_frameProvider;
};

#endif // APPLICATION_H
