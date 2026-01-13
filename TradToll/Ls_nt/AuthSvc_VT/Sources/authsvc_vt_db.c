/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : base de données
 * --------------------------------------------------------------------
 * SUMMARY    : Module d'interface avec la base de données
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
#include <col.h>
#include <csr_msg.h>
#include <acom.h>
#include <ntsvc.h>
#include <csr_list.h>
#include <reg.h>

#include <authsvc_vt_glob.h>
#include <authsvc_vt_main.h>
#include <csr_msg.h>
#include <msg_lc_auth_vt_enl_rep.h>
#include <msg_lc_auth_vt_enl_req.h>
#include <msg_lc_auth_vt_exl_rep.h>
#include <msg_lc_auth_vt_exl_req.h>
#include <msg_lc_auth_vt_enl_rep_v2.h>
#include <msg_lc_auth_vt_enl_req_v2.h>
#include <msg_lc_auth_vt_exl_rep_v2.h>
#include <msg_lc_auth_vt_exl_req_v2.h>
#include <msg_lc_auth_vt_enl_rep_v3.h>
#include <msg_lc_auth_vt_enl_req_v3.h>
#include <msg_lc_auth_vt_exl_rep_v3.h>
#include <msg_lc_auth_vt_exl_req_v3.h>

#define LOC_DEF
#include <authsvc_vt_db.h>
#undef LOC_DEF

#include <memclass.h>

//PROCEDURE VT_ENL_InsertPassage(p_Dte_Ref_Entry          IN     VARCHAR2,
//	p_Ref_Entry_Trs 	IN 		INTEGER,
//	p_Id_Entry_Plaza     IN     INTEGER,
//	p_Id_Entry_Lane      IN     INTEGER,
//	p_Id_Entry_Trs       IN     VARCHAR2,
//	p_Dte_Entry          IN     VARCHAR2,
//	p_Entry_VRN          IN     VARCHAR2,
//	p_Entry_VRN_Country  IN     VARCHAR2,
//	p_Id_Entry_Provider  IN     VARCHAR2,
//	p_Result             OUT	INTEGER,
//	p_id_Reason			 OUT	INTEGER)


#define DB_DEFAULT_ENL_Insert_REQUEST \
       "BEGIN AUTH_VT.VT_ENL_InsertPassage(" \
			":p_Dte_Ref_Entry, :p_Ref_Entry_Trs, :p_Id_Entry_Plaza, :p_Id_Entry_Lane, :p_Id_Entry_Trs, :p_Dte_Entry, :p_Entry_VRN, :p_Entry_VRN_Country, :p_Id_Entry_Provider, :p_Result, :p_id_Reason ); COMMIT; END;"





// PROCEDURE VT_ENL_InsertPassage_V2(p_Dte_Ref_Entry       IN     VARCHAR2,
// p_Ref_Entry_Trs       IN     INTEGER,
// p_Id_Entry_Plaza      IN     INTEGER,
// p_Id_Entry_Lane       IN     INTEGER,
// p_Id_Entry_Trs        IN     VARCHAR2,
// p_Dte_Entry           IN     VARCHAR2,
// p_Entry_VRN           IN     VARCHAR2,
// p_Entry_VRN_Country   IN     VARCHAR2,
// p_Ticket_Type         IN     INTEGER,
// p_Id_Entry_Provider   OUT    VARCHAR2,
// p_Id_Payment_SubType  OUT    INTEGER,
// p_Id_Class            OUT    INTEGER,
// p_Accepted_Days       OUT    VARCHAR2,
// p_Result              OUT    INTEGER,
// p_Id_Reason           OUT    INTEGER) 


#define DB_DEFAULT_ENL_Insert_REQUEST_V2 \
       "BEGIN AUTH_VT.VT_ENL_InsertPassage_V2(" \
			":p_Dte_Ref_Entry, :p_Ref_Entry_Trs, :p_Id_Entry_Plaza, :p_Id_Entry_Lane, :p_Id_Entry_Trs, :p_Dte_Entry, :p_Entry_VRN, :p_Entry_VRN_Country, :p_Ticket_Type, \
			:p_Id_Entry_Provider, :p_Id_Payment_SubType, :p_Id_Class, :p_Accepted_Days, :p_Result, :p_id_Reason ); COMMIT; END;"





// PROCEDURE VT_ENL_InsertPassage_V3(p_Dte_Ref_Entry       IN     VARCHAR2,
// p_Ref_Entry_Trs       IN     INTEGER,
// p_Id_Entry_Plaza      IN     INTEGER,
// p_Id_Entry_Lane       IN     INTEGER,
// p_Id_Entry_Trs        IN     VARCHAR2,
// p_Dte_Entry           IN     VARCHAR2,
// p_Entry_VRN           IN     VARCHAR2,
// p_Entry_VRN_Country   IN     VARCHAR2,
// p_Ticket_Type         IN     INTEGER,
// p_Flag_Delete		 IN		INTEGER,
// p_Flag_No_Insert		 IN		INTEGER,
// p_Id_Entry_Provider   OUT    VARCHAR2,
// p_Id_Contract_Type	 OUT	INTEGER,
// p_Id_Payment_SubType  OUT    INTEGER,
// p_Id_Class            OUT    INTEGER,
// p_Accepted_Days       OUT    VARCHAR2,
// p_Account_Balance	 OUT	INTEGER,
// p_User_Balance		 OUT	INTEGER,
// p_Result              OUT    INTEGER,
// p_Id_Reason           OUT    INTEGER) 


#define DB_DEFAULT_ENL_Insert_REQUEST_V3 \
       "BEGIN AUTH_VT.VT_ENL_InsertPassage_V3(" \
			":p_Dte_Ref_Entry, :p_Ref_Entry_Trs, :p_Id_Entry_Plaza, :p_Id_Entry_Lane, :p_Id_Entry_Trs, :p_Dte_Entry, :p_Entry_VRN, :p_Entry_VRN_Country, :p_Ticket_Type, :p_Flag_Delete, :p_Flag_No_Insert, \
			:p_Id_Entry_Provider, :p_Id_Contract_Type, :p_Id_Payment_SubType, :p_Id_Class, :p_Accepted_Days, :p_Account_Balance, :p_User_Balance, :p_Result, :p_id_Reason ); COMMIT; END;"





//PROCEDURE VT_EXL_UpdatePassage(
//	p_Dte_Ref_Exit       IN     VARCHAR2,
//	p_Ref_Exit_Trs 		 IN 	INTEGER,
//	p_Id_Exit_Plaza      IN     INTEGER,
//	p_Id_Exit_Lane       IN     INTEGER,
//	p_Id_Exit_Trs        IN     VARCHAR2,
//	p_Dte_Exit           IN     VARCHAR2,
//	p_Exit_VRN           IN     VARCHAR2,
//	p_Exit_VRN_Country   IN     VARCHAR2,
//	p_Id_Exit_Provider   IN     VARCHAR2,
//	p_Flag_ETicket  	 IN     INTEGER,
//	p_Dte_Ref_Entry      OUT    VARCHAR2,
//	p_Ref_Entry_Trs 	 OUT 	INTEGER,
//	p_Id_Entry_Plaza     OUT    INTEGER,
//	p_Id_Entry_Lane      OUT    INTEGER,
//	p_Id_Entry_Trs       OUT    VARCHAR2,
//	p_Dte_Entry          OUT    VARCHAR2,
//	p_Entry_VRN          OUT    VARCHAR2,
//	p_Entry_VRN_Country  OUT    VARCHAR2,
//	p_Id_Entry_Provider  OUT    VARCHAR2,
//	p_Flag_Result        OUT 	INTEGER,
//	p_Id_Reason          OUT 	INTEGER)


#define DB_DEFAULT_EXL_Update_REQUEST \
	    "BEGIN AUTH_VT.VT_EXL_UpdatePassage (" \
			":p_Dte_Ref_Exit, :p_Ref_Exit_Trs, :p_Id_Exit_Plaza, :p_Id_Exit_Lane, :p_Id_Exit_Trs, :p_Dte_Exit, :p_Exit_VRN, :p_Exit_VRN_Country, :p_Id_Exit_Provider, :p_Flag_ETicket, " \
			":p_Dte_Ref_Entry, :p_Ref_Entry_Trs, :p_Id_Entry_Plaza, :p_Id_Entry_Lane, :p_Id_Entry_Trs, :p_Dte_Entry, :p_Entry_VRN, :p_Entry_VRN_Country, :p_Id_Entry_Provider, :p_Flag_Result, :p_Id_Reason); COMMIT; END;"





//  PROCEDURE VT_EXL_UpdatePassage_V2
// ( p_Dte_Ref_Exit		IN		VARCHAR2,
// p_Ref_Exit_Trs		IN		INTEGER,
// p_Id_Exit_Plaza		IN		INTEGER,
// p_Id_Exit_Lane		IN		INTEGER,
// p_Id_Exit_Trs		IN		VARCHAR2,
// p_Dte_Exit			IN		VARCHAR2,
// p_Exit_VRN			IN		VARCHAR2,
// p_Exit_VRN_Country	IN		VARCHAR2,
// p_Ticket_Type		IN		INTEGER,
// p_Id_Exit_Provider	OUT		VARCHAR2,
// p_Flag_ETicket		IN		INTEGER,
// p_Dte_Ref_Entry		OUT		VARCHAR2,
// p_Ref_Entry_Trs		OUT		INTEGER,
// p_Id_Entry_Plaza		OUT		INTEGER,
// p_Id_Entry_Lane		OUT		INTEGER,
// p_Id_Entry_Trs		OUT		VARCHAR2,
// p_Dte_Entry			OUT		VARCHAR2,
// p_Entry_VRN			OUT		VARCHAR2,
// p_Entry_VRN_Country	OUT		VARCHAR2,
// p_Id_Entry_Provider	OUT		VARCHAR2,
// p_Id_Payment_SubType OUT		INTEGER,
// p_Id_Class			OUT		INTEGER,
// p_Accepted_Days		OUT		VARCHAR2,
// p_Accoun_No			OUT		INTEGER,
// p_User_No			OUT		INTEGER,
// p_Renewal_No			OUT		INTEGER,
// p_Flag_Result		OUT		INTEGER,
// p_Id_Reason			OUT		INTEGER) IS


#define DB_DEFAULT_EXL_Update_REQUEST_V2 \
	    "BEGIN AUTH_VT.VT_EXL_UpdatePassage_V( :p_Dte_Ref_Exit, :p_Ref_Exit_Trs, :p_Id_Exit_Plaza, :p_Id_Exit_Lane, :p_Id_Exit_Trs, :p_Dte_Exit, :p_Exit_VRN, :p_Exit_VRN_Country, :p_Ticket_Type, :p_Id_Exit_Provider, :p_Flag_ETicket," \
			":p_Dte_Ref_Entry, :p_Ref_Entry_Trs, :p_Id_Entry_Plaza, :p_Id_Entry_Lane, :p_Id_Entry_Trs, :p_Dte_Entry, :p_Entry_VRN, :p_Entry_VRN_Country, \
			:p_Id_Entry_Provider, :p_Id_Payment_SubType, :p_Id_Class, :p_Accepted_Days, :p_Accoun_No, :p_User_No, :p_Renewal_No, :p_Flag_Result, : p_Id_Reason); COMMIT; END; "





// 	PROCEDURE VT_EXL_UpdatePassage_V3
// 	( p_Dte_Ref_Exit		IN		VARCHAR2,
// 	 p_Ref_Exit_Trs			IN		INTEGER,
// 	 p_Id_Exit_Plaza		IN		INTEGER,
// 	 p_Id_Exit_Lane			IN		INTEGER,
// 	 p_Id_Exit_Trs			IN		VARCHAR2,
// 	 p_Dte_Exit				IN		VARCHAR2,
// 	 p_Exit_VRN				IN		VARCHAR2,
// 	 p_Exit_VRN_Country		IN		VARCHAR2,
// 	 p_Ticket_Type			IN		INTEGER,
// 	 p_Id_Exit_Provider		OUT		VARCHAR2,
// 	 p_Flag_ETicket			IN		INTEGER,
// 	 p_Flag_Delete			IN		INTEGER,
// 	 p_Flag_Open_System		IN		INTEGER,
//	 p_Flag_No_Update		IN		INTEGER,
// 	 p_Dte_Ref_Entry		OUT		VARCHAR2,
// 	 p_Ref_Entry_Trs		OUT		INTEGER,
// 	 p_Id_Entry_Plaza		OUT		INTEGER,
// 	 p_Id_Entry_Lane		OUT		INTEGER,
// 	 p_Id_Entry_Trs			OUT		VARCHAR2,
// 	 p_Dte_Entry			OUT		VARCHAR2,
// 	 p_Entry_VRN			OUT		VARCHAR2,
// 	 p_Entry_VRN_Country	OUT		VARCHAR2,
// 	 p_Id_Entry_Provider	OUT		VARCHAR2,
// 	 p_Id_Contract_Type		OUT		INTEGER,
// 	 p_Id_Payment_SubType	OUT		INTEGER,
// 	 p_Id_Class				OUT		INTEGER,
// 	 p_Accepted_Days		OUT		VARCHAR2,
// 	 p_Accoun_No			OUT		INTEGER,
// 	 p_User_No				OUT		INTEGER,
//	p_Renewal_No			OUT		INTEGER,
//	p_Account_Balance		OUT		INTEGER,
//	p_User_Balance			OUT		INTEGER,
//	p_Flag_Result			OUT		INTEGER,
//	p_Id_Reason				OUT		INTEGER),
//	p_Discount_Group		OUT    INTEGER)	IS


#define DB_DEFAULT_EXL_Update_REQUEST_V3 \
	    "BEGIN AUTH_VT.VT_EXL_UpdatePassage_V3( :p_Dte_Ref_Exit, :p_Ref_Exit_Trs, :p_Id_Exit_Plaza, :p_Id_Exit_Lane, :p_Id_Exit_Trs, :p_Dte_Exit, :p_Exit_VRN, :p_Exit_VRN_Country, :p_Ticket_Type, :p_Id_Exit_Provider, :p_Flag_ETicket, :p_Flag_Delete, :p_Flag_Open_System, :p_Flag_No_Update," \
			":p_Dte_Ref_Entry, :p_Ref_Entry_Trs, :p_Id_Entry_Plaza, :p_Id_Entry_Lane, :p_Id_Entry_Trs, :p_Dte_Entry, :p_Entry_VRN, :p_Entry_VRN_Country, \
			:p_Id_Entry_Provider, :p_Id_Contract_Type, :p_Id_Payment_SubType, :p_Id_Class, :p_Accepted_Days, :p_Accoun_No, :p_User_No, :p_Renewal_No, :p_Account_Balance, :p_User_Balance, :p_Flag_Result, :p_Id_Reason, :p_Discount_Group); COMMIT; END; "


PRIVATE char * DBTrim( char * pcStr );
PRIVATE BOOL DB_ConvertStr_ToSYSTEMTIME(char * pcStr, SYSTEMTIME *psDate);
PRIVATE void DB_ConvertSYSTEMTIME_ToStr(SYSTEMTIME *psDate, char * pcStr, DWORD dwSizeofStr);


// --------------- CODE ----------------------


PROTECTED BOOL DBGetIsDatabaseMissing()
{
	BOOL bRetVal;

	EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);
	
	bRetVal = (gsSvcWork.hDbCnx == NULL);

	LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

	return bRetVal;
}


