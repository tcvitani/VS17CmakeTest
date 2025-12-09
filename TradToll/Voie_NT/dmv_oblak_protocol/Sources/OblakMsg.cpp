#include <OblakMsg.h>
#include <HelpFuncs.h>

const unsigned char ucNotUsed = 0xFF;
const int g_iErrCodePosition = 12;
const int g_iErrDescriptionPosition = 13;
const int g_iErrDescriptionLen = 4;

const QByteArray SetPropertiesAndText::baYAxisLine1("\x00\x00", 2);
const QByteArray SetPropertiesAndText::baYAxisLine2("\x00\x08", 2);
const QByteArray SetPropertiesAndText::baYAxisLine3("\x00\x10", 2);
const QByteArray SetPropertiesAndText::baYAxisLine4("\x00\x18", 2);
const QByteArray SetPropertiesAndText::baXLAxis("\x00\x00", 2);
const QByteArray SetPropertiesAndText::baXRAxis("\x00\x3F", 2);
unsigned char SetPropertiesAndText::ucTextLineEnd = '\x00';

unsigned char SetTextMessage::ucTextLineEnd = '\x00';

unsigned char SetSymbol::ucNoBlink = '\x00';
unsigned char SetSymbol::ucBlink = '\x01';

const QByteArray SetFlasher::baDynamicPanelsMemoryIndexOldModel("\xFE\x71", 2);
const QByteArray SetFlasher::baDynamicPanelsMemoryIndexNewModel("\xFE\x75", 2);
const QByteArray SetFlasher::baTableEntryPositionOldModel("\x00\x33", 2);
const QByteArray SetFlasher::baTableEntryPositionNewModel("\x00\x35", 2);
const QByteArray SetFlasher::baNumberOfBytesSet("\x00\x01", 2);
unsigned char SetFlasher::ucPartialTableSet = '\x03';

const QByteArray ChangeFlasherParameter::baChangeParameterIndexOldModel("\x6E\x01", 2);
const QByteArray ChangeFlasherParameter::baChangeParameterIndexNewModel("\x72\x01", 2);
const QByteArray ChangeFlasherParameter::baPacketLength("\x00\x03", 2);

QString OblakDevice::getDeviceTypeString(eOblakDeviceType eDeviceType)
{
	switch (eDeviceType)
	{
	case eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_12x12:		return "eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_12x12";
	case eOblakDevice_Canopy_TRS_CA60:									return "eOblakDevice_Canopy_TRS_CA60";
	case eOblakDevice_Canopy_TRS_PT80_CMC_ENP:							return "eOblakDevice_Canopy_TRS_PT80_CMC_ENP";
	case eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_Circle:		return "eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_Circle";

	default:
		return "UnknownType";
	}
}

QString OblakMsg::getMsgTypeDescription(eMessageType eMsgType)
{
	switch (eMsgType)
	{
		case eOblakMsg_ActivateScenario:		return "eOblakMsg_ActivateScenario";
		case eOblakMsg_SetBrightness:			return "eOblakMsg_SetBrightness";
		case eOblakMsg_TimerReset:				return "eOblakMsg_TimerReset";
		case eOblakMsg_SetTextProperties:		return "eOblakMsg_SetTextProperties";
		case eOblakMsg_SetTextMessage:			return "eOblakMsg_SetTextMessage";
		case eOblakMsg_SetPropertiesAndText:	return "eOblakMsg_SetPropertiesAndText";
		case eOblakMsg_SetImage:				return "eOblakMsg_SetImage";
		case eOblakMsg_SetSymbol:				return "eOblakMsg_SetSymbol";
		case eOblakMsg_Response:				return "eOblakMsg_Response";

		default:
			return "UnknownType";
	}
}

OblakMsg * OblakMsg::createMessageByType(eMessageType eMsgType)
{
	switch (eMsgType)
	{
	case eOblakMsg_ActivateScenario:		return new ActivateScenario();
	case eOblakMsg_SetBrightness:			return new SetBrightness();
	case eOblakMsg_TimerReset:				return new CommunicationTimerReset();
	case eOblakMsg_SetTextProperties:		return new SetTextProperties();
	case eOblakMsg_SetTextMessage:			return new SetTextMessage();
	case eOblakMsg_SetPropertiesAndText:	return new SetPropertiesAndText();
	case eOblakMsg_SetImage:				return new SetImage(); 
	case eOblakMsg_SetSymbol:				return new SetSymbol();
	case eOblakMsg_Response:				return new OblakResponse();

	default:
		return nullptr;
	}
}

