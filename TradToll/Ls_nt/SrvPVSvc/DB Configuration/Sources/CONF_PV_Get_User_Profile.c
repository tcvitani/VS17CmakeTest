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
    
	//recuperation du pointeur de connexion
	Profile.hDbCnx = DB_CONNECTION_Open();

	if (Profile.hDbCnx == NULL)
	{
		NTSVCInfo( "CONF_PV_Get_User_Profile(), erreur connexion à la base impossible");
		return FALSE;
	}

	// Boucle do while() utilisée pour la commodité du break. En fait, la condition
    // de bouclage est FALSE, on ne passe donc qu'une fois dedans.
    bRet = FALSE;
    
	do 
    {
		// NTSVCInfo( "CONF_PV_Get_User_Profile(), préparation de la requête CONF_PV_get_User_Profile()" );

		// Préparer la requête
        _snprintf (szStatement, sizeof(szStatement), 
            "BEGIN PV.PV_DB_Get_User_Profile( :Name_in, %d, :Password, :Name_out, :Matricule, :Firstname ); END;", matricule_in );


		Profile.hStat = DBOpenStatement( Profile.hDbCnx, szStatement);

		if (Profile.hStat == NULL)
		{
            DBGetLastError( Profile.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "CONF_PV_Get_User_Profile(), erreur %u, statement CONF_PV_get_user_profile() : %s", dwErr, szErr );
            break;
		}

		// Associer les variables

		Profile.hName_in = DBBindPlaceHolderVariable( 
                Profile.hStat, 
                ":Name_in", 
                DB_TYPE_STR, 
                CONF_MAX_USER_NAME, 
                0 );

		if ( Profile.hName_in == NULL )
		{
            DBGetLastError( Profile.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "CONF_PV_Get_User_Profile(), erreur %u, bind Name_in : %s", dwErr, szErr );
            break;
        }

		Profile.hPassword = DBBindPlaceHolderVariable( 
                Profile.hStat, 
                ":Password", 
                DB_TYPE_STR, 
                CONF_MAX_USER_PASSWORD, 
                0 );

		if ( Profile.hPassword == NULL )
		{
            DBGetLastError( Profile.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "CONF_PV_Get_User_Profile(), erreur %u, bind Password : %s", dwErr, szErr );
            break;
        }
		
		Profile.hName_out = DBBindPlaceHolderVariable( 
                Profile.hStat, 
                ":Name_out", 
                DB_TYPE_STR, 
                CONF_MAX_USER_NAME, 
                0 );

		if ( Profile.hName_out == NULL )
		{
            DBGetLastError( Profile.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "CONF_PV_Get_User_Profile(), erreur %u, bind Name_out : %s", dwErr, szErr );
            break;
        }

		Profile.hMatricule = DBBindPlaceHolderVariable( 
                Profile.hStat, 
                ":Matricule", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );

		if ( Profile.hMatricule == NULL )
		{
            DBGetLastError( Profile.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "CONF_PV_Get_User_Profile(), erreur %u, bind Matricule : %s", dwErr, szErr );
            break;
        }

		Profile.hFirstname = DBBindPlaceHolderVariable( 
                Profile.hStat, 
                ":Firstname", 
                DB_TYPE_STR, 
                CONF_MAX_USER_FIRSTNAME, 
                0 );

		if ( Profile.hFirstname == NULL )
		{
            DBGetLastError( Profile.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "CONF_PV_Get_User_Profile(), erreur %u, bind Fistname : %s", dwErr, szErr );
            break;
        }


		Profile.dwConfIndex = 0;

		pvResult = DBSetVariableItemValue( Profile.hName_in, Profile.dwConfIndex, name_in, strlen(name_in) + 1 );
        
		if ( pvResult != name_in )
            break;

		pvResult = DBSetVariableItemValue( Profile.hPassword, Profile.dwConfIndex, password, strlen(password) + 1 );
        
		if ( pvResult != password )
            break;


        // Exécuter la requète
        if ( ! DBExecuteStatement( Profile.hStat ) )
        {
            DBGetLastError( Profile.hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "CONF_PV_Get_User_Profile(), erreur %u, exécute CONF_PV_get_user_Profile : %s", dwErr, szErr );
            break;
        }

        // Récupérer les résultats
        Profile.dwConfIndex = 0;
        
		pvResult = DBGetVariableItemValue( Profile.hName_out, Profile.dwConfIndex, name_out, NULL );
        
		if ( pvResult != name_out )
            break;

		pvResult = DBGetVariableItemValue( Profile.hMatricule, Profile.dwConfIndex, matricule_out, NULL );
        
		if ( pvResult != matricule_out )
            break;

		pvResult = DBGetVariableItemValue( Profile.hFirstname, Profile.dwConfIndex, firstname, NULL );
        
		if ( pvResult != firstname )
            break;

        NTSVCInfo( "CONF_PV_Get_User_Profile(), name = %s, matricule = %d, firstname = %s", name_out, *matricule_out, firstname);
        
        bRet = TRUE;
    }
    while ( FALSE );
    
    if ( Profile.hDbCnx != NULL )
    {
        // NTSVCInfo( "CONF_PV_Get_User_Profile(), déconnexion de la base" );
        DBCloseStatement(Profile.hStat);
        DB_CONNECTION_Close();
		Profile.hDbCnx = NULL;
    }

    return bRet;
}
