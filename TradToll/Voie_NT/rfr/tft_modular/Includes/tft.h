/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : rfr_tft
 * FILE       : tft.h
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

#ifndef RFR_TFT_H
#define RFR_TFT_H

#ifdef TFT_MODULAR_DEF
   #include "public.h"
#else
   #include "export.h"
#endif

/*--------------- CONSTANTS: ---------------*/

#define		  MAX_TFT_ID	 5

/*--------------------Registry---------------*/
#define TFT_BAL_NOM "BL_TFT"
#define TFT_REG_KEYn_ModTFT "RFR_TFT\\"

#define MOD_REG_KEYv_LA_BAL     "bl_la_name"
#define MOD_REG_KEYv_RFR_BAL    "bl_rfr_name"
#define MOD_REG_KEYv_TEMP_PATH	"path_temp_file"
#define MOD_REG_KEYv_OBL		"Obligatoire"	   	
#define MOD_REG_KEYv_DEST_PATH	"path_dest_file"

/*--------------- FONCTIONS ---------------*/
EXPORT enum_instance_result WINAPI MODLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId );
EXPORT enum_instance_result WINAPI MODArret(noyau_bal_id iBalId);

EXPORT enum_instance_result WINAPI TFTLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId );
EXPORT enum_instance_result WINAPI TFTArret(noyau_bal_id iBalId);


#undef PUBLIC
#undef I
#undef INIT
#endif
