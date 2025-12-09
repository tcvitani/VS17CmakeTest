#ifndef OBLAK_MSG_H
#define OBLAK_MSG_H

#include <QByteArray>
#include <QString>

#pragma once

#ifdef DMV_OBLAK_MSG_LIB_EXPORT
#define OBLAK_MSG_EXPORT __declspec(dllexport)
#else
#define OBLAK_MSG_EXPORT  __declspec(dllimport)
#endif

class OBLAK_MSG_EXPORT OblakDevice
{
public:
	OblakDevice(){};
	~OblakDevice(){};

	enum eOblakDeviceType {
		eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_12x12, // Device with RGB matrix field, resolution (VxH) 32x64 pixels and RGB matrix traffic light on the top, resolution 12x12 pixels.
		eOblakDevice_Canopy_TRS_CA60, // Device shows 2 symbols: red cross and green arrow.
		eOblakDevice_Canopy_TRS_PT80_CMC_ENP, // Device shows 3 symbols: red cross, man and ENP.
		eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_Circle, // Device with RGB matrix field, resolution (VxH) 32x64 pixels and RGB matrix traffic light on the top in circle shape.

		eLastOblakDevice
	};

	void setDeviceType(eOblakDeviceType eDeviceType) { m_eUsedDevice = eDeviceType; }
	eOblakDeviceType getDeviceType() { return m_eUsedDevice; }
	
	static QString getDeviceTypeString(eOblakDeviceType eDeviceType);

protected:
	eOblakDeviceType m_eUsedDevice;
};

class OBLAK_MSG_EXPORT OblakMsg : public OblakDevice
{
public:
	OblakMsg(){};
	virtual ~OblakMsg(){};

	enum eMessageType {
		eOblakMsg_ActivateScenario = 0,
		eOblakMsg_SetBrightness,
		eOblakMsg_TimerReset,
		eOblakMsg_SetImage,
		eOblakMsg_SetTextProperties,
		eOblakMsg_SetTextMessage,
		eOblakMsg_SetPropertiesAndText,
		eOblakMsg_SetSymbol,
		eOblakMsg_SetFlasher,
		eOblakMsg_ChangeFlasherParameter,
		eOblakMsg_Response,

		eLastMessageType
	};

	enum eOblakFont {
		eFont_IsoLatin2 = 0x00,
		eFont_IsoCyrillic = 0x01,

		eLastFon,
	};

	enum eOblakFontColor {
		eColor_Off = 0x00,
		eColor_Red = 0x01,
		eColor_Green = 0x02,
		eColor_Blue = 0x03,
		eColor_White = 0x04,
		eColor_Yellow = 0x05,
		eColor_Orange = 0x06,

		eLastColor
	};

	enum eOblakAlignment {
		eAlignLeft = 0x00,
		eAlignRight = 0x01,
		eAlignCenter = 0x02,

		eLastAlign
	};

	enum eOblakImage {
		eImage_Green = 0x00,
		eImage_GreenArrow = 0x01,
		eImage_Red = 0x02,
		eImage_RedCross = 0x03,

		eLastImage
	};

	enum eOblakSymbol{
		eSymbol_NoSymbol = 0x00,
		eSymbol_RedCross = 0x01,
		eSymbol_GreenArrow = 0x02,
		eSymbol_TollCollector = 0x02,
		eSymbol_ENP = 0x03,

		eLastSymbol
	};

	enum eLineNumber {
		eOblak_NoLines,

		eOblak_Line1,
		eOblak_Line2,
		eOblak_Line3,
		eOblak_Line4,

		eOblak_LastLine = eOblak_Line4,

		eOblak_LineEnd
	};

	enum eFlasherState {
		eFlasher_OFF = 0x00, 
		eFlasher_ON = 0x01,

		eLastFlasher
	};

	virtual eMessageType getType() = 0;
	virtual bool isRequest() = 0;

