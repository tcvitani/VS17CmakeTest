/************** (v) 2017 EMOVIS - All rights reserved ***************/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Afficheur ext‚rieur
* FICHIER: AFF_IOS_ERI.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Code de la tache AFF_IOS
* --------------------------------------------------------------------
* DESCRIPTION: La tache AFF_IOS est la tache qui gŠre le p‚ripherique.
*              Elle communique avec la tache AFF_ANI uniquement.
*
* --------------------------------------------------------------------
* HISTORIQUE:
*/

/*--------------- INCLUDES: ---------------*/
#include <mbstring.h>

//#include <convert.h>
#include <noyau.h>
#include <serie.h>
#include <debug.h>
#include <err.h>
#include <reg.h>
#include <fic_gere.h>
#include <horodate.h>

#include <aff_ext.h>
#include <aff_util.h>
#include <aff_glob.h>
#define LOC_DEF
#include <aff_ios_eri.h>
#undef LOC_DEF
#include <aff_mess.h>

/*--------------- RESERVED: ---------------*/

#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/

#define AFFIOS_EXECUTE_ALL_REQUEST          FALSE
#define AFFIOS_EXECUTE_LAST_REQUEST_ONLY    TRUE

#define AFF_ERI_FIRMWARE_VER_RESPONSE_LEN	40
#define AFF_ERI_ROW_CHAR_COUNT_OLD_MODEL	31	// Text to old ERI display model is sent in one line for whole display, and thus, it has max 31 characters
#define AFF_ERI_ROW_CHAR_COUNT_NEW_MODEL	10	// Text to new ERI display model is sent in three separate lines, and thus, it has max 10 characters for each line

/*--------------- TYPEDEFS: ---------------*/
// Enumeration used for executing single command to the display
typedef enum
{
	COMMAND_GET_FIRMWARE_VERSION = 0x01
}ERI_command;

typedef struct
{
	UCHAR		aucCmdToSend[AFF_LIGNE_MAX];
	short int	iCmdNbBytes;
	BOOL		bSendCmd;
	UCHAR		szText[AFF_LIGNE_MAX];
}
struct_line_cmd;

/*--------------- FUNCTIONS: ---------------*/

PROTECTED void InitAffIos( short int ident_aff );

PRIVATE int AFFPurgeBALFiltranteEtExecute(short ident_aff, BOOL flag_only_last);
PRIVATE void AFFEnvoiAniEtat(short ident_aff, struct_b_etat_aff sEtat);
PRIVATE void AFFAffichageLignes( short ident_aff, short priority);
PRIVATE void AFFClearDisplay(short ident_aff);
PRIVATE void AFFResetHW( short ident_aff);
PRIVATE void AFFChoixTexteLigne(short ident_aff, 
								struct_serie_peripherique *aff_periph,
								struct_list_text *psTextList,
								enum_eri_line eLine);
PRIVATE void EriSetDateTime( short ident_aff);

PRIVATE void AFFNouvelleVisu( short ident_aff, char * ligne1, char * ligne2, char * ligne3 );

PROTECTED boolean AFFDeviceInfoCmd(short int ident_aff, ERI_command command);
PRIVATE unsigned char CalculateChecksum(unsigned char *mbsCommand, short int iCmdCounter);
PRIVATE UINT CountNonPrintableCharacters(unsigned char *pSource, size_t sourceSize);
PRIVATE BOOL ReadDeviceInfoResponse(short	ident_aff,
									ERI_command command,
									_AFF_ANI_DEV_INFO_MSG *pDevInfoMsg);
PRIVATE void AFFGetDeviceInfo(short int ident_aff, struct_neutre *p_neutre);
PRIVATE void CleanUpBeforeExit(short int ident_aff);

PRIVATE void AFFSendLineCommands(short ident_aff);
PRIVATE void ReceptionHorloge(short ident_aff, struct_neutre *p_neutre);

/*--------------- VARIABLES: ---------------*/

struct_line_cmd asLineCmd[ERI_LINE_END];

