/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_ani.c                                                   */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The animation thread functions.                               */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
//#include <wingdi.h>
// Module interface
#include <emi_pic_glob.h>
#include <emi_pic_serv.h>
#include <emi_pic_mess.h>
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE short int InitEmiPicAni(void *param);
PRIVATE void EmiPicAniReceptionEmi(short siInstId,
                                   struct_neutre *pNeutre);
PRIVATE void EmiPicAniReceptionRestit(short siInstId,
                                   struct_neutre *pNeutre);
PRIVATE void EmiPicAniReceptionTrf(short siInstId,
                                   struct_neutre *pNeutre);
PRIVATE void EmiPicReceptionService(short siInstId,
                                   struct_neutre *pNeutre);
PRIVATE void EmiPicGestionArretAni(short siInstId);
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE: ------------------------------------*/
/**/
/*****************************************************************************/
/*SYNTAX: DWORD WINAPI EmiPicAni(PVOID pvParam)                              */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function receives messages from the main application,     */
/*            processes received messages, and sends processed messages to   */
/*            other threads.                                                 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  PVOID pvParam  - Pointer to the instance ID.                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  0                     This function allways returns zero.                */
/*****************************************************************************/
PROTECTED DWORD WINAPI EmiPicAni(PVOID pvParam)
{
	struct_neutre *pNeutre;
	noyau_enum_retour eResult;
	short siInstId;

	// Thread initialisation (get instance id)
	siInstId = InitEmiPicAni(pvParam);

	// Bad instance id
	if(siInstId >= EMI_PIC_INSTANCE_MAX)
	{
		ExitBad();
	}

	ChangePriorite(TacheCourante(), EMI_PIC[siInstId].dwPrioMax);

	// Wait and process the messages
	while(TRUE)
	{
		// Wait for the message in the ANI thread mailbox
		// (infinite timeout)
		eResult = Recoit(EMI_PIC[siInstId].dwAniBalId,
						 (struct_neutre **)(&pNeutre), 
						 NOYAU_ATTENTE_INFINIE);

		// Protecting the message processing
		DebutRegion();

		if(eResult == NOYAU_BAL_MESS)
		{
			if(pNeutre->bl_retour == EMI_PIC[siInstId].dwEmiBalId)
			{
				// Message received from the EMI thread
				EmiPicAniReceptionEmi( siInstId, pNeutre );
			}
			else if(pNeutre->bl_retour == EMI_PIC[siInstId].dwRestitBalId)
			{
				// Message received from the RESTIT thread
				EmiPicAniReceptionRestit(siInstId, pNeutre);
			}
			else if(pNeutre->bl_retour == EMI_PIC[siInstId].dwTrfBalId)
			{
				// Message received from the TRF thread
				EmiPicAniReceptionTrf(siInstId, pNeutre);
			}
			else
			{
				// Message received from the user
				EmiPicReceptionService(siInstId, pNeutre);
			}

			// Free the message in the mailbox
			ExitLibere((struct_neutre **)(&pNeutre));
		}

		FinRegion();
	}

	return 0;
}

/**/
/*****************************************************************************/
/*SYNTAX: short int InitEmiPicAni(void *pvParam)                             */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function opens the exported mailbox and waits for the      */
/*            internal mailbox publication.                                  */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  void *pvParam   - Poiter to the instance ID.                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  short int             Instance ID.                                       */
/*****************************************************************************/
PRIVATE short int InitEmiPicAni(void *pvParam)
{
	short siInstId;
	// MFR start 10/7/2008
	noyau_bal_id
		BalEMI;
	// MFR end   10/7/2008

	// Get instance id
	siInstId = (short)pvParam;
    
	// Publication of 'EMI_PIC' exported mailbox
	// (for user(s) of this software component)
	EMI_PIC[siInstId].dwAniBalId = PublieBAL(EMI_PIC[siInstId].szAniBalName, 0);
	if(EMI_PIC[siInstId].dwAniBalId <= 0)
		ExitBad();

	// Waiting for the internal mailbox publication (by EMI thread)
	BalEMI = AttendBAL(EMI_PIC[siInstId].szEmiBalName);
	if( BalEMI <= 0)
		ExitBad();

	return (siInstId);
}
/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicAniReceptionEmi(short siInstId, struct_neutre *pNeutre)  */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function processes the messages received from the EMI      */
/*            thread.                                                        */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId          - Instance ID.                           */
/*      IN  struct_neutre *pNeutre  - Message structure received in the mail */
/*                                    box.                                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void EmiPicAniReceptionEmi(short siInstId, struct_neutre *pNeutre)
{
	struct_emi_pic_int_message *pMessage = (struct_emi_pic_int_message *)pNeutre;

	struct_emi_pic_message
		sMessage;
	struct_emi_pic_envoi_service
		sData = { siInstId, &sMessage };
	

	// Type of the message
	switch(pMessage->entete.type_message)
	{
		case M_EMISSION_PURGE_EFFECTUE:
			EmiPicEnvoiEmi(siInstId, 
						   pMessage, 
						   M_EMI_PIC_TYP_EMISSION_PURGE_EFFECTUE);
			break;

		case M_ARRET_EFFECTUE:
			EmiPicFichierTrace(siInstId,
							"EMI_PIC_ANI : EmiPicAniReceptionEmi => ARRET effectue");
			// Stopping ANI thread
			EMI_PIC[siInstId].bEmiStopped = TRUE;
			EmiPicGestionArretAni(siInstId);
			break;

		case M_STATUS_CHANGED:
			EmiPicEnvoiEtat( siInstId, &(EMI_PIC[siInstId].sStatus), SRV_TYP_NOUVEL_ETAT );
			break;

		case M_EMI_PIC_MAINT_ACCEPTED:
			EmiPicEnvoiMaintMode( siInstId, pMessage, M_EMI_PIC_MAINT_MODE_START_ACK );
			break;
			
		case M_EMI_PIC_MAINT_EFFECTUE:
			sMessage.u.srv_maint_mode.u.NbFiles = pMessage->u.sAniMaintMode.u.NbFiles;
			EmiPicEnvoiMaintMode( siInstId, pMessage, M_EMI_PIC_MAINT_MODE_EFFECTUE );
			break;
			
		case M_EMI_PIC_MAINT_NON_EFFECTUE:
			EmiPicEnvoiMaintMode( siInstId, pMessage, M_EMI_PIC_MAINT_MODE_NON_EFFECTUE );
			break;

		case M_EMI_PIC_MAINT_UPDATE:
			EmiPicEnvoiMaintMode( siInstId, pMessage, M_EMI_PIC_MAINT_MODE_UPDATE );
			break;

		default :
			EmiPicFichierDebug(siInstId,
							   "EMI_PIC_ANI ***** EmiPicAniReceptionEmi => type_message '%d' inconnu *****",
							   pMessage->entete.type_message);
			break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicAniReceptionRestit(short siInstId,                      */
/*                                      struct_neutre *pNeutre)              */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function processes the messages received from the RESTIT   */
/*            thread.                                                        */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId          - Instance ID.                           */
/*      IN  struct_neutre *pNeutre  - Message structure received in the mail */
/*                                    box.                                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void EmiPicAniReceptionRestit(short siInstId,
                                struct_neutre *pNeutre)
{
	struct_emi_pic_int_message *pMessage = (struct_emi_pic_int_message *)pNeutre;

	// Type of the message
	switch(pMessage->entete.type_message)
	{
		case M_RESTIT_DATE_EFFECTUE:
			EmiPicEnvoiRestit(siInstId, 
							  pMessage, 
							  M_EMI_PIC_TYP_RESTIT_BY_DATE_EFFECTUE);
			break;

		case M_RESTIT_DATE_NON_EFFECTUE:
			EmiPicEnvoiRestit(siInstId, 
							  pMessage, 
							  M_EMI_PIC_TYP_RESTIT_BY_DATE_NON_EFFECTUE);
			break;

		case M_RESTIT_FILE_EFFECTUE:
			EmiPicEnvoiRestit(siInstId, 
							  pMessage, 
							  M_EMI_PIC_TYP_RESTIT_BY_FILE_EFFECTUE);
			break;

		case M_RESTIT_FILE_NON_EFFECTUE:
			EmiPicEnvoiRestit(siInstId, 
							  pMessage, 
							  M_EMI_PIC_TYP_RESTIT_BY_FILE_NON_EFFECTUE);
			break;

			// ***************************
		case M_RESTIT_DATETIME_EFFECTUE:
			EmiPicEnvoiRestit(siInstId, 
				pMessage, 
				M_EMI_PIC_TYP_RESTIT_BY_DATETIME_EFFECTUE);
			break;

		case M_RESTIT_DATETIME_NON_EFFECTUE:
			EmiPicEnvoiRestit(siInstId, 
				pMessage, 
				M_EMI_PIC_TYP_RESTIT_BY_DATETIME_NON_EFFECTUE);
			break;

		case M_RESTIT_RECORD_ID_EFFECTUE:
			EmiPicEnvoiRestit(siInstId, 
				pMessage, 
				M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID_EFFECTUE);
			break;

		case M_RESTIT_RECORD_ID_NON_EFFECTUE:
			EmiPicEnvoiRestit(siInstId, 
				pMessage, 
				M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID_NON_EFFECTUE);
			break;

		case M_ARRET_EFFECTUE:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_ANI : EmiPicAniReceptionRestit => ARRET effectue");
			// Stopping ANI thread
			EMI_PIC[siInstId].bRestitStopped = TRUE;
			EmiPicGestionArretAni(siInstId);
			break;

		default :
			EmiPicFichierDebug(siInstId,
							   "EMI_PIC_ANI ***** EmiPicAniReceptionRestit => type_message '%d' inconnu *****",
							   pMessage->entete.type_message);
			break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicAniReceptionTrf(short siInstId,                         */
/*                                   struct_neutre *pNeutre)                 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function processes the messages received from the TRF      */
/*            thread.                                                        */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId          - Instance ID.                           */
/*      IN  struct_neutre *pNeutre  - Pointer to message received in the     */
/*                                    mailbox.                               */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void EmiPicAniReceptionTrf(short siInstId,
                                   struct_neutre *pNeutre)
{
	struct_emi_pic_int_message *pMessage = (struct_emi_pic_int_message *)pNeutre;

	// Type of the message
	switch(pMessage->entete.type_message)
	{
		case M_TRANSFER_SEND_EFFECTUE:
			EmiPicEnvoiTrf(siInstId, 
						  pMessage, 
						  M_EMI_PIC_TYP_TRANSFER_SEND_EFFECTUE);
			break;

		case M_TRANSFER_SEND_NON_EFFECTUE:
			EmiPicEnvoiTrf(siInstId, 
						  pMessage, 
						  M_EMI_PIC_TYP_TRANSFER_SEND_NON_EFFECTUE);
			break;

		case M_TRANSFER_DEL_EFFECTUE:
			EmiPicEnvoiTrf(siInstId, 
						  pMessage, 
						  M_EMI_PIC_TYP_TRANSFER_DEL_EFFECTUE);
			break;

		case M_TRANSFER_DEL_NON_EFFECTUE:
			EmiPicEnvoiTrf(siInstId, 
						  pMessage, 
						  M_EMI_PIC_TYP_TRANSFER_DEL_NON_EFFECTUE);
			break;

		case M_ARRET_EFFECTUE:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_ANI : EmiPicAniReceptionTrf => ARRET effectue");
			// Stopping ANI thread
			EMI_PIC[siInstId].bTrfStopped = TRUE;
			EmiPicGestionArretAni(siInstId);
			break;

		default:
			EmiPicFichierDebug(siInstId,
							   "EMI_PIC_ANI ***** EmiPicAniReceptionTrf => type_message '%d' inconnu *****",
							   pMessage->entete.type_message);
			break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicReceptionService(short siInstId,                        */
/*								      struct_neutre *pNeutre)                */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function processes the messages received from the          */
/*            application.                                                   */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId          - Instance ID.                           */
/*      IN  struct_neutre *pNeutre  - Pointer to message received in the     */
/*                                    mailbox.                               */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void EmiPicReceptionService(short siInstId,
                                    struct_neutre *pNeutre)
{
	struct_emi_pic_message *pMessage = (struct_emi_pic_message *)pNeutre;

	// type of the service
	switch(pMessage->entete.service)
	{
		// RESTITUTION service
		case M_EMI_PIC_RESTITUTION:
			EmiPicReceptionRestit(siInstId, pMessage);
			break;

		// TRANSFER service
		case M_EMI_PIC_TRANSFER:
			EmiPicReceptionTrf(siInstId, pMessage);
			break;

		// EMISSION service
		case M_EMI_PIC_EMISSION:
			EmiPicReceptionEmi(siInstId, pMessage);
			break;

		// ETAT service
		case M_SRV_ETAT:
			EmiPicReceptionEtat( siInstId, pMessage );
			break;

		// ARRET
		case M_SRV_ARRET:
			EmiPicReceptionArret(siInstId, pMessage);
			break;

		case M_EMI_PIC_MAINT_MODE:
			EmiPicReceptionMaintMode(siInstId, pMessage);
			break;

		// If service is not available or service is undefined
		default:
			if(pMessage->entete.service >= BUTEE_EMI_PIC_SERVICE)
			{
				EmiPicEnvoiAcquittement(siInstId,
										pMessage->entete.neutre.bl_retour,
										pMessage->entete.service,
										SRV_TYP_SERVICE_INCONNU);

				EmiPicFichierDebug(siInstId,
								   "EMI_PIC_ANI ***** EmiPicReceptionService => service_id '%d' inconnu *****",
								   pMessage->entete.service);
			}
			else
			{
				EmiPicEnvoiAcquittement(siInstId,
										pMessage->entete.neutre.bl_retour,
										pMessage->entete.service,
										SRV_TYP_SERVICE_NON_GERE);

				EmiPicFichierDebug(siInstId,
								   "EMI_PIC_ANI ***** EmiPicReceptionService => service_id '%d' non gere *****",
								   pMessage->entete.service);
			}
			break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicGestionArretAni(short siInstId)                         */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function stops the animation thread.                      */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId  - Instance ID.                                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PRIVATE void EmiPicGestionArretAni(short siInstId)
{
	if( EMI_PIC[siInstId].bTrfStopped == TRUE 
		&& EMI_PIC[siInstId].bRestitStopped == TRUE
		&& EMI_PIC[siInstId].bEmiStopped == TRUE )
	{
		EmiPicEnvoiAcquittement(siInstId, 
								EMI_PIC[siInstId].dwArretBalId, 
								M_SRV_ARRET, 
								SRV_TYP_EFFECTUE);

		EmiPicFichierTrace(siInstId,
						   "EMI_PIC_ANI : EmiPicGestionArretAni ==> Arret de la tache ANI");

		EmiPicLibereBAL(EMI_PIC[siInstId].dwAniBalId);
		Termine();
	}
}
/*-------------------------------- END OF FILE ------------------------------*/