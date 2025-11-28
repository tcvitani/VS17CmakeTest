/*------   (v) 1997 CS-Route   -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Tache REFERENCE
* FICHIER: referenc.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Tache de Reception du fichier de reference
*         Code des fonctions utiles pour l'application
*
* --------------------------------------------------------------------
* DESCRIPTION:
*             1) recoit un message de reference
*             2) telecharge le fichier
*             3) le traite pour mettre en oeuvre la nouvelle configuration
*
* --------------------------------------------------------------------
* NOTA:
*      Cette tache utilise en permanence 6 ou 7 fichiers, et peut monter
*      temporairement … 10 ou 12 fichiers ( avec les fichiers du LS)
*
*      Sachant que BORLAND limite … 14 le nombre de fichiers ouvrables
*      ( j'ai fait le test), on d‚cide que les fichiers les plus simples
*      ( qui ne change pas de nom) sont ouverts au debut de la boucle
*      a l'entree de la region, et referm‚s avant la sortie de la region
*
*      En effet, la VM est TRES fragile sur les accŠs disques, on a
*      constat‚ des plantes inexplicables sur:
*      - fopen
*      - fread, ...
*
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log : $
 *
 *    Rev 1.0   Apr 24 1997 15:02:26   DPI
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

/* outils_C */
#include "err.h"
#include "fic.h"
#include "str.h"
#include "fic_conf.h"

/* module NOYAU */
#include "noyau.h"
//#include "tcp_ip.h"
#include "csr_lan.h"

#include "rc_def.h"
#include "fic_gere.h"

#include "referenc.h"
#include "pre_main.h"

#include "rc_glob.h"

#define LOC_DEF
#include "rc_loc.h"
#undef LOC_DEF

#include "rc_recpt.h"
#include "rc_trait.h"
#include "rc_emis.h"


#include "fic_ref.h"
#include "fic_act.h"
#include "fic_alar.h"
#include "fic_msg.h"
#include "fic_id.h"
#include "fic_idcn.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- VERSION: ---------------*/

/*--------------- DEFINES: ---------------*/


/*--------------- TYPEDEFS: ---------------*/



/*--------------- FUNCTIONS: ---------------*/
PRIVATE void        InitReference ( TpTTacheContext Context);
PRIVATE void        TerminateReference ( TpTTacheContext Context);
PRIVATE void        ResetContext( TpTTacheContext Context);
PRIVATE void        InitAction( TpTTacheContext Context);

PRIVATE boolean     SontMessagesAcceptes( TpTTacheContext Context);
PRIVATE void        InitFichiersBoucle ( TpTTacheContext Context);
PRIVATE void        FiniFichiersBoucle ( TpTTacheContext Context);
PRIVATE void        ReferenceColdStart( TpTTacheContext Context);


/*--------------- VARIABLES: ---------------*/

PRIVATE   TGereFic         GereConfig;
PRIVATE   TValeur          IdConfig;
PRIVATE   TValeur          ValConfig;
PRIVATE   char             ItemConfig[ CONF_MAX_ITEM];

PRIVATE   TGereFic         GereAncien;
PRIVATE   TFichier         IdAncien;
PRIVATE   TFichier         ValAncien;

PRIVATE   TGereFic         GereNouveau;
PRIVATE   TFichier         IdNouveau;
PRIVATE   TFichier         ValNouveau;

PRIVATE   TGereFic         GereCreer;
PRIVATE   TFichier         IdCreer;
PRIVATE   TFichier         ValCreer;

PRIVATE   TGereFic         GereId;
PRIVATE   TGereFic         GereActions;

PRIVATE   TGereFic         GereIdContext;
PRIVATE   TGereFic         GereAlarmes;

PRIVATE   TGereFic         GereMessages;
//PRIVATE struct_rfr_gestion_etat ancien_etat;

