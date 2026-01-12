/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_SV.C                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_sv.h>
#include <msg_sv_con_req.h>
#include <msg_sv_con_rep.h>
#include <msg_sv_filt_dec.h>

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// INITIALISATION
extern int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	HLIST msg_pv_record = NULL;

    switch (dwReason) 
    {  
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary.  
		case DLL_PROCESS_ATTACH:
			MSG_SV_CON_REP_Init();
			MSG_SV_CON_REP_New_Record(NULL);

			MSG_SV_CON_REQ_Init();
			MSG_SV_CON_REQ_New_Record(NULL);

			MSG_SV_FILT_DEC_Init();
			MSG_SV_FILT_DEC_New_Record(NULL);
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