PROTECTED unsigned long long DBGetLastTimeDbCnxUsed()
{
	unsigned long long ullLastDbCnxUse;

	EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);

	ullLastDbCnxUse = gsSvcWork.ullLastDbCnxUse;

	LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

	return ullLastDbCnxUse;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void DBOpen( )
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
PROTECTED BOOL DBOpen()
{
	gsSvcWork.ullLastDbCnxUse = GetSystemULLTime();

    if ( gsSvcWork.hDbCnx == NULL )
    {
        if ( strlen( gsSvcWork.sParmWork.szDbInst ) == 0)
        {
            NTSVCInfo( "DBOpen(), no database defined" );
            gsSvcWork.hDbCnx = NULL;

            return FALSE;
        }

        NTSVCInfo( "DBOpen(), connect to the database" );

        gsSvcWork.hDbCnx = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
        // Si toujours pas connecté, on ne va pas plue long
        if ( gsSvcWork.hDbCnx == NULL )
        {
            NTSVCInfo( "DBOpen(), error connecting to the databse" );

            return FALSE;
        }
    }

    return TRUE;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void DBCleanup( )
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : gsSvcWork
 * --------------------------------------------------------------------
 * ROLE      : Si on est encore connecté à la base, on la déconnecte
 * --------------------------------------------------------------------
 */
PROTECTED void DBCleanup()
{
    if ( gsSvcWork.hDbCnx != NULL )
    {
        NTSVCInfo( "DBCleanup(), disconnect from the database" );
        DBDisconnect( gsSvcWork.hDbCnx );
        gsSvcWork.hDbCnx = NULL;
    }
}


PRIVATE BOOL UtlFindInList(IN DWORD dwFind, IN char * szList)
{
	DWORD               dwValue;
	BOOL                fFound = FALSE;
	BYTE              * pbScan = (BYTE*)szList;

	while ((!fFound) && ((*pbScan) != (char)0))
	{
		if (!isdigit(*pbScan))
		{
			pbScan++;
			continue;
		}

		dwValue = 0;
		while (isdigit(*pbScan))
		{
			dwValue = (dwValue * 10) + (DWORD)(*pbScan) - (DWORD)('0');
			pbScan++;
		}

		fFound = (dwFind == dwValue);
	}

	return fFound;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL DBIfShouldRetry( )
 * PARAMETERS: 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
PROTECTED BOOL DBIfShouldRetry(DWORD dwErr)
{


	if(gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		// If the connection was lost
		if ((dwErr >= 3100 && dwErr < 3122) || UtlFindInList(dwErr, gsSvcWork.sParmWork.szDbReconnectIfErr))
		{
			NTSVCInfo( "DBIfShouldRetry(), database connection error" );

			EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);
				DBCleanup();// Try to connect
			LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

			return TRUE; // DBOpen(); - not needed it will try to connect in the DB function
		}
		else if (dwErr == 4068) //ORA-04068: existing state of packages has been discarded
		{
			return TRUE;
		}

	}

	return FALSE;
}


PROTECTED DWORD DBAuthVT_EnlReq(struct MSG_LC_AUTH_VT_ENL_REQ *psAuthReq, struct MSG_LC_AUTH_VT_ENL_REP *psAuthRsp)
{
	DWORD dwErr = NO_ERROR;          // Returned error code
	DB_STMT * hStmt = NULL;          // Handle de requete
	char szErr[2000] = {0};            // Texte d'erreur
    DB_CNX * hDbCnx = NULL;          // Database conection handle
	CHAR	szStatement[1024] = { 0 };
	DB_VAR * hDte_Ref_Entry; //:p_Dte_Ref_Entry, 
	char szDteRefEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hRef_Entry_Trs; //: p_Ref_Entry_Trs, 
	DB_VAR * hId_Entry_Plaza; //: p_Id_Entry_Plaza, 
	DB_VAR * hId_Entry_Lane; //: p_Id_Entry_Lane, 
	DB_VAR * hId_Entry_Trs; //: p_Id_Entry_Trs, 
	DB_VAR * hDte_Entry; //: p_Dte_Entry, 
	char szDteEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hEntry_VRN; //: p_Entry_VRN, 
	DB_VAR * hEntry_VRN_Country; //: p_Entry_VRN_Country, 
	DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
	DB_VAR * hResult; //: p_Result, 
	unsigned int uiResult = 0;
	DB_VAR * h_id_Reason; //: p_id_Reason
	unsigned int uiIdReason = 0;
	unsigned int *puiResult = NULL;
	DWORD	dwLen;

    // Boucle do while() utilisée pour la commodité du break. En faite, la condition
    // de bouclage est FALSE, on ne passe donc qu'une fois dedans.
    do 
    {
        if(gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
        {
			EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);

           if (!DBOpen())
            {
                NTSVCInfo( "DBAuthVT_EnlReq(), error reconnecting to the database" );
                dwErr = ERROR_PIPE_NOT_CONNECTED;
                break;
            }

			hDbCnx = gsSvcWork.hDbCnx;
		}
		else
		{
            // Essayer d'établir la connexion
            NTSVCInfo( "DBAuthVT_EnlReq(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr );
            hDbCnx  = DBConnect( gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd );
            // Si toujours pas connecté, on ne va pas plue long
            if ( hDbCnx == NULL )
            {
                dwErr = ERROR_PIPE_NOT_CONNECTED;
                NTSVCInfo( "DBAuthVT_EnlReq(), error %u, connexion à la base impossible", dwErr );
                break;
            }
		}

        // REQUEST: Select record for requested media ID
        NTSVCInfo( "DBAuthVT_EnlReq(), preparing the request" );

		// Prepare the request
		_snprintf_s(szStatement,
			_countof(szStatement),
			sizeof(szStatement),
			DB_DEFAULT_ENL_Insert_REQUEST);


        // Prepare the request
		hStmt = DBOpenStatement(hDbCnx, szStatement);

        if ( hStmt == NULL  )
        {
            DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "DBAuthVT_EnlReq(), error %u, statement AuthRequest : %s", dwErr, szErr );
            break;
        }

		// Associer les variables
		hDte_Ref_Entry = DBBindPlaceHolderVariable(
                hStmt, 
                ":p_Dte_Ref_Entry", 
                DB_TYPE_STR, 
				DB_DATE_CHAR_LENGHT + 1,
                0 );
		if (hDte_Ref_Entry == NULL)
        {
            DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "DBAuthVT_EnlReq(), error %u, bind p_Dte_Ref_Entry : %s", dwErr, szErr );
            break;
        }


		hRef_Entry_Trs = DBBindPlaceHolderVariable(
                hStmt, 
                ":p_Ref_Entry_Trs", 
                DB_TYPE_INT, 
                sizeof(DWORD), 
                0 );
		if (hRef_Entry_Trs == NULL)
        {
            DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "DBAuthVT_EnlReq(), error %u, bind p_Ref_Entry_Trs : %s", dwErr, szErr );
            break;
        }

		hId_Entry_Plaza = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Plaza",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Plaza == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReq(), error %u, bind p_Id_Entry_Plaza : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Lane = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Lane",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Lane == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReq(), error %u, bind p_Id_Entry_Lane : %s", dwErr, szErr);
			break;
		}


		//DB_VAR * hId_Entry_Trs; //:p_Id_Entry_Trs, 
		hId_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Trs",
			DB_TYPE_STR,
			DB_ID_ENTRY_TRS_LENGHT + 1,
			0);
		if (hId_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReq(), error %u, bind p_Id_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hDte_Entry; //:p_Dte_Entry, 
		hDte_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReq(), error %u, bind p_Dte_Entry : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hEntry_VRN; //:p_Entry_VRN, 
		hEntry_VRN = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN",
			DB_TYPE_STR,
			DB_ENTRY_VRN_LENGHT + 1,
			0);
		if (hEntry_VRN == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReq(), error %u, bind p_Entry_VRN : %s", dwErr, szErr);
			break;
		}
		
		//DB_VAR * hEntry_VRN_Country; //:p_Entry_VRN_Country, 
		hEntry_VRN_Country = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN_Country",
			DB_TYPE_STR,
			DB_ENTRY_VRN_COUNTRY + 1,
			0);
		if (hEntry_VRN_Country == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReq(), error %u, bind p_Entry_VRN_Country : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hId_Entry_Provider; //:p_Id_Entry_Provider, 
		hId_Entry_Provider = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Provider",
			DB_TYPE_STR,
			DB_ENTRY_VRN_PROVIDER + 1,
			0);
		if (hId_Entry_Provider == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReq(), error %u, bind p_Id_Entry_Provider : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hResult; //:p_Result, 
		hResult = DBBindPlaceHolderVariable(
			hStmt,
			":p_Result",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hResult == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReq(), error %u, bind p_Result : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * h_id_Reason; //:p_id_Reason
		h_id_Reason = DBBindPlaceHolderVariable(
			hStmt,
			":p_id_Reason",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (h_id_Reason == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReq(), error %u, bind p_id_Reason : %s", dwErr, szErr);
			break;
		}

        // Initialiser les variables en input
        //DBSetVariableItemValue( hMediaNum, 0, &psAutResp->szMediaNum, AUTHSVC_DB_MEDIA_NUM_MAX_LENTG );
		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_ref_entry, szDteRefEntry, sizeof(szDteRefEntry));
		DBSetVariableItemValue(hDte_Ref_Entry, 0, &szDteRefEntry, DB_DATE_CHAR_LENGHT + 1);

		DBSetVariableItemValue(hRef_Entry_Trs, 0, &psAuthReq->body.entry_trs_ref_num, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Entry_Plaza, 0, &psAuthReq->body.entry_plaza_id, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Entry_Lane, 0, &psAuthReq->body.entry_lane_id, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Entry_Trs, 0, &psAuthReq->body.entry_trs_id, DB_ID_ENTRY_TRS_LENGHT + 1);

		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_entry, szDteEntry, sizeof(szDteEntry));
		DBSetVariableItemValue(hDte_Entry, 0, &szDteEntry, DB_DATE_CHAR_LENGHT + 1);

		DBSetVariableItemValue(hEntry_VRN, 0, &psAuthReq->body.entry_vrn, DB_ENTRY_VRN_LENGHT + 1);
		DBSetVariableItemValue(hEntry_VRN_Country, 0, &psAuthReq->body.entry_vrn_country, DB_ENTRY_VRN_COUNTRY + 1);
		DBSetVariableItemValue(hId_Entry_Provider, 0, &psAuthReq->body.entry_provider, DB_ENTRY_VRN_PROVIDER + 1);

        NTSVCInfo( "DBAuthVT_EnlReq(), execute request" );

        // Exécuter la requète
        if ( ! DBExecuteStatement( hStmt ) )
        {
            DBGetLastError( hDbCnx, NULL, &dwErr, szErr, sizeof(szErr) );
            NTSVCInfo( "DBAuthVT_EnlReq(), error %u, executing: %s", dwErr, szErr );
            break;
        }

		// Get database data
		dwLen = sizeof(uiResult);
		puiResult = DBGetVariableItemValue(hResult, 0, &uiResult, &dwLen);
		if (puiResult != &uiResult && dwLen == sizeof(uiResult))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReq(), error getting uiResult");
			break;
		}

		dwLen = sizeof(uiIdReason);
		puiResult = DBGetVariableItemValue(h_id_Reason, 0, &uiIdReason, &dwLen);
		if (puiResult != &uiIdReason && dwLen == sizeof(uiIdReason))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReq(), error getting uiIdReason");
			break;
		}

 		NTSVCInfo("DBAuthVT_EnlReq(), result:%d reason:%d", uiResult, uiIdReason);
    }
    while ( FALSE );
    
    if ( dwErr != NO_ERROR )
    {
        NTSVCInfo( "DBAuthVT_EnlReq(), Error getting data %d", dwErr );

        // 3113 - Database down during statement exec
        // ERROR_PIPE_NOT_CONNECTED - Database down before connect
		psAuthRsp->body.flag_result = AUTH_NOK;
		psAuthRsp->body.reason_id = dwErr;

    }
	else //	if ( dwErr == NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_EnlReq() - Result:%s Reason:%d", (uiResult == AUTH_NOK) ? "AUTH_NOK" : "AUTH_OK", uiIdReason);
		psAuthRsp->body.flag_result = uiResult;
		
		if (uiResult == AUTH_NOK)
			psAuthRsp->body.reason_id = uiIdReason;
		else
			psAuthRsp->body.reason_id = 0;
    }

	if ( hStmt != NULL  )
        DBCloseStatement( hStmt );

	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

	}
	else if ( hDbCnx != NULL) // && gsSvcWork.sParmWork.dwDbKeepConnection == FALSE)
    {
        NTSVCInfo( "DBAuthVT_EnlReq(), disconnect from the database" );
        DBDisconnect( hDbCnx );
    }

    return dwErr;
}


