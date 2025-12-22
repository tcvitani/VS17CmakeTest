/********************************************************************************************************/
/*																										*/
/*	Projet		: BEACON MANAGER																		*/
/*																										*/
/*	Composant	: BEACON MANAGER																		*/
/*																										*/
/*	Module		: BeaconManager.h																		*/
/*																										*/
/*	Description	: Fonctions de gestion du protocole de communication PERTEL/TGB_v2/TGR_SB.				*/
/*																										*/
/*	Auteur		: C.Salenc																				*/
/*																										*/
/*	Date modif	: 18/08/2010																			*/
/*																										*/
/********************************************************************************************************/
/*	References																							*/
/*																										*/
/*	 . Interface couche 7 PERTEL_PC		(BAC)			(CGA 7112/2008/06/DSCS/0007 v2.1a)				*/
/*	 . Interface couche 2 PERTEL-PC		(BAC)			(CGA 7112/2008/06/DSCS/0006	v1.1)				*/
/*																										*/
/********************************************************************************************************/
/*																										*/
/*	Copyright (c) 1998-2010 GEA Meylan France.	All rights reserved.									*/
/*																										*/
/********************************************************************************************************/


#ifndef __BEACON_MANAGER__
#define __BEACON_MANAGER__

//___ #include __________________________________________________________________________________________

// Pour obtenir une DLL, il faut ajouter __BCM_DLL__ dans les options de compilation.

#include <windows.h>


//___ #define ___________________________________________________________________________________________

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __BCM_DLL__
	#define BCM_EXPORT	__stdcall
#else
	#define BCM_EXPORT
#endif


// Erreurs du gestionnaire
#define	BCM_ERR		int

enum
{
	BCM_NoErr				= 0,
	BCM_BadParam			= -100,
	BCM_ErrMemory			= -101,

	BCM_Busy				= -104,
	BCM_Collision			= -105,
	BCM_CommAborted			= -106,
	BCM_CommTimeout			= -107,
	BCM_ErrResponse			= -108,

	BCM_ErrCreateEvent		= -120,
	BCM_ErrCreateMutex		= -121,
	BCM_ErrCreateTimer		= -122,
	BCM_ErrEvent			= -123,

#ifndef COMM_ERR
	COMM_BadParam			= -1000,
	COMM_PortNotActif		= -1001,
	COMM_PortFrozen			= -1002,
	COMM_PortOutBusy		= -1003,
	COMM_ErrCreateBuffer	= -1004,
	COMM_ErrCreateEvent		= -1005,
	COMM_ErrCreateThread	= -1006,
	COMM_ErrSetPriority		= -1007,
	COMM_ErrSetEvent		= -1008,
	COMM_ErrTypePort		= -1009,
	COMM_ErrOpenPort		= -1010,
	COMM_ErrConfigPort		= -1011,
	COMM_ErrClosePort		= -1012,
	COMM_ErrReadPort		= -1013,
	COMM_ErrWritePort		= -1014,
	COMM_ErrEvent			= -1015,
#endif

#ifndef SOCK_ERR
	SOCK_BadParam			= -1050,
	SOCK_ErrCreateBuffer	= -1051,
	SOCK_ErrCreateEvent		= -1052,
	SOCK_ErrCreateThread	= -1053,
	SOCK_ErrEventSelect		= -1054,
	SOCK_ErrWaitEvent		= -1055,
	SOCK_ErrSetEvent		= -1056,
	SOCK_NotConnected		= -1057,
	SOCK_BufOutFull			= -1058,

	SOCK_ErrStartup			= -1061,
	SOCK_ErrCleanup			= -1062,
	SOCK_ErrCreateSocket	= -1063,
	SOCK_ErrOptionSocket	= -1064,
	SOCK_ErrControlSocket	= -1065,
	SOCK_ErrBindSocket		= -1066,
	SOCK_ErrListenSocket	= -1067,
	SOCK_ErrAcceptEvent		= -1068,
	SOCK_ErrAcceptSocket	= -1069,
	SOCK_ErrConnectEvent	= -1070,
	SOCK_ErrConnectSocket	= -1071,
	SOCK_ErrSendEvent		= -1072,
	SOCK_ErrSendSocket		= -1073,
	SOCK_ErrRecvEvent		= -1074,
	SOCK_ErrRecvSocket		= -1075,
	SOCK_ErrShutdownSocket	= -1076,
	SOCK_ErrCloseEvent		= -1077,
	SOCK_ErrCloseSocket		= -1078,
	SOCK_ErrCloseTimeout	= -1079,
	SOCK_ErrGetHost			= -1080,
#endif
};


// Type de station
typedef enum
{
	BCM_Primary = 0,							// La station est prioritaire en cas de collision
	BCM_Secondary								// La station n'est pas prioritaire en cas de collision
} BCM_STATION;


// Type de callback
typedef enum
{
	BCM_CB_IN = 0,								// Callback suite a la reception d'une trame
	BCM_CB_ERR = 3								// Callback suite a une erreur
} BCM_CALLBACK;


// Alarmes gerees par le gestionnaire
typedef enum
{
	BCM_AlarmPeriph = 1,						// L'antenne n'est pas active
	BCM_AlarmBeacon,							// La balise est HS

	BCM_EventReset,								// L'antenne vient de reseter
	BCM_EventPollingOK							// L'antenne est OK
} BCM_ALARMS;


// Type de BST
#define	BCM_BST_ChangeBID		0x03			// Lancement des BST avec changement du Beacon ID
#define	BCM_BST_Normal			0x17			// Lancement des BST sans changement du Beacon ID


// Mode de fonctionnement
#define BCM_MOD_Stopped			0x00			// Mode arret
#define BCM_MOD_Transparent		0x01			// Mode transparent
#define	BCM_MOD_Maintenance		0x03			// Mode maintenance


// Taille maxi des commandes couche 7
#define	BCM_SIZEMAX_BST			121				// Taille maxi d'une BST
#define	BCM_SIZEMAX_CMD			118				// Taille maxi d'une commande couche 7
#define	BCM_SIZEMAX_ANSWER		122				// Taille maxi d'une reponse


