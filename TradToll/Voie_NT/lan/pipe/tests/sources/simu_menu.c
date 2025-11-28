/*------   (v) 1997 CS-Route   -----------    Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SIMULATEUR
* FICHIER: SIMU_LAN.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fonctions de test propres au module LAN/IP
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/test/simu_menu.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:50:18   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:18   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jan 20 1999 11:36:20   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:46:14   bph
 *  
 * 
 *    Rev 1.12   03 Nov 1997 14:21:04   BPH
 * Utilisation de ExitAlloue, ExitEnvoie, ExitLibere
 * et du debug facon noyau 6
 * 
 *    Rev 1.11   07 Oct 1997 11:32:08   ANA
 * Test version 4.00 avec nouvelle librairie de 
 * decoupage/recollage des fichiers à transferer
 * 
 *    Rev 1.10   Aug 04 1997 17:34:44   ANA
 * Transfert de fichiers avec renommage
 * 
 *    Rev 1.9   Jul 02 1997 15:22:30   ANA
 * Mise à jour pour PEMM et noyau 5.00
 * Nouvelle gestion du DOP
 * 
 *    Rev 1.8   Apr 05 1997 16:59:28   ANA
 * Modifications pour l'arret des taches
 * 
 *    Rev 1.7   Mar 21 1997 15:09:40   ANA
 *  
 * 
 *    Rev 1.6   Mar 11 1997 16:52:40   ANA
 * Adaptation au fur et a mesure des tests
 * 
 *    Rev 1.5   Mar 07 1997 18:33:36   ANA
 * Evolution du message de vie et Ajout de la tache
 * de gestion du service HORAIRE
 *
 *    Rev 1.4   Mar 03 1997 14:09:48   ANA
 * Integration du module NOYAU gerant la com LAN/IP
 *
 *    Rev 1.3   Feb 26 1997 16:41:06   ANA
 * Configuration reseau par fichier
 *
 *    Rev 1.2   Feb 20 1997 15:57:00   ANA
 * Version 0
 *
*
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
//#include <console.h>
//#include <conio.h>

/* module noyau */
#include <noyau.h>

/* module LAN/IP */
#include <csr_lan.h>

#define LOC_DEF
#include "simu_glo.h"
#undef LOC_DEF

#include "simu.h"

/*--------------- RESERVED: ---------------*/
#include "memclass.h"

/*--------------- EXTERNALS:---------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/

PROTECTED void MenuServices(noyau_bal_id /*bal_ident*/);
PROTECTED short int MenuNatureDop(void);
PROTECTED short int MenuSensDop(void);
PROTECTED void GestionMessage(noyau_bal_id /*bal_ident*/);
PROTECTED void GestionFichier(noyau_bal_id /*bal_ident*/,enum_lan_type);

/*--------------- VARIABLES: --------------*/

PROTECTED char tableau[512];
PROTECTED char emission_possible = 0;
PROTECTED char fichier_possible = 0;
PROTECTED long num_msg = 0L;

PROTECTED struct_lan_gestion_msg msg_test;