/*--------------- CODE: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned int Emission(void)
* PARAMETRES: aucun
* RETOUR:     code
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme principal de la tache temps reel
* ROLE: Recuperer les messages qui arrivent dans la boite aux lettres
*       de la tache et les traiter
*       Envoi automatique des fichiers cr‚es
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED DWORD WINAPI Reference (PVOID param)
{
   TpTTacheContext    Context;
   struct_neutre      *p_message = NULL;
   noyau_enum_retour  code_rtc;
   noyau_bal_id       bal;
   boolean            accepte;
   boolean            dejaArrete = FALSE;

   /********************************/
   /* indirection de la variable globale g‚rant la tache */

   Context = &RFR;

   /********************************/
   /* Initialisation de la tache   */
   /* et changement de sa priorite */
   /********************************/

   DebutRegion();

   bal = Context->util.BalLocale;

   InitReference( Context);

   InitFichiersBoucle( Context);

   ReferenceTraitement( Context);

   ChangePriorite (TacheCourante(), Context->util.PrioriteMax);

   FiniFichiersBoucle( Context);

   /* Signaler initialisation OK */
   /* (utilise pour enregistrement des modules applicatifs en attente) */
   if (SignalEvt (Context->wait_init_evt) != NOYAU_OK)
         ERR_ErreurFatale();

   FinRegion();

	// Laisser un peu de temps pour l'enregistrement des modules applicatifs (eventuellement en 
	// attente) avant de lire la BAL
	Sleep(1000 * RFR.sleep_duration_after_init_sec);

   /*************************/
   /* Traitement periodique */
   /*************************/
   while (TRUE)
   {
      /*****************************************************/
      /* Attente de message dans la BAL de la tache */
      /*****************************************************/
//      DebutRegion();
      accepte = SontMessagesAcceptes( Context);
//      FinRegion();

      if( accepte)
         code_rtc = Recoit(bal,&p_message,RFR_TIMEOUT_LENT);
      else
      {
         DelaiTache( RFR_TIMEOUT_RAPIDE);
         code_rtc = NOYAU_BAL_TIME;
      }

//      DebutRegion();

      Context->action.ActionId = NO_ACTION_TO_DO;
      if (code_rtc == NOYAU_BAL_MESS)
      {
         /***********************************************************/
         /* Traitement des messages recus dans la boite aux lettres */
         /***********************************************************/
         Context->util.BalRetour  = p_message->bl_retour;

         if(p_message->bl_retour == RFR.util.BalHorodate )
         {
	         /* messages recu du module de reception des messages */
	         ReceptionRfrAlarme( Context, (struct_hrd_message *)p_message);
	      }
	      else if(p_message->bl_retour == RFR.util.BalFichier )
         {
	         /* messages recu du module de telechargement des fichiers */
	         ReceptionRfrFichier( Context, (struct_lan_message *)p_message);
	      }
         else
         {
	         /* messages recu du module de reception des messages
             * ou de l'applicatif : c'est un service
             */
	         ReceptionRfrService( Context, (struct_rfr_message *)p_message);
	      }

	      ExitLibere (&p_message);
      }
      else
      {
         ReceptionRfrTime( Context);

      }

      if( ! dejaArrete)
      {
         /*----------------------------------------------*/
         /* il faut ouvrir tous les fichiers n‚cessaires */
         InitFichiersBoucle( Context);

         /* on passe ici si on n'est pas d‚j… arret‚ */
         ReferenceTraitement( Context);

         /*----------------------------------------------*/
         /* il faut fermer tous les fichiers possibles */
         FiniFichiersBoucle( Context);

         if( Context->util.TemoinArret)
         {
            /* on doit s'arreter */
             RC_AcquitterArret( Context->util.BalRetour,
                                Context->util.BalLocale,
                                Context->util.NumeroPool);
             TerminateReference( Context);
             dejaArrete = TRUE;
         }
      }

//      FinRegion();
   }
}





