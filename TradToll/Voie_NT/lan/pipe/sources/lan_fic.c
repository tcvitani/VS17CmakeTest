/*------   (v) 1998 CS-Route   -----------    Droits reserves   ------*/

/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN
* FICHIER: LAN_FIC.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: tache FIC
* --------------------------------------------------------------------
* DESCRIPTION:
*
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/lan_fic.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:49:56   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jan 20 1999 11:35:56   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:45:54   bph
 *  
 * 
 *    Rev 1.17   05 Feb 1998 10:38:18   BPH
 * Ajout de traces
 * 
*
* --------------------------------------------------------------------
* $F_HEAD
*/


/*--------------- INCLUDES: ---------------*/
#include <stdio.h>

#include <str.h>

/* module NOYAU */
#include <noyau.h>

#include <csr_lan.h>
#include <lan_glob.h>

#include <lan_mess.h>

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/
PRIVATE void CompteRenduFic(fic_enum_retour result, 
							enum_lan_msg_interne msg_interne, 
							struct_interne_fic * p_msg_fic, 
							short int ident);

PRIVATE fic_enum_retour FIC_Copier_Vers(short int ident, 
										char * machine, 
										char * source, 
										char * dest, 
										BOOL flag_renommer);

PRIVATE fic_enum_retour FIC_Copier_Depuis(short int ident, 
										char * machine, 
										char * source, 
										char * dest, 
										size_t iDestSize,
										BOOL flag_renommer);

PRIVATE char * PathFindFileName(IN char * Path);


/*--------------- VARIABLES: --------------*/
/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned int Fic (void)
* PARAMETRES: aucun
* RETOUR:     code
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme principal de la tache temps reel FIC
* ROLE: Recuperer les messages qui arrivent dans la boite aux lettres
*       De la tache.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED DWORD WINAPI LanFic(LPVOID param)
{
   struct_interne_fic * p_msg_fic;
   noyau_enum_retour code_rtc;
   noyau_bal_id fic_bal;
   short int ident;
   fic_enum_retour result;
   
   /* initialisation de la tache et changement de priorite */
   DebutRegion();
   
   ident = (short)param;
   
   
   /* numero d'identificateur inconnu */
   if (ident >= LAN_NB_IDENT_MAX)
   {
      ExitBad();
   }
   
   // publication de la BAL FIC
   fic_bal = LAN[ident].fic_bal = PublieBAL(LAN[ident].nom_fic_bal, 100);
   
   if (LAN[ident].fic_bal <= 0)
      ExitBad();
   
   ChangePriorite(TacheCourante(), LAN[ident].lan_priorite_max);
   
   FinRegion();
   
   // attente des autres bals
   LanAttendBAL(ident);
   
   
   /* attente et traitement des messages issus de LAN_ANI */
   while (TRUE)
   {
      code_rtc = Recoit(fic_bal, (struct_neutre * *)(&p_msg_fic), 
            NOYAU_ATTENTE_INFINIE);
      
      _DebutRegion();
      
      if (code_rtc == NOYAU_BAL_MESS)
      {
         switch ((enum_lan_msg_interne)p_msg_fic->entete.type_message)
         {
            case LAN_ENVOI_FIC:
               
               /* Transfert par FIC avec ou sans decoupage */
               result = FIC_Copier_Vers(ident, p_msg_fic->param.hostname, 
                     p_msg_fic->param.fichier, p_msg_fic->param.path, FALSE);
               
               /* DOP */
               LanEnvoiDop(ident, (char *)&p_msg_fic->param, sizeof(p_msg_fic->
                     param), SRV_ESPION_SORTANT, LAN_DOP_FIC);
               
               /* compte rendu du transfert */
               CompteRenduFic(result, p_msg_fic->entete.type_message, p_msg_fic, 
                     ident);
               break;
            
            case LAN_ENVOI_RENOMME_FIC:
               
               /* Transfert par FIC avec ou sans decoupage */
               result = FIC_Copier_Vers(ident, p_msg_fic->param.hostname, 
                     p_msg_fic->param.fichier, p_msg_fic->param.path, TRUE);
               
               /* DOP */
               LanEnvoiDop(ident, (char *)&p_msg_fic->param, sizeof(p_msg_fic->
                     param), SRV_ESPION_SORTANT, LAN_DOP_FIC);
               
               /* compte rendu du transfert */
               CompteRenduFic(result, p_msg_fic->entete.type_message, p_msg_fic, 
                     ident);
               break;
            
            
            case LAN_RECEPTION_FIC:
               result = FIC_Copier_Depuis(ident, 
										p_msg_fic->param.hostname, 
										p_msg_fic->param.fichier, 
										p_msg_fic->param.path,
										sizeof(p_msg_fic->param.path),
										FALSE);
               
               /* DOP */
               LanEnvoiDop(ident, (char *)&p_msg_fic->param, sizeof(p_msg_fic->
                     param), SRV_ESPION_ENTRANT, LAN_DOP_FIC);
               
               CompteRenduFic(result, p_msg_fic->entete.type_message, p_msg_fic, 
                     ident);
               break;
            
            case LAN_RECEPTION_RENOMME_FIC:
               
               result = FIC_Copier_Depuis(ident, 
										p_msg_fic->param.hostname, 
										p_msg_fic->param.fichier, 
										p_msg_fic->param.path,
										sizeof(p_msg_fic->param.path),
										TRUE);
               
               /* DOP */
               LanEnvoiDop(ident, (char *)&p_msg_fic->param, sizeof(p_msg_fic->
                     param), SRV_ESPION_ENTRANT, LAN_DOP_FIC);
               
               CompteRenduFic(result, p_msg_fic->entete.type_message, p_msg_fic, 
                     ident);
               break;
            
            default:
               /* reception d un message errone : message_id inconnu ... */
               LanFichierDebug(ident, "FIC : reception type inconnu %d", 
                     p_msg_fic->entete.type_message);
               break;
         }
         ExitLibere((struct_neutre * *)(&p_msg_fic));
      }
      
      if (LAN[ident].temoin_arret)
      {
         LanFichierTrace(ident, "FIC : reception demande ARRET");
         
         /* acquittement vers la tache principale du module */
         LanEnvoiAcquittement(ident, p_msg_fic->entete.neutre.bl_retour, 
               M_LAN_ARRET, LAN_DEMANDE_ACQ);
         
         _FinRegion();
         
         /* fin de la tache dans le prochain FinRegion();*/
         Termine();
      }
      
      _FinRegion();
   }
   
   return 0;
}