// Comptes rendus des commandes couche 7 (Antenne->PC)
#define	BCM_CdeNoErr			BCM_NoErr		// Commande acceptee
#define	BCM_CdeRefused			0x01			// Commande refusee
#define	BCM_TrxInProgress		0x02			// Commande refusee car une transaction est en cours
#define	BCM_PbBeacon			0x03			// Il y a un probleme avec la balise
#define	BCM_TmoOBE				0x09			// Commande terminee par un TimeOut OBE
#define	BCM_ResetBeacon			0x0A			// L'antenne vient de redemarrer
#define	BCM_PbParam				0x0B			// Commande refusee car au moins un parametre est incorrect
#define	BCM_PbFichConfig		0x0C			// Commande refusee car probleme avec le fichier de config (PERTEL)
#define	BCM_NotConfig			0x1D			// Commande refusee car configuration non effectuee (TGx)


// Codes specifiques au changement des parametres de configuration
#define	BCM_CFG_Immediate		0x00			// Prise en compte des changements immediatement
#define	BCM_CFG_Reset			0x01			// Prise en compte des changements au prochain reset

#define	BCM_CFG_F1				0x01			// Frequence balise F1
#define	BCM_CFG_F2				0x02			// Frequence balise F2
#define	BCM_CFG_F3				0x03			// Frequence balise F3
#define	BCM_CFG_F4				0x04			// Frequence balise F4

#define	BCM_CFG_1200			0x00			// Debit a 1200 bauds	(pas pour TGx)
#define	BCM_CFG_2400			0x01			// Debit a 2400 bauds	(pas pour TGx)
#define	BCM_CFG_4800			0x02			// Debit a 4800 bauds	(pas pour TGx)
#define	BCM_CFG_9600			0x03			// Debit a 9600 bauds
#define	BCM_CFG_19200			0x04			// Debit a 19200 bauds
#define	BCM_CFG_38400			0x05			// Debit a 38400 bauds
#define	BCM_CFG_57600			0x06			// Debit a 57600 bauds
#define	BCM_CFG_115200			0x07			// Debit a 115200 bauds


#define	BCM_SIZE_CONFIG			28				// Taille des parametres de config
#define	BCM_SIZE_BEACONID		6				// Taille du beacon ID


#define BCM_FN_GetLibVersion		"BCM_GetLibVersion"
#define	BCM_FN_InitManagerWND		"BCM_InitManagerWND"
#define	BCM_FN_InitManagerTHD		"BCM_InitManagerTHD"
#define	BCM_FN_InitManagerFNC		"BCM_InitManagerFNC"
#define	BCM_FN_InitManagerWND_IP	"BCM_InitManagerWND_IP"
#define	BCM_FN_InitManagerTHD_IP	"BCM_InitManagerTHD_IP"
#define	BCM_FN_InitManagerFNC_IP	"BCM_InitManagerFNC_IP"
#define	BCM_FN_CloseManager			"BCM_CloseManager"
#define	BCM_FN_ChangeMode			"BCM_ChangeMode"
#define	BCM_FN_StartBST				"BCM_StartBST"
#define	BCM_FN_GetVST				"BCM_GetVST"
#define	BCM_FN_GetUserParams		"BCM_GetUserParams"
#define	BCM_FN_SendCmd				"BCM_SendCmd"
#define	BCM_FN_StopBST				"BCM_StopBST"
#define	BCM_FN_CheckState			"BCM_CheckState"
#define	BCM_FN_Reset				"BCM_Reset"
#define	BCM_FN_SetConfig			"BCM_SetConfig"
#define	BCM_FN_GetConfig			"BCM_GetConfig"
#define	BCM_FN_GetBeaconID			"BCM_GetBeaconID"


//___ Prototypes ________________________________________________________________________________________

// Etat de l'antenne
typedef struct ST_BCM_STATE ST_BCM_STATE;
typedef ST_BCM_STATE *ST_BCM_STATE_PTR;

struct ST_BCM_STATE
{
	BYTE	state;								// Etat de l'antenne (cf Comptes rendus commandes couche 7)
	BYTE	mode;								// Mode de fonctionnement de l'antenne (cf BCM_MOD_xxx)
	BYTE	trxInProgress;						// Indique si des BST ou une transaction sont en cours
};


// Parametres de config
typedef struct ST_BCM_CONFIG ST_BCM_CONFIG;
typedef ST_BCM_CONFIG *ST_BCM_CONFIG_PTR;

struct ST_BCM_CONFIG
{
	BYTE	version[255+1];						// Version logicielle de l'antenne

	// Parametres de l'echange BST_VST
	WORD	manufacturerID;
	DWORD	individualID;
	WORD	versionDescam;

	// Parametres de la couche 2 I2C2
	WORD	nbRetriesBST;
	WORD	timeoutRetryBST;					// Exprime en ms
	WORD	nbRetriesACn;
	BYTE	frequency;							// 1=F1 ... 4=F4 (cf section #define BCM_CFG_xxx)

	// Parametres de la couche 2 BAC
	WORD	baudRate;							// 0=1200 ... 7=115200 (cf section #define BCM_CFG_xxx)
	WORD	watchdog;							// 0=inhibe, 1=actif
	WORD	technology;							// 1=Inside technology, 2=Acrosser technology

	// Parametres de MARTEL
	BYTE	nbBeacons;							// 1 < nbBalises < 12
	WORD	numLocation;

	// ????
	BYTE	dummy[4];
};


// Declaration de la structure du gestionnaire
typedef struct ST_BCM_REG ST_BCM_REG;
typedef ST_BCM_REG *ST_BCM_REG_PTR;


// Type de la fonction executee lorsqu'une trame a ete recue, ou si une erreur est apparue
// Parametres :
//		 -> ST_BCM_REG_PTR		argBcmRegPtr	: Pointeur sur la structure du gestionnaire
//		 -> BCM_CALLBACK		argTypeCB		: Type de callback
//		 -> DWORD				argParam		: Parametre dependant du type callback
// Retour :
// Remarque :
//	Si argTypeCB = BCM_CB_IN alors une VST a ete recue
//		 -> argParam = non utilise
//
//	Si argTypeCB = BCM_CB_ERR alors il y a eu une erreur
//		 -> argParam = COMM_ErrOpenPort		erreur lors d'une reouverture du port
//					   COMM_ErrConfigPort	erreur lors de la configuration du port
//					   COMM_ErrReadPort		erreur lors d'une lecture sur le port
//					   COMM_ErrWritePort	erreur lors d'une ecriture sur le port
//					   COMM_ErrEvent		un evenement non prevu a ete recu
//					   BCM_CommAborted		probleme de communication avec l'antenne
//					   BCM_CommTimeout		aucune reponse a une commande n'a ete recue
//					   BCM_ErrResponse		la reponse a une commande est incomprehensible
//					   BCM_CdeRefused		commande refusee
//					   BCM_TrxInProgress 	commande refusee car une transaction est en cours
//					   BCM_PbBeacon			il y a un probleme avec la balise
//					   BCM_TmoOBE			commande terminee par un TimeOut OBE
//					   BCM_PbParam			commande refusee car au moins un parametre est incorrect
//					   BCM_PbFichConfig		commande refusee car probleme avec le fichier de config
//					   BCM_NotConfig		commande refusee car configuration non effectuee (TGx)
//
typedef void (BCM_EXPORT *BCM_CB_HANDLER) ( ST_BCM_REG_PTR	argBcmRegPtr,
											BCM_CALLBACK	argTypeCB,
											DWORD			argParam );


