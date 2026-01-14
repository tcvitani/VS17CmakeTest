/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV.c                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_pv.h>
#include <msg_pv_ack_req.h>
#include <msg_pv_alar_rep.h>
#include <msg_pv_auth_rep.h>
#include <msg_pv_auth_req.h>
#include <msg_pv_bowl_rep.h>
#include <msg_pv_com_req.h>
#include <msg_pv_conf_rep.h>
#include <msg_pv_conf_req.h>
#include <msg_pv_dec_req.h>
#include <msg_pv_disk_rep.h>
#include <msg_pv_gal_req.h>
#include <msg_pv_lane_rep.h>
#include <msg_pv_log_rep.h>
#include <msg_pv_log_req.h>
#include <msg_pv_part_rep.h>
#include <msg_pv_plaz_rep.h>
#include <msg_pv_serv_rep.h>
#include <msg_pv_trac_rep.h>
#include <msg_pv_user_rep.h>
#include <msg_pv_user_req.h>
#include <msg_pv_vaul_rep.h>

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
			MSG_PV_ACK_REQ_Init();
			MSG_PV_ACK_REQ_New_Record(NULL);

			MSG_PV_ALAR_REP_Init();
			MSG_PV_ALAR_REP_New_Record(NULL);

			MSG_PV_AUTH_REP_Init();
			MSG_PV_AUTH_REP_New_Record(NULL);

			MSG_PV_AUTH_REQ_Init();
			MSG_PV_AUTH_REQ_New_Record(NULL);

			MSG_PV_BOWL_REP_Init();
			MSG_PV_BOWL_REP_New_Record(NULL);

			MSG_PV_COM_REQ_Init();
			MSG_PV_COM_REQ_New_Record(NULL);

			MSG_PV_CONF_REP_Init();
			MSG_PV_CONF_REP_New_Record(NULL);

			MSG_PV_CONF_REQ_Init();
			MSG_PV_CONF_REQ_New_Record(NULL);

			MSG_PV_DEC_REQ_Init();
			MSG_PV_DEC_REQ_New_Record(NULL);

			MSG_PV_DISK_REP_Init();
			MSG_PV_DISK_REP_New_Record(NULL);

			MSG_PV_GAL_REQ_Init();
			MSG_PV_GAL_REQ_New_Record(NULL);

			MSG_PV_LANE_REP_Init();
			MSG_PV_LANE_REP_New_Record(NULL);

			MSG_PV_LOG_REP_Init();
			MSG_PV_LOG_REP_New_Record(NULL);

			MSG_PV_LOG_REQ_Init();
			MSG_PV_LOG_REQ_New_Record(NULL);

			MSG_PV_PART_REP_Init();
			MSG_PV_PART_REP_New_Record(NULL);

			MSG_PV_LANE_REP_New_Record(&msg_pv_record);
			MSG_PV_ALAR_REP_New_Record(&msg_pv_record);
			MSG_PV_PLAZ_REP_Init(msg_pv_record);
			MSG_PV_PLAZ_REP_New_Record(NULL);

			MSG_PV_SERV_REP_Init();
			MSG_PV_SERV_REP_New_Record(NULL);

			MSG_PV_TRAC_REP_Init();
			MSG_PV_TRAC_REP_New_Record(NULL);

			MSG_PV_USER_REP_Init();
			MSG_PV_USER_REP_New_Record(NULL);

			MSG_PV_USER_REQ_Init();
			MSG_PV_USER_REQ_New_Record(NULL);

			MSG_PV_VAUL_REP_Init();
			MSG_PV_VAUL_REP_New_Record(NULL);
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