PROTECTED DWORD DBAuthVT_ExlReq(struct MSG_LC_AUTH_VT_EXL_REQ *psAuthReq, struct MSG_LC_AUTH_VT_EXL_REP *psAuthRsp)
{
	DWORD dwErr = NO_ERROR;          // Returned error code
	DB_STMT * hStmt = NULL;          // Handle de requete
	char szErr[2000] = { 0 };            // Texte d'erreur
	DB_CNX * hDbCnx = NULL;          // Database conection handle
	CHAR	szStatement[1024] = { 0 };

	DB_VAR * hDte_Ref_Exit; //:p_Dte_Ref_Exit, 
	char szDteRefExit[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hRef_Exit_Trs;//:p_Ref_Exit_Trs,
	DB_VAR * hId_Exit_Plaza;//:p_Id_Exit_Plaza,
	DB_VAR * hId_Exit_Lane;//:p_Id_Exit_Lane, 
	DB_VAR * hId_Exit_Trs;//:p_Id_Exit_Trs, 
	DB_VAR * hDte_Exit; //: p_Dte_Exit, 
	char szDteExit[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hExit_VRN;//:p_Exit_VRN, 
	DB_VAR * hExit_VRN_Country;//:p_Exit_VRN_Country, 
	DB_VAR * hId_Exit_Provider;//:p_Id_Exit_Provider, 
	DB_VAR * hFlag_ETicket;//:p_Flag_ETicket,

	DB_VAR * hDte_Ref_Entry; //:p_Dte_Ref_Entry, 
	char szDteRefEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hRef_Entry_Trs; //: p_Ref_Entry_Trs, 
	DB_VAR * hId_Entry_Plaza; //: p_Id_Entry_Plaza, 
	DB_VAR * hId_Entry_Lane; //: p_Id_Entry_Lane, 
	DB_VAR * hId_Entry_Trs; //: p_Id_Entry_Trs, 
	DB_VAR * hDte_Entry; //: p_Dte_Entry, 
	char szDteEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hEntry_VRN; //: p_Entry_VRN, 
	DB_VAR * hEntry_VRN_Country; //: p_Entry_VRN_Country, 
	DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
	DB_VAR * hResult; //: p_Result, 
	unsigned int uiResult = 0;
	DB_VAR * h_id_Reason; //: p_id_Reason
	unsigned int uiIdReason = 0;
	unsigned int *puiResult = NULL;
	char *pbResult;
	DWORD	dwLen;

	// Boucle do while() utilisée pour la commodité du break. En faite, la condition
	// de bouclage est FALSE, on ne passe donc qu'une fois dedans.
	do
	{
		if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
		{
			EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);

			if (!DBOpen())
			{
				NTSVCInfo("DBAuthVT_ExlReq(), error reconnecting to the database");
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				break;
			}

			hDbCnx = gsSvcWork.hDbCnx;
		}
		else
		{
			// Essayer d'établir la connexion
			NTSVCInfo("DBAuthVT_ExlReq(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr);
			hDbCnx = DBConnect(gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd);
			// Si toujours pas connecté, on ne va pas plue long
			if (hDbCnx == NULL)
			{
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				NTSVCInfo("DBAuthVT_ExlReq(), error %u, connexion à la base impossible", dwErr);
				break;
			}
		}

		// REQUEST: Select record for requested media ID
		NTSVCInfo("DBAuthVT_ExlReq(), preparing the request");

		// Prepare the request
		_snprintf_s(szStatement,
			_countof(szStatement),
			sizeof(szStatement),
			DB_DEFAULT_EXL_Update_REQUEST);


		// Prepare the request
		hStmt = DBOpenStatement(hDbCnx, szStatement);

		if (hStmt == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, statement AuthRequest : %s", dwErr, szErr);
			break;
		}


		hDte_Ref_Exit = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Ref_Exit",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Ref_Exit == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Dte_Ref_Exit : %s", dwErr, szErr);
			break;
		}

		hRef_Exit_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ref_Exit_Trs",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hRef_Exit_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Ref_Exit_Trs : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Plaza = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Plaza",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Exit_Plaza == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Id_Exit_Plaza : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Lane = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Lane",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Exit_Lane == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Id_Exit_Lane : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Trs",
			DB_TYPE_STR,
			DB_ID_ENTRY_TRS_LENGHT + 1,
			0);
		if (hId_Exit_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Id_Exit_Trs : %s", dwErr, szErr);
			break;
		}

		hDte_Exit = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Exit",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Exit == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Dte_Exit : %s", dwErr, szErr);
			break;
		}

		hExit_VRN = DBBindPlaceHolderVariable(
			hStmt,
			":p_Exit_VRN",
			DB_TYPE_STR,
			DB_ENTRY_VRN_LENGHT + 1,
			0);
		if (hExit_VRN == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Exit_VRN : %s", dwErr, szErr);
			break;
		}

		hExit_VRN_Country = DBBindPlaceHolderVariable(
			hStmt,
			":p_Exit_VRN_Country",
			DB_TYPE_STR,
			DB_ENTRY_VRN_COUNTRY + 1,
			0);
		if (hExit_VRN_Country == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Exit_VRN_Country : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Provider = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Provider",
			DB_TYPE_STR,
			DB_ENTRY_VRN_PROVIDER + 1,
			0);
		if (hId_Exit_Provider == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Id_Exit_Provider : %s", dwErr, szErr);
			break;
		}

		hFlag_ETicket = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_ETicket",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hFlag_ETicket == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Flag_ETicket : %s", dwErr, szErr);
			break;
		}

		// Associer les variables
		hDte_Ref_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Ref_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Ref_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Dte_Ref_Entry : %s", dwErr, szErr);
			break;
		}


		hRef_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ref_Entry_Trs",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hRef_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Ref_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Plaza = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Plaza",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Plaza == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Id_Entry_Plaza : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Lane = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Lane",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Lane == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Id_Entry_Lane : %s", dwErr, szErr);
			break;
		}


		//DB_VAR * hId_Entry_Trs; //:p_Id_Entry_Trs, 
		hId_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Trs",
			DB_TYPE_STR,
			DB_ID_ENTRY_TRS_LENGHT + 1,
			0);
		if (hId_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Id_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hDte_Entry; //:p_Dte_Entry, 
		hDte_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Dte_Entry : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hEntry_VRN; //:p_Entry_VRN, 
		hEntry_VRN = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN",
			DB_TYPE_STR,
			DB_ENTRY_VRN_LENGHT + 1,
			0);
		if (hEntry_VRN == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Entry_VRN : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hEntry_VRN_Country; //:p_Entry_VRN_Country, 
		hEntry_VRN_Country = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN_Country",
			DB_TYPE_STR,
			DB_ENTRY_VRN_COUNTRY + 1,
			0);
		if (hEntry_VRN_Country == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Entry_VRN_Country : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hId_Entry_Provider; //:p_Id_Entry_Provider, 
		hId_Entry_Provider = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Provider",
			DB_TYPE_STR,
			DB_ENTRY_VRN_PROVIDER + 1,
			0);
		if (hId_Entry_Provider == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Id_Entry_Provider : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hResult; //:p_Result, 
		hResult = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_Result",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hResult == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind p_Flag_Result : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * h_id_Reason; //:p_id_Reason
		h_id_Reason = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Reason",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (h_id_Reason == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, bind :p_Id_Reason : %s", dwErr, szErr);
			break;
		}

		// Initialiser les variables en input
		//":p_Dte_Ref_Exit, :p_Ref_Exit_Trs, :p_Id_Exit_Plaza, :p_Id_Exit_Lane, :p_Id_Exit_Trs, :p_Dte_Exit, :p_Exit_VRN, :p_Exit_VRN_Country, :p_Id_Exit_Provider, :p_Flag_ETicket, " 

		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_ref_exit, szDteRefExit, sizeof(szDteRefExit));
		DBSetVariableItemValue(hDte_Ref_Exit, 0, &szDteRefExit, DB_DATE_CHAR_LENGHT + 1);

		DBSetVariableItemValue(hRef_Exit_Trs, 0, &psAuthReq->body.exit_trs_ref_num, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Exit_Plaza, 0, &psAuthReq->body.exit_plaza_id, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Exit_Lane, 0, &psAuthReq->body.exit_lane_id, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Exit_Trs, 0, &psAuthReq->body.exit_trs_id, DB_ID_ENTRY_TRS_LENGHT + 1);

		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_exit, szDteExit, sizeof(szDteExit));
		DBSetVariableItemValue(hDte_Exit, 0, &szDteExit, DB_DATE_CHAR_LENGHT + 1);

		DBSetVariableItemValue(hExit_VRN, 0, &psAuthReq->body.exit_vrn, DB_ENTRY_VRN_LENGHT + 1);
		DBSetVariableItemValue(hExit_VRN_Country, 0, &psAuthReq->body.exit_vrn_country, DB_ENTRY_VRN_COUNTRY + 1);
		DBSetVariableItemValue(hId_Exit_Provider, 0, &psAuthReq->body.exit_provider, DB_ENTRY_VRN_PROVIDER + 1);
		DBSetVariableItemValue(hFlag_ETicket, 0, &psAuthReq->body.flag_eticket, sizeof(unsigned int));

		NTSVCInfo("DBAuthVT_ExlReq(szDteRefExit:[%s], exit_trs_ref_num:[%u], exit_plaza_id:[%u], exit_lane_id[%u], exit_trs_id:[%s], szDteExit[%s], exit_vrn[%s], exit_vrn_country[%s], exit_provider[%s],flag_eticket:[%u]), execute request", 
			szDteRefExit,
			psAuthReq->body.exit_trs_ref_num,
			psAuthReq->body.exit_plaza_id,
			psAuthReq->body.exit_lane_id,
			psAuthReq->body.exit_trs_id,
			szDteExit,
			psAuthReq->body.exit_vrn,
			psAuthReq->body.exit_vrn_country,
			psAuthReq->body.exit_provider,
			psAuthReq->body.flag_eticket);

		// Exécuter la requète
		if (!DBExecuteStatement(hStmt))
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReq(), error %u, executing: %s", dwErr, szErr);
			break;
		}

		//First get result and reason
		//: p_Flag_Result, 
		dwLen = sizeof(uiResult);
		puiResult = DBGetVariableItemValue(hResult, 0, &uiResult, &dwLen);
		if (puiResult != &uiResult && dwLen == sizeof(uiResult))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Flag_Result");
			break;
		}

		//:p_Id_Reason); COMMIT; END; "
		dwLen = sizeof(uiIdReason);
		puiResult = DBGetVariableItemValue(h_id_Reason, 0, &uiIdReason, &dwLen);
		if (puiResult != &uiIdReason && dwLen == sizeof(uiIdReason))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Id_Reason");
			break;
		}

		//Do not try to read other OUT variables in case of any error...
		if (dwErr != NO_ERROR || uiResult == AUTH_NOK)
			break;

		// Get database data
		//":p_Dte_Ref_Entry, 
		pbResult = DBGetVariableItemValue(hDte_Ref_Entry, 0, szDteRefEntry, NULL);
		if (pbResult != szDteRefEntry)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Dte_Ref_Entry");
			break;
		}
		else
		{
			if (FALSE == DB_ConvertStr_ToSYSTEMTIME(szDteRefEntry, &psAuthRsp->body.dte_ref_entry))
			{
				NTSVCInfo("DBAuthVT_ExlReq(), Error DB_ConvertStr_ToSYSTEMTIME from szDteRefEntry [%s]", szDteRefEntry);
			}
		}

		//:p_Ref_Entry_Trs, 		dwLen = sizeof(psAuthRsp->body.entry_trs_ref_num);
		dwLen = sizeof(psAuthRsp->body.entry_trs_ref_num);
		puiResult = DBGetVariableItemValue(hRef_Entry_Trs, 0, &psAuthRsp->body.entry_trs_ref_num, &dwLen);
		if (puiResult != &psAuthRsp->body.entry_trs_ref_num)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Ref_Entry_Trs");
			break;
		}

		//:p_Id_Entry_Plaza, 
		dwLen = sizeof(psAuthRsp->body.entry_plaza_id);
		puiResult = DBGetVariableItemValue(hId_Entry_Plaza, 0, &psAuthRsp->body.entry_plaza_id, &dwLen);
		if (puiResult != &psAuthRsp->body.entry_plaza_id)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Id_Entry_Plaza");
			break;
		}

		//DB_VAR * hId_Entry_Lane; //: p_Id_Entry_Lane, 
		dwLen = sizeof(psAuthRsp->body.entry_lane_id);
		puiResult = DBGetVariableItemValue(hId_Entry_Lane, 0, &psAuthRsp->body.entry_lane_id, &dwLen);
		if (puiResult != &psAuthRsp->body.entry_lane_id)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Id_Entry_Lane");
			break;
		}

		//DB_VAR * hId_Entry_Trs; //: p_Id_Entry_Trs, 
		pbResult = DBGetVariableItemValue(hId_Entry_Trs, 0, psAuthRsp->body.entry_trs_id, NULL);
		if (pbResult != psAuthRsp->body.entry_trs_id)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Id_Entry_Trs");
			break;
		}

		//DB_VAR * hDte_Entry; //: p_Dte_Entry, 
		//char szDteEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
		pbResult = DBGetVariableItemValue(hDte_Entry, 0, szDteEntry, NULL);
		if (pbResult != szDteEntry)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Dte_Entry");
			break;
		}
		else
		{
			if (FALSE == DB_ConvertStr_ToSYSTEMTIME(szDteEntry, &psAuthRsp->body.dte_entry))
			{
				NTSVCInfo("DBAuthVT_ExlReq(), Error DB_ConvertStr_ToSYSTEMTIME from szDteEntry [%s]", szDteEntry);
			}
		}

		//DB_VAR * hEntry_VRN; //: p_Entry_VRN, 
		pbResult = DBGetVariableItemValue(hEntry_VRN, 0, psAuthRsp->body.entry_vrn, NULL);
		if (pbResult != psAuthRsp->body.entry_vrn)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Entry_VRN");
			break;
		}

		
		//DB_VAR * hEntry_VRN_Country; //: p_Entry_VRN_Country, 
		pbResult = DBGetVariableItemValue(hEntry_VRN_Country, 0, psAuthRsp->body.entry_vrn_country, NULL);
		if (pbResult != psAuthRsp->body.entry_vrn_country)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Entry_VRN_Country");
			break;
		}

		//DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
		pbResult = DBGetVariableItemValue(hId_Entry_Provider, 0, psAuthRsp->body.entry_provider, NULL);
		if (pbResult != psAuthRsp->body.entry_provider)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Id_Entry_Provider");
			break;
		}
		
	} while (FALSE);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_ExlReq(), Error getting data %d", dwErr);

		// 3113 - Database down during statement exec
		// ERROR_PIPE_NOT_CONNECTED - Database down before connect
		psAuthRsp->body.flag_result = AUTH_NOK;
		psAuthRsp->body.reason_id = dwErr;
	}
	else //	if ( dwErr == NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_ExlReq() - Result:%s Reason:%d", (uiResult == AUTH_NOK) ? "AUTH_NOK" : "AUTH_OK", uiIdReason);
		psAuthRsp->body.flag_result = uiResult;

		if (uiResult == AUTH_NOK)
			psAuthRsp->body.reason_id = uiIdReason;
		else
			psAuthRsp->body.reason_id = 0;
	}

	if (hStmt != NULL)
		DBCloseStatement(hStmt);
	
	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

	}
	else if (hDbCnx != NULL) // && gsSvcWork.sParmWork.dwDbKeepConnection == FALSE)
	{
		NTSVCInfo("DBAuthVT_ExlReq(), disconnect from the database");
		DBDisconnect(hDbCnx);
	}

	return dwErr;
}

PRIVATE char * DBTrim( char * pcStr )
{
    char * pcSrc = pcStr;
    char * pcDst = pcStr;
    char * pcStop = NULL;

    while ( (*pcSrc > '\0') && (*pcSrc <= ' ') ) pcSrc ++;
    while ( *pcSrc != '\0' )
    {
        *pcDst = *pcSrc;
        if ( ( pcStop == NULL ) && ( *pcDst <= ' ' ) && (*pcDst >= '\0') )
            pcStop = pcDst;
        else if ( (*pcSrc < '\0') || (*pcSrc > ' ') )
            pcStop = NULL;
        pcDst ++;
        pcSrc ++;
    }
    if ( pcStop != NULL ) 
        *pcStop = '\0';
    else
        *pcDst = '\0';
    return pcStr;
}


PRIVATE void DB_ConvertSYSTEMTIME_ToStr(SYSTEMTIME *psDate, char * pcStr, DWORD dwSizeofStr)
{
	sprintf_s(pcStr, dwSizeofStr, "%04d%02d%02d%02d%02d%02d",
		(int)(psDate->wYear),
		(int)(psDate->wMonth),
		(int)(psDate->wDay),
		(int)(psDate->wHour),
		(int)(psDate->wMinute),
		(int)(psDate->wSecond));
	
}

PRIVATE BOOL DB_ConvertStr_ToSYSTEMTIME(char * pcStr, SYSTEMTIME *psDate)
{
	char szTemp[15] = { 0 };
	strcpy_s(szTemp, sizeof(szTemp), pcStr);

	if (strlen(szTemp) == 14) //pcStr, "%04d%02d%02d%02d%02d%02d",
	{
		psDate->wSecond = atoi(&szTemp[12]); szTemp[12] = '\0';
		psDate->wMinute = atoi(&szTemp[10]); szTemp[10] = '\0';
		psDate->wHour = atoi(&szTemp[8]); szTemp[8] = '\0';
		psDate->wDay = atoi(&szTemp[6]); szTemp[6] = '\0';
		psDate->wMonth = atoi(&szTemp[4]); szTemp[4] = '\0';
		psDate->wYear = atoi(&szTemp[0]);

		return TRUE;
	}
	else
		return FALSE;

}



//---------------------------------------------------------------------------------------
// V2 messages...
//---------------------------------------------------------------------------------------


