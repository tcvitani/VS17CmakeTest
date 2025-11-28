/*------   (v) 1998 CS-Route   -----------    Droits reserves   ------*/

/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN
* FICHIER: LANENVOI.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Code de la fonction EnvoiLan pour les applications
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/lan_envoi.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:49:54   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Oct 14 1998 14:18:46   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:46:06   bph
 *  
 * 
 *    Rev 1.9   09 Dec 1997 11:25:28   BPH
 * Modification de la what string
 * 
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include "str.h"


#include "noyau.h"
#include "csr_lan.h"
#include "lan_glob.h"

#include "memclass.h"


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void EnvoiLan(short int bal_dest,
*                        short int bal_source,
*                        struct_lan_message_externe *p_msg)
* PARAMETRES:
*     entree: BAL destinataire
*           : BAL source
*           : pointeur sur le message a transmettre
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Fonction qui transmet un message au module LAN/IP
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour EnvoiLan(noyau_bal_id bal_dest, noyau_bal_id bal_source
      , noyau_pool_id pool, struct_lan_message_externe *p_msg)
{
   noyau_taille_bloc taille_alloue;
   noyau_taille_bloc taille_message;
   boolean parametre = FALSE;
   struct_lan_message * p_msg_lan;
   
  
   /****** ATTENTION **************/
   /* les structures "struct_lan_message" et "struct_lan_message_externe" */
   /* ne sont pas identiques !!!!!!!                                      */
   
   /* la taille minimum allou‚e pour le message est celle de l'entete */
   /* qui correspond a une demande de service n'ayant pas de paramŠtres */
   taille_alloue = sizeof(struct_lan_entete);
   
   
   
   /* en fonction du type de service, on ajuste la taille du message */
   switch (p_msg->entete.service)
   {
      case M_LAN_CONNEXION:
         switch (p_msg->entete.type_message)
         {
            case LAN_DEBUT:   /* periode de reconnexion */
               taille_alloue += sizeof(struct_lan_gestion_connexion);
               parametre = TRUE;
               ExitAlloue((struct_neutre * *)(&p_msg_lan), taille_alloue, pool);
               p_msg_lan->entete.service = M_LAN_CONNEXION;
               p_msg_lan->entete.type_message = LAN_DEBUT;
               p_msg_lan->u.msg_connexion.periode_reconnexion = p_msg->u.
                     msg_connexion.periode_reconnexion;
               break;
            
            case LAN_FIN:   /* pas de paramŠtres suppl‚mentaires */
            
            default:
               break;
         }
         break;
      
      case M_LAN_MESSAGE:
         switch (p_msg->entete.type_message)
         {
            case LAN_EMISSION:
               taille_message = sizeof(int) /* pour le champ longueur */ +p_msg
                     ->u.message.longueur + sizeof(int);   
               /* pour le champ bal_serv */
               taille_alloue += taille_message;
               parametre = TRUE;
               ExitAlloue((struct_neutre * *)(&p_msg_lan), taille_alloue, pool);
               p_msg_lan->entete.service = M_LAN_MESSAGE;
               p_msg_lan->entete.type_message = LAN_EMISSION;
               p_msg_lan->u.message.longueur = p_msg->u.message.longueur;
               memcpy(&p_msg_lan->u.message.contenu, p_msg->u.message.contenu, 
                     p_msg->u.message.longueur);
               break;
            
            case LAN_DEBUT:   /* pas de paramŠtres suppl‚mentaires */
            
            case LAN_FIN:
            default:
               break;
         }
         break;
      
      case M_LAN_FICHIER:
         switch (p_msg->entete.type_message)
         {
            case LAN_EMISSION:
            case LAN_RECEPTION:
            case LAN_EMISSION_RENOMME:
            case LAN_RECEPTION_RENOMME:
               taille_alloue += sizeof(struct_lan_gestion_fic);
               parametre = TRUE;
               ExitAlloue((struct_neutre * *)(&p_msg_lan), taille_alloue, pool);
               p_msg_lan->entete.service = M_LAN_FICHIER;
               p_msg_lan->entete.type_message = p_msg->entete.type_message;
               
               /* recopie du message */
               STR_strcpy(_MAX_PATH, p_msg_lan->u.param_fic.fichier, p_msg->u.
                     param_fic.fichier);
               STR_strcpy(_MAX_PATH, p_msg_lan->u.param_fic.path, p_msg->u.
                     param_fic.path);
               STR_strcpy(_MAX_PATH, p_msg_lan->u.param_fic.hostname, p_msg->u.
                     param_fic.hostname);
               break;
            
            case LAN_DEBUT:   /* pas de paramŠtres suppl‚mentaires */
            
            case LAN_FIN:
            default:
               break;
         }
         
         break;
      
      case M_LAN_HORAIRE:
         switch (p_msg->entete.type_message)
         {
            case LAN_DEBUT:
            case LAN_DEMANDE:
               taille_alloue += sizeof(struct_lan_gestion_tps);
               parametre = TRUE;
               ExitAlloue((struct_neutre * *)(&p_msg_lan), taille_alloue, pool);
               p_msg_lan->entete.service = M_LAN_HORAIRE;
               p_msg_lan->entete.type_message = p_msg->entete.type_message;
               
               /* recopie du message */
               STR_strcpy(_MAX_PATH, p_msg_lan->u.msg_tps.hostname, p_msg->u.
                     msg_tps.hostname);
               break;
            
            case LAN_FIN:   /* pas de paramŠtres suppl‚mentaires */
            
            default:
               break;
         }
         break;
      
      case M_LAN_DOP:
         switch (p_msg->entete.type_message)
         {
            case LAN_DEBUT:   /* sens et nature des messages */
            
            case LAN_DEMANDE:
               taille_alloue += sizeof(enum_lan_msg_dop)+sizeof(enum_lan_msg_dop
                     );
               parametre = TRUE;
               ExitAlloue((struct_neutre * *)(&p_msg_lan), taille_alloue, pool);
               p_msg_lan->entete.service = M_LAN_DOP;
               p_msg_lan->entete.type_message = p_msg->entete.type_message;
               p_msg_lan->u.msg_dop.nature = p_msg->u.msg_dop.nature;
               p_msg_lan->u.msg_dop.sens = p_msg->u.msg_dop.sens;
               break;
            
            case LAN_FIN:
               taille_alloue += sizeof(enum_lan_msg_dop);
               parametre = TRUE;
               ExitAlloue((struct_neutre * *)(&p_msg_lan), taille_alloue, pool);
               p_msg_lan->entete.service = M_LAN_DOP;
               p_msg_lan->entete.type_message = LAN_FIN;
               p_msg_lan->u.msg_dop.nature = p_msg->u.msg_dop.nature;
               break;
            
            default:
               break;
         }
         break;
      
      case M_LAN_ETAT:   /* pas de paramŠtres suppl‚mentaires */
      
      case M_LAN_ARRET:
      default:
         break;
   }
   
   if (parametre == FALSE)
   {
      parametre = TRUE;
      ExitAlloue((struct_neutre * *)(&p_msg_lan), taille_alloue, pool);
      p_msg_lan->entete.service = p_msg->entete.service;
      p_msg_lan->entete.type_message = p_msg->entete.type_message;
   }
   
   ExitEnvoie(bal_dest, bal_source, (struct_neutre *)(p_msg_lan));
   
   return NOYAU_OK;
}
