/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: 
* FICHIER: NOY_DBG.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE: 
* --------------------------------------------------------------------
* RESUME: 
* --------------------------------------------------------------------
* DESCRIPTION: 
* --------------------------------------------------------------------
* HISTORIQUE: 
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef NOY_DBG_H
#define NOY_DBG_H

/*--------------- INCLUDES: ---------------*/
#include "debug.h"

/*--------------- RESERVED: ---------------*/

#include <protect.h>

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/
#define RtcFichierDebug DBG_EcritFichierErreurs

#define RtcFichierTrace DBG_EcritFichierTraces

#define FMT_STATUS " { Status %ld }"

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

/* lance le debug du noyau */
PROTECTED noyau_enum_retour LanceDebugNoyau (dbg_struct_debug *debug);
PROTECTED noyau_enum_retour ArretDebugNoyau (dbg_struct_debug *debug);

/* teste les piles et les blocs allou‚s */
PROTECTED void TestsPeriodiques (void);

#undef I
#undef INIT
#undef PROTECTED
#endif /* NOY_DBG_H */