PROTECTED DWORD DBAuthVT_EnlReqV2(struct MSG_LC_AUTH_VT_ENL_REQ_V2 *psAuthReq, struct MSG_LC_AUTH_VT_ENL_REP_V2 *psAuthRsp)
{
	DWORD dwErr = NO_ERROR;          // Returned error code
	DB_STMT * hStmt = NULL;          // Handle de requete
	char szErr[2000] = { 0 };            // Texte d'erreur
	DB_CNX * hDbCnx = NULL;          // Database conection handle
	CHAR	szStatement[1024] = { 0 };
	DB_VAR * hDte_Ref_Entry; //:p_Dte_Ref_Entry, 
	char szDteRefEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hRef_Entry_Trs; //: p_Ref_Entry_Trs, 
	DB_VAR * hId_Entry_Plaza; //: p_Id_Entry_Plaza, 
	DB_VAR * hId_Entry_Lane; //: p_Id_Entry_Lane, 
	DB_VAR * hId_Entry_Trs; //: p_Id_Entry_Trs, 
	DB_VAR * hDte_Entry; //: p_Dte_Entry, 
	char szDteEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hEntry_VRN; //: p_Entry_VRN, 
	DB_VAR * hEntry_VRN_Country; //: p_Entry_VRN_Country, 
	DB_VAR * hEntry_Ticket_Type; //:p_Ticket_Type, 
	DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
	DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType,
	unsigned int uiId_Payment_SubType = 0;
	DB_VAR * hId_Class; //:p_Id_Class,
	unsigned int uiId_Class = 0;
	DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
	DB_VAR * hResult; //: p_Result, 
	unsigned int uiResult = 0;
	DB_VAR * h_id_Reason; //: p_id_Reason
	unsigned int uiIdReason = 0;
	unsigned int *puiResult = NULL;
	char *pbResult;
	DWORD	dwLen;

	// Boucle do while() utilisée pour la commodité du break. En faite, la condition
	// de bouclage est FALSE, on ne passe donc qu'une fois dedans.
	do
	{
		if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
		{
			EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);
			if (!DBOpen())
			{
				NTSVCInfo("DBAuthVT_EnlReqV2(), error reconnecting to the database");
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				break;
			}

			hDbCnx = gsSvcWork.hDbCnx;
		}
		else
		{
			// Essayer d'établir la connexion
			NTSVCInfo("DBAuthVT_EnlReqV2(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr);
			hDbCnx = DBConnect(gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd);
			// Si toujours pas connecté, on ne va pas plue long
			if (hDbCnx == NULL)
			{
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, connexion à la base impossible", dwErr);
				break;
			}
		}

		// REQUEST: Select record for requested media ID
		NTSVCInfo("DBAuthVT_EnlReqV2(), preparing the request");

		// Prepare the request
		_snprintf_s(szStatement,
			_countof(szStatement),
			sizeof(szStatement),
			DB_DEFAULT_ENL_Insert_REQUEST_V2);


		// Prepare the request
		hStmt = DBOpenStatement(hDbCnx, szStatement);

		if (hStmt == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, statement AuthRequest : %s", dwErr, szErr);
			break;
		}

		// Associer les variables
		hDte_Ref_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Ref_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Ref_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Dte_Ref_Entry : %s", dwErr, szErr);
			break;
		}


		hRef_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ref_Entry_Trs",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hRef_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Ref_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Plaza = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Plaza",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Plaza == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Id_Entry_Plaza : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Lane = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Lane",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Lane == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Id_Entry_Lane : %s", dwErr, szErr);
			break;
		}


		//DB_VAR * hId_Entry_Trs; //:p_Id_Entry_Trs, 
		hId_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Trs",
			DB_TYPE_STR,
			DB_ID_ENTRY_TRS_LENGHT + 1,
			0);
		if (hId_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Id_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hDte_Entry; //:p_Dte_Entry, 
		hDte_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Dte_Entry : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hEntry_VRN; //:p_Entry_VRN, 
		hEntry_VRN = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN",
			DB_TYPE_STR,
			DB_ENTRY_VRN_LENGHT + 1,
			0);
		if (hEntry_VRN == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Entry_VRN : %s", dwErr, szErr);
			break;
		}

		// p_Entry_VRN_Country   IN     VARCHAR2,
		//DB_VAR * hEntry_VRN_Country; //:p_Entry_VRN_Country, 
		hEntry_VRN_Country = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN_Country",
			DB_TYPE_STR,
			DB_ENTRY_VRN_COUNTRY + 1,
			0);
		if (hEntry_VRN_Country == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Entry_VRN_Country : %s", dwErr, szErr);
			break;
		}

		// p_Ticket_Type         IN     NUMBER,
		//DB_VAR * hEntry_Ticket_Type; //:p_Ticket_Type, 
		hEntry_Ticket_Type = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ticket_Type",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hEntry_Ticket_Type == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Ticket_Type : %s", dwErr, szErr);
			break;
		}

		// p_Id_Entry_Provider   OUT    VARCHAR2,
		//DB_VAR * hId_Entry_Provider; //:p_Id_Entry_Provider, 
		hId_Entry_Provider = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Provider",
			DB_TYPE_STR,
			DB_ENTRY_VRN_PROVIDER + 1,
			0);
		if (hId_Entry_Provider == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Id_Entry_Provider : %s", dwErr, szErr);
			break;
		}


		// p_Id_Payment_SubType OUT    INTEGER,
		//DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType, 
		hId_Payment_SubType = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Payment_SubType",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Payment_SubType == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Id_Payment_SubType : %s", dwErr, szErr);
			break;
		}

		// p_Id_Class            OUT    INTEGER,
		//DB_VAR * hId_Class; //:p_Id_Class, 
		hId_Class = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Class",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Class == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Id_Class : %s", dwErr, szErr);
			break;
		}

		// p_Accepted_Days       OUT    VARCHAR2,
		//DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
		hAccepted_Days = DBBindPlaceHolderVariable(
			hStmt,
			":p_Accepted_Days",
			DB_TYPE_STR,
			DB_ENTRY_ACCEPTED_DAYS + 1,
			0);
		if (hAccepted_Days == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Accepted_Days : %s", dwErr, szErr);
			break;
		}


		// p_Result              OUT    INTEGER,
		//DB_VAR * hResult; //:p_Result, 
		hResult = DBBindPlaceHolderVariable(
			hStmt,
			":p_Result",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hResult == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Result : %s", dwErr, szErr);
			break;
		}

		// p_Id_Reason           OUT    INTEGER) 
		//DB_VAR * h_id_Reason; //:p_id_Reason
		h_id_Reason = DBBindPlaceHolderVariable(
			hStmt,
			":p_id_Reason",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (h_id_Reason == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_id_Reason : %s", dwErr, szErr);
			break;
		}

		// Initialiser les variables en input
		//DBSetVariableItemValue( hMediaNum, 0, &psAutResp->szMediaNum, AUTHSVC_DB_MEDIA_NUM_MAX_LENTG );
		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_ref_entry, szDteRefEntry, sizeof(szDteRefEntry));
		DBSetVariableItemValue(hDte_Ref_Entry, 0, &szDteRefEntry, DB_DATE_CHAR_LENGHT + 1);

		DBSetVariableItemValue(hRef_Entry_Trs, 0, &psAuthReq->body.entry_trs_ref_num, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Entry_Plaza, 0, &psAuthReq->body.entry_plaza_id, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Entry_Lane, 0, &psAuthReq->body.entry_lane_id, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Entry_Trs, 0, &psAuthReq->body.entry_trs_id, DB_ID_ENTRY_TRS_LENGHT + 1);

		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_entry, szDteEntry, sizeof(szDteEntry));
		DBSetVariableItemValue(hDte_Entry, 0, &szDteEntry, DB_DATE_CHAR_LENGHT + 1);

		DBSetVariableItemValue(hEntry_VRN, 0, &psAuthReq->body.entry_vrn, DB_ENTRY_VRN_LENGHT + 1);
		DBSetVariableItemValue(hEntry_VRN_Country, 0, &psAuthReq->body.entry_vrn_country, DB_ENTRY_VRN_COUNTRY + 1);
		DBSetVariableItemValue(hEntry_Ticket_Type, 0, &psAuthReq->body.ticket_type, sizeof(unsigned int));

		NTSVCInfo("DBAuthVT_EnlReqV2(), execute request");

		// Exécuter la requète
		if (!DBExecuteStatement(hStmt))
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, executing: %s", dwErr, szErr);
			break;
		}

		// Get database data
		dwLen = sizeof(uiResult);
		puiResult = DBGetVariableItemValue(hResult, 0, &uiResult, &dwLen);
		if (puiResult != &uiResult && dwLen == sizeof(uiResult))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting uiResult");
			break;
		}

		dwLen = sizeof(uiIdReason);
		puiResult = DBGetVariableItemValue(h_id_Reason, 0, &uiIdReason, &dwLen);
		if (puiResult != &uiIdReason && dwLen == sizeof(uiIdReason))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting uiIdReason");
			break;
		}

// 		//Do not try to read other OUT variables in case of any error...
// 		if (dwErr != NO_ERROR || uiResult == AUTH_NOK)
// 			break;

		//DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
		pbResult = DBGetVariableItemValue(hId_Entry_Provider, 0, psAuthRsp->body.entry_provider, NULL);

		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV2(), p_Id_Entry_Provider == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_provider)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting p_Id_Entry_Provider");
			break;
		}

		// p_Id_Payment_SubType  OUT    INTEGER,
		//DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType,
		dwLen = sizeof(uiId_Payment_SubType);
		puiResult = DBGetVariableItemValue(hId_Payment_SubType, 0, &uiId_Payment_SubType, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV2(), p_Id_Payment_SubType == NULL");
			uiId_Payment_SubType = 0;
		}
		else if (puiResult != &uiId_Payment_SubType && dwLen == sizeof(uiId_Payment_SubType))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting p_Id_Payment_SubType");
			break;
		}

		psAuthRsp->body.payment_subtype = uiId_Payment_SubType;

		// p_Id_Class            OUT    INTEGER,
		//DB_VAR * hId_Class; //:p_Id_Class,
		dwLen = sizeof(uiId_Class);
		puiResult = DBGetVariableItemValue(hId_Class, 0, &uiId_Class, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV2(), p_Id_Class == NULL");
			uiId_Class = 0;
		}
		else if (puiResult != &uiId_Class && dwLen == sizeof(uiId_Class))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting p_Id_Class");
			break;
		}

		psAuthRsp->body.vehicle_class = uiId_Class;

		// p_Accepted_Days       OUT    VARCHAR2,
		//DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
		pbResult = DBGetVariableItemValue(hAccepted_Days, 0, psAuthRsp->body.accepted_days, NULL);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV2(), p_Accepted_Days == NULL");

		}
		else if (pbResult != psAuthRsp->body.accepted_days)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting p_Accepted_Days");
			break;
		}


		NTSVCInfo("DBAuthVT_EnlReqV2(), result:%d reason:%d", uiResult, uiIdReason);
	} while (FALSE);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_EnlReqV2(), Error getting data %d", dwErr);

		// 3113 - Database down during statement exec
		// ERROR_PIPE_NOT_CONNECTED - Database down before connect
		psAuthRsp->body.flag_result = AUTH_NOK;
		psAuthRsp->body.reason_id = dwErr;

	}
	else //	if ( dwErr == NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_EnlReqV2() - Result:%s Reason:%d", (uiResult == AUTH_NOK) ? "AUTH_NOK" : "AUTH_OK", uiIdReason);
		psAuthRsp->body.flag_result = uiResult;

		if (uiResult == AUTH_NOK)
			psAuthRsp->body.reason_id = uiIdReason;
		else
			psAuthRsp->body.reason_id = 0;
	}

	if (hStmt != NULL)
		DBCloseStatement(hStmt);

	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);
	}
	else if (hDbCnx != NULL) // && gsSvcWork.sParmWork.dwDbKeepConnection == FALSE)
	{
		NTSVCInfo("DBAuthVT_EnlReqV2(), disconnect from the database");
		DBDisconnect(hDbCnx);
	}

	return dwErr;
}


PROTECTED DWORD DBAuthVT_ExlReqV2(struct MSG_LC_AUTH_VT_EXL_REQ_V2 *psAuthReq, struct MSG_LC_AUTH_VT_EXL_REP_V2 *psAuthRsp)
{
	DWORD dwErr = NO_ERROR;          // Returned error code
	DB_STMT * hStmt = NULL;          // Handle de requete
	char szErr[2000] = { 0 };            // Texte d'erreur
	DB_CNX * hDbCnx = NULL;          // Database conection handle
	CHAR	szStatement[1024] = { 0 };

	DB_VAR * hDte_Ref_Exit; //:p_Dte_Ref_Exit, 
	char szDteRefExit[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hRef_Exit_Trs;//:p_Ref_Exit_Trs,
	DB_VAR * hId_Exit_Plaza;//:p_Id_Exit_Plaza,
	DB_VAR * hId_Exit_Lane;//:p_Id_Exit_Lane, 
	DB_VAR * hId_Exit_Trs;//:p_Id_Exit_Trs, 
	DB_VAR * hDte_Exit; //: p_Dte_Exit, 
	char szDteExit[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hExit_VRN;//:p_Exit_VRN, 
	DB_VAR * hExit_VRN_Country;//:p_Exit_VRN_Country, 
	DB_VAR * hEntry_Ticket_Type; //:p_Ticket_Type, 
	DB_VAR * hId_Exit_Provider;//:p_Id_Exit_Provider, 
	DB_VAR * hFlag_ETicket;//:p_Flag_ETicket,

	DB_VAR * hDte_Ref_Entry; //:p_Dte_Ref_Entry, 
	char szDteRefEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hRef_Entry_Trs; //: p_Ref_Entry_Trs, 
	DB_VAR * hId_Entry_Plaza; //: p_Id_Entry_Plaza, 
	DB_VAR * hId_Entry_Lane; //: p_Id_Entry_Lane, 
	DB_VAR * hId_Entry_Trs; //: p_Id_Entry_Trs, 
	DB_VAR * hDte_Entry; //: p_Dte_Entry, 
	char szDteEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hEntry_VRN; //: p_Entry_VRN, 
	DB_VAR * hEntry_VRN_Country; //: p_Entry_VRN_Country, 
	DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 

	DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType,
	unsigned int uiId_Payment_SubType = 0;
	DB_VAR * hId_Class; //:p_Id_Class,
	unsigned int uiId_Class = 0;
	DB_VAR * hAccepted_Days; //:p_Accepted_Days, 

	DB_VAR * hAccoun_No; //:p_Accoun_No,
	unsigned int uiAccoun_No = 0;

	DB_VAR * hUser_No; //:p_User_No,
	unsigned int uiUser_No = 0;

	DB_VAR * hRenewal_No; //:p_Renewal_No,
	unsigned int uiRenewal_No = 0;


	DB_VAR * hResult; //: p_Result, 
	unsigned int uiResult = 0;
	DB_VAR * h_id_Reason; //: p_id_Reason
	unsigned int uiIdReason = 0;
	unsigned int *puiResult = NULL;
	char *pbResult;
	DWORD	dwLen;

	// Boucle do while() utilisée pour la commodité du break. En faite, la condition
	// de bouclage est FALSE, on ne passe donc qu'une fois dedans.
	do
	{
		if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
		{
			EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);
			if (!DBOpen())
			{
				NTSVCInfo("DBAuthVT_ExlReqV2(), error reconnecting to the database");
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				break;
			}

			hDbCnx = gsSvcWork.hDbCnx;
		}
		else
		{
			// Essayer d'établir la connexion
			NTSVCInfo("DBAuthVT_ExlReqV2(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr);
			hDbCnx = DBConnect(gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd);
			// Si toujours pas connecté, on ne va pas plue long
			if (hDbCnx == NULL)
			{
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, connexion à la base impossible", dwErr);
				break;
			}
		}

		// REQUEST: Select record for requested media ID
		NTSVCInfo("DBAuthVT_ExlReqV2(), preparing the request");

		// Prepare the request
		_snprintf_s(szStatement,
			_countof(szStatement),
			sizeof(szStatement),
			DB_DEFAULT_EXL_Update_REQUEST_V2);


		// Prepare the request
		hStmt = DBOpenStatement(hDbCnx, szStatement);

		if (hStmt == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, statement AuthRequest : %s", dwErr, szErr);
			break;
		}


		hDte_Ref_Exit = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Ref_Exit",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Ref_Exit == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Dte_Ref_Exit : %s", dwErr, szErr);
			break;
		}

		hRef_Exit_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ref_Exit_Trs",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hRef_Exit_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Ref_Exit_Trs : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Plaza = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Plaza",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Exit_Plaza == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Exit_Plaza : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Lane = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Lane",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Exit_Lane == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Exit_Lane : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Trs",
			DB_TYPE_STR,
			DB_ID_ENTRY_TRS_LENGHT + 1,
			0);
		if (hId_Exit_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Exit_Trs : %s", dwErr, szErr);
			break;
		}

		hDte_Exit = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Exit",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Exit == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Dte_Exit : %s", dwErr, szErr);
			break;
		}

		hExit_VRN = DBBindPlaceHolderVariable(
			hStmt,
			":p_Exit_VRN",
			DB_TYPE_STR,
			DB_ENTRY_VRN_LENGHT + 1,
			0);
		if (hExit_VRN == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Exit_VRN : %s", dwErr, szErr);
			break;
		}

		hExit_VRN_Country = DBBindPlaceHolderVariable(
			hStmt,
			":p_Exit_VRN_Country",
			DB_TYPE_STR,
			DB_ENTRY_VRN_COUNTRY + 1,
			0);
		if (hExit_VRN_Country == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Exit_VRN_Country : %s", dwErr, szErr);
			break;
		}

		// p_Ticket_Type         IN     NUMBER,
		//DB_VAR * hEntry_Ticket_Type; //:p_Ticket_Type, 
		hEntry_Ticket_Type = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ticket_Type",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hEntry_Ticket_Type == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Ticket_Type : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Provider = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Provider",
			DB_TYPE_STR,
			DB_ENTRY_VRN_PROVIDER + 1,
			0);
		if (hId_Exit_Provider == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Exit_Provider : %s", dwErr, szErr);
			break;
		}

		hFlag_ETicket = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_ETicket",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hFlag_ETicket == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Flag_ETicket : %s", dwErr, szErr);
			break;
		}

		// Associer les variables
		hDte_Ref_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Ref_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Ref_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Dte_Ref_Entry : %s", dwErr, szErr);
			break;
		}


		hRef_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ref_Entry_Trs",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hRef_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Ref_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Plaza = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Plaza",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Plaza == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Entry_Plaza : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Lane = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Lane",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Lane == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Entry_Lane : %s", dwErr, szErr);
			break;
		}


		//DB_VAR * hId_Entry_Trs; //:p_Id_Entry_Trs, 
		hId_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Trs",
			DB_TYPE_STR,
			DB_ID_ENTRY_TRS_LENGHT + 1,
			0);
		if (hId_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hDte_Entry; //:p_Dte_Entry, 
		hDte_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Dte_Entry : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hEntry_VRN; //:p_Entry_VRN, 
		hEntry_VRN = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN",
			DB_TYPE_STR,
			DB_ENTRY_VRN_LENGHT + 1,
			0);
		if (hEntry_VRN == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Entry_VRN : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hEntry_VRN_Country; //:p_Entry_VRN_Country, 
		hEntry_VRN_Country = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN_Country",
			DB_TYPE_STR,
			DB_ENTRY_VRN_COUNTRY + 1,
			0);
		if (hEntry_VRN_Country == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Entry_VRN_Country : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hId_Entry_Provider; //:p_Id_Entry_Provider, 
		hId_Entry_Provider = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Provider",
			DB_TYPE_STR,
			DB_ENTRY_VRN_PROVIDER + 1,
			0);
		if (hId_Entry_Provider == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Entry_Provider : %s", dwErr, szErr);
			break;
		}

		// p_Id_Payment_SubType  OUT    INTEGER,
		//DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType, 
		hId_Payment_SubType = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Payment_SubType",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Payment_SubType == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Payment_SubType : %s", dwErr, szErr);
			break;
		}

		// p_Id_Class  OUT    INTEGER,
		//DB_VAR * hId_Class; //:p_Id_Class, 
		hId_Class = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Class",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Class == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Id_Class : %s", dwErr, szErr);
			break;
		}

		// p_Accepted_Days       OUT    VARCHAR2,
		//DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
		hAccepted_Days = DBBindPlaceHolderVariable(
			hStmt,
			":p_Accepted_Days",
			DB_TYPE_STR,
			DB_ENTRY_ACCEPTED_DAYS + 1,
			0);
		if (hAccepted_Days == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV2(), error %u, bind p_Accepted_Days : %s", dwErr, szErr);
			break;
		}


		// p_Accoun_No  OUT    INTEGER,
		// DB_VAR * hAccoun_No; //:p_Accoun_No 
		hAccoun_No = DBBindPlaceHolderVariable(
			hStmt,
			":p_Accoun_No",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hAccoun_No == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Accoun_No : %s", dwErr, szErr);
			break;
		}

		// p_User_No  OUT    INTEGER,
		// DB_VAR * hUser_No; //:p_User_No 
		hUser_No = DBBindPlaceHolderVariable(
			hStmt,
			":p_User_No",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hUser_No == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_User_No : %s", dwErr, szErr);
			break;
		}

		// p_Renewal_No  OUT    INTEGER,
		// DB_VAR * hRenewal_No; //:p_Renewal_No 
		hRenewal_No = DBBindPlaceHolderVariable(
			hStmt,
			":p_Renewal_No",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hRenewal_No == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Renewal_No : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hResult; //:p_Result, 
		hResult = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_Result",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hResult == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind p_Flag_Result : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * h_id_Reason; //:p_id_Reason
		h_id_Reason = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Reason",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (h_id_Reason == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, bind :p_Id_Reason : %s", dwErr, szErr);
			break;
		}

		// Initialiser les variables en input
		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_ref_exit, szDteRefExit, sizeof(szDteRefExit));
		DBSetVariableItemValue(hDte_Ref_Exit, 0, &szDteRefExit, DB_DATE_CHAR_LENGHT + 1);

		DBSetVariableItemValue(hRef_Exit_Trs, 0, &psAuthReq->body.exit_trs_ref_num, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Exit_Plaza, 0, &psAuthReq->body.exit_plaza_id, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Exit_Lane, 0, &psAuthReq->body.exit_lane_id, sizeof(unsigned int));
		DBSetVariableItemValue(hId_Exit_Trs, 0, &psAuthReq->body.exit_trs_id, DB_ID_ENTRY_TRS_LENGHT + 1);

		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_exit, szDteExit, sizeof(szDteExit));
		DBSetVariableItemValue(hDte_Exit, 0, &szDteExit, DB_DATE_CHAR_LENGHT + 1);

		DBSetVariableItemValue(hExit_VRN, 0, &psAuthReq->body.exit_vrn, DB_ENTRY_VRN_LENGHT + 1);
		DBSetVariableItemValue(hExit_VRN_Country, 0, &psAuthReq->body.exit_vrn_country, DB_ENTRY_VRN_COUNTRY + 1);
		DBSetVariableItemValue(hEntry_Ticket_Type, 0, &psAuthReq->body.ticket_type, sizeof(unsigned int));

		DBSetVariableItemValue(hFlag_ETicket, 0, &psAuthReq->body.flag_eticket, sizeof(unsigned int));

		NTSVCInfo("DBAuthVT_ExlReqV2(szDteRefExit:[%s], exit_trs_ref_num:[%u], exit_plaza_id:[%u], exit_lane_id[%u], exit_trs_id:[%s], szDteExit[%s], exit_vrn[%s], exit_vrn_country[%s], ticket_type[%u], flag_eticket:[%u]), execute request",
			szDteRefExit,
			psAuthReq->body.exit_trs_ref_num,
			psAuthReq->body.exit_plaza_id,
			psAuthReq->body.exit_lane_id,
			psAuthReq->body.exit_trs_id,
			szDteExit,
			psAuthReq->body.exit_vrn,
			psAuthReq->body.exit_vrn_country,
			psAuthReq->body.ticket_type,
			psAuthReq->body.flag_eticket);

		// Exécuter la requète
		if (!DBExecuteStatement(hStmt))
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV2(), error %u, executing: %s", dwErr, szErr);
			break;
		}

		//First get result and reason
		//: p_Flag_Result, 
		dwLen = sizeof(uiResult);
		puiResult = DBGetVariableItemValue(hResult, 0, &uiResult, &dwLen);
		if (puiResult != &uiResult && dwLen == sizeof(uiResult))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Flag_Result");
			break;
		}

		//:p_Id_Reason); COMMIT; END; "
		dwLen = sizeof(uiIdReason);
		puiResult = DBGetVariableItemValue(h_id_Reason, 0, &uiIdReason, &dwLen);
		if (puiResult != &uiIdReason && dwLen == sizeof(uiIdReason))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Id_Reason");
			break;
		}