	static QString getMsgTypeDescription(eMessageType eMsgType);

	static OblakMsg *createMessageByType(eMessageType eMsgType);
	static void deleteOblakMsg(OblakMsg * pMsg);

protected:
	
};

class OBLAK_MSG_EXPORT OblakReq : public OblakMsg
{
public:
	OblakReq(){};
	virtual ~OblakReq()
	{
		m_baDynamicData.squeeze();
	};

	virtual bool isRequest() { return true; };
	virtual bool isAbsoluteCommand() = 0;
	virtual QByteArray getHeader() = 0;
	virtual unsigned char getConstant() = 0;
	virtual unsigned char getCommandCode() = 0;
	virtual void encode(QByteArray &baPacket) = 0;
	virtual void encodeDynamicData() = 0;
	
	void setDynamicData(const QByteArray& baDynamicData) { m_baDynamicData = baDynamicData; }
	QByteArray& getDynamicData() { return m_baDynamicData; }

protected:
	void encodeGeneric(QByteArray &baPacket);
	void calculateAndAppendChecksum(QByteArray& baMessage);

	QByteArray m_baDynamicData;
};

class OBLAK_MSG_EXPORT OblakRsp : public OblakMsg
{
public:
	OblakRsp(){};
	virtual ~OblakRsp(){};

	enum eRspCode {
		eRspCode_NoError = 0x00,
		eRspCode_WrongPacketBodyLength = 0x03,
		eRspCode_WrongChecksum = 0x04,
		eRspCode_ErrorInAbsouluteCommand = 0x1A,
		eRspCode_InvalidData = 0xFF,

		eLastRspCode
	};

	virtual bool isRequest() { return false; };
	virtual bool decode(const QByteArray& baPacket) = 0;
	virtual bool validateData() = 0;

	void setResponseCode(eRspCode eResult) { m_eRspCode = eResult; }
	eRspCode getResponseCode() { return m_eRspCode; }

protected:
	eRspCode m_eRspCode;
};

class OBLAK_MSG_EXPORT SetBrightness : virtual public OblakReq
{
public:
	SetBrightness(){};
	~SetBrightness(){};

	virtual eMessageType getType() { return eOblakMsg_SetBrightness; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x02", 12); };
	virtual unsigned char getConstant() { return 0xFF; };
	virtual unsigned char getCommandCode() { return 0x08; };
	virtual bool isAbsoluteCommand() { return false; };
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();

	void setBrightness(unsigned char ucBrighness) { m_ucBrighness = ucBrighness; }
	unsigned char getBrightness() { return m_ucBrighness; }

protected:
	unsigned char m_ucBrighness;
};

class OBLAK_MSG_EXPORT ActivateScenario : virtual public OblakReq
{
public:
	ActivateScenario(){};
	~ActivateScenario(){};

	virtual eMessageType getType() { return eOblakMsg_ActivateScenario; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x04", 12); };
	virtual unsigned char getConstant() { return 0x06; };
	virtual unsigned char getCommandCode() { return 0x0D; };
	virtual bool isAbsoluteCommand() { return false; };
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();

	void setScenarioIndex(const QByteArray& baScenarioIndex) { m_baScenarioIndex = baScenarioIndex; }
	QByteArray& getScenarioIndex() { return m_baScenarioIndex; }

protected:
	QByteArray m_baScenarioIndex;
};

class OBLAK_MSG_EXPORT CommunicationTimerReset : virtual public OblakReq
{
public:
	CommunicationTimerReset(){};
	~CommunicationTimerReset(){};

	virtual eMessageType getType() { return eOblakMsg_TimerReset; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x01", 12); };
	virtual unsigned char getConstant() { return 0xFF; };
	virtual unsigned char getCommandCode() { return 0x09; };
	virtual bool isAbsoluteCommand() { return false; };
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();
};

class OBLAK_MSG_EXPORT SetTextProperties : virtual public OblakReq
{
public:
	SetTextProperties(){};
	~SetTextProperties(){};

