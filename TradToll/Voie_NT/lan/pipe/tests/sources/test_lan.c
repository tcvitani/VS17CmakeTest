/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN_IP
* FICHIER: LAN_TEST.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Module contenant la routine main par laquelle le
*         programme demarre. Ce module est charge de positionner
*         les variables globales avant de lancer les autres taches.
*         Ce module devient ensuite la tache de fond.
* --------------------------------------------------------------------
* DESCRIPTION:
* Le travail effectue par ce module est le suivant :
*     1)Initialiser RTC
*     2)Initialiser LAN/IP
*     3)Lancer les taches temps-reel
*     4)Attendre dans un while(1) que les taches s'arretent.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/test/test_lan.c_v  $
 * 
 *    Rev 1.0   Nov 22 1999 14:55:18   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.2   Jan 20 1999 11:36:22   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:46:16   bph
 *  
 * 
 *    Rev 1.14   09 Dec 1997 16:48:48   BPH
 *  
 * 
 *    Rev 1.13   03 Nov 1997 14:21:06   BPH
 * Utilisation de ExitAlloue, ExitEnvoie, ExitLibere
 * et du debug facon noyau 6
 * 
 *    Rev 1.12   07 Oct 1997 11:32:10   ANA
 * Test version 4.00 avec nouvelle librairie de 
 * decoupage/recollage des fichiers ‡ transferer
 * 
 *    Rev 1.11   11 Sep 1997 15:56:14   SEL
 *  
 * 
 *    Rev 1.10   Aug 04 1997 17:34:46   ANA
 * Transfert de fichiers avec renommage
 * 
 *    Rev 1.9   Jul 02 1997 15:24:52   ANA
 * Mise ‡ jour pour PEMM et noyau 5.00
 * Nouvelle gestion du DOP
 * 
 *    Rev 1.7   Apr 05 1997 16:59:30   ANA
 * Modifications pour l'arret des taches
 * 
 *    Rev 1.6   Mar 11 1997 16:52:42   ANA
 * Adaptation au fur et a mesure des tests
 * 
 *    Rev 1.5   Mar 07 1997 18:33:36   ANA
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
 *  
 *
*
* --------------------------------------------------------------------
* $F_HEAD
*/


/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <string.h>
#include <time.h>

/* module NOYAU */
#include <noyau.h>

#include <csr_lan.h>

#define INIT_DEF
#include "simu.h"

#include "test_lan.h"


/*--------------- RESERVED: ---------------*/
#include "memclass.h"

/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/

/*--------------- VARIABLES: ---------------*/

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
* ROLE: Lancer le soft de communication par LAN_IP
* --------------------------------------------------------------------
* $F_FCTN
*/

void __cdecl main(void)
{
   int temp;
   noyau_bal_id bl_lan;
   struct_simu_priorite priorite;
   char * fichier_init = "lan.ini";

/*
 * Si le lancement du reseau echoue,
 * on ne demarre pas !
*/
   if( (temp = LanLance(
       CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG MOD_REG_KEYn_MODULES "LAN\\BL_LAN",
       "BAL_LAN_0", &bl_lan) )!= INST_INIT_OK )
   {
      printf("Pb Lancement du reseau : %d",temp);
      exit(EXIT_NO_RESEAU);
   }

   priorite.init = PRIORITE_MAX;
   priorite.courant = PRIORITE_TACHE;

   if( SimuLance(priorite) != NOYAU_OK)
   {
      printf("Probläme lance simu\n");
      exit(EXIT_NO_TACHES);
   }

/*
 * On boucle en attendant que les autres taches
 * s'arretent.
 */
   etat_logiciel = LOGICIEL_EN_COURS;


   while (etat_logiciel == LOGICIEL_EN_COURS)
      DelaiTache (100);  

/*
 * On termine de la maniere souhaitee par les
 * autres taches.
 */
   DebutRegion();
   temp = LanArret(bl_lan);
   if( temp != INST_ARRET_OK )
   {
      SimuArret();
      exit(EXIT_PB_ARRET_TACHES);
   }

   temp = SimuArret ();
   if( temp != NOYAU_ARRET_TACHE_OK )
   {
      exit(EXIT_PB_ARRET_TACHES);
   }

   exit(EXIT_OK);
}