// 		//Do not try to read other OUT variables in case of any error...
// 		if (dwErr != NO_ERROR || uiResult == AUTH_NOK)
// 			break;
		
		// Get database data	

		// :p_Id_Exit_Provider 
		pbResult = DBGetVariableItemValue(hId_Exit_Provider, 0, psAuthRsp->body.exit_provider, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Id_Exit_Provider == NULL");
		}
		else if (pbResult != psAuthRsp->body.exit_provider)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Id_Exit_Provider");
			break;
		}


		//":p_Dte_Ref_Entry, 
		pbResult = DBGetVariableItemValue(hDte_Ref_Entry, 0, szDteRefEntry, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Dte_Ref_Entry == NULL");
		}
		else if (pbResult != szDteRefEntry)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Dte_Ref_Entry");
			break;
		}
		else
		{
			if (strlen(szDteRefEntry) > 0)
				if (FALSE == DB_ConvertStr_ToSYSTEMTIME(szDteRefEntry, &psAuthRsp->body.dte_ref_entry))
				{
					NTSVCInfo("DBAuthVT_ExlReqV2(), Error DB_ConvertStr_ToSYSTEMTIME from szDteRefEntry [%s]", szDteRefEntry);
				}
		}

		//:p_Ref_Entry_Trs, 		dwLen = sizeof(psAuthRsp->body.entry_trs_ref_num);
		dwLen = sizeof(psAuthRsp->body.entry_trs_ref_num);
		puiResult = DBGetVariableItemValue(hRef_Entry_Trs, 0, &psAuthRsp->body.entry_trs_ref_num, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Ref_Entry_Trs == NULL");
		}
		else if (puiResult != &psAuthRsp->body.entry_trs_ref_num)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Ref_Entry_Trs");
			break;
		}

		//:p_Id_Entry_Plaza, 
		dwLen = sizeof(psAuthRsp->body.entry_plaza_id);
		puiResult = DBGetVariableItemValue(hId_Entry_Plaza, 0, &psAuthRsp->body.entry_plaza_id, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Id_Entry_Plaza == NULL");
		}
		else if (puiResult != &psAuthRsp->body.entry_plaza_id)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Id_Entry_Plaza");
			break;
		}

		//DB_VAR * hId_Entry_Lane; //: p_Id_Entry_Lane, 
		dwLen = sizeof(psAuthRsp->body.entry_lane_id);
		puiResult = DBGetVariableItemValue(hId_Entry_Lane, 0, &psAuthRsp->body.entry_lane_id, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Id_Entry_Lane == NULL");
		}
		else if (puiResult != &psAuthRsp->body.entry_lane_id)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Id_Entry_Lane");
			break;
		}

		//DB_VAR * hId_Entry_Trs; //: p_Id_Entry_Trs, 
		pbResult = DBGetVariableItemValue(hId_Entry_Trs, 0, psAuthRsp->body.entry_trs_id, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Id_Entry_Trs == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_trs_id)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Id_Entry_Trs");
			break;
		}

		//DB_VAR * hDte_Entry; //: p_Dte_Entry, 
		//char szDteEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
		pbResult = DBGetVariableItemValue(hDte_Entry, 0, szDteEntry, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Dte_Entry == NULL");
		}
		else if (pbResult != szDteEntry)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Dte_Entry");
			break;
		}
		else
		{	
			if (strlen(szDteEntry)>0)
				if (FALSE == DB_ConvertStr_ToSYSTEMTIME(szDteEntry, &psAuthRsp->body.dte_entry))
				{
					NTSVCInfo("DBAuthVT_ExlReqV2(), Error DB_ConvertStr_ToSYSTEMTIME from szDteEntry [%s]", szDteEntry);
				}
		}

		//DB_VAR * hEntry_VRN; //: p_Entry_VRN, 
		pbResult = DBGetVariableItemValue(hEntry_VRN, 0, psAuthRsp->body.entry_vrn, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Entry_VRN == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_vrn)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Entry_VRN");
			break;
		}


		//DB_VAR * hEntry_VRN_Country; //: p_Entry_VRN_Country, 
		pbResult = DBGetVariableItemValue(hEntry_VRN_Country, 0, psAuthRsp->body.entry_vrn_country, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Entry_VRN_Country == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_vrn_country)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Entry_VRN_Country");
			break;
		}

		//DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
		pbResult = DBGetVariableItemValue(hId_Entry_Provider, 0, psAuthRsp->body.entry_provider, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Id_Entry_Provider == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_provider)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV2(), error getting p_Id_Entry_Provider");
			break;
		}

		// p_Id_Payment_SubType  OUT    INTEGER,
		//DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType,
		dwLen = sizeof(uiId_Payment_SubType);
		puiResult = DBGetVariableItemValue(hId_Payment_SubType, 0, &uiId_Payment_SubType, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Id_Payment_SubType == NULL");
			uiId_Payment_SubType = 0;
		}
		else if (puiResult != &uiId_Payment_SubType && dwLen == sizeof(uiId_Payment_SubType))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting p_Id_Payment_SubType");
			break;
		}

		psAuthRsp->body.payment_subtype = uiId_Payment_SubType;

		// p_Id_Class            OUT    INTEGER,
		//DB_VAR * hId_Class; //:p_Id_Class,
		dwLen = sizeof(uiId_Class);
		puiResult = DBGetVariableItemValue(hId_Class, 0, &uiId_Class, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Id_Class == NULL");
			uiId_Class = 0;
		}
		else if (puiResult != &uiId_Class && dwLen == sizeof(uiId_Class))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting p_Id_Class");
			break;
		}

		psAuthRsp->body.vehicle_class = uiId_Class;

		// p_Accepted_Days       OUT    VARCHAR2,
		//DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
		pbResult = DBGetVariableItemValue(hAccepted_Days, 0, psAuthRsp->body.accepted_days, NULL);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Accepted_Days == NULL");

		}
		else if (pbResult != psAuthRsp->body.accepted_days)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReq(), error getting p_Accepted_Days");
			break;
		}

		// p_Accoun_No            OUT    INTEGER,
		//DB_VAR * hAccoun_No; //:p_Accoun_No,
		dwLen = sizeof(uiAccoun_No);
		puiResult = DBGetVariableItemValue(hAccoun_No, 0, &uiAccoun_No, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Accoun_No == NULL");
			uiAccoun_No = 0;
		}
		else if (puiResult != &uiAccoun_No && dwLen == sizeof(uiAccoun_No))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting p_Accoun_No");
			break;
		}
		psAuthRsp->body.exempt_acccount_number = uiAccoun_No;

		// p_User_No            OUT    INTEGER,
		//DB_VAR * hUser_No; //:p_User_No,
		dwLen = sizeof(uiUser_No);
		puiResult = DBGetVariableItemValue(hUser_No, 0, &uiUser_No, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_User_No == NULL");
			uiUser_No = 0;
		}
		else if (puiResult != &uiUser_No && dwLen == sizeof(uiUser_No))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting p_User_No");
			break;
		}
		psAuthRsp->body.exempt_user_number = uiUser_No;

		// p_Renewal_No            OUT    INTEGER,
		//DB_VAR * hRenewal_No; //:p_Renewal_No,
		dwLen = sizeof(uiRenewal_No);
		puiResult = DBGetVariableItemValue(hRenewal_No, 0, &uiRenewal_No, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV2(), p_Renewal_No == NULL");
			uiRenewal_No = 0;
		}
		else if (puiResult != &uiRenewal_No && dwLen == sizeof(uiRenewal_No))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV2(), error getting p_Renewal_No");
			break;
		}
		psAuthRsp->body.exempt_renewal_number = uiRenewal_No;


	} while (FALSE);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_ExlReqV2(), Error getting data %d", dwErr);

		// 3113 - Database down during statement exec
		// ERROR_PIPE_NOT_CONNECTED - Database down before connect
		psAuthRsp->body.flag_result = AUTH_NOK;
		psAuthRsp->body.reason_id = dwErr;
	}
	else //	if ( dwErr == NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_ExlReqV2() - Result:%s Reason:%d", (uiResult == AUTH_NOK) ? "AUTH_NOK" : "AUTH_OK", uiIdReason);
		psAuthRsp->body.flag_result = uiResult;

		if (uiResult == AUTH_NOK)
			psAuthRsp->body.reason_id = uiIdReason;
		else
			psAuthRsp->body.reason_id = 0;
	}

	if (hStmt != NULL)
		DBCloseStatement(hStmt);

	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

	}
	else if (hDbCnx != NULL) // && gsSvcWork.sParmWork.dwDbKeepConnection == FALSE)
	{
		NTSVCInfo("DBAuthVT_ExlReqV2(), disconnect from the database");
		DBDisconnect(hDbCnx);
	}

	return dwErr;
}

//---------------------------------------------------------------------------------------
// V3 messages...
//---------------------------------------------------------------------------------------

