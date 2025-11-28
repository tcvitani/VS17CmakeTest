/*------   (v) 1998 CS-Route   -----------    Droits reserves   ------*/

/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN
* FICHIER: LAN_INIT.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Code des fonctions utiles pour l'application
* --------------------------------------------------------------------
* DESCRIPTION: Fonctions LanRessource(), LanLance() et LanArret()
*              qui permettent a une application utilisatrice du module
*              de l'initialiser, de le lancer et de l'arreter.
*              Fonctions LanDebutDebug() et LanFinDebug() pour ecrire
*              dans un fichier les erreurs survenues.
*              Fonctions LanDebutTrace() et LanFinTrace() pour ecrire
*              dans un fichier les traces utiles lors de l'integration
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/lan_init.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:49:58   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.5   09 Apr 1999 16:28:00   afx
 *  
 * 
 *    Rev 1.4   Jan 20 1999 11:35:58   bph
 *  
 * 
 *    Rev 1.3   Dec 21 1998 13:34:50   AFX
 *  
 * 
 *    Rev 1.2   Oct 14 1998 14:18:40   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:45:56   bph
 *  
 * 
 *    Rev 1.24   09 Dec 1997 15:08:28   BPH
 * 
*
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

/* module NOYAU */
#include "noyau.h"
#include "reg.h"

#include "csr_lan.h"

#define LOC_DEF
#   include "lan_glob.h"
#undef LOC_DEF

#include "lan_mess.h"

/* gestion de fichier */
//#include "fic_conf.h"
//#include "fic_gere.h"
//#include "err.h"

#include <run.h>

/*--------------- RESERVED: ---------------*/
#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/
#define MAXCHAINE 80

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/
PRIVATE enum_lan_retour LanTrouveConfigDistant(short int ident, char * pcKey, 
      struct_lan_parametre * );
PRIVATE enum_lan_retour LanTrouvePeriode(short int ident, char * pcKey);

PRIVATE void LanceRegion(void);

PRIVATE void ArretRegion(void);

/*--------------- VARIABLES: --------------*/



