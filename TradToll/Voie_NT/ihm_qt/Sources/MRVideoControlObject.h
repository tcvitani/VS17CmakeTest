#ifndef MRVideoControlObject_H
#define MRVideoControlObject_H

#include <qobject.h>
#include <qstring.h>

class MRVideoMsg;
class MRVideoCmdThread;

#define MAX_RVIDEO_SOURCES 10

typedef struct
{
	bool bValidSource;
	QString sStreamSourceUrlLocal;
	QString sStreamSourceUrlRemote;
	QString sBaseUrl;
	QString sUser;
	QString sPwd;
	
	QString sHost;
	quint16 uiPort;
	QString sRestreamServerStreamId;
}
struct_rvideo_sources;


class MRVideoControlObject:public QObject
{
    Q_OBJECT
public:
	MRVideoControlObject(int eTargetVirtObj, QString sTargetRVideoObjId);
	~MRVideoControlObject();
	
	int getVirtObject(){return m_eTargetVirtObject;};
	QString getActivatedStreamSourceUrlLocal();
	QString getActivatedStreamSourceUrlRemote();
	QString getActivatedStreamBaseUrl(); //used for the web interface for ActiveX control install cab file path

	bool init(QString sRSourcesRegKey);

	void activate(int iSource);
	void reactivate();
	void deactivate();
	void grabb();
	void save(QString sFilePath, bool bUseJPG);

signals:
	void newOutputRVideoMessage(MRVideoMsg*pMsg, int eTargetVirtObject);
private slots:
	void onOutputRVideoMessage(MRVideoMsg* pRVMsg);

private:
	void initRVideoCtrlTread();

	MRVideoCmdThread * m_pCmdThread;

	int m_eTargetVirtObject;
	QString m_sTargetRVideoObjId;
	bool m_bInitialized;

	struct_rvideo_sources m_aRVideoSourceParameters[MAX_RVIDEO_SOURCES];
	int m_iActivatedSource;

	bool m_bTraceAll;
};


#endif


