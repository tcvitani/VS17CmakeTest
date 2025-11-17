#ifndef MKyb_SRV_H
#define MKyb_SRV_H

#include <QObject>


extern "C" {
	#include <noyau.h>
	#include <virt_kyb.h>
}

class MKybMsg;
class MKybAniThread;



class MKybSrv:public QObject
{
    Q_OBJECT
public:
	MKybSrv(MKybAniThread * pAniThread);
	~MKybSrv();

	void KybTreatMessageService( IN struct_VIRT_KYB_message * psNeutre );

	void KybTreatMessageTCL(MKybMsg * pMsg);

	static MKybSrv * getGlobalMSrv(){return m_pGlobalSrv;}
	noyau_pool_id getPoolId();
	noyau_bal_id getAniBalId();

signals:
	void requestStop();

public slots:
	
private:

	void KybSrvReceptionArret( IN struct_VIRT_KYB_message * psMsg );
	
	void KybSrvReceptionAffichage ( IN struct_VIRT_KYB_message *psMsg );
	bool KybSrvTraitementAffichage ( IN struct_VIRT_KYB_message * psMsg );
	
	void KybMessEnvoiAcquitement(IN noyau_bal_id iBalDest, 
                                       IN enum_srv_service eService,
                                       IN enum_srv_type eTypeMessage);


	enum_srv_type KybDebutService(IN enum_srv_service eService,
                                        IN noyau_bal_id dwBalDest);
	enum_srv_type KybFinService(IN enum_srv_service eService,
                                      IN noyau_bal_id dwBalDest);

	static MKybSrv * m_pGlobalSrv;

	HANDLE				m_hService[NB_VIRT_KYB_SERVICE];

	MKybAniThread *				m_pAniThread;
};


#endif