// Type de la fonction executee lorsqu'une alarme apparait ou disparait
// Parametres :
//		 -> ST_BCM_REG_PTR		argBcmRegPtr	: Pointeur sur la structure du gestionnaire
//		 -> BCM_ALARMS			argAlarm		: Type de l'alarme
//		 -> DWORD				argState		: TRUE indique apparition, FALSE indique disparition
// Retour :
//
typedef void (BCM_EXPORT *BCM_ALARM_HANDLER) ( ST_BCM_REG_PTR	argBcmRegPtr,
											   BCM_ALARMS		argAlarm,
											   DWORD			argState );


//___ Declarations des variables globales _______________________________________________________________
//___ Prototypes des fonctions globales _________________________________________________________________

typedef DWORD   (BCM_EXPORT *BCM_LPFN_GetLibVersion)	  ( void );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_InitManagerWND)	  ( ST_BCM_REG_PTR		*argBcmRegPtr,
															DWORD				argRegNumber,
															VOID				*argUserParamPtr,
															BYTE				argPort,
															BYTE				argBaudRate,
															BCM_STATION			argStation,
															DWORD				argCheckBeacon,
															BOOL				argSendEvtPollingOK,
															UINT				argMsg,
															HWND				argWindowHdl );
												  
typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_InitManagerTHD)	  ( ST_BCM_REG_PTR		*argBcmRegPtr,
															DWORD				argRegNumber,
															VOID				*argUserParamPtr,
															BYTE				argPort,
															BYTE				argBaudRate,
															BCM_STATION			argStation,
															DWORD				argCheckBeacon,
															BOOL				argSendEvtPollingOK,
															UINT				argMsg,
															DWORD				argThreadId );
												  
typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_InitManagerFNC)	  ( ST_BCM_REG_PTR		*argBcmRegPtr,
															DWORD				argRegNumber,
															VOID				*argUserParamPtr,
															BYTE				argPort,
															BYTE				argBaudRate,
															BCM_STATION			argStation,
															DWORD				argCheckBeacon,
															BOOL				argSendEvtPollingOK,
															BCM_CB_HANDLER		argCallbackProc,
															BCM_ALARM_HANDLER	argAlarmProc );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_InitManagerWND_IP)  ( ST_BCM_REG_PTR		*argBcmRegPtr,
															DWORD				argRegNumber,
															VOID				*argUserParamPtr,
															char				*argAddr,
															WORD				argPort,
															BCM_STATION			argStation,
															DWORD				argCheckBeacon,
															BOOL				argSendEvtPollingOK,
															UINT				argMsg,
															HWND				argWindowHdl );
												  
typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_InitManagerTHD_IP)  ( ST_BCM_REG_PTR		*argBcmRegPtr,
															DWORD				argRegNumber,
															VOID				*argUserParamPtr,
															char				*argAddr,
															WORD				argPort,
															BCM_STATION			argStation,
															DWORD				argCheckBeacon,
															BOOL				argSendEvtPollingOK,
															UINT				argMsg,
															DWORD				argThreadId );
												  
typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_InitManagerFNC_IP)  ( ST_BCM_REG_PTR		*argBcmRegPtr,
															DWORD				argRegNumber,
															VOID				*argUserParamPtr,
															char				*argAddr,
															WORD				argPort,
															BCM_STATION			argStation,
															DWORD				argCheckBeacon,
															BOOL				argSendEvtPollingOK,
															BCM_CB_HANDLER		argCallbackProc,
															BCM_ALARM_HANDLER	argAlarmProc );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_CloseManager)		  ( ST_BCM_REG_PTR 		*argBcmRegPtr );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_ChangeMode)		  ( ST_BCM_REG_PTR		argBcmRegPtr,
															BYTE				argMode );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_StartBST)			  ( ST_BCM_REG_PTR		argBcmRegPtr,
															BYTE				*argDatagram,
															DWORD				argLgDatagram,
															BYTE				argTypeBST );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_GetVST)			  ( ST_BCM_REG_PTR		argBcmRegPtr,
															BYTE				*argDatagram,
															DWORD				*argLgDatagram,
															DWORD				argLgMaxDatagram );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_GetUserParams)	  ( ST_BCM_REG_PTR		argBcmRegPtr,
															DWORD				*argRegNumberPtr,
															VOID				**argUserParamPtr );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_SendCmd)			  ( ST_BCM_REG_PTR		argBcmRegPtr,
															BYTE				*argDatagramOut,
															DWORD				argLgDatagramOut,
															BYTE				*argDatagramIn,
															DWORD				*argLgDatagramIn,
															DWORD				argLgMaxDatagramIn,
															BOOL				argIsLast );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_StopBST)			  ( ST_BCM_REG_PTR		argBcmRegPtr );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_CheckState)		  ( ST_BCM_REG_PTR		argBcmRegPtr,
															ST_BCM_STATE_PTR	argStatePtr );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_Reset)			  ( ST_BCM_REG_PTR		argBcmRegPtr );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_SetConfig)		  ( ST_BCM_REG_PTR		argBcmRegPtr,
															BYTE				argMode,
															BYTE				argFrequency,
															BYTE				argBaudRate );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_GetConfig)		  ( ST_BCM_REG_PTR		argBcmRegPtr,
															ST_BCM_CONFIG_PTR	argConfigPtr );

typedef BCM_ERR (BCM_EXPORT *BCM_LPFN_GetBeaconID)		  ( ST_BCM_REG_PTR		argBcmRegPtr,
															BYTE				*argBeaconIDPtr );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_GetLibVersion																		*/
