#include "Application.h"

#include <QCameraInfo>
#include <QQmlContext>
/**
 * @brief Application::Application
 * @param parent
 */
Application::Application(QObject *parent) : QObject(parent)
    , m_webSocket(Q_NULLPTR)
    , m_webSocketServer(Q_NULLPTR)
    , m_camera(Q_NULLPTR)
    , m_videoSurface(Q_NULLPTR) {
    m_counter = 0;
}

Application::~Application() {
    if(m_camera) {
        m_camera->stop();
        m_camera->unload();
    }
}

/**
 * @brief Application::run
 */
void Application::run() {
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    //    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
    //                     &app, [url](QObject *obj, const QUrl &objUrl) {
    //                         if (!obj && url == objUrl)
    //                             QCoreApplication::exit(-1);
    //                     }, Qt::QueuedConnection);
    m_engine.rootContext()->setContextProperty("$application", this);

    m_frameProvider = new FrameProvider(this);
    m_frameProvider->setFormat(200, 200);

    m_engine.rootContext()->setContextProperty("$provider", m_frameProvider);


    QObject *camera = m_engine.rootObjects().at(0)->findChild<QObject *>("myCamera");
    if(camera) {
        m_camera = qvariant_cast<QCamera *>(camera->property("mediaObject"));
        m_camera->setCaptureMode(QCamera::CaptureStillImage);
        m_probe = new QVideoProbe(this);
        m_probe->setSource(m_camera);
        m_videoSurface = new MyVideoSurface(m_camera);
        m_camera->setViewfinder(m_videoSurface);
        connect(m_probe, &QVideoProbe::videoFrameProbed, m_frameProvider, &FrameProvider::onNewVideoContentReceived);
        connect(m_probe, &QVideoProbe::videoFrameProbed, this, &Application::onNewVideoContentReceived);
    } else {
        startQCamera();
    }
    m_engine.load(url);

    qRegisterMetaType<QAbstractSocket::SocketState>();
    m_webSocketServer = new QWebSocketServer("cameraServer", QWebSocketServer::NonSecureMode, this);
    if(m_webSocketServer) {
        if(m_webSocketServer->listen(QHostAddress::Any, 8080)) {
            connect(m_webSocketServer, &QWebSocketServer::newConnection, this, &Application::onNewConnection);
            qDebug() << "Listening on: " << m_webSocketServer->serverUrl() << m_webSocketServer->serverPort();
        }
    }
}

void Application::startQCamera() {
    m_camera = new QCamera(QCameraInfo::availableCameras().first(), this);
    m_camera->setCaptureMode(QCamera::CaptureStillImage);
    m_probe = new QVideoProbe(this);
    m_probe->setSource(m_camera);
    m_videoSurface = new MyVideoSurface(m_camera);
    m_camera->setViewfinder(m_videoSurface);
    //connect(m_probe, &QVideoProbe::videoFrameProbed, m_frameProvider, &FrameProvider::onNewVideoContentReceived);
    connect(m_probe, &QVideoProbe::videoFrameProbed, this, &Application::onNewVideoContentReceived);
    m_camera->load();
    m_camera->start();
}

/**
 * @brief Application::startCameraQml - example how to retrieve QML Camera and its frames
 */
void Application::startCameraQml() {
    QObject *camera = m_engine.rootObjects().at(0)->findChild<QObject *>("myCamera");
    if(camera) {
        m_camera = qvariant_cast<QCamera *>(camera->property("mediaObject"));
        m_camera->setCaptureMode(QCamera::CaptureStillImage);
        m_probe = new QVideoProbe(this);
        m_probe->setSource(m_camera);
        m_videoSurface = new MyVideoSurface(m_camera);
        m_camera->setViewfinder(m_videoSurface);
        connect(m_probe, &QVideoProbe::videoFrameProbed, m_frameProvider, &FrameProvider::onNewVideoContentReceived);
        connect(m_probe, &QVideoProbe::videoFrameProbed, this, &Application::onNewVideoContentReceived);
    }
}

void Application::clientConnected() {
    qDebug() << "clientConnected";
    m_webSocket->sendTextMessage(QStringLiteral("Hello, world!"));
}

void Application::clientClosed() {
    qDebug() << "clientClosed";
}

void Application::createClient() {
    qDebug() << __FUNCTION__;
    if(!m_webSocket)    {
        m_webSocket = new QWebSocket();
        connect(m_webSocket, &QWebSocket::connected, this, &Application::clientConnected);
        connect(m_webSocket, &QWebSocket::disconnected, this, &Application::clientClosed);
        connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &Application::clientReceivedBinary);
        connect(m_webSocket, &QWebSocket::textMessageReceived, this, &Application::clientReceivedText);
        connect(m_webSocket, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error), this, &Application::onError);
        m_webSocket->setParent(this);
    }
    m_webSocket->open(m_webSocketServer->serverUrl());
}

void Application::closeClient() {
    m_webSocket->close();
    qDebug() << __FUNCTION__ << "disconnected";
}

