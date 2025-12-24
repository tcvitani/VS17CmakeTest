/*------   (v) 1997 CS-Route   -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER MESSAGES
* FICHIER: emission.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Tache d'interface entre une application et la tache d'emission
*         des fichiers
*
* --------------------------------------------------------------------
* DESCRIPTION:
*       1 - GŠre les emissions "automatiques" de fichiers d'extension .000
*       2 - GŠre les demandes de restitution de fichier
*             - par date  (jour)
*             - par numero de fichier
*       3 - GŠre la purge des fichiers
* --------------------------------------------------------------------
* ARRET :
*        Lors de la reception de la demande d'arret, la tache
*        1 - envoie le dernier fichier
*        2 - demande l'arret de la tache de backup
*        Elle passe dans l'etat 'arret_demande', et ne traite plus
*        que les messages du service ARRET.
*        Lorsque la tache de backup s'arrete, elle demande l'arret
*        du service fichier, et cesse de traiter quoique ce soit.
* --------------------------------------------------------------------
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/Sources/emission.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:04   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.20   06 Nov 1997 15:18:34   DPI
 * Integration DUT 9.00
 * 
 *    Rev 1.19   03 Nov 1997 10:59:18   DPI
 * Reprise de la gestion de HRD
 * 
 *    Rev 1.18   30 Oct 1997 15:54:12   DPI
 * Suppression du chrono, remplacer par HRD
 * Formule de calcul de l'heure
 * 
 *    Rev 1.17   29 Oct 1997 19:42:12   DPI
 * Suppression des fonctions de debug
 * Modifs des fonctions Lance et Arret
 *
 * 
 *    Rev 1.16   29 Oct 1997 18:42:46   DPI
 * Evolution ExitAlloue... du noyau 6.00
 * 
 *    Rev 1.15   10 Oct 1997 17:08:12   HMO
 * Suppression des traces sur la durée de chrono
 * 
 *    Rev 1.14   25 Sep 1997 16:01:56   HMO
 * Corection erreurs
 *
 *    Rev 1.13   23 Sep 1997 14:39:36   HMO
 * Modifications effectuées sur site
 * 
 *    Rev 1.12   Aug 01 1997 17:28:08   HMO
 *  
 * 
 *    Rev 1.11   Jul 21 1997 18:25:44   HMO
 * Ajout d'un DelaiTache
 * 
 *    Rev 1.10   Jul 09 1997 17:36:26   HMO
 *  
 * 
 *    Rev 1.9   Jul 08 1997 11:08:22   DPI
 * Modification de l'arret pour
 * - emettre le dernier fichier
 * - attendre la tache backup
 * 
 *    Rev 1.8   Jun 20 1997 17:35:34   HMO
 * Gestion du fichier Cold Start jusqu'a ce qu'il soit envoyé, même sur warm start
 * et suppression du fichier Cold Start quand il a été envoyé
 * 
 *    Rev 1.7   Jun 13 1997 10:34:22   HMO
 *  
 * 
 *    Rev 1.6   Jun 11 1997 16:13:44   HMO
 * Sortie de la gestion d'emission des fichiers lors de la reception d'un message
 * 
 *    Rev 1.5   Jun 10 1997 12:00:02   DPI
 * Ouverture du service FTP une seule fois.
 * Idem fermeture
 * 
 *    Rev 1.4   May 16 1997 13:33:04   ANA
 * Remplacement du fichier emi_act.c par msg_fic.c
 * 
 * 
 *    Rev 1.3   May 13 1997 16:51:36   ANA
 * Gestion de la sauvegarde sur disque et de la purge,
 * Nouvel algorithme de recherche du  fichier de départ.
 * 
 * 
 *    Rev 1.2   Apr 15 1997 10:03:40   ANA
 * Correction du bug sur le service FICHIER deTCP/IP
 * 
 *    Rev 1.1   Apr 07 1997 11:51:50   ANA
 * Nouvelle gestion du "cold start"
 * 
 *    Rev 1.0   Mar 21 1997 09:24:56   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <conio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dos.h>
#include <time.h>


/* outils_C */
#include <str.h>
#include <err.h>

/* module NOYAU */
#include <noyau.h>
//#include <tcp_ip.h>

/* module LAN */
#include <csr_lan.h>

/* module Horodate */
#include "horodate.h"

/* module EMISSION */
#include "emi_fic.h"
#include "emi_glob.h"

#include "emi_serv.h"
#include "emi_mess.h"

#include <fic_conf.h>
#include <msg_fic.h>
#include "fic.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/