PROTECTED DWORD DBAuthVT_EnlReqV3(struct MSG_LC_AUTH_VT_ENL_REQ_V3 *psAuthReq, struct MSG_LC_AUTH_VT_ENL_REP_V3 *psAuthRsp)
{
	DWORD dwErr = NO_ERROR;          // Returned error code
	DB_STMT * hStmt = NULL;          // Handle de requete
	char szErr[2000] = { 0 };            // Texte d'erreur
	DB_CNX * hDbCnx = NULL;          // Database conection handle
	CHAR	szStatement[1024] = { 0 };
	DB_VAR * hDte_Ref_Entry; //:p_Dte_Ref_Entry, 
	char szDteRefEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hRef_Entry_Trs; //: p_Ref_Entry_Trs, 
	DB_VAR * hId_Entry_Plaza; //: p_Id_Entry_Plaza, 
	DB_VAR * hId_Entry_Lane; //: p_Id_Entry_Lane, 
	DB_VAR * hId_Entry_Trs; //: p_Id_Entry_Trs, 
	DB_VAR * hDte_Entry; //: p_Dte_Entry, 
	char szDteEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hEntry_VRN; //: p_Entry_VRN, 
	DB_VAR * hEntry_VRN_Country; //: p_Entry_VRN_Country, 
	DB_VAR * hEntry_Ticket_Type; //:p_Ticket_Type, 
	DB_VAR * hEntry_Flag_Delete; //:p_Flag_delete, 
	DB_VAR * hEntry_Flag_No_Insert; //:p_Flag_No_Insert, 
	DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
	DB_VAR * hId_Contract_Type; //:p_Id_Contract_Type,
	unsigned int uiId_Contract_Type = 0;
	DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType,
	unsigned int uiId_Payment_SubType = 0;
	DB_VAR * hId_Class; //:p_Id_Class,
	unsigned int uiId_Class = 0;
	DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
	DB_VAR * hAccount_Balance; //:p_Account_Balance,
	unsigned int uiAccount_Balance = 0;
	DB_VAR * hUser_Balance; //:p_User_Balance,
	int uiUser_Balance = 0;
	DB_VAR * hResult; //: p_Result, 
	unsigned int uiResult = 0;
	DB_VAR * h_id_Reason; //: p_id_Reason
	unsigned int uiIdReason = 0;
	unsigned int *puiResult = NULL;
	char *pbResult;
	DWORD	dwLen;

	// Boucle do while() utilisée pour la commodité du break. En faite, la condition
	// de bouclage est FALSE, on ne passe donc qu'une fois dedans.
	do
	{
		if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
		{
			EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);
			if (!DBOpen())
			{
				NTSVCInfo("DBAuthVT_EnlReqV3(), error reconnecting to the database");
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				break;
			}

			hDbCnx = gsSvcWork.hDbCnx;
		}
		else
		{
			// Essayer d'établir la connexion
			NTSVCInfo("DBAuthVT_EnlReqV3(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr);
			hDbCnx = DBConnect(gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd);
			// Si toujours pas connecté, on ne va pas plue long
			if (hDbCnx == NULL)
			{
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, connexion à la base impossible", dwErr);
				break;
			}
		}

		// REQUEST: Select record for requested media ID
		NTSVCInfo("DBAuthVT_EnlReqV3(), preparing the request");

		// Prepare the request
		_snprintf_s(szStatement,
			_countof(szStatement),
			sizeof(szStatement),
			DB_DEFAULT_ENL_Insert_REQUEST_V3);


		// Prepare the request
		hStmt = DBOpenStatement(hDbCnx, szStatement);

		if (hStmt == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, statement AuthRequest : %s", dwErr, szErr);
			break;
		}

		// Associer les variables
		hDte_Ref_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Ref_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Ref_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Dte_Ref_Entry : %s", dwErr, szErr);
			break;
		}


		hRef_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ref_Entry_Trs",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hRef_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Ref_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Plaza = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Plaza",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Plaza == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Id_Entry_Plaza : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Lane = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Lane",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Lane == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Id_Entry_Lane : %s", dwErr, szErr);
			break;
		}


		//DB_VAR * hId_Entry_Trs; //:p_Id_Entry_Trs, 
		hId_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Trs",
			DB_TYPE_STR,
			DB_ID_ENTRY_TRS_LENGHT_V3 + 1,
			0);
		if (hId_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Id_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hDte_Entry; //:p_Dte_Entry, 
		hDte_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Dte_Entry : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hEntry_VRN; //:p_Entry_VRN, 
		hEntry_VRN = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN",
			DB_TYPE_STR,
			DB_ENTRY_VRN_LENGHT + 1,
			0);
		if (hEntry_VRN == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Entry_VRN : %s", dwErr, szErr);
			break;
		}

		// p_Entry_VRN_Country   IN     VARCHAR2,
		//DB_VAR * hEntry_VRN_Country; //:p_Entry_VRN_Country, 
		hEntry_VRN_Country = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN_Country",
			DB_TYPE_STR,
			DB_ENTRY_VRN_COUNTRY + 1,
			0);
		if (hEntry_VRN_Country == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Entry_VRN_Country : %s", dwErr, szErr);
			break;
		}

		// p_Ticket_Type         IN     NUMBER,
		//DB_VAR * hEntry_Ticket_Type; //:p_Ticket_Type, 
		hEntry_Ticket_Type = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ticket_Type",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hEntry_Ticket_Type == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Ticket_Type : %s", dwErr, szErr);
			break;
		}

		// p_Flag_Delete         IN     NUMBER,
		//DB_VAR * hEntry_Flag_Delete; //:p_Flag_Delete, 
		hEntry_Flag_Delete = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_Delete",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hEntry_Flag_Delete == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Flag_Delete : %s", dwErr, szErr);
			break;
		}

		// p_Flag_No_Insert         IN     NUMBER,
		//DB_VAR * hEntry_Flag_Delete; //:p_Flag_No_Insert, 
		hEntry_Flag_No_Insert = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_No_Insert",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hEntry_Flag_No_Insert == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Flag_No_Insert : %s", dwErr, szErr);
			break;
		}

		// p_Id_Entry_Provider   OUT    VARCHAR2,
		//DB_VAR * hId_Entry_Provider; //:p_Id_Entry_Provider, 
		hId_Entry_Provider = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Provider",
			DB_TYPE_STR,
			DB_ENTRY_VRN_PROVIDER + 1,
			0);
		if (hId_Entry_Provider == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Id_Entry_Provider : %s", dwErr, szErr);
			break;
		}

		// p_Id_Contract_Type OUT    INTEGER,
		//DB_VAR * hId_Contract_Type; //:p_Id_Contract_Type, 
		hId_Contract_Type = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Contract_Type",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Contract_Type == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Id_Contract_Type : %s", dwErr, szErr);
			break;
		}

		// p_Id_Payment_SubType OUT    INTEGER,
		//DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType, 
		hId_Payment_SubType = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Payment_SubType",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Payment_SubType == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Id_Payment_SubType : %s", dwErr, szErr);
			break;
		}

		// p_Id_Class            OUT    INTEGER,
		//DB_VAR * hId_Class; //:p_Id_Class, 
		hId_Class = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Class",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Class == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Id_Class : %s", dwErr, szErr);
			break;
		}

		// p_Accepted_Days       OUT    VARCHAR2,
		//DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
		hAccepted_Days = DBBindPlaceHolderVariable(
			hStmt,
			":p_Accepted_Days",
			DB_TYPE_STR,
			DB_ENTRY_ACCEPTED_DAYS + 1,
			0);
		if (hAccepted_Days == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Accepted_Days : %s", dwErr, szErr);
			break;
		}

		// p_Account_Balance            OUT    INTEGER,
		//DB_VAR * hAccount_Balance; //:p_Account_Balance, 
		hAccount_Balance = DBBindPlaceHolderVariable(
			hStmt,
			":p_Account_Balance",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hAccount_Balance == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Account_Balance : %s", dwErr, szErr);
			break;
		}

		// p_User_Balance            OUT    INTEGER,
		//DB_VAR * hUser_Balance; //:p_User_Balance, 
		hUser_Balance = DBBindPlaceHolderVariable(
			hStmt,
			":p_User_Balance",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hUser_Balance == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_User_Balance : %s", dwErr, szErr);
			break;
		}


		// p_Result              OUT    INTEGER,
		//DB_VAR * hResult; //:p_Result, 
		hResult = DBBindPlaceHolderVariable(
			hStmt,
			":p_Result",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hResult == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Result : %s", dwErr, szErr);
			break;
		}

		// p_Id_Reason           OUT    INTEGER) 
		//DB_VAR * h_id_Reason; //:p_id_Reason
		h_id_Reason = DBBindPlaceHolderVariable(
			hStmt,
			":p_id_Reason",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (h_id_Reason == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_id_Reason : %s", dwErr, szErr);
			break;
		}

		// Initialiser les variables en input
		//DBSetVariableItemValue( hMediaNum, 0, &psAutResp->szMediaNum, AUTHSVC_DB_MEDIA_NUM_MAX_LENTG );
		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_ref_entry, szDteRefEntry, sizeof(szDteRefEntry));
		DBSetVariableItemValue(hDte_Ref_Entry, 0, &szDteRefEntry, DB_DATE_CHAR_LENGHT + 1);
		NTSVCInfo("DBAuthVT_EnlReqV3(), hDte_Ref_Entry [%s]", szDteRefEntry);

		DBSetVariableItemValue(hRef_Entry_Trs, 0, &psAuthReq->body.entry_trs_ref_num, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_EnlReqV3(), hRef_Entry_Trs [%d]", psAuthReq->body.entry_trs_ref_num);

		DBSetVariableItemValue(hId_Entry_Plaza, 0, &psAuthReq->body.entry_plaza_id, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_EnlReqV3(), hId_Entry_Plaza [%d]", psAuthReq->body.entry_plaza_id);

		DBSetVariableItemValue(hId_Entry_Lane, 0, &psAuthReq->body.entry_lane_id, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_EnlReqV3(), hId_Entry_Lane [%d]", psAuthReq->body.entry_lane_id);

		DBSetVariableItemValue(hId_Entry_Trs, 0, &psAuthReq->body.entry_trs_id, DB_ID_ENTRY_TRS_LENGHT_V3 + 1);
		NTSVCInfo("DBAuthVT_EnlReqV3(), hId_Entry_Trs [%s]", psAuthReq->body.entry_trs_id);

		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_entry, szDteEntry, sizeof(szDteEntry));
		DBSetVariableItemValue(hDte_Entry, 0, &szDteEntry, DB_DATE_CHAR_LENGHT + 1);
		NTSVCInfo("DBAuthVT_EnlReqV3(), hDte_Entry [%s]", szDteEntry);

		DBSetVariableItemValue(hEntry_VRN, 0, &psAuthReq->body.entry_vrn, DB_ENTRY_VRN_LENGHT + 1);
		NTSVCInfo("DBAuthVT_EnlReqV3(), hEntry_VRN [%s]", psAuthReq->body.entry_vrn);

		DBSetVariableItemValue(hEntry_VRN_Country, 0, &psAuthReq->body.entry_vrn_country, DB_ENTRY_VRN_COUNTRY + 1);
		NTSVCInfo("DBAuthVT_EnlReqV3(), hEntry_VRN_Country [%s]", psAuthReq->body.entry_vrn_country);

		DBSetVariableItemValue(hEntry_Ticket_Type, 0, &psAuthReq->body.ticket_type, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_EnlReqV3(), hEntry_Ticket_Type [%d]", psAuthReq->body.ticket_type);

		DBSetVariableItemValue(hEntry_Flag_Delete, 0, &psAuthReq->body.flag_delete, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_EnlReqV3(), hEntry_Flag_Delete [%d]", psAuthReq->body.flag_delete);

		DBSetVariableItemValue(hEntry_Flag_No_Insert, 0, &psAuthReq->body.flag_no_insert, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_EnlReqV3(), hEntry_Flag_No_Insert [%d]", psAuthReq->body.flag_no_insert);


		NTSVCInfo("DBAuthVT_EnlReqV3(), execute request");

		// Exécuter la requète
		if (!DBExecuteStatement(hStmt))
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, executing: %s", dwErr, szErr);
			break;
		}

		// Get database data
		dwLen = sizeof(uiResult);
		puiResult = DBGetVariableItemValue(hResult, 0, &uiResult, &dwLen);
		if (puiResult != &uiResult && dwLen == sizeof(uiResult))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting uiResult");
			break;
		}

		dwLen = sizeof(uiIdReason);
		puiResult = DBGetVariableItemValue(h_id_Reason, 0, &uiIdReason, &dwLen);
		if (puiResult != &uiIdReason && dwLen == sizeof(uiIdReason))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting uiIdReason");
			break;
		}

		// 		//Do not try to read other OUT variables in case of any error...
		// 		if (dwErr != NO_ERROR || uiResult == AUTH_NOK)
		// 			break;

		//DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
		pbResult = DBGetVariableItemValue(hId_Entry_Provider, 0, psAuthRsp->body.entry_provider, NULL);

		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV3(), p_Id_Entry_Provider == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_provider)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Id_Entry_Provider");
			break;
		}


		// p_Id_Contract_Type  OUT    INTEGER,
		//DB_VAR * hId_Contract_Type; //:p_Id_Contract_Type,
		dwLen = sizeof(uiId_Contract_Type);
		puiResult = DBGetVariableItemValue(hId_Contract_Type, 0, &uiId_Contract_Type, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV3(), p_Id_Contract_Type == NULL");
			uiId_Contract_Type = 0;
		}
		else if (puiResult != &uiId_Contract_Type && dwLen == sizeof(uiId_Contract_Type))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Id_Contract_Type");
			break;
		}
		psAuthRsp->body.contract_type = uiId_Contract_Type;


		// p_Id_Payment_SubType  OUT    INTEGER,
		//DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType,
		dwLen = sizeof(uiId_Payment_SubType);
		puiResult = DBGetVariableItemValue(hId_Payment_SubType, 0, &uiId_Payment_SubType, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV3(), p_Id_Payment_SubType == NULL");
			uiId_Payment_SubType = 0;
		}
		else if (puiResult != &uiId_Payment_SubType && dwLen == sizeof(uiId_Payment_SubType))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Id_Payment_SubType");
			break;
		}
		psAuthRsp->body.payment_subtype = uiId_Payment_SubType;


		// p_Id_Class            OUT    INTEGER,
		//DB_VAR * hId_Class; //:p_Id_Class,
		dwLen = sizeof(uiId_Class);
		puiResult = DBGetVariableItemValue(hId_Class, 0, &uiId_Class, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV3(), p_Id_Class == NULL");
			uiId_Class = 0;
		}
		else if (puiResult != &uiId_Class && dwLen == sizeof(uiId_Class))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Id_Class");
			break;
		}
		psAuthRsp->body.vehicle_class = uiId_Class;


		// p_Accepted_Days       OUT    VARCHAR2,
		//DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
		pbResult = DBGetVariableItemValue(hAccepted_Days, 0, psAuthRsp->body.accepted_days, NULL);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV3(), p_Accepted_Days == NULL");

		}
		else if (pbResult != psAuthRsp->body.accepted_days)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Accepted_Days");
			break;
		}


		// p_Account_Balance            OUT    INTEGER,
		//DB_VAR * hAccount_Balance; //:p_Account_Balance,
		dwLen = sizeof(uiAccount_Balance);
		puiResult = DBGetVariableItemValue(hAccount_Balance, 0, &uiAccount_Balance, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV3(), p_Account_Balance == NULL");
			uiAccount_Balance = 0;
		}
		else if (puiResult != &uiAccount_Balance && dwLen == sizeof(uiAccount_Balance))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Account_Balance");
			break;
		}
//		psAuthRsp->body.account_balance = uiAccount_Balance;
		_itoa_s(uiAccount_Balance, psAuthRsp->body.account_balance, sizeof(psAuthRsp->body.account_balance), 10);


		// p_User_Balance            OUT    INTEGER,
		//DB_VAR * hUser_Balance; //:p_User_Balance,
		dwLen = sizeof(uiUser_Balance);
		puiResult = DBGetVariableItemValue(hUser_Balance, 0, &uiUser_Balance, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_EnlReqV3(), p_User_Balance == NULL");
			uiUser_Balance = 0;
		}
		else if (puiResult != &uiUser_Balance && dwLen == sizeof(uiUser_Balance))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_User_Balance");
			break;
		}
//		psAuthRsp->body.user_balance = uiUser_Balance;
		_itoa_s(uiUser_Balance, psAuthRsp->body.user_balance, sizeof(psAuthRsp->body.user_balance), 10);


		NTSVCInfo("DBAuthVT_EnlReqV3(), result:%d reason:%d", uiResult, uiIdReason);
	} while (FALSE);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_EnlReqV3(), Error getting data %d", dwErr);

		// 3113 - Database down during statement exec
		// ERROR_PIPE_NOT_CONNECTED - Database down before connect
		psAuthRsp->body.flag_result = AUTH_NOK;
		psAuthRsp->body.reason_id = dwErr;

	}
	else //	if ( dwErr == NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_EnlReqV3() - Result:%s Reason:%d", (uiResult == AUTH_NOK) ? "AUTH_NOK" : "AUTH_OK", uiIdReason);
		psAuthRsp->body.flag_result = uiResult;

		if (uiResult == AUTH_NOK)
			psAuthRsp->body.reason_id = uiIdReason;
		else
			psAuthRsp->body.reason_id = 0;
	}

	if (hStmt != NULL)
		DBCloseStatement(hStmt);

	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

	}
	else if (hDbCnx != NULL) // && gsSvcWork.sParmWork.dwDbKeepConnection == FALSE)
	{
		NTSVCInfo("DBAuthVT_EnlReqV3(), disconnect from the database");
		DBDisconnect(hDbCnx);
	}

	return dwErr;
}

