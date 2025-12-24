/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : pcs_msg_files
 * FILE       : msg_fic_glob.h
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

#ifndef MSG_FIC_GLOB_H
#define MSG_FIC_GLOB_H

/*--------------- INCLUDES: ---------------*/
#include <fic_conf.h>
/*--------------- RESERVED: ---------------*/
#include <protect.h>

/*----------------DEFINES:-----------------*/

/* version */
#define MSG_FIC_VERSION "1.00"
#define MSG_FIC_COMMENT "PCS_MSG_FIC 1.00"

/*--------------- TYPEDEFS: ---------------*/

typedef struct
{
//   char						hostname[MAX_PATH];
//   char						path_distant[MAX_PATH];
   char						path_disk[MAX_PATH];
   char						path_backup[MAX_PATH];
//   char						local_directory[MAX_PATH];
   char						local_drive[MAX_PATH];
}struct_directories;


/*-------------------- variables ---------------- */

// début de section de données partagées initialisées
#include "global.h"
   
	PROTECTED char item[CONF_MAX_ITEM] INIT(0);
	PROTECTED TValeur id INIT(0);
	PROTECTED TValeur valeur INIT(0);
	PROTECTED SYSTEMTIME date INIT(0);
	PROTECTED int Position_fichier_date INIT(0);

// fin de section de données partagées initialisées
#include "global.h"


/*-------------------- fonctions----------------- */

#include <undef.h>
#endif