/*																										*/
/*  But	: Recuperer la version de la librairie				 											*/
/*																										*/
/*  Parametres :																						*/
/*																										*/
/*	Retour : 																							*/
/*		<-  DWORD	: Version de la librairie															*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
DWORD BCM_EXPORT BCM_GetLibVersion( void );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_InitManagerWND																		*/
/*																										*/
/*  But	: Creer et initialiser le gestionnaire				 											*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	*argBcmRegPtr			: Adresse du pointeur sur la struct du gestionnaire	*/
/*		 -> DWORD			argRegNumber			: Parametre libre pour l'utilisateur				*/
/*		 -> VOID			*argUserParamPtr		: Parametre libre pour l'utilisateur				*/
/*		 -> BYTE			argPort					: Numero du port serie sur lequel il y a l'antenne	*/
/*		 -> BYTE			argBaudRate				: Vitesse de communication de l'antenne				*/
/*		 -> BCM_STATION		argStation				: Indique si la station est primaire ou secondaire	*/
/*		 -> DWORD			argCheckBeacon			: Periode du timer de polling (en ms)				*/
/*		 -> BOOL			argSendEvtPollingOK		: Indique si l'event polling OK doit etre envoye	*/
/*		 -> UINT			argMsg					: Message a envoyer (doir etre >= WM_USER(1024))	*/
/*		 -> HWND			argWindowHdl			: Handle de la fenetre qui doit recevoir le message	*/
/*																										*/
/*	Retour : 																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr				: Si tout c'est bien passe							*/
/*							BCM_BadParam			: Parametre de la fonction incorrect				*/
/*							BCM_ErrMemory			: Erreur lors de la creation du gestionnaire		*/
/*							BCM_ErrCreateEvent		: Erreur lors de la creation de l'evenement			*/
/*							BCM_ErrCreateMutex		: Erreur lors de la creation du mutex				*/
/*							BCM_ErrCreateTimer		: Erreur lors de la creation du timer				*/
/*							COMM_ErrCreateBuffer	: Erreur lors de la creation des buffers			*/
/*							COMM_ErrCreateEvent		: Erreur lors de la creation d'un evenement			*/
/*							COMM_ErrCreateThread	: Erreur lors de la creation du thread				*/
/*							COMM_ErrOpenPort		: Erreur lors de l'ouverture du port				*/
/*							COMM_ErrConfigPort		: Erreur lors de la configuration du port			*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_InitManagerWND( ST_BCM_REG_PTR	*argBcmRegPtr,
									   DWORD			argRegNumber,
									   VOID				*argUserParamPtr,
									   BYTE				argPort,
									   BYTE				argBaudRate,
									   BCM_STATION		argStation,
									   DWORD			argCheckBeacon,
									   BOOL				argSendEvtPollingOK,
									   UINT				argMsg,
									   HWND				argWindowHdl );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_InitManagerTHD																		*/
/*																										*/
/*  But	: Creer et initialiser le gestionnaire				 											*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	*argBcmRegPtr			: Adresse du pointeur sur la struct du gestionnaire	*/
/*		 -> DWORD			argRegNumber			: Parametre libre pour l'utilisateur				*/
/*		 -> VOID			*argUserParamPtr		: Parametre libre pour l'utilisateur				*/
/*		 -> BYTE			argPort					: Numero du port serie sur lequel il y a l'antenne	*/
/*		 -> BYTE			argBaudRate				: Vitesse de communication de l'antenne				*/
/*		 -> BCM_STATION		argStation				: Indique si la station est primaire ou secondaire	*/
/*		 -> DWORD			argCheckBeacon			: Periode du timer de polling (en ms)				*/
/*		 -> BOOL			argSendEvtPollingOK		: Indique si l'event polling OK doit etre envoye	*/
/*		 -> UINT			argMsg					: Message a envoyer (doir etre >= WM_USER(1024))	*/
/*		 -> DWORD			argThreadId				: Id du thread qui doit recevoir le message			*/
/*																										*/
/*	Retour : 																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr				: Si tout c'est bien passe							*/
/*							BCM_BadParam			: Parametre de la fonction incorrect				*/
/*							BCM_ErrMemory			: Erreur lors de la creation du gestionnaire		*/
/*							BCM_ErrCreateEvent		: Erreur lors de la creation de l'evenement			*/
/*							BCM_ErrCreateMutex		: Erreur lors de la creation du mutex				*/
/*							BCM_ErrCreateTimer		: Erreur lors de la creation du timer				*/
/*							COMM_ErrCreateBuffer	: Erreur lors de la creation des buffers			*/
/*							COMM_ErrCreateEvent		: Erreur lors de la creation d'un evenement			*/
/*							COMM_ErrCreateThread	: Erreur lors de la creation du thread				*/
/*							COMM_ErrOpenPort		: Erreur lors de l'ouverture du port				*/
/*							COMM_ErrConfigPort		: Erreur lors de la configuration du port			*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_InitManagerTHD( ST_BCM_REG_PTR	*argBcmRegPtr,
									   DWORD			argRegNumber,
									   VOID				*argUserParamPtr,
									   BYTE				argPort,
									   BYTE				argBaudRate,
									   BCM_STATION		argStation,
									   DWORD			argCheckBeacon,
									   BOOL				argSendEvtPollingOK,
									   UINT				argMsg,
									   DWORD			argThreadId );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_InitManagerFNC																		*/
/*																										*/
/*  But	: Creer et initialiser le gestionnaire				 											*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR		*argBcmRegPtr		: Adresse du pointeur sur la struct du gestionnaire	*/
/*		 -> DWORD				argRegNumber		: Parametre libre pour l'utilisateur				*/
/*		 -> VOID				*argUserParamPtr	: Parametre libre pour l'utilisateur				*/
/*		 -> BYTE				argPort				: Numero du port serie sur lequel il y a l'antenne	*/
/*		 -> BYTE				argBaudRate			: Vitesse de communication de l'antenne				*/
/*		 -> BCM_STATION			argStation			: Indique si la station est primaire ou secondaire	*/
/*		 -> DWORD				argCheckBeacon		: Periode du timer de polling (en ms)				*/
/*		 -> BOOL				argSendEvtPollingOK	: Indique si l'event polling OK doit etre envoye	*/
/*		 -> BCM_CB_HANDLER		argCallbackProc		: Fonction de callback de reception, ou d'erreur	*/
/*		 -> BCM_ALARM_HANDLER	argAlarmProc		: Fonction de callback d'alarme						*/
/*																										*/
/*	Retour : 																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr				: Si tout c'est bien passe							*/
/*							BCM_BadParam			: Parametre de la fonction incorrect				*/
/*							BCM_ErrMemory			: Erreur lors de la creation du gestionnaire		*/
/*							BCM_ErrCreateEvent		: Erreur lors de la creation de l'evenement			*/
/*							BCM_ErrCreateMutex		: Erreur lors de la creation du mutex				*/
/*							BCM_ErrCreateTimer		: Erreur lors de la creation du timer				*/
/*							COMM_ErrCreateBuffer	: Erreur lors de la creation des buffers			*/
/*							COMM_ErrCreateEvent		: Erreur lors de la creation d'un evenement			*/
/*							COMM_ErrCreateThread	: Erreur lors de la creation du thread				*/
/*							COMM_ErrOpenPort		: Erreur lors de l'ouverture du port				*/
/*							COMM_ErrConfigPort		: Erreur lors de la configuration du port			*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_InitManagerFNC( ST_BCM_REG_PTR		*argBcmRegPtr,
									   DWORD				argRegNumber,
									   VOID					*argUserParamPtr,
									   BYTE					argPort,
									   BYTE					argBaudRate,
									   BCM_STATION			argStation,
									   DWORD				argCheckBeacon,
									   BOOL					argSendEvtPollingOK,
									   BCM_CB_HANDLER		argCallbackProc,
									   BCM_ALARM_HANDLER	argAlarmProc );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_InitManagerWND_IP																	*/
