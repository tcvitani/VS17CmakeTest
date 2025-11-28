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
 * $Log:   T:/MODULO/VoieNt/Referenc/test/Sources/simu0.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:56   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jun 04 1997 19:11:40   DPI
 *  
 * 
 *    Rev 1.1   Jun 03 1997 17:38:38   DPI
 *  
 * 
 *    Rev 1.0   Apr 28 1997 15:44:54   DPI
 *  
 * 
 *    Rev 1.1   Apr 07 1997 13:53:32   ANA
 * Mise au point pour avoir deux taches distinctes :
 * emission et reception de messages vers le module
 * 
 *    Rev 1.0   Mar 21 1997 09:29:36   ANA
 * Creation
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <direct.h>

/* module noyau */
#include <noyau.h>
#include <tcp_ip.h>
#include <console.h>

#include ".\simu0.h"

#include ".\simu_glo.h"

#include <memclass.h>

/*------------------DEFINES:--------------------------*/
#define ESCAPE        0x1B
#define ENTER         0x0D
#define CORRECTION    0x08



/*------------------TYPEDEF:--------------------------*/


/*--------------------FONCTIONS: ---------------*/


/*------------------VARIABLES:------------------------*/

/* tƒche de simulation */
PROTECTED struct_tache t_simu[NB_TACHE+1] =
{
   /* Activation, priorite, taille pile, point d'entree */
   { 1, 0, 2048, Simulateur, NULL, NULL, "Simulateur"},
   { 1,
   0,
   2048,
   SimuRecoit,
   NULL,
   NULL,
   "SimuRecoit"},

   { 0,0,0,NULL, NULL, NULL, "" }
};

/*-------------------CODE:----------------------*/

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
PROTECTED void cadre( int largeur, int longueur )
{
   short int i;

//   textbackground(WHITE);
   clrscr();
   textcolor(CYAN);
   for( i = 2; i<longueur; i++)
   {
      gotoxy(1,i );
      cprintf("º");
      gotoxy(largeur,i);
      cprintf("º");
   }

   gotoxy(1,1);
   cprintf("É");
   gotoxy(1,longueur);
   cprintf("È");
   gotoxy(largeur,1);
   cprintf("»");
   gotoxy(largeur,longueur);
   cprintf("¼");

   for( i = 2; i<largeur; i++ )
   {
      gotoxy(i,1);
      cprintf("Í");
      gotoxy(i,longueur);
      cprintf("Í");
   }

}

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


BOOLEAN SaisieChaine( char * chaine, short lg_max, int xpos, int ypos )
{
	short int fin = FALSE;
	short int x_min, y_min /*, y_ref*/;
	short int i = 0;
	boolean retour = TRUE;
	x_min = xpos;
	y_min = ypos;
	
/*	for( i = 0; i<lg_max; i++ )
		cprintf("-");*/
	gotoxy(x_min ,y_min);
	
	i = 0;
	//y_ref = wherey();
	
	while((fin == FALSE) && (i < lg_max - 1))
	{
		//x_min = wherex();
		//y_min = wherey();
		gotoxy(x_min + i ,y_min);
		if( kbhit())
		{
			/* R‚cup‚ration de la touche clavier */
			chaine[i] = getche();
			switch (chaine[i])
			{
            case ENTER :
				fin = TRUE;
				break;
				
            case CORRECTION:
				i --;
				cprintf(" ");
				gotoxy(x_min-1,y_min);
				break;
				
            case ESCAPE :
				fin = TRUE;
				retour = FALSE;
				break;
				
            default:
				i++;
    			//printf("%s", chaine[i]);
				break;
			}// switch
		}// if kbhit
	}// while
	/* caractŠre de fin de chaine */
	chaine[i] = '\0';
	
	return retour;
	
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
   window( 1, 1, 80, 25);
   cadre(80,15);

   /* entete */
   gotoxy(1,1);
   textbackground(CYAN);
   textcolor(YELLOW);
   clreol();
   gotoxy( 20,1);
   cprintf(" SIMULATOR 'Test for reference and TFT&TCI modules \r");
 
   textbackground(BLACK);
   gotoxy(1,2);

   /* contenu */
   textcolor(YELLOW);
   gotoxy(20,3);
   cprintf ("\t a : SEND REFERENCE MESSAGE\r");
   gotoxy(20,5);
   cprintf ("\t b : SEND MANUAL MESSAGE\r");
   gotoxy(20,7);
   cprintf ("\t c : START/STOP SERVICES\r");
   gotoxy(20,9);
   cprintf ("\t d : TWO REFERENCE FILES\r");
   gotoxy(20,11);
   cprintf ("\t q : QUIT");

   gotoxy(20,13);
   cprintf ("   Choix => ");

}

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
   short int choix;
   short int x1 = 0, y1 = 0;
   char command[10] = {0};
   int   hit;

   DebutRegion();

   /* Changement de la priorite de la tache  */
   ChangePriorite( TacheCourante() , SIMU.priorite_tache) ;

   SIMU.flag_fin = FALSE;

   MenuPrincipal();
//   x1 = wherex();
  // y1 = wherey();

   FinRegion();

   /* Attente publication de la BAL par le module en test (Reference) */
   SIMU.bal_rfr = AttendBAL ("BL_RFR");
   
   /* traitement */
   while ( TRUE)
   {
      DebutRegion();

      window( 1, 1, 80, 25);
      gotoxy(x1,y1);
      hit = kbhit();
      if( hit != 0)
      {
         /* R‚cup‚ration de la touche clavier */
         choix = getch();
         switch (choix)
         {
            case 'a' :   /* Messages vers le module */
            case 'A' :
               SendReference() ;
               break;

            case 'b' :   /* Messages vers le module */
            case 'B' :
               SendManual() ;
               break;

            case 'c' :   /* Messages vers le module */
            case 'C' :
               ProcessStop() ;
               break;

            case 'd' :   /* Process two messages */
            case 'D' :
               ProcessMessages() ;
               break;

            case 'q' :     /* quitter */
            case 'Q' :
               SIMU.flag_fin = TRUE;
               break ;

            default:
            break;
         } /* switch */

        MenuPrincipal();
      }/* touche !=0 */

      if( SIMU.flag_fin )
      {
         /* Terminaison de la tƒche */
         etat_logiciel = LOGICIEL_FINI;
      }

//      printf ("\nTache Simu0");
      FinRegion();

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
