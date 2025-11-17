/* --------------------------------------------------------------------
 * (C) 1998 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : runner
 * FILE       : runner.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Definitions communes aux modules CSR. Tout module doit
 *   definir imperativement les deux fonctions suivantes :
 *      EXPORT enum_instance_result WINAPI MODLance(
 *                 IN char * pcKey,
 *                 IN char * pcBalName,
 *                 OUT noyau_bal_id * piBalId );
 *      EXPORT enum_instance_result WINAPI MODArret( IN noyau_bal_id iBalId );
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef RUNNER_H
#define RUNNER_H


// Standard Microsoft definitions
#include <WINDOWS.H>

// CS Route specific includes
#include <noyau.h>   // Pour le type noyau_bal_id


// -----------------------------------------------
// CODES DE RETOURS DES POINTS D'ENTREE MODULE
// -----------------------------------------------
typedef enum
{
    // Codes de retour sur lancement
    FIRST_INST_INIT_CODE = 0,

    INST_INIT_OK            = FIRST_INST_INIT_CODE,
    INST_INIT_ERR_LANCE,
    INST_INIT_ERR_FICHIER_PARAM,
    INST_INIT_ERR_PORT,
    INST_INIT_ERR_FICHIER_DEBUG,
    INST_INIT_ERR_MAX_INSTANCE,
    INST_INIT_ERR_REGISTRE,
    INST_INIT_ERR_CHRONO,
    INST_INIT_ERR_AUTOMATE,
    INST_INIT_ERR_EVENT,
    INST_INIT_ERR_BAL,

    BUTEE_INST_INIT_CODE,
    LAST_INST_INIT_CODE = BUTEE_INST_INIT_CODE - 1,
    NB_INST_INIT_CODE = LAST_INST_INIT_CODE - FIRST_INST_INIT_CODE + 1,

    // Codes de retour sur arret
    FIRST_INST_ARRET_CODE = 0,

    INST_ARRET_OK           = 0,
    INST_ARRET_NOK,
    INST_ARRET_ERR_RESSOURCE,
    INST_ARRET_ERR_AUTOMATE,
    INST_ARRET_ERR_PORT,
    INST_ARRET_ERR_FICHIER_DEBUG,
    INST_ARRET_ERR_TACHE,

    BUTEE_INST_ARRET_CODE,
    LAST_INST_ARRET_CODE = BUTEE_INST_ARRET_CODE - 1,
    NB_INST_ARRET_CODE = LAST_INST_ARRET_CODE - FIRST_INST_ARRET_CODE + 1,

    NB_INST_CODE = NB_INST_INIT_CODE + NB_INST_ARRET_CODE  // total codes
}
enum_instance_result;

// -----------------------------------------------
// MACROS ET TYPES DEFINISSANT LES POINTS D'ENTREE DANS LES MODULES
// -----------------------------------------------

// Lancement de module
#define MOD_PROC_LANCE "MODLance"
typedef enum_instance_result (WINAPI *PMOD_PROC_LANCE)( IN char * pcKey, IN char * pcBalName, OUT noyau_bal_id * piBalId );

// Arret de module
#define MOD_PROC_ARRET "MODArret"
typedef enum_instance_result (WINAPI *PMOD_PROC_ARRET)( IN noyau_bal_id iBalId );





#endif // RUNNER_H

/* -------------------- END OF FILE --------------------- */