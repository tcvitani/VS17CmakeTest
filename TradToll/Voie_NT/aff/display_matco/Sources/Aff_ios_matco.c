/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Afficheur ext‚rieur
* FICHIER: AFF_IOS.c
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
#include <dos.h>
#include <string.h>
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include <noyau.h>
#include <serie.h>
#include <debug.h>
#include <err.h>
#include <reg.h>
#include <fic_gere.h>

#include <aff_ext.h>
#include <aff_util.h>
#include <aff_glob.h>
#define LOC_DEF
#include <aff_ios_matco.h>
#undef LOC_DEF
#include <aff_mess.h>

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/

#define AFFIOS_EXECUTE_ALL_REQUEST          FALSE
#define AFFIOS_EXECUTE_LAST_REQUEST_ONLY    TRUE

#define AFF_FIRMWARE_VER_RESPONSE_LEN		2

/*--------------- TYPEDEFS: ---------------*/
// Enumeration used for executing single command to the printer
typedef enum
{
	COMMAND_GET_FIRMWARE_VERSION = 0x01
}MATCO_command;

/*--------------- FUNCTIONS: ---------------*/

PROTECTED void InitAffIos( short int ident_aff );

PRIVATE int AFFPurgeBALFiltranteEtExecute(short ident_aff, BOOL flag_only_last);
PRIVATE void AFFAffichageLignes( short ident_aff, short priorite);
PRIVATE void AFFRazAfficheur( short ident_aff);
PRIVATE void AFFEndComm(short ident_aff, short port);
PRIVATE void AFFChoixTexteLigne(short ident_aff, 
								struct_serie_peripherique *aff_periph,
								char *texte,
								enum_matco_line eLine);
PRIVATE UCHAR AFFCalculateLRC(char *texte);
// <-- NBL
PRIVATE void AFFNouvelleVisu( short ident_aff, char * ligne1, char * ligne2, char *ligne3);
// NBL -->
PRIVATE void AFFAfficheurLedsCmd(short ident_aff, unsigned char ucCmd);

PROTECTED boolean AFFDeviceInfoCmd(short int ident_aff, MATCO_command command);
PRIVATE BOOL ReadDeviceInfoResponse(short	ident_aff,
									MATCO_command command,
									_AFF_ANI_DEV_INFO_MSG *pDevInfoMsg);
PRIVATE void AFFGetDeviceInfo(short int ident_aff, struct_neutre *p_neutre);
PRIVATE void CleanUpBeforeExit(short int ident_aff);
/*--------------- VARIABLES: ---------------*/

PRIVATE char last_texte1[AFF_LIGNE_MAX];
PRIVATE char last_texte2[AFF_LIGNE_MAX];
PRIVATE char last_texte3[AFF_LIGNE_MAX];

/*--------------- CODE: ---------------*/

/************************************************************************\
*
*  FUNCTION: CalcCheckSum();
*
*  PURPOSE:  Calculates checksum for Matco display
*
\************************************************************************/
unsigned short CalcCheckSum( unsigned char* pszC, int iStart, int iEnd )
{

	unsigned short sSum = 0, i = 0;
	for(i=iStart;i<=iEnd;i++)
	{
		sSum += pszC[i];
	}

	sSum = ~sSum;
	
	return sSum;
}

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
    
// <-- NBL
    AffFichierTrace( ident_aff, "MATCO - Instance(%s) - IOS tid=%u/0x%08X",
            AFF[ident_aff].nom_bal_ani,
            GetCurrentThreadId(),
            GetCurrentThreadId() );
// NBL -->

    /* initialisation de la tache et changement de priorite */
	InitAffIos( ident_aff);
	
	/* r‚cuperation du num‚ro de la boite aux lettres IOS */
	aff_ios_bal = AFF[ident_aff].ios_bal;
	
	/* recuperation du num‚ro de la boite aux lettres ANI */
	aff_ani_bal = AFF[ident_aff].ani_bal;
	
	/* positionnement du pointeur sur l adresse  */
	aff_periph = (struct_serie_peripherique *)&AFF[ident_aff].gestion_peripherique;
	
	ChangePriorite( TacheCourante(), AFF[ident_aff].priorite_max);
	
	AFFNouvelleVisu( ident_aff, "", "", "");