void OblakMsg::deleteOblakMsg(OblakMsg * pMsg)
{
	if (pMsg != nullptr)
	{
		delete pMsg;
	}
}

void OblakReq::encodeGeneric(QByteArray &baPacket)
{
	QByteArray body;
	int iPacketBodyLenPosition = 0;
	int iPacketBodyLen = 0;
	int iPacketLenPosition = 0;
	int iPacketLen = 0;
	QByteArray baPacketLen;
	QByteArray baPacketBodyLen;

	baPacket.clear();

	baPacket.append(getHeader());

	if (isAbsoluteCommand())
	{
		iPacketBodyLenPosition = baPacket.length();
	}

	if (getConstant() != ucNotUsed) // Some messages do not have constants in request, there is no rule to it
	{
		baPacket.append(getConstant());
	}
	
	baPacket.append(getCommandCode());

	if (isAbsoluteCommand())
	{
		iPacketLenPosition = baPacket.length();
		iPacketLen = m_baDynamicData.length();
	}
	
	if (!m_baDynamicData.isEmpty())
	{
		baPacket.append(m_baDynamicData);
	}

	if (isAbsoluteCommand())
	{
		baPacketLen = baPacketLen.append(static_cast<unsigned char>(iPacketLen));
		baPacket.insert(iPacketLenPosition, baPacketLen);
		
		for (QByteArray::iterator it = baPacket.begin() + iPacketBodyLenPosition; it != baPacket.end(); ++it)
		{
			++iPacketBodyLen;
		}

		baPacketBodyLen = baPacketBodyLen.append(static_cast<unsigned char>(iPacketBodyLen)).rightJustified(2, '\x00');
		baPacket.insert(iPacketBodyLenPosition, baPacketBodyLen);
	}

	calculateAndAppendChecksum(baPacket);
}

void OblakReq::calculateAndAppendChecksum(QByteArray& baMessage)
{
	QString sChecksum;
	QByteArray baChecksum;
	int checksum = 0;
	
	for (char byte : baMessage)
	{
		checksum += static_cast<unsigned char>(byte);
	}
	
	sChecksum = QString::number(checksum, 16);
	baChecksum = QByteArray::fromHex(sChecksum.toUtf8()).rightJustified(2, '\x00');
	
	baMessage.append(baChecksum);
}

void SetBrightness::encode(QByteArray &baPacket)
{
	SetBrightness::encodeDynamicData();
	encodeGeneric(baPacket);
}

void SetBrightness::encodeDynamicData()
{
	m_baDynamicData.clear();
	m_baDynamicData.append(SetBrightness::getBrightness());
}

void ActivateScenario::encode(QByteArray &baPacket)
{
	ActivateScenario::encodeDynamicData();
	encodeGeneric(baPacket);
}

void ActivateScenario::encodeDynamicData()
{
	m_baDynamicData.clear();
	m_baDynamicData.append(ActivateScenario::getScenarioIndex());
}

void CommunicationTimerReset::encode(QByteArray &baPacket)
{
	CommunicationTimerReset::encodeDynamicData();
	encodeGeneric(baPacket);
}

void CommunicationTimerReset::encodeDynamicData()
{
	m_baDynamicData.clear();
}

void SetTextProperties::encode(QByteArray &baPacket)
{
	SetTextProperties::encodeDynamicData();
	encodeGeneric(baPacket);
}

void SetTextProperties::encodeDynamicData()
{
	m_baDynamicData.clear();

	m_baDynamicData.append(SetTextProperties::getFont());
	m_baDynamicData.append(SetTextProperties::getFontColor());
	m_baDynamicData.append(SetTextProperties::getAlignment());
	m_baDynamicData.append(SetTextProperties::getMaxSpacing());
	m_baDynamicData.append(SetTextProperties::getYCoordinate().rightJustified(2, '\x00'));
	m_baDynamicData.append(SetTextProperties::getXCoordinateLeftSide().rightJustified(2, '\x00'));
	m_baDynamicData.append(SetTextProperties::getXCoordinateRighttSide().rightJustified(2, '\x00'));
}

void SetTextMessage::encode(QByteArray &baPacket)
{
	SetTextMessage::encodeDynamicData();
	encodeGeneric(baPacket);
}

