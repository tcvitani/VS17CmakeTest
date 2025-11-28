/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: INIT
* FICHIER: INIT.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Module contenant la routine main par laquelle le
* programme demarre. Ce module est charge de verifier la coherance
* de l'environnement logiciel et de positionner les variables
* globales avant de lancer les autres taches. Ce module devient
* ensuite la tache de fond.
* --------------------------------------------------------------------
* DESCRIPTION:
* Le travail effectue par ce module est le suivant :
*     1) verifier que le repertoire d'execution existe sur le disque
*        ( le logiciel ne demarre pas si ce test echoue)
*     2) Choisir ce repertoire comme repertoire courant.
*     3) Verifier que le repertoire de demarrage existe et le creer
*        le cas echeant.
*     4) Verifier que le disque virtuel existe.
*        ( le logiciel ne demarre pas si ce test echoue)
*     5) Ouvrir le fichier d'initialisation du logiciel et verifier
*        que son contenu est coherant.
*        ( le logiciel ne demarre pas si ce test echoue)
*    14) Effacer l'ecran de la VM et rendre la main a DOS.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/modulo/voies/horodate/simu/init.c_v  
 * 
 *    Rev 1.1   Aug 06 1996 14:40:30   NHA
 *  
 * 
 *    Rev 1.0   Aug 05 1996 09:00:26   NHA
 *  
* --------------------------------------------------------------------
* $F_HEAD
*/


/*--------------- INCLUDES: ---------------*/

#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>

#include <noyau.h>

#include <init.h>

#include <simu1.h>

#include <horodate.h>


/*--------------- RESERVED: ---------------*/
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/

short int priorite_tache = THREAD_PRIORITY_NORMAL;

/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void main(void)
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Programme principal : point d'entree dans l'application
* ROLE: Lancer le soft VM
* --------------------------------------------------------------------
* $F_FCTN
*/

void main(void)
{
   short int         rc;
   noyau_bal_id      bl;
   struct_hrd_config hrd_config;

   while (1)
   {
      /* initialisation de la structure config de hrd */
      hrd_config.num_pool = NULL;
      hrd_config.nb_alarme_max = 8;
      hrd_config.priorite_max = priorite_tache;
      hrd_config.priorite_init_max = THREAD_PRIORITY_HIGHEST ;

      syst_ecrire_trace("Appel de la fonction HRDLance()\n");
      rc = HRDLance(hrd_config, &bl);
      if ( rc != HRD_LANCE_OK)
      {
         FinRegion();
         syst_ecrire_trace("Probläme HRDLance\n");
         exit(EXIT_NO_TACHES);
      }

      syst_ecrire_trace("Appel de la fonction SIMULance()\n");
      rc = SimuLance(PRIORITE_MAX);
      if ( rc != NOYAU_OK)
      {
         HRDArret();
         FinRegion();
         syst_ecrire_trace("Probläme lance simu\n");
         exit(EXIT_NO_TACHES);
      }

      etat_logiciel = LOGICIEL_EN_COURS;

      while (etat_logiciel == LOGICIEL_EN_COURS)
         Sleep (1000);

      etat_logiciel = LOGICIEL_FINI;

      syst_ecrire_trace("LOGICIEL_FINI\n");

      HRDArret ();
      SimuArret();

      if (etat_logiciel == LOGICIEL_FINI)
         break;
   }

   printf("LOGICIEL SORTIE\n");

   return ;
}

/*======================= THAT'S ALL FOLKS ==============================*/
