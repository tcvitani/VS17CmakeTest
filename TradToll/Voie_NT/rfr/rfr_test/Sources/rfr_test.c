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
 * $Log:   T:/MODULO/VoieNt/Referenc/test/Sources/rfr_test.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:56   afx
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


/* module NOYAU, ... */
#include <noyau.h>
#include <horodate.h>
#include <fic.h>
#include <run.h>

#include <referenc.h>

#define INIT_DEF
#include ".\simu0.h"

#include ".\simu_glo.h"
#include ".\rfr_test.h"

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


void __cdecl main( int iArgc, char ** ppcArgv )
{
	enum_instance_result	result;
	struct_simu_priorite	priorite;
	noyau_bal_id			void_bal;
	int iIndex;
	char *pcParm;
	HINSTANCE hinstance;
	char szModulBal[MAX_PATH] = {0}, szModulPath[MAX_PATH] = {0};
	char szModulRegKey[MAX_PATH] = {0};

	PMOD_PROC_LANCE	pMODLance;
	PMOD_PROC_ARRET	pMODArret;
	PMOD_PROC_LANCE	pREFLance;
	PMOD_PROC_ARRET	pREFArret;
#ifdef VRAI_HORODATE
	enum_hrd_compte_rendu	res_hrd;
	/* structure utilis‚e par HRDLANCE */
	struct_hrd_config config_hrd = { THREAD_PRIORITY_ABOVE_NORMAL,
							               THREAD_PRIORITY_NORMAL,
							               (noyau_pool_id)0,
				                        10 }; /* nbre d'alarmes max */

	FILE *pf_hrd;
#endif

	// Arguments
	if ( iArgc < 3 )
    {
        printf( "ERROR : Parameter is missing, use /HELP to display help\n" );
        return;
    }

    for ( iIndex = 1 ; iIndex < iArgc ; iIndex ++ )
    {
        pcParm = ppcArgv[iIndex];

        if ( stricmp( pcParm, "/HELP" ) == 0 )
        {
            printf( "RFR TEST.\n" );
            printf( "Parameters :\n" );
            printf( "    /HELP	: Display this text.\n" );
			printf( "    MODULE PATH\n" );
			printf( "    REG PATH\n" );
            printf( "Example :\n" );
            printf( "RFR_TEST RFR_TCI.DLL RFR_TCI\\BL_TFT" );
            return;
        }
		else if(strstr(pcParm, ".dll") != NULL)
		{
			memcpy(szModulPath, pcParm, sizeof(szModulPath));
			continue;
		}
		else if(strstr(pcParm, "BL_") != NULL)
		{
			memcpy(szModulBal, pcParm, sizeof(szModulBal));
			continue;
		}
    }	

	/* BAL module LAN (necessaire au module Reference) */
	SIMU.bal_lan = PublieBAL ("BL_LAN" ,NOYAU_BAL_ILLIMITEE);

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

	/* BAL simulateur (qui correspond a une BAL applicative pour module Reference */
	SIMU.bal_simu = PublieBAL ("BL_POSTE" ,NOYAU_BAL_ILLIMITEE);

    /* BAL module COM_LS (necessaire au module Reference) */
	SIMU.bal_msg = PublieBAL ("BL_PCS" ,NOYAU_BAL_ILLIMITEE);

	/* Recuperer les adresses des fonctions de lancement et d'arret */
	if ((hinstance = LoadLibrary ("csr_referenc_vs17_x64.dll")) != NULL)
	{
		pREFLance = (PMOD_PROC_LANCE)GetProcAddress (hinstance, MOD_PROC_LANCE);
		if (pREFLance == NULL)
			return;

		pREFArret = (PMOD_PROC_ARRET)GetProcAddress (hinstance, MOD_PROC_ARRET);
		if (pREFArret == NULL)
			return;
	}
	else // Impossible de charger la DLL
	{
		printf("Problem lance simu:Error:%d\n",GetLastError());
		return;
	}

   // Lancement du module Reference 
    sprintf(szModulRegKey, CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG MOD_REG_KEYn_MODULES "RFR\\BL_RFR");


	if( (result = pREFLance(szModulRegKey,"BL_RFR", &SIMU.bal_rfr) )!= INST_INIT_OK )
   {
      printf("Pb Lancement du module ReFeRence : %d",result);


#ifdef VRAI_HORODATE
	  if (pf_hrd != NULL)
	  {
		HRDFinTrace ();
		fclose (pf_hrd);
	  }
#endif
      exit(result);
   }

	// Lancement du module Reference 
    sprintf(szModulRegKey, CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG MOD_REG_KEYn_MODULES "%s", szModulBal);


   /* Recuperer les adresses des fonctions de lancement et d'arret */
	if ((hinstance = LoadLibrary (szModulPath)) != NULL)
	{
		pMODLance = (PMOD_PROC_LANCE)GetProcAddress (hinstance, MOD_PROC_LANCE);
		if (pMODLance == NULL)
			return;

		pMODArret = (PMOD_PROC_ARRET)GetProcAddress (hinstance, MOD_PROC_ARRET);
		if (pMODArret == NULL)
			return;
	}
	else // Impossible de charger la DLL
	{
		printf("Problem lance simu:Error:%d\n",GetLastError());
		return;
	}
   	// Lancement du module  
   if( (result = pMODLance(
       szModulRegKey,
       "BL_VRN", &void_bal) )!= INST_INIT_OK )
   {
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
	   DelaiTache (5);

/*
 * On termine de la maniere souhaitee par les
 * autres taches.
 */
   pREFArret(SIMU.bal_rfr);
#ifdef VRAI_HORODATE
   HRDArret ();
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