/*--------------- CODE:      ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: DWORD AffIos(void * pvFoo)
* PARAMETRES:
*    entree : pvFoo : Pour conformité Win32
*    retour : aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme principal de la tache temps-r‚el aff_ios
* ROLE: Receptionner les messages qui arrivent de la tache Aff_ani
*       et dialoguer avec le p‚riph‚rique par la liaison s‚rie.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED DWORD WINAPI AffIos(void * pvFoo)
{
	struct_neutre     *p_neutre = (struct_neutre *)(0);
	noyau_bal_id      aff_ios_bal;
	noyau_bal_id      aff_ani_bal;
	short             ident_aff = (short int)(pvFoo);
	struct_serie_peripherique *aff_periph = (struct_serie_peripherique *)(0);
    
    AffFichierTrace( ident_aff, "NOM - Instance(%s) - IOS tid=%u/0x%08X",
            AFF[ident_aff].nom_bal_ani,
            GetCurrentThreadId(),
            GetCurrentThreadId() );

    /* Initialization of the task and change of priority */
	InitAffIos( ident_aff);
	
	/* r‚cuperation du num‚ro de la boite aux lettres IOS */
	aff_ios_bal = AFF[ident_aff].ios_bal;
	
	/* recuperation du num‚ro de la boite aux lettres ANI */
	aff_ani_bal = AFF[ident_aff].ani_bal;
	
	/* positionnement du pointeur sur l adresse  */
	aff_periph = (struct_serie_peripherique *)&AFF[ident_aff].gestion_peripherique;
	
	ChangePriorite( TacheCourante(), AFF[ident_aff].priorite_max);
	
	AFFNouvelleVisu( ident_aff, "", "", "");

	DebutRegionLocale();
	
	AFFClearDisplay(ident_aff);
	
	FinRegionLocale();

	
	/*************************/
	/* Periodic processing	 */
	/*************************/
	
	while ( TRUE )
	{
		DebutRegionLocale();

        /* Purger les messages de la BAL avec le mode de purge choisi */
        AFFPurgeBALFiltranteEtExecute (ident_aff, AFFIOS_EXECUTE_ALL_REQUEST);
		
		FinRegionLocale();

		// Condition for thread exit
		if (AFF[ident_aff].bIosThreadStopRequest)
			break;
		        
        DelaiTache( 2 );
	}

	CleanUpBeforeExit(ident_aff);
	return 0;	
}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: short InitAffIos(void)
* PARAMETRES:
*     entree: rien
*     retour: numero d'identificateur de l'imprimante
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Initialisation des variables utiles … la tache IOS
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void InitAffIos( short int ident_aff )
{
    struct_serie_peripherique *aff_periph = NULL;
	
	/**************************************************************/
	/* Reset flag which tells if IOS thread has to be stopped	  */
	/**************************************************************/	
	AFF[ident_aff].bIosThreadStopRequest = FALSE;
	
	/***********************************************/
	/* Initialisation de la structure peripherique */
	/***********************************************/
	aff_periph = /*J(struct_serie_peripherique *)*/&AFF[ident_aff].gestion_peripherique;
	
	/* Init buffers emission et reception */
	memset( aff_periph->bufrec, 0xFF, LG_TRAME_MAX);
	memset( aff_periph->bufemi, 0xFF, LG_TRAME_MAX);
	
	/* liaison OK au demarrage */
	memset( &aff_periph->b_etat, FALSE, sizeof( struct_srv_etat));
	aff_periph->err_rec = 0;
	aff_periph->err_emi = 0;
	aff_periph->nb_tentatives = 0;
	
	/* par defaut */
	aff_periph->longueur_rep = 0/*LG_REP_STATUS*/;
	
	/* Pas de commande en cours */
	aff_periph->commande = FALSE;
	/* Pas de reponse … attendre */
	aff_periph->ack = TRUE;
	
	/* Vider les buffers de reception et d'emission */
	SerRazTampon( aff_periph->port , SER_TAMPON_RECEPTION);
	SerRazTampon( aff_periph->port , SER_TAMPON_EMISSION);
	
	// Publication de la BAL IOS
	AFF[ident_aff].ios_bal = PublieBAL (AFF[ident_aff].nom_bal_ios, NOYAU_BAL_ILLIMITEE );
	if (AFF[ident_aff].ios_bal <= 0)
		ExitBad();
	
	// Attente publication de la BAL ANI
	AFF[ident_aff].ani_bal = AttendBAL (AFF[ident_aff].nom_bal_ani);
	if (AFF[ident_aff].ani_bal <= 0)
		ExitBad();

	EriSetDateTime(ident_aff);
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE int AFFPurgeBALFiltranteEtExecute ( short ident_aff, BOOL flag_only_last )
 * PARAMETERS: short ident_aff     : identificateur de l'afficheur
 *             BOOL flag_only_last : 
 *               AFFIOS_EXECUTE_LAST_REQUEST_ONLY  = pour chaque service possible, executer uniquement la derniere requete
 *               AFFIOS_EXECUTE_ALL_REQUEST = pour chaque service possible, executer toutes les requetes
  * RETURN    : nombre de messages lus dans la BAL.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Purge la BAL en une seule passe et execute toutes ou la derniere requete de chaque service possible.
 * --------------------------------------------------------------------
 */
PRIVATE int AFFPurgeBALFiltranteEtExecute(short ident_aff, BOOL flag_only_last)
{
	struct_neutre     *p_neutre = (struct_neutre *)(0);
	noyau_enum_retour code_noyau;
	struct_ani__ios *p_ani=(struct_ani__ios *)(0);
    int nb_msg = 0;
    struct struct_derniere_requete
    {
        int arret;
        int etat;
        int affichage;
        int allume;
		int reset_hw;
    }
    derniere_requete;
    
    /* Initialiser structure derniere requete a aucune demande */
    derniere_requete.arret = -1;
    derniere_requete.etat = -1;
    derniere_requete.affichage = -1;
    derniere_requete.allume = -1;
    derniere_requete.reset_hw = -1;

	while ((code_noyau = TestRecoit( AFF[ident_aff].ios_bal, &p_neutre)) == NOYAU_BAL_MESS)
	{
		if (p_neutre->bl_retour == NOYAU_BAL_HORLOGE_ID)
		{
			ReceptionHorloge(ident_aff, p_neutre);
		}
		else
		{
			nb_msg++;
			if (!flag_only_last)
			{
				/* Initialiser structure derniere requete a aucune demande (a chaque message) */
				derniere_requete.arret = -1;
				derniere_requete.etat = -1;
				derniere_requete.affichage = -1;
				derniere_requete.allume = -1;
				derniere_requete.reset_hw = -1;
			}

			/* initialisation des pointeurs */
			p_ani = (struct_ani__ios *)p_neutre;

			/* Traitement selon requete demandee */
			switch (p_ani->message_id)
			{
				/*********************************/
			case M_ARRET:   /* Requete d'arret de la tache */
				/*********************************/
				AffFichierTrace(ident_aff, "AFF_IOS: ReceptionANI() => demande ARRET ");
				/* positionner l'indicateur de demande d'arret */
				derniere_requete.arret = TRUE;
				break;
				/************************************/
			case M_ETAT:   /* Requete sur l'etat de la liaison */
				/************************************/
				AffFichierTrace(ident_aff, "AFF_IOS: ReceptionANI() => demande ETAT ");
				derniere_requete.etat = TRUE;
				break;

			case M_AFFICHAGE:
				AffFichierTrace(ident_aff, "AFF_IOS: ReceptionANI() => demande AFFICHAGE");
				derniere_requete.affichage = (int)p_ani->u.ani_ios.u.contenu.modif_priorite;
				break;

			case M_ALLUME:
				AffFichierTrace(ident_aff, "AFF_IOS: ReceptionANI() => demande ECLAIRAGE");
				derniere_requete.allume = (int)p_ani->u.ani_ios.u.allume.etat;
				break;

			case M_RESET_HW:
				AffFichierTrace(ident_aff, "AFF_IOS: ReceptionANI() => demande RESET HW");
				derniere_requete.reset_hw = TRUE;

			case M_VOYANT:
				break;

			case SRV_TYP_GET:
				AFFGetDeviceInfo(ident_aff, p_neutre);
				break;

			default:
				AffFichierDebug(ident_aff, "AFF_IOS ***** ReceptionANI() => requete '%d' inconnue *****",
					p_ani->message_id);
				break;
			}

			if (!flag_only_last)
			{
				/* Verifier structure requete et si demande, effectuer l'action appropriee */
				if (derniere_requete.arret != -1)
					AFF[ident_aff].bIosThreadStopRequest = TRUE;
				if (derniere_requete.affichage != -1)
					AFFCommande(ident_aff, M_AFFICHAGE, (void *)derniere_requete.affichage);
				if (derniere_requete.allume != -1)
					AFFCommande(ident_aff, M_ALLUME, (void *)derniere_requete.allume);
				if (derniere_requete.reset_hw != -1)
					AFFCommande(ident_aff, M_RESET_HW, (void *)derniere_requete.reset_hw);
			}
		}
        /* Liberer la memoire occupee par le message traite */
        ExitLibere( &p_neutre );
    }	

    if (flag_only_last)
    {
        /* Verifier structure derniere requete et si demande, effectuer l'action appropriee */
        if (derniere_requete.arret != -1)
			AFF[ident_aff].bIosThreadStopRequest = TRUE;

//        Pas de demande d'etat car afficheur sans protocole
//        if (derniere_requete.etat != -1)
//            AFFCommande( ident_aff, M_ETAT, NULL);

        if (derniere_requete.affichage != -1)
            AFFCommande(ident_aff, M_AFFICHAGE, (void *)derniere_requete.affichage);
        if (derniere_requete.allume != -1)
            AFFCommande(ident_aff, M_ALLUME, (void *)derniere_requete.allume);
		if (derniere_requete.reset_hw != -1)
				AFFCommande(ident_aff, M_RESET_HW, (void *)derniere_requete.reset_hw);
    }

    if (nb_msg > 0)
	    AffFichierTrace( ident_aff, "AFF_IOS => %u message(s) lu(s) dans la BAL (mode = %u)", 
            nb_msg, (unsigned)flag_only_last);

    return nb_msg;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFCommande (short ident_aff, uchar cmd,
*                            void *parametre)
* PARAMETRES:
*     entree: identificateur de l'afficheur
*           : commande … envoyer
*           : paramŠtres associ‚s … la commande
*     retour: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Envoi d'une commande vers l'imprimante
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFCommande (short ident_aff, UCHAR cmd,
                            void *parametre)
{
	
	/* en fonction de la commande */
	/* recopie des parametres et d‚termination de la longueur */
	/* de la r‚ponse attendue suite … la commande */
	switch( cmd )
	{
	case M_AFFICHAGE :
		AFFAffichageLignes( ident_aff, (short)parametre);
		break;
		
	case M_ETAT :
		AFFEnvoiAni( ident_aff, M_CHANGEMENT_ETAT, NULL);
		break;
		
	case M_ALLUME :
		break;
	case M_RESET_HW:
		AFFResetHW( ident_aff );
		break;
	default :
		AffFichierDebug(ident_aff,"AFFCommande() => Commande '%d' inconnue",cmd);
		break;
	}
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: Ser_enum_cpt_erreur_t AFFEcritMessageSerie(short ident_aff, short port,
char * buffer, short nb_octets)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : numero de port associ‚
*           : buffer contenant les donn‚es … transmettre
*           : nombre d'octets des donn‚es
*     retour: error description
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Envoi d'un message sur la liaison SERIE
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED Ser_enum_cpt_erreur_t AFFEcritMessageSerie(short ident_aff, short port,
                                    UCHAR * buffer, short nb_octets)
{
	Ser_enum_cpt_erreur_t	result;
	UCHAR					szMsg[MAX_PATH]	= {'\0'};
	UCHAR					szBuffer[5]		= {'\0'};
	short					sCounter		= 0;
	UCHAR					szMsgTemp[MAX_PATH]={'\0'};

	for( sCounter = 0; sCounter < nb_octets; sCounter++)
	{
		if( buffer[sCounter] < 32 || buffer[sCounter] > 125 )
			sprintf_s(szBuffer, sizeof(szBuffer), "%02X", buffer[sCounter]);
		else
			sprintf_s(szBuffer, sizeof(szBuffer), "%c", buffer[sCounter]);
		strcat_s(szMsg, sizeof(szMsg), szBuffer);
	}
	
	AffFichierTrace( ident_aff, "IOS: To serial port =>  '%s' ", szMsg );

	/* envoi d'une commande sur la liaison serie */
	result = SerEcritMessage(port, buffer, nb_octets );
	if ( result != SER_CPT_OK )
		/* erreur … l'‚mission */
		AFFErreurLiaison(ident_aff, SER_TAMPON_EMISSION);
	
	/* si dialogue operateur demande */
	//if( AFF[ident_aff].temoin_dop_sortant )
	AFFEnvoiMessageEspion(ident_aff,(UCHAR *)buffer, nb_octets, SRV_ESPION_MESSAGE_DONNEES, SRV_ESPION_SORTANT);

	return result;
}

/**/
/*******************************************************************/
/*SYNTAX: Ser_enum_cpt_erreur_t AFFReadMessageFromPort(			   */
/*											short ident_aff,	   */
/*											BYTE *byDataBuffer,	   */
/*											int iLength,		   */
/*											int *iReadData)		   */
/*=================================================================*/
/*TYPE:   PROTECTED		                                           */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            The function cleans up all data that has been		   */
/*            initialized by this thread.			               */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      short int ident_aff	- Instance ID.				           */
/*		BYTE *byDataBuffer	- buffer for data from COM port		   */
/*		int iLength			- length to be read	(to expect on port)*/
/*		int *iReadData)		- length of data read from port		   */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  Ser_enum_cpt_erreur_t	error description					   */
/*******************************************************************/
PROTECTED Ser_enum_cpt_erreur_t AFFReadMessageFromPort(short ident_aff, BYTE *byDataBuffer, int iLength, int *iReadData)
{
	INT						iLen = MAX_PATH;
	Ser_enum_cpt_erreur_t	port_err;

	FinRegionLocale();
	port_err = SerLitMessage(AFF[ident_aff].gestion_peripherique.port,
							byDataBuffer,
							iLength,
							(unsigned long *)&iLen,
							0);
	DebutRegionLocale();

	if (iLen)
	{
		// Send the message to the ESPION message subscribers
		AFFEnvoiMessageEspion(	ident_aff,
								byDataBuffer,
								(short int)iLen,
								SRV_ESPION_MESSAGE_DONNEES,
								SRV_ESPION_ENTRANT);
	}

	if (port_err != SER_CPT_OK)
	{
		switch (port_err)
		{
			case SER_CPT_NUM_PORT_INCORRECT:
				AffFichierDebug(ident_aff, "AFF_IOS: AFFReadMessageFromPort() => specified port not supported");
				break;

			case SER_CPT_PORT_NON_OUVERT:
				AffFichierDebug(ident_aff, "AFF_IOS: AFFReadMessageFromPort() => specified port not opened");
				break;

			case SER_CPT_DONNEES_INCOMPLETES:
				AffFichierDebug(ident_aff, "AFF_IOS: AFFReadMessageFromPort() => nb char. smaller then the length");
				break;

			case SER_CPT_DONNEES_ABSENTES:
				AffFichierDebug(ident_aff, "AFF_IOS: AFFReadMessageFromPort() => no char. in the rec. buffer");
				break;

			default:
				AffFichierDebug(ident_aff, "AFF_IOS : Error reading port, err id = %d", port_err);
				break;
		}
	}
	
	*iReadData = iLen;

	return port_err;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFErreurLiaison( short ident_aff,
*                                 Ser_enum_type_tampon_t tampon)
* PARAMETRES:
*     entree: ident de l'afficheur
*           : buffer EMISSION ou RECEPTION en ‚chec
*     retour: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Gestion d'une erreur de transmission ou reception
*       Avertir la tache ANI si erreur critique.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFErreurLiaison( short ident_aff,
								Ser_enum_type_tampon_t tampon)
{
	struct_serie_peripherique *aff_periph = &AFF[ident_aff].gestion_peripherique;
	
	switch( tampon )
	{
	case SER_TAMPON_RECEPTION:
		if( aff_periph->err_rec == AFF[ident_aff].MaxLinkError)
		{
            aff_periph->err_rec = 0;
            if( aff_periph->b_etat.liaison_hs == FALSE )
            {
				/* si changement d'‚tat, avertir l'application */
				aff_periph->b_etat.liaison_hs = TRUE;
				aff_periph->b_etat.aff_hs = TRUE;

				AFFEnvoiAniEtat(ident_aff, aff_periph->b_etat);
            }
            /* ne plus consid‚rer la commande en cours */
            aff_periph->commande = FALSE;
		}
		else if( aff_periph->commande )
            /* si une commande est en cours et qu'il n'y a pas de reponse */
            aff_periph->err_rec ++;
		break;
		
	case SER_TAMPON_EMISSION:
		if( aff_periph->err_emi == AFF[ident_aff].MaxLinkError )
		{
			//       AffFichierTrace(ident_aff," ERREUR LIAISON EMISSION");
            aff_periph->err_emi = 0;
            if( aff_periph->b_etat.liaison_hs == FALSE )
            {
				/* si changement d'‚tat, avertir l'application */
				aff_periph->b_etat.liaison_hs = TRUE;
				aff_periph->b_etat.aff_hs = TRUE;

				AFFEnvoiAniEtat(ident_aff, aff_periph->b_etat);
            }
            /* ne plus consid‚rer la commande en cours */
            aff_periph->commande = FALSE;
		}
		else
            aff_periph->err_emi ++;
		break;
		
	default:
		break;
	}
	
	/* Vider les buffers */
	SerRazTampon(aff_periph->port ,SER_TAMPON_EMISSION);
	SerRazTampon(aff_periph->port ,SER_TAMPON_RECEPTION);
	
	/* si liaison HS */
	if( aff_periph->b_etat.liaison_hs )
	{
		/* ne plus consid‚rer la commande en cours */
		aff_periph->commande = FALSE;
	}
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void  AFFEnvoiAni(short ident_aff, enum_ani__ios msg_id,
*                            void * parametres )
* PARAMETRES:
*     entree: ident de l'imprimante
*           : message_type
*           : parametres du messages
*     retour: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: emission de messages a destination de la tache ANI
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFEnvoiAni (short ident_aff, enum_ani__ios msg_id,
                            void *parametre)
{
	struct_ani__ios *p_ani = NULL;
	
	NO_WARNING( parametre);
	
	/* allocation memoire pour le message */
	ExitAlloue ((struct_neutre **)(&p_ani),sizeof(struct_ani__ios),
		AFF[ident_aff].pool);

	/* mise a jour de l'identificateur du message */
	p_ani->message_id = msg_id;
	
	/* commande en fonction du message */
	switch (msg_id)
	{
		case M_ACK_AFFICHAGE:
		case M_ERR_AFFICHAGE:
			break;

		case M_ARRET_EFFECTUE:
			break;

		case M_CHANGEMENT_ETAT:
			memcpy( &p_ani->u.ios_ani.u.uiEtat, parametre, sizeof(unsigned int) );
			break;

		case M_DEVICE_INFO_COMPLETE :
			memcpy( &p_ani->u.ios_ani.u.sDevInfoMsg, parametre, sizeof(_AFF_ANI_DEV_INFO_MSG) );
			break;
		
		default :  /* pas de parametres */
			break;
	}
	
	/* redirection trace (avant le Envoie() car apres le pointeur est libere !) */
	AffFichierTrace( ident_aff, "AFF_IOS: EnvoiANI() =>  msg_id '%d' ",
		p_ani->message_id);
	
	/* postage dans la boite aux lettres */
	if(Envoie (AFF[ident_aff].ani_bal,AFF[ident_aff].ios_bal,(struct_neutre *)p_ani) != NOYAU_OK){
		ExitLibere((struct_neutre **)&p_ani);
	}
}

/**/
/*******************************************************************/
/*SYNTAX: void AFFEnvoiAniEtat(short ident_aff,					   */
/*                             struct_b_etat_aff sEtat)	           */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            The function checks the printer status message and   */
/*            sends it in the animation thread mail box if it is   */
/*            needed.                                              */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      short ident_aff				- Instance ID.                 */
/*      struct_b_etat_aff sEtat		- Printer status.              */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PRIVATE void AFFEnvoiAniEtat(short ident_aff, struct_b_etat_aff sEtat)
{	
	UINT	uiStatus	= 0;
	
	memcpy( &uiStatus, &sEtat, sizeof(UINT) );

	// If the device status has changed
	if (memcmp(&AFF[ident_aff].gestion_peripherique.old_b_etat, &sEtat, sizeof(struct_b_etat_aff)) != 0)
	{
		AFFEnvoiAni(ident_aff, M_CHANGEMENT_ETAT, &uiStatus);
		memcpy(&AFF[ident_aff].gestion_peripherique.old_b_etat, &sEtat, sizeof(struct_b_etat_aff));
	}
}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFClearDisplay(short ident_aff)
* PARAMETRES: aucun
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Procedure
* ROLE: Clear screen
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void AFFClearDisplay(short ident_aff)
{
	struct_serie_peripherique	*aff_periph = &AFF[ident_aff].gestion_peripherique;
	char						szCmdToSend[AFF_LIGNE_MAX + 1]	  = {0};
	char						szText[31] = "                              ";
	short int					iCmdLastIndex					  = 0;
	
	memset( szCmdToSend, 32, AFF_LIGNE_MAX );
	szCmdToSend[AFF_LIGNE_MAX - 1] = 0;

	szCmdToSend[iCmdLastIndex++] = ERI_INDICATE_ADDR_MODE;
	szCmdToSend[iCmdLastIndex++] = ERI_UNIT_ADDRESS + 1;
	szCmdToSend[iCmdLastIndex++] = ERI_TERMINATE_ADDR_MODE;
	szCmdToSend[iCmdLastIndex++] = ERI_TEXT_START;
	szCmdToSend[iCmdLastIndex++] = ERI_1ST_CHAR_POS + ERI_1ST_ROW_START;

	memcpy(&szCmdToSend[iCmdLastIndex], szText, strlen(szText));
	iCmdLastIndex += (short int)strlen(szText);

	szCmdToSend[iCmdLastIndex++] = ERI_TEXT_END;
	szCmdToSend[iCmdLastIndex++] = ERI_END;

	AFFEcritMessageSerie(ident_aff, aff_periph->port, szCmdToSend, iCmdLastIndex);
}

PRIVATE void AFFResetHW( short ident_aff)
{
	struct_serie_peripherique	*aff_periph = &AFF[ident_aff].gestion_peripherique;
	char						szCmdToSend[ AFF_LIGNE_MAX + 1 ] = { 0 };
	UCHAR						iCmdLastIndex = 0;
	
	// Fill with command ...

	//AFFEcritMessageSerie( ident_aff, aff_periph->port, szCmdToSend, (short int)iCmdLastIndex);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFAffichageLignes( short ident_aff, short priority)
* PARAMETRES: la priority modifi‚e par la tache ani
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Procedure
* ROLE: Afficher les message avec la plus haute priorit‚
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void AFFAffichageLignes( short ident_aff, short priority)
{
	short						iPriorityInProgress1	= 0,
								iPriorityInProgress2	= 0,
								iPriorityInProgress3	= 0;
	struct_list_text			*psText1				= NULL,
								*psText2				= NULL,
								*psText3				= NULL;
	struct_list_text			sEmptyText10			= { 'A', "          ", FALSE, NULL };
	struct_request_screen		*p_ligne1, *p_ligne2, *p_ligne3;
	struct_serie_peripherique	*aff_periph = &AFF[ident_aff].gestion_peripherique;
 
	iPriorityInProgress1 = AFF_MsgRequestLinePriority(1, &AFF[ident_aff].sScreen);
	iPriorityInProgress2 = AFF_MsgRequestLinePriority(2, &AFF[ident_aff].sScreen);	
	iPriorityInProgress3 = AFF_MsgRequestLinePriority(3, &AFF[ident_aff].sScreen);

	/* Mettre a jour l'affichage si c'est la priorit‚ … afficher */
	if( iPriorityInProgress1 <= priority || iPriorityInProgress2 <= priority || iPriorityInProgress3 <= priority)
	{
		/* Choix de la ligne 1 */
		if( iPriorityInProgress1 != -1)
		{
			p_ligne1 = &(AFF[ident_aff].sScreen.tab_request[iPriorityInProgress1]);
			psText1 = AFF_MsgResearchLineNumber(p_ligne1, 1);
		}
		else
			psText1 = &sEmptyText10;

		/* Choix de la ligne 2 */
		if( iPriorityInProgress2 != -1)
		{
			p_ligne2 = &(AFF[ident_aff].sScreen.tab_request[iPriorityInProgress2]);
			psText2 = AFF_MsgResearchLineNumber( p_ligne2, 2);
		}
		else
			psText2 = &sEmptyText10;

		/* Choix de la ligne 3 */
		if( iPriorityInProgress3 != -1)
		{
			p_ligne3 = &(AFF[ident_aff].sScreen.tab_request[iPriorityInProgress3]);
			psText3 = AFF_MsgResearchLineNumber( p_ligne3, 3);
		}
		else
			psText3 = &sEmptyText10;

		if (psText1 != 0 && psText1->police == 'A' && psText1->texte[0] != 0)
		{
			// Text to old ERI display model is sent in one line for whole display, and thus, it has max 31 + those non printable characters
			psText1->texte[AFF_ERI_ROW_CHAR_COUNT_OLD_MODEL + CountNonPrintableCharacters(psText1->texte, sizeof(psText1->texte))] = '\0';

			AFFChoixTexteLigne(ident_aff, aff_periph, psText1, ERI_LINE_1);
		}

		if (psText2 != 0 && psText2->police == 'A' && psText2->texte[0] != 0)
		{
			// Text to new ERI display model is sent in three separate lines, and thus, it has max 10 + those non printable characters for each line
			psText2->texte[AFF_ERI_ROW_CHAR_COUNT_NEW_MODEL + CountNonPrintableCharacters(psText2->texte, sizeof(psText2->texte))] = '\0';

			AFFChoixTexteLigne(ident_aff, aff_periph, psText2, ERI_LINE_2);
		}

		if (psText3 != 0 && psText3->police == 'A' && psText3->texte[0] != 0)
		{
			// Text to new ERI display model is sent in three separate lines, and thus, it has max 10 + those non printable characters for each line
			psText3->texte[AFF_ERI_ROW_CHAR_COUNT_NEW_MODEL + CountNonPrintableCharacters(psText3->texte, sizeof(psText3->texte))] = '\0';

			AFFChoixTexteLigne(ident_aff, aff_periph, psText3, ERI_LINE_3);
		}

		// send command to serial port only if DelayCmd chrono is not started.
		// if chrorno is started, the command will be executed when it expires.
		if (AFF[ident_aff].bChronoDelayCmdstarted == FALSE)
			AFFSendLineCommands(ident_aff);
	}
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFChoixTexteLigne( short ident_aff, struct_serie_peripherique *aff_periph,
*									 char *psTextList, size_t max_texte)
* PARAMETRES: aucun
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Procedure
* ROLE: 
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void AFFChoixTexteLigne(short ident_aff, 
								struct_serie_peripherique *aff_periph,
								struct_list_text *psTextList,
								enum_eri_line eLine)
{
	UCHAR				mbsCmdToSend[AFF_LIGNE_MAX];
	UCHAR				mbsTemp[AFF_LIGNE_MAX]		= { 0 };
	short int			iCmdLastIndex				= 0;	
	size_t				sizeToCopy					= 0;
	struct_line_cmd		*psLineSelected = NULL;

	// Fill message with spaces and put '\0' at the end.
	memset( mbsCmdToSend, 32, AFF_LIGNE_MAX );
	mbsCmdToSend[AFF_LIGNE_MAX - 1] = 0;

	mbsCmdToSend[iCmdLastIndex++] = ERI_INDICATE_ADDR_MODE;
	mbsCmdToSend[iCmdLastIndex++] = ERI_UNIT_ADDRESS + 1;
	mbsCmdToSend[iCmdLastIndex++] = ERI_TERMINATE_ADDR_MODE;
	mbsCmdToSend[iCmdLastIndex++] = ERI_TEXT_START;

	// Set line start character
	switch (eLine)
	{
		case ERI_LINE_1:
			mbsCmdToSend[iCmdLastIndex++] = ERI_1ST_CHAR_POS + ERI_1ST_ROW_START;
			break;

		case ERI_LINE_2:
			mbsCmdToSend[iCmdLastIndex++] = ERI_1ST_CHAR_POS + ERI_2ND_ROW_START;
			break;

		case ERI_LINE_3:
			mbsCmdToSend[iCmdLastIndex++] = ERI_1ST_CHAR_POS + ERI_3RD_ROW_START;
			break;

		default:
			mbsCmdToSend[iCmdLastIndex++] = ERI_1ST_CHAR_POS + ERI_1ST_ROW_START;
			break;
	}

	// Add message text depending of policie used
	switch (psTextList->police)
	{
		case 'A':
			memcpy(&mbsTemp[0], psTextList->texte, _mbslen(psTextList->texte));
			if (_mbslen(mbsTemp) < AFF_ERI_ROW_CHAR_COUNT_NEW_MODEL)
			{
				int iFilledSpaces = 0;

				for (int i = (int)_mbslen(mbsTemp); i < AFF_ERI_ROW_CHAR_COUNT_NEW_MODEL; i++)
				{
					mbsTemp[i] = 32;
					mbsTemp[i + 1] = '\0';
					iFilledSpaces++;
				}

				AffFichierDebug( ident_aff, "AFF_ANI: AFFChoixTexteLigne() => PED file WARNING - Message '%s' filled with %d missing space(s)!", psTextList->texte, iFilledSpaces);
			}

			memcpy(&mbsCmdToSend[iCmdLastIndex], mbsTemp, _mbslen(mbsTemp));
			iCmdLastIndex += (short int)_mbslen(mbsTemp);
			break;

		case 'B':
			break;
	}

	mbsCmdToSend[iCmdLastIndex++] = ERI_TEXT_END;
	mbsCmdToSend[iCmdLastIndex++] = ERI_END;

	// memorize line command details
	psLineSelected = &asLineCmd[eLine];
	memcpy(psLineSelected->aucCmdToSend, mbsCmdToSend, sizeof(psLineSelected->aucCmdToSend));
	psLineSelected->iCmdNbBytes = iCmdLastIndex;
	psLineSelected->bSendCmd = TRUE;
	memcpy(psLineSelected->szText, psTextList->texte, sizeof(psLineSelected->szText));

}

/**/
/*
* --------------------------------------------------------------------
* SYNTAXE: void EriSetDateTime( short ident_aff)
* PARAMETRES:
*     entree: identificateur de l'afficheur
*     retour: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: local function
* ROLE: Set display clock date and time
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void EriSetDateTime( short ident_aff)
{
	struct_hrd_date_entree	cur_date;
	struct_hrd_date_sortie	ful_date;
	struct_hrd_heure_hex	cur_time;
	char					buffer[15] = { 0 };

	// Set date
	cur_date.hex = HRDDonneDate();
	cur_date.format = FORMAT_HEX;
	HRDConversionDate( cur_date, AMJ, &ful_date);
	ful_date.hex.annee %= 100;

	buffer[0]	= ERI_INDICATE_ADDR_MODE;
	buffer[1]	= ERI_UNIT_ADDRESS;
	buffer[2]	= ERI_TERMINATE_ADDR_MODE;
	buffer[3]	= ERI_DATE_TIME_START;
	buffer[4]	= 0x30 + ful_date.hex.annee / 10;
	buffer[5]	= 0x30 + ful_date.hex.annee % 10;
	buffer[6]	= 0x30 + ful_date.hex.mois / 10;
	buffer[7]	= 0x30 + ful_date.hex.mois % 10;
	buffer[8]	= 0x30 + ful_date.hex.jour / 10;
	buffer[9]	= 0x30 + ful_date.hex.jour % 10;
	buffer[10]	= 0x30 + ful_date.hex.num_jour_semaine;
	buffer[11]	= ERI_DATE_END;
	buffer[12]	= ERI_TEXT_END;
	buffer[13]	= ERI_END;
	buffer[14]	= 0;

	AFFEcritMessageSerie( ident_aff,
                          AFF[ident_aff].gestion_peripherique.port,
                          buffer,
                          (short int)strlen( buffer));

	// Set time
	cur_time = HRDDonneHeure();
	buffer[0]	= ERI_INDICATE_ADDR_MODE;
	buffer[1]	= ERI_UNIT_ADDRESS;
	buffer[2]	= ERI_TERMINATE_ADDR_MODE;
	buffer[3]	= ERI_DATE_TIME_START;
	buffer[4]	= 0x30 + cur_time.heure / 10;
	buffer[5]	= 0x30 + cur_time.heure % 10;
	buffer[6]	= 0x30 + cur_time.minute / 10;
	buffer[7]	= 0x30 + cur_time.minute % 10;
	buffer[8]	= 0x30 + cur_time.seconde / 10;
	buffer[9]	= 0x30 + cur_time.seconde % 10;
	buffer[10]	= ERI_TIME_END1;
	buffer[11]	= ERI_TIME_END2;
	buffer[12]	= ERI_TEXT_END;
	buffer[13]	= ERI_END;

	AFFEcritMessageSerie( ident_aff,
                          AFF[ident_aff].gestion_peripherique.port,
                          buffer,
                          (short int)strlen( buffer));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFNouvelleVisu( short ident_aff, char * ligne1, char * ligne2, char * ligne3)
* PARAMETRES: ident de l'imprimante, 1st line pointer, 2nd line pointer
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Procedure
* ROLE: Sends the state to VISU service
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void AFFNouvelleVisu( short ident_aff, char * ligne1, char * ligne2, char * ligne3)
{
    DWORD dwSize;
    struct_ani__ios * p_ios = (struct_ani__ios *)(0);

    AffFichierTrace(ident_aff, "AFF_CMD: AFFNouvelleVisu()" );

    // allocation memoire pour le message 
    ExitAlloue( 
       (struct_neutre **)(&p_ios),
       sizeof(*p_ios),
    AFF[ident_aff].pool);

    // mise a jour de l'identificateur du message
    p_ios->message_id = M_CHANGEMENT_VISU;

    // Remplissage des chaines
    dwSize = sizeof(p_ios->u.ios_ani.visu_en_cours[0]);
	strncpy_s(p_ios->u.ios_ani.visu_en_cours[0], sizeof(p_ios->u.ios_ani.visu_en_cours[0]), ligne1, dwSize);
    p_ios->u.ios_ani.visu_en_cours[0][dwSize-1] = '\0';
	
	dwSize = sizeof(p_ios->u.ios_ani.visu_en_cours[0]);
	strncpy_s(p_ios->u.ios_ani.visu_en_cours[1], sizeof(p_ios->u.ios_ani.visu_en_cours[1]), ligne2, dwSize);
	p_ios->u.ios_ani.visu_en_cours[1][dwSize-1] = '\0';

	dwSize = sizeof(p_ios->u.ios_ani.visu_en_cours[0]);
	strncpy_s(p_ios->u.ios_ani.visu_en_cours[2], sizeof(p_ios->u.ios_ani.visu_en_cours[2]), ligne3, dwSize);
	p_ios->u.ios_ani.visu_en_cours[2][dwSize-1] = '\0';

   /* postage dans la boite aux lettres */
	if(Envoie( AFF[ident_aff].ani_bal,AFF[ident_aff].ios_bal,(struct_neutre *)p_ios) != NOYAU_OK){
		ExitLibere((struct_neutre **)&p_ios);
	}
}

/**/
/*****************************************************************************/
/*SYNTAX:	PROTECTED boolean AFFDeviceInfoCmd(short int ident_aff,			 */
/*											   ERI_command command);		 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*				Executes requested command for device info.					 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*							short int		ident_aff	- Instance ID.		 */
/*							ERI_command		command		- type of command	 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  The function is successful.						 */
/*  FALSE                 The function is not successful.					 */
/*****************************************************************************/
PROTECTED boolean AFFDeviceInfoCmd(short int ident_aff, ERI_command command )
{
	struct_serie_peripherique	*aff_periph = &AFF[ident_aff].gestion_peripherique;
	char						mbsCmdToSend[AFF_LIGNE_MAX + 1] = { 0 };

	UCHAR						iCmdLastIndex = 0;

	switch (command)
	{
		case COMMAND_GET_FIRMWARE_VERSION:
			// This is command example from DCP printer.
			// Write commands specialized for this display.
			//szCmd[iCmdCount++]=0x1B;	// ESC
			//szCmd[iCmdCount++]=0x05;	// ENQ	
			//szCmd[iCmdCount++]=0x07;	// 007			
			break;

		// for every single command make single case
	}

	// Clean read buffer before sending message so response can be read.
	SerRazTampon( AFF[ident_aff].gestion_peripherique.port, SER_TAMPON_RECEPTION );
	
	//if (AFFEcritMessageSerie(ident_aff, aff_periph->port, mbsCmdToSend, (short int)iCmdLastIndex) != SER_CPT_OK)
	//{
	//	AffFichierDebug(ident_aff, "AFF_IOS: AFFDeviceInfoCmd() => Error writing command to the port %d",
	//					AFF[ident_aff].gestion_peripherique.port);

	//	return FALSE;
	//}
	//
	//return TRUE;

	// If some newer version of this dispay has device info uncomment above code and delete below code
	AffFichierDebug(ident_aff, "AFF_IOS: AFFDeviceInfoCmd() => Device has cannot return device info! Message isn't sent to the port %d",
						AFF[ident_aff].gestion_peripherique.port);

	return FALSE;
}

/**/
/*****************************************************************************/
/*SYNTAX:	unsigned char CalculateChecksum(unsigned char *mbsCommand, 		 */
/*											short int iCmdCounter)			 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:	- Function calculates checksum for this display				 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*	unsigned char *mbsCommand	- message to calculate checksum				 */
/*	short int iCmdCounter		- last position filled with data inside msg	 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*	vunsigned char			Calculated checksum								 */
/*****************************************************************************/
PRIVATE unsigned char CalculateChecksum(unsigned char *mbsCommand, short int iCmdCounter)
{
	unsigned char ucRet = 0;


	return ucRet;
}

/**/
/*****************************************************************************/
/*SYNTAX:  UINT CountNonPrintableCharacters(unsigned char *pSource,			 */
/*													size_t sourceSize)		 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:	- Function counts number of non printable characters inside  */
/*				given string.												 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*	unsigned char	*pSource		- Source string to count in				 */
/*	size_t			sourceSize		- Source string size					 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*	UINT				- Number of non printable characters				 */
/*****************************************************************************/
PRIVATE UINT CountNonPrintableCharacters(unsigned char *pSource, size_t sourceSize)
{
	int iNumOfOccurences = 0;

	// Find all occurences of non printable charactes inside string and extend string accordingly
	// This is done because ERI display supports line breaks and some other control characters.
	for (int i = 0; i < _mbslen(pSource); i++)
		if (pSource[i] < 0x32)
			iNumOfOccurences++;

	return iNumOfOccurences;
}

/**/
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*    FALSE                   Function failed to read from port, link error	 */
/*****************************************************************************/
PRIVATE BOOL ReadDeviceInfoResponse(short	ident_aff,
									ERI_command command,
									_AFF_ANI_DEV_INFO_MSG *pDevInfoMsg)
{
	BYTE					byDataBuffer[MAX_PATH]	= {0};
	char					szTemp[MAX_PATH]		= {0};
	INT						iLen;
	INT						iResponseLength			= 1;

	// Depending of command, length to receive from serial port is known
	switch (command)
	{
		case COMMAND_GET_FIRMWARE_VERSION:
			iResponseLength = AFF_ERI_FIRMWARE_VER_RESPONSE_LEN;
			break;
	}
	
	
	if (AFFReadMessageFromPort(ident_aff, byDataBuffer, iResponseLength, &iLen) != SER_CPT_OK)
		return FALSE; // Link error

	// Depending of command, it is known which data to store
	switch (command)
	{
		case COMMAND_GET_FIRMWARE_VERSION:
			strcpy_s(pDevInfoMsg->sDeviceInfoList.items[pDevInfoMsg->sDeviceInfoList.nb_item].name,
					sizeof(pDevInfoMsg->sDeviceInfoList.items[pDevInfoMsg->sDeviceInfoList.nb_item].name), "Firmware version");
			memcpy_s(szTemp, sizeof(szTemp), &byDataBuffer[4], 17 * sizeof(szTemp[0]));
			sprintf_s(pDevInfoMsg->sDeviceInfoList.items[pDevInfoMsg->sDeviceInfoList.nb_item].data,
					sizeof(pDevInfoMsg->sDeviceInfoList.items[pDevInfoMsg->sDeviceInfoList.nb_item].data),
					"%s", szTemp);
			//pDevInfoMsg->sDeviceInfoList.nb_item++;
			break;			
	}
	return TRUE;
}

/**/
/*******************************************************************/
/*SYNTAX: void AFFGetDeviceInfo(short int ident_aff,			   */
/*								struct_neutre *p_neutre)		   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            The function gets all device info data.	           */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      short int		ident_aff	- Instance ID.                 */
/*		struct_neutre	*p_neutre	- struct with bal id		   */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PRIVATE void AFFGetDeviceInfo(short int ident_aff, struct_neutre *p_neutre)
{
	_AFF_ANI_DEV_INFO_MSG sDevInfoMsg = {0};

	struct_ani__ios *pMsgTemp = (struct_ani__ios*)p_neutre;

	sDevInfoMsg.neutre = pMsgTemp->u.ani_ios.u.sDevInfoMsg.neutre;

	// Reset nb_item that ReadDeviceInfoResponse can load nb_item correctly
	sDevInfoMsg.sDeviceInfoList.nb_item = 0;

	// This device can't give device info. This if-else and corresponding functions are for example.
	if ( AFFDeviceInfoCmd(ident_aff, COMMAND_GET_FIRMWARE_VERSION) )
		ReadDeviceInfoResponse(ident_aff, COMMAND_GET_FIRMWARE_VERSION, &sDevInfoMsg);
	else
	{
		// Check if device info message is empty and fill it with message 
		// so user get some message because this device can't give device info.
		strcpy_s(sDevInfoMsg.sDeviceInfoList.items[sDevInfoMsg.sDeviceInfoList.nb_item].name,
			sizeof(sDevInfoMsg.sDeviceInfoList.items[sDevInfoMsg.sDeviceInfoList.nb_item].name), "Info");
		sprintf_s(sDevInfoMsg.sDeviceInfoList.items[sDevInfoMsg.sDeviceInfoList.nb_item].data,
			sizeof(sDevInfoMsg.sDeviceInfoList.items[sDevInfoMsg.sDeviceInfoList.nb_item].data),
			"Device didn't return any device info");
		sDevInfoMsg.sDeviceInfoList.nb_item++;
	}
	
	AFFEnvoiAni (ident_aff, M_DEVICE_INFO_COMPLETE, &sDevInfoMsg);
}

/**/
/*******************************************************************/
/*SYNTAX: void CleanUpBeforeExit(short int ident_aff)			   */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            The function cleans up all data that has been		   */
/*            initialized by this thread.			               */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      short int ident_aff			- Instance ID.		           */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void														   */
/*******************************************************************/
PRIVATE void CleanUpBeforeExit(short int ident_aff)
{
	AFFEnvoiAni(ident_aff, M_ARRET_EFFECTUE, NULL);

	// Cleaning mailbox
	AFFLibereBAL(AFF[ident_aff].ios_bal);
	// Closing mailbox
	SupprimeBAL(AFF[ident_aff].nom_bal_ios);

	AffFichierTrace(ident_aff, "AFF_IOS: CleanUpBeforeExit() => Ios thread cleaning up complete!");
}


PRIVATE void AFFSendLineCommands(short ident_aff)
{
	struct_serie_peripherique	*aff_periph = &AFF[ident_aff].gestion_peripherique;
	int							i = 0;
	UCHAR						aszTextLine[ERI_LINE_END][AFF_LIGNE_MAX] = { 0 };
	BOOL						bMsgSentToPort = FALSE;

	for (i=ERI_LINE_FIRST; i<=ERI_LINE_LAST; i++)
	{
		if (asLineCmd[i].bSendCmd)
		{
			asLineCmd[i].bSendCmd = FALSE;
			memcpy(aszTextLine[i], asLineCmd[i].szText, sizeof(aszTextLine[i]));
			AFFEcritMessageSerie(ident_aff, aff_periph->port, asLineCmd[i].aucCmdToSend, asLineCmd[i].iCmdNbBytes);
			bMsgSentToPort = TRUE;
		}
	}

	// if any line changed
	if (bMsgSentToPort)
	{
		// Set new visu state
		AFFNouvelleVisu(ident_aff, aszTextLine[ERI_LINE_1], aszTextLine[ERI_LINE_2], aszTextLine[ERI_LINE_3]);

		// start command delay timer
		if (AFF[ident_aff].dwDelayCmdTimeout > 0)
		{
			LanceChronoMs(AFF[ident_aff].uiChronoDelayCmd, AFF[ident_aff].dwDelayCmdTimeout, AFF[ident_aff].ios_bal);
			AFF[ident_aff].bChronoDelayCmdstarted = TRUE;
		}
	}
}

PRIVATE void ReceptionHorloge(short ident_aff, struct_neutre *p_neutre)
{
	struct_chrono *p_chrono = (struct_chrono *)(p_neutre);

	if (p_chrono->numero == AFF[ident_aff].uiChronoDelayCmd)
	{
		if (AFF[ident_aff].bChronoDelayCmdstarted)
		{
			AFF[ident_aff].bChronoDelayCmdstarted = FALSE;
			AFFSendLineCommands(ident_aff);
		}
	}
	else
	{
		AffFichierDebug(ident_aff, "AFF_IOS: ReceptionHorloge()->unhandled timer!");
	}
}