/*																										*/
/*  But	: Creer et initialiser le gestionnaire				 											*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	*argBcmRegPtr			: Adresse du pointeur sur la struct du gestionnaire	*/
/*		 -> DWORD			argRegNumber			: Parametre libre pour l'utilisateur				*/
/*		 -> VOID			*argUserParamPtr		: Parametre libre pour l'utilisateur				*/
/*		 -> char			*argAddr				: Adresse IP ou nom de l'antenne					*/
/*		 -> WORD			argPort					: Numero du port IP									*/
/*		 -> BCM_STATION		argStation				: Indique si la station est primaire ou secondaire	*/
/*		 -> DWORD			argCheckBeacon			: Periode du timer de polling (en ms)				*/
/*		 -> BOOL			argSendEvtPollingOK		: Indique si l'event polling OK doit etre envoye	*/
/*		 -> UINT			argMsg					: Message a envoyer (doir etre >= WM_USER(1024))	*/
/*		 -> HWND			argWindowHdl			: Handle de la fenetre qui doit recevoir le message	*/
/*																										*/
/*	Retour : 																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr				: Si tout c'est bien passe							*/
/*							BCM_BadParam			: Parametre de la fonction incorrect				*/
/*							BCM_ErrMemory			: Erreur lors de la creation du gestionnaire		*/
/*							BCM_ErrCreateEvent		: Erreur lors de la creation de l'evenement			*/
/*							BCM_ErrCreateMutex		: Erreur lors de la creation du mutex				*/
/*							BCM_ErrCreateTimer		: Erreur lors de la creation du timer				*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_InitManagerWND_IP( ST_BCM_REG_PTR	*argBcmRegPtr,
										  DWORD				argRegNumber,
										  VOID				*argUserParamPtr,
										  char				*argAddr,
										  WORD				argPort,
										  BCM_STATION		argStation,
										  DWORD				argCheckBeacon,
										  BOOL				argSendEvtPollingOK,
										  UINT				argMsg,
										  HWND				argWindowHdl );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_InitManagerTHD_IP																	*/
/*																										*/
/*  But	: Creer et initialiser le gestionnaire				 											*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	*argBcmRegPtr			: Adresse du pointeur sur la struct du gestionnaire	*/
/*		 -> DWORD			argRegNumber			: Parametre libre pour l'utilisateur				*/
/*		 -> VOID			*argUserParamPtr		: Parametre libre pour l'utilisateur				*/
/*		 -> char			*argAddr				: Adresse IP ou nom de l'antenne					*/
/*		 -> WORD			argPort					: Numero du port IP									*/
/*		 -> BCM_STATION		argStation				: Indique si la station est primaire ou secondaire	*/
/*		 -> DWORD			argCheckBeacon			: Periode du timer de polling (en ms)				*/
/*		 -> BOOL			argSendEvtPollingOK		: Indique si l'event polling OK doit etre envoye	*/
/*		 -> UINT			argMsg					: Message a envoyer (doir etre >= WM_USER(1024))	*/
/*		 -> DWORD			argThreadId				: Id du thread qui doit recevoir le message			*/
/*																										*/
/*	Retour : 																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr				: Si tout c'est bien passe							*/
/*							BCM_BadParam			: Parametre de la fonction incorrect				*/
/*							BCM_ErrMemory			: Erreur lors de la creation du gestionnaire		*/
/*							BCM_ErrCreateEvent		: Erreur lors de la creation de l'evenement			*/
/*							BCM_ErrCreateMutex		: Erreur lors de la creation du mutex				*/
/*							BCM_ErrCreateTimer		: Erreur lors de la creation du timer				*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_InitManagerTHD_IP( ST_BCM_REG_PTR	*argBcmRegPtr,
										  DWORD				argRegNumber,
										  VOID				*argUserParamPtr,
										  char				*argAddr,
										  WORD				argPort,
										  BCM_STATION		argStation,
										  DWORD				argCheckBeacon,
										  BOOL				argSendEvtPollingOK,
										  UINT				argMsg,
										  DWORD				argThreadId );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_InitManagerFNC_IP																	*/
/*																										*/
/*  But	: Creer et initialiser le gestionnaire				 											*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR		*argBcmRegPtr		: Adresse du pointeur sur la struct du gestionnaire	*/
/*		 -> DWORD				argRegNumber		: Parametre libre pour l'utilisateur				*/
/*		 -> VOID				*argUserParamPtr	: Parametre libre pour l'utilisateur				*/
/*		 -> char				*argAddr			: Adresse IP ou nom de l'antenne					*/
/*		 -> WORD				argPort				: Numero du port IP									*/
/*		 -> BCM_STATION			argStation			: Indique si la station est primaire ou secondaire	*/
/*		 -> DWORD				argCheckBeacon		: Periode du timer de polling (en ms)				*/
/*		 -> BOOL				argSendEvtPollingOK	: Indique si l'event polling OK doit etre envoye	*/
/*		 -> BCM_CB_HANDLER		argCallbackProc		: Fonction de callback de reception, ou d'erreur	*/
/*		 -> BCM_ALARM_HANDLER	argAlarmProc		: Fonction de callback d'alarme						*/
/*																										*/
/*	Retour : 																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr				: Si tout c'est bien passe							*/
/*							BCM_BadParam			: Parametre de la fonction incorrect				*/
/*							BCM_ErrMemory			: Erreur lors de la creation du gestionnaire		*/
/*							BCM_ErrCreateEvent		: Erreur lors de la creation de l'evenement			*/
/*							BCM_ErrCreateMutex		: Erreur lors de la creation du mutex				*/
/*							BCM_ErrCreateTimer		: Erreur lors de la creation du timer				*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_InitManagerFNC_IP( ST_BCM_REG_PTR	*argBcmRegPtr,
										  DWORD				argRegNumber,
										  VOID				*argUserParamPtr,
										  char				*argAddr,
										  WORD				argPort,
										  BCM_STATION		argStation,
										  DWORD				argCheckBeacon,
										  BOOL				argSendEvtPollingOK,
										  BCM_CB_HANDLER	argCallbackProc,
										  BCM_ALARM_HANDLER	argAlarmProc );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_CloseManager																			*/
