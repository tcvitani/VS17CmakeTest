/* --------------------------------------------------------------------
 * (C) 1998 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CSR
 * FILE       : csrlc32.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : common
 * --------------------------------------------------------------------
 * SUMMARY    : Common definitions
 * --------------------------------------------------------------------
 * DESCRIPTION: This file contains common definitions for CS Route
 *              applications.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CSRLC32_H
#define CSRLC32_H

// Standard Microsoft definitions
#include <WINDOWS.H>

 
// **********
// * Define *
// **********

#define	CSR_OK						ERROR_SUCCESS		// ERROR_SUCCESS CS Route !


/* i = identifier, n = name, v = value */
#define CSR_REG_KEYi_ROOT			HKEY_LOCAL_MACHINE

#define CSR_REG_KEYn_CSRBASE		"SOFTWARE\\CSRoute\\"
#define CSR_REG_KEYn_LANE_BASE		"LaneController\\"
#define CSR_REG_KEYn_MAINTENANCE	"Maintenance\\"

// Main Level Keys
// ===============
#define CSR_REG_KEYn_DYNAMIC		"Dynamic\\"
#define CSR_REG_KEYn_PROJECT		"Project\\"
#define CSR_REG_KEYn_CONFIG			"Config\\"
#define CSR_REG_KEYn_COLLECTOR		"Collector\\"
#define CSR_REG_KEYn_LANGUAGES		"Languages\\"

// General configuration registry values
// =====================================

// Values in CSR_REG_KEYn_DYNAMIC
#define CSR_REG_KEYn_RUNDATA		"RunTimeData\\"

// Values in CSR_REG_KEYn_PROJECT

// Values in CSR_REG_KEYn_CONFIG
#define CSR_REG_KEYv_APPPATH		"ApplicationPath"  // ZBR
#define CSR_REG_KEYv_ERRORPATH		"ErrorPath"
#define CSR_REG_KEYv_TRACEPATH		"TracePath"
#define CSR_REG_KEYv_FILEMAXSIZE	"FileMaxSize"

// Values in CSR_REG_KEYn_COLLECTOR

// Values in CSR_REG_KEYn_MAINTENANCE

// Values in CSR_REG_KEYn_LANGUAGES

// -----------------------------------------------
// MACROS UTILISEES POUR ACCES AUX CLES DU REGISTRES
// COMMUNES AUX MODULES
// -----------------------------------------------

// Sous-cle parametrage modules
#define MOD_REG_KEYn_MODULES		"Modules\\"

// Valeurs de parametrage des modules
#define MOD_REG_KEYv_ID             "_Id"
#define MOD_REG_KEYv_PORT           "_Port"
#define MOD_REG_KEYv_PARAMPORT      "_ParamPort"
#define MOD_REG_KEYv_PROTOCOL       "_ProtocolLayer"
#define MOD_REG_KEYv_INTERFACE      "_InterfaceType"
#define MOD_REG_KEYv_PRIO_INIT      "PriorityInit"
#define MOD_REG_KEYv_PRIO_MAX       "PriorityMax"
#define MOD_REG_KEYv_POOL           "Pool"

#endif			// CSRLC32_H