/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void InitReference(void)
* PARAMETRES: - le context
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: initialisation des variables utilisees par la tache.
*       Ouverture et fermeture des fichiers
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE void InitReference ( TpTTacheContext Context)
{
   char             fichier_dest[ RFR_MAX_PATH];
   char				attrib[3];
   short int        id_serv;


   /**************************************************************/
   /* On positionne le temoin qui permet de savoir si l'arret de */
   /* la tache a ete demande                                     */
   /**************************************************************/
   Context->util.TemoinArret = FALSE;

   /**********************************************************/
   /* le chargement de fichier n'est possible que si le service */
   /* FICHIER du module LAN est active                    */
   /**********************************************************/

   /******************************************/
   /* initialiser tous les services inactifs */
   /******************************************/
   for( id_serv=PREMIER_RFR_SERVICE; id_serv<BUTEE_RFR_SERVICE; id_serv ++)
   {
      Context->service[id_serv].etat = RFR_LIBRE;
      Context->service[id_serv].demandeur = RFR_AUCUN;
   }

   /* initialiser le Fonctionnel */
   Context->GereConfig                  = &GereConfig;
   Context->GereAncien                  = &GereAncien;
   Context->GereNouveau                 = &GereNouveau;
   Context->GereCreer                   = &GereCreer;
   Context->GereId                      = &GereId;
   Context->GereActions                 = &GereActions;
   Context->GereIdContext               = &GereIdContext;
   Context->GereAlarmes                 = &GereAlarmes;
   Context->GereMessages                = &GereMessages;

   Context->util.Demarrage              = TRUE;

   ResetContext( Context);
   InitAction( Context);

   /*---------------------------------------------------*/
   /* initialiser le fichier de configuration */

   if( Context->util.ColdStart)
   {
      ReferenceColdStart( Context);
   }

   if( ! InitFichierConfig( Context->GereConfig, FICHIER_CONFIG_COURANT, "r+t",
                               ItemConfig, &IdConfig, &ValConfig))
   {
      /* cela signifie que le repertoire n'existe pas ou que le fichier
       * n'existe pas
       */
      FIC_makepath( fichier_dest, RFR.disque, RFR.path_reference, NULL, NULL);

      /* on essaie de le creer sans verifier qu'on y arrive */
      FIC_CreatePath( fichier_dest);

      /* mais si on n'arrive pas … cr‚er le fichier, on ‚choue */
      if( ! InitFichierConfig( Context->GereConfig, FICHIER_CONFIG_COURANT, "a+t",
                               ItemConfig, &IdConfig, &ValConfig))
         ERR_ErreurFatale();

      /* il faut memoriser qu'on demarre sans fichier de configuration */
      Context->util.ColdStart = TRUE;
   }
   if( ! FermerFichierConfig( Context->GereConfig))
       ERR_ErreurFatale();

   /* initialiser le fichier des ID de r‚f‚rence */
   if( ! InitFichierIdReception( Context->GereId, "rb"))
   {
      /* le fichier n'existe pas, on doit ˆtre en test
       * Il faut creer le repertoire
       */
      CreerRepertoires();
      if( ! InitFichierIdReception( Context->GereId, "wb"))
      {
         ERR_ErreurFatale();
      }
   }
   if( ! FermerFichierIdReception( Context->GereId))
      ERR_ErreurFatale();

   /* initialiser le fichier des ID Context */
   if( ! InitFichierIdContext( Context->GereIdContext, "wb"))
      ERR_ErreurFatale();
   if( ! FermerFichierIdContext( Context->GereIdContext))
      ERR_ErreurFatale();

   /* initialiser le fichier des actions */
   if( ! InitFichierAction( Context->GereActions, "wb"))
      ERR_ErreurFatale();
   if( ! FermerFichierAction( Context->GereActions))
      ERR_ErreurFatale();

   /* initialiser le fichier des alarmes */
   if( ! InitFichierAlarme( Context->GereAlarmes, "wb"))
      ERR_ErreurFatale();
   if( ! FermerFichierAlarme( Context->GereAlarmes))
      ERR_ErreurFatale();

   /* initialiser le fichier des messages */
   if( ! InitFichierMessage( Context->GereMessages, "wb"))
      ERR_ErreurFatale();
   if( ! FermerFichierMessage( Context->GereMessages))
      ERR_ErreurFatale();


   /* Initialiser les gestionnaires de fichiers de references
    * pour n'affecter les buffers qu'une seule fois
    */
   FIC_makepath( fichier_dest, RFR.disque_virtuel, RFR.path_reference,
                               FICHIER_REFERENCE_COURANT, NULL);

   if (RFR.util.ColdStart)
	   STR_strcpy(3,attrib,"wt");
   else
		STR_strcpy(3,attrib,"rt");  

   if( ! InitFichierReference( Context->GereAncien, fichier_dest, attrib,
                               &IdAncien, &ValAncien))
      ERR_ErreurFatale();
   if( ! FermerFichierReference( Context->GereAncien))
      ERR_ErreurFatale();
   if( ! InitFichierReference( Context->GereNouveau, fichier_dest, attrib,
                               &IdNouveau, &ValNouveau))
      ERR_ErreurFatale();
   if( ! FermerFichierReference( Context->GereNouveau))
      ERR_ErreurFatale();
   if( ! InitFichierReference( Context->GereCreer, fichier_dest, attrib,
                               &IdCreer, &ValCreer))
      ERR_ErreurFatale();
   if( ! FermerFichierReference( Context->GereCreer))
      ERR_ErreurFatale();
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void InitFichiersBoucle(void)
* PARAMETRES: - le context
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: initialisation des variables utilisees par la tache.
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE void InitFichiersBoucle ( TpTTacheContext Context)
{
   /* Ce n'est possible pour config que parce que c'est toujours
    * le mˆme nom qui est employ‚ avec Context->GereConfig
    */
   if( ! EchangerFichierConfig( Context->GereConfig, FICHIER_CONFIG_COURANT, "r+t"))
      ERR_ErreurFatale();

   if( ! InitFichierIdReception( Context->GereId, "r+b"))
      ERR_ErreurFatale();

   /* initialiser le fichier des ID Context */
   if( ! InitFichierIdContext( Context->GereIdContext, "r+b"))
      ERR_ErreurFatale();

   /* initialiser le fichier des actions */
   if( ! InitFichierAction( Context->GereActions, "r+b"))
      ERR_ErreurFatale();

   /* initialiser le fichier des alarmes */
   if( ! InitFichierAlarme( Context->GereAlarmes, "r+b"))
      ERR_ErreurFatale();

   /* initialiser le fichier des messages */
   if( ! InitFichierMessage( Context->GereMessages, "r+b"))
      ERR_ErreurFatale();
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void FiniFichiersBoucle(void)
* PARAMETRES: - le context
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: initialisation des variables utilisees par la tache.
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE void FiniFichiersBoucle ( TpTTacheContext Context)
{
   if( ! FermerFichierConfig( Context->GereConfig))
      ERR_ErreurFatale();
   if( ! FermerFichierIdReception( Context->GereId))
      ERR_ErreurFatale();
   if( ! FermerFichierIdContext( Context->GereIdContext))
      ERR_ErreurFatale();
   if( ! FermerFichierAction( Context->GereActions))
      ERR_ErreurFatale();
   if( ! FermerFichierAlarme( Context->GereAlarmes))
      ERR_ErreurFatale();
   if( ! FermerFichierMessage( Context->GereMessages))
      ERR_ErreurFatale();
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void TerminateReference
* PARAMETRES: - le context
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: initialisation des variables utilisees par la tache.
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE void TerminateReference ( TpTTacheContext Context)
{
   ResetContext( Context);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   SontMessagesAcceptes
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Determine si la tache accepte les messages en fonction de la
*       phase dans lequel se trouve la tache
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE boolean SontMessagesAcceptes( TpTTacheContext Context)
{
  boolean      accepte = TRUE;

  accepte = ( Context->PhaseEnCours != NO_STEP) &&
            ( Context->PhaseEnCours != COMPARING_REFERENCE_FILES) &&
            ( Context->PhaseEnCours != DOING_RECEPTION_ACTIONS) &&
            ( Context->PhaseEnCours != CLOSING_RECEPTION_ACTIONS) &&
            ( Context->PhaseEnCours != DOING_ALARM_ACTIONS) &&
            ( Context->PhaseEnCours != CLOSING_ALARM_ACTIONS)&&
            ( Context->PhaseEnCours != DOING_MESSAGE_ACTIONS) &&
            ( Context->PhaseEnCours != CLOSING_MESSAGE_ACTIONS);

  return( accepte);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ResetContext
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Reset les donn‚es qui ne sont pas r‚manentes entre 2 d‚marrages
*       de tache.
*       Pr‚cisement, tous les gestionnaires sont r‚manents.
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void ResetContext( TpTTacheContext Context)
{
   Context->NbFichiersReclames          = 0;
   Context->ServiceFichierOuvert        = FALSE;
   Context->TousFichiersValides         = TRUE;
   Context->PhaseEnCours                = NO_STEP;
   Context->PremiereActionReception     = NO_ACTION;
   Context->DerniereActionReception     = NO_ACTION;
   STR_strcpy( RFR_MAX_ID, Context->IdFichierReference, ID_FICHIER_REFERENCE);
   Context->NomFichierCourant[ 0]       = '\0';
   Context->ReferenceFichierCourant[ 0] = '\0';
   Context->NomFichierNouveau[ 0]       = '\0';
   Context->ReferenceFichierNouveau[ 0] = '\0';
   Context->NomFichierFutur[ 0]         = '\0';
   Context->ReferenceFichierFutur[ 0]   = '\0';
   Context->AlarmeEnCours               = NO_ALARME;
   Context->MessageEnCours              = NO_MESSAGE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   InitAction
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Initialise l'action a faire au d‚marrage
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void InitAction( TpTTacheContext Context)
{
   Context->action.ActionId = ACTION_START;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void InitReference(void)
* PARAMETRES: - le context
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: initialisation des variables utilisees par la tache.
*       Ouverture et fermeture des fichiers
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE void ReferenceColdStart ( TpTTacheContext Context)
{
   char             fichier_dest[ RFR_MAX_PATH];
   int              ok_dos;
   FIC_enum_retour  fic_retour;
   TReception       *idReception;
   long             position;

   /* il faut detruire le repertoire de reference */
   FIC_makepath( fichier_dest, RFR.disque, RFR.path_reference, NULL, NULL);
   if( _access( fichier_dest, 0) == 0)
   {
      ok_dos = FIC_deltree( fichier_dest);
      ERR_EstVrai( ok_dos == 0);
   }

   /* effacer les fichier de configuration */
   if( ! InitFichierIdReception( Context->GereId, "rb"))
   {
      /* le fichier n'existe pas, on doit ˆtre en test
       * aucun fichier n'est attendu, il n'y a rien a faire
       */
      return;
   }

   position = 1;
   fic_retour = LirePosIdReception( Context->GereId, position, &idReception);
   while( fic_retour == FIC_OK)
   {
      /* traiter l'item en cours */
      STR_strcpy( RFR_MAX_PATH, fichier_dest, idReception->Path);
      if (fichier_dest[ strlen( fichier_dest) - 1] == '\\') /* If '\\' exists */
	      fichier_dest[ strlen( fichier_dest) - 1] ='\0';  /* ...remove it */
      if( _access( fichier_dest, 0) == 0)
      {
         ok_dos = FIC_deltree( fichier_dest);
         ERR_EstVrai( ok_dos == 0);
      }

      FIC_CreatePath( fichier_dest);

      /* obtenir l'item suivant */
      position ++;
      fic_retour = LirePosIdReception( Context->GereId, position, &idReception);
   }

   if( ! FermerFichierIdReception( Context->GereId))
      ERR_ErreurFatale();
}

