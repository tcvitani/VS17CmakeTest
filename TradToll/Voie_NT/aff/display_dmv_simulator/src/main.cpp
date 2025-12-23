#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include "Worker/Worker.h"

QFile logFile("DmvSimulator.log");

void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qCritical() << "Unable to open log file.";
        return -1;
    }

    qInstallMessageHandler(customMessageHandler);
    
    Worker* worker = new Worker(QString("cfg.json"));
    if (!worker->init())
    {
        return -1;
    }
    
    QQmlApplicationEngine engine;

    const QUrl url("qrc:/qt/qml/dmvsimulator/main.qml");

    engine.rootContext()->setContextProperty("worker", worker);
    engine.load(url);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}


void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QTextStream out(&logFile);
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    switch (type) {
    case QtDebugMsg:
        out << time << " [DEBUG] " << msg << "\n";
        break;
    case QtInfoMsg:
        out << time << " [INFO] " << msg << "\n";
        break;
    case QtWarningMsg:
        out << time << " [WARNING] " << msg << "\n";
        break;
    case QtCriticalMsg:
        out << time << " [CRITICAL] " << msg << "\n";
        break;
    case QtFatalMsg:
        out << time << " [FATAL] " << msg << "\n";
        abort();
    }
    out.flush(); // Ensure the message is written to the file immediately
}