PROTECTED DWORD DBAuthVT_ExlReqV3(struct MSG_LC_AUTH_VT_EXL_REQ_V3 *psAuthReq, struct MSG_LC_AUTH_VT_EXL_REP_V3 *psAuthRsp)
{
	DWORD dwErr = NO_ERROR;          // Returned error code
	DB_STMT * hStmt = NULL;          // Handle de requete
	char szErr[2000] = { 0 };            // Texte d'erreur
	DB_CNX * hDbCnx = NULL;          // Database conection handle
	CHAR	szStatement[1024] = { 0 };

	DB_VAR * hDte_Ref_Exit; //:p_Dte_Ref_Exit, 
	char szDteRefExit[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hRef_Exit_Trs;//:p_Ref_Exit_Trs,
	DB_VAR * hId_Exit_Plaza;//:p_Id_Exit_Plaza,
	DB_VAR * hId_Exit_Lane;//:p_Id_Exit_Lane, 
	DB_VAR * hId_Exit_Trs;//:p_Id_Exit_Trs, 
	DB_VAR * hDte_Exit; //: p_Dte_Exit, 
	char szDteExit[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hExit_VRN;//:p_Exit_VRN, 
	DB_VAR * hExit_VRN_Country;//:p_Exit_VRN_Country, 
	DB_VAR * hEntry_Ticket_Type; //:p_Ticket_Type, 
	DB_VAR * hId_Exit_Provider;//:p_Id_Exit_Provider, 
	DB_VAR * hFlag_ETicket;//:p_Flag_ETicket,
	DB_VAR * hFlag_Delete;//:p_Flag_Delete,
	DB_VAR * hFlag_Open_System;//:p_Flag_Open_System,
	DB_VAR * hFlag_No_Update;//:p_Flag_No_Update,
	DB_VAR * hDte_Ref_Entry; //:p_Dte_Ref_Entry, 
	char szDteRefEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hRef_Entry_Trs; //: p_Ref_Entry_Trs, 
	DB_VAR * hId_Entry_Plaza; //: p_Id_Entry_Plaza, 
	DB_VAR * hId_Entry_Lane; //: p_Id_Entry_Lane, 
	DB_VAR * hId_Entry_Trs; //: p_Id_Entry_Trs, 
	DB_VAR * hDte_Entry; //: p_Dte_Entry, 
	char szDteEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
	DB_VAR * hEntry_VRN; //: p_Entry_VRN, 
	DB_VAR * hEntry_VRN_Country; //: p_Entry_VRN_Country, 
	DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
	DB_VAR * hId_Contract_Type; //:p_Id_Contract_Type,
	unsigned int uiId_Contract_Type = 0;
	DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType,
	unsigned int uiId_Payment_SubType = 0;
	DB_VAR * hId_Class; //:p_Id_Class,
	unsigned int uiId_Class = 0;
	DB_VAR * hAccepted_Days; //:p_Accepted_Days, 

	DB_VAR * hAccoun_No; //:p_Accoun_No,
	unsigned int uiAccoun_No = 0;

	DB_VAR * hUser_No; //:p_User_No,
	unsigned int uiUser_No = 0;

	DB_VAR * hRenewal_No; //:p_Renewal_No,
	unsigned int uiRenewal_No = 0;

	DB_VAR * hAccount_Balance; //:p_Account_Balance,
	unsigned int uiAccount_Balance = 0;
	DB_VAR * hUser_Balance; //:p_User_Balance,
	int uiUser_Balance = 0;


	DB_VAR * hResult; //: p_Result, 
	unsigned int uiResult = 0;
	DB_VAR * h_id_Reason; //: p_id_Reason
	unsigned int uiIdReason = 0;
	unsigned int *puiResult = NULL;
	DB_VAR * hDiscountGroup; //:p_Discount_Group,
	unsigned int uiDiscountGroup = 0;

	char *pbResult;
	DWORD	dwLen;

	// Boucle do while() utilisée pour la commodité du break. En faite, la condition
	// de bouclage est FALSE, on ne passe donc qu'une fois dedans.
	do
	{
		if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
		{
			EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);
			if (!DBOpen())
			{
				NTSVCInfo("DBAuthVT_ExlReqV3(), error reconnecting to the database");
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				break;
			}

			hDbCnx = gsSvcWork.hDbCnx;
		}
		else
		{
			// Essayer d'établir la connexion
			NTSVCInfo("DBAuthVT_ExlReqV3(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr);
			hDbCnx = DBConnect(gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd);
			// Si toujours pas connecté, on ne va pas plue long
			if (hDbCnx == NULL)
			{
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, connexion à la base impossible", dwErr);
				break;
			}
		}

		// REQUEST: Select record for requested media ID
		NTSVCInfo("DBAuthVT_ExlReqV3(), preparing the request");

		// Prepare the request
		_snprintf_s(szStatement,
			_countof(szStatement),
			sizeof(szStatement),
			DB_DEFAULT_EXL_Update_REQUEST_V3);


		// Prepare the request
		hStmt = DBOpenStatement(hDbCnx, szStatement);

		if (hStmt == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, statement AuthRequest : %s", dwErr, szErr);
			break;
		}


		hDte_Ref_Exit = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Ref_Exit",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Ref_Exit == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Dte_Ref_Exit : %s", dwErr, szErr);
			break;
		}

		hRef_Exit_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ref_Exit_Trs",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hRef_Exit_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Ref_Exit_Trs : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Plaza = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Plaza",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Exit_Plaza == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Exit_Plaza : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Lane = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Lane",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Exit_Lane == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Exit_Lane : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Trs",
			DB_TYPE_STR,
			DB_ID_ENTRY_TRS_LENGHT_V3 + 1,
			0);
		if (hId_Exit_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Exit_Trs : %s", dwErr, szErr);
			break;
		}

		hDte_Exit = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Exit",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Exit == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Dte_Exit : %s", dwErr, szErr);
			break;
		}

		hExit_VRN = DBBindPlaceHolderVariable(
			hStmt,
			":p_Exit_VRN",
			DB_TYPE_STR,
			DB_ENTRY_VRN_LENGHT + 1,
			0);
		if (hExit_VRN == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Exit_VRN : %s", dwErr, szErr);
			break;
		}

		hExit_VRN_Country = DBBindPlaceHolderVariable(
			hStmt,
			":p_Exit_VRN_Country",
			DB_TYPE_STR,
			DB_ENTRY_VRN_COUNTRY + 1,
			0);
		if (hExit_VRN_Country == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Exit_VRN_Country : %s", dwErr, szErr);
			break;
		}

		// p_Ticket_Type         IN     NUMBER,
		//DB_VAR * hEntry_Ticket_Type; //:p_Ticket_Type, 
		hEntry_Ticket_Type = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ticket_Type",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hEntry_Ticket_Type == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Ticket_Type : %s", dwErr, szErr);
			break;
		}

		hId_Exit_Provider = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Exit_Provider",
			DB_TYPE_STR,
			DB_ENTRY_VRN_PROVIDER + 1,
			0);
		if (hId_Exit_Provider == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Exit_Provider : %s", dwErr, szErr);
			break;
		}

		hFlag_ETicket = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_ETicket",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hFlag_ETicket == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Flag_ETicket : %s", dwErr, szErr);
			break;
		}

		hFlag_Delete = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_Delete",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hFlag_Delete == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Flag_Delete : %s", dwErr, szErr);
			break;
		}

		hFlag_Open_System = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_Open_System",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hFlag_Open_System == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Flag_Open_System : %s", dwErr, szErr);
			break;
		}

		hFlag_No_Update = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_No_Update",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hFlag_No_Update == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Flag_No_Update : %s", dwErr, szErr);
			break;
		}

		// Associer les variables
		hDte_Ref_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Ref_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Ref_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Dte_Ref_Entry : %s", dwErr, szErr);
			break;
		}


		hRef_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Ref_Entry_Trs",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hRef_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Ref_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Plaza = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Plaza",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Plaza == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Entry_Plaza : %s", dwErr, szErr);
			break;
		}

		hId_Entry_Lane = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Lane",
			DB_TYPE_INT,
			sizeof(DWORD),
			0);
		if (hId_Entry_Lane == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Entry_Lane : %s", dwErr, szErr);
			break;
		}


		//DB_VAR * hId_Entry_Trs; //:p_Id_Entry_Trs, 
		hId_Entry_Trs = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Trs",
			DB_TYPE_STR,
			DB_ID_ENTRY_TRS_LENGHT_V3 + 1,
			0);
		if (hId_Entry_Trs == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Entry_Trs : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hDte_Entry; //:p_Dte_Entry, 
		hDte_Entry = DBBindPlaceHolderVariable(
			hStmt,
			":p_Dte_Entry",
			DB_TYPE_STR,
			DB_DATE_CHAR_LENGHT + 1,
			0);
		if (hDte_Entry == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Dte_Entry : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hEntry_VRN; //:p_Entry_VRN, 
		hEntry_VRN = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN",
			DB_TYPE_STR,
			DB_ENTRY_VRN_LENGHT + 1,
			0);
		if (hEntry_VRN == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Entry_VRN : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hEntry_VRN_Country; //:p_Entry_VRN_Country, 
		hEntry_VRN_Country = DBBindPlaceHolderVariable(
			hStmt,
			":p_Entry_VRN_Country",
			DB_TYPE_STR,
			DB_ENTRY_VRN_COUNTRY + 1,
			0);
		if (hEntry_VRN_Country == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Entry_VRN_Country : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hId_Entry_Provider; //:p_Id_Entry_Provider, 
		hId_Entry_Provider = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Entry_Provider",
			DB_TYPE_STR,
			DB_ENTRY_VRN_PROVIDER + 1,
			0);
		if (hId_Entry_Provider == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Entry_Provider : %s", dwErr, szErr);
			break;
		}

		// p_Id_Contract_Type  OUT    INTEGER,
		//DB_VAR * hId_Contract_Type; //:p_Id_Contract_Type, 
		hId_Contract_Type = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Contract_Type",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Contract_Type == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Contract_Type : %s", dwErr, szErr);
			break;
		}

		// p_Id_Payment_SubType  OUT    INTEGER,
		//DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType, 
		hId_Payment_SubType = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Payment_SubType",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Payment_SubType == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Payment_SubType : %s", dwErr, szErr);
			break;
		}

		// p_Id_Class  OUT    INTEGER,
		//DB_VAR * hId_Class; //:p_Id_Class, 
		hId_Class = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Class",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hId_Class == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Id_Class : %s", dwErr, szErr);
			break;
		}

		// p_Accepted_Days       OUT    VARCHAR2,
		//DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
		hAccepted_Days = DBBindPlaceHolderVariable(
			hStmt,
			":p_Accepted_Days",
			DB_TYPE_STR,
			DB_ENTRY_ACCEPTED_DAYS + 1,
			0);
		if (hAccepted_Days == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_EnlReqV3(), error %u, bind p_Accepted_Days : %s", dwErr, szErr);
			break;
		}


		// p_Accoun_No  OUT    INTEGER,
		// DB_VAR * hAccoun_No; //:p_Accoun_No 
		hAccoun_No = DBBindPlaceHolderVariable(
			hStmt,
			":p_Accoun_No",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hAccoun_No == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Accoun_No : %s", dwErr, szErr);
			break;
		}

		// p_User_No  OUT    INTEGER,
		// DB_VAR * hUser_No; //:p_User_No 
		hUser_No = DBBindPlaceHolderVariable(
			hStmt,
			":p_User_No",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hUser_No == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_User_No : %s", dwErr, szErr);
			break;
		}

		// p_Renewal_No  OUT    INTEGER,
		// DB_VAR * hRenewal_No; //:p_Renewal_No 
		hRenewal_No = DBBindPlaceHolderVariable(
			hStmt,
			":p_Renewal_No",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hRenewal_No == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Renewal_No : %s", dwErr, szErr);
			break;
		}

		// p_Account_Balance  OUT    INTEGER,
		//DB_VAR * hAccount_Balance; //:p_Account_Balance, 
		hAccount_Balance = DBBindPlaceHolderVariable(
			hStmt,
			":p_Account_Balance",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hAccount_Balance == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Account_Balance : %s", dwErr, szErr);
			break;
		}

		// p_User_Balance  OUT    INTEGER,
		//DB_VAR * hUser_Balance; //:p_User_Balance, 
		hUser_Balance = DBBindPlaceHolderVariable(
			hStmt,
			":p_User_Balance",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hUser_Balance == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_User_Balance : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hResult; //:p_Result, 
		hResult = DBBindPlaceHolderVariable(
			hStmt,
			":p_Flag_Result",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hResult == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Flag_Result : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * h_id_Reason; //:p_id_Reason
		h_id_Reason = DBBindPlaceHolderVariable(
			hStmt,
			":p_Id_Reason",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (h_id_Reason == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind :p_Id_Reason : %s", dwErr, szErr);
			break;
		}

		// p_Discount_Group  OUT    INTEGER,
		//DB_VAR * hDiscountGroup; //:p_Discount_Group, 
		hDiscountGroup = DBBindPlaceHolderVariable(
			hStmt,
			":p_Discount_Group",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hDiscountGroup == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, bind p_Discount_Group : %s", dwErr, szErr);
			break;
		}

		// Initialiser les variables en input
		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_ref_exit, szDteRefExit, sizeof(szDteRefExit));
		DBSetVariableItemValue(hDte_Ref_Exit, 0, &szDteRefExit, DB_DATE_CHAR_LENGHT + 1);
		NTSVCInfo("DBAuthVT_ExlReqV3(), szDteRefExit %s", szDteRefExit);

		DBSetVariableItemValue(hRef_Exit_Trs, 0, &psAuthReq->body.exit_trs_ref_num, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_ExlReqV3(), exit_trs_ref_num %d", psAuthReq->body.exit_trs_ref_num);

		DBSetVariableItemValue(hId_Exit_Plaza, 0, &psAuthReq->body.exit_plaza_id, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_ExlReqV3(), exit_plaza_id %d", psAuthReq->body.exit_plaza_id);

		DBSetVariableItemValue(hId_Exit_Lane, 0, &psAuthReq->body.exit_lane_id, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_ExlReqV3(), exit_lane_id %d", psAuthReq->body.exit_lane_id);

		DBSetVariableItemValue(hId_Exit_Trs, 0, &psAuthReq->body.exit_trs_id, DB_ID_ENTRY_TRS_LENGHT_V3 + 1);
		NTSVCInfo("DBAuthVT_ExlReqV3(), exit_trs_id %s", psAuthReq->body.exit_trs_id);

		DB_ConvertSYSTEMTIME_ToStr(&psAuthReq->body.dte_exit, szDteExit, sizeof(szDteExit));
		DBSetVariableItemValue(hDte_Exit, 0, &szDteExit, DB_DATE_CHAR_LENGHT + 1);
		NTSVCInfo("DBAuthVT_ExlReqV3(), szDteExit %s", szDteExit);

		DBSetVariableItemValue(hExit_VRN, 0, &psAuthReq->body.exit_vrn, DB_ENTRY_VRN_LENGHT + 1);
		NTSVCInfo("DBAuthVT_ExlReqV3(), exit_vrn %s", psAuthReq->body.exit_vrn);

		DBSetVariableItemValue(hExit_VRN_Country, 0, &psAuthReq->body.exit_vrn_country, DB_ENTRY_VRN_COUNTRY + 1);
		NTSVCInfo("DBAuthVT_ExlReqV3(), exit_vrn_country %s", psAuthReq->body.exit_vrn_country);

		DBSetVariableItemValue(hEntry_Ticket_Type, 0, &psAuthReq->body.ticket_type, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_ExlReqV3(), ticket_type %d", psAuthReq->body.ticket_type);

		DBSetVariableItemValue(hFlag_ETicket, 0, &psAuthReq->body.flag_eticket, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_ExlReqV3(), flag_eticket %d", psAuthReq->body.flag_eticket);

		DBSetVariableItemValue(hFlag_Delete, 0, &psAuthReq->body.flag_delete, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_ExlReqV3(), flag_delete %d", psAuthReq->body.flag_delete);

		DBSetVariableItemValue(hFlag_Open_System, 0, &psAuthReq->body.flag_open_system, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_ExlReqV3(), flag_open_system %d", psAuthReq->body.flag_open_system);

		DBSetVariableItemValue(hFlag_No_Update, 0, &psAuthReq->body.flag_no_update, sizeof(unsigned int));
		NTSVCInfo("DBAuthVT_ExlReqV3(), flag_no_update %d", psAuthReq->body.flag_no_update);

		NTSVCInfo("DBAuthVT_ExlReqV3(szDteRefExit:[%s], exit_trs_ref_num:[%u], exit_plaza_id:[%u], exit_lane_id[%u], exit_trs_id:[%s], szDteExit[%s], exit_vrn[%s], exit_vrn_country[%s], ticket_type[%u], flag_eticket:[%u], flag_delete:[%u], flag_open_system:[%u], flag_no_update:[%u]), execute request",
			szDteRefExit,
			psAuthReq->body.exit_trs_ref_num,
			psAuthReq->body.exit_plaza_id,
			psAuthReq->body.exit_lane_id,
			psAuthReq->body.exit_trs_id,
			szDteExit,
			psAuthReq->body.exit_vrn,
			psAuthReq->body.exit_vrn_country,
			psAuthReq->body.ticket_type,
			psAuthReq->body.flag_eticket,
			psAuthReq->body.flag_delete,
			psAuthReq->body.flag_open_system,
			psAuthReq->body.flag_no_update);

		// Exécuter la requète
		if (!DBExecuteStatement(hStmt))
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBAuthVT_ExlReqV3(), error %u, executing: %s", dwErr, szErr);
			break;
		}

		//First get result and reason
		//: p_Flag_Result, 
		dwLen = sizeof(uiResult);
		puiResult = DBGetVariableItemValue(hResult, 0, &uiResult, &dwLen);
		if (puiResult != &uiResult && dwLen == sizeof(uiResult))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Flag_Result");
			break;
		}

		//:p_Id_Reason); COMMIT; END; "
		dwLen = sizeof(uiIdReason);
		puiResult = DBGetVariableItemValue(h_id_Reason, 0, &uiIdReason, &dwLen);
		if (puiResult != &uiIdReason && dwLen == sizeof(uiIdReason))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Id_Reason");
			break;
		}

		// 		//Do not try to read other OUT variables in case of any error...
		// 		if (dwErr != NO_ERROR || uiResult == AUTH_NOK)
		// 			break;

		// Get database data	

		// :p_Id_Exit_Provider 
		pbResult = DBGetVariableItemValue(hId_Exit_Provider, 0, psAuthRsp->body.exit_provider, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Id_Exit_Provider == NULL");
		}
		else if (pbResult != psAuthRsp->body.exit_provider)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Id_Exit_Provider");
			break;
		}


		//":p_Dte_Ref_Entry, 
		pbResult = DBGetVariableItemValue(hDte_Ref_Entry, 0, szDteRefEntry, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Dte_Ref_Entry == NULL");
		}
		else if (pbResult != szDteRefEntry)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Dte_Ref_Entry");
			break;
		}
		else
		{
			if (strlen(szDteRefEntry) > 0)
				if (FALSE == DB_ConvertStr_ToSYSTEMTIME(szDteRefEntry, &psAuthRsp->body.dte_ref_entry))
				{
					NTSVCInfo("DBAuthVT_ExlReqV3(), Error DB_ConvertStr_ToSYSTEMTIME from szDteRefEntry [%s]", szDteRefEntry);
				}
		}

		//:p_Ref_Entry_Trs, 		dwLen = sizeof(psAuthRsp->body.entry_trs_ref_num);
		dwLen = sizeof(psAuthRsp->body.entry_trs_ref_num);
		puiResult = DBGetVariableItemValue(hRef_Entry_Trs, 0, &psAuthRsp->body.entry_trs_ref_num, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Ref_Entry_Trs == NULL");
		}
		else if (puiResult != &psAuthRsp->body.entry_trs_ref_num)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Ref_Entry_Trs");
			break;
		}

		//:p_Id_Entry_Plaza, 
		dwLen = sizeof(psAuthRsp->body.entry_plaza_id);
		puiResult = DBGetVariableItemValue(hId_Entry_Plaza, 0, &psAuthRsp->body.entry_plaza_id, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Id_Entry_Plaza == NULL");
		}
		else if (puiResult != &psAuthRsp->body.entry_plaza_id)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Id_Entry_Plaza");
			break;
		}

		//DB_VAR * hId_Entry_Lane; //: p_Id_Entry_Lane, 
		dwLen = sizeof(psAuthRsp->body.entry_lane_id);
		puiResult = DBGetVariableItemValue(hId_Entry_Lane, 0, &psAuthRsp->body.entry_lane_id, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Id_Entry_Lane == NULL");
		}
		else if (puiResult != &psAuthRsp->body.entry_lane_id)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Id_Entry_Lane");
			break;
		}

		//DB_VAR * hId_Entry_Trs; //: p_Id_Entry_Trs, 
		pbResult = DBGetVariableItemValue(hId_Entry_Trs, 0, psAuthRsp->body.entry_trs_id, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Id_Entry_Trs == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_trs_id)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Id_Entry_Trs");
			break;
		}

		//DB_VAR * hDte_Entry; //: p_Dte_Entry, 
		//char szDteEntry[DB_DATE_CHAR_LENGHT + 1] = { 0 };
		pbResult = DBGetVariableItemValue(hDte_Entry, 0, szDteEntry, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Dte_Entry == NULL");
		}
		else if (pbResult != szDteEntry)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Dte_Entry");
			break;
		}
		else
		{
			if (strlen(szDteEntry) > 0)
				if (FALSE == DB_ConvertStr_ToSYSTEMTIME(szDteEntry, &psAuthRsp->body.dte_entry))
				{
					NTSVCInfo("DBAuthVT_ExlReqV3(), Error DB_ConvertStr_ToSYSTEMTIME from szDteEntry [%s]", szDteEntry);
				}
		}

		//DB_VAR * hEntry_VRN; //: p_Entry_VRN, 
		pbResult = DBGetVariableItemValue(hEntry_VRN, 0, psAuthRsp->body.entry_vrn, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Entry_VRN == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_vrn)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Entry_VRN");
			break;
		}


		//DB_VAR * hEntry_VRN_Country; //: p_Entry_VRN_Country, 
		pbResult = DBGetVariableItemValue(hEntry_VRN_Country, 0, psAuthRsp->body.entry_vrn_country, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Entry_VRN_Country == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_vrn_country)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Entry_VRN_Country");
			break;
		}

		//DB_VAR * hId_Entry_Provider; //: p_Id_Entry_Provider, 
		pbResult = DBGetVariableItemValue(hId_Entry_Provider, 0, psAuthRsp->body.entry_provider, NULL);
		if (pbResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Id_Entry_Provider == NULL");
		}
		else if (pbResult != psAuthRsp->body.entry_provider)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Id_Entry_Provider");
			break;
		}

		// p_Id_Contract_Type  OUT    INTEGER,
		//DB_VAR * hId_Contract_SType; //:p_Id_Contract_Type,
		dwLen = sizeof(uiId_Contract_Type);
		puiResult = DBGetVariableItemValue(hId_Contract_Type, 0, &uiId_Contract_Type, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Id_Contract_Type == NULL");
			uiId_Contract_Type = 0;
		}
		else if (puiResult != &uiId_Contract_Type && dwLen == sizeof(uiId_Contract_Type))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Id_Contract_Type");
			break;
		}
		psAuthRsp->body.contract_type = uiId_Contract_Type;


		// p_Id_Payment_SubType  OUT    INTEGER,
		//DB_VAR * hId_Payment_SubType; //:p_Id_Payment_SubType,
		dwLen = sizeof(uiId_Payment_SubType);
		puiResult = DBGetVariableItemValue(hId_Payment_SubType, 0, &uiId_Payment_SubType, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Id_Payment_SubType == NULL");
			uiId_Payment_SubType = 0;
		}
		else if (puiResult != &uiId_Payment_SubType && dwLen == sizeof(uiId_Payment_SubType))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Id_Payment_SubType");
			break;
		}
		psAuthRsp->body.payment_subtype = uiId_Payment_SubType;


		// p_Id_Class            OUT    INTEGER,
		//DB_VAR * hId_Class; //:p_Id_Class,
		dwLen = sizeof(uiId_Class);
		puiResult = DBGetVariableItemValue(hId_Class, 0, &uiId_Class, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Id_Class == NULL");
			uiId_Class = 0;
		}
		else if (puiResult != &uiId_Class && dwLen == sizeof(uiId_Class))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Id_Class");
			break;
		}
		psAuthRsp->body.vehicle_class = uiId_Class;


		// p_Accepted_Days       OUT    VARCHAR2,
		//DB_VAR * hAccepted_Days; //:p_Accepted_Days, 
		pbResult = DBGetVariableItemValue(hAccepted_Days, 0, psAuthRsp->body.accepted_days, NULL);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Accepted_Days == NULL");

		}
		else if (pbResult != psAuthRsp->body.accepted_days)
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_ExlReqV3(), error getting p_Accepted_Days");
			break;
		}

		// p_Accoun_No            OUT    INTEGER,
		//DB_VAR * hAccoun_No; //:p_Accoun_No,
		dwLen = sizeof(uiAccoun_No);
		puiResult = DBGetVariableItemValue(hAccoun_No, 0, &uiAccoun_No, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Accoun_No == NULL");
			uiAccoun_No = 0;
		}
		else if (puiResult != &uiAccoun_No && dwLen == sizeof(uiAccoun_No))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Accoun_No");
			break;
		}
		psAuthRsp->body.exempt_acccount_number = uiAccoun_No;

		// p_User_No            OUT    INTEGER,
		//DB_VAR * hUser_No; //:p_User_No,
		dwLen = sizeof(uiUser_No);
		puiResult = DBGetVariableItemValue(hUser_No, 0, &uiUser_No, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_User_No == NULL");
			uiUser_No = 0;
		}
		else if (puiResult != &uiUser_No && dwLen == sizeof(uiUser_No))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_User_No");
			break;
		}
		psAuthRsp->body.exempt_user_number = uiUser_No;

		// p_Renewal_No            OUT    INTEGER,
		//DB_VAR * hRenewal_No; //:p_Renewal_No,
		dwLen = sizeof(uiRenewal_No);
		puiResult = DBGetVariableItemValue(hRenewal_No, 0, &uiRenewal_No, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Renewal_No == NULL");
			uiRenewal_No = 0;
		}
		else if (puiResult != &uiRenewal_No && dwLen == sizeof(uiRenewal_No))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Renewal_No");
			break;
		}
		psAuthRsp->body.exempt_renewal_number = uiRenewal_No;

		// p_Account_Balance  OUT    INTEGER,
		//DB_VAR * hAccount_Balance; //:p_Account_Balance,
		dwLen = sizeof(uiAccount_Balance);
		puiResult = DBGetVariableItemValue(hAccount_Balance, 0, &uiAccount_Balance, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Account_Balance == NULL");
			uiAccount_Balance = 0;
		}
		else if (puiResult != &uiAccount_Balance && dwLen == sizeof(uiAccount_Balance))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Account_Balance");
			break;
		}
