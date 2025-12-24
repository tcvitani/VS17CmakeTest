/*------   (v) 1997 CS-Route   -----------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER MESSAGES
* FICHIER: emi_back.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Tache pour effectuer les demandes de restitution de fichiers
*         et la purge periodique des fichiers
* --------------------------------------------------------------------
* DESCRIPTION:
*      1 - GŠre les demandes de restitution de fichier
*             - par date  (jour)
*             - par numero de fichier
*      2- effectue la purge des fichiers de plus de X jours
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/Sources/emi_back.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:04   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.22   06 Nov 1997 15:18:26   DPI
 * Integration DUT 9.00
 * 
 *    Rev 1.21   30 Oct 1997 15:53:50   DPI
 * Suppression du chrono, remplacer par HRD
 * Formule de calcul de l'heure
 * 
 *    Rev 1.20   29 Oct 1997 19:42:10   DPI
 * Suppression des fonctions de debug
 * Modifs des fonctions Lance et Arret
 * 
 * 
 *    Rev 1.19   29 Oct 1997 18:42:40   DPI
 * Evolution ExitAlloue... du noyau 6.00
 * 
 *    Rev 1.18   10 Oct 1997 17:04:46   HMO
 * Correction
 * 
 *    Rev 1.17   25 Sep 1997 16:01:58   HMO
 * Corection erreurs
 *
 *    Rev 1.16   23 Sep 1997 14:39:50   HMO
 * Modifications effectuées sur site
 * 
 *    Rev 1.15   Aug 06 1997 15:38:24   DPI
 * Correction emission en backup numero
 * de fichier sans extension
 * 
 *    Rev 1.14   Aug 01 1997 17:32:02   HMO
 *  
 * 
 *    Rev 1.13   Jul 28 1997 11:28:50   HMO
 * Modification des traces
 * Utilisation de FIC_EstRepertoireVide
 * 
 *    Rev 1.12   Jul 21 1997 10:30:40   HMO
 * Gestion de la purge sur tout les fichiers
 * 
 *    Rev 1.11   Jul 09 1997 17:54:28   HMO
 * L'arret n'attend pas la fin du backup
 * 
 *    Rev 1.10   Jul 09 1997 17:36:20   HMO
 * Arret du module quand il n'y a pas de backup en cours
 * ou quand le backup en cours se termine
 * 
 *    Rev 1.9   Jul 03 1997 09:54:16   HMO
 * Contrôle de la fin de backup sur l'année, le mois et le jour
 * 
 *    Rev 1.8   Jun 13 1997 10:34:12   HMO
 *  
 * 
 *    Rev 1.7   Jun 10 1997 11:59:54   DPI
 * Ouverture du service FTP une seule fois.
 * Idem fermeture
 * 
 *    Rev 1.6   May 27 1997 09:30:48   ANA
 * Modifications pour la restitution
 * 
 *    Rev 1.5   May 16 1997 13:32:56   ANA
 * Remplacement du fichier emi_act.c par msg_fic.c
 * 
 * 
 *    Rev 1.4   May 13 1997 17:20:48   ANA
 *  
 * 
 *    Rev 1.3   May 13 1997 16:51:32   ANA
 * Gestion de la sauvegarde sur disque et de la purge,
 * Nouvel algorithme de recherche du  fichier de départ.
 * 
 * 
 *    Rev 1.2   Apr 15 1997 10:03:36   ANA
 * Correction du bug sur le service FICHIER deTCP/IP
 * 
 *    Rev 1.1   Apr 07 1997 11:51:46   ANA
 * Nouvelle gestion du "cold start"
 * 
 *    Rev 1.0   Mar 21 1997 09:24:54   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <conio.h>
#include <io.h>
//#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include <time.h>

#include "err.h"
#include "fic.h"
#include <str.h>

/* module NOYAU */
#include <noyau.h>
//#include <tcp_ip.h>

/* module LAN */
#include <csr_lan.h>

/* module Horodate */
#include "horodate.h"


#include "emi_fic.h"
#include "emi_glob.h"
#include "emi_serv.h"
#include "emi_mess.h"

#define LOC_DEF
#include "emi_back.h"
#undef LOC_DEF

#include <fic_conf.h>
#include <msg_fic.h>
/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/
/* Cette valeur est li‚e aux bios qui commence … cette ann‚e */
#define PREMIERE_ANNEE_POSSIBLE 1994
#define TEXTE_PURGE  "PURGE"
#define TEXTE_BACKUP "BACKUP"

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

/*--------------- VARIABLES: ---------------*/
PRIVATE boolean temoin_arret_backup;
PRIVATE struct_backup backup;

