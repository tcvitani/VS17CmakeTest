/********** (v) 2000 CSEE-Peage -   All rights reserved ************/
/*                                                                 */
/* ----------------------------------------------------------------*/
/* MODULE:   Printer Module Test Application                       */
/* FILE:     simu.c                                                */
/* LANGUAGE: C                                                     */
/* ----------------------------------------------------------------*/
/* DESCRIPTION:                                                    */
/*             This file contains the functions that simulate the  */
/*             main application and functions that start and       */
/*             stop the "Printer" module.                          */
/* ----------------------------------------------------------------*/
/* HISTORY:                                                        */
/*******************************************************************/

/*--------------------------- INCLUDES:  --------------------------*/
// VC++ interface
#include <windows.h>
#include <stdio.h>
#include <resource.h>

#include <global.h>
// CS Route interface
//#include <noyau.h>
// Module interface
//#define DCP_PROTOTYPES
//#include <dcp.h>
//#undef DCP_PROTOTYPES

//#include <CSR_PYL.H>

// Module interface
//#define LNK_PROTOTYPES
//#include <liaisons.h>
//#undef LNK_PROTOTYPES
// Application iterface
//#include <MAINT_dcp.h>
//#include <language.h>
#define LOC_DEF
#include <simu.h>
#undef LOC_DEF
/*--------------------------- RESERVED:  --------------------------*/
#include <memclass.h>
/*--------------------------- EXTERNALS: --------------------------*/
// Registry keys
#define PYL_REG_KEYv_PATH_COIN_ACCEPTORS				"\\coinacceptors"
#define PYL_REG_KEYv_PATH_COIN_ACCEPTOR					"\\coinacceptor"
#define PYL_REG_KEYv_PATH_COIN_ACCEPTOR_DATA			"\\coin"

#define PYL_REG_KEYv_PATH_BANKNOTE_ACCEPTORS			"\\billacceptors"
#define PYL_REG_KEYv_PATH_BANKNOTE_ACCEPTOR				"\\billacceptor"
#define PYL_REG_KEYv_PATH_BANKNOTE_ACCEPTOR_DATA		"\\bill"



// Registry values
// Coin data values
#define PYL_REG_KEYv_COIN_ACCEPTOR_COIN_ACTIVE			"active"
#define PYL_REG_KEYv_COIN_ACCEPTOR_COIN_ID				"ID"
/*--------------------------- DEFINES:   --------------------------*/
/*--------------------------- TYPEDEFS:  --------------------------*/
typedef enum
{
	FIRST_MONEY_TYPE			= 0,

	TYPE_COIN	= FIRST_MONEY_TYPE,
	TYPE_BILL,

	END_MONEY_TYPE,

	LAST_MONEY_TYPE	= END_MONEY_TYPE - 1,
	NB_MONEY_TYPES	= END_MONEY_TYPE - FIRST_MONEY_TYPE, 
}enum_money_type;
/*--------------------------- FUNCTIONS: --------------------------*/
PRIVATE void SimuRecoit(void);
//PRIVATE void SimuReceptionEtat( char *lb_item, 
//							   struct_emi_pic_message *p_msg_rec);

/*--------------------------- VARIABLES: --------------------------*/
// Thread definition
PRIVATE struct_tache t_simu[] =
{
    { TRUE , THREAD_PRIORITY_NORMAL, 1024, (LPTHREAD_START_ROUTINE)(SimuRecoit), NULL, NULL, "SIMU" },
    { FALSE, 0                     , 0   , NULL                                , NULL, NULL, ""   }
	
};
BOOL bSubscribedEtat;
BOOL bSubscribedImpresion;
CHAR NEWLINE[2] =  { "\r\n" };
/*--------------------------- CODE: -------------------------------*/
/**/
/*******************************************************************/
/*SYNTAX: int ArretModule(HWND hDlg)                               */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function stops the "Printer" module and test    */
/*            application thread.                                  */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      (IN )  HWND hDlg      - Handle of the main dialog box.     */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  0                     The function is successful.              */
/*  1                     The function is not successful.          */
/*******************************************************************/
PROTECTED int ArretModule(HWND hDlg)
{
	enum_instance_result   rc;


	// Stopping the simulation thread
	rc=ArretTaches(t_simu);
	if(rc != NOYAU_ARRET_TACHE_OK)
	{
		return 0;
	}

	// Removing mail boxes
	SupprimeBAL(MAINT_BAL_NAME);

	return 1;
}

