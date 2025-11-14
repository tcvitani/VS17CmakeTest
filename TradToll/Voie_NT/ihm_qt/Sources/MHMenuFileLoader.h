
#ifndef MHMENU_FILE_LOADER_H
#define MHMENU_FILE_LOADER_H

#include <QWidget>
#include <QDomDocument>
#include <MMenuModelItem.h>


class MHMenuFileLoader
{

public:
    MHMenuFileLoader();
	~MHMenuFileLoader();

	bool loadDataFromFile(QString sDataFilePath, bool& bReload);
	bool loadDataFromBuffer(QString sData, bool& bReload);

	QList <MMenuModelItem*> * getLoadedModel(){return &m_lstMenuItems;};

	QString getLastErrorMessage(){return m_sLastError;};
private:
	void clearData();
	QDomDocument m_xmlData;
	QList <MMenuModelItem*> m_lstMenuItems;

	bool OpenXMLFile(QString sDataFilePath);
	bool OpenXMLBuffer(QString sData);

	bool safeReadModelFromXML(bool& bReload);
	bool readModelFromXML(bool bReload);
	MMenuModelItem * recreateMenuItem(QDomNode &currItem, MMenuModelItem * parent);
	bool updateMenuItem(QDomNode currItem, MMenuModelItem * currentItem);
	
	QString m_sLastError;
};






#endif