/*--------------- CODE: -------------------*/
/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI MODLance ( char * pcKey, char * pcBalName, noyau_bal_id * piBalId )
 * PARAMETERS: IN char * pcKey           : Nom de la clé du registre où aller chercher les paramètres
 *                                         de lancement de l'instance.
 *             IN char * pcBalName       : Nom de la boite à lettre à creer pour l'instance
 *             OUT noyau_bal_id * piBalId: Pointe sur une variable recevant l'id de la boite à lettre
 *                                         créée pour l'instance (et servant d'identifiant d'instance.
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour démarrage
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI MODLance(char *pcKey, char *pcBalNam, 
      noyau_bal_id *piBalId)
{
   return LanLance(pcKey, pcBalNam, piBalId);
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT enum_instance_result WINAPI MODArret ( IN noyau_bal_id iBalId )
 * PARAMETERS: IN noyau_bal_id iBalId : Id de la bal identifiant l'instance
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour arret
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI MODArret(noyau_bal_id iBalId)
{
   return LanArret(iBalId);
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI LanLance ( char * pcKey, char * pcBalName, noyau_bal_id * piBalId )
 * PARAMETERS: IN char * pcKey           : Nom de la clé du registre où aller chercher les paramètres
 *                                         de lancement de l'instance.
 *             IN char * pcBalName       : Nom de la boite à lettre à creer pour l'instance
 *             OUT noyau_bal_id * piBalId: Pointe sur une variable recevant l'id de la boite à lettre
 *                                         créée pour l'instance (et servant d'identifiant d'instance.
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour démarrage
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI LanLance(char *pcKey, char *pcBalNam, 
      noyau_bal_id *piBalId)
{
   enum_lan_retour retour;
   short int ident;
   char NomTache[MAX_PATH + 1];
   char pcPoolId[MAX_PATH + 1];   // REG : Chaine de config du pool
   DWORD dwPrioMax;   // REG : Priorité max taches
   DWORD dwPrioInitMax;   // REG : Priorité max taches initiale
   DWORD dwLen;
   
   DebutRegion();   // pour protéger LAN_NB_INSTANCES
   
   if (LAN_NB_INSTANCES >= LAN_NB_IDENT_MAX)
      return INST_INIT_ERR_MAX_INSTANCE;
   
   ident = LAN_NB_INSTANCES++;
   
   LanceRegion();
   
   FinRegion();
   
   
   
   /* initialisation du mode trace */
   if (LanInitTrace(pcBalNam, ident) != INST_INIT_OK)
      return INST_INIT_ERR_FICHIER_DEBUG;
   
   
   
   // Vérification de la taille du nom de la BAL ANI
   if (strlen(pcBalNam) > MAX_PATH)
   {
      LanFichierDebug(ident, 
            "LAN_INIT ***** LanLance() => Nom de BAL trop long : %s", pcBalNam);
      return INST_INIT_ERR_LANCE;
   }
   
   // Lecture des entrées de configuration dans le registre
   
   // Pool
   dwLen = sizeof(pcPoolId);
   
   if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_POOL, pcPoolId, &
         dwLen) != ERROR_SUCCESS)
   {
      LanFichierDebug(ident, 
            "LAN_INIT ***** LanLance() => Nom de BAL trop long : %s", pcBalNam);
      return INST_INIT_ERR_REGISTRE;
   }
   
   
   
   // Priorité initiale max
   if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_INIT, &
         dwPrioInitMax) != ERROR_SUCCESS)
   {
      LanFichierDebug(ident, 
            "LAN_INIT ***** LanLance() => Nom de BAL trop long : %s", pcBalNam);
      return INST_INIT_ERR_REGISTRE;
   }
   dwPrioInitMax = NOYAU_MapPriority(dwPrioInitMax);
   
   
   
   // Priorité max
   if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_PRIO_MAX, &
         dwPrioMax) != ERROR_SUCCESS)
   {
      LanFichierDebug(ident, 
            "LAN_INIT ***** LanLance() => Nom de BAL trop long : %s", pcBalNam);
      return INST_INIT_ERR_REGISTRE;
   }
   dwPrioMax = NOYAU_MapPriority(dwPrioMax);


   //Sleep time
   if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_MAX_SLEEP_TIME_MS,
	   &LAN[ident].dwSleepTimeMs) != ERROR_SUCCESS)
   {
	   LanFichierDebug(ident,
		   "LAN_INIT ***** LANLance() => Erreur registre : [%s]:%s, set to default of 10ms!", pcKey,
		   MOD_REG_KEYv_MAX_SLEEP_TIME_MS);

	   LAN[ident].dwSleepTimeMs = 10;
   }
   
   /* parametres d'initialisation des taches */
   sprintf_s(NomTache, sizeof(NomTache), "LAN IP (%s)", pcBalNam);
   
   NOYAU_INIT_TACHE(LAN[ident].taches[0], TRUE, dwPrioInitMax, 2048, (
         LPTHREAD_START_ROUTINE)(Lan_ani), (PVOID)(ident), NULL, NomTache);
   
   sprintf_s(NomTache, sizeof(NomTache), "LAN EMISSION (%s)", pcBalNam);
   
   NOYAU_INIT_TACHE(LAN[ident].taches[1], TRUE, dwPrioInitMax, 2048, (
         LPTHREAD_START_ROUTINE)(LanEmission), (PVOID)(ident), NULL, NomTache);
   
   sprintf_s(NomTache, sizeof(NomTache), "LAN RECEPTION (%s)", pcBalNam);
   
   NOYAU_INIT_TACHE(LAN[ident].taches[2], TRUE, dwPrioInitMax, 2048, (
         LPTHREAD_START_ROUTINE)(LanReception), (PVOID)(ident), NULL, NomTache);
   
   sprintf_s(NomTache, sizeof(NomTache), "LAN FIC (%s)", pcBalNam);
   
   NOYAU_INIT_TACHE(LAN[ident].taches[3], TRUE, dwPrioInitMax, 3072, (
         LPTHREAD_START_ROUTINE)(LanFic), (PVOID)(ident), NULL, NomTache);
   
   sprintf_s(NomTache, sizeof(NomTache), "LAN TIME (%s)", pcBalNam);
   
   // Plus d'autre tache
   NOYAU_VIDE_TACHE(LAN[ident].taches[4]);
   
   /* mise a jour des priorites des taches a l'init */
   
   // Priorite max du module LAN 
   LAN[ident].lan_priorite_max = dwPrioMax;
   
   // Numero de pool
   LAN[ident].lan_pool = NOYAU_GetPoolId(pcPoolId);
   
   // Noms de BAL
   strcpy_s(LAN[ident].nom_lan_bal, sizeof(LAN[ident].nom_lan_bal), pcBalNam);
   sprintf_s(LAN[ident].nom_emis_bal, sizeof(LAN[ident].nom_emis_bal), "iBAL_LAN_EMI_%d_%d", ident,
         GetCurrentProcessId());
   sprintf_s(LAN[ident].nom_fic_bal, sizeof(LAN[ident].nom_fic_bal), "iBAL_LAN_FIC_%d_%d", ident,
         GetCurrentProcessId());
   
   /* periode du message de vie et de deconnexion automatique */
   retour = LanTrouvePeriode(ident, pcKey);
   
   if (retour != LAN_CONFIG_PERIODE_OK)
      return INST_INIT_ERR_REGISTRE;
   
   /* lecture des fichiers de configuration du réseau */
   
   /* SERVEUR.LAN */
   retour = LanTrouveConfigDistant(ident, pcKey, &LAN[ident].param);
   
   if (retour != LAN_CONFIG_RESEAU_OK)
      return INST_INIT_ERR_REGISTRE;
   
   
   
   // reservation des chronometres
   if (AlloueChrono(&LAN[ident].chrono_attente_msg, "chrono vie") != NOYAU_OK)
      return INST_INIT_ERR_LANCE;
   
   if (AlloueChrono(&LAN[ident].chrono_reconnexion, "chrono reconnexion") != 
            NOYAU_OK)
      return INST_INIT_ERR_LANCE;
   
   
   
   // réservation des evenements
   if (AlloueEvent(&LAN[ident].param.rEvent, NULL) != NOYAU_OK)
      return INST_INIT_ERR_LANCE;
   
   // Init de la struct overlapped
   memset(&LAN[ident].param.rOver, 0, sizeof(OVERLAPPED));
   LAN[ident].param.rOver.hEvent = LAN[ident].param.rEvent;
   
   if (AlloueEvent(&LAN[ident].param.wEvent, NULL) != NOYAU_OK)
      return INST_INIT_ERR_LANCE;
   
   // Init de la struct overlapped
   memset(&LAN[ident].param.wOver, 0, sizeof(OVERLAPPED));
   LAN[ident].param.wOver.hEvent = LAN[ident].param.wEvent;
   
   
   
   /* initialiser tous les services */
   if ((LAN[ident].service[M_LAN_ARRET] = SrvLance(1)) == NULL)
      ExitBad();
   
   if ((LAN[ident].service[M_LAN_ETAT] = SrvLance(SRV_ILLIMITE)) == NULL)
      ExitBad();
   
   if ((LAN[ident].service[M_LAN_DOP] = SrvLance(SRV_ILLIMITE)) == NULL)
      ExitBad();
   
   if ((LAN[ident].service[M_LAN_CONNEXION] = SrvLance(1)) == NULL)
      ExitBad();
   
   if ((LAN[ident].service[M_LAN_MESSAGE] = SrvLance(SRV_ILLIMITE)) == NULL)
      ExitBad();
   
   if ((LAN[ident].service[M_LAN_FICHIER] = SrvLance(SRV_ILLIMITE)) == NULL)
      ExitBad();
   
   if ((LAN[ident].service[M_LAN_HORAIRE] = SrvLance(SRV_ILLIMITE)) == NULL)
      ExitBad();
   
   
   
   /* demarrage de l'automate */
   if (ATM_LAN_Lance(&LAN[ident].atm_lan, ident) != AUT_OK)
   {
      LanFichierDebug(ident, "LanLance() => Echec ATM_LAN_Lance()");
      return INST_INIT_ERR_LANCE;
   }
   
   
   
   /* lancement des taches */
   if (LanceTache(LAN[ident].taches) != NOYAU_OK)
   {
      LanFichierDebug(ident, "LanLance() => Echec LanceTache()");
      return INST_INIT_ERR_LANCE;
   }
   
   /* Initialisation des BAL du module */
   *piBalId = AttendBAL(pcBalNam);
   
   if (*piBalId <= 0)
   {
      LanFichierDebug(ident, "LanLance() => Echec AttendBal(%s)", pcBalNam);
      return INST_INIT_ERR_LANCE;
   }
   
   return INST_INIT_OK;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT enum_instance_result WINAPI LanArret ( IN noyau_bal_id iBalId )
 * PARAMETERS: IN noyau_bal_id iBalId : Id de la bal identifiant l'instance
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour arret
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI LanArret(noyau_bal_id iBalId)
{
   short int ident;
   noyau_enum_retour cr_arret;
   enum_lan_retour result = INST_ARRET_OK;
   SYSTEMTIME today;
   
   
   
   /* recherche de l'ident sur son numero de Boite aux Lettres */
   /* obtenu dans la fonction LanLance()                     */
   for (ident = 0; ident < LAN_NB_IDENT_MAX; ident++)
   {
      if (LAN[ident].lan_bal == iBalId)
      {
         /* arret des taches associees au socket trouve */
         cr_arret = ArretTaches(LAN[ident].taches);
         
         
         
         /* si l'arret s'est mal passe */
         if (cr_arret == NOYAU_ARRET_TACHE_NOK)
         {
            LanFichierDebug(ident, "Echec ArretTaches()");
            result = INST_ARRET_NOK;
         }
         else
         {
            
            
            // arret de l'automate 
            if (ATM_LAN_Arret(&LAN[ident].atm_lan) != AUT_OK)
            {
               LanFichierDebug(ident, "LanLance() => Echec ATM_LAN_Arret()");
               result = INST_ARRET_NOK;
            }
            
            // suppresion des BAL dans la registry  
            LanSupprimeBAL(ident);
            
            
            
            // liberer les services
            if (SrvArret(&LAN[ident].service[M_LAN_ARRET]) == FALSE)
               ExitBad();
            
            if (SrvArret(&LAN[ident].service[M_LAN_ETAT]) == FALSE)
               ExitBad();
            
            if (SrvArret(&LAN[ident].service[M_LAN_DOP]) == FALSE)
               ExitBad();
            
            if (SrvArret(&LAN[ident].service[M_LAN_CONNEXION]) == FALSE)
               ExitBad();
            
            if (SrvArret(&LAN[ident].service[M_LAN_MESSAGE]) == FALSE)
               ExitBad();
            
            if (SrvArret(&LAN[ident].service[M_LAN_FICHIER]) == FALSE)
               ExitBad();
            
            if (SrvArret(&LAN[ident].service[M_LAN_HORAIRE]) == FALSE)
               ExitBad();
            
            DebutRegion();   // pour protéger ZIP_NB_INSTANCES
            
            
            
            /* decrementer les compteurs s'ils sont positifs */
            if (LAN_NB_INSTANCES > 0)
               LAN_NB_INSTANCES--;
            
            ArretRegion();
            
            FinRegion();
            
            result = INST_ARRET_OK;
         }
         
         /*** Récupération de la date et de l'heure */
         GetLocalTime(&today);
         
         LanFichierTrace(ident, 
               "LanArret() => Fermeture du fichier de TRACE le %2d/%2d/%4u a %02d-%02d-%02d"
               , today.wDay, today.wMonth, today.wYear, today.wHour, today.
               wMinute, today.wSecond);
         
         
         
         /* Arret du mode DEBUG */
         if (DBG_Arret(&LAN[ident].dbg) != DBG_OK)
            return INST_INIT_ERR_FICHIER_DEBUG;
         
         /* sortie de la boucle for */
         break;
      }
   }
   
   
   /* aucune connexion n'a ete trouve */
   if (ident == LAN_NB_IDENT_MAX)
   {
      LanFichierDebug(ident, 
            "LanArret() => La BAL '%d' ne correspond … aucun socket", iBalId);
      result = INST_ARRET_NOK;
   }
   else
   {
      // Arret des chronometres
      ArretChrono(LAN[ident].chrono_attente_msg);
      ArretChrono(LAN[ident].chrono_reconnexion);
      
      // Liberation des chronometres
      LibereChrono(&LAN[ident].chrono_attente_msg);
      LibereChrono(&LAN[ident].chrono_reconnexion);
      
      LibereEvent(&LAN[ident].param.rEvent);
      LibereEvent(&LAN[ident].param.wEvent);
   }
   
   /* retourner la valeur de l'arret des taches */
   return(result);
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED enum_lan_retour LanTrouveConfigDistant()
* PARAMETRES: aucun
* RETOUR:     TRUE fichier existant et coherant  !
*             FALSE fichier inexistant ou incoherant !
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: met a jour les parametres utiles du reseau
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_retour LanTrouveConfigDistant(short int ident, char *pcKey, 
      struct_lan_parametre *p_param)
{
   DWORD dwLen;
   short int nb_serveur = 0;
   
   // REG : Chaine de config du serveur MSG
   dwLen = sizeof(p_param->nom_distant[nb_serveur]);
   
   if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_MSG_SERVER, 
         p_param->nom_distant[nb_serveur], &dwLen) == ERROR_SUCCESS)
   {
      p_param->num_msg = (char)nb_serveur++;
   }
   else
   {
      LanFichierTrace(ident, 
            "LAN_INIT ***** LANLance() => Erreur registre : [%s]:%s", pcKey, 
            MOD_REG_KEYv_MSG_SERVER);
   }
   
   // REG : Chaine de config du serveur FIC
   dwLen = sizeof(p_param->nom_distant[nb_serveur]);
   
   if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_FILE_SERVER, 
         p_param->nom_distant[nb_serveur], &dwLen) == ERROR_SUCCESS)
   {
      // REG : Chaine du nom d'utilisateur FIC
        /*
        if( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_FILE_USER, p_param->user_fic, &dwLen ) != ERROR_SUCCESS )
        {
            LanFichierDebug(ident,"LAN_INIT ***** LANLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_FILE_USER);
            return INST_INIT_ERR_REGISTRE;
        }
        
        // REG : Chaine du mot de passe FIC
        if( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_FILE_PASSWORD, p_param->password_fic, &dwLen ) != ERROR_SUCCESS )
        {
            LanFichierDebug(ident,"LAN_INIT ***** LANLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_FILE_PASSWORD);
            return INST_INIT_ERR_REGISTRE;
        }
        */
      
      p_param->num_fic = (char)nb_serveur++;
   }
   else
   {
      LanFichierTrace(ident, 
            "LAN_INIT ***** LANLance() => Erreur registre : [%s]:%s", pcKey, 
            MOD_REG_KEYv_FILE_SERVER);
   }
   
   return LAN_CONFIG_RESEAU_OK;
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:PROTECTED enum_lan_retour LanTrouvePeriode(char *nom_fichier,
*                                      struct_lan_priode *p_periode)
* PARAMETRES: fichier de parametre du reseau
*             structure periode:
*                       periode lue en SECONDE, convertie en TICKS
* RETOUR:     TRUE fichier existant et coherant  !
*             FALSE fichier inexistant ou incoherant !
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: met a jour les periodes concernatn le msg de vie
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_retour LanTrouvePeriode(short int ident, char *pcKey)
{
   
   
   // REG : periode en SECONDES du msg de vie
   if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_KEEP_ALIVE, &LAN[
         ident].duree_chrono_attente_msg) != ERROR_SUCCESS)
   {
      LanFichierDebug(ident, 
            "LAN_INIT ***** LANLance() => Erreur registre : [%s]:%s", pcKey, 
            MOD_REG_KEYv_KEEP_ALIVE);
      return INST_INIT_ERR_REGISTRE;
   }
   
   /* conversion en TICKS */
   LAN[ident].duree_chrono_attente_msg *= NOYAU_NB_TICK_PAR_SEC;
   
   if (LAN[ident].duree_chrono_attente_msg <= 0)
      return LAN_CONFIG_ERR_PERIODE;
   
   
   
   // REG : periode en SECONDES avant reconnexion
   if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_CONNECTION_DELAY, 
         &LAN[ident].duree_chrono_reconnexion) != ERROR_SUCCESS)
   {
      LanFichierDebug(ident, 
            "LAN_INIT ***** LANLance() => Erreur registre : [%s]:%s", pcKey, 
            MOD_REG_KEYv_CONNECTION_DELAY);
      return INST_INIT_ERR_REGISTRE;
   }
   
   /* conversion en TICKS */
   LAN[ident].duree_chrono_reconnexion *= NOYAU_NB_TICK_PAR_SEC;
   
   if (LAN[ident].duree_chrono_reconnexion <= 0)
      return LAN_CONFIG_ERR_PERIODE;
   
   
   
   // lecture du compteur d'echec du msg de vie avant deconnexion
   if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, 
         MOD_REG_KEYv_MAX_KEEP_ALIVE_COUNTER, &LAN[ident].max_cpt_timeout_msg) 
         != ERROR_SUCCESS)
   {
      LanFichierDebug(ident, 
            "LAN_INIT ***** LANLance() => Erreur registre : [%s]:%s", pcKey, 
            MOD_REG_KEYv_MAX_KEEP_ALIVE_COUNTER);
      return LAN_CONFIG_ERR_PERIODE;
   }
   
   
   
   // REG : lecture de la tolerance de difference de temps en SECONDES
   if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_MAX_DIFF_TIME, &
         LAN[ident].max_diff_time) != ERROR_SUCCESS)
   {
      LanFichierDebug(ident, 
            "LAN_INIT ***** LANLance() => Erreur registre : [%s]:%s", pcKey, 
            MOD_REG_KEYv_MAX_DIFF_TIME);
      return INST_INIT_ERR_REGISTRE;
   }
   
   return LAN_CONFIG_PERIODE_OK;
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_instance_result LanInitTrace (char * pcBal, short int cpt_ressource)
* PARAMETRES:
*     cpt_ressource : index de l'instance
*     pcBal : nom de la bal d'interface
*     retour: compte-rendu
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: fonction qui initialise le mode DEBUG associé au noyau 6.00 :
*       un fichier spécifique de trace pour chaque instance
*       du module doit etre present dans le répertoire C:\TRACES !!!
*       un fichier spécifique d'erreurs pour chaque instance
*       du module doit etre present dans le répertoire C:\ERREURS !!!
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_instance_result LanInitTrace(char *pcBal, short int cpt_ressource
      )
{
   DWORD dwLen;
   DWORD dwTailleMax;
   char * pcKey = CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE 
         CSR_REG_KEYn_CONFIG;
   dbg_struct_debug * debug;
   dbg_struct_trace * tab_traces;
   
   debug = &LAN[cpt_ressource].dbg;
   tab_traces = &LAN[cpt_ressource].tab_traces[LAN_TRC];
   
   // chemin traces
   dwLen = sizeof(debug->rep_fichiers_traces);
   
   if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_TRACEPATH, debug->
            rep_fichiers_traces, &dwLen) != ERROR_SUCCESS)
      return INST_INIT_ERR_FICHIER_DEBUG;
   
   // chemin erreurs
   dwLen = sizeof(debug->rep_fichier_erreurs);
   
   if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_ERRORPATH, debug->
            rep_fichier_erreurs, &dwLen) != ERROR_SUCCESS)
      return INST_INIT_ERR_FICHIER_DEBUG;
   
   
   
   // Taille max des fichiers
   if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_FILEMAXSIZE, &
            dwTailleMax) != ERROR_SUCCESS)
      return INST_INIT_ERR_FICHIER_DEBUG;
   debug->taille_limite = dwTailleMax;
   
   // pour initialiser les traces à l'écran
   // chaque instance du module LAN aura son propre
   // fichier donné par son nom de bal (unique)
   strcpy_s(debug->nom_fichier_traces_ecran, sizeof(debug->nom_fichier_traces_ecran), pcBal);
   
   // chaque instance du module LAN aura également son propre
   // fichier d'erreurs donné par son nom de bal (unique)
   strcpy_s(debug->nom_fichier_erreurs, sizeof(debug->nom_fichier_erreurs), pcBal);
   
   /* chaque instance du module LAN aura son propre fichier de trace */
   strcpy_s(tab_traces->nom, sizeof(tab_traces->nom), pcBal);
   
   debug->tab_traces = tab_traces;
   debug->nb_fichiers_traces = LAN_NB_TRACES;
   
   if (DBG_Lance(debug) != DBG_OK)
      return INST_INIT_ERR_FICHIER_DEBUG;
   
   return INST_INIT_OK;
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void LanFichierTrace (short int ident_lecteur,char *fmt,...)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui ecrit dans le fichier de TRACES
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void LanFichierTrace(short int ident_lecteur, char *fmt, ...)
{
   va_list args;
   char string[16384];
   
   va_start(args, fmt);
   vsprintf_s(string, sizeof(string), fmt, args);
   
   DBG_EcritFichierTraces(LAN_TRC, &LAN[ident_lecteur].dbg, "%s", string);
   
   va_end(args);

}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void LanFichierDebug (short int ident_lecteur,char *fmt,...)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui ecrit dans le fichier d'erreurs
*       et dans le fichier de traces
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void DEFINE_LanFichierDebug(short int ident_lecteur, char *fmt, ...)
{
   va_list args;
   char string[16384];
   
   va_start(args, fmt);
   vsprintf_s(string, sizeof(string), fmt, args);
   
   DBG_FILE = LAN_FILE;
   DBG_LINE = LAN_LINE;
   
   DEFINE_DBG_EcritFichierErreurs(LAN_TRC, &LAN[ident_lecteur].dbg, "%s", string
         );
   
   va_end(args);
}


PRIVATE void LanceRegion(void)
{
   InitializeCriticalSection(&CS);
}


PRIVATE void ArretRegion(void)
{
   DeleteCriticalSection(&CS);
}


PROTECTED void _DebutRegion(void)
{
   EnterCriticalSection(&CS);
}


PROTECTED void _FinRegion(void)
{
   LeaveCriticalSection(&CS);
}