/*--------------- CODE: -------------------*/
void gotoxy(int x, int y)
{
	COORD c = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void cadre(int largeur, int longueur)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Dessine le cadre d'une fenetre
* --------------------------------------------------------------------
* $F_FCTN
*/
//PROTECTED void cadre( int left, int top, int right, int bottom)
//{
//   int x, y;
//
//   left--;
//   top--;
//   right--;
//   bottom--;
//
//   window (left, top, right, bottom);
//
//   //textcolor(CYAN);
//   
//   _gotoxy (left, top);
//
//   putch('É');
//   for (x = left + 1; x < right; x++)
//       putch('Í'); 
//   putch('»');
//   
//   for (y = top + 1; y < bottom; y++)
//   {
//       _gotoxy(left, y);
//       putch('º');
//       for (x = left + 1; x < right; x++)
//           putch(' ');
//       putch('º');
//
//   } 
//   
//   _gotoxy (left, bottom);
//   putch('È');
//   for (x = left + 1; x < right; x++)
//       putch('Í'); 
//   putch('¼');    
//
//   _gotoxy (left, top);
//}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void SaisieChaine( char * chaine )
* PARAMETRES: BAL destinataire
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Choix du fichier a transmettre
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void SaisieChaine( char * chaine )
{
   short int fin = FALSE;
   short int x_min, y_min, y_ref;
   short int i = 0;

   y_ref = 0; // wherey();

   while( fin == FALSE)
   {
	   x_min = 0; //wherex();
	   y_min = 0; // wherey();
      gotoxy(x_min ,y_min);
      if( kbhit())
      {
         /* R‚cup‚ration de la touche clavier */
         chaine[i] = getche();
         switch (chaine[i])
         {
            case CR :
               fin = TRUE;
            break;

            case CORRECTION:
               i --;
               cprintf(" ");
               gotoxy(x_min-1,y_ref);
            break;

            default:
               i++;
            break;
         }
      }
      else
      {
         FinRegion();
         DelaiTache(10);
         DebutRegion();
      }
   }
   chaine[i] = '\0';
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void menu_principal( void )
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Affiche le menu principal
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void MenuPrincipal(void)
{
   /* fenetre principale */
   //cadre( 1, 1, 80, 24);
	system("cls");
   /* entete */
   gotoxy(1,1);
   //textbackground(CYAN);
   //textcolor(RED);
   //clreol();
   gotoxy( 20,1);
   cprintf(" SIMULATEUR CLIENT LAN/IP\r");
   gotoxy(60,0);
   cprintf(" ³  Quitter: Q  ³");
   //textbackground(BLACK);

   gotoxy(1,2);

   /* contenu */
   //textcolor(YELLOW);
   gotoxy(20,3);
   cprintf ("\t a : MESSAGES VERS LE MODULE\r");
   gotoxy(20,5);
   cprintf ("\t b : CHANGEMENT BAL DE RECEPTION PAR DEFAUT\r");
   gotoxy(20,6);
   cprintf ("       La bal de reception par defaut est %c \r",SIMU.boite) ;
   gotoxy(20,8);
   cprintf ("\t c : COMMANDE DOS");

   gotoxy(20,10);
   cprintf ("   Choix => ");

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void simu_num_bal_defaut(void)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Permet de choisir une boite aux lettres pour l'envoi et
*       la reception des messages
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void SimuNumBalDefaut(void)
{
   int  x, y;
   int  fin = FALSE;

  /* nouvelle fenetre */
   //cadre( 2, 4, 32 ,14);
   //textbackground(BLACK);
   system("cls");
   //textcolor(YELLOW);
   gotoxy(2,2);
   cprintf (" CHOIX D'UNE BAL DE RETOUR");
   gotoxy(2,4);
   cprintf ("     \t a : BAL A\r") ;
   gotoxy(2,5);
   cprintf ("     \t b : BAL B\r") ;
   gotoxy(2,6);
   cprintf ("     \t c : BAL C\r") ;
   gotoxy(2,7);
   cprintf ("     \t d : BAL D\r") ;
   gotoxy(2,9);
   cprintf (" Choix => ");

   x = 0; //wherex();
   y = 0; //wherey();

   while( !fin )
   {
      if( kbhit() )
      {
         /* R‚cup‚ration de la touche clavier */
         SIMU.boite = getche();
         fin = TRUE;

         switch (SIMU.boite)
         {
            case 'a' :
            case 'A' :
               SIMU.bal_id = BAL_A ;
            break ;

            case 'b' :
            case 'B' :
              SIMU.bal_id = BAL_B ;
            break ;

            case 'c' :
            case 'C' :
               SIMU.bal_id = BAL_C ;
            break ;

            case 'd' :
            case 'D' :
               SIMU.bal_id = BAL_D ;
            break ;

            default :
               fin = FALSE;
               gotoxy(x,y);
              break ;
         } /* switch */
      }
      else
      {
         FinRegion() ;
         DelaiTache (10);
         DebutRegion();
      }
   } /* while the_end */
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void MenuServices(short int bal_ident)
* PARAMETRES:
*     entree: Boite aux lettres du module
*     retour: Rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Affiche les services disponibles pour le module test‚
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void MenuServices(noyau_bal_id bal_ident)
{
   /* fenetre SERVICE */
//   cadre( 2, 3, 79, 23);
   //textbackground(BLACK);
   //textcolor(LIGHTGREEN);
	system("cls");

   gotoxy(20,2);
   cprintf("  SIMULATION DES MESSAGES VERS LE RESEAU %d\r",bal_ident);

   gotoxy(5,4);
   cprintf ("\t a : CONNEXION\r");
   gotoxy(5,5);
   cprintf ("\t b : DECONNEXION\r");
   gotoxy(5,6);
   cprintf ("\t c : ENVOI D'UN MESSAGE\r");
   gotoxy(5,7);
   cprintf ("\t d : EMISSION D'UN FICHIER");
   gotoxy(5,8);
   cprintf ("\t e : RECEPTION D'UN FICHIER");
   gotoxy(5,9);
   cprintf ("\t f : EMISSION D'UN FICHIER RENOMME");
   gotoxy(5,10);
   cprintf ("\t g : RECEPTION D'UN FICHIER RENOMME");

   gotoxy(45,4);
   cprintf ("\t i : DEBUT DU SERVICE HORAIRE\r");
   gotoxy(45,5);
   cprintf ("\t j : DEMANDE DE MISE A L'HEURE\r");
   gotoxy(45,6);
   cprintf ("\t k : FIN DU SERVICE HORAIRE\r");

   gotoxy(45,7);
   cprintf ("\t l : Debut du service ETAT\r");
   gotoxy(45,8);
   cprintf ("\t m : Demande d'ETAT courant\r");
   gotoxy(45,9);
   cprintf ("\t n : Fin du service ETAT\r");

   gotoxy(45,10);
   cprintf ("\t o : Debut du service DOP\n\r");
   gotoxy(45,11);
   cprintf ("\t p : Demande DOP\n\r");
   gotoxy(45,12);
   cprintf ("\t q : Fin du service DOP\n\r");


   gotoxy(20,13);
   cprintf ("\t s : Changement de BAL de reception temporaire.\r");
   gotoxy(20,14);
   cprintf ("\t t : Terminaison des tƒches\r");

   gotoxy(20,16);
   cprintf ("   Choix => ");

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void MenuDop(short int bal_ident)
* PARAMETRES:
*     entree: Boite aux lettres du module
*     retour: Rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: permet de choisir les types de messages pour le DOP
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED short int MenuNatureDop(void)
{
   short int type = 0;
   char fin = 0;

   /* choix de la nature des messages */
   //cadre( 25, 12, 80, 22);
   //textbackground(BLACK);
   system("cls");
   //textcolor(LIGHTRED);
   gotoxy(2,2);
   cprintf("         SERVICE DOP \r");
   gotoxy(2,4);
   cprintf (" \t 1 - Messages applicatifs + vie");
   gotoxy(2,5);
   cprintf (" \t 2 - Messages applicatifs");

   gotoxy(2,7);
   cprintf ("    Choix => ");

   scanf ("%hd", &type);

   return (type);
}

PROTECTED short int MenuSensDop(void)
{
   short int type = 0;
   char fin = 0;

   /* choix du sens des messages  */
   //cadre( 25, 12, 60, 20);
   //textbackground(BLACK);
   system("cls");
   //textcolor(LIGHTRED);
   gotoxy(2,2);
   cprintf("         SERVICE DOP \r");
   gotoxy(2,4);
   cprintf (" \t 1 - Messages recu du serveur");
   gotoxy(2,5);
   cprintf (" \t 2 - Messages vers le serveur");
   gotoxy(2,6);
   cprintf (" \t 3 - Tous les messages");
   gotoxy(2,8);
   cprintf ("    Choix => ");

   while ( !fin )
   {
      if( kbhit() )
      {
         /* R‚cup‚ration de la touche clavier */
         type = getche();
         type = atoi((char *)&type);
         if( type == SRV_ESPION_ENTRANT || type == SRV_ESPION_SORTANT
               || type == SRV_ESPION_ENTRANT_SORTANT)
            break;
      }
      else
      {
         FinRegion();
         DelaiTache(10);
         DebutRegion();
      }
   }

   return(type);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void SimuEnvoiLan( short int bal_dest, short int service,
*                           short int type_message, void *p_message)
* PARAMETRES: BAL destinataire
*             service demande
*             message du service
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Envoi des messages vers le module test‚
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void SimuEnvoiLan(noyau_bal_id bal_dest,enum_lan_service service,
                  			  enum_lan_type type_message, char *p_msg)
{
   struct_lan_message  *p_message = (struct_lan_message *)(0);

   ExitAlloue((struct_neutre **)(&p_message),
               sizeof(struct_lan_message),POOL_SIMU);

      p_message->entete.service = service;
      p_message->entete.type_message = type_message;

      switch( service )
      {
         case M_LAN_CONNEXION :
            p_message->u.msg_connexion.periode_reconnexion = 180;
         break;

         case M_LAN_MESSAGE :
            if (type_message != LAN_FIN)
            {
                gotoxy (13,18);
                cprintf (" > %s", ((struct_lan_gestion_msg *)p_msg)->contenu);
                p_message->u.message.longueur = ((struct_lan_gestion_msg *)p_msg)->longueur;
                memcpy(&p_message->u.message.contenu,((struct_lan_gestion_msg *)p_msg)->contenu,
                 ((struct_lan_gestion_msg *)p_msg)->longueur);
            }
         break;

         case M_LAN_FICHIER :
                memset(p_message->u.param_fic.hostname,0,LAN_MAX_CAR);
                p_message->u.param_fic.fichier[0] = '\0';
                p_message->u.param_fic.path[0] = '\0';
         break;

         case M_LAN_DOP:
            if( type_message == LAN_DEBUT || type_message == LAN_DEMANDE)
            {
               p_message->u.msg_dop.nature = MenuNatureDop();
               p_message->u.msg_dop.sens = MenuSensDop();
            }
         break;

         default:
         break;
      }

      ExitEnvoie(bal_dest,SIMU.bal_id,(struct_neutre *)(p_message));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void MessagesModule(short int bal_ident)
* PARAMETRES:
*     entree: Boite aux lettres du module
*     retour: Rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Envoi les messages vers le module test‚
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void MessagesModule(noyau_bal_id bal_ident)
{
   int        choix;
   noyau_bal_id    bal_defaut ;
   unsigned char    lettre ;
   int     fin = 0 ;

   /* sauvegarde de la BAL par defaut */
   bal_defaut = SIMU.bal_id ;
   lettre = SIMU.boite ;

   MenuServices(bal_ident);

   while( !fin )
   {
      if( kbhit() )
      {
         /* R‚cup‚ration de la touche clavier */
         choix = getch();
         fin = 1;

         switch (choix)
         {
            case 'a' :
            case 'A' :
               SimuEnvoiLan(bal_ident,M_LAN_CONNEXION,LAN_DEBUT,NULL);
            break ;

            case 'b' :
            case 'B' :
            if( emission_possible )
               SimuEnvoiLan(bal_ident,M_LAN_MESSAGE,LAN_FIN,NULL);
            if( fichier_possible )
               SimuEnvoiLan(bal_ident,M_LAN_FICHIER,LAN_FIN,NULL);
            SimuEnvoiLan(bal_ident,M_LAN_CONNEXION,LAN_FIN,NULL);
            break ;

            case 'c' :
            case 'C' :
            if( emission_possible )
               GestionMessage(bal_ident);
            else
            {
               num_msg ++;
               sprintf(tableau,"Message test numero %lu",num_msg);
               msg_test.longueur = strlen(tableau) +1;
               strcpy(msg_test.contenu,tableau);
               SimuEnvoiLan(bal_ident,M_LAN_MESSAGE,LAN_DEBUT,(char *)&msg_test);
            }
            break;

            case 'd' :
            case 'D' :
               if( fichier_possible )
               { 
                  // SimuEnvoiLan(bal_ident,M_LAN_FICHIER,LAN_DEBUT,NULL);
                  GestionFichier(bal_ident,LAN_EMISSION);
               }
               else
               {
                  SimuEnvoiLan(bal_ident,M_LAN_FICHIER,LAN_DEBUT,NULL);
                  GestionFichier(bal_ident,LAN_EMISSION);
               }
            break ;

            case 'e' :
            case 'E' :
               if( fichier_possible )
                  GestionFichier(bal_ident,LAN_RECEPTION);
               else
               {
                  SimuEnvoiLan(bal_ident,M_LAN_FICHIER,LAN_DEBUT,NULL);
                  GestionFichier(bal_ident,LAN_RECEPTION);
               }
            break;

            case 'f' :
            case 'F' :
               if( fichier_possible )
                  GestionFichier(bal_ident,LAN_EMISSION_RENOMME);
               else
               {
                  SimuEnvoiLan(bal_ident,M_LAN_FICHIER,LAN_DEBUT,NULL);
                  GestionFichier(bal_ident,LAN_EMISSION_RENOMME);
               }
            break ;

            case 'g' :
            case 'G' :
               if( fichier_possible )
                  GestionFichier(bal_ident,LAN_RECEPTION_RENOMME);
               else
               {
                  SimuEnvoiLan(bal_ident,M_LAN_FICHIER,LAN_DEBUT,NULL);
                  GestionFichier(bal_ident,LAN_RECEPTION_RENOMME);
               }
            break;

            case 'i':
            case 'I':
               SimuEnvoiLan(bal_ident,M_LAN_HORAIRE,LAN_DEBUT, NULL);
            break ;

            case 'j' :
            case 'J' :
               SimuEnvoiLan(bal_ident,M_LAN_HORAIRE,LAN_DEMANDE, NULL);
            break ;

            case 'k' :
            case 'K' :
               SimuEnvoiLan(bal_ident,M_LAN_HORAIRE,LAN_FIN,NULL);
            break ;

            case 'l':
            case 'L':
               SimuEnvoiLan(bal_ident,M_LAN_ETAT,LAN_DEBUT, NULL);
            break ;

            case 'm' :
            case 'M' :
               SimuEnvoiLan(bal_ident,M_LAN_ETAT,LAN_DEMANDE, NULL);
            break ;

            case 'n' :
            case 'N' :
               SimuEnvoiLan(bal_ident,M_LAN_ETAT,LAN_FIN,NULL);
            break ;

            case 'o' :
            case 'O' :
               SimuEnvoiLan(bal_ident,M_LAN_DOP,LAN_DEBUT,NULL);
            break ;

            case 'p' :
            case 'P' :
               SimuEnvoiLan(bal_ident,M_LAN_DOP,LAN_DEMANDE,NULL);
            break ;

            case 'q' :
            case 'Q' :
               SimuEnvoiLan(bal_ident,M_LAN_DOP,LAN_FIN,NULL);
            break ;

            case 's' :
            case 'S' :
               /* Changement du Nø de BAL pour la reception */
               SimuNumBalDefaut() ;
               MenuServices(bal_ident);
               break ;

            case 't' :
            case 'T' :
            	/* Envoi du message de terminaison */
               SimuEnvoiLan(bal_ident,M_LAN_MESSAGE,LAN_FIN,NULL);
            	SimuEnvoiLan(bal_ident,M_LAN_ARRET,LAN_DEMANDE,NULL);
            break;

            default :
               fin = 0;
            break ;

         } /* switch  */
      }/* touche !=0 */
      else
      {
      	FinRegion() ;
      	DelaiTache (10);
      	DebutRegion();
      }
   }/* while the_end */
   /* restitution de la BAL par defaut */
   SIMU.bal_id = bal_defaut ;
   SIMU.boite = lettre ;

}

PROTECTED void EnvoiMessage(noyau_bal_id bal_dest)
{
   struct_lan_message_externe msg_lan;

   if( emission_possible)
   {
      msg_lan.entete.service = M_LAN_MESSAGE;
      msg_lan.entete.type_message = LAN_EMISSION;

      num_msg ++;
      msg_lan.u.message.contenu = &tableau[0];
      sprintf(msg_lan.u.message.contenu," Message test numero %lu",num_msg);
      msg_lan.u.message.longueur = strlen(tableau) +1;
      EnvoiLan(bal_dest,SIMU.bal_id,POOL_SIMU,&msg_lan);
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void GestionMessage( short int bal_dest )
* PARAMETRES: BAL destinataire
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Choix des messages a transmettre
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void GestionMessage(noyau_bal_id bal_dest )
{
   short int i, x_min, y_min;
   char valeur[3];
   char fin = 0;
   struct_lan_message_externe msg_lan;

   /* nouvelle fenetre */
   //cadre( 3, 14, 73, 22);
   //textbackground(WHITE);
   system("cls");
   //textcolor(BLUE);
   gotoxy(2,2);
   cprintf("      MESSAGE A TRANSMETTRE   \r");
   gotoxy(2,4);
   cprintf (" longueur (si '= 0',message test) : ");

   i = 0;
   while ( fin == 0)
   {
      if( kbhit())
      {
         /* R‚cup‚ration de la touche clavier */
         valeur[i] = getche();
         if( valeur[i++] == CR )
            break;
      }
      else
      {
         FinRegion();
         DelaiTache(10);
         DebutRegion();
      }
   }
   if( i == 1)
   {
      /* message de test car longueur == 0 */
      num_msg ++;
      sprintf(tableau," Message test numero %d",num_msg);
      msg_lan.u.message.longueur = strlen(&tableau[0])+1;
   }
   else
   {
      msg_lan.u.message.longueur = atoi(&valeur[0]);

      /* Message autre que le message de test */
     if( msg_lan.u.message.longueur > 0 )
     {
		 x_min = 0; //wherex();
		 y_min = 0; // wherey();
        gotoxy(x_min+1,y_min+1);
        cprintf (" message : ");
        SaisieChaine(&tableau[0]);
     }
   }

   /* recuperer le message */
   msg_lan.u.message.contenu = &tableau[0];

   msg_lan.entete.service = M_LAN_MESSAGE;
   msg_lan.entete.type_message = LAN_EMISSION;

   /* envoi du message */
   EnvoiLan(bal_dest,SIMU.bal_id, POOL_SIMU, &msg_lan);

   //textbackground(BLACK);
   system("cls");

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void GestionFichier( short int bal_dest )
* PARAMETRES: BAL destinataire
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Choix du fichier a transmettre
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void GestionFichier(noyau_bal_id bal_dest, enum_lan_type type)
{
   struct_lan_message_externe msg_lan;
   char fichier[80];
   char path[80];

   /* nouvelle fenetre */
   //cadre( 5, 10, 75, 16);
   //textbackground(BLACK);
   system("cls");
   //textcolor(LIGHTRED);
   gotoxy(2,2);
   switch( type )
   {
      case LAN_EMISSION:
      case LAN_EMISSION_RENOMME:
         cprintf("            FICHIER A TRANSMETTRE   \r");
      break;
 	   case LAN_RECEPTION:
 	   case LAN_RECEPTION_RENOMME:
          cprintf("            FICHIER A RECEVOIR   \r");
      break;

      default :
      break;
   }
   gotoxy(2,4);
   cprintf (" Nom complet du fichier source : ");
   /* saisie du nom du fichier */
   SaisieChaine(fichier);

   gotoxy(2,5);
   if( type == LAN_EMISSION_RENOMME || type == LAN_RECEPTION_RENOMME )
      cprintf(" nom complet du fichier final: ");
   else
      cprintf (" R‚pertoire final : ");
   /* saisie du path du fichier */
   SaisieChaine(path);

   /* envoi */

   //textbackground(BLACK);
   system("cls");

   msg_lan.entete.service = M_LAN_FICHIER;
   msg_lan.entete.type_message = type;
   msg_lan.u.param_fic.fichier = fichier;
   msg_lan.u.param_fic.path = path;
   msg_lan.u.param_fic.hostname = "\0";

   EnvoiLan(bal_dest,SIMU.bal_id, POOL_SIMU, &msg_lan);

}


