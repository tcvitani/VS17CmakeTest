/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC.c                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc.h>
#include <msg_lc_backup_date.h>
#include <msg_lc_backup_file.h>
#include <msg_lc_bowl_movement.h>
#include <msg_lc_bowl_stat.h>
#include <msg_lc_coin_listing.h>
#include <msg_lc_command.h>
#include <msg_lc_comp_inf_tr.h>
#include <msg_lc_counter_status.h>
#include <msg_lc_endshift.h>
#include <msg_lc_event.h>
#include <msg_lc_header.h>
#include <msg_lc_hourly.h>
#include <msg_lc_partly.h>
#include <msg_lc_payment.h>
#include <msg_lc_perm_rq_send_file.h>
#include <msg_lc_perm_send_file.h>
#include <msg_lc_reference.h>
#include <msg_lc_startshift.h>
#include <msg_lc_status.h>
#include <msg_lc_tab_ack.h>
#include <msg_lc_trace.h>
#include <msg_lc_transaction.h>
#include <msg_lc_transaction_ves_altair.h>
#include <msg_lc_vault_ins.h>
#include <msg_lc_vault_stat.h>
#include <msg_lc_vlt_withdraw.h>

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
			MSG_BACKUP_DATE_Init();
			MSG_BACKUP_DATE_New_Record(NULL);

			MSG_BACKUP_FILE_Init();
			MSG_BACKUP_FILE_New_Record(NULL);

			MSG_BOWL_MOVEMENT_Init();
//			MSG_BOWL_MOVEMENT_Record(NULL);

			MSG_BOWL_STAT_Init();
//			MSG_BOWL_STAT_Record(NULL);

			MSG_COIN_LISTING_Init();
//			MSG_COIN_LISTING_Record(NULL);

			MSG_COMMAND_Init();
			MSG_COMMAND_New_Record(NULL);

			MSG_COMP_INF_TR_Init();
//			MSG_COMP_INF_TR_Record(NULL);

			MSG_END_SHIFT_Init();
//			MSG_END_SHIFT_Record(NULL);

			MSG_EVENT_Init();
//			MSG_EVENT_Record(NULL);

			MSG_HEADER_Init();

			MSG_HOURLY_Init();
//			MSG_HOURLY_Record(NULL);

			MSG_COUNTER_STATUS_Init();
//			MSG_COUNTER_STATUS_Record(NULL);

			MSG_PARTLY_Init();
//			MSG_PARTLY_Record(NULL);

			MSG_PAYMENT_Init();
//			MSG_PAYMENT_Record(NULL);

			MSG_PERM_RQ_SEND_FILE_Init();
//			MSG_PERM_RQ_SEND_FILE_Record(NULL);

			MSG_PERM_SEND_FILE_Init();
			MSG_PERM_SEND_FILE_New_Record(NULL);

			MSG_REFERENCE_Init();
			MSG_REFERENCE_New_Record(NULL);

			MSG_START_SHIFT_Init();
//			MSG_START_SHIFT_Record(NULL);

			MSG_STATUS_Init();
//			MSG_STATUS_Record(NULL);

			MSG_TAB_ACK_Init();
//			MSG_TAB_ACK_Record(NULL);

			MSG_TRACE_Init();
//			MSG_TRACE_Record(NULL);

			MSG_TRANSACTION_Init();
//			MSG_TRANSACTION_Record(NULL);

			MSG_TRANSACTION_VES_ALTAIR_Init();
//			MSG_TRANSACTION_VES_ALTAIR_Record(NULL);

			MSG_VAULT_INS_Init();
//			MSG_VAULT_INS_Record(NULL);

			MSG_VAULT_STAT_Init();
//			MSG_VAULT_STAT_Record(NULL);

			MSG_VLT_WITHDRAW_Init();
//			MSG_VLT_WITHDRAW_Record(NULL);
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