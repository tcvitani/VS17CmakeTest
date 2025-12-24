/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_serv.h                                                  */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef EMI_PIC_SERV_H
#define EMI_PIC_SERV_H
/*-------------------------------- INCLUDES:  -------------------------------*/
/*-------------------------------- RESERVED:  -------------------------------*/
#include <protect.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PROTECTED void EmiPicReceptionEmi(short siInstId,
                                  struct_emi_pic_message *pMessage);
PROTECTED void EmiPicReceptionRestit(short siInstId,
                                  struct_emi_pic_message *pMessage);
PROTECTED void EmiPicReceptionTrf(short siInstId,
                                  struct_emi_pic_message *pMessage);
PROTECTED void EmiPicReceptionArret(short siInstId,
                                  struct_emi_pic_message *pMessage);
PROTECTED void EmiPicReceptionEtat( short siInstId,
								   struct_emi_pic_message *pMessage);
PROTECTED void EmiPicReceptionMaintMode( short siInstId,
										struct_emi_pic_message *pMessage);
/*-------------------------------- VARIABLES: -------------------------------*/
#include <undef.h>
#endif
/*-------------------------------- END OF FILE ------------------------------*/