	virtual eMessageType getType() { return eOblakMsg_SetTextProperties; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0D", 10); };
	virtual unsigned char getConstant() { return 0x02; };
	virtual unsigned char getCommandCode() { return 0x11; };
	virtual bool isAbsoluteCommand() { return true; }
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();

	void setFont(eOblakFont eFont) { m_eFont = eFont; }
	eOblakFont getFont() { return m_eFont; }

	void setFontColor(eOblakFontColor eFontColor) { m_eFontColor = eFontColor; }
	eOblakFontColor getFontColor() { return m_eFontColor; }

	void setAlignment(eOblakAlignment eAlignment) { m_eAlignment = eAlignment; }
	eOblakAlignment getAlignment() { return m_eAlignment; }

	void setMaxSpacing(unsigned char ucMaxSpacing) { m_ucMaxSpacing = ucMaxSpacing; }
	unsigned char getMaxSpacing() { return m_ucMaxSpacing; }

	void setYCoordinate(const QByteArray& baYCoodrinate) { m_baYCoordinate = baYCoodrinate; }
	QByteArray& getYCoordinate() { return m_baYCoordinate; }

	void setXCoordinateLeftSide(const QByteArray& baXCoodrinateLeftSide) { m_baXCoordinateLeftSide = baXCoodrinateLeftSide; }
	QByteArray& getXCoordinateLeftSide() { return m_baXCoordinateLeftSide; }

	void setXCoordinateRighttSide(const QByteArray& baXCoodrinateRightSide) { m_baXCoordinateRightSide = baXCoodrinateRightSide; }
	QByteArray& getXCoordinateRighttSide() { return m_baXCoordinateRightSide; }

protected:
	eOblakFont		m_eFont;
	eOblakFontColor m_eFontColor;
	eOblakAlignment m_eAlignment;

	unsigned char m_ucMaxSpacing;
	QByteArray    m_baYCoordinate;
	QByteArray	  m_baXCoordinateLeftSide;
	QByteArray	  m_baXCoordinateRightSide;
};

class OBLAK_MSG_EXPORT SetTextMessage : virtual public OblakReq
{
public:
	SetTextMessage(){};
	~SetTextMessage(){};

	virtual eMessageType getType() { return eOblakMsg_SetTextMessage; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0D", 10); };
	virtual unsigned char getConstant() { return 0x02; };
	virtual unsigned char getCommandCode() { return 0x25; };
	virtual bool isAbsoluteCommand() { return true; }
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();

	static unsigned char ucTextLineEnd;

	void setLine(const QByteArray& baLineText, eLineNumber eLine);

	void setLine1(const QByteArray& baLineText) { m_baLine1 = baLineText;  m_bLine1Set = true; }
	QByteArray& getLine1() { return m_baLine1; }
	bool isLine1Set() { return m_bLine1Set; }

	void setLine2(const QByteArray& baLineText) { m_baLine2 = baLineText; m_bLine2Set = true; }
	QByteArray& getLine2() { return m_baLine2; }
	bool isLine2Set() { return m_bLine2Set; }

	void setLine3(const QByteArray& baLineText) { m_baLine3 = baLineText; m_bLine3Set = true; }
	QByteArray& getLine3() { return m_baLine3; }
	bool isLine3Set() { return m_bLine3Set; }

	void setLine4(const QByteArray& baLineText) { m_baLine4 = baLineText; m_bLine4Set = true; }
	QByteArray& getLine4() { return m_baLine4; }
	bool isLine4Set() { return m_bLine4Set; }

protected:
	QByteArray m_baLine1;
	bool	   m_bLine1Set;
	QByteArray m_baLine2;
	bool	   m_bLine2Set;
	QByteArray m_baLine3;
	bool	   m_bLine3Set;
	QByteArray m_baLine4;
	bool	   m_bLine4Set;
};

class OBLAK_MSG_EXPORT SetPropertiesAndText : public SetTextProperties, public SetTextMessage
{
public:
	SetPropertiesAndText(){};
	~SetPropertiesAndText(){};