/* tableau donnant le nombre de jour dans chaque mois */
/* en tenant compte des annees bisextilles.           */
PRIVATE short int jour_mois[4][12] =
{
   /* J   F   M   A   M   J   J   A   S   O   N   D */
   { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
   { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
   { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
   { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};


/*--------------- CODE: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned int EmiBackup(void)
* PARAMETRES: aucun
* RETOUR:     code
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme
* ROLE: Recuperer les messages qui arrivent dans la boite aux lettres
*       de la tache de backup et les traiter
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED DWORD WINAPI EmiBackup (PVOID param)
{
   struct_emi_message *p_message = (struct_emi_message *)(0);
   noyau_enum_retour  code_rtc;
   noyau_bal_id  bal;
   boolean    suivant;

   /********************************/
   /* Initialisation de la tache   */
   /* et changement de sa priorite */
   /********************************/

   //DebutRegionLocale();

   InitBackup();

   ChangePriorite (TacheCourante(),EMI.priorite_max);

   bal = EMI.util.bal_backup;

   //FinRegionLocale();

   /*************************/
   /* Traitement periodique */
   /*************************/

   while (TRUE)
   {
      /**********************************************/
      /* Attente de message dans la BAL de la tache */
      /**********************************************/
      code_rtc = Recoit(bal,(struct_neutre **)&p_message,EMI_TIMEOUT_BACKUP);

      DebutRegionLocale();

      if (code_rtc == NOYAU_BAL_MESS)
      {
         suivant = FALSE;
         /***********************************************************/
         /* Traitement des messages recus dans la boite aux lettres */
         /***********************************************************/
      	switch(p_message->entete.service )
         {
            case M_EMI_ARRET :
               EmiFichierTrace("%s : reception ARRET \n", TEXTE_BACKUP);
               temoin_arret_backup = TRUE;
            break;

            case M_EMI_RESTITUTION :
               /* si une restitution est deja en cours */
               /* on replace le message dans la boite au lettre */
               if( EMI.msg_etat.backup == EMI_BACKUP_EN_COURS ||
                   EMI.msg_etat.backup == EMI_PURGE_EN_COURS  )
               {
                  EmiFichierTrace("Restitution (ou purge) retardee car une deja en cours");
                  ExitEnvoie(bal,bal,(struct_neutre *)p_message);
                  suivant = TRUE;
               }
               else
               {
                  /* Memorisation du type de backup */
                  EMI.util.restit_type = p_message->u.msg_restit.type;
                  EMI.util.mode = p_message->u.msg_restit.mode;
                  if( EMI.util.restit_type == RESTIT_DATE)
                     EmiFichierTrace("%s : debut par DATE\n", TEXTE_BACKUP);
                  else
                     EmiFichierTrace("%s : debut par NUMERO\n", TEXTE_BACKUP);

                  EmiRestitution(&p_message->u.msg_restit,
	                              &EMI.msg_etat,
	                              &EMI.num_fichier);
               }
            break;

            default:
               EmiFichierTrace("%s : message inconnu %d\n", TEXTE_BACKUP, p_message->entete.service);
            break;
         }

         /* le message n'est lib‚r‚ que s'il a ‚t‚ traite */
         if( !suivant )
         {
            ExitLibere ((struct_neutre **)&p_message);
         }
      }


      if( temoin_arret_backup)
      {
         EmiFichierTrace("%s : ARRET effectue \n", TEXTE_BACKUP);
         EmiEnvoiAcquittement(EMI.util.bal_id,M_EMI_ARRET,EMI_DEMANDE_ACQ);
         Termine();
      }
      else
      {
         EmiGestionEtat(&EMI.msg_etat, &EMI.ancien_etat);
         EmiGestionPurge(&EMI.msg_etat,&EMI.num_fichier);
         EmiGestionBackup(&EMI.msg_etat.backup, &backup, &EMI.util);
      }

      FinRegionLocale();
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void InitBackup(void)
* PARAMETRES: aucun
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: initialisation des variables utilisees par la tache.
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED void InitBackup (void)
{
   /**************************************************************/
   /* On positionne le temoin qui permet de savoir si l'arret de */
   /* la tache a ete demande                                     */
   /**************************************************************/
   noyau_bal_id bal;
	
	temoin_arret_backup = FALSE;

	
   /* mailboxes */

   // publication of internal mailbox
	EMI.util.bal_backup = PublieBAL (EMI.util.BackupBalNam, NOYAU_BAL_ILLIMITEE);
    if (EMI.util.bal_backup <= 0)
        ExitBad();

    // Waiting for exported mailbox publication
    bal = AttendBAL (EMI.util.BalNam);
    if (bal <= 0)
        ExitBad();
   


   backup.flag_date = FALSE;
   backup.etat = FICHIER_DATE_ABSENT;
   backup.nb_fichier_trouve = 0;
   backup.nb_fichier_cherche = 0;

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void EmiRestitution(struct_emi_gestion_restit *p_msg_restit,
*                              struct_emi_gestion_etat *p_etat,
*	   		      struct_emi_num_fichier *num_fichier)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche des fichiers … restituer.
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED void EmiRestitution( struct_emi_gestion_restit *p_msg_restit,
                               struct_emi_gestion_etat *p_etat,
			      struct_emi_num_fichier *num_fichier)
{
   backup.nb_fichier_trouve = backup.nb_fichier_cherche = 0;

   EmiTrouveNumero(p_msg_restit,p_etat,num_fichier);

   if( p_etat->backup == EMI_BACKUP_ERR_NUMERO ||
       p_etat->backup == EMI_BACKUP_ERR_DATE || !EMI.config_received ||
	   (!EMI.authorized && (p_msg_restit->action != RESTIT_DISQUE)))
   {
      /* non acquittement de la demande */
      EmiEnvoiAcquittement(EMI.service[M_EMI_RESTITUTION].demandeur[0],
                                  M_EMI_RESTITUTION,EMI_DEMANDE_NACQ);
   }
   else
   {
      /* acquittement de la demande */
      EmiEnvoiAcquittement(EMI.service[M_EMI_RESTITUTION].demandeur[0],
                                  M_EMI_RESTITUTION,EMI_DEMANDE_ACQ);

      p_etat->backup = EMI_BACKUP_EN_COURS;

      if( p_msg_restit->type == RESTIT_DATE )
      {
         backup.nb_fichier_cherche ++;
         if( RechercherFichierParDate(backup.courante,
                                   &num_fichier->premier_backup,
                                   &num_fichier->dernier_backup,
                                   backup.iteration) == EMI_PAS_ERREUR )
         {
            /* couple suivant */
            backup.iteration ++;
            num_fichier->backup = num_fichier->premier_backup;
            backup.etat = FICHIER_DATE_PRESENT;
            backup.nb_fichier_trouve ++;
         }
         else
         {
            backup.etat = FICHIER_DATE_ABSENT;
            EmiFichierTrace( "%s : Fichier Date %2d/%02d/%04d Absent !! \n",
                             TEXTE_BACKUP,
                             backup.courante.wDay,
                             backup.courante.wMonth,
                             backup.courante.wYear);
         }
      }
      /* memorisation des numeros */
      backup.ptr_premier = &num_fichier->premier_backup;
      backup.ptr_dernier = &num_fichier->dernier_backup;
      backup.ptr_courant = &num_fichier->backup;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED short int EmiTrouveNumero( )
* PARAMETRES:
* RETOUR:     envoi correct ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche des premier et dernier numeros de fichier
*       soit directement, soit a partir d'une date
*       pour la restitution et pour la purge;
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmiTrouveNumero(struct_emi_gestion_restit *p_msg,
                               struct_emi_gestion_etat *p_etat,
                               struct_emi_num_fichier *num_fichier)
{
   SYSTEMTIME date_debut;
   SYSTEMTIME date_fin;

   /* purge, restitution ou backup manuel */
   backup.action = p_msg->action;

   /* recherche des numeros de fichier */
   /* selon le type de backup choisi  */
   switch ( p_msg->type )
   {
      case RESTIT_DATE :
         /* convertir la date au format systeme */
         ConvertirDate(p_msg->t.date.debut,&date_debut);
         ConvertirDate(p_msg->t.date.fin,&date_fin);

         /* verifier la coherence des dates */
         if( VerifDate(date_debut.wYear,
                       date_debut.wDay,
                       date_debut.wMonth,
                       date_fin.wYear,
                       date_fin.wDay,
                       date_fin.wMonth) == FALSE)
         {
            p_etat->backup = EMI_BACKUP_ERR_DATE;
            EmiEnvoiAppli(M_EMI_ETAT, EMI_NOUVEL_ETAT,
                          (struct_emi_gestion_etat *)p_etat);
            EmiFichierTrace("%s : Date incoherente !! \n", TEXTE_BACKUP);
            break;
         }

         backup.flag_date = TRUE;
         /* sauvegarder les dates de debut et de fin */
         backup.debut = date_debut;
         backup.fin = date_fin;

         /* premiere date */
         backup.courante = date_debut;

         /* initialisation de l'iteration pour les couples[premier-dernier] */
         backup.iteration = 0;
      break;

      case RESTIT_NUMERO :
         backup.flag_date = FALSE;
         /* pour backup par numero */
         num_fichier->premier_backup = p_msg->t.numero.debut;
         num_fichier->dernier_backup = p_msg->t.numero.fin;
         num_fichier->backup = num_fichier->premier_backup;
          /* verifier la coherence des numeros de fichier    */
         /* ne pas depasser le numero de fichier courant !!!*/
         if( num_fichier->premier_backup > num_fichier->courant
          || num_fichier->dernier_backup > num_fichier->courant )
         {
            p_etat->backup = EMI_BACKUP_ERR_NUMERO;
            EmiEnvoiAppli(M_EMI_ETAT, EMI_NOUVEL_ETAT,(struct_emi_gestion_etat *) p_etat);
         }
      break;

      default:
      break;
   }


}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void RestitutionFichierNumero(struct_backup *ptr_backup,
*                                        struct_emi_utile *p_util)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Effectue la restitution en fonction du type choisi.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void RestitutionFichierNumero(struct_backup *ptr_backup,
                                           struct_emi_utile *p_util)
{
   boolean              trouve;
   boolean              valide;
   boolean              ecrit;
   char                 path_fichier[MAX_PATH];
   char                 ext[MAX_PATH];
   char					newname[MAX_PATH];
   //char                 *path_distant;
   char					path_distant[MAX_PATH];
   FILE                 *fichier;
   //noyau_event_id       event;
   short int            status;
 		

   valide = TRUE;

   /* Recherche du fichier */
   trouve = RechercherFichier( *ptr_backup->ptr_courant,
                               FICHIER_SANS_EXTENSION,
                               path_fichier);
   if( trouve)
   {
      /* Verification que le fichier possede bien une extension */
      FIC_splitpath( path_fichier, NULL, NULL, NULL, ext);

      /* il se peut que ext soit la chaine vide */
      if( ext[0] != '.' || ! isdigit( ext[1]))
         valide = FALSE;
   }
   else if( ( *ptr_backup->ptr_courant >= *ptr_backup->ptr_dernier) &&
            ( ptr_backup->flag_date ))
   {

        EmiFichierTrace("%s dernier fichier non cree atteint'\n", TEXTE_BACKUP);
       /* il faut s'arreter sinon on cree un fichier vide */
       (*ptr_backup->ptr_courant)++;
       return;
   }

   if( ! valide)
   {
       /* On arrete le traitement et on passe au suivant
        * qui n'existe pas (c'etait le dernier)
        */
       (*ptr_backup->ptr_courant)++;
       return;
   }

   switch( ptr_backup->action )
   {
      case RESTIT_FIC :
            if( trouve == FALSE)
            {
               /* si le fichier n'existe pas , on cree un fichier */
               fichier = CreerFichier( *ptr_backup->ptr_courant, 0, path_fichier);
               ERR_EstVrai( fichier != NULL);

               ecrit = TerminerFichier( fichier, path_fichier, FALSE);
               ERR_EstVrai( ecrit);
            }

            if(p_util->restit_type == RESTIT_DATE)
            {
               //path_distant = p_util->path_backup_date;
				STR_strcpy(MAX_PATH,path_distant, p_util->path_backup_date);
            }
            else
            {
               //path_distant = p_util->path_backup_numero;
				STR_strcpy(MAX_PATH,path_distant, p_util->path_backup_numero);
            }

			if(p_util->mode == EMI_RESTIT_AUTOMATIQUE)
				sprintf_s(newname, sizeof(newname), "%s%04d%s%04d%s%08d", "\\MSG.PCS", EMI.plaza_number, ".L", EMI.lane_number, ".S", *ptr_backup->ptr_courant);
			else
				sprintf_s(newname, sizeof(newname), "%s%04d%s%04d%s%08d", "\\BAK.PCS", EMI.plaza_number, ".L", EMI.lane_number, ".S", *ptr_backup->ptr_courant);


			if (path_distant[strlen(path_distant) - 1] == '\\')
				path_distant[strlen(path_distant) - 1] = '\0';
			
			STR_strcat(MAX_PATH, path_distant, newname);

            if (EMI.authorized)
			{
				EmiFichierTrace("%s : emettre '%s'\n", TEXTE_BACKUP, path_fichier);
		        EmettreFichier( path_fichier, path_distant,
	   		                   p_util->bal_fic, p_util->bal_id);
			}
			else
			{
				EmiFichierDebug("RestitutionFichierNumero => Transfer not authorized");
				break;
			}

            /* attente acquittement FIC */
            FinRegionLocale();
            status =  AttenteEvt( EMI.event, 180);
            DebutRegionLocale();

            if(status != NOYAU_OK)
               EmiFichierTrace("%s : AttenteEvt emission FIC: fin timeout \n", TEXTE_BACKUP);
            /* Annuler les evenements survenus */
            EffaceEvt (EMI.event);

            /* a noter que l'increment est effectuer dans la tache Emission
             * qui exploite le retour de FIC
             */
            break;

      case RESTIT_DISQUE:
            if( trouve )
            {
               EmiFichierTrace("%s : -> Copie du fichier '%s'\n", TEXTE_BACKUP, path_fichier);
			   if(p_util->mode == EMI_RESTIT_AUTOMATIQUE)
				   sprintf_s(newname, sizeof(newname), "%s%04d%s%04d%s%08d", "MSG.PCS", EMI.plaza_number, ".L", EMI.lane_number, ".S", *ptr_backup->ptr_courant);
			   else
				   sprintf_s(newname, sizeof(newname), "%s%04d%s%04d%s%08d", "BAK.PCS", EMI.plaza_number, ".L", EMI.lane_number, ".S", *ptr_backup->ptr_courant);

               CopierFichierDisque(path_fichier, newname);
            }
            else
            {
               EmiFichierTrace("%s : -> fichier '%s' absent\n", TEXTE_BACKUP, path_fichier);
            }

            /* on passe au fichier suivant */
            (*ptr_backup->ptr_courant)++;
            break;

      case RESTIT_PURGE :
            EmiFichierTrace("%s : -> Effacement du fichier %s\n",TEXTE_PURGE, path_fichier);
            if( trouve )
               EffacerFichier(*ptr_backup->ptr_courant,FICHIER_SANS_EXTENSION);

            /* on passe au fichier suivant */
            (*ptr_backup->ptr_courant)++;
            break;

      default:
      break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void EmiTermineRestitution(struct_backup *ptr_backup,
*                                     enum_emi_etat * ptr_etat)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Termine la restitution en pr‚cisant s'il elle a ‚t‚ complŠte
*       ou non.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmiTermineRestitution(struct_backup * ptr_backup,
                                     enum_emi_etat * ptr_etat)
{
   switch( ptr_backup->action )
   {
      case RESTIT_PURGE :
         *ptr_etat = EMI_PURGE_TERMINE;
         break;

      case RESTIT_FIC :
      case RESTIT_DISQUE :
         EmiFichierTrace("%s : -> backup termine \n",TEXTE_BACKUP);
         if( ptr_backup->flag_date )
         {
            if( ptr_backup->nb_fichier_trouve == 0 )
            {
               *ptr_etat = EMI_AUCUN_FICHIER;
            }
            else if( ptr_backup->nb_fichier_trouve < ptr_backup->nb_fichier_cherche )
            {
              *ptr_etat = EMI_BACKUP_TERMINE_INCOMPLET;
            }
            else if( ptr_backup->nb_fichier_trouve == ptr_backup->nb_fichier_cherche )
            {
               *ptr_etat = EMI_BACKUP_TERMINE_COMPLET;
            }
         }
         else
            *ptr_etat = EMI_BACKUP_TERMINE_COMPLET;
      break;

      default:
         EmiFichierTrace("BACKUP_PURGE Pb\n");
      break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void EmiGestionPurge()
* PARAMETRES: etat de la purge periodique des fichiers
* RETOUR: numero des fichiers … purger
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Verifie l'heure courante et l'heure demand‚e pour une purge,
*       Entame la purge des qu'il est l'heure.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmiGestionPurge(struct_emi_gestion_etat *p_etat,
                               struct_emi_num_fichier *p_num_fichier)
{
   SYSTEMTIME         time;
   struct_emi_gestion_purge msg_purge;
   boolean                  purge_demandee;

   /* Gestion autonome de la purge des fichiers */
   /* tous les jours … 3h15 du matin */
   GetLocalTime(&time);

   /* si la purge est demand‚e,
   /* Si c'est l'heure de la purge des fichiers */
   /* et que la demande n'a pas encore ete prise en compte */

   if( (EMI.periode_purge > 0 ) &&
       (p_etat->backup != EMI_PURGE_EN_COURS) &&
       (EMI.heure_purge.wMinute == time.wMinute) &&
       (EMI.heure_purge.wHour   == time.wHour))
   {
      purge_demandee = TRUE;
   }
   else
   {
      purge_demandee = FALSE;
   }

   if( purge_demandee)
   {
      EmiFichierTrace("%s : Debut a %d:%02d \n", TEXTE_PURGE, time.wHour, time.wMinute);

      msg_purge.action = RESTIT_PURGE;
      msg_purge.type   = RESTIT_DATE;

      /* le dernier jour de purge est le jour J-X */
	  DatePurgeFin(EMI.periode_purge, msg_purge.t.date.fin, sizeof(msg_purge.t.date.fin));

      /* le premier jour de purge est le jour le plus ancien */
	  DatePurgeDebut(EMI.periode_purge, msg_purge.t.date.debut, sizeof(msg_purge.t.date.debut));

      EmiTrouveNumero(&msg_purge,p_etat,p_num_fichier);

      if( RechercherFichierParDate(backup.courante,
                                   &p_num_fichier->premier_backup,
                                   &p_num_fichier->dernier_backup,
                                   backup.iteration) == EMI_PAS_ERREUR )
      {
         EmiFichierTrace( "%s : -> Analyse du fichier date %2d/%02d/%04d \n",
                          TEXTE_PURGE,
                          backup.courante.wDay,
                          backup.courante.wMonth,
                          backup.courante.wYear);
         /* couple suivant */
         backup.iteration ++;
         /* memorisation des numeros */
         p_num_fichier->backup = p_num_fichier->premier_backup;
         backup.ptr_courant = &p_num_fichier->backup;
         backup.ptr_premier = &p_num_fichier->premier_backup;
         backup.ptr_dernier = &p_num_fichier->dernier_backup;

         p_etat->backup = EMI_PURGE_EN_COURS;
         backup.etat = FICHIER_DATE_PRESENT;
      }
      else
      {
         backup.etat = FICHIER_DATE_ABSENT;
         EmiFichierTrace( "%s : Fichier Date %2d/%02d/%04d Absent !! \n",
                          TEXTE_PURGE,
                          backup.courante.wDay,
                          backup.courante.wMonth,
                          backup.courante.wYear);
      }
    }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void EmiGestionBackup(enum_emi_etat etat_backup)
* PARAMETRES: etat de la demande de restitution
* RETOUR:     rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmiGestionBackup(enum_emi_etat *p_etat_backup,
                                struct_backup *ptr_backup,
                                struct_emi_utile *p_util)
{
   short int        status;
   SYSTEMTIME time;
   char             *trace = TEXTE_BACKUP;

   switch(*p_etat_backup)
   {
      case EMI_PURGE_EN_COURS :
         trace = TEXTE_PURGE;
      case EMI_BACKUP_EN_COURS :
         /* cas de la restitution par numero */
         if( ! ptr_backup->flag_date )
         {
            if( *ptr_backup->ptr_courant > *ptr_backup->ptr_dernier )
            {
               GetLocalTime(&time);
               EmiFichierTrace( "%s : Fin a %d:%d \n",
                                      trace,
                                      time.wHour,
                                      time.wMinute);
               ptr_backup->nb_fichier_cherche --;
               EmiTermineRestitution(ptr_backup, p_etat_backup);
               break;
            }
         }
         else
         {
            /* verifier le numero du fichier en cours */
            if( *ptr_backup->ptr_courant > *ptr_backup->ptr_dernier
              || ptr_backup->etat == FICHIER_DATE_ABSENT)
            {
               /* pour une restitution par date */
               /* prochain couple [premier-dernier] */
               ptr_backup->nb_fichier_cherche ++;
               status = RechercherFichierParDate(ptr_backup->courante,
                                                 ptr_backup->ptr_premier,
                                                 ptr_backup->ptr_dernier,
                                                 backup.iteration);
               if( status == EMI_PAS_ERREUR )
               {
                  /* couple suivant */
                  backup.iteration ++;
                  *ptr_backup->ptr_courant = *ptr_backup->ptr_premier;
                  ptr_backup->etat = FICHIER_DATE_PRESENT;
                  ptr_backup->nb_fichier_trouve ++;
               }
               else
               {
                  /* couple non trouve => date suivante */
                  /* effacement du fichier jj.txt lors de la purge */
                  /* une fois que le fichier a ete entierement lu */
                  if(ptr_backup->action == RESTIT_PURGE )
                  {
                     EmiFichierTrace( "%s : -> Effacement du fichier date %2d/%02d/%04d \n",
                                      trace,
                                      ptr_backup->courante.wDay,
                                      ptr_backup->courante.wMonth,
                                      ptr_backup->courante.wYear);
                     EffacerFichierDate(ptr_backup->courante);
                  }

                  /* Passage au prochain fichier */
                  status = DateFichierSuivant(ptr_backup);
                  if ( status == EMI_RESTIT_OK)
                  {
                     EmiFichierTrace( "%s : -> Analyse du fichier date %2d/%02d/%04d \n",
                                      trace,
                                      ptr_backup->courante.wDay,
                                      ptr_backup->courante.wMonth,
                                      ptr_backup->courante.wYear);

                     /* repartir … 0 pour les couples */
                     backup.iteration = 0;
                     status = RechercherFichierParDate(ptr_backup->courante,
                                      ptr_backup->ptr_premier,
                                      ptr_backup->ptr_dernier,
                                      backup.iteration);
                     if( status == EMI_PAS_ERREUR )
                     {
                        backup.iteration ++;
                        *ptr_backup->ptr_courant = *ptr_backup->ptr_premier;
                        ptr_backup->etat = FICHIER_DATE_PRESENT;
                        ptr_backup->nb_fichier_trouve ++;
                     }
                     else
                     {
                        /* fichier absent */
                        ptr_backup->etat = FICHIER_DATE_ABSENT;
                        EmiFichierTrace( "%s : Fichier Date %2d/%02d/%04d Absent !! \n",
                                         trace,
                                         ptr_backup->courante.wDay,
                                         ptr_backup->courante.wMonth,
                                         ptr_backup->courante.wYear);
                     }
                  }
                  else
                  {
                     GetLocalTime(&time);
                     EmiFichierTrace( "%s : Fin a %d:%d \n",
                                      trace,
                                      time.wHour,
                                      time.wMinute);
                     ptr_backup->nb_fichier_cherche --;
                     EmiTermineRestitution(ptr_backup, p_etat_backup);
                  }
               }
            }
         }

         //there is posibility that EmiTermineRestitution 
		 //changes p_etat_backup
		 if( (ptr_backup->etat == FICHIER_DATE_PRESENT ||
             ptr_backup->flag_date == FALSE ) 
			 && (*p_etat_backup == EMI_PURGE_EN_COURS || 
			 *p_etat_backup == EMI_BACKUP_EN_COURS) )
            RestitutionFichierNumero(ptr_backup, p_util);
      break;

      case EMI_BACKUP_ERR_NUMERO :
      case EMI_BACKUP_ERR_DATE :
      case EMI_BACKUP_TERMINE_COMPLET:
      case EMI_BACKUP_TERMINE_INCOMPLET:
      case EMI_PURGE_TERMINE :
      case EMI_AUCUN_FICHIER :
         *p_etat_backup = EMI_INACTIF;
      break;

      default:
      break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED boolean VerifDate()
* PARAMETRES: date de debut, date de fin
* RETOUR:     date VALIDE /NON VALIDE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Verifie la coherence des dates de restitution ou de purge
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE boolean VerifDate(short int start_year,
                          short int start_day,
                          short int start_month,
                          short int end_year,
                          short int end_day,
                          short int end_month)
{
   /* verification des jour et des mois */
   if ((start_day<1) || (start_day>31))
      return (FALSE);
   if ((start_month<1) || (start_month>12))
      return (FALSE);

   if ((end_day<1) || (end_day>31))
      return (FALSE);
   if ((end_month<1) || (end_month>12))
      return (FALSE);

   if (start_day>jour_mois[start_year&0x03][start_month-1])
      return (FALSE);
   if (end_day>jour_mois[end_year&0x03][end_month-1])
      return (FALSE);

   if (start_year > end_year)
      return (FALSE);
   else
      return (TRUE);

}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: int ComparerDatesJour( SYSTEMTIME *debut,
*                                 SYSTEMTIME *fin)
* PARAMETRES: date de debut, date de fin
* RETOUR: < 0 date de debut < date de fin
*         > 0 date de debut > date de fin
*         = 0 date de debut = date de fin
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Compare deux dates
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED int ComparerDatesJour( SYSTEMTIME *debut, SYSTEMTIME *fin)
{
   int retour;

   /* Il faut typer les donn‚es car ce sont des unsigned char */
   /* comparaison a la date de fin de backup sur l'ann‚e, le mois et le jour */
   retour = (signed int)debut->wYear - (signed int)fin->wYear;
   if( retour == 0)
   {
      retour = (signed int)debut->wMonth - (signed int)fin->wMonth;
      if(retour == 0)
         retour = (signed int)debut->wDay - (signed int)fin->wDay;
   }

   return( retour);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PRIVATE enum_restit_erreur DateFichierSuivant(struct_backup *p_date)
* PARAMETRES: date courante de restitution
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Donne le fichier suivant a partir d'une date donnee.
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE enum_restit_erreur DateFichierSuivant(struct_backup *p_date)
{
   enum_restit_erreur retour;
   boolean            continuer;
   boolean            ok_year,
                      ok_month,
                      ok_day;
   char               path_fichier[MAX_PATH];


   /* On considŠre la recherche valide */
   continuer = TRUE;

   /* Recherche du prochain fichier existant */
   p_date->courante.wDay += 1 ;

   /*************************************************/
   /* Recherche jusqu'a trouver un fichier existant */
   do
   {
      /************************************/
      /* V‚rification de l'ann‚e courante */
      ok_year = FALSE;
      do
      {
         ObtenirFichierDateAnnee(p_date->courante, path_fichier);
         if( _access(path_fichier,0 ) == 0)
            ok_year = TRUE;
         else
         {
            /* Positionnement sur l'ann‚e suivante */
            p_date->courante.wYear++;
            p_date->courante.wMonth = 1;
            p_date->courante.wDay = 1;
         }
      } while( ok_year == FALSE);

      /********************************/
      /* V‚rification du mois courant */
      ok_month = FALSE;
      do
      {
         ObtenirFichierDateMois(p_date->courante, path_fichier);
         if( _access(path_fichier,0 ) == 0)
            ok_month = TRUE;
         else
         {
            /* Positionnement sur le mois suivant */
            p_date->courante.wMonth++;
            p_date->courante.wDay = 1;

            if( p_date->courante.wMonth > 12)
            {
               /* Il faut effacer le r‚pertoire ann‚e pr‚c‚dent
                * car la VM a ‚t‚ arret‚e juste entre la suppression
                * d'un fichier et la suppression du r‚pertoire
                */
               ObtenirFichierDateAnnee(p_date->courante, path_fichier);
               if( FIC_EstRepertoireVide( path_fichier))
               {
                  EmiFichierTrace( "%s : Suppression du repertoire %s car aucun repertoire mois \n",
                                   TEXTE_BACKUP,
                                   path_fichier);
                  FIC_deltree( path_fichier);
               }

               /* Il n'y a plus de mois, il faut passer */
               /* … une nouvelle ann‚e sans tester les dates */
               p_date->courante.wYear++;
               p_date->courante.wMonth = 1;


               /* On arrete la recherche du mois */
               break;
            }
         }
      } while( ok_month == FALSE);

      /* L'ann‚e et le mois sont valides */
      if( ok_month == TRUE)
      {
         /********************************/
         /* V‚rification du jour courant */
         ok_day = FALSE;
         do
         {
            ObtenirFichierDateJour(p_date->courante, path_fichier);

            /* comparaison … la date de fin de backup sur l'ann‚e, le mois et le jour */
            if( ComparerDatesJour(&p_date->courante, &p_date->fin) > 0)
            {
               /* la date est d‚pass‚e, il faut s'arreter */
               continuer = FALSE;
               break;
            }

            if( _access(path_fichier,0 ) == 0)
               ok_day = TRUE;
            else
            {
               /* Positionnement sur le mois suivant */
               p_date->courante.wDay++;

               if( p_date->courante.wDay >
                   jour_mois[p_date->courante.wYear&0x03][p_date->courante.wMonth-1])
               {
                  /* Il faut effacer le r‚pertoire mois pr‚c‚dent car
                   * la VM a ‚t‚ arret‚e juste entre la suppression
                   * d'un fichier et la suppression du r‚pertoire
                   */
                  ObtenirFichierDateMois(p_date->courante, path_fichier);
                  if( FIC_EstRepertoireVide( path_fichier))
                  {
                     EmiFichierTrace( "%s : Suppression du repertoire %s car aucun fichier jour \n",
                                      TEXTE_BACKUP,
                                      path_fichier);
                     FIC_deltree( path_fichier);
                  }

                  /* Il n'y a plus de jours, il faut passer */
                  /* … un nouveau mois */
                  p_date->courante.wMonth++;
                  p_date->courante.wDay = 1;
                  if( p_date->courante.wMonth > 12)
                  {
                     /* Il n'y a plus de mois, il faut passer */
                     /* … une nouvelle ann‚e */
                     p_date->courante.wYear++;
                     p_date->courante.wMonth = 1;
                  }

                  /* On arrete la recherche du jour */
                  break;
               }
            }
         } while( ok_day == FALSE);

      }
   /* La date n'a pas ‚t‚ d‚pass‚e et aucun jour n'est trouve */
   } while( continuer == TRUE && ok_day != TRUE);

   if( continuer)
      retour = EMI_RESTIT_OK;
   else
      retour = EMI_RESTIT_DATE_FIN;

   return retour;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void DatePurge(int nb_jours,struct dostime_t *)
* PARAMETRES: periode de purge en nombre de jours
*             pointeur sur la date recherch‚e
* RETOUR:     date des fichiers a purger
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Recherche de la date des fichiers … purger en fonction
*       de la periode de purge
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void DatePurgeFin(int nb_jours, char *date_purge, size_t sDate_purgeSize)
{
   unsigned long int nb_second;
   unsigned long int nb_second_from80;
   unsigned long int date_second;
   struct_hrd_date_entree_hex date;
   struct_hrd_heure_hex heure;

   nb_second_from80 = HRDSecondesDepuis80 (HRDDonneDate(), HRDDonneHeure());

   /* nombre de secondes correspondants au nb_jours */
   /* 1 jour = 86400 secondes */
   nb_second = 86400L * nb_jours;

   /* date recherch‚e en nombre de secondes depuis 1980 : J - X */
   date_second = nb_second_from80 - nb_second;

   /* conversion du nombre de secondes en date/heure */
   HRDConvSecondesEnDateHeure (date_second, &date, &heure);

   sprintf_s(date_purge,sDate_purgeSize,"%4d%02d%02d",date.annee,date.mois,date.jour);

}

PROTECTED void DatePurgeDebut(int nb_jours, char *date_purge, size_t sDate_purgeSize)
{
   SYSTEMTIME		date;
   int              annee = 0;
   int              mois = 0;
   int              jour = 0;
   char             path_fichier[MAX_PATH];
   char             date_purge_fin[9];
   boolean          ok_year,
                    ok_month,
                    ok_day;

   /* Gestion autonome de la purge des fichiers */
   /* tous les jours … 3h15 du matin */

   DatePurgeFin(nb_jours, date_purge_fin, sizeof(date_purge_fin));

   sscanf_s(date_purge_fin, "%4d%2d%2d", &annee, &mois, &jour);

   /* Recherche a partir de la premiŠre ann‚e possible */
   date.wYear  = PREMIERE_ANNEE_POSSIBLE-1; /* -1 car elle va ˆtre increment‚e de suite */
   date.wMonth = 0;        /* 0 car il va ˆtre incr‚met‚ aussi */
   date.wDay   = 0;        /* 0 car il va ˆtre incr‚met‚ aussi */

   /*************************************************/
   /* Recherche jusqu'a trouver un fichier existant */
   do
   {
      /* Recherche de la plus vielle ann‚e */
      ok_year = FALSE;
      do
      {
         /* Recherche sur l'ann‚e suivante */
         date.wYear++; /* Il en existe au moins une */
         ObtenirFichierDateAnnee(date,path_fichier);
         if( _access(path_fichier,0 ) == 0)
            ok_year = TRUE;
      } while( ok_year == FALSE);

      /* Recherche du plus vieux mois */
      ok_month = FALSE;
      do
      {
         /* Recherche sur le mois suivant */
         date.wMonth++;
         if( date.wMonth > 12)
         {
            /* On n'a pas trouv‚ de mois pour cette ann‚e */
            /* Il faut effacer le r‚pertoire car la VM */
            /* a ‚t‚ arret‚e juste entre la suppression */
            /* d'un fichier et la suppression du r‚pertoire */
            ObtenirFichierDateAnnee( date, path_fichier);
            if( FIC_EstRepertoireVide( path_fichier))
            {
               EmiFichierTrace( "%s : Suppression du repertoire %s car aucun repertoire mois \n",
                                TEXTE_PURGE,
                                path_fichier);
               FIC_deltree( path_fichier);
            }

            /* On arrete la recherche du mois */
            break;
         }
         ObtenirFichierDateMois(date,path_fichier);
         if( _access(path_fichier,0 ) == 0)
            ok_month = TRUE;
      } while( ok_month == FALSE);

      /* L'ann‚e et le mois sont valides */
      if( ok_month == TRUE)
      {
         /* Recherche du plus vieux jour */
         ok_day = FALSE;
         do
         {
            /* Recherche sur le jour suivant */
            date.wDay++;
            if( date.wDay >
                jour_mois[date.wYear&0x03][date.wMonth-1])
            {
               /* Il faut effacer le r‚pertoire car la VM */
               /* a ‚t‚ arret‚e juste entre la suppression */
               /* d'un fichier et la suppression du r‚pertoire */
               ObtenirFichierDateMois(date, path_fichier);
               if( FIC_EstRepertoireVide( path_fichier))
               {
                  EmiFichierTrace( "%s : Suppression du repertoire %s car aucun fichier jour \n",
                                   TEXTE_PURGE,
                                   path_fichier);
                  FIC_deltree( path_fichier);
               }

               /* On arrete la recherche du jour */
               break;
            }
            ObtenirFichierDateJour(date,path_fichier);
            if( _access(path_fichier,0 ) == 0)
               ok_day = TRUE;
         } while( ok_day == FALSE);
      }
   } while( ok_day != TRUE);


   /* Construction de la date de d‚but */
   sprintf_s(date_purge,sDate_purgeSize,"%4d%02d%02d",date.wYear,date.wMonth,date.wDay);

   /* Test entre la date de d‚but et la date de fin */
   if( strcmp(date_purge, date_purge_fin) > 0)
   {
      /* La date de d‚but d‚passe la date de fin, il ne faut pas */
      strncpy_s(date_purge, sDate_purgeSize, date_purge_fin, 9);
   }

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED boolean ConvertirDate(char *date_chaine,SYSTEMTIME *)
* PARAMETRES: date sou forme de caine de caracteres
*             pointeur sur la date recherch‚e
* RETOUR:     conversion OK /NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Convertir une date sous forme de chaine de caractere en une
*       date au format DOS (structure)
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED boolean ConvertirDate(char *date_chaine, SYSTEMTIME *date)
{
   int status;
   int annee, mois, jour;

   /* chaine de 9 caracteres */
   status = sscanf_s(date_chaine,"%4d%2d%2d",&annee,&mois,&jour);
   if( status == 3 )
   {
      date->wYear = annee;
      date->wMonth = mois;
      date->wDay = jour;
      return TRUE;
   }
   else
      return FALSE;

}
