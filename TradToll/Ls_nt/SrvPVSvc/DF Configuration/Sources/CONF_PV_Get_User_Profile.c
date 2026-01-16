/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : DB  
 * FILE       : CONF_PV_Get_User_Profile.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Module principal du service
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h> 
#include <dbif.h>
#include <ntsvc.h>
#include <conf_srvpv.h>
#include <conf_define.h>
#include <Db_connection.h>

// ------------ INTERNALS --------------------

#include <gen.h>
#include <conf_ini.h>

#include <memclass.h>

// ------------- VARIABLES -------------------

// -------------- DEFINES --------------------
 
// CONF GARE

PRIVATE struct 
{
 DWORD dwConfIndex;
 DB_STMT * hStat;
 DB_VAR * hName_in;
 DB_VAR * hPassword;
 DB_VAR * hName_out;
 DB_VAR * hMatricule;
 DB_VAR * hFirstname;
 DB_CNX * hDbCnx;
} Profile;

// --------------- CODE ----------------------

// CONFIGURATION DE LA GARE ET DES VOIES

PROTECTED BOOL CONF_PV_Get_User_Profile (IN char * name_in,
                                         IN DWORD matricule_in,
                                         IN char * password,
                                         OUT char * name_out,
                                         OUT DWORD * matricule_out,
                                         OUT char * firstname)
{
    void * pvResult;         // Pour récuperer un pointeur sur la valeur d'un bind
    char szErr[200] = "";    // Texte d'erreur
    DWORD dwErr;
	CHAR szStatement[1024];    // Pour construire l'appel de la fonction stockée
    BOOL bRet;
    
    bRet = FALSE;
    
    return bRet;
}