void SetTextMessage::encodeDynamicData()
{
	m_baDynamicData.clear();

	if (isLine1Set())
	{
		m_baDynamicData.append(SetTextMessage::getLine1());
		m_baDynamicData.append(SetTextMessage::ucTextLineEnd);
	}
	if (isLine2Set())
	{
		m_baDynamicData.append(SetTextMessage::getLine2());
		m_baDynamicData.append(SetTextMessage::ucTextLineEnd);
	}
	if (isLine3Set())
	{
		m_baDynamicData.append(SetTextMessage::getLine3());
		m_baDynamicData.append(SetTextMessage::ucTextLineEnd);
	}
	if (isLine4Set())
	{
		m_baDynamicData.append(SetTextMessage::getLine4());
		m_baDynamicData.append(SetTextMessage::ucTextLineEnd);
	}
}

void SetTextMessage::setLine(const QByteArray& baLineText, eLineNumber eLine)
{
	switch (eLine)
	{
	case eOblak_Line1:
		setLine1(baLineText);
		break;
	case eOblak_Line2:
		setLine2(baLineText);
		break;
	case eOblak_Line3:
		setLine3(baLineText);
		break;
	case eOblak_Line4:
		setLine4(baLineText);
		break;
	}
}

void SetPropertiesAndText::encode(QByteArray &baPacket)
{
	int iPacketBodyLenPosition = 0;
	int iPacketBodyLen = 0;
	int iPacketLenPosition = 0;
	int iPacketLen = 0;
	QByteArray baPacketLen;
	QByteArray baPacketBodyLen;

	baPacket.clear();

	baPacket.append(getHeader());

	iPacketBodyLenPosition = baPacket.length();

	baPacket.append(getConstant());

	// Text Properties for each line
	baPacket.append(SetTextProperties::getCommandCode());

	iPacketLenPosition = baPacket.length();

	if (isLine1Set())
	{
		baPacket.append(getPropertiesArray());
		baPacket.append(SetPropertiesAndText::baYAxisLine1);
		baPacket.append(SetPropertiesAndText::baXLAxis);
		baPacket.append(SetPropertiesAndText::baXRAxis);
	}

	if (isLine2Set())
	{
		baPacket.append(getPropertiesArray());
		baPacket.append(SetPropertiesAndText::baYAxisLine2);
		baPacket.append(SetPropertiesAndText::baXLAxis);
		baPacket.append(SetPropertiesAndText::baXRAxis);
	}

	if (isLine3Set())
	{
		baPacket.append(getPropertiesArray());
		baPacket.append(SetPropertiesAndText::baYAxisLine3);
		baPacket.append(SetPropertiesAndText::baXLAxis);
		baPacket.append(SetPropertiesAndText::baXRAxis);
	}

	if (isLine4Set())
	{
		baPacket.append(getPropertiesArray());
		baPacket.append(SetPropertiesAndText::baYAxisLine4);
		baPacket.append(SetPropertiesAndText::baXLAxis);
		baPacket.append(SetPropertiesAndText::baXRAxis);
	}

	for (QByteArray::iterator it = baPacket.begin() + iPacketLenPosition; it != baPacket.end(); ++it)
	{
		++iPacketLen;
	}
	baPacketLen = baPacketLen.append(static_cast<unsigned char>(iPacketLen));
	baPacket.insert(iPacketLenPosition, baPacketLen);
	
	// reset for text packet len
	iPacketLen = 0; 
	baPacketLen.clear();

	// Set Text for each line
	baPacket.append(SetTextMessage::getCommandCode());
	
	iPacketLenPosition = baPacket.length();

	if (isLine1Set())
	{
		baPacket.append(getLine1());
		baPacket.append(SetPropertiesAndText::ucTextLineEnd);
	}

	if (isLine2Set())
	{
		baPacket.append(getLine2());
		baPacket.append(SetPropertiesAndText::ucTextLineEnd);
	}

	if (isLine3Set())
	{
		baPacket.append(getLine3());
		baPacket.append(SetPropertiesAndText::ucTextLineEnd);
	}

	if (isLine4Set())
	{
		baPacket.append(getLine4());
		baPacket.append(SetPropertiesAndText::ucTextLineEnd);
	}

	for (QByteArray::iterator it = baPacket.begin() + iPacketLenPosition; it != baPacket.end(); ++it)
	{
		++iPacketLen;
	}
	baPacketLen = baPacketLen.append(static_cast<unsigned char>(iPacketLen));
	baPacket.insert(iPacketLenPosition, baPacketLen);
	

	for (QByteArray::iterator it = baPacket.begin() + iPacketBodyLenPosition; it != baPacket.end(); ++it)
	{
		++iPacketBodyLen;
	}
	
	baPacketBodyLen = baPacketBodyLen.append(static_cast<unsigned char>(iPacketBodyLen)).rightJustified(2, '\x00');
	baPacket.insert(iPacketBodyLenPosition, baPacketBodyLen);

	calculateAndAppendChecksum(baPacket);
}