	virtual eMessageType getType() { return eOblakMsg_SetTextMessage; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0D", 10); };
	virtual unsigned char getConstant() { return 0x02; }; // Uses combination of SetTextProperties and SetTextMessage
	virtual unsigned char getCommandCode() { return 0xFF; }; // Uses combination of SetTextProperties and SetTextMessage
	virtual bool isAbsoluteCommand() { return true; }
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();

	// Constants
	static const QByteArray baYAxisLine1;
	static const QByteArray baYAxisLine2;
	static const QByteArray baYAxisLine3;
	static const QByteArray baYAxisLine4;
	static const QByteArray baXLAxis;
	static const QByteArray baXRAxis;
	static unsigned char ucTextLineEnd;

	void setPropertiesArray();
	QByteArray& getPropertiesArray() { return m_baGenericTextProperties; }

	void setPropertiesObject(SetTextProperties *pProperties) { m_pProperties = pProperties; }
	SetTextProperties *getPropertiesObject() { return m_pProperties; }

protected:
	QByteArray m_baGenericTextProperties;

	SetTextProperties *m_pProperties;
};

class OBLAK_MSG_EXPORT SetImage : virtual public OblakReq
{
public:
	SetImage(){};
	~SetImage(){};

	virtual eMessageType getType() { return eOblakMsg_SetPropertiesAndText; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0D", 10); };
	virtual unsigned char getConstant() { return 0x02; };
	virtual unsigned char getCommandCode() { return 0x0C; };
	virtual bool isAbsoluteCommand() { return true; }
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();

	void setImage(eOblakImage eImage) { m_eImage = eImage; }
	eOblakImage getImage() { return m_eImage; }

	void setXCoordinate(const QByteArray& baXCoodrinate) { m_baXCoordinate = baXCoodrinate; }
	QByteArray& getXCoordinate() { return m_baXCoordinate; }

	void setYCoordinate(const QByteArray& baYCoodrinate) { m_baYCoordinate = baYCoodrinate; }
	QByteArray& getYCoordinate() { return m_baYCoordinate; }

protected:
	eOblakImage m_eImage;
	QByteArray	m_baXCoordinate;
	QByteArray	m_baYCoordinate;
};

class OBLAK_MSG_EXPORT SetSymbol : virtual public OblakReq
{
public:
	SetSymbol(){ m_bTwoSymbols = false; };
	~SetSymbol(){};

	virtual eMessageType getType() { return eOblakMsg_SetSymbol; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0D", 10); };
	virtual unsigned char getConstant() { return 0x02; };
	virtual unsigned char getCommandCode() { return 0x1A; };
	virtual bool isAbsoluteCommand() { return true; }
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();

	// Constant
	static unsigned char ucNoBlink;
	static unsigned char ucBlink;

	void setSymbol(eOblakSymbol eSymbol) { m_eSymbol = eSymbol; }
	eOblakSymbol getSymbol() { return m_eSymbol; }
	
	void setSecondSymbol(eOblakSymbol eSecondSymbol) { m_eSecondSymbol = eSecondSymbol; m_bTwoSymbols = true; }
	eOblakSymbol getSecondSymbol() { return m_eSecondSymbol; }

	void setSymbolBlink(unsigned char ucIsBlinking) { m_ucIsBlinking = ucIsBlinking; }
	unsigned char getIsSymbolBlinking() { return m_ucIsBlinking; }

	bool checkSymbolValidityForDevice(eOblakDeviceType eDevice, eOblakSymbol eSymbol);
	void correctSymbolForDevice(eOblakDeviceType eDevice, eOblakSymbol eSymbol);
protected:

	eOblakSymbol m_eSymbol;
	eOblakSymbol m_eSecondSymbol;
	bool m_bTwoSymbols;
	unsigned char m_ucIsBlinking;
};

class OBLAK_MSG_EXPORT SetFlasher : virtual public OblakReq
{
public:
	SetFlasher(){};
	~SetFlasher(){};

