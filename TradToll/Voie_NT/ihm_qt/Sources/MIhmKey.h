#ifndef MIHM_KEY_H
#define MIHM_KEY_H

#include <QObject>


extern "C" {
	#include <noyau.h>
	#include <ihm.h>
}

#define IHM_KB_NB_KEY_PER_GROUP         25  // Nbre max de touches par groupe
#define IHM_KB_NB_GROUPES_MAX           100  // Nbre max de groupes de touches definies dabs le registre
#define IHM_KB_NB_TOUCHES_MAX           999 // Code ASCII du dernier caractere gere par le tableau de controle de touches
#define IHM_KB_NB_TACHES_MAX            100  // Nombre max de taches pour le tableau de controle de touches
#define IHM_KB_GRP_SEPARATOR_CHAR       ',' // separateur dans le format des groupes de cles dans le registre
#define IHM_KB_GRP_SEPARATOR_STRING     "," // separateur dans le format des groupes de cles dans le registre
#define IHM_REG_KEY_KEYSET              "KeySet"            
#define IHM_REG_KEY_KEYMAP              "KeyTranslationAppli"

#define IHM_REG_KEY_INVALID_KEY_DISPLAY	"InvalidKeyDisplay"



class MIhmKey:public QObject
{
    Q_OBJECT
public:
	MIhmKey();
	~MIhmKey();

	bool IhmKeyUpdateKeyTable(IN noyau_bal_id   iBalId,
								IN char         * szGroup,
								IN int            iKey,
								IN BYTE           bState );

	bool IhmKeyDontCare( IN int iKey, IN int iTask );
	bool IhmCommandDontCare(IN int iTask );
	int IhmKeyMap( IN int iKey );
	void IhmKeyInit(QString sConfigKey);

	bool isConfigInvalidKeyDisplay() { return m_bInvalidKeyDisplay;}

private:

	bool IhmKeyLoadGroupsFromRegister ( IN char * pcKey );

	bool IhmKeyAnalyseKeyGroup ( DWORD dwIdx, char * szDesc);

	// Tableau de controle des touches
	BYTE gttbKeyTasks[ IHM_KB_NB_TOUCHES_MAX + 1 ][ IHM_KB_NB_TACHES_MAX ];

	// Tableau de remapping des touches
	int gtiKeyMap[ IHM_KB_NB_TOUCHES_MAX + 1 ];

// Tableau de definition des groupes de touches
	struct structKeyGroup
	{
		char szGroupName[IHM_LG_CHAINES_SRV];   // Nom du groupe (chaine)
		int  tiKeys[IHM_KB_NB_KEY_PER_GROUP];   // liste des touches du groupe (0 = Fin de liste)
	};
	
	structKeyGroup gtsKeyGroups[IHM_KB_NB_GROUPES_MAX];
	
	QString m_sModuleConfigKey;
	bool m_bInvalidKeyDisplay;
};


#endif	// MIHM_KEY_H
/*---------------------------- END OF FILE -------------------------*/
