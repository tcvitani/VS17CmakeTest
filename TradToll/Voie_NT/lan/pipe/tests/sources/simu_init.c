/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SIMULATEUR
* FICHIER: SIMU0.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/test/simu_init.c_v  $ 
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jan 20 1999 11:36:18   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:46:10   bph
 *  
 * 
 *    Rev 1.12   03 Nov 1997 14:20:54   BPH
 * Utilisation de ExitAlloue, ExitEnvoie, ExitLibere
 * et du debug facon noyau 6
 * 
 *    Rev 1.11   07 Oct 1997 11:31:58   ANA
 * Test version 4.00 avec nouvelle librairie de 
 * decoupage/recollage des fichiers à transferer
 * 
 *    Rev 1.10   Aug 04 1997 17:34:42   ANA
 * Transfert de fichiers avec renommage
 * 
 *    Rev 1.9   Jul 02 1997 15:22:26   ANA
 * Mise à jour pour PEMM et noyau 5.00
 * Nouvelle gestion du DOP
 * 
 *    Rev 1.8   Apr 05 1997 16:59:26   ANA
 * Modifications pour l'arret des taches
 * 
 *    Rev 1.7   Mar 21 1997 15:09:30   ANA
 *  
 * 
 *    Rev 1.6   Mar 11 1997 16:52:36   ANA
 * Adaptation au fur et a mesure des tests
 * 
 *    Rev 1.5   Mar 07 1997 18:33:34   ANA
 * Evolution du message de vie et Ajout de la tache
 * de gestion du service HORAIRE
 *
 *    Rev 1.4   Mar 03 1997 14:09:40   ANA
 * Integration du module NOYAU gerant la com LAN/IP
 *
 *    Rev 1.3   Feb 26 1997 16:41:06   ANA
 * Configuration reseau par fichier
 *
 *    Rev 1.2   Feb 20 1997 15:56:58   ANA
 * Version 0
 * 
 *    Rev 1.1   Feb 10 1997 17:15:38   ANA
 *    Reprise du simulateur du module ZIP
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <direct.h>

/* module noyau */
#include <noyau.h>
#include <csr_lan.h>
//#include <console.h>
#include <conio.h>

#include "simu.h"

#define INIT_DEF
#include "simu_glo.h"

#include <memclass.h>

/*------------------DEFINES:--------------------------*/



/*------------------TYPEDEF:--------------------------*/


/*--------------------FONCTIONS: ---------------*/


/*------------------VARIABLES:------------------------*/

/* tƒche de simulation */
PROTECTED struct_tache t_simu[NB_TACHE+1] =
{
   /* Activation, priorite, taille pile, point d'entree */
   { 1, 0, 2048, Simulateur, NULL, NULL},
   { 1, 0, 2048, SimuRecoit},
   { 1, 0, 2048, SimuSrvPipe},
   { 0, 0, 0, NULL,NULL,NULL }
};

/*-------------------CODE:----------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned int Simulateur(void)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: tache de simulation
* ROLE: simuler les echanges entre le module et l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED DWORD WINAPI Simulateur (void *param)
{
   short int       choix;
   short int x1,y1;
   char command[10];
   struct_lan_message_externe msg_lan;

   DebutRegion();

   /* Changement de la priorite de la tache  */
   ChangePriorite( TacheCourante() , SIMU.priorite_tache) ;

   SIMU.flag_fin = FALSE;

   /* Boite aux lettres par defaut*/
   SIMU.boite = 'A' ;
   MenuPrincipal();
   x1 = 0; // wherex();
   y1 = 0; // wherey();
   

   FinRegion();
   
   SIMU.bal_id = PublieBAL ("SIMU_LAN_0", NOYAU_BAL_ILLIMITEE);
   
   SIMU.bal_module = AttendBAL ("BAL_LAN_0");
   
   /* traitement */
   while ( TRUE )
   {
      if( _kbhit())
      {
         /* R‚cup‚ration de la touche clavier */
         choix = _getch();

         DebutRegion();

         switch (choix)
         {
            case 'a' :   /* Messages vers le module */
            case 'A' :
               MessagesModule(SIMU.bal_module) ;
               break;

            case 'b' :   /* Messages de service */
            case 'B' :
               SimuNumBalDefaut();
               break;

            case 'c':
            case 'C':
               system("cls");
               //textcolor(LIGHTGRAY);
               //_getdcwd(0,command, sizeof(command));
               cprintf("C:\\%s>",command);
               SaisieChaine(command);
               system(command);
               cprintf("Appuyer sur une touche pour revenir au menu principal...");
               while(!kbhit())
               {
                  FinRegion();
                  DelaiTache(10);
                  DebutRegion();
               }
            break;

            case 'q' :     /* quitter */
            case 'Q' :
               msg_lan.entete.service = M_LAN_ARRET;
               msg_lan.entete.type_message = LAN_DEMANDE;
               EnvoiLan(SIMU.bal_module,SIMU.bal_id,POOL_SIMU,&msg_lan);
               FinRegion(); 
               Termine();

            break ;

            case 'x' :
//               geninterrupt (199);
               break;

            default:
               cprintf(" TOUCHE NON VALIDE");
               gotoxy(x1,y1);
               break;
         } /* switch */

        MenuPrincipal();

        FinRegion();

      }/* touche !=0 */
    
      DelaiTache(5);
   }
}



/*****************************************************************************
* Fonction  : SimuLance
*-----------------------------------------------------------------------------
* Parametres: short int   pool_id : Nø du pool … utiliser
*-----------------------------------------------------------------------------
*  Action   : Lance la tƒche SIMU
*-----------------------------------------------------------------------------
* Retour    : G_OK
*****************************************************************************/

PUBLIC short int SimuLance(struct_simu_priorite priorite)
{
    short int retour = 0;
    short int nb_tache ;

    /* Initialisation de la priorite des taches */
    for ( nb_tache=0 ; nb_tache<NB_TACHE ; nb_tache++)
       t_simu[nb_tache].noyau_priorite_tache = priorite.init;

    SIMU.priorite_tache = priorite.courant;

    /* Lancement de la tƒche SIMU */
    retour = LanceTache (t_simu);
    
    AttendBAL ("SIMU_LAN_0");
    
    return( retour );
}

/*****************************************************************************
* Fonction  : SIMUArret
*-----------------------------------------------------------------------------
* Parametres: Aucun
*-----------------------------------------------------------------------------
*  Action   : Arrˆt la tƒche SIMU
*-----------------------------------------------------------------------------
* Retour    : G_OK si OK
*****************************************************************************/

PUBLIC short int SimuArret(void)
{
    short int retour = 0;

    /* Arrˆt des taches du module */
    retour = ArretTaches (t_simu);

    return retour;
}
