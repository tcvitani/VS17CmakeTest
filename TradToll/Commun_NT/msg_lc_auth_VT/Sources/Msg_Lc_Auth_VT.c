/*****************(v) 2015 SANEF ITS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_AUTH.c                                                   */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_auth_vt.h>
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

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// INITIALISATION
extern int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	HLIST msg_pv_record = NULL;

    switch(dwReason) 
    {  
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary.  
		case DLL_PROCESS_ATTACH:
			MSG_LC_AUTH_VT_ENL_REQ_Init();
			MSG_LC_AUTH_VT_ENL_REQ_New_Record(NULL);
			MSG_LC_AUTH_VT_EXL_REQ_Init();
			MSG_LC_AUTH_VT_EXL_REQ_New_Record(NULL);

			MSG_LC_AUTH_VT_ENL_REP_Init();
			MSG_LC_AUTH_VT_ENL_REP_New_Record(NULL);
			MSG_LC_AUTH_VT_EXL_REP_Init();
			MSG_LC_AUTH_VT_EXL_REP_New_Record(NULL);


			MSG_LC_AUTH_VT_ENL_REQ_V2_Init();
			MSG_LC_AUTH_VT_ENL_REQ_V2_New_Record(NULL);
			MSG_LC_AUTH_VT_EXL_REQ_V2_Init();
			MSG_LC_AUTH_VT_EXL_REQ_V2_New_Record(NULL);

			MSG_LC_AUTH_VT_ENL_REP_V2_Init();
			MSG_LC_AUTH_VT_ENL_REP_V2_New_Record(NULL);
			MSG_LC_AUTH_VT_EXL_REP_V2_Init();
			MSG_LC_AUTH_VT_EXL_REP_V2_New_Record(NULL);

			MSG_LC_AUTH_VT_ENL_REQ_V3_Init();
			MSG_LC_AUTH_VT_ENL_REQ_V3_New_Record(NULL);
			MSG_LC_AUTH_VT_EXL_REQ_V3_Init();
			MSG_LC_AUTH_VT_EXL_REQ_V3_New_Record(NULL);

			MSG_LC_AUTH_VT_ENL_REP_V3_Init();
			MSG_LC_AUTH_VT_ENL_REP_V3_New_Record(NULL);
			MSG_LC_AUTH_VT_EXL_REP_V3_Init();
			MSG_LC_AUTH_VT_EXL_REP_V3_New_Record(NULL);

			break;
        
        // The attached process creates a new thread.  
		case DLL_THREAD_ATTACH:  
			break; 
		
			// The thread of the attached process terminates. 
		case DLL_THREAD_DETACH:  
			break;  
		
			// The DLL unloading due to process termination or call to FreeLibrary. 
		case DLL_PROCESS_DETACH:  
			break;          
		
		default:             
			break;     
    }      
    
    return TRUE; 
	
    UNREFERENCED_PARAMETER(hInstance);     
    UNREFERENCED_PARAMETER(lpReserved); 
} 

/*-------------------------------- END OF FILE ------------------------------*/