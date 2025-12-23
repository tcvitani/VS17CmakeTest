/************** (v) 2017 EMOVIS - All rights reserved ***************/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Afficheur ext‚rieur
* FICHIER: AFF_ANI.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Code de la tache AFF_ANI
* --------------------------------------------------------------------
* DESCRIPTION: La tache AFF_ANI est la tache d'animation de l'afficheur
*              La tache communique d'une part avec l'application qui
*              souhaite utiliser une telle imprimante, et d'autre part avec
*              la tache AFF_IOS qui pilote directement l'afficheur.
* --------------------------------------------------------------------
* HISTORIQUE:
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <noyau.h>
#include <debug.h>
#include <fic_gere.h>

#include <aff_ext.h>
#include <aff_util.h>
#include <aff_glob.h>
#include <aff_ani.h>
#include <aff_serv.h>
#include <aff_mess.h>

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

PRIVATE void CleanUpBeforeExit(short int ident_aff);

/*--------------- VARIABLES: ---------------*/

/*--------------- CODE: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: DWORD AffAni(void * pvFoo)
* PARAMETRES:
*     entree: pvFoo : Pour conformité Win32
*     retour:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme principal de la tache temps-reel Aff_ani
* ROLE: Receptionner les messages qui arrivent de l'application ou
*       de la tache AffIos et lancer les routines de traitement
*       adaptees a chaque type de message.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED DWORD WINAPI AffAni(void * pvFoo)
{
   struct_neutre       *p_neutre = (struct_neutre *)(0);
   noyau_enum_retour   code_rtc;
   noyau_bal_id        aff_ani_bal;
   noyau_bal_id        aff_ios_bal;
   short int           ident_aff=(short int)(pvFoo);
   noyau_delai         timeout_bal;

    AffFichierTrace( ident_aff, "DEN - Instance(%s) - ANI tid=%u/0x%08X",
            AFF[ident_aff].nom_bal_ani,
            GetCurrentThreadId(),
            GetCurrentThreadId() );

    /* init de la tache et recherche de son identificateur */
    InitAffAni( ident_aff );

    /* recuperation du num‚ro de boite aux lettres */
    aff_ani_bal = AFF[ident_aff].ani_bal;

    /* recuperation du num‚ro de la boite aux lettres IOS */
    aff_ios_bal = AFF[ident_aff].ios_bal;

    /* initialisation du timeout d'attente sur la BAL */
    timeout_bal = AFF[ident_aff].timeout_bal = AFF_TIMEOUT_LENT;

    ChangePriorite( TacheCourante(), AFF[ident_aff].priorite_max );

    /* attente et traitement des messages */
    while ( TRUE )
    {
       code_rtc = Recoit( aff_ani_bal, &p_neutre, timeout_bal);

		DebutRegionLocale();

       /* Messages issus de l'application */
       if( code_rtc == NOYAU_BAL_MESS )
       {
          /* Reception d'un message de la tache IOS */
          if (p_neutre->bl_retour == aff_ios_bal)
          {
             AFFReceptionIos (ident_aff, p_neutre);
          }
          /* Reception d'une demande de service */
          else
          {
             AFFReceptionService (ident_aff, p_neutre);
          }

          /* lib‚ration du message recu */
          #ifdef DEBUG
          AffFichierTrace(ident_aff,"AFF_ANI => Libere() : %p ", p_neutre);
          #endif
          ExitLibere( &p_neutre );

       }
       else if (code_rtc == NOYAU_BAL_TIME )
          /* demande d'etat periodique si aucunes autres commandes */
          /* et liaison OK */
          AFFEnvoiIos(ident_aff, M_ETAT, NULL);

       /* test des fichiers de debug */
       DBG_TesteTailleFichiersTraces ( &AFF[ident_aff].dbg);

       /* memoriser le nouveau timeout */
       timeout_bal = AFF[ident_aff].timeout_bal;

		FinRegionLocale();

		// Condition for thread exit
		if (AFF[ident_aff].bAniThreadStopRequest)
			break;
    }

	CleanUpBeforeExit(ident_aff);
	return 0;
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void InitAffAni(void)
* PARAMETRES:
*     entree: rien
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Initialisation des variables utiles a la tache AFF_ANI
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void InitAffAni(short int ident_aff)
{
	/**************************************************************/
	/* Reset flag which tells if ANI thread has to be stopped	  */
	/**************************************************************/	
	AFF[ident_aff].bAniThreadStopRequest = FALSE;

   // publication de la BAL ANI
   AFF[ident_aff].ani_bal = PublieBAL( AFF[ident_aff].nom_bal_ani, NOYAU_BAL_ILLIMITEE );
   if (AFF[ident_aff].ani_bal <= 0)
      ExitBad();

   // Attente publication de la BAL IOS
   AFF[ident_aff].ios_bal = AttendBAL (AFF[ident_aff].nom_bal_ios);
   if (AFF[ident_aff].ios_bal <= 0)
      ExitBad();
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFReceptionIos(short int ident_aff,struct_neutre *p_neutre)
* PARAMETRES:
*     entree:
*     retour:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFReceptionIos(short int ident_aff,struct_neutre *p_neutre)
{
	struct_ani__ios		*p_ios = (struct_ani__ios *)(0);
	struct_b_etat_aff	*p_etat;
	int					i;
	DWORD				dwSize;

	p_etat = (struct_b_etat_aff*)&AFF[ident_aff].gestion_peripherique.b_etat;

	/* initialisation du pointeur */
	p_ios = (struct_ani__ios *)(p_neutre);

	/* analyse du message recu */
	switch (p_ios->message_id)
	{
		case M_ACK_AFFICHAGE :
			AffFichierTrace(ident_aff, "AFF_ANI: AFFReceptionIos() => Displaying finished");
			break;

		case M_ERR_AFFICHAGE :
			AffFichierTrace(ident_aff, "AFF_ANI: AFFReceptionIos() => Displaying error");
			break;

		case M_CHANGEMENT_ETAT :
			AffFichierTrace(ident_aff, "AFF_ANI: AFFReceptionIos() => New AFF state: link = %d, aff_hs = %d", p_etat->liaison_hs, p_etat->aff_hs);
			
			/* apparition d'un changement d'etat => transmission vers l'appli */			
			AFFEnvoiEtat (ident_aff, 0, SRV_TYP_NOUVEL_ETAT, p_ios->u.ios_ani.u.uiEtat);
			
			/* Traitement si liaison HS */
			if( p_etat->liaison_hs )
			{
	            // Ne plus faire de demande d'etat ou d'impression
				AFF[ident_aff].timeout_bal = AFF_TIMEOUT_INFINI;
			}
			else
			{
	            // Reprendre l'envoi periodique du message d'etat
				// (To take again the periodic sending of the message of state)
	    		if (SrvEstVide(AFF[ident_aff].service[M_AFF_AFFICHAGE]) )
	    			AFF[ident_aff].timeout_bal = AFF_TIMEOUT_LENT;
				else
				    AFF[ident_aff].timeout_bal = AFF_TIMEOUT_RAPIDE;
			}
			break;

		case M_CHANGEMENT_VISU :
            AffFichierTrace(ident_aff, "AFF_ANI: ReceptionIOS() => VISU Changed" );
            dwSize = sizeof( AFF[ident_aff].tab_lignes_courantes[0] );
			for (i=0; i<AFF_MAX_LIGNES; i++)
			{
				if (&p_ios->u.ios_ani.visu_en_cours[i] != NULL)
				{
					strncpy_s(AFF[ident_aff].tab_lignes_courantes[i], 
							sizeof(AFF[ident_aff].tab_lignes_courantes[i]),
							p_ios->u.ios_ani.visu_en_cours[i], dwSize);
					
					AFF[ident_aff].tab_lignes_courantes[i][dwSize-1] = '\0';
				}
			}

            AFFEnvoiVisu( ident_aff, 0, AFF_TYP_AFFICHAGE_COURANT );
            break;

		case M_ARRET_EFFECTUE :
			AffFichierTrace( ident_aff, "AFF_ANI: AFFReceptionIos() => IOS Stopped succsessfully");

			// Tout est bien arrété, on peut arrêter le thread
			AFF[ident_aff].bAniThreadStopRequest = TRUE;
			break;

		// DEVICE INFO
		case M_DEVICE_INFO_COMPLETE :
			AffFichierTrace( ident_aff, "AFF_ANI: AFFReceptionIos() => Device info");

 			AFFEnvoiDeviceInfo( ident_aff, SRV_TYP_EFFECTUE, &p_ios->u.ios_ani.u.sDevInfoMsg);
			break;

		default :
			/* reception d un message errone : message_id inconnu ... */
			AffFichierDebug(ident_aff, "AFF_ANI  ***** AFFReceptionIos() => msg_id '%d' unknown *****",
							p_ios->message_id);
			break;
	}
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFReceptionService( short int ident_aff,
*                                    struct_neutre *p_neutre)
* PARAMETRES:
*     entree:
*     retour:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFReceptionService(short int ident_aff,struct_neutre *p_neutre)
{
   struct_aff_message *p_message = (struct_aff_message *)p_neutre;

   switch(p_message->entete.service)
   {
      /* service affichage */
      case M_AFF_AFFICHAGE:
         AFFReceptionAffichage (ident_aff,p_message);
      break;

      /* service visu */
      case M_AFF_VISU:
         AFFReceptionVisu(ident_aff,p_message);
      break;

      /* service etat */
      case M_SRV_ETAT:
         AFFReceptionEtat(ident_aff,p_message);
      break;

      /* service dialogue operateur */
      case M_SRV_ESPION:
         AFFReceptionEspion(ident_aff,p_message);
      break;

      /* service arret */
      case M_SRV_ARRET:
         AFFReceptionArret(ident_aff,p_message);
      break;

	  /* service for device info */
	  case M_SRV_DEVICE_INFO:
         AFFReceptionDeviceInfo(ident_aff,p_message);
      break;

      /* service non valid */
      default:
		/* Sending an acknowledgment to the application to indicate	*/
		/* that this service is not managed by this module or is unknown */
         if( p_message->entete.service >= BUTEE_AFF_SERVICE)
         {
            AFFEnvoiAcquittement(ident_aff, p_message->entete.neutre.bl_retour,
                           p_message->entete.service, SRV_TYP_SERVICE_INCONNU);

            AffFichierDebug( ident_aff, "AFF_ANI ***** AFFReceptionService() => service_id '%d' unknown *****",
                             p_message->entete.service);
         }
         else
         {
            AFFEnvoiAcquittement(ident_aff, p_message->entete.neutre.bl_retour,
                           p_message->entete.service, SRV_TYP_SERVICE_NON_GERE);

            AffFichierDebug( ident_aff, "AFF_ANI ***** AFFReceptionService() => service_id '%d' non managed *****",
                             p_message->entete.service);
         }
      break;
   }
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
	// Sending 'effective stop' acknowledgment message to the application
	AFFEnvoiAcquittement(ident_aff,
						AFF[ident_aff].arret_bal,
						M_SRV_ARRET,
						SRV_TYP_ARRET_EFFECTUE );
	
	// Cleaning mailbox
	AFFLibereBAL(AFF[ident_aff].ani_bal);
	// Closing mailbox
	SupprimeBAL(AFF[ident_aff].nom_bal_ani);

	AffFichierTrace(ident_aff, "AFF_ANI: CleanUpBeforeExit() => Ani thread cleaning up complete!");
}
