#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QStringConverter>
#include "Worker.h"


constexpr int imagePosition = 16;
constexpr int textPosition = 57;

constexpr unsigned char c_SetTextCmdCode = '\x11';
constexpr unsigned char c_SetImageCmdCode = '\x0C';
constexpr unsigned char c_ActivateScenarioCmdCode = '\x0D';

const QByteArray c_FlasherCmdHeader("\x00\x00\x00\x00\x00\x00\x00\x00\x00\07", 10);
const QByteArray c_FlasherTableIndex("\xFE\x71", 2);


Worker::Worker(QString cfgFile, QObject* parent) : QObject(parent)
{
	std::memset(&data, 0, sizeof(data));
	jsonFile = cfgFile;
}

Worker::~Worker()
{
	if (server != nullptr)
	{
		server->deleteLater();
	}

	if (this != nullptr)
	{
		this->deleteLater();
	}
}

bool Worker::init()
{
	if (!deserializeJson(jsonFile))
		return false;

	server = new UdpServer(data.Addr, data.Port, data.EchoPort);
	connect(server, &UdpServer::receivedDatagram, this, &Worker::onReceivedDatagram);
	
	return true;
}

bool Worker::deserializeJson(const QString& jsonFile)
{
	QFile file(jsonFile);
	QByteArray fileContent;
	QJsonParseError parseError;
	QJsonDocument jsonDoc;
	QJsonObject jsonObj;

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
	{
		qWarning("Couldn't open the cfg.json file. Maybe missing");
		return false;
	}

	fileContent = file.readAll();
	file.close();

	jsonDoc = QJsonDocument::fromJson(fileContent, &parseError);

	if (parseError.error != QJsonParseError::NoError) 
	{
		qWarning() << "JSON parse error: " << parseError.errorString();
		return false;
	}

	if (!jsonDoc.isObject()) 
	{
		qWarning("JSON is not an object.");
		return false;
	}

	jsonObj = jsonDoc.object();

	// The actual deserialization

	if (jsonObj.contains("Addr") && jsonObj["Addr"].isString()) 
	{
		data.Addr = jsonObj["Addr"].toString();
		qDebug() << "Value for Addr:" << data.Addr;
	}
	else
	{
		return false;
	}

	if (jsonObj.contains("Port") && jsonObj["Port"].isDouble())
	{
		data.Port = jsonObj["Port"].toInt();
		qDebug() << "Value for Port:" << data.Port;
	}
	else
	{
		return false;
	}

	if (jsonObj.contains("EchoPort") && jsonObj["EchoPort"].isDouble())
	{
		data.EchoPort = jsonObj["EchoPort"].toInt();
		qDebug() << "Value for EchoPort:" << data.EchoPort;
	}
	else
	{
		return false;
	}

	if (jsonObj.contains("ClientAddr") && jsonObj["ClientAddr"].isString())
	{
		data.ClientAddr = jsonObj["ClientAddr"].toString();
		qDebug() << "Value for ClientAddr:" << data.ClientAddr;
	}
	else
	{
		return false;
	}

	if (jsonObj.contains("ClientPort") && jsonObj["ClientPort"].isDouble())
	{
		data.ClientPort = jsonObj["ClientPort"].toInt();
		qDebug() << "Value for ClientPort:" << data.ClientPort;
	}
	else
	{
		return false;
	}

	return true;
}

OblakMsg::eMessageType Worker::getMessageType(const QByteArray& datagram)
{
	// This is manual parsing of DMV OBLAK protocol messages
	// If the received datagram starts with c_FlasherCmdHeader it is a flasher command
	// If the received datagram has the byte 0x11 on the 14th position, then it is a text message
	// If the received datagram has the byte 0x0C on the 14th position then it is a traffic light image

	if (datagram.startsWith(c_FlasherCmdHeader))
	{
		if (datagram.contains(c_FlasherTableIndex))
		{
			return OblakMsg::eOblakMsg_SetFlasher;
		}
		else
		{
			return OblakMsg::eOblakMsg_ChangeFlasherParameter;
		}
	}
	else // Absolute commands
	{
		unsigned char msgType = static_cast<unsigned char>(datagram.at(13));

		if (msgType == c_SetTextCmdCode)
		{
			return OblakMsg::eOblakMsg_SetPropertiesAndText;
		}
		else if (msgType == c_SetImageCmdCode)
		{
			return OblakMsg::eOblakMsg_SetImage;
		}
		else if (msgType == c_ActivateScenarioCmdCode)
		{
			return OblakMsg::eOblakMsg_ActivateScenario;
		}
		else
		{
			QString msgTypeStr = QString::number(msgType, 16).toUpper().rightJustified(2, '0');
			qWarning() << "Worker::getMessageType() - Undefined message type" << msgTypeStr;
			return OblakMsg::eOblakMsg_ActivateScenario; // This is just so an acknowledgement is sent to main app
		}
	}
}