void SetPropertiesAndText::encodeDynamicData()
{
	;
}

void SetPropertiesAndText::setPropertiesArray()
{
	m_baGenericTextProperties.clear();
	m_baGenericTextProperties.append(getPropertiesObject()->getFont());
	m_baGenericTextProperties.append(getPropertiesObject()->getFontColor());
	m_baGenericTextProperties.append(getPropertiesObject()->getAlignment());
	m_baGenericTextProperties.append(getPropertiesObject()->getMaxSpacing());
}

void SetImage::encode(QByteArray &baPacket)
{
	SetImage::encodeDynamicData();
	encodeGeneric(baPacket);
}

void SetImage::encodeDynamicData()
{
	QString sImg;

	sImg = QString::number(SetImage::getImage(), 16);
	
	m_baDynamicData.clear();

	m_baDynamicData.append(QByteArray::fromHex(sImg.toUtf8()).rightJustified(2, '\x00'));
	m_baDynamicData.append(SetImage::getXCoordinate().rightJustified(2, '\x00'));
	m_baDynamicData.append(SetImage::getYCoordinate().rightJustified(2, '\x00'));
}

void SetSymbol::encode(QByteArray &baPacket)
{
	SetSymbol::encodeDynamicData();
	encodeGeneric(baPacket);
}

void SetSymbol::encodeDynamicData()
{
	m_baDynamicData.clear();

	m_baDynamicData.append(SetSymbol::getSymbol());
	m_baDynamicData.append(SetSymbol::getIsSymbolBlinking());

	if (m_bTwoSymbols)
	{
		m_baDynamicData.append(SetSymbol::getSecondSymbol());
		m_baDynamicData.append(SetSymbol::getIsSymbolBlinking());
	}
}

