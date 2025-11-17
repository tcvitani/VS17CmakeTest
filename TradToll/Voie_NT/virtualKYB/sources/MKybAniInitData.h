#ifndef MKyb_ANI_INIT_DATA_H
#define MKyb_ANI_INIT_DATA_H

#include <QObject>
#include <QList>


extern "C" {
	#include <noyau.h>
}

class MKybAniInitData:public QObject
{
    Q_OBJECT
public:
	MKybAniInitData();
	~MKybAniInitData();

	bool initialize(QString szMboxName, QString sConfigKey);
	QString getMboxName();

	noyau_pool_id getPoolId() {return m_iPoolId;};
	QString& getModuleConfigKey() {return m_sConfigKey;};

public slots:

private:
	noyau_pool_id        m_iPoolId;

	QString m_sMboxName;
	QString m_sConfigKey;
	DWORD m_dwVideoZoomNo;

	QString	m_sCommFilesRoot;
	DWORD	m_dwMaxTableRowsInMemory;

};





#endif


