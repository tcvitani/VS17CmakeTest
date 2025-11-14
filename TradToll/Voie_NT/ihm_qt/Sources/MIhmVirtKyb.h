#ifndef MIHM_VIRTKYB_H
#define MIHM_VIRTKYB_H

#include <QObject>

class MIhmAniThread;

extern "C" {
	#include <noyau.h>
	#include <module.h>
}


class MIhmVirtKyb:public QObject
{
    Q_OBJECT
public:
	MIhmVirtKyb();
	~MIhmVirtKyb();

	void Initialize(MIhmAniThread * pAniThread);

	void Stop();
	void Hide();
	void Show(QString sCfg, int X, int Y);

public slots:
	
private:
	void sendInitConfig(QString sKeybID);
	void sendAffMove(int x, int y);
	void sendAffHide();
	void sendAffShow();
	void sendAffDEBUT();
	void sendAffFIN();
	void sendArret();

	bool sendMessage(enum_srv_service service, enum_srv_type eTypeMessage, void *data);

	MIhmAniThread *				m_pAniThread;

	QString m_sCfg;
	int m_X;
	int m_Y;
	
	QString m_sDefaultCfg;
	int m_iDefaultX; 
	int m_iDefaultY;
};


#endif