/*																										*/
/*  But	: Libere tout ce qui a pu etre liee au gestionnaire												*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	*argBcmRegPtr		: Adresse du pointeur sur la structure du gestionnaire	*/
/*																										*/
/*	Retour : 																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							COMM_ErrClosePort	: Si il y a eu une erreur lors de la fermeture du port	*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_CloseManager( ST_BCM_REG_PTR *argBcmRegPtr );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_ChangeMode																			*/
/*																										*/
/*  But	: Changement du mode de fonctionnement															*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	argBcmRegPtr		: Pointeur sur la structure du gestionnaire				*/
/*		 ->	BYTE			argMode				: Mode de fonctionnement								*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							BCM_ErrMemory		: Si on n'a pas pu empiler la nouvelle commande			*/
/*							BCM_CommAborted		: Si il y a eu probleme de communication avec l'antenne	*/
/*							BCM_CommTimeout		: Si aucune reponse a la commande n'a ete recue			*/
/*							BCM_ErrResponse		: Si la reponse est incomprehensible					*/
/*							BCM_ErrEvent		: Si une erreur interne d'evenement est apparue			*/
/*							COMM_PortNotActif	: Si le thread de gestion du port est arrete			*/
/*							COMM_PortOutBusy	: Si le port n'est pas pret								*/
/*							COMM_ErrSetEvent	: Si l'evenement USER n'a pas pu etre envoye			*/
/*							BCM_CdeRefused		: Commande refusee pour l'une des causes suivantes :	*/
/*													- la trame commande est incorrecte					*/
/*													- le mode de l'antenne est inconnu					*/
/*							BCM_TrxInProgress	: Commande refusee car une transaction est en cours		*/
/*							BCM_PbBeacon		: Commande refusee car la balise est HS et le mode		*/
/*													demande est different de arret						*/
/*							BCM_NotConfig		: Commande refusee car le TGx n'est pas configure		*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_ChangeMode( ST_BCM_REG_PTR argBcmRegPtr, BYTE argMode );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_StartBST																				*/
/*																										*/
/*  But	: Demarrer l'envoi periodique des BST															*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	argBcmRegPtr		: Pointeur sur la structure du gestionnaire				*/
/*		 ->	BYTE			*argDatagram		: Trame a envoyer au badge								*/
/*		 -> DWORD			argLgDatagram		: Longueur de la trame a envoyer						*/
/*		 -> BYTE			argTypeBST			: Type de la BST :										*/
/*													. BCM_BST_Normal      : BST normale					*/
/*													. BCM_BST_ChangeBID   : BST avec chmgt de beacon ID	*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							BCM_ErrMemory		: Si on n'a pas pu empiler la nouvelle commande			*/
/*							BCM_CommAborted		: Si il y a eu probleme de communication avec l'antenne	*/
/*							BCM_CommTimeout		: Si aucune reponse a la commande n'a ete recue			*/
/*							BCM_ErrResponse		: Si la reponse est incomprehensible					*/
/*							BCM_ErrEvent		: Si une erreur interne d'evenement est apparue			*/
/*							COMM_PortNotActif	: Si le thread de gestion du port est arrete			*/
/*							COMM_PortOutBusy	: Si le port n'est pas pret								*/
/*							COMM_ErrSetEvent	: Si l'evenement USER n'a pas pu etre envoye			*/
/*							BCM_CdeRefused		: Commande refusee pour l'une des causes suivantes :	*/
/*													- la trame commande est incorrecte					*/
/*													- le mode de l'antenne est incorrect				*/
/*							BCM_TrxInProgress	: Commande refusee car une transaction est en cours		*/
/*							BCM_PbBeacon		: Commande refusee car la balise est en panne			*/
/*							BCM_NotConfig		: Commande refusee car le TGx n'est pas configure		*/
/*																										*/
/*	Remarques :	L'antenne doit etre mise en mode TRANSPARENT  !!!										*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_StartBST( ST_BCM_REG_PTR	argBcmRegPtr,
								 BYTE			*argDatagram,
								 DWORD			argLgDatagram,
								 BYTE			argTypeBST );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_GetVST																				*/
/*																										*/
/*  But	: Recuperer la derniere VST recue																*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	argBcmRegPtr		: Pointeur sur la structure du gestionnaire				*/
/*		 -> BYTE			*argDatagram		: Trame VST recue du badge								*/
/*		 -> DWORD			*argLgDatagram		: Longueur de la trame VST								*/
/*		 -> DWORD			argLgMaxDatagram	: Taille du buffer de reception							*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_GetVST( ST_BCM_REG_PTR	argBcmRegPtr,
							   BYTE				*argDatagram,
							   DWORD			*argLgDatagram,
							   DWORD			argLgMaxDatagram );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_GetUserParams																		*/
/*																										*/
/*  But	: Recuperer les parametres utilisateur definis lors de l'initialisation							*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	argBcmRegPtr		: Pointeur sur la structure du gestionnaire				*/
/*		<-	DWORD			*argRegNumberPtr	: Pointeur sur le premier parametre utilisateur			*/
/*		<-	VOID			**argUserParamPtr	: Pointeur sur le second parametre utilisateur			*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_GetUserParams( ST_BCM_REG_PTR	argBcmRegPtr,
									  DWORD				*argRegNumberPtr,
									  VOID				**argUserParamPtr );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_SendCmd																				*/