// NBL -->

	DebutRegionLocale();
	// appel de la fonction de remise a zero de l afficheur
	AFFRazAfficheur( ident_aff);
	
	FinRegionLocale();

	
	/*************************/
	/* Traitement periodique */
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
	/* On positionne le temoin qui permet de savoir si l'arret de */
	/* la tache a ete demande                                     */
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
    }
    derniere_requete;
    
    /* Initialiser structure derniere requete a aucune demande */
    derniere_requete.arret = -1;
    derniere_requete.etat = -1;
    derniere_requete.affichage = -1;
    derniere_requete.allume = -1;

	while ((code_noyau = TestRecoit( AFF[ident_aff].ios_bal, &p_neutre)) == NOYAU_BAL_MESS)
	{
        nb_msg ++;
        if (!flag_only_last)
        {
            /* Initialiser structure derniere requete a aucune demande (a chaque message) */
            derniere_requete.arret = -1;
            derniere_requete.etat = -1;
            derniere_requete.affichage = -1;
            derniere_requete.allume = -1;
        }

        /* initialisation des pointeurs */
        p_ani = (struct_ani__ios *)p_neutre;
        
        /* Traitement selon requete demandee */
        switch (p_ani->message_id)
        {
            /*********************************/
        case M_ARRET :   /* Requete d'arret de la tache */
            /*********************************/
            AffFichierTrace(ident_aff,"AFF_IOS: ReceptionANI() => demande ARRET ");
            /* positionner l'indicateur de demande d'arret */
            derniere_requete.arret = TRUE;
            break;
            /************************************/
        case M_ETAT :   /* Requete sur l'etat de la liaison */
            /************************************/
            AffFichierTrace(ident_aff,"AFF_IOS: ReceptionANI() => demande ETAT ");
            derniere_requete.etat = TRUE;
            break;
            
        case M_AFFICHAGE :
            AffFichierTrace(ident_aff,"AFF_IOS: ReceptionANI() => demande AFFICHAGE");
            derniere_requete.affichage = (int)p_ani->u.ani_ios.u.contenu.modif_priorite;
            break;
            
        case M_ALLUME :
            AffFichierTrace(ident_aff,"AFF_IOS: ReceptionANI() => demande ECLAIRAGE");
            derniere_requete.allume = (int)p_ani->u.ani_ios.u.allume.etat;
            break;
            
		case SRV_TYP_GET:
			AFFGetDeviceInfo(ident_aff, p_neutre);
			break;

        default :
            AffFichierDebug(ident_aff,"AFF_IOS ***** ReceptionANI() => requete '%d' inconnue *****",
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
	default :
		AffFichierDebug(ident_aff,"AFFCommande() => Commande '%d' inconnue",cmd);
		break;
	}
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFEcritMessageSerie(short ident_aff, short port,
char * buffer, short nb_octets)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : numero de port associ‚
*           : buffer contenant les donn‚es … transmettre
*           : nombre d'octets des donn‚es
*     retour: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Envoi d'un message sur la liaison SERIE
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFEcritMessageSerie(short ident_aff, short port,
                                    unsigned char * buffer, short nb_octets)
{
	Ser_enum_cpt_erreur_t result;
	UCHAR					szMsg[1024]={0};
	unsigned char   szDataReceived[MAX_PATH] = {0};
	DWORD dwDataRead = 0;
	short sCounter;

	for( sCounter = 0; sCounter < nb_octets; sCounter++)
	{
		UCHAR szBuffer[4] = {0};

		if( buffer[sCounter] < 32 || buffer[sCounter] > 125 )
		{
			sprintf_s(szBuffer, sizeof(szBuffer), "%02X", buffer[sCounter] );			
		}
		else
		{
			sprintf_s(szBuffer, sizeof(szBuffer), "%c", buffer[sCounter] );	
		}
		strcat_s(szMsg, sizeof(szMsg), szBuffer);
	}

	AffFichierTrace( ident_aff, "IOS: To serial port =>  '%s' ", szMsg );
	
	
	/* envoi d'une commande sur la liaison serie */
	result = SerEcritMessage(port, buffer, nb_octets );
	if( result != SER_CPT_OK )
		/* erreur … l'‚mission */
		AFFErreurLiaison(ident_aff,SER_TAMPON_EMISSION);

	/* si dialogue operateur demande */
	//if( AFF[ident_aff].temoin_dop_sortant )
	AFFEnvoiMessageEspion(ident_aff,(UCHAR *)buffer, nb_octets, SRV_ESPION_MESSAGE_DONNEES, SRV_ESPION_SORTANT);

	result = SerLitMessage( port, szDataReceived, 3, &dwDataRead, 2 );
	if( result != SER_CPT_OK )
	{
		/* erreur … l'‚mission */
		AFFErreurLiaison(ident_aff,SER_TAMPON_RECEPTION);
	}
	else
	{
		if(dwDataRead > 0)
		{
			int i = 0;
			char szBuff[MAX_PATH] = {0};
			sprintf_s(szBuff, sizeof(szBuff), "<--: 0x%02X", szDataReceived[0]);
   
			// we got ACK from port in any of received bytes
			for(i=0;i<(int)dwDataRead;i++)
			{
				if(szDataReceived[i] == ACK)
				{
					AFFEndComm(ident_aff, port);
					break;
				}
			}
		}
	}

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
		if( aff_periph->err_rec == AFF_NB_ERREURS_MAX)
		{
            aff_periph->err_rec = 0;
            if( aff_periph->b_etat.liaison_hs == FALSE )
            {
				/* si changement d'‚tat, avertir l'application */
				aff_periph->b_etat.liaison_hs = TRUE;
				aff_periph->b_etat.aff_hs = TRUE;
				AFFEnvoiAni(ident_aff, M_CHANGEMENT_ETAT, NULL);
            }
            /* ne plus consid‚rer la commande en cours */
            aff_periph->commande = FALSE;
		}
		else if( aff_periph->commande )
            /* si une commande est en cours et qu'il n'y a pas de reponse */
            aff_periph->err_rec ++;
		break;
		
	case SER_TAMPON_EMISSION:
		if( aff_periph->err_emi == AFF_NB_ERREURS_MAX )
		{
			//       AffFichierTrace(ident_aff," ERREUR LIAISON EMISSION");
            aff_periph->err_emi = 0;
            if( aff_periph->b_etat.liaison_hs == FALSE )
            {
				/* si changement d'‚tat, avertir l'application */
				aff_periph->b_etat.liaison_hs = TRUE;
				aff_periph->b_etat.aff_hs = TRUE;
				AFFEnvoiAni(ident_aff, M_CHANGEMENT_ETAT, NULL);
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
		
		case M_DEVICE_INFO_COMPLETE :
			memcpy( &p_ani->u.ios_ani.sDevInfoMsg, parametre, sizeof(_AFF_ANI_DEV_INFO_MSG) );
			break;
		
		default :
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

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFRazAfficheur( short ident_aff )
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
PRIVATE void AFFRazAfficheur( short ident_aff)
{
	struct_serie_peripherique	*aff_periph = &AFF[ident_aff].gestion_peripherique;
	char						cText[11] = "          ";
	
	cText[10] = 0;

	//Send empty text for line 1
	FinRegionLocale();
	DelaiTache(1);
	DebutRegionLocale();
	AFFChoixTexteLigne(ident_aff, aff_periph, cText, MATCO_LIGNE_1);
	
	//Send empty text for line 2
	FinRegionLocale();
	DelaiTache(1);
	DebutRegionLocale();
	AFFChoixTexteLigne(ident_aff, aff_periph, cText, MATCO_LIGNE_2);		
	
	//Send empty text for line 3
	FinRegionLocale();
	DelaiTache(1);
	DebutRegionLocale();
	AFFChoixTexteLigne(ident_aff, aff_periph, cText, MATCO_LIGNE_3);		
	
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFAffichageLignes( short ident_aff, short priorite)
* PARAMETRES: la priorite modifi‚e par la tache ani
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Procedure
* ROLE: Afficher les message avec la plus haute priorit‚
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void AFFAffichageLignes( short ident_aff, short priorite)
{
	short						prio_en_cours1,	prio_en_cours2, prio_en_cours3;
	struct_list_text			*p_texte1, *p_texte2, *p_texte3;
	struct_list_text			texte_vide =
								{
									'A',
									"                        ",
									FALSE,
									NULL
								};
   
	char						police;
	struct_request_screen		*p_ligne1, *p_ligne2, *p_ligne3;
	struct_serie_peripherique	*aff_periph = &AFF[ident_aff].gestion_peripherique;
	static UCHAR				szText1[MAX_PATH] = {0}, szText2[MAX_PATH] = {0}, szText3[MAX_PATH] = {0};

	prio_en_cours1 = AFF_MsgRequestLinePriority(ident_aff, &AFF[ident_aff].sScreen);
	prio_en_cours2 = AFF_MsgRequestLinePriority(ident_aff, &AFF[ident_aff].sScreen);
	prio_en_cours3 = AFF_MsgRequestLinePriority(ident_aff, &AFF[ident_aff].sScreen);

	/* Mettre a jour l'affichage si c'est la priorit‚ … afficher */
	if( prio_en_cours1 <= priorite || prio_en_cours2 <= priorite )
	{
		/* Choix de la ligne 1 */
		if( prio_en_cours1 != -1)
		{
			p_ligne1 = &(AFF[ident_aff].sScreen.tab_request[prio_en_cours1]);
			p_texte1 = AFF_MsgResearchLineNumber( p_ligne1, 1);
		}
		else
			p_texte1 = &texte_vide;

		/* Choix de la ligne 2 */
		if( prio_en_cours2 != -1)
		{
			p_ligne2 = &(AFF[ident_aff].sScreen.tab_request[prio_en_cours2]);
			p_texte2 = AFF_MsgResearchLineNumber( p_ligne2, 2);
		}
		else
			p_texte2 = &texte_vide;

		/* Choix de la ligne 3 */
		if( prio_en_cours3 != -1)
		{
			p_ligne3 = &(AFF[ident_aff].sScreen.tab_request[prio_en_cours3]);
			p_texte3 = AFF_MsgResearchLineNumber( p_ligne3, 3);
		}
		else
			p_texte3 = &texte_vide;

		police = 'A';

		switch( police)
		{
			case 'A':
				if (p_texte1 != 0 && p_texte1->texte[0] != 0)
				{
					FinRegionLocale();
					DelaiTache(1);
					DebutRegionLocale();
					AFFChoixTexteLigne(ident_aff, aff_periph, p_texte1->texte, MATCO_LIGNE_1);		
					strncpy_s(last_texte1, sizeof(last_texte1), p_texte1->texte, AFF_LIGNE_MAX);
				}
				else
				{
					FinRegionLocale();
					DelaiTache(1);
					DebutRegionLocale();
					AFFChoixTexteLigne( ident_aff, aff_periph, last_texte1, MATCO_LIGNE_1);
				}

				if (p_texte2 != 0 && p_texte2->texte[0] != 0)		
				{
					FinRegionLocale();
					DelaiTache(1);
					DebutRegionLocale();
					AFFChoixTexteLigne(ident_aff, aff_periph, p_texte2->texte, MATCO_LIGNE_2);
					strncpy_s(last_texte2, sizeof(last_texte2), p_texte2->texte, AFF_LIGNE_MAX);
					FinRegionLocale();
					DelaiTache(1);
					DebutRegionLocale();
				}
				else
				{
					FinRegionLocale();
					DelaiTache(1);
					DebutRegionLocale();
					AFFChoixTexteLigne( ident_aff, aff_periph, last_texte2, MATCO_LIGNE_2);
				}				

				if (p_texte3 != 0 && p_texte3->texte[0] != 0)		
				{
					FinRegionLocale();
					DelaiTache(1);
					DebutRegionLocale();
					AFFChoixTexteLigne(ident_aff, aff_periph, p_texte3->texte, MATCO_LIGNE_3);
					strncpy_s(last_texte3, sizeof(last_texte3), p_texte3->texte, AFF_LIGNE_MAX);
			//		DelaiTache(1);
				}
				else
				{
					FinRegionLocale();
					DelaiTache(1);
					DebutRegionLocale();
					AFFChoixTexteLigne( ident_aff, aff_periph, last_texte3, MATCO_LIGNE_3);
				}

		// <-- NBL
				if (p_texte1 != 0 && p_texte1->texte[0])
					strcpy_s(szText1, sizeof(szText1), p_texte1->texte);
				if (p_texte2 != 0 && p_texte2->texte[0])
					strcpy_s(szText2, sizeof(szText2), p_texte2->texte);
				if (p_texte3 != 0 && p_texte3->texte[0])
					strcpy_s(szText3, sizeof(szText3), p_texte3->texte);
				
			  AFFNouvelleVisu( ident_aff, szText1, szText2, szText3 );
			  break;

		  case 'B':
			  break;

		  default:
			  AffFichierDebug( ident_aff,
								 "AFF_IOS *** Service AFFICHAGE => police '%d' inconnu ***",
								  police);		
			  break;
		}
	}
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFEndComm( short ident_aff)
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
PRIVATE void AFFEndComm(short ident_aff, short port) 
{
	Ser_enum_cpt_erreur_t result;
    unsigned char   szMessageOut[MAX_PATH] = {0};
	unsigned short	sCheckSum1 = 0;

	DWORD dwDataRead = 0;

	// Sync bytes
	szMessageOut[0] = 0x24;	//	$
	szMessageOut[1] = 0x57;	//	W
	szMessageOut[2] = 0x52;	//	R
	szMessageOut[3] = 0x4c;	//	L
	szMessageOut[4] = 0x3e;	//	>

	// Length [7] - [15]
	szMessageOut[5] = 0x9;	//	Byte Count from byte 7 - to 15
	szMessageOut[6] = 0;	//	byte count high order

	// Serial address for all signs 
	szMessageOut[7] = 0x41;	//	Set serial address
	szMessageOut[8] = 0x00; // all signs are going to receive this message

	// Frame Number 
	szMessageOut[9] = 0x01;

	// Reserved bytes must be zero
	szMessageOut[10] = 0;
	szMessageOut[11] = 0;
	szMessageOut[12] = 0;
	szMessageOut[13] = 0;
	szMessageOut[14] = 0;

	szMessageOut[15] = 0x7A;

	sCheckSum1 = CalcCheckSum( szMessageOut, 7, 15 );	//	7 - start of check sum to calc   15 -  end of check sum to calc
	szMessageOut[16] = (unsigned char)((unsigned char *)&sCheckSum1)[0];
	szMessageOut[17] = (unsigned char)((unsigned char *)&sCheckSum1)[1];

	result = SerEcritMessage(port, szMessageOut, 18 );
	if( result != SER_CPT_OK )
		/* erreur … l'‚mission */
		AFFErreurLiaison(ident_aff,SER_TAMPON_EMISSION);

	/* si dialogue operateur demande */
	//if( AFF[ident_aff].temoin_dop_sortant )
	AFFEnvoiMessageEspion(ident_aff,(UCHAR *)szMessageOut, 18, SRV_ESPION_MESSAGE_DONNEES, SRV_ESPION_SORTANT);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFChoixTexteLigne( short ident_aff, struct_serie_peripherique *aff_periph,
*									 char *texte1, size_t max_texte)
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
								char *texte,
								enum_matco_line eLine)
{
	UCHAR	msg_commande[MAX_PATH],
			*szTemp;
	UCHAR	ucCtr = 0;
	UCHAR	ucLRC = 0;

	unsigned short	sCheckSum1 = 0;
	unsigned short	sCheckSum2 = 0;

	int i = 0;

	DWORD dwDataRead = 0;
	DWORD dwDataWrote = 0;
	
	int iFirstByteCount = 0x28;		//	fream head + frame data (frame verify not included)
	int iSecondByteCount = 0x1E;	// command header + verify, still missing command data (all this is frame data)	
	int iStartOfText	=	44;

	int iStrByteCount=0;
	
	memset( msg_commande, 0, MAX_PATH );
	szTemp = texte;

	//filtering agains non-displaying characters
	while( *szTemp != '\0' )
	{
		if ( ( *szTemp < 32 ) || ( *szTemp > 122 ) || ( *szTemp == 96 ) )
			*szTemp = '?';
		else if( islower( *szTemp ) )
			*szTemp = toupper( *szTemp );
		szTemp++;
	}

	while( texte[iStrByteCount] != 0 )
	{
		msg_commande[ iStartOfText + iStrByteCount ] = texte[iStrByteCount];
		iStrByteCount++;
	}

	// Sync bytes
	msg_commande[0] = 0x24;	//	$
	msg_commande[1] = 0x57;	//	W
	msg_commande[2] = 0x52;	//	R
	msg_commande[3] = 0x4c;	//	L
	msg_commande[4] = 0x3e;	//	>

	// Length
	msg_commande[5] = iFirstByteCount + iStrByteCount;	//	Byte Count from byte 7 - to right before last 2 bytes ( 1st checksum)
	msg_commande[6] = 0;

	// Serial address for sign
	msg_commande[7] = 0x41;		//	Set serial address start of 2nd checksum

	//put appropriate character for the line number 
	switch(eLine)
	{
	case MATCO_LIGNE_1:
		msg_commande[8] = MATCO_LINE_1;
		break;
	case MATCO_LIGNE_2:
		msg_commande[8] = MATCO_LINE_2;
		break;
	case MATCO_LIGNE_3:
		msg_commande[8] = MATCO_LINE_3;
		break;
	}

	// Frame Number 
	msg_commande[9]  = 0;

	// Reserved bytes must be zero
	msg_commande[10] = 0;
	msg_commande[11] = 0;
	msg_commande[12] = 0;
	msg_commande[13] = 0;
	msg_commande[14] = 0;

	msg_commande[15] = 0x65;	//	Clear memory
	msg_commande[16] = 0x61;	//	Transmit text file command
	msg_commande[17] = 0x00;	//	File Type 0 = Text
	msg_commande[18] = 0x4d;	//	"M"		start of 1st checksum
	msg_commande[19] = 0x41;	//	"A"
	msg_commande[20] = 0x49;	//	"I"
	msg_commande[21] = 0x4e;	//	"N"
	msg_commande[22] = 0x00;	//	blank
	msg_commande[23] = 0x00;	//	blank
	msg_commande[24] = 0x00;	//	blank
	msg_commande[25] = 0x00;	//	blank
	msg_commande[26] = iSecondByteCount + iStrByteCount;	//	Byte count from 17 - to right before last 4 bytes ( 2nd checksum)
	msg_commande[27] = 0x00;								//	Byte count
	msg_commande[28] = 0x1c;	//	Second
	msg_commande[29] = 0x04;	//	Minute
	msg_commande[30] = 0x15;	//	Hour
	msg_commande[31] = 0x0f;	//	Day
	msg_commande[32] = 0x06;	//	Month
	msg_commande[33] = 0x0D;	//	Year
	msg_commande[34] = 0xef;	//	Begining of Text
	msg_commande[35] = 0xf1;	//	Method start
	msg_commande[36] = 0x01;	//	Slide to left
	msg_commande[37] = 0xf1;	//	Method end
	msg_commande[38] = 0xf2;	//	Font start
	msg_commande[39] = (UCHAR)AFF[ident_aff].dwTextFormat;		//	Font byte
	msg_commande[40] = 0xf2;	//	Font end
	msg_commande[41] = 0xf3;	//	Color start
	msg_commande[42] = 0x01;	//	Red
	msg_commande[43] = 0xf3;	//	Color end
	msg_commande[iStartOfText + iStrByteCount] = 0xef; //	End of Text

	sCheckSum1 = CalcCheckSum( msg_commande, 18, iStartOfText + iStrByteCount);			//  checksum for send text command
	msg_commande[iStartOfText + iStrByteCount + 1] = (unsigned char)((unsigned char *)&sCheckSum1)[0];
	msg_commande[iStartOfText + iStrByteCount + 2] = (unsigned char)((unsigned char *)&sCheckSum1)[1];

	sCheckSum2 = CalcCheckSum( msg_commande, 7, iStartOfText + iStrByteCount + 2 );		// frame checksum
	msg_commande[iStartOfText + iStrByteCount + 3] = (unsigned char)((unsigned char *)&sCheckSum2)[0];
	msg_commande[iStartOfText + iStrByteCount + 4] = (unsigned char)((unsigned char *)&sCheckSum2)[1];

	//send message 
	AFFEcritMessageSerie( ident_aff, aff_periph->port, msg_commande, (short int)(iStartOfText + iStrByteCount + 5));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFNouvelleVisu( short ident_aff, char * ligne1, char * ligne2, char *ligne3)
* PARAMETRES: ident de l'imprimante, 1st line pointer, 2nd line pointer, 3rd line pointer
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Procedure
* ROLE: Sends the state to VISU service
* --------------------------------------------------------------------
* $F_FCTN
*/

// <-- NBL

PRIVATE void AFFNouvelleVisu( short ident_aff, char * ligne1, char * ligne2, char *ligne3)
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
	strncpy_s(p_ios->u.ios_ani.visu_en_cours[0], dwSize, ligne1, dwSize);
	p_ios->u.ios_ani.visu_en_cours[0][dwSize-1] = '\0';

	dwSize = sizeof(p_ios->u.ios_ani.visu_en_cours[1]);
	strncpy_s(p_ios->u.ios_ani.visu_en_cours[1], dwSize, ligne2, dwSize);
	p_ios->u.ios_ani.visu_en_cours[1][dwSize-1] = '\0';

	dwSize = sizeof(p_ios->u.ios_ani.visu_en_cours[2]);
	strncpy_s(p_ios->u.ios_ani.visu_en_cours[2], dwSize, ligne3, dwSize);
	p_ios->u.ios_ani.visu_en_cours[2][dwSize-1] = '\0';
   
   /* postage dans la boite aux lettres */
	if(Envoie( AFF[ident_aff].ani_bal,AFF[ident_aff].ios_bal,(struct_neutre *)p_ios) != NOYAU_OK)
	{
		ExitLibere((struct_neutre **)&p_ios);
	}

}

/**/
/*****************************************************************************/
/*SYNTAX:	PROTECTED boolean AFFDeviceInfoCmd(short int ident_aff,			 */
/*											   MATCO_command command);		 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*				Executes requested command for device info.					 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*							short int		ident_aff	- Instance ID.		 */
/*							MATCO_command		command	- type of command	 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  The function is successful.						 */
/*  FALSE                 The function is not successful.					 */
/*****************************************************************************/
PROTECTED boolean AFFDeviceInfoCmd(short int ident_aff, MATCO_command command )
{
	char	szCmd[5]	= { 0 };
	int		iCmdCount	= 0;

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
	
	// Clean read buffer
	SerRazTampon( AFF[ident_aff].gestion_peripherique.port, SER_TAMPON_RECEPTION );
	
	// Send command through serial port - this can be separate function if needed, ex:
		//PRIVATE boolean AFFExecCmd(short int ident_aff,
		//							 char *szCmd,
		//							 int iLen )

	if (szCmd[0] != '\0')
	{
		Ser_enum_cpt_erreur_t ser_erreur = SER_CPT_OK;

		// Send the message to the ESPION service subscribers
		AFFEnvoiMessageEspion(ident_aff,
							(unsigned char *)szCmd,
							(short int)iCmdCount,
							SRV_ESPION_MESSAGE_DONNEES,
							SRV_ESPION_SORTANT);

		// Send the command to the communication port
		FinRegionLocale();
		ser_erreur = SerEcritMessage(AFF[ident_aff].gestion_peripherique.port,
									szCmd,
									iCmdCount);
		DebutRegionLocale();

		if (ser_erreur != SER_CPT_OK)
		{
			AffFichierDebug(ident_aff, "Error writing command to the port %d",
							AFF[ident_aff].gestion_peripherique.port);

			return FALSE;
		}
	}
	else
		return FALSE;
	
	return TRUE;
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
									MATCO_command command,
									_AFF_ANI_DEV_INFO_MSG *pDevInfoMsg)
{
	BYTE					byDataBuffer[MAX_PATH] = {0};
	INT						iLen;
	INT						iResponseLength	= 1;
	Ser_enum_cpt_erreur_t	port_err;

	// Depending of command, length to receive from serial port is known
	switch (command)
	{
		case COMMAND_GET_FIRMWARE_VERSION:
			iResponseLength = AFF_FIRMWARE_VER_RESPONSE_LEN;
			break;
	}
	
	FinRegionLocale ();
	iLen = MAX_PATH;
	port_err = SerLitMessage(AFF[ident_aff].gestion_peripherique.port,
							byDataBuffer,
							iResponseLength,
							(unsigned long *)&iLen,
							AFF[ident_aff].delay_command );
	DebutRegionLocale ();
	
	if(iLen)
	{
		// Send the message to the ESPION message subscribers
		AFFEnvoiMessageEspion(ident_aff,
							byDataBuffer,
							(short int)iLen,
							SRV_ESPION_MESSAGE_DONNEES,
							SRV_ESPION_ENTRANT);
	}
	
	if( port_err != SER_CPT_OK )
	{
		switch( port_err )
		{
		case SER_CPT_DONNEES_ABSENTES:
		case SER_CPT_DONNEES_INCOMPLETES:				
		default:
			AffFichierDebug(ident_aff, "AFF_IOS : Error reading port, err id = %d", port_err);
			
			return FALSE; // Link error
		}
	}

	// Depending of command, it is known which data to store
	switch (command)
	{
		case COMMAND_GET_FIRMWARE_VERSION:
			strcpy_s(pDevInfoMsg->sDeviceInfoList.items[pDevInfoMsg->sDeviceInfoList.nb_item].name,
					sizeof(pDevInfoMsg->sDeviceInfoList.items[pDevInfoMsg->sDeviceInfoList.nb_item].name), "Firmware version");
			sprintf_s(pDevInfoMsg->sDeviceInfoList.items[pDevInfoMsg->sDeviceInfoList.nb_item].data,
					sizeof(pDevInfoMsg->sDeviceInfoList.items[pDevInfoMsg->sDeviceInfoList.nb_item].data),
					"%02d.%02d", byDataBuffer[0], byDataBuffer[1]);
			pDevInfoMsg->sDeviceInfoList.nb_item++;
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
