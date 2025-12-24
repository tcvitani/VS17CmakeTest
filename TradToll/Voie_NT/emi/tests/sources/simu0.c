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
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/test/Sources/simu0.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:08   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   May 16 1997 13:35:20   ANA
 * Test de la creation  et changement de fichier
 * avec emission en parallèle
 * 
 *    Rev 1.2   May 13 1997 17:12:10   ANA
 * Gestion de la sauvegarde sur disque et de la purge
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
//#include <tcp_ip.h>
//#include <console.h>

#include <msg_fic.h>

#include <simu0.h>
#include <simu_glo.h>

#include <memclass.h>

/*------------------DEFINES:--------------------------*/



/*------------------TYPEDEF:--------------------------*/


/*--------------------FONCTIONS: ---------------*/

/*------------------VARIABLES:------------------------*/

/* tƒche de simulation */
/* tƒche de simulation */
PROTECTED struct_tache t_simu[NB_TACHE+1] =
{
   /* Activation, priorite, taille pile, point d'entree */
   { 1, 0, 2048, Simulateur, NULL, NULL, "Simulateur"},
   { 1, 0, 2048, SimuRecoit, NULL, NULL, "SimuRecoit"},

   { NOYAU_FAUX,0,NULL,NULL, NULL, NULL, NULL }
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
   //clrscr();
   //textcolor(LIGHTMAGENTA);
   //for( i = 2; i<longueur; i++)
   //{
   //   gotoxy(1,i );
   //   cprintf("º");
   //   gotoxy(largeur,i);
   //   cprintf("º");
   //}

   //gotoxy(1,1);
   //cprintf("É");
   //gotoxy(1,longueur);
   //cprintf("È");
   //gotoxy(largeur,1);
   //cprintf("»");
   //gotoxy(largeur,longueur);
   //cprintf("¼");

 /*  for( i = 2; i<largeur; i++ )
   {
      //gotoxy(i,1);
      cprintf("Í");
      //gotoxy(i,longueur);
      cprintf("Í");
   }*/

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
PROTECTED void SaisieChaine( char * chaine )
{
   short int fin = FALSE;
   short int x_min, y_min, y_ref;
   short int i = 0;

   //y_ref = wherey();

   while( fin == FALSE)
   {
      //x_min = wherex();
      //y_min = wherey();
      //gotoxy(x_min ,y_min);
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
               //gotoxy(x_min-1,wherey());
            break;

            default:
               i++;
            break;
         }
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
   ////window( 1, 1, 80, 25);
   //cadre(80,13);

   ///* entete */
   //gotoxy(1,1);
   //textcolor(YELLOW);
   //clreol();
   //gotoxy( 20,1);
   cprintf(" SIMULATEUR DE L'EMISSION DE FICHIER \r");
 
   //textbackground(BLACK);
   //gotoxy(1,2);

   ///* contenu */
   //textcolor(YELLOW);
   //gotoxy(20,2);
   //gotoxy(20,4);
   cprintf ("\t a : RESTITUTION DE FICHIER\r");

 //  gotoxy(20,5);
   cprintf ("\t b : SERVICE ETAT\r");

 //  gotoxy(20,6);
   cprintf("\t c : CONFIG\r");
   
 //  gotoxy(20,7);
   cprintf("\t d : CHANGE MSG FILE\r");

 //  gotoxy(20,8);
   cprintf ("\t q : QUIT\r");
   
//   gotoxy(25,10);
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
   short int x1, y1;
   char command[10];
   int   hit;
   char path_fichier[MAX_PATH];

   DebutRegion();

   /* Changement de la priorite de la tache  */
   ChangePriorite( TacheCourante() , SIMU.priorite_tache) ;

   SIMU.FileStatus = RechercherPremierMessageFichier(&SIMU.DateFile, &SIMU.num_fichier, &SIMU.MsgFile);

   SIMU.flag_fin = FALSE;

   MenuPrincipal();

   FinRegion();

   /* Attente publication de la BAL par le module en test (Reference) */
   SIMU.bal_emi = AttendBAL ("BL_EMI");
   
   /* traitement */
   while ( TRUE)
   {
      DebutRegion();

      //window( 1, 1, 80, 25);
      //gotoxy(x1,y1);
      hit = kbhit();
      if( hit != 0)
      {
         /* R‚cup‚ration de la touche clavier */
         choix = getch();
         switch (choix)
         {
            case 'a' :   /* Messages vers le module */
            case 'A' :
               SendRestitution();
               break;

            case 'b' :   /* Messages vers le module */
            case 'B' :
               SendEtat() ;
               break;

            case 'c' :   /* Messages vers le module */
            case 'C' :
               SendConfig(); ;
               break;
			case 'd':
			case 'D':
				ChangerMessageFichier(&SIMU.DateFile,&SIMU.num_fichier, &SIMU.MsgFile);
				NotifyChange();
				break;

            case 'q' :     /* quitter */
            case 'Q' :
				SendArret(); 
				DelaiTache(3);
				SIMU.flag_fin = TRUE;
               break ;

            default:
            break;
         } /* switch */

        MenuPrincipal();
      }/* touche !=0 */
	  else
		  DelaiTache(3);

      if( SIMU.flag_fin )
      {
         /* Terminaison de la tƒche */
         etat_logiciel = LOGICIEL_FINI;
         //sprintf(path_fichier, "%08d" ,SIMU.num_fichier);
		 //TerminerFichier(SIMU.MsgFile,path_fichier,TRUE);
		 if (SIMU.FileStatus)
			FermerMessageFichier(&SIMU.DateFile, &SIMU.num_fichier, &SIMU.MsgFile);
      }

      FinRegion();

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

PUBLIC noyau_enum_retour SimuLance(struct_simu_priorite priorite)
{
    noyau_enum_retour retour;
    short int nb_tache ;

    /* Initialisation de la priorite des taches */
    for ( nb_tache=0 ; nb_tache<NB_TACHE ; nb_tache++)
       t_simu[nb_tache].noyau_priorite_tache = priorite.init;

    SIMU.priorite_tache = priorite.courant;
    //SIMU.bal_module = bal;

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

PUBLIC noyau_enum_retour SimuArret(void)
{
    noyau_enum_retour retour;

    /* Arrˆt des taches du module */
    retour = ArretTaches (t_simu);

    return retour;
}



