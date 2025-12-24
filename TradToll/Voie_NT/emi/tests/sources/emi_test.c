/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Reception du fichier de reference
* FICHIER: RFR_TEST.C
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
*     3)Lancer les taches temps-reel
*     4)Attendre dans un while(1) que les taches s'arretent.
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/test/Sources/emi_test.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:08   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.4   19 Mar 1998 14:35:04   DPI
 *  
 * 
 *    Rev 1.3   Jun 12 1997 18:51:30   DPI
 *  
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
#include <csr_lan.h>


/* module NOYAU, ... */
#include <noyau.h>
#include <horodate.h>
#include <fic.h>

//#include <emi_fic.h>

#define INIT_DEF
#include "simu0.h"

#include "simu_glo.h"
#include "emi_test.h"

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
* ROLE: Lancer le soft de communication par EMI_IP
* --------------------------------------------------------------------
* $F_FCTN
*/


void __cdecl main(void)
{
	enum_instance_result	result;
	struct_simu_priorite	priorite;
//	noyau_bal_id			void_bal_tft,void_bal_tci;
#ifdef VRAI_HORODATE
	enum_hrd_compte_rendu	res_hrd;
	/* structure utilis‚e par HRDLANCE */
	struct_hrd_config config_hrd = { THREAD_PRIORITY_ABOVE_NORMAL,
							               THREAD_PRIORITY_NORMAL,
							               (noyau_pool_id)0,
				                        10 }; /* nbre d'alarmes max */

	FILE *pf_hrd;
#endif
	

	/* BAL module LAN (necessaire au module Reference) */
	//SIMU.bal_lan = PublieBAL ("BL_LAN" ,NOYAU_BAL_ILLIMITEE);
/* if( (LanLance(
       CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG MOD_REG_KEYn_MODULES "LAN\\BL_LAN",
       "BL_LAN", &SIMU.bal_lan) )!= INST_INIT_OK )
  {
	  printf("Problem starting LAN!!");	
	  exit(EXIT_NO_TACHES);
  }*/

	SIMU.bal_lan = AttendBAL("BL_LAN");
    	/* lancement d'horodate */
#ifdef VRAI_HORODATE
	res_hrd = HRDLance(config_hrd, &SIMU.bal_hrd);
	
	if ( res_hrd != HRD_LANCE_OK)
	{
		printf("Pb Lancement du module Horodate : %d", res_hrd);
		exit(res_hrd);
   	}
	if ((pf_hrd = fopen ("c:\\csr\\_traces\\hrd.trc", "wt")) != NULL)
		HRDDebutTrace (pf_hrd);
#else
    /* BAL module HRD (horodate) */
	SIMU.bal_hrd = PublieBAL ("HRD_0" ,NOYAU_BAL_ILLIMITEE);
#endif

//	SIMU.bal_hrd = NOYAU_BAL_HORLOGE_ID; /* BAL module Horodate */

   /* Lancement du module Emi */
   if( (result = EmiLance(
       CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG MOD_REG_KEYn_MODULES "EMI\\BL_EMI",
       "BL_EMI", &SIMU.bal_emi) )!= INST_INIT_OK )
   {
      printf("Pb Lancement du module EMI : %d",result);


#ifdef VRAI_HORODATE
	  if (pf_hrd != NULL)
	  {
		HRDFinTrace ();
		fclose (pf_hrd);
	  }
#endif
      exit(result);
   }

   priorite.init = PRIORITE_MAX;
   priorite.courant = PRIORITE_TACHE;

   if( SimuLance(priorite) != NOYAU_OK)
   {
      printf("ProblŠme lance simu\n");
#ifdef VRAI_HORODATE
	  if (pf_hrd != NULL)
	  {
		HRDFinTrace ();
		fclose (pf_hrd);
	  }
#endif
      exit(EXIT_NO_TACHES);
   }

/*
 * On boucle en attendant que les autres taches
 * s'arretent.
 */
   etat_logiciel = LOGICIEL_EN_COURS;


   while (etat_logiciel == LOGICIEL_EN_COURS)
	   DelaiTache(2);
   
/*
 * On termine de la maniere souhaitee par les
 * autres taches.
 */

   EmiArret(SIMU.bal_emi);

//	LanArret(SIMU.bal_lan);

#ifdef VRAI_HORODATE
   HRDArret();
	  if (pf_hrd != NULL)
	  {
		HRDFinTrace ();
		fclose (pf_hrd);
	  }
#endif

   result = SimuArret ();   
   if( result != NOYAU_ARRET_TACHE_OK )
   {
      exit(EXIT_PB_ARRET_TACHES);
   }

   system("cls");

   exit(EXIT_OK);

}












