#ifndef MIHM_CONFIG_CURSORS_H
#define MIHM_CONFIG_CURSORS_H

#include <QCursor>
#include <QString>
#include <QMap>

class MIhmConfigImages;

class MCursor
{
public:
	QString m_sId;
	QString m_sImage;
	int m_iQTCursorShape;
};

//Default data read from file
class MIhmConfigCursors
{
	public:
		MIhmConfigCursors();
		~MIhmConfigCursors();
		
		bool loadConfigFromFile(QString sFilePath);
		QCursor getCursor(MIhmConfigImages *pImages, QString sCursorID);

	private:
		MCursor * findCursor(QString sCursorID);
		QList <MCursor*> m_lstCursors;

		static const QString cCURSORS_ROOT_ELEMENT; 
		static const QString cCURSOR_ELEMENT; 
		static const QString cATTRIBUTE_ID; 
		static const QString cATTRIBUTE_IMAGE; 
		static const QString cATTRIBUTE_QT_CURSOR; 
		
};



#endif