void Worker::onReceivedDatagram(const QByteArray& datagram)
{
	OblakMsg::eMessageType msgType = getMessageType(datagram);

	switch (msgType)
	{
	case OblakMsg::eOblakMsg_ActivateScenario:
		qInfo() << "Worker::onReceivedDatagram() - The message eOblakMsg_ActivateScenario was sent, this does not need to be simulated";
		server->sendMessageProcessed(true);
		break;
	case OblakMsg::eOblakMsg_SetBrightness:
		qInfo() << "Worker::onReceivedDatagram() - The message eOblakMsg_SetBrightness is not currently simulated";
		server->sendMessageProcessed(true);
		break;
	case OblakMsg::eOblakMsg_TimerReset:
		qInfo() << "Worker::onReceivedDatagram() - The message eOblakMsg_TimerReset is not currently simulated";
		server->sendMessageProcessed(true);
		break;
	case OblakMsg::eOblakMsg_SetImage:
		emit newTrafficLightSymbol(getTrafficLightSymbol(datagram));
		break;
	case OblakMsg::eOblakMsg_SetTextProperties:
		qInfo() << "Worker::onReceivedDatagram() - The message eOblakMsg_SetTextProperties is simulated in another message";
		server->sendMessageProcessed(true);
		break;
	case OblakMsg::eOblakMsg_SetTextMessage:
		qInfo() << "Worker::onReceivedDatagram() - The message eOblakMsg_SetTextMessage is simulated in another message";
		server->sendMessageProcessed(true);
		break;
	case OblakMsg::eOblakMsg_SetPropertiesAndText:
		emit newDisplayMessage(getDisplayMessage(datagram));
		break;
	case OblakMsg::eOblakMsg_SetSymbol:
		qInfo() << "Worker::onReceivedDatagram() - The message eOblakMsg_SetSymbol is not currently simulated";
		server->sendMessageProcessed(true);
		break;
	case OblakMsg::eOblakMsg_Response:
		qInfo() << "Worker::onReceivedDatagram() - The message eOblakMsg_Response should not be handled here";
		server->sendMessageProcessed(true);
		break;
	case OblakMsg::eOblakMsg_SetFlasher:
		processFlasherCommand(datagram);
		server->sendMessageProcessed(true);
		break;
	case OblakMsg::eOblakMsg_ChangeFlasherParameter:
		qInfo() << "Worker::onReceivedDatagram() - Change Flasher Parameter (Confirm SetFlasher)";
		server->sendMessageProcessed(true);
		break;
	case OblakMsg::eLastMessageType:
		server->sendMessageProcessed(false);
		break;
	default:
		qWarning() << "Worker::onReceivedDatagram() - Undefined message received";
		server->sendMessageProcessed(false);
		break;
	}
}

QString Worker::getDisplayMessage(const QByteArray& datagram)
{
	// We get all bytes that are between the 58th and the last two bytes
	// Additionali we convert all 0x00 bytes to new line characters so that 
	// everyhing is displayed correctly

	int length = datagram.length() - textPosition - 2;

	if (length > 0)
	{
		QByteArray displayMessage = datagram.mid(textPosition, length);
		displayMessage = displayMessage.replace('\x00', '\n');

		qDebug() << "Worker::getDisplayMessage() - [" << QString::fromLatin1(displayMessage) <<"]";

		server->sendMessageProcessed(true);

		return QString::fromLatin1(displayMessage);
	}
	else
	{
		qDebug() << "Worker::getDisplayMessage() - Invalid range or length of display message";

		server->sendMessageProcessed(false);

		return QString::fromLatin1(QByteArray('\xFF', 2)); //Set this as error array
	}
}

QString Worker::getTrafficLightSymbol(const QByteArray& datagram)
{
	// The 17th byte defines the image to be displayed

	unsigned char symbol = datagram.at(imagePosition);

	switch (symbol)
	{
	case OblakMsg::eImage_Red:
		qDebug() << "Worker::getTrafficLightSymbol() - eImage_Red";
		server->sendMessageProcessed(true);
		return "red";
		break;
	case OblakMsg::eImage_RedCross:
		qDebug() << "Worker::getTrafficLightSymbol() - eImage_RedCross";
		server->sendMessageProcessed(true);
		return "red";
		break;
	case OblakMsg::eImage_Green:
		qDebug() << "Worker::getTrafficLightSymbol() - eImage_Green";
		server->sendMessageProcessed(true);
		return "green";
		break;
	case OblakMsg::eImage_GreenArrow:
		qDebug() << "Worker::getTrafficLightSymbol() - eImage_GreenArrow";
		server->sendMessageProcessed(true);
		return "green";
		break;
	case OblakMsg::eLastImage:
		qDebug() << "Worker::getTrafficLightSymbol() - eLastImage";
		server->sendMessageProcessed(true);
		return "red";
		break;
	default:
		qDebug() << "Worker::getTrafficLightSymbol() - default";
		server->sendMessageProcessed(false);
		return "red";
		break;
	}
}

void Worker::processFlasherCommand(const QByteArray& datagram)
{
	if (datagram.at(19) == '\x01')
	{
		qInfo() << "Worker::processFlasherCommand() - Flasher turned ON";
	}
	else
	{
		qInfo() << "Worker::processFlasherCommand() - Flasher turned OFF";
	}
}