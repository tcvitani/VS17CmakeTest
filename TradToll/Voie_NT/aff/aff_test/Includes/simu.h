/********************************************
*  PROGRAM: simu.h
*  PURPOSE: function definitions of simu.c
*           functions
/********************************************

/*--------------- INCLUDES: ---------------*/

#include <csrlc32.h>
#include <run.h>

#include "protect.h"

#ifndef SIMU_H
#define SIMU_H

/*--------------- RESERVED: ---------------*/

/*--------------- EXTERNALS: --------------*/

/*--------------- DEFINES: ----------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: --------------*/

PROTECTED int LanceModule(HWND hDlg, char *p_MbName);
PROTECTED int ArretModule(HWND hDlg);
PROTECTED void simu_envoi_aff (noyau_bal_id bal,short int service, short int type_message);

/*--------------- VARIABLES: ---------------*/
#endif