/*																										*/
/*  But	: Envoi d'une trame a un badge et reception de la reponse										*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	argBcmRegPtr		: Pointeur sur la structure du gestionnaire				*/
/*		 ->	BYTE			*argDatagramOut		: Trame a envoyer au badge								*/
/*		 -> DWORD			argLgDatagramOut	: Longueur de la trame a envoyer						*/
/*		 -> BYTE			*argDatagramIn		: Trame recue du badge									*/
/*		 -> DWORD			*argLgDatagramIn	: Longueur de la trame recue							*/
/*		 -> DWORD			argLgMaxDatagramIn	: Taille du buffer de reception							*/
/*		 -> BOOL			argIsLast			: Indique si il s'agit de la derniere commande			*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							BCM_ErrMemory		: Si on n'a pas pu empiler la nouvelle commande			*/
/*							BCM_CommAborted		: Si il y a eu probleme de communication avec l'antenne	*/
/*							BCM_CommTimeout		: Si aucune reponse a la commande n'a ete recue			*/
/*							BCM_ErrResponse		: Si la reponse est incomprehensible					*/
/*							BCM_ErrEvent		: Si une erreur interne d'evenement est apparue			*/
/*							COMM_PortNotActif	: Si le thread de gestion du port est arrete			*/
/*							COMM_PortOutBusy	: Si le port n'est pas pret								*/
/*							COMM_ErrSetEvent	: Si l'evenement USER n'a pas pu etre envoye			*/
/*							BCM_CdeRefused		: Commande refusee pour l'une des causes suivantes :	*/
/*													- la trame commande est incorrecte					*/
/*													- le mode de l'antenne est incorrect				*/
/*													- pas de transaction en cours						*/
/*							BCM_PbBeacon		: Commande refusee car la balise est en panne			*/
/*							BCM_TmoOBE			: Commande terminee par un timeout OBE					*/
/*							BCM_NotConfig		: Commande refusee car le TGx n'est pas configure		*/
/*																										*/
/*	Remarques :	L'antenne doit etre mise en mode TRANSPARENT  !!!										*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_SendCmd( ST_BCM_REG_PTR	argBcmRegPtr,
								BYTE			*argDatagramOut,
								DWORD			argLgDatagramOut,
								BYTE			*argDatagramIn,
								DWORD			*argLgDatagramIn,
								DWORD			argLgMaxDatagrameIn,
								BOOL			argIsLast );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_StopBST																				*/
/*																										*/
/*  But	: Arreter l'emission periodique des BST															*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	argBcmRegPtr		: Pointeur sur la structure du gestionnaire				*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							BCM_ErrMemory		: Si on n'a pas pu empiler la nouvelle commande			*/
/*							BCM_CommAborted		: Si il y a eu probleme de communication avec l'antenne	*/
/*							BCM_CommTimeout		: Si aucune reponse a la commande n'a ete recue			*/
/*							BCM_ErrResponse		: Si la reponse est incomprehensible					*/
/*							BCM_ErrEvent		: Si une erreur interne d'evenement est apparue			*/
/*							COMM_PortNotActif	: Si le thread de gestion du port est arrete			*/
/*							COMM_PortOutBusy	: Si le port n'est pas pret								*/
/*							COMM_ErrSetEvent	: Si l'evenement USER n'a pas pu etre envoye			*/
/*							BCM_CdeRefused		: Commande refusee pour l'une des causes suivantes :	*/
/*													- la trame commande est incorrecte					*/
/*													- le mode de l'antenne est incorrect				*/
/*							BCM_TrxInProgress	: Commande refusee car une transaction est en cours		*/
/*							BCM_NotConfig		: Commande refusee car le TGx n'est pas configure		*/
/*																										*/
/*	Remarques :	L'antenne doit etre mise en mode TRANSPARENT  !!!										*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_StopBST( ST_BCM_REG_PTR argBcmRegPtr );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_CheckState																			*/
/*																										*/
/*  But	: Lecture de l'etat de l'antenne																*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR		argBcmRegPtr	: Pointeur sur la structure du gestionnaire				*/
/*		<-  ST_BCM_STATE_PTR	argStatePtr		: Pointeur sur une structure remplie avec l'etat		*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							BCM_ErrMemory		: Si on n'a pas pu empiler la nouvelle commande			*/
/*							BCM_CommAborted		: Si il y a eu probleme de communication avec l'antenne	*/
/*							BCM_CommTimeout		: Si aucune reponse a la commande n'a ete recue			*/
/*							BCM_ErrResponse		: Si la reponse est incomprehensible					*/
/*							BCM_ErrEvent		: Si une erreur interne d'evenement est apparue			*/
/*							COMM_PortNotActif	: Si le thread de gestion du port est arrete			*/
/*							COMM_PortOutBusy	: Si le port n'est pas pret								*/
/*							COMM_ErrSetEvent	: Si l'evenement USER n'a pas pu etre envoye			*/
/*							BCM_CdeRefused		: Commande refusee car la trame commande est incorrecte	*/
/*							BCM_NotConfig		: Commande refusee car le TGx n'est pas configure		*/
/*																										*/
/*	Remarques :																							*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_CheckState( ST_BCM_REG_PTR argBcmRegPtr, ST_BCM_STATE_PTR argStatePtr );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_Reset																				*/
/*																										*/
/*  But	: Reset de l'antenne																			*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR		argBcmRegPtr	: Pointeur sur la structure du gestionnaire				*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							BCM_ErrMemory		: Si on n'a pas pu empiler la nouvelle commande			*/
/*							BCM_CommAborted		: Si il y a eu probleme de communication avec l'antenne	*/
/*							BCM_CommTimeout		: Si aucune reponse a la commande n'a ete recue			*/
/*							BCM_ErrResponse		: Si la reponse est incomprehensible					*/
/*							BCM_ErrEvent		: Si une erreur interne d'evenement est apparue			*/
/*							COMM_PortNotActif	: Si le thread de gestion du port est arrete			*/
/*							COMM_PortOutBusy	: Si le port n'est pas pret								*/
/*							COMM_ErrSetEvent	: Si l'evenement USER n'a pas pu etre envoye			*/
/*							BCM_CdeRefused		: Commande refusee pour l'une des causes suivantes :	*/
/*													- la trame commande est incorrecte					*/
/*													- le mode de l'antenne est incorrect				*/
/*													- l'identifiant est incorrect						*/
/*							BCM_NotConfig		: Commande refusee car le TGx n'est pas configure		*/
/*																										*/
/*	Remarques : L'antenne doit etre mise en mode MAINTENANCE !!!										*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_Reset( ST_BCM_REG_PTR argBcmRegPtr );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_SetConfig																			*/
/*																										*/
/*  But	: Changement des parametres de configuration de l'antenne										*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR	argBcmRegPtr		: Pointeur sur la structure du gestionnaire				*/
/*		 -> BYTE			argMode				: Mode de prise en compte								*/
/*														BCM_CFG_Immediate								*/
/*														BCM_CFG_Reset									*/
/*		 -> BYTE			argFrequency		: Frequence balise desiree								*/
/*														BCM_CFG_F1										*/
/*														BCM_CFG_F1										*/
/*		 -> BYTE			argBaudRate			: Vitesse de communication								*/
/*														BCM_CFG_1200	(pas pour TGx)					*/
/*														BCM_CFG_2400	(pas pour TGx)					*/
/*														BCM_CFG_4800	(pas pour TGx)					*/
/*														BCM_CFG_9600									*/
/*														BCM_CFG_19200									*/
/*														BCM_CFG_38400									*/
/*														BCM_CFG_57600									*/
/*														BCM_CFG_115200									*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							BCM_ErrMemory		: Si on n'a pas pu empiler la nouvelle commande			*/
/*							BCM_CommAborted		: Si il y a eu probleme de communication avec l'antenne	*/
/*							BCM_CommTimeout		: Si aucune reponse a la commande n'a ete recue			*/
/*							BCM_ErrResponse		: Si la reponse est incomprehensible					*/
/*							BCM_ErrEvent		: Si une erreur interne d'evenement est apparue			*/
/*							COMM_PortNotActif	: Si le thread de gestion du port est arrete			*/
/*							COMM_PortOutBusy	: Si le port n'est pas pret								*/
/*							COMM_ErrSetEvent	: Si l'evenement USER n'a pas pu etre envoye			*/
/*							BCM_CdeRefused		: Commande refusee pour l'une des causes suivantes :	*/
/*													- la trame commande est incorrecte					*/
/*													- le mode de l'antenne est incorrect				*/
/*							BCM_PbBeacon		: Commande refusee car il y a un pb avec la balise		*/
/*							BCM_PbParam			: Commande refusee car un parametre est incorrect		*/
/*							BCM_PbFichConfig	: Commande refusee car il y a un pb avec le fichier de	*/
/*													config												*/
/*							BCM_NotConfig		: Commande refusee car le TGx n'est pas configure		*/
/*																										*/
/*	Remarques : L'antenne doit etre mise en mode STOPPED ou MAINTENANCE !!!								*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_SetConfig( ST_BCM_REG_PTR	argBcmRegPtr,
								  BYTE				argMode,
								  BYTE				argFrequency,
								  BYTE				argBaudRate );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_GetConfig																			*/
