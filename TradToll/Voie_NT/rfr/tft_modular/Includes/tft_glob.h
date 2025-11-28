/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : rfr_tft
 * FILE       : tft_glob.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef TFT_GLOB_H
#define TFT_GLOB_H

#include <protect.h>
/*--------------- DEFINES ---------------*/
#define RFR_TFT_NB_TACHE		1

/* pour les traces du module */
enum rfr_index_traces
{
   RFR_TFT_TRC,
   RFR_TFT_NB_TRACES
};

#define RfrTftFichierDebug  RFR_TFT_FILE=__FILE__,\
							RFR_TFT_LINE=__LINE__,\
							DEFINE_RfrTftFichierDebug


typedef struct
{
	noyau_priorite_tache		priorite_max;	// reg data
	noyau_priorite_tache		priorite_init;	// reg data
	noyau_pool_id				pool;			// reg data
	
	char						rfr_tft_bal_name[MAX_PATH];
	noyau_bal_id				rfr_tft_bal_id;

	char						csr_tft_bal_name[MAX_PATH + 1];
	noyau_bal_id				csr_tft_bal_id;

	char						NewFiles[MAX_TFT_SIZE + 1];

	dbg_struct_debug			dbg;
	dbg_struct_trace			tab_traces[RFR_TFT_NB_TRACES];

	struct_tache				taches[RFR_TFT_NB_TACHE + 1];
}
struct_rfr_tft_glob;

/*--------------- VARIABLES ---------------*/
/* pour le mode DEBUG */
PROTECTED char *RFR_TFT_FILE;
PROTECTED int RFR_TFT_LINE;

PROTECTED struct_rfr_tft_glob RFR_TFT;

/*------------------FUNCTIONS:-----------------*/
PROTECTED DWORD WINAPI RfrTft(void * pvFoo);

PROTECTED enum_instance_result RfrTftInitTrace( char * pcBal );
PROTECTED void RfrTftFichierTrace (char *fmt,...);
PROTECTED void DEFINE_RfrTftFichierDebug (char *fmt,...);

PROTECTED void EnvoiCsrTft(int type);

#undef PUBLIC
#undef I
#undef INIT
#endif
