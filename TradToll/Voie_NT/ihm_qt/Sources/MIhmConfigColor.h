#ifndef MIHM_CONFIG_COLOR_H
#define MIHM_CONFIG_COLOR_H

#include <QString>
#include <QMap>
#include <QColor>

//Default data read from registry
class MIhmConfigColor
{
	public:
		MIhmConfigColor();
		~MIhmConfigColor();
		static QString makeHexColorString(int iColor);
		static QColor makeColorFromHexString(QString sHexColor);
		static QString createColorStyle(QString sHexColour);
		static QString createBkgColorStyle(QString sHexColour);
		
		bool loadConfigFromFile(QString sFilePath);

		bool getColourForShift(QString sShiftType, QString& sShiftColour);

	private:
		
		QMap<QString, QString> m_mapShiftColours;
		QString m_sModuleConfigKey;	
		static const QString cCOLORS_ROOT_ELEMENT; // = "COLORS"
		static const QString cCOLOR_ELEMENT; // = "COLOR"
		static const QString cATTRIBUTE_NAME; // = "NAME"
		static const QString cATTRIBUTE_VALUE; // = "VALUE"
		
};



#endif