/*																										*/
/*  But	: Lecture des parametres de configuration de l'antenne 											*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR		argBcmRegPtr	: Pointeur sur la structure du gestionnaire				*/
/*		<-	ST_BCM_CONFIG_PTR	argConfigPtr	: Pointeur sur une structure remplie avec la config		*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							BCM_ErrMemory		: Si on n'a pas pu empiler la nouvelle commande			*/
/*							BCM_CommAborted		: Si il y a eu probleme de communication avec l'antenne	*/
/*							BCM_CommTimeout		: Si aucune reponse a la commande n'a ete recue			*/
/*							BCM_ErrResponse		: Si la reponse est incomprehensible					*/
/*							BCM_ErrEvent		: Si une erreur interne d'evenement est apparue			*/
/*							COMM_PortNotActif	: Si le thread de gestion du port est arrete			*/
/*							COMM_PortOutBusy	: Si le port n'est pas pret								*/
/*							COMM_ErrSetEvent	: Si l'evenement USER n'a pas pu etre envoye			*/
/*							BCM_CdeRefused		: Commande refusee pour l'une des causes suivantes :	*/
/*													- la trame commande est incorrecte					*/
/*													- le mode de l'antenne est incorrect				*/
/*							BCM_PbFichConfig	: Commande refusee car il y a un pb avec le fichier de	*/
/*													config												*/
/*							BCM_NotConfig		: Commande refusee car le TGx n'est pas configure		*/
/*																										*/
/*	Remarques :	L'antenne doit etre mise en mode STOPPED ou MAINTENANCE !!!								*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_GetConfig( ST_BCM_REG_PTR argBcmRegPtr, ST_BCM_CONFIG_PTR argConfigPtr );


/*------------------------------------------------------------------------------------------------------*/
/*	Fonction : BCM_GetBeaconID																			*/
/*																										*/
/*  But	: Lecture du beacon ID de l'antenne																*/
/*																										*/
/*  Parametres :																						*/
/*		 -> ST_BCM_REG_PTR			argBcmRegPtr	: Pointeur sur la structure du gestionnaire			*/
/*		<-	BYTE					*argBeaconIDPtr	: Pointeur sur un buffer remplie avec le beacon ID	*/
/*																										*/
/*  Retour :																							*/
/*		<-  BCM_ERR	: Resultat de l'operation															*/
/*							BCM_NoErr			: Si tout c'est bien passe								*/
/*							BCM_BadParam		: Si un parametre est incorrect							*/
/*							BCM_ErrMemory		: Si on n'a pas pu empiler la nouvelle commande			*/
/*							BCM_CommAborted		: Si il y a eu probleme de communication avec l'antenne	*/
/*							BCM_CommTimeout		: Si aucune reponse a la commande n'a ete recue			*/
/*							BCM_ErrResponse		: Si la reponse est incomprehensible					*/
/*							BCM_ErrEvent		: Si une erreur interne d'evenement est apparue			*/
/*							COMM_PortNotActif	: Si le thread de gestion du port est arrete			*/
/*							COMM_PortOutBusy	: Si le port n'est pas pret								*/
/*							COMM_ErrSetEvent	: Si l'evenement USER n'a pas pu etre envoye			*/
/*							BCM_CdeRefused		: Commande refusee car la trame commande est incorrecte	*/
/*							BCM_NotConfig		: Commande refusee car le TGx n'est pas configure		*/
/*																										*/
/*	Remarques :	En mode TRANSPARENT, le BeaconID renvoye correspond au dernier BeaconID envoye par		*/
/*				l'antenne vers le badge.																*/
/*				En mode STOPPED ou MAINTENANCE, le BeaconID renvoye correspond au BeaconID stocke dans	*/
/*				la memoire de l'antenne.																*/
/*------------------------------------------------------------------------------------------------------*/
BCM_ERR BCM_EXPORT BCM_GetBeaconID( ST_BCM_REG_PTR argBcmRegPtr, BYTE *argBeaconIDPtr );


#ifdef __cplusplus
}
#endif

#endif

/********************************************************************************************************/
