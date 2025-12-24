/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_mess.h                                                  */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef EMI_PIC_MESS_H
#define EMI_PIC_MESS_H
/*-------------------------------- INCLUDES:  -------------------------------*/
/*-------------------------------- RESERVED:  -------------------------------*/
#include <protect.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PROTECTED enum_emi_pic_type EmiPicDebutService(short siInstId,
                              enum_emi_pic_service eService,
                              noyau_bal_id dwBalDest);
PROTECTED enum_emi_pic_type EmiPicFinService(short siInstId,
                              enum_emi_pic_service eService,
                              noyau_bal_id dwBalDest);
PROTECTED void EmiPicEnvoiAcquittement(short siInstId,
                              noyau_bal_id dwBalDest,
                              enum_emi_pic_service eService,
                              enum_emi_pic_type eTypeMessage);
PROTECTED void EmiPicEnvoiRestit(short siInstId,
                              struct_emi_pic_int_message *pMessage,
                              enum_emi_pic_type eTypeMessage);
PROTECTED void EmiPicEnvoiTrf(short siInstId,
                              struct_emi_pic_int_message *pMessage,
                              enum_emi_pic_type eTypeMessage);
PROTECTED void EmiPicEnvoiEmi(short siInstId,
                              struct_emi_pic_int_message *pMessage,
                              enum_emi_pic_type eTypeMessage);
PROTECTED void EmiPicAniEnvoiEmi(short siInstId,
                              enum_emi_pic_internal_type eTypeMessage,
                              void *pvParam);
PROTECTED void EmiPicAniEnvoiImport( short siInstId,
									enum_emi_pic_internal_type eTypeMessage,
									void *pvParam );
PROTECTED void EmiPicAniEnvoiRestit(short siInstId,
                              enum_emi_pic_internal_type eTypeMessage,
                              void *pvParam);
PROTECTED void EmiPicAniEnvoiTrf(short siInstId,
                              enum_emi_pic_internal_type eTypeMessage,
                              void *pvParam);
PROTECTED void EmiPicEnvoiEtat(short siInstId,
							   struct_emi_pic_etat *pMessage,
							   enum_emi_pic_type eTypeMessage);
PROTECTED void EmiPicEnvoiMaintMode( short siInstId,
									 struct_emi_pic_int_message *pMessage,
									 enum_emi_pic_type eTypeMessage);
PROTECTED void EmiPicLibereBAL( noyau_bal_id dwBal );
/*-------------------------------- VARIABLES: -------------------------------*/
#include <undef.h>
#endif
/*-------------------------------- END OF FILE ------------------------------*/