/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void CompteRenduFic(short int result,enum_lan_msg_interne msg_interne,
*                               short int ident)
* PARAMETRES:
*     entree: rien
*     retour: numero du socket
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Envoi d'un message d'acquittement vers l'application
*       en fonction de l'etat du transfert FIC.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void CompteRenduFic(fic_enum_retour result, enum_lan_msg_interne 
      msg_interne, struct_interne_fic *p_msg_fic, short int ident)
{
   enum_lan_type msg_acq;
   enum_lan_type msg_nacq;
   
   switch (msg_interne)
   {
      case LAN_ENVOI_FIC:
         msg_acq = LAN_EMISSION_EFFECTUEE;
         msg_nacq = LAN_EMISSION_NON_EFFECTUEE;
         LanFichierTrace(ident, " FIC : emission sans renommage");
         break;
      
      case LAN_ENVOI_RENOMME_FIC:
         msg_acq = LAN_EMISSION_RENOMME_EFFECTUEE;
         msg_nacq = LAN_EMISSION_RENOMME_NON_EFFECTUEE;
         LanFichierTrace(ident, " FIC : emission avec renommage");
         break;
      
      case LAN_RECEPTION_FIC:
         msg_acq = LAN_RECEPTION_EFFECTUEE;
         msg_nacq = LAN_RECEPTION_NON_EFFECTUEE;
         LanFichierTrace(ident, " FIC : reception sans renommage");
         break;
      
      case LAN_RECEPTION_RENOMME_FIC:
         msg_acq = LAN_RECEPTION_RENOMME_EFFECTUEE;
         msg_nacq = LAN_RECEPTION_RENOMME_NON_EFFECTUEE;
         LanFichierTrace(ident, " FIC : reception avec renommage");
         break;
      
      default:
         break;
   }
   
   if (result == FIC_OK)
   {
      /* message d'etat pour liaison OK */
      LAN[ident].msg_etat.serveur_fic = LAN_TRANSFERT_FICHIER_OK;
      LanEnvoiAppli(ident, p_msg_fic->param.bal_serv, M_LAN_FICHIER, msg_acq, &
            p_msg_fic->param);
      LanFichierTrace(ident, " FIC : fichier '%s' transf‚r‚ ", p_msg_fic->param.
            fichier);
   
   }
   else
   {
      LanEnvoiAppli(ident, p_msg_fic->param.bal_serv, M_LAN_FICHIER, msg_nacq, &
            p_msg_fic->param);
      LanFichierTrace(ident, " FIC : fichier '%s' non transf‚r‚ ", p_msg_fic->
            param.fichier);
      
      switch (result)
      {
         case FIC_ERR_TRANSFERT_PUT:
         case FIC_ERR_TRANSFERT_GET:
            /* message d'etat pour Pb transfert FIC */
            LAN[ident].msg_etat.serveur_fic = LAN_TRANSFERT_FICHIER_NOK;
            LanFichierTrace(ident, "FIC => Erreur transmission FIC ");
            break;
         
         default:
            break;
      }
   }
}