void Application::onError(QAbstractSocket::SocketError error) {
    qDebug() << __FUNCTION__ << error << ((m_webSocket != Q_NULLPTR) ? m_webSocket->errorString() : "");
}

void Application::clientReceivedBinary(const QByteArray &message) {
    qDebug() << __FUNCTION__ << message;
    //m_frameProvider->onNewVideoContentReceived()
}

void Application::clientReceivedText(const QString &message) {
    qDebug() << __FUNCTION__ << message;
}

void Application::onNewConnection() {
    qDebug() << __FUNCTION__;
    QWebSocket *socket = m_webSocketServer->nextPendingConnection();
    connect(socket, &QWebSocket::textMessageReceived, this, &Application::processTextMessage);
    connect(socket, &QWebSocket::binaryMessageReceived, this, &Application::processBinaryMessage);
    connect(socket, &QWebSocket::disconnected, this, &Application::socketDisconnected);
    m_clients << socket;
}

void Application::processTextMessage(QString message) {
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Message received:" << message;
    if(client) { client->sendTextMessage(message); }
}

void Application::processBinaryMessage(QByteArray message) {
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Binary Message received:" << message;
    if(client) { client->sendBinaryMessage(message); }
}

void Application::socketDisconnected() {
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qDebug() << "socketDisconnected:" << client;
    if (client) {
        m_clients.removeAll(client);
        client->deleteLater();
    }
}

void Application::onNewVideoContentReceived(const QVideoFrame &frame) {
    emit updateText("onNewVideoContentReceived");
    if(frame.isValid()) {
        emit updateText("validFrame");
    }
    if((m_counter % 15 ) == 0 && !m_clients.isEmpty()) {
        qDebug() << "sending frame" << m_counter;
        QVideoFrame clone(frame);
        clone.map(QAbstractVideoBuffer::ReadOnly);
        //        qDebug() << clone->isMapped();
        //frame.map(QAbstractVideoBuffer::ReadOnly);
        //        QByteArray *datagram = new QByteArray((char *)(clone->bits()), clone->mappedBytes());
        QByteArray datagram((char *)(clone.bits()), clone.mappedBytes());
        qDebug() << datagram.size();

        QImage i;
        QByteArray block;
        QDataStream stream(&block, QIODevice::WriteOnly);

        stream.writeBytes((char *)(clone.bits()), clone.mappedBytes());
        //        stream << qint16(0) << (char*)frame->bits();
        //        stream.device()->seek(0);
        //        stream << qint16(block.size() - sizeof(qint16));


        QWebSocket *client = m_clients.at(0);
        if(client) {
            //            QByteArray array = "Broadcasting frame " + QByteArray::number(m_counter);
            client->sendTextMessage("Sending frame...");
            qDebug() << client->sendBinaryMessage(block);
        }
        //writer->writeDatagram((char*)clone->bits(),clone->mappedBytes(),QHostAddress::Broadcast, 45454);
        clone.unmap();
    }
    m_counter++;
}

QList<QVideoFrame::PixelFormat> MyVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const {
    qDebug() << type;
    if (type == QAbstractVideoBuffer::NoHandle) {
        qDebug() << "type == QAbstractVideoBuffer::NoHandle";
        return QList<QVideoFrame::PixelFormat>()
               << QVideoFrame::Format_RGB24
               << QVideoFrame::Format_ARGB32
               << QVideoFrame::Format_ARGB32_Premultiplied
               << QVideoFrame::Format_RGB32
               << QVideoFrame::Format_RGB24
               << QVideoFrame::Format_RGB565
               << QVideoFrame::Format_RGB555
               << QVideoFrame::Format_ARGB8565_Premultiplied
               << QVideoFrame::Format_BGRA32
               << QVideoFrame::Format_BGRA32_Premultiplied
               << QVideoFrame::Format_BGR32
               << QVideoFrame::Format_BGR24
               << QVideoFrame::Format_BGR565
               << QVideoFrame::Format_BGR555
               << QVideoFrame::Format_BGRA5658_Premultiplied
               << QVideoFrame::Format_AYUV444
               << QVideoFrame::Format_AYUV444_Premultiplied
               << QVideoFrame::Format_YUV444
               << QVideoFrame::Format_YUV420P
               << QVideoFrame::Format_YV12
               << QVideoFrame::Format_UYVY
               << QVideoFrame::Format_YUYV
               << QVideoFrame::Format_NV12
               << QVideoFrame::Format_NV21
               << QVideoFrame::Format_IMC1
               << QVideoFrame::Format_IMC2
               << QVideoFrame::Format_IMC3
               << QVideoFrame::Format_IMC4
               << QVideoFrame::Format_Y8
               << QVideoFrame::Format_Y16
               << QVideoFrame::Format_Jpeg
               << QVideoFrame::Format_CameraRaw
               << QVideoFrame::Format_AdobeDng;
    } else {
        return QList<QVideoFrame::PixelFormat>();
    }
}

bool MyVideoSurface::present(const QVideoFrame &frame) {
    //    qDebug() << "present";
    emit frameIsPresent("present");
    emit framePresent(frame);
    return true;
}