/*--------------- TYPEDEFS: ---------------*/



/*--------------- FUNCTIONS: ---------------*/
PROTECTED void InitEmission (void);
PROTECTED void ReceptionEmiService(struct_emi_message *);
PROTECTED void ReceptionHrd(struct_hrd_message *p_message);

PROTECTED void EmissionCourante(long, struct_emi_utile );

//PROTECTED void CreerFichierColdStart(void);
//PROTECTED void EnvoiFichierColdStart(struct_emi_utile *);

PRIVATE void    ArmeReveilRegulier( void);

/*--------------- VARIABLES: ---------------*/
PRIVATE boolean temoin_arret_demande;

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

PROTECTED DWORD WINAPI Emission (PVOID param)
{
   struct_neutre      *p_message = (struct_neutre *)(0);
   noyau_enum_retour  code_rtc;
   noyau_bal_id       bal;
   boolean            temoin_arret_total;

   /********************************/
   /* Initialisation de la tache   */
   /* et changement de sa priorite */
   /********************************/

   
   
   InitEmission();

   DebutRegionLocale();
   
   bal = EMI.util.bal_id;
   temoin_arret_total = FALSE;


   //DebutRegionLocale();
   
   ChangePriorite (TacheCourante(), EMI.priorite_max);

   FinRegionLocale();


   /*************************/
   /* Traitement periodique */
   /*************************/
   while (TRUE)
   {
      /*****************************************************/
      /* Attente de message dans la BAL de la tache */
      /*****************************************************/
      code_rtc = Recoit(bal, &p_message, EMI_TIMEOUT_EMISSION);

      DebutRegionLocale();

      if (code_rtc == NOYAU_BAL_MESS)
      {
         if( ! EMI.temoin_arret_emission)
         {
            /***********************************************************/
            /* Traitement des messages recus dans la boite aux lettres */
            /***********************************************************/
            if(p_message->bl_retour == EMI.util.bal_fic )
            {
               /* messages recu du module LAN */
               ReceptionEmiFic((struct_lan_message *)p_message);
            }
            else if( p_message->bl_retour == EMI.util.bal_hrd)
            {
               /* si on est en arret, on ne fait plus les emissions
                * automatiques
                */
               if( ! temoin_arret_demande)
               {
                  ReceptionHrd((struct_hrd_message *)p_message);
               }
            }
            else
               ReceptionEmiService((struct_emi_message *)p_message);

            /* le service FICHIER a ete ouvert et acquitte */
            if( EMI.flag_chrono && EMI.flag_emission )
            {
               EMI.flag_chrono = FALSE;
               EmissionCourante(EMI.num_fichier.courant,EMI.util);
            }
         }

         ExitLibere (&p_message);
      }

      if( ! temoin_arret_demande)
         EmiGestionEtat(&EMI.msg_etat, &EMI.ancien_etat);
      else
      {
         if( EMI.temoin_arret_emission && ! temoin_arret_total)
         {
            EmiFichierTrace("EMISSION : ARRET effectue \n");
            /* fermer le service FICHIER */
            EmiEnvoiFic( M_LAN_FICHIER, LAN_FIN, NULL);
            /* Acquitter la demande d'arret */
            EmiEnvoiAcquittement(EMI.service[M_LAN_ARRET].demandeur[0],
                                 M_EMI_ARRET, EMI_DEMANDE_ACQ);
            temoin_arret_total = TRUE;
         }
      }

      FinRegionLocale();
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void InitEmission(void)
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

PROTECTED void InitEmission (void)
{
   short int id_serv, j;
   noyau_bal_id  bal;

   /**************************************************************/
   /* On positionne le temoin qui permet de savoir si l'arret de */
   /* la tache a ete demande                                     */
   /**************************************************************/
   temoin_arret_demande = FALSE;

	/* mailboxes */
    // publication of exported mailbox
    EMI.util.bal_id = PublieBAL (EMI.util.BalNam,NOYAU_BAL_ILLIMITEE);
    if (EMI.util.bal_id <= 0)
        ExitBad();

    // Waiting for internal mailbox publication (by backup thread)
    bal = AttendBAL (EMI.util.BackupBalNam);
    if (bal <= 0)
        ExitBad();

   /**********************************************************/
   /* l'emission de fichier n'est possible que si le service */
   /* FICHIER du module LAN est active                    */
   /**********************************************************/
   EMI.flag_emission         = FALSE;
   EMI.flag_restitution      = FALSE;
   EMI.flag_chrono           = FALSE;
   EMI.temoin_arret_emission = FALSE;

   /******************************************/
   /* initialiser tous les services inactifs */
   /******************************************/
   for( id_serv=PREMIER_EMI_SERVICE; id_serv<BUTEE_EMI_SERVICE; id_serv ++)
   {
      EMI.service[id_serv].etat = EMI_LIBRE;
      for(j=0; j<EMI_NB_JETONS; j++)
         EMI.service[id_serv].demandeur[j] = EMI_AUCUN;
   }

   /* etat courant */
   EMI.msg_etat.liaison = EMI_LIAISON_OK;
   EMI.msg_etat.backup  = EMI_INACTIF;

   /* initialiser les numeros de fichiers */
   memset(&EMI.num_fichier, 0L,sizeof(struct_emi_num_fichier));

   /* ancien etat */
   memcpy(&EMI.ancien_etat, &EMI.msg_etat, sizeof(struct_emi_gestion_etat));

   /* ouvrir le service FIC une fois pour toutes */
   EmiEnvoiFic(M_LAN_FICHIER,LAN_DEBUT,NULL);

   ArmeReveilRegulier();
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void EmissionCourante(struct_chrono * p_msg,
*                                             short int bal_source)
* PARAMETRES: message recu de la tache horloge
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Arriv‚e … ech‚ances d'un chrono
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmissionCourante(long num_fichier,struct_emi_utile param)
{
   char path_fichier[MAX_PATH];
   struct _finddata_t  ffblk;
   intptr_t hfind;
   char					newname[MAX_PATH];
   char					path_distant[MAX_PATH];

   if( !EMI.config_received)
   {
      EmiFichierDebug("EMISSION : Demande d'emission de fichier sans config recue\n");
      return;
   }

   /* extension .000 */
   if( RechercherFichier(num_fichier,0,path_fichier) == TRUE )
   {

	   sprintf_s(newname, sizeof(newname), "%s%04d%s%04d%s%08d", "\\MSG.PCS", EMI.plaza_number, ".L", EMI.lane_number, ".S", num_fichier);
	  
	  //path_distant can be without hostname
	  STR_strcpy(MAX_PATH,path_distant, EMI.util.DIR.path_distant);
	  if (path_distant[strlen(path_distant) - 1] == '\\')
		  path_distant[strlen(path_distant) - 1] = '\0';

	  STR_strcat(MAX_PATH, path_distant, newname);
	  if (!EMI.authorized)
	  	  EmiFichierDebug("EmissionCourante => Transfer not authorized");
	  else
	  {
		  if( EmettreFichier(path_fichier, path_distant,param.bal_fic,
		                  param.bal_id) != CSR_OK )
		  {
				EmiFichierTrace("EMISSION : FIC fichier %s KO \n", path_fichier);
		  }
		  else
				EmiFichierTrace("EMISSION : FIC fichier %s OK \n", path_fichier);
	  }
   }
   else
   {
      /* Verification si dernier fichier */
      ObtenirFichier(num_fichier,path_fichier);

      EmiFichierTrace("EMISSION : Existance du fichier %s \n", path_fichier);

      /* le fichier existe t'il */
	  hfind = FIC_FindFirst (path_fichier, &ffblk);
      if (hfind >= 0)
      {
         /* C'est le dernier fichier, il faut attendre */
		FIC_FindClose (hfind);
      }
      else
      {
         /* Le fichier a deja ete transmis, on passe au suivant */
         EMI.num_fichier.courant ++;
         EmiFichierTrace("EMISSION : Passage au fichier %ld \n", EMI.num_fichier.courant);
      }
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void EmiGestionEtat (void)
* PARAMETRES: aucun
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Verifie les changements d'etats et avertit l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EmiGestionEtat(struct_emi_gestion_etat *p_etat,
                              struct_emi_gestion_etat *p_ancien_etat)
{
   boolean flag_changement = FALSE;
   char    *change;

   if( p_etat->liaison != p_ancien_etat->liaison )
   {
      change = "liaison";
      flag_changement = TRUE;
   }

   if( p_etat->backup != p_ancien_etat->backup )
   {
      change = "backup";
      flag_changement = TRUE;
   }

   if( flag_changement )
   {
      EmiFichierTrace("EMISSION : changement d'etat de '%s'\n", change);
      EmiEnvoiAppli(M_EMI_ETAT,EMI_NOUVEL_ETAT,p_etat);

      /* sauvegarde de l'etat courant */
      memcpy(p_ancien_etat, p_etat, sizeof(struct_emi_gestion_etat));

   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void ReceptionEmiService( struct_emi_message *)
* PARAMETRES: pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages de service recus de l'application
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED void ReceptionEmiService(struct_emi_message *p_message)
{
   
   switch(p_message->entete.service)
   {
      case M_EMI_ARRET:
         if( ! temoin_arret_demande)
         {
            temoin_arret_demande = TRUE;
            /* on essaie d'envoyer le dernier fichier,
             * mais on n'attends pas l'acquittement
             */
            EmissionCourante(EMI.num_fichier.courant,EMI.util);
         }
        ReceptionEmiArret(p_message);
      break;

      case M_EMI_ETAT:
 		  if( ! temoin_arret_demande)
              ReceptionEmiEtat(p_message);
      break;

      case M_EMI_RESTITUTION :
           if( ! temoin_arret_demande)
		   {
				EmiFichierTrace("EMISSION : Demande de restitution\n");
              ReceptionEmiRestitution(p_message);
		   }
      break;

      case M_EMI_PURGE :
           if( ! temoin_arret_demande)
              ReceptionEmiPurge(p_message);
      break;

	  case M_EMI_CONFIG:
			ReceptionEmiConfig(p_message);
	  break;

      default:
          ERR_ErreurFatale();
      break;
   }
 
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void ReceptionHrd( struct_hrd_message *)
* PARAMETRES: pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages de service recus de l'application
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED void ReceptionHrd(struct_hrd_message *p_message)
{
   if( p_message->entete.service != M_HRD_GESTION_ALARME)
   {
     /* on n'attends que ces messages-la */
     return;
   }

   switch(p_message->entete.type_message)
   {
      case HRD_ALARME:
           /* le r‚veil vient de sonner */
           EMI.flag_chrono = TRUE;
           EmiFichierTrace("EMISSION : reveil de HRD\n");
      break;

      case HRD_ARME_REGUL_NACQ:
           /* HRD a refuse le r‚veil */
           ExitBad();
      break;

      case HRD_ARME_REGUL_ACQ:
           /* tout va bien */
          EmiFichierTrace("EMISSION : acquittement de HRD\n");
      break;

      default:
         EmiFichierTrace("EMISSION : service recu inconnu %d\n",p_message->entete.service);
      break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PARAMETRES: pointeur sur le message recu.
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement des messages de service recus de l'application
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE void ArmeReveilRegulier( void)
{
   SYSTEMTIME                datetime;
   unsigned long              sec_depuis_0h;
   unsigned long              nb_periodes;
   struct_hrd_date_entree_hex date_tmp;
   struct_hrd_heure_hex       temps_tmp;
   struct_hrd_date_entree_hex date;
   struct_hrd_heure_hex       temps;
   enum_hrd_reveil            retour;

   GetLocalTime(&datetime);

   /* on d‚termine la date de debut du chrono en standard et en seconde
    * a partir de 0 heure
    * on calcule le nombre de 'periode_fermeture' qui se sont ‚coul‚s
    * et on ajoute un pour avoir la prochaine
    */
   /* on ajoute une minute pour la securite */
   sec_depuis_0h = datetime.wHour*60L*60L + datetime.wMinute*60L + 60;
   nb_periodes = sec_depuis_0h / ( EMI.util.duree_sommeil);
   nb_periodes ++;
   sec_depuis_0h = nb_periodes * ( EMI.util.duree_sommeil);
   /* on determine la date relative … la voie */
   sec_depuis_0h += EMI.util.heure_debut_reveil;

   /* on ajoute cela … la date … 00:00:00 */
   date_tmp.annee    = (unsigned short)datetime.wYear;
   date_tmp.mois     = (unsigned char)datetime.wMonth;
   date_tmp.jour     = (unsigned char)datetime.wDay;
   temps_tmp.heure   = 0;
   temps_tmp.minute  = 0;
   temps_tmp.seconde = 0;

   HRDAjouteDateHeure( date_tmp,
                       temps_tmp,
                       sec_depuis_0h,
                       &date,
                       &temps);

   EmiFichierTrace( "Date de d‚but du reveil = '%2d/%2d/%4d %2d:%2d:%2d'\n",
                    date.jour,
                    date.mois,
                    date.annee,
                    temps.heure,
                    temps.minute,
                    temps.seconde);
   EmiFichierTrace( "Duree sommeil = '%d'\n", EMI.util.duree_sommeil);
   retour = HRDArmeReveilRegulier( date, temps, EMI.util.duree_sommeil,
                                   EMI.util.bal_id, &EMI.util.reveil);
   if( retour != HRD_REVEIL_OK)
   {
      ExitBad();
   }
}