	virtual eMessageType getType() { return eOblakMsg_SetFlasher; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00", 9); };
	virtual unsigned char getConstant() { return 0xFF; };
	virtual unsigned char getCommandCode() { return 0x07; };
	virtual bool isAbsoluteCommand() { return false; };
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();

	void setFlasherState(eFlasherState eState) { m_eFlasherState = eState; }
	unsigned char getFlasherState() { return m_eFlasherState; }

	void setDisplayType(eOblakDeviceType eType) { m_eDisplayType = eType; }
	eOblakDeviceType getDisplayType() { return m_eDisplayType; }

	static const QByteArray baDynamicPanelsMemoryIndexOldModel;
	static const QByteArray baDynamicPanelsMemoryIndexNewModel;
	static const QByteArray baTableEntryPositionOldModel;
	static const QByteArray baTableEntryPositionNewModel;
	static const QByteArray baNumberOfBytesSet;
	static unsigned char ucPartialTableSet;

protected:
	eFlasherState m_eFlasherState;
	eOblakDeviceType m_eDisplayType;
	QByteArray m_baDynamicPanelMemoryIndex;
	QByteArray m_baTableEntryPosition;
};

class OBLAK_MSG_EXPORT ChangeFlasherParameter : virtual public OblakReq
{
public:
	ChangeFlasherParameter(){};
	~ChangeFlasherParameter(){};

	virtual eMessageType getType() { return eOblakMsg_ChangeFlasherParameter; }
	virtual QByteArray getHeader() { return QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00\x00", 9); };
	virtual unsigned char getConstant() { return 0xFF; };
	virtual unsigned char getCommandCode() { return 0x07; };
	virtual bool isAbsoluteCommand() { return false; };
	virtual void encode(QByteArray &baPacket);
	virtual void encodeDynamicData();

	void setDisplayType(eOblakDeviceType eType) { m_eDisplayType = eType; }
	eOblakDeviceType getDisplayType() { return m_eDisplayType; }

	static const QByteArray baPacketLength; // This is constant only for this message
	static const QByteArray baChangeParameterIndexOldModel;
	static const QByteArray baChangeParameterIndexNewModel;

protected:
	eOblakDeviceType m_eDisplayType;
	QByteArray m_baChangeParameterIndex;
};

class OBLAK_MSG_EXPORT OblakResponse : virtual public OblakRsp
{
public:
	OblakResponse(){};
	~OblakResponse(){};

	virtual eMessageType getType() { return eOblakMsg_Response; }
	virtual bool decode(const QByteArray& baPacket);
	virtual bool validateData();

	eRspCode AnalyzeOblakResponse(const QByteArray& baResponse);

	void setErrorCode(unsigned char ucErrorCode) { m_ucErrorCode = ucErrorCode; }
	unsigned char getErrorCode() { return m_ucErrorCode; }

	void setErrorDescription(const QByteArray& baErrorDescription) { m_baErrorDescription = baErrorDescription; }
	QByteArray& getErrorDescription() { return m_baErrorDescription; }

	void setErrorString(const QString baErrorString) { m_sErrorString = baErrorString; }
	QString getErrorString(){ return m_sErrorString; }

	void setFixedBytes(const QByteArray& baFixedBytes) { m_baFixedBytes = baFixedBytes; }
	QByteArray& getFixedBytes() { return m_baFixedBytes; }

protected:
	void decodeErrorDescription();

	unsigned char m_ucErrorCode;
	QByteArray m_baErrorDescription;
	QString m_sErrorString;

	QByteArray m_baFixedBytes;
};

#endif