bool SetSymbol::checkSymbolValidityForDevice(eOblakDeviceType eDevice, eOblakSymbol eSymbol)
{
	if (eDevice == eOblakDevice_Canopy_TRS_CA60)
	{
		if (eSymbol == eSymbol_TollCollector || eSymbol_ENP)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	if (eDevice == eOblakDevice_Canopy_TRS_PT80_CMC_ENP)
	{
		if (eSymbol == eSymbol_GreenArrow)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

void SetSymbol::correctSymbolForDevice(eOblakDeviceType eDevice, eOblakSymbol eSymbol)
{
	if (eDevice == eOblakDevice_Canopy_TRS_CA60)
	{
		if (eSymbol == eSymbol_TollCollector || eSymbol_ENP)
		{
			eSymbol = eSymbol_GreenArrow;
		}
	}

	if (eDevice == eOblakDevice_Canopy_TRS_PT80_CMC_ENP)
	{
		if (eSymbol == eSymbol_GreenArrow)
		{
			eSymbol = eSymbol_TollCollector;
		}
	}
}

// This one is not part of the simplified protocol (absolute command), but another protocol so there is no generic encoding
void SetFlasher::encode(QByteArray &baPacket)
{
	int iPacketBodyLenPosition = 0;
	int iPacketBodyLen = 0;
	QByteArray baPacketBodyLen;

	SetFlasher::encodeDynamicData();
	
	baPacket.append(getHeader());
	baPacket.append(getCommandCode());

	iPacketBodyLenPosition = baPacket.length();

	baPacket.append(m_baDynamicPanelMemoryIndex);
	baPacket.append(ucPartialTableSet);
	baPacket.append(m_baTableEntryPosition);
	baPacket.append(baNumberOfBytesSet);
	baPacket.append(m_baDynamicData); // Flasher ON or OFF (R3 value in memory)
	baPacket.append('\xFF'); // This is end of message byte

	for (QByteArray::iterator it = baPacket.begin() + iPacketBodyLenPosition; it != baPacket.end(); ++it)
	{
		++iPacketBodyLen;
	}

	baPacketBodyLen = baPacketBodyLen.append(static_cast<unsigned char>(iPacketBodyLen)).rightJustified(2, '\x00');
	baPacket.insert(iPacketBodyLenPosition, baPacketBodyLen);

	calculateAndAppendChecksum(baPacket);
}

void SetFlasher::encodeDynamicData()
{
	m_baDynamicData.clear();
	m_baDynamicData.append(SetFlasher::getFlasherState());

	if (m_eDisplayType == OblakDevice::eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_12x12)
	{
		m_baDynamicPanelMemoryIndex = baDynamicPanelsMemoryIndexOldModel;
		m_baTableEntryPosition = baTableEntryPositionOldModel;
	}
	else
	{
		m_baDynamicPanelMemoryIndex = baDynamicPanelsMemoryIndexNewModel;
		m_baTableEntryPosition = baTableEntryPositionNewModel;
	}
}

void ChangeFlasherParameter::encode(QByteArray &baPacket)
{
	ChangeFlasherParameter::encodeDynamicData();

	baPacket.append(getHeader());
	baPacket.append(getCommandCode());

	baPacket.append(baPacketLength);
	baPacket.append(m_baChangeParameterIndex);
	baPacket.append('\xFF'); // This is end of message byte

	calculateAndAppendChecksum(baPacket);
}

void ChangeFlasherParameter::encodeDynamicData()
{
	m_baDynamicData.clear();

	if (m_eDisplayType == OblakDevice::eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_12x12)
	{
		m_baChangeParameterIndex = baChangeParameterIndexOldModel;
	}
	else
	{
		m_baChangeParameterIndex = baChangeParameterIndexNewModel;
	}
}

bool OblakResponse::decode(const QByteArray& baPacket)
{
	unsigned char ucErrorCode= baPacket.at(g_iErrCodePosition);
	setErrorCode(ucErrorCode);

	QByteArray baErrDescription = baPacket.mid(g_iErrDescriptionPosition, g_iErrDescriptionLen);
	setErrorDescription(baErrDescription);

	if (validateData())
	{
		return true;
	}
	else
	{
		OblakRsp::setResponseCode(OblakRsp::eRspCode_InvalidData);
		return false;
	}
}

bool OblakResponse::validateData()
{
	unsigned char ucErrCode = getErrorCode();

	if ((ucErrCode != eRspCode_NoError) &&
		(ucErrCode != eRspCode_WrongChecksum) &&
		(ucErrCode != eRspCode_WrongPacketBodyLength) &&
		(ucErrCode != eRspCode_ErrorInAbsouluteCommand))
	{
		return false;
	}

	if (OblakResponse::getErrorDescription().isEmpty() || OblakResponse::getErrorDescription().isNull())
	{
		return false;
	}

	return true;
}

OblakRsp::eRspCode OblakResponse::AnalyzeOblakResponse(const QByteArray& baResponse)
{
	if (OblakResponse::getErrorCode() == eRspCode_NoError)
	{
		setResponseCode(eRspCode_NoError);
	}
	else
	{
		decodeErrorDescription();
	}

	return getResponseCode();
}

void OblakResponse::decodeErrorDescription()
{
	QString sErrorString;
	eRspCode eResult;
	QByteArray baErrorDescription = getErrorDescription();
	QByteArray baError = baErrorDescription.mid(0, 2);
	QString sError = HelpFuncs::ByteArrayToHexStr(baError);
	QByteArray baFix = baErrorDescription.mid(2, 2);
	QString sFix = HelpFuncs::ByteArrayToHexStr(baFix);

	switch (getErrorCode())
	{
	case eRspCode_WrongPacketBodyLength:
		eResult = eRspCode_WrongPacketBodyLength;
		sErrorString = QString("The request had the wrong packet body length [%1]. The following packet body length is correct [%2]").arg(sError).arg(sFix);
		setFixedBytes(baFix);
		break;
	case eRspCode_WrongChecksum:
		eResult = eRspCode_WrongChecksum;
		sErrorString = QString("The request had the wrong checksum [%1]. The following checksum is correct [%2]").arg(sError).arg(sFix);
		setFixedBytes(baFix);
		break;
	case eRspCode_ErrorInAbsouluteCommand:
		eResult = eRspCode_ErrorInAbsouluteCommand;
		sErrorString = QString("The request had an error in absolute command [%1]. The following command is correct [%2]").arg(sError).arg(sFix);
		setFixedBytes(baFix);
		break;
	default:
		eResult = eRspCode_InvalidData;
		sErrorString = QString("Unknown error appeared [%1]").arg(sError);
		break;
	}

	setResponseCode(eResult);
	setErrorString(sErrorString);
}