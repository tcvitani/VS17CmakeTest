/*------   (v) 1996 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SERIE
* FICHIER: PCL_LOC.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: prototypage des fonctions protegees du module serie
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface local
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log : $
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef SER_LOC_H
#define SER_LOC_H

/*--------------- INCLUDES: ---------------*/
#include <windows.h>
#include <stdarg.h>
#include <noyau.h>
#include <debug.h>

#include <protect.h>

/*--------------- DEFINES: ----------------*/

/* version */
#define SER_VERSION "2.0.0"
#define SER_COMMENT "NOYAU 2.0.0"

/* pour les traces du module */
enum index_traces
{
   SER_TRC,
   SER_NB_TRACES
};

typedef struct 
{
    BOOL fAllowOpenError;
    DWORD dwLastAttemptTick;
   HANDLE hComm;
   char mode_command_line[1024];
   DCB dcb;
   COMMTIMEOUTS timeouts;
   dbg_struct_debug dbg;
   dbg_struct_trace tab_traces[SER_NB_TRACES];
}
Ser_struct_port;
 
typedef struct 
{
   Ser_struct_port port[SER_NB_PORTS_MAX + 1];
}
Ser_struct_locale;

/*--------------- FUNCTIONS: ---------------*/

/*--------------- VARIABLES: ---------------*/

// cette structure de donnes n'est plus partage entre les process
// car il y a des pb de ports deja ouverts sur redmarrage d'applications...
// pas d'explications mais ca reste une solution au dtriment du test des ports
// rouverts...
PROTECTED Ser_struct_locale _SER_ INIT(0);


PROTECTED DWORD _RetryOpenDelay_ INIT(0);


/*
#include <global.h>
   PROTECTED WORD SER_NB_INSTANCES INIT(0);
#include <global.h>
*/

#undef PROTECTED
#undef I
#undef INIT
#endif




 
 