/**/
/*******************************************************************/
/*SYNTAX: void OpenMailBox()                                       */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            The function opens the test application mail boxes.  */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*            This function has no parameters.                     */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PROTECTED BOOL OpenMailBox( VOID )
{

	CHAR                   szKey[ MAX_PATH + 1 ];
	enum_instance_result   rc;
	
	
	sprintf( szKey, "%s%s%s%s%s%s",
		CSR_REG_KEYn_CSRBASE,
		CSR_REG_KEYn_LANE_BASE,
		CSR_REG_KEYn_CONFIG,
		MOD_REG_KEYn_MODULES,
		EMI_REG_KEYn_ModEMI_PIC,
		SIMU.MailBoxName );
	

	SIMU.MAINT_bal_id = PublieBAL( MAINT_BAL_NAME, NOYAU_BAL_ILLIMITEE );
	SIMU.module_bal_id = AttendBALTantQue( SIMU.MailBoxName, 3 );
	
	if(SIMU.module_bal_id == 0)
	{
		rc = SIMU.LpFncMODLance( szKey, SIMU.MailBoxName, &SIMU.module_bal_id );
		if ( rc != INST_INIT_OK)
		{
			MessageBox( NULL, "Failed to load module", "Error", MB_OK|MB_ICONWARNING );
			return 0;
		}
		
		SIMU.module_bal_id = AttendBAL( SIMU.MailBoxName );
		if ( SIMU.module_bal_id <= 0 )
		{
			MessageBox( SIMU.hDlg, 			
				"Failed loading module", 
				"Error", 
				MB_OK|MB_ICONWARNING);
			
			return 0;
		}
	}
	else
	{
		if( SIMU.MAINT_bal_id == 0 || SIMU.module_bal_id == 0 )
		{
			SupprimeBAL ( MAINT_BAL_NAME );
			return FALSE;
		}
		
	}
	
	rc = LanceTache (t_simu);

	if ( rc != NOYAU_OK)
	{
		return 0;
	}	
	
	return 1;
}
/**/
/*******************************************************************/
/*SYNTAX: void SimuRecoit(void)                                    */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            The function adds received messages into the trace   */
/*            list box.                                            */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*            This function has no parameters.                     */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PRIVATE void SimuRecoit(void)
{
	short int
		code_rtc;
	struct_neutre 
		*p_neutre;
	struct_emi_pic_message
		*p_simu_message;
	struct_emi_pic_etat
		sEtat;
	
	ChangePriorite( TacheCourante(), THREAD_PRIORITY_NORMAL );
	while (SIMU.bRun !=FALSE )
	{
		DelaiTache(3);

		code_rtc = TestRecoit(SIMU.MAINT_bal_id,
							  (struct_neutre **)&p_neutre);

		if (code_rtc == NOYAU_BAL_MESS)
		{

			p_simu_message = (struct_emi_pic_message *)(p_neutre);

			switch(p_simu_message->entete.service)
			{

				// Messages from the ETAT service
				case M_SRV_ETAT:
					switch( p_simu_message->entete.type_message )
					{
						case SRV_TYP_NOUVEL_ETAT:
							memcpy( &sEtat, &(p_simu_message->u.srv_etat.status), sizeof(struct_emi_pic_etat) );
							
							SendMessage(GetDlgItem(SIMU.hDlg, IDC_LINK_ERR), 
								BM_SETCHECK, sEtat.errorLink, 0);
							break;
							
						default:
							break;
					}
					break;
					
//					SimuReceptionEtat(lb_item, p_simu_message);
					break;

				default:
					break;
			}
			// Free memory
			Libere(&p_neutre);
		}
	}

	PostMessage( SIMU.hDlg, WM_CLOSE, 1, 0 );
	// Thread stopping
	Termine();

}
/*******************************************************************/
/*SYNTAX: void SimuReceptionEtat(char *lb_item,                    */
/*								 struct_emi_pic_message *p_msg_rec)    */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function creates a string of the messages       */
/*            received from the ETAT service. The string will be   */
/*            displayed in the trace list box.                     */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*   (IN-OUT) char *lb_item                 - String which will be */
/*                                            created.             */
/*   (IN )    struct_emi_pic_message *p_msg_rec - Pointer to the       */
/*                                            message structure    */
/*                                            received from the    */
/*                                            ETAT service.        */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
/*
PRIVATE void SimuReceptionEtat( char *lb_item, 
							   struct_emi_pic_message *p_msg_rec)
{
	
	switch(p_msg_rec->entete.type_message)
	{
	case SRV_TYP_NOUVEL_ETAT :
		
		memcpy( &SIMU.sStatus,
			&p_msg_rec->u.msg_etat.u.etat,
			sizeof( PYL_msg_etat ) );
		
		ManageStatusReport();

		InvalidateRect( SIMU.hDlg, NULL, FALSE );
		
		break;
	case SRV_TYP_DEBUT_ACQ:
		SIMU.bSubscribedEtat = TRUE;
		break;
	case SRV_TYP_FIN_ACQ:
		SIMU.bSubscribedEtat = FALSE;
		
		PostMessage(	SIMU.hDlg,
			WM_CLOSE,
			0,
			1);
		break;
	default :
		//			SimuCommonMessage(lb_item, 
		//							  p_msg_rec->entete.type_message);
		break;
	}
}*/