//		psAuthRsp->body.account_balance = uiAccount_Balance;
		_itoa_s(uiAccount_Balance, psAuthRsp->body.account_balance, sizeof(psAuthRsp->body.account_balance), 10);

		// p_User_Balance  OUT    INTEGER,
		//DB_VAR * hUser_Balance; //:p_User_Balance,
		dwLen = sizeof(uiUser_Balance);
		puiResult = DBGetVariableItemValue(hUser_Balance, 0, &uiUser_Balance, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_User_Balance == NULL");
			uiUser_Balance = 0;
		}
		else if (puiResult != &uiUser_Balance && dwLen == sizeof(uiUser_Balance))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_User_Balance");
			break;
		}
//		psAuthRsp->body.user_balance = uiUser_Balance;
		_itoa_s(uiUser_Balance, psAuthRsp->body.user_balance, sizeof(psAuthRsp->body.user_balance), 10);

		// p_Discount_Group  OUT    INTEGER,
		//DB_VAR * hDiscountGroup; //:p_Discount_Group,
		dwLen = sizeof(uiDiscountGroup);
		puiResult = DBGetVariableItemValue(hDiscountGroup, 0, &uiDiscountGroup, &dwLen);
		if (puiResult == DB_VALUE_NULL)
		{
			NTSVCInfo("DBAuthVT_ExlReqV3(), p_Discount_Group == NULL");
			uiDiscountGroup = 0;
		}
		else if (puiResult != &uiDiscountGroup && dwLen == sizeof(uiDiscountGroup))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBAuthVT_EnlReqV3(), error getting p_Discount_Group");
			break;
		}
		_itoa_s(uiDiscountGroup, psAuthRsp->body.discount_group, sizeof(psAuthRsp->body.discount_group), 2);


	} while (FALSE);

	if (dwErr != NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_ExlReqV3(), Error getting data %d", dwErr);

		// 3113 - Database down during statement exec
		// ERROR_PIPE_NOT_CONNECTED - Database down before connect
		psAuthRsp->body.flag_result = AUTH_NOK;
		psAuthRsp->body.reason_id = dwErr;
	}
	else //	if ( dwErr == NO_ERROR)
	{
		NTSVCInfo("DBAuthVT_ExlReqV3() - Result:%s Reason:%d", (uiResult == AUTH_NOK) ? "AUTH_NOK" : "AUTH_OK", uiIdReason);
		psAuthRsp->body.flag_result = uiResult;

		if (uiResult == AUTH_NOK)
			psAuthRsp->body.reason_id = uiIdReason;
		else
			psAuthRsp->body.reason_id = 0;
	}

	if (hStmt != NULL)
		DBCloseStatement(hStmt);

	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);

	}
	else if (hDbCnx != NULL) // && gsSvcWork.sParmWork.dwDbKeepConnection == FALSE)
	{
		NTSVCInfo("DBAuthVT_ExlReqV3(), disconnect from the database");
		DBDisconnect(hDbCnx);
	}

	return dwErr;
}



PROTECTED DWORD DBDoDummyDBReq()
{
	DWORD dwErr = NO_ERROR;
	DB_STMT * hStmt = NULL;          // Handle de requete
	char szErr[2000] = { 0 };            // Texte d'erreur
	DB_CNX * hDbCnx = NULL;          // Database conection handle
	CHAR	szStatement[1024] = { 0 };
	DB_VAR * hResult; //: p_Result, 
	unsigned int uiResult = 0;
	unsigned int *puiResult = NULL;
	DWORD dwLen;

	do{
		if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
		{
			EnterCriticalSection(&gsSvcWork.csDBConnectionProtect);

			if (!DBOpen())
			{
				NTSVCInfo("DBDoDummyDBReq(), error reconnecting to the database");
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				break;
			}

			hDbCnx = gsSvcWork.hDbCnx;
		}
		else
		{
			// Essayer d'établir la connexion
			NTSVCInfo("DBDoDummyDBReq(), connexion à la base [%s] en tant que [%s]", gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr);
			hDbCnx = DBConnect(gsSvcWork.sParmWork.szDbInst, gsSvcWork.sParmWork.szDbUsr, gsSvcWork.sParmWork.szDbPwd);
			// Si toujours pas connecté, on ne va pas plue long
			if (hDbCnx == NULL)
			{
				dwErr = ERROR_PIPE_NOT_CONNECTED;
				NTSVCInfo("DBDoDummyDBReq(), error %u, connexion à la base impossible", dwErr);
				break;
			}
		}

		// REQUEST: Select record for requested media ID
		NTSVCInfo("DBDoDummyDBReq(), preparing the request");

		// Prepare the request
		_snprintf_s(szStatement,
			_countof(szStatement),
			sizeof(szStatement),
			"BEGIN SELECT 10*10 INTO :p_Result FROM DUAL; END;");


		// Prepare the request
		hStmt = DBOpenStatement(hDbCnx, szStatement);

		if (hStmt == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBDoDummyDBReq(), error %u, statement AuthRequest : %s", dwErr, szErr);
			break;
		}

		//DB_VAR * hResult; //:p_Result, 
		hResult = DBBindPlaceHolderVariable(
			hStmt,
			":p_Result",
			DB_TYPE_INT,
			sizeof(int),
			0);
		if (hResult == NULL)
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBDoDummyDBReq(), error %u, bind p_Result : %s", dwErr, szErr);
			break;
		}

		// Exécuter la requète
		if (!DBExecuteStatement(hStmt))
		{
			DBGetLastError(hDbCnx, NULL, &dwErr, szErr, sizeof(szErr));
			NTSVCInfo("DBDoDummyDBReq(), error %u, executing: %s", dwErr, szErr);
			break;
		}

		//First get result and reason
		//: p_Flag_Result, 
		dwLen = sizeof(uiResult);
		puiResult = DBGetVariableItemValue(hResult, 0, &uiResult, &dwLen);
		if (puiResult != &uiResult && dwLen == sizeof(uiResult))
		{
			dwErr = AUTHSVC_INTERNAL_SERVER_ERROR;
			NTSVCInfo("DBDoDummyDBReq(), error getting p_Flag_Result");
			break;
		}
		else
		{
			NTSVCInfo("DBDoDummyDBReq() - DB connection OK!");

		}

	} while (FALSE);

	if (hStmt != NULL)
		DBCloseStatement(hStmt);

	if (gsSvcWork.sParmWork.dwDbKeepConnection == TRUE)
	{
		LeaveCriticalSection(&gsSvcWork.csDBConnectionProtect);
	}
	else if (hDbCnx != NULL) // && gsSvcWork.sParmWork.dwDbKeepConnection == FALSE)
	{
		NTSVCInfo("DBDoDummyDBReq(), disconnect from the database");
		DBDisconnect(hDbCnx);
	}

	return dwErr;
}