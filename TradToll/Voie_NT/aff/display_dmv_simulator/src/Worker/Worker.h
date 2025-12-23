#pragma once

#include <QObject>
#include <QQmlContext>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <OblakMsg.h>
#include "UdpServer/UdpServer.h"

class Worker : public QObject
{
	Q_OBJECT

public:
	explicit Worker(QString cfgFile, QObject *parent = 0);
	~Worker();

	class JsonData {
	public:
		QString Addr;
		quint16 Port;
		quint16 EchoPort;
		QString ClientAddr;
		quint16 ClientPort;
	};

	bool init();
	bool deserializeJson(const QString& jsonFile);

signals:
	void newTrafficLightSymbol(QString color);
	void newDisplayMessage(QString message);
	void messageProcessed(bool isMessageOK);

private slots:
	void onReceivedDatagram(const QByteArray& datagram);

private:
	JsonData   data;
	UdpServer* server;
	
	QString	   jsonFile;

	OblakMsg::eMessageType getMessageType(const QByteArray& datagram);
	QString getDisplayMessage(const QByteArray& datagram);
	QString getTrafficLightSymbol(const QByteArray& datagram);
	void processFlasherCommand(const QByteArray& datagram);
};