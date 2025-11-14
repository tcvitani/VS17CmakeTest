#ifndef MIHM_ANI_INIT_DATA_H
#define MIHM_ANI_INIT_DATA_H

#include <QObject>
#include <QList>


extern "C" {
	#include <noyau.h>
}

class MIhmAniInitData:public QObject
{
    Q_OBJECT
public:
	MIhmAniInitData();
	~MIhmAniInitData();

	bool initialize(QString szMboxName, QString sConfigKey);
	QString getMboxName();

	noyau_pool_id getPoolId() {return m_iPoolId;};
	QString& getModuleConfigKey() {return m_sConfigKey;};

	DWORD getMaxVideoZoom(){return m_dwVideoZoomNo;};
	QString getCommFileFullPath(QString sDataFile);
	DWORD getMaxTableRowsInMemory(){ return	m_dwMaxTableRowsInMemory;};

	//virtual keyboard parameters...
	bool isVirtualKeyboardEnabled() {return m_bVirtualKeyboardEnabled;};
	QString getVirtualKeyboardMboxName() {return m_sVirtKeybMboxName;};
	
	QString getVirtKeyDefaultCfg(){return m_sVirtKeyDefaultCfg;};
	int getVirtKeyDefaultPosX(){return m_iVirtKeyDefaultPosX;};
	int getVirtKeyDefaultPosY(){return m_iVirtKeyDefaultPosY;};

public slots:

private:
	QString getCleanPath(QString sPath, QString sSubPath);

	noyau_pool_id        m_iPoolId;

	QString m_sMboxName;
	QString m_sConfigKey;
	DWORD m_dwVideoZoomNo;

	QString	m_sCommFilesRoot;
	DWORD	m_dwMaxTableRowsInMemory;

	//virtual keyboard parameters...
	bool m_bVirtualKeyboardEnabled;
	QString m_sVirtKeybMboxName;

	QString m_sVirtKeyDefaultCfg;
	int m_iVirtKeyDefaultPosX;
	int m_iVirtKeyDefaultPosY;

};





#endif


