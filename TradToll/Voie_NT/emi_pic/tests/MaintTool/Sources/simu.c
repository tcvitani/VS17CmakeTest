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
#define LOC_DEF
#include <simu.h>
#undef LOC_DEF
/*--------------------------- RESERVED:  --------------------------*/
#include <memclass.h>
/*--------------------------- EXTERNALS: --------------------------*/

/*--------------------------- DEFINES:   --------------------------*/

/*--------------------------- TYPEDEFS:  --------------------------*/

/*--------------------------- FUNCTIONS: --------------------------*/
PRIVATE void SimuRecoit(void);
PRIVATE VOID HandleMaintenanceService( struct_emi_pic_message *lpMessage );
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
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
PUBLIC BOOL LanceModule( VOID )
{
	if( SIMU.MAINT_bal_id == 0 )
	{// Own mailbox not set
		return FALSE;
	}

	return LanceTache( t_simu );
}
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
//	enum_instance_result   rc;

	// Stopping the simulation thread
//	rc=ArretTaches(t_simu);
//	if(rc != NOYAU_ARRET_TACHE_OK)
//	{
//		return 0;
//	}

	// Removing mail boxes
	SupprimeBAL( SIMU.MAINT_bal_name );

	return TRUE;
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

	CHAR
		szKey[ MAX_PATH + 1 ];
	enum_instance_result
		rc;
	LONG
		i;

	for( i = 0; i < SIMU.NbModuleInstances; i++ )
	{	
		// Try to get module mailbox (if module is already running)
		SIMU.ModuleData[i].MailboxID = AttendBALTantQue( SIMU.ModuleData[i].szMailboxName, 3 );

		if( SIMU.ModuleData[i].MailboxID != 0 )
		{// Module was running, all done
			SIMU.ModuleData[i].bModuleWasRunning = TRUE;
		}
		else
		{// Module is not running, start it
			SIMU.ModuleData[i].bModuleWasRunning = FALSE;
			
			sprintf( szKey, "%s%s%s%s%s%s",
				CSR_REG_KEYn_CSRBASE,
				CSR_REG_KEYn_LANE_BASE,
				CSR_REG_KEYn_CONFIG,
				MOD_REG_KEYn_MODULES,
				EMI_REG_KEYn_ModEMI_PIC,
				SIMU.ModuleData[i].szMailboxName );

			rc = SIMU.LpFncMODLance( szKey, SIMU.ModuleData[i].szMailboxName, &SIMU.ModuleData[i].MailboxID );
			if ( rc != INST_INIT_OK)
			{
				MessageBox( SIMU.hDlg, 
					Translation( TXT_ERROR_START_MODULE ), 			
					Translation( TXT_ERR_ERR ), 
					MB_OK|MB_ICONWARNING);
				
				return FALSE;
			}
		}
	}

	return TRUE;
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
	short int           code_rtc;
	struct_emi_pic_message  *p_simu_message;
	struct_neutre       *p_neutre;
	
	SIMU.fin=FALSE;


	ChangePriorite( TacheCourante(), THREAD_PRIORITY_NORMAL );
	while (SIMU.fin==FALSE)
	{
		DelaiTache(3);

		code_rtc = TestRecoit(SIMU.MAINT_bal_id,
							  (struct_neutre **)&p_neutre);

		if (code_rtc == NOYAU_BAL_MESS)
		{

			p_simu_message = (struct_emi_pic_message *)(p_neutre);

			switch(p_simu_message->entete.service)
			{
				case M_EMI_PIC_MAINT_MODE:
					HandleMaintenanceService( p_simu_message );
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
PROTECTED void SimuEnvoi(	noyau_bal_id	MailboxID,
							short int		service, 
							short int		type_message )
{
	struct_emi_pic_message  *p_msg_emis;
	struct_emi_pic_message  *p_message = &SIMU.message;

	ExitAlloue((struct_neutre **)(&p_msg_emis),
				sizeof(struct_emi_pic_message),
				NOYAU_GetPoolId("SIMUEMIPIC"));

	p_msg_emis->entete.service = service;
	p_msg_emis->entete.type_message = type_message;

	switch( service )
	{	
		case M_EMI_PIC_MAINT_MODE:
			memcpy( &p_msg_emis->u.srv_maint_mode, &p_message->u.srv_maint_mode, sizeof(struct_maint_mode) );
			break;
		default:
			break;
	}


	if( Envoie( MailboxID,
				SIMU.MAINT_bal_id,
				(struct_neutre *)(p_msg_emis)) != NOYAU_OK )
	{
		ExitLibere((struct_neutre **)&p_msg_emis);
	}
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
PRIVATE VOID HandleMaintenanceService( struct_emi_pic_message *lpMessage )
{
	LONG
		i;
	LP_MODULE_INSTANCE_DATA
		lpModuleData;
	BOOL
		SubscribedOnAllInstances;
	

	lpModuleData = NULL;
	for( i = 0; SIMU.NbModuleInstances; i++ )
	{
		if( SIMU.ModuleData[i].MailboxID != lpMessage->entete.neutre.bl_retour )
		{
			continue;
		}
		
		lpModuleData = &SIMU.ModuleData[i];
		break;
	}

	if( lpModuleData == NULL )
	{// Unknown mailbox message, ignore
		return;
	}

	switch( lpMessage->entete.type_message )
	{
		case SRV_TYP_DEBUT_ACQ:
			lpModuleData->SubscribedServices[lpMessage->entete.service] = TRUE;
			lpModuleData->Completed = TRUE;

			SubscribedOnAllInstances = TRUE;

			// If subscribed to MAINT MODE service on all instances, enable start button
			for( i = 0; i < SIMU.NbModuleInstances; i++ )
			{
				if( SIMU.ModuleData[i].SubscribedServices[M_EMI_PIC_MAINT_MODE] != TRUE )
				{// Not subscribed to service
					SubscribedOnAllInstances = FALSE;
					break;
				}
			}

			if( SubscribedOnAllInstances != FALSE )
			{
				SetStatus( APP_STATUS_READY );
			}
			break;

		case SRV_TYP_FIN_ACQ:
			lpModuleData->SubscribedServices[lpMessage->entete.service] = FALSE;
			// Disable start button
			EnableWindow( GetDlgItem( SIMU.hDlg, IDSTART ), FALSE );
			PostMessage( SIMU.hDlg, WM_CLOSE, 0, 0 );			
			break;

		case M_EMI_PIC_MAINT_MODE_START_ACK:
			PostMessage( GetDlgItem( lpModuleData->hDlg, IDC_PROGRESS ),
				PBM_SETRANGE,
				(WPARAM)0,
				MAKELPARAM( 0, lpMessage->u.srv_maint_mode.u.dirs_data.NbDirs ) );
			
			PostMessage( GetDlgItem( lpModuleData->hDlg, IDC_PROGRESS ),
				PBM_SETSTEP,
				(WPARAM)1,
				(LPARAM)0 );

			SetStatus( APP_STATUS_WORKING );			
			break;

		case M_EMI_PIC_MAINT_MODE_UPDATE:
			PostMessage( GetDlgItem( lpModuleData->hDlg, IDC_PROGRESS ),
				PBM_STEPIT,
				(WPARAM)0,
				(LPARAM)0 );
			break;
			
		case M_EMI_PIC_MAINT_MODE_EFFECTUE:

			lpModuleData->Completed = TRUE;
			lpModuleData->NbFilesTransfered = lpMessage->u.srv_maint_mode.u.NbFiles;
			
			if( AllInstancesCompleted() )
			{
				// Enable browse buttons
				for( i = 0; i < SIMU.NbModuleInstances; i++ )
				{
					EnableWindow( GetDlgItem( SIMU.ModuleData[i].hDlg, IDC_BROWSE ), TRUE );
					
				}
				// Enable start button
				EnableWindow( GetDlgItem( SIMU.hDlg, IDSTART ), TRUE );
				
				SetStatus( APP_STATUS_COMPLETED );
				ReportMaintComplete();
			}
			break;
		
		default:
			break;
	}
}
/*--------------------------- END FILE ----------------------------*/
 