/**/
/*******************************************************************/
/*SYNTAX: void SimuCommonMessage(char *lb_item,                    */
/*								 short int type_message)           */
/*=================================================================*/
/*TYPE:   Local function.                                          */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function creates a string of the messages       */
/*            common for all services. The string will be          */
/*            displayed in the trace list box.                     */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*   (IN-OUT) char *lb_item                 - String which will be */
/*                                            created.             */
/*   (IN )    struct_emi_pic_message *p_msg_rec - Pointer to the       */
/*                                            message structure    */
/*                                            received from the    */
/*                                            services.            */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PRIVATE void SimuCommonMessage(char *lb_item, 
							   int type_message)
{
	return;
}
/**/
/*******************************************************************/
/*SYNTAX: void SimuEnvoi(short int service,						  */
/*							short int type_message)                */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            The function sends the message to the "Printer"      */
/*            module.                                              */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      (IN ) short int service       - Service type.              */
/*      (IN ) short int type_message  - Message type.              */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PROTECTED void SimuEnvoi(short int service, 
							short int type_message)
{
	struct_emi_pic_message
		*p_msg_emis,
		*p_message = &SIMU.message;
	

	ExitAlloue((struct_neutre **)(&p_msg_emis),
				sizeof(struct_emi_pic_message),
				NOYAU_GetPoolId("SIMUDCP"));

	p_msg_emis->entete.service = service;
	p_msg_emis->entete.type_message = type_message;

	switch( service )
	{
		case M_EMI_PIC_RESTITUTION:
			switch( type_message )
			{
				case M_EMI_PIC_TYP_RESTIT_BY_DATETIME:
					memcpy( &(p_msg_emis->u.srv_restit.u.sRequestByDateTime.sBegin), &(SIMU.sFromTime), sizeof(SYSTEMTIME) );
					memcpy( &(p_msg_emis->u.srv_restit.u.sRequestByDateTime.sEnd), &(SIMU.sToTime), sizeof(SYSTEMTIME) );
					break;
					
				case M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID:
					p_msg_emis->u.srv_restit.u.sRequestByRecordID.lRecordID = SIMU.lRecordID;
					break;
				default:
					break;
			}
			break;
			
		case M_SRV_ETAT:
			switch( type_message )
			{
				default:
					break;
			}
			break;

/*
		case M_PYL_PAYMENT:
			switch( type_message )
			{
				case PYL_START:
					p_msg_emis->u.msg_payment.u.pay_start.amount = SIMU.lPayIn;
					break;

				case PYL_PAYOUT:
					p_msg_emis->u.msg_payment.u.pay_payout.amount = SIMU.lPayOut;					
					break;
			
				default:
					break;
			}
			break;
*/
	
		default:
			break;
	}


	if(Envoie(SIMU.module_bal_id,
			  SIMU.MAINT_bal_id,
			  (struct_neutre *)(p_msg_emis))!=NOYAU_OK)
	{
			  ExitLibere((struct_neutre **)&p_msg_emis);
	}
}
/*--------------------------- END FILE ----------------------------*/
 