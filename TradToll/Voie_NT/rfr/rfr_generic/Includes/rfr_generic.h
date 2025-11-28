/* --------------------------------------------------------------------
 * (C) 2015 SANEF ITS - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : rfr_generic
 * FILE       : rfr_generic.h
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

#ifndef RFR_GENERIC_H
#define RFR_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

// standard csroute definitions
#include <csrlc32.h>

#ifdef RFR_GENERIC_EXPORTS
	#include <public.h>
#else
	#include <export.h>
#endif


/*--------------- CONSTANTS: ---------------*/

#define		  MAX_RFR_GEN_ID	 5
 	
/*--------------------Registry---------------*/
#define MOD_REG_KEYv_LA_BAL			"bl_la_name"
#define MOD_REG_KEYv_RFR_BAL		"bl_rfr_name"
#define MOD_REG_KEYv_TEMP_PATH		"path_temp_file"
#define MOD_REG_KEYv_OBL			"Obligatoire"	   	
#define MOD_REG_KEYv_ALLOW_EMPTY	"AllowEmptyFile"	   	
#define MOD_REG_KEYv_FILE_ID		"FileID"
#define MOD_REG_KEYv_ITERABLE		"Iterable"
/*--------------- FONCTIONS ---------------*/
EXPORT enum_instance_result WINAPI MODLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId );
EXPORT enum_instance_result WINAPI MODArret(noyau_bal_id iBalId);

EXPORT enum_instance_result WINAPI RfrGenericLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId );
EXPORT enum_instance_result WINAPI RfrGenericArret(noyau_bal_id iBalId);

#ifdef __cplusplus
}
#endif

#endif
