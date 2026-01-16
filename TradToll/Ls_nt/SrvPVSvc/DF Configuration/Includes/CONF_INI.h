/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     CONF_INI.h														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef CONF_INI_H
#define CONF_INI_H

/*-------------------------------- INCLUDES:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/

#include "protect.h"

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


PROTECTED BOOL INI_Recupere(IN char *fichier,
							IN DWORD num_section, 
							IN DWORD nb,
							IN OUT struct tab TabInfo[]);

PROTECTED DWORD INI_Recherche(IN char *fichier, 
							OUT DWORD *pt_num_section,
							IN DWORD nb_critere, 
							IN OUT struct tab TabCritere[]);

PROTECTED BOOL INI_Initialize(void);

PROTECTED char * INI_Get_Files_Path(void);

PROTECTED VOID INI_Resultat_ZeroMemory(IN OUT struct resul Resultat[], IN DWORD Taille);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/