/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: fic_enum_retour FIC_Copier_Vers( char *machine,
*                                           char *userid,
*                                           char *passwd,
*                                           char *fichier_source,
*                                           char *path_distant,
*                                           boolean flag_renommer)
* PARAMETRES: machine : hostname distant
*             userid  : identifiant d'utilisateur sur le serveur fic
*             passwd  : password d'utilisateur sur le serveur fic
*             fichier_source: nom du fichier a envoyer (AVEC path)
*             path_distant  : nom du path o— doit ˆtre copi‚ le fichier
*                             sur la machine distante
*             flag_renommer : … TRUE si le fichier doit changer de nom,
*                             Dans ce cas, "path_distant" contient le
*                             nouveau nom .
* RETOUR: valeur d'erreur
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Copie un fichier par fic sur une machine distante.
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE fic_enum_retour FIC_Copier_Vers(short int ident, char *machine, char *
      source, char *dest, BOOL flag_renommer)
{
   char path_dest[MAX_PATH];
   
   
   /* si pas de serveur specifie, serveur par defaut du registre */
   if (machine == NULL || machine[0] == '\0')
	   strncpy_s(path_dest, sizeof(path_dest), LAN[ident].param.nom_distant[LAN[ident].param.num_fic],
            sizeof(path_dest));
   else
	   strncpy_s(path_dest, sizeof(path_dest), machine, sizeof(path_dest));
   
   // ajouter la destination
   strncat_s(path_dest, sizeof(path_dest), "\\", sizeof(path_dest));
   strncat_s(path_dest, sizeof(path_dest), dest, sizeof(path_dest));
   
   
   // si pas de renommage, recopier la source
   if (flag_renommer == FALSE)
   {
	   strncat_s(path_dest, sizeof(path_dest), "\\", sizeof(path_dest));
	   strncat_s(path_dest, sizeof(path_dest), PathFindFileName(source), sizeof(path_dest));
   }
   
   
   // rem : copyfile se fout des slash en trop
   _FinRegion();
   
   if (CopyFile(source, path_dest, FALSE) == FALSE)
   {
	   _DebutRegion();
	   return FIC_ERR_TRANSFERT_PUT;
   }

   _DebutRegion();
   
   return FIC_OK;
}


PRIVATE fic_enum_retour FIC_Copier_Depuis(short int ident, 
										char *machine, 
										char * source, 
										char *dest,
										size_t iDestSize,
										BOOL flag_renommer)
{
	char path_dest[MAX_PATH] = {0};
   
   
   /* si pas de serveur specifie, serveur par defaut du registre */
   if (machine == NULL || machine[0] == '\0')
	   strncpy_s(path_dest, sizeof(path_dest), LAN[ident].param.nom_distant[LAN[ident].param.num_fic],
            sizeof(path_dest));
   else
	   strncpy_s(path_dest, sizeof(path_dest), machine, sizeof(path_dest));
   
   // ajouter la source    
   strncat_s(path_dest, sizeof(path_dest), "\\", sizeof(path_dest));
   strncat_s(path_dest, sizeof(path_dest), source, sizeof(path_dest));
   
   // si pas de renommage, recopier la source
   if (flag_renommer == FALSE)
   {
	   strncat_s(dest, iDestSize, "\\", sizeof(path_dest));
	   strncat_s(dest, iDestSize, PathFindFileName(source), sizeof(path_dest));
   }
   
   
   _FinRegion();
   
   // rem : copyfile se fout des slash en trop
   if (CopyFile(path_dest, dest, FALSE) == FALSE)
   {
	   _DebutRegion();
	   return FIC_ERR_TRANSFERT_GET;
   }
   
   _DebutRegion();

   return FIC_OK;
}


// Extrait le nom du fichier (s'il existe) d'un chemin LFN ou UNC
// 
// quelques cas :
//
//  1 c:\dir\foo.ext    -> foo.txt
//  2 c:foo.txt         -> foo.txt
//  3 c:\foo.txt\       -> FALSE
//  4 c:\               -> FALSE
//  5 c:                -> FALSE 
//  6 c:.               -> FALSE 
//  7 c:..              -> FALSE 
//  8 \foo.txt          -> foo.txt
//  9 foo.txt           -> foo.txt
// 10 ..\foo.txt        -> foo.txt
// 11 .\foo.txt         -> foo.txt
// 12 .                 -> FALSE
// 13 ..                -> FALSE
// 14 \.                -> FALSE
// 15 \..               -> FALSE
// 16 \\x\dir\foo.txt   -> foo.txt
// 17 \\x               -> FALSE    (non traité !)
// 
PRIVATE char *PathFindFileName(IN char *Path)
{
   int i, j;
   
   
   // on parcours jusqu'a trouver un '\', un ':' ou la fin de chaine
   for (i = j = 0; Path[i] != '\0'; i++)
      if (Path[i] == '\\' || Path[i] == ':')
         j = i;
   
   
   // erreur  si j == '\0' ou si j == '.' ou  (si  j == ('\\' ou ':') et j+1 == ('\0' ou '.'))
   if (Path[j] == '\0' || Path[j] == '.' || ((Path[j] == '\\' || Path[j] == ':') 
            && (Path[j + 1] == '\0' || Path[j + 1] == '.')))
      return Path;
   
   // c'est OK, le nom du fichier commence au caractere suivant
   return &Path[j + 1];
}
