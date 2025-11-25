/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     TEST.H                                                          */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_LC_TEST_H
#define MSG_LC_TEST_H

/*-------------------------------- INCLUDES:  -------------------------------*/


/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_LC_TEST_EXPORTS
	#include "public.h"
#else
	#include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/

#include "memclass.h"

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED void Test_Payment(void);
PROTECTED void Test_Tab_Ack(void);
PROTECTED void Test_Bowl_Movement(void);
PROTECTED void Test_Bowl_Stat(void);
PROTECTED void Test_End_Shift(void);
PROTECTED void Test_Event(void);
PROTECTED void Test_Start_Shift(void);
PROTECTED void Test_Vault_Ins(void);
PROTECTED void Test_Status(void);
PROTECTED void Test_Transaction(void);
PROTECTED void Test_Vault_Witdraw(void);
PROTECTED void Test_Vault_Stat(void);
PROTECTED void Test_Comp_Inf_Tr(void);
PROTECTED void Test_Perm_Rq_Send_File(void);
PROTECTED void Test_Reference(void);
PROTECTED void Test_Perm_Send_File(void);
PROTECTED void Test_Backup_Date(void);
PROTECTED void Test_Backup_File(void);
PROTECTED void Test_Command(void);
PROTECTED void Test_Hourly(void);
PROTECTED void Test_VarStr(void);

/*-------------------------------- VARIABLES: -------------------------------*/

#include <undef.h>
#endif

/*-------------------------------- END OF FILE ------------------------------*/