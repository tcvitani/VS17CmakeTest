/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_serv.c                                                  */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the function that processes messages       */
/*             received from the application in the exported mailbox.        */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <wingdi.h>

// Module interface
#include <emi_pic_glob.h>
#include <emi_pic_mess.h>
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE: ------------------------------------*/
/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicReceptionEmi(short siInstId,                            */
/*                                struct_emi_pic_message *pMessage)          */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function processes the EMISSION service messages.         */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                    - Instance ID.                 */
/*      IN  struct_emi_pic_message *pMessage  - Pointer to the message.      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicReceptionEmi(short siInstId,
                                  struct_emi_pic_message *pMessage)
{
	struct_emi_pic_int_message sIntMessage;

	// Message type 
	switch(pMessage->entete.type_message)
	{
		// Start service
		case SRV_TYP_DEBUT:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionEmi ==> Bal '%d', Debut service EMISSION",
							   pMessage->entete.neutre.bl_retour);

			EmiPicDebutService(siInstId,
							   M_EMI_PIC_EMISSION,
							   pMessage->entete.neutre.bl_retour);
			break;
		// Stop service
		case SRV_TYP_FIN:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionEmi ==> Bal '%d', Fin service EMISSION",
							   pMessage->entete.neutre.bl_retour);

			EmiPicFinService(siInstId,
							 M_EMI_PIC_EMISSION,
							 pMessage->entete.neutre.bl_retour);
			break;
		// Purge all directories (cold start)
		case M_EMI_PIC_TYP_EMISSION_PURGE:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionTrf ==> Bal '%d', Demande EMISSION purge",
							   pMessage->entete.neutre.bl_retour);

			if(SrvEstDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_EMISSION],
				pMessage->entete.neutre.bl_retour) == TRUE)
			{
				EmiPicAniEnvoiEmi(siInstId, M_EMISSION_PURGE, pMessage);
			}
			else
			{
				memcpy(&sIntMessage.u.sAniEmi,
					   &pMessage->u.srv_emi,
					   sizeof(struct_emi_pic_emi));

				EmiPicEnvoiEmi(siInstId,
							   &sIntMessage,
							   M_EMI_PIC_TYP_EMISSION_PURGE_NON_EFFECTUE);
			}
			break;

		default:
			EmiPicFichierDebug(siInstId,
							  "EMI_PIC_SERV ***** EmiPicReceptionTrf ==> Service TRANSFER => type '%d' inconnu *****",
							  pMessage->entete.type_message);
			break;
	}
}
/*****************************************************************************/
/*SYNTAX: void EmiPicReceptionMaintMode(short siInstId,                      */
/*                                struct_emi_pic_message *pMessage)          */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function processes the EMISSION service messages.         */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                    - Instance ID.                 */
/*      IN  struct_emi_pic_message *pMessage  - Pointer to the message.      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicReceptionMaintMode( short siInstId,
										 struct_emi_pic_message *pMessage)
{
	struct_emi_pic_int_message sIntMessage;
	
	// Message type 
	switch(pMessage->entete.type_message)
	{
		// Start service
	case SRV_TYP_DEBUT:
		EmiPicFichierTrace(siInstId,
			"EMI_PIC_SERV : EmiPicReceptionEmi ==> Bal '%d', Debut service MAINT_MODE",
			pMessage->entete.neutre.bl_retour);
		
		EmiPicDebutService(siInstId,
			M_EMI_PIC_MAINT_MODE,
			pMessage->entete.neutre.bl_retour);
		break;
		// Stop service
	case SRV_TYP_FIN:
		EmiPicFichierTrace(siInstId,
			"EMI_PIC_SERV : EmiPicReceptionMaintMode ==> Bal '%d', Fin service MAINT_MODE",
			pMessage->entete.neutre.bl_retour);
		
		EmiPicFinService(siInstId,
			M_EMI_PIC_MAINT_MODE,
			pMessage->entete.neutre.bl_retour);
		break;
		// Purge all directories (cold start)
	case M_EMI_PIC_MAINT_MODE_START:
		EmiPicFichierTrace(siInstId,
			"EMI_PIC_SERV : EmiPicReceptionMaintMode ==> Bal '%d', Demande MAINT_MODE start",
			pMessage->entete.neutre.bl_retour);
		
		if(SrvEstDemandeur( EMI_PIC[siInstId].hService[M_EMI_PIC_MAINT_MODE],
							pMessage->entete.neutre.bl_retour) == TRUE )
		{
			// Force abort on current transfer attempt
			SetAbort( siInstId, TRUE );
			
			EmiPicAniEnvoiEmi(siInstId, M_EMI_PIC_MAINT_REQUEST, pMessage);
		}
		else
		{
			memcpy( &sIntMessage.u.sAniEmi,
					&pMessage->u.srv_emi,
					sizeof(struct_emi_pic_emi) );

			EmiPicEnvoiAcquittement( siInstId,
				pMessage->entete.neutre.bl_retour,
				M_EMI_PIC_MAINT_MODE,
				SRV_TYP_DEMANDE_INCONNUE );
		}
		break;

	// MFR start 2010/1/13
	case M_EMI_PIC_MAINT_MODE_STOP:
		EmiPicFichierTrace(siInstId,
			"EMI_PIC_SERV : EmiPicReceptionMaintMode ==> Bal '%d', Demande MAINT_MODE stop",
			pMessage->entete.neutre.bl_retour);
		
		if(SrvEstDemandeur( EMI_PIC[siInstId].hService[M_EMI_PIC_MAINT_MODE],
			pMessage->entete.neutre.bl_retour) == TRUE )
		{
			SetAbort( siInstId, TRUE );
		}
		else
		{
			memcpy( &sIntMessage.u.sAniEmi,
				&pMessage->u.srv_emi,
				sizeof(struct_emi_pic_emi) );
			
			EmiPicEnvoiAcquittement( siInstId,
				pMessage->entete.neutre.bl_retour,
				M_EMI_PIC_MAINT_MODE,
				SRV_TYP_DEMANDE_INCONNUE );
		}
		break;
	// MFR end

	default:
		EmiPicFichierDebug(siInstId,
			"EMI_PIC_SERV ***** EmiPicReceptionMaintMode ==> Service MAINT_MODE => type '%d' inconnu *****",
			pMessage->entete.type_message);
		break;
}}
/*****************************************************************************/
/*SYNTAX: void EmiPicReceptionRestit(short siInstId,                         */
/*                                   struct_emi_pic_message *pMessage)       */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function processes the RESTITUTION service messages.      */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                    - Instance ID.                 */
/*      IN  struct_emi_pic_message *pMessage  - Pointer to the message.      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicReceptionRestit(short siInstId,
                                     struct_emi_pic_message *pMessage)
{
	struct_emi_pic_int_message sIntMessage;

	// Message types 
	switch(pMessage->entete.type_message)
	{
		// Start service
		case SRV_TYP_DEBUT:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionRestit ==> Bal '%d', Debut service RESTITUTION",
							   pMessage->entete.neutre.bl_retour);

			EmiPicDebutService(siInstId,
							   M_EMI_PIC_RESTITUTION,
							   pMessage->entete.neutre.bl_retour);
			break;
		// Stop service
		case SRV_TYP_FIN:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionRestit ==> Bal '%d', Fin service RESTITUTION",
							   pMessage->entete.neutre.bl_retour);

			EmiPicFinService(siInstId,
							 M_EMI_PIC_RESTITUTION,
							 pMessage->entete.neutre.bl_retour);
			break;
		// Retransmission by date
		case M_EMI_PIC_TYP_RESTIT_BY_DATE:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionRestit ==> Bal '%d', Demande RESTITUTION by date",
							   pMessage->entete.neutre.bl_retour);

			if(SrvEstDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_RESTITUTION],
				pMessage->entete.neutre.bl_retour) == TRUE)
			{
				EmiPicAniEnvoiRestit(siInstId, M_RESTIT_DATE_REQUEST, pMessage);
			}
			else
			{
				memcpy(&sIntMessage.u.sAniRestit.sRestit,
					   &pMessage->u.srv_restit,
					   sizeof(struct_emi_pic_restit));
				EmiPicEnvoiRestit(siInstId, 
								  &sIntMessage, 
								  M_EMI_PIC_TYP_RESTIT_BY_DATE_NON_EFFECTUE);
			}
			break;
		// Retransmission by file name
		case M_EMI_PIC_TYP_RESTIT_BY_FILE:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionRestit ==> Bal '%d', Demande RESTITUTION by file name",
							   pMessage->entete.neutre.bl_retour);

			if(SrvEstDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_RESTITUTION],
				pMessage->entete.neutre.bl_retour) == TRUE)
			{
				EmiPicAniEnvoiRestit(siInstId, M_RESTIT_FILE_REQUEST, pMessage);
			}
			else
			{
				memcpy(&sIntMessage.u.sAniRestit.sRestit,
					   &pMessage->u.srv_restit,
					   sizeof(struct_emi_pic_restit));
				EmiPicEnvoiRestit(siInstId, 
								  &sIntMessage, 
								  M_EMI_PIC_TYP_RESTIT_BY_FILE_NON_EFFECTUE);
			}
			break;
			// Retransmission by datetime
		case M_EMI_PIC_TYP_RESTIT_BY_DATETIME:
			EmiPicFichierTrace(siInstId,
				"EMI_PIC_SERV : EmiPicReceptionRestit ==> Bal '%d', Demande RESTITUTION by date and time",
				pMessage->entete.neutre.bl_retour);
			
			if(SrvEstDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_RESTITUTION],
				pMessage->entete.neutre.bl_retour) == TRUE)
			{
				EmiPicAniEnvoiRestit(siInstId, M_RESTIT_DATETIME_REQUEST, pMessage);
			}
			else
			{
				memcpy(&sIntMessage.u.sAniRestit.sRestit,
					&pMessage->u.srv_restit,
					sizeof(struct_emi_pic_restit));
				EmiPicEnvoiRestit(siInstId, 
					&sIntMessage, 
					M_EMI_PIC_TYP_RESTIT_BY_DATETIME_NON_EFFECTUE);
			}
			break;

		case M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID:
			EmiPicFichierTrace(siInstId,
				"EMI_PIC_SERV : EmiPicReceptionRestit ==> Bal '%d', Demande RESTITUTION by record ID",
				pMessage->entete.neutre.bl_retour);
			
			if(SrvEstDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_RESTITUTION],
				pMessage->entete.neutre.bl_retour) == TRUE)
			{
				EmiPicAniEnvoiRestit(siInstId, M_RESTIT_RECORD_ID_REQUEST, pMessage);
			}
			else
			{
				memcpy(&sIntMessage.u.sAniRestit.sRestit,
					&pMessage->u.srv_restit,
					sizeof(struct_emi_pic_restit));
				EmiPicEnvoiRestit(siInstId, 
					&sIntMessage, 
					M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID_NON_EFFECTUE);
			}
			break;
			
		default:
			EmiPicFichierDebug(siInstId,
							   "EMI_PIC_SERV ***** EmiPicReceptionRestit ==> Service RESTITUTION => type '%d' inconnu *****",
							   pMessage->entete.type_message);
			break;
	}
}
/*****************************************************************************/
/*SYNTAX: void EmiPicReceptionEtat(short siInstId,                         */
/*                                   struct_emi_pic_message *pMessage)       */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function processes the RESTITUTION service messages.      */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                    - Instance ID.                 */
/*      IN  struct_emi_pic_message *pMessage  - Pointer to the message.      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicReceptionEtat( short siInstId,
                                    struct_emi_pic_message *pMessage)
{
	struct_emi_pic_message
		*lpEmiPicMsg;

	// Message types 
	switch(pMessage->entete.type_message)
	{
		// Start service
		case SRV_TYP_DEBUT:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionEtat ==> Bal '%d', Debut service ETAT",
							   pMessage->entete.neutre.bl_retour);

			if( EmiPicDebutService(siInstId,
							   M_SRV_ETAT,
							   pMessage->entete.neutre.bl_retour) != SRV_TYP_DEBUT_NACQ )
			{
				EmiPicEnvoiEtat( siInstId, &(EMI_PIC[siInstId].sStatus), SRV_TYP_NOUVEL_ETAT );				
			}
			break;
		// Stop service
		case SRV_TYP_FIN:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionEtat ==> Bal '%d', Fin service ETAT",
							   pMessage->entete.neutre.bl_retour);

			EmiPicFinService(siInstId,
							 M_SRV_ETAT,
							 pMessage->entete.neutre.bl_retour);
			break;

		case SRV_TYP_DEMANDE:
			EmiPicFichierTrace(siInstId,
				"EMI_PIC_SERV : EmiPicReceptionEtat ==> Bal '%d', Demande ETAT",
				pMessage->entete.neutre.bl_retour);

			// Allocating the memory for response
			ExitAlloue( (struct_neutre **)(&lpEmiPicMsg),
				sizeof(struct_emi_pic_message),
				EMI_PIC[siInstId].hPool );

			if( lpEmiPicMsg == NULL )
			{
				EmiPicFichierDebug( siInstId, "EmiPicReceptionEtat()->Error allocating memory for message" );
				_flushall();
				ExitBad();
			}

			lpEmiPicMsg->entete.service = M_SRV_ETAT;			

			if( SrvEstDemandeur( EMI_PIC[siInstId].hService[M_SRV_ETAT],
								 pMessage->entete.neutre.bl_retour) == TRUE )
			{
				lpEmiPicMsg->entete.type_message = SRV_TYP_NOUVEL_ETAT;

				memcpy( &(lpEmiPicMsg->u.srv_etat.status), &(EMI_PIC[siInstId].sStatus), sizeof(struct_emi_pic_etat) );
			}
			else
			{
				lpEmiPicMsg->entete.type_message = SRV_TYP_DEMANDE_NACQ;
			}

			// Sending the message
			if( Envoie( pMessage->entete.neutre.bl_retour,
						EMI_PIC[siInstId].dwAniBalId,
						(struct_neutre *)lpEmiPicMsg) != NOYAU_OK)
			{
				ExitLibere((struct_neutre **)&lpEmiPicMsg);
			}
			break;
			
		default:
			EmiPicFichierDebug(siInstId,
							   "EMI_PIC_SERV ***** EmiPicReceptionEtat ==> Service ETAT => type '%d' inconnu *****",
							   pMessage->entete.type_message);
			break;
	}
}
/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicReceptionTrf(short siInstId,                            */
/*                                struct_emi_pic_message *pMessage)          */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function processes the TRANSFER service messages.         */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                    - Instance ID.                 */
/*      IN  struct_emi_pic_message *pMessage  - Pointer to the message.      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicReceptionTrf(short siInstId,
                                  struct_emi_pic_message *pMessage)
{
	struct_emi_pic_int_message sIntMessage;

	// Message type 
	switch(pMessage->entete.type_message)
	{
		// Start service
		case SRV_TYP_DEBUT:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionTrf ==> Bal '%d', Debut service TRANSFER",
							   pMessage->entete.neutre.bl_retour);

			EmiPicDebutService(siInstId,
							   M_EMI_PIC_TRANSFER,
							   pMessage->entete.neutre.bl_retour);
			break;
		// Stop service
		case SRV_TYP_FIN:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionTrf ==> Bal '%d', Fin service TRANSFER",
							   pMessage->entete.neutre.bl_retour);

			EmiPicFinService(siInstId,
							 M_EMI_PIC_TRANSFER,
							 pMessage->entete.neutre.bl_retour);
			break;
		// Send files from requested directory
		case M_EMI_PIC_TYP_TRANSFER_SEND:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionTrf ==> Bal '%d', Demande TRANSFER send",
							   pMessage->entete.neutre.bl_retour);

			if(SrvEstDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_TRANSFER],
				pMessage->entete.neutre.bl_retour) == TRUE)
			{
				EmiPicAniEnvoiTrf(siInstId, M_TRANSFER_SEND, pMessage);
			}
			else
			{
				memcpy(&sIntMessage.u.sAniTrf,
					   &pMessage->u.srv_trf,
					   sizeof(struct_emi_pic_trf));

				EmiPicEnvoiTrf(siInstId, 
							   &sIntMessage, 
							   M_TRANSFER_SEND_NON_EFFECTUE);
			}
			break;
		// Delete files from requested directory
		case M_EMI_PIC_TYP_TRANSFER_DEL:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionTrf ==> Bal '%d', Demande TRANSFER delete",
							   pMessage->entete.neutre.bl_retour);

			if(SrvEstDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_TRANSFER],
				pMessage->entete.neutre.bl_retour) == TRUE)
			{
				EmiPicAniEnvoiTrf(siInstId, M_TRANSFER_DEL, pMessage);
			}
			else
			{
				memcpy(&sIntMessage.u.sAniTrf,
					   &pMessage->u.srv_trf,
					   sizeof(struct_emi_pic_trf));

				EmiPicEnvoiTrf(siInstId, 
							   &sIntMessage, 
							   M_TRANSFER_DEL_NON_EFFECTUE);
			}
			break;

		default:
			EmiPicFichierDebug(siInstId,
							  "EMI_PIC_SERV ***** EmiPicReceptionTrf ==> Service TRANSFER => type '%d' inconnu *****",
							  pMessage->entete.type_message);
			break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicReceptionArret(short siInstId,                          */
/*                                  struct_emi_pic_message *pMessage)        */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function processes the ARRET service messages.            */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                    - Instance ID.                 */
/*      IN  struct_emi_pic_message *pMessage  - Pointer to the message.      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicReceptionArret(short siInstId,
                                    struct_emi_pic_message *pMessage)
{
	// Message type
	switch(pMessage->entete.type_message)
	{
		case SRV_TYP_DEMANDE:
			EmiPicFichierTrace(siInstId,
							   "EMI_PIC_SERV : EmiPicReceptionArret ==> Bal '%d', demande ARRET ",
							   pMessage->entete.neutre.bl_retour);

			if(EMI_PIC[siInstId].dwArretBalId == -1)
			{
				EMI_PIC[siInstId].dwArretBalId = pMessage->entete.neutre.bl_retour;

				EmiPicEnvoiAcquittement(siInstId,
										pMessage->entete.neutre.bl_retour,
										M_SRV_ARRET,
										SRV_TYP_DEMANDE_ACQ);

				// Send ARRET message to the TRF thread
				EmiPicAniEnvoiTrf(siInstId, M_ARRET_DEMANDE, NULL);
				// Send ARRET message to the RESTIT thread
				EmiPicAniEnvoiRestit(siInstId, M_ARRET_DEMANDE, NULL);
				// Send ARRET message to the EMI thread
				EmiPicAniEnvoiEmi(siInstId, M_ARRET_DEMANDE, NULL);
				// Send ARRET message to the IMPORT thread
				EmiPicAniEnvoiImport(siInstId, M_ARRET_DEMANDE, NULL);
			}
			else
				EmiPicEnvoiAcquittement(siInstId,
										pMessage->entete.neutre.bl_retour,
										M_SRV_ARRET,
										SRV_TYP_DEMANDE_NACQ);
			break;

		default:
			EmiPicFichierDebug(siInstId,
							   "EMI_PIC_SERV ***** EmiPicReceptionArret ==> Service ARRET => type '%d' inconnu, bal emettrice '%d', Bal '%d' *****",
							   pMessage->entete.type_message,
							   pMessage->entete.neutre.bl_retour,
							   pMessage->entete.neutre.bl_retour );
			break;
	}
}
/*-------------------------------- END OF FILE ------------------------------*/