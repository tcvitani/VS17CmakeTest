/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES SRVPV
* FICHIER: plaza_msg_event.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter le message d'event des voies
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/route_msg_event.c_v  $
*
/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     route_msg_event.c												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <stdio.h>

#include <alarm.h>
#include <plaza.h>
#include <msg_lc_event.h>
#include <pv_wm.h>
#include <msg_pv_com_req.h>
#include <route_wm.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define MAX_LANENUM	4


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED DWORD Valid_Response_Get_Lanenum(CHAR *p_message);

PROTECTED void ProcessValidationReqEvent(struct LANE_CNF *p_lane,
										struct MSG_EVENT *p_event,
										CHAR *pDescription,
										DWORD dwDescLen,
										DWORD dwDescSize);

/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL ROUTE_Process_Msg_Event(struct MSG_EVENT *p_event)
{
	struct LANE_CNF						*p_lane			= NULL;
	struct MSG_EVENT_Event_Description	*p_event_desc	= NULL;
	BOOL								bRet			= TRUE;
	CHAR								szDescription[MSG_EVENT_ANOMALY_DESCRIPTION_LENGTH] = { 0 };

	// search for lane
	p_lane = PLAZA_Search_Lane(p_event->header.lane_number);
	if (p_lane == NULL)
		return FALSE;

	// get event description
	p_event_desc = MSG_EVENT_Get_First_Anomaly_Description(p_event->body.list_event_descritpion);
	if (p_event_desc != NULL && p_event_desc->description.type == MSG_FIELD_VARSTR)
		strcpy_s(szDescription, sizeof(szDescription), p_event_desc->description.String);

	// processing validation request
	ProcessValidationReqEvent(p_lane, p_event, szDescription, (DWORD)strlen(szDescription), sizeof(szDescription));

	// processing of status modifying the state of the lane: shift and mode
	bRet &= ALARM_Set_Lane_Shift_And_Mode(p_lane,
										p_event->header.toll_collector_id,
										p_event->header.active_vault_id,
										p_event->body.event_category,
										p_event->body.event_sub_category,
										p_event->body.current_event_status);

	// processing of status modifying the state of the lane: active vault
	bRet &= ALARM_Set_Lane_Active_Vault(p_lane,
										p_event->header.active_vault_id,
										p_event->body.event_category);

	// test if changed state of lane equipment
	bRet &= LANE_Search_And_Set_Eqt(p_lane,
									p_event->body.event_category,
									p_event->body.event_sub_category,
									p_event->body.current_event_status);

	// generate alarm
	bRet &= ALARM_Search_And_Set_List(p_lane,
									p_event->body.event_category,
									p_event->body.event_sub_category,
									p_event->body.current_event_status,
									&p_event->body.time_of_event,
									szDescription);
	
	return bRet;
}

PROTECTED void ProcessValidationReqEvent(struct LANE_CNF *p_lane,
										 struct MSG_EVENT *p_event,
										 CHAR *pDescription,
										 DWORD dwDescLen,
										 DWORD dwDescSize)
{
	struct LANE_CNF					*pFreeLane		= NULL;
	struct LANE_CNF					*pBusyLane		= NULL;
	struct MSG_PV_COM_REQ_Lanenum	*pLanenNumber	= NULL;
	struct MSG_PV_COM_REQ			*pComReq		= MSG_PV_COM_REQ_New();

	// Validation request event
	if (p_event->body.event_category == VALIDATION_REQUEST_TYPE &&
		p_event->body.event_sub_category == VALIDATION_REQUEST_SUBTYPE &&
		p_event->body.current_event_status == VALIDATION_REQUEST_STATUS)
	{
		// Is there any manual lane
		if (PLAZA_Search_For_Manual_Lane() == TRUE)
		{
			// Is there any active-free lane
			pFreeLane = PLAZA_Search_For_ActiveFree_Lane();
			if (pFreeLane != NULL)
			{
				// Prepare command
				pComReq->body.id = 120;
				pComReq->body.cd = 72;

				// Command_ID, size of command information, command information
				sprintf_s(pComReq->body.command, sizeof(pComReq->body.command), "%02u%03u%s", VALIDATION_REQUEST_COMMAND_ID, dwDescLen, pDescription);
				pComReq->body.size_of_command = (DWORD)strlen(pComReq->body.command);

				pLanenNumber = MSG_PV_COM_REQ_Lanenum_New(&pComReq->list_lanenum);
				pLanenNumber->lanenum = pFreeLane->msg_lane->body.lanenum;

				// Forward validation request on the active-free manual lane
				ROUTE_Command(pComReq);

				//Set lane to busy state
				pFreeLane->LaneActiveState = BUSY_STATE;
			}
			else if (dwNbOfPVConnection != 0)	// Else if there any PV connected, forward validation request on the PV		
				strcat_s(pDescription, dwDescSize, "|PV");
			else //Else if there any busy lane //if(PLAZA_Search_For_Busy_Lane() == TRUE)
			{
				//Prepare command				
				pComReq->body.id = 120;
				pComReq->body.cd = 72;

				//Command_ID, size of command information, command information
				sprintf_s(pComReq->body.command, sizeof(pComReq->body.command), "%02u%03u%s%s", VALIDATION_RESPONSE_COMMAND_ID, dwDescLen + 4, pDescription, "|0|0|B");
				pComReq->body.size_of_command = (DWORD)strlen(pComReq->body.command);

				pLanenNumber = MSG_PV_COM_REQ_Lanenum_New(&pComReq->list_lanenum);
				pLanenNumber->lanenum = p_event->header.lane_number;

				//Send validation response
				ROUTE_Command(pComReq);
			}
		}
		else if (dwNbOfPVConnection != 0) // Else if there PV connected, send validation request on the PV
			strcat_s(pDescription, dwDescSize, "|PV");
		else // Else, send validation response with message "validation impossible"	
		{
			// Prepare command			
			pComReq->body.id = 120;
			pComReq->body.cd = 72;

			// Command_ID, size of command information, command information			
			sprintf_s(pComReq->body.command, sizeof(pComReq->body.command), "%02u%03u%s%s", VALIDATION_RESPONSE_COMMAND_ID, dwDescLen + 4, pDescription, "|0|0|I");
			pComReq->body.size_of_command = (DWORD)strlen(pComReq->body.command);

			pLanenNumber = MSG_PV_COM_REQ_Lanenum_New(&pComReq->list_lanenum);
			pLanenNumber->lanenum = p_event->header.lane_number;

			// Send validation response
			ROUTE_Command(pComReq);
		}
	}
	// Validation response event	
	else if (p_event->body.event_category == VALIDATION_RESPONSE_TYPE &&
		p_event->body.event_sub_category == VALIDATION_RESPONSE_SUBTYPE &&
		p_event->body.current_event_status == VALIDATION_RESPONSE_STATUS)
	{
		// Set lane to active-free		
		pBusyLane = PLAZA_Search_Lane(p_event->header.lane_number);
		if (p_lane != NULL)
			pBusyLane->LaneActiveState = ACTIVE_FREE_STATE;

		// Prepare command		
		pComReq->body.id = 120;
		pComReq->body.cd = 72;

		// Command_ID, size of command information, command information		
		sprintf_s(pComReq->body.command, sizeof(pComReq->body.command), "%02u%03u%s", VALIDATION_RESPONSE_COMMAND_ID, dwDescLen, pDescription);
		pComReq->body.size_of_command = (DWORD)strlen(pComReq->body.command);

		pLanenNumber = MSG_PV_COM_REQ_Lanenum_New(&pComReq->list_lanenum);
		pLanenNumber->lanenum = Valid_Response_Get_Lanenum(pDescription);

		// Send validation response
		ROUTE_Command(pComReq);
	}

	MSG_PV_COM_REQ_Delete_All(pComReq);
}

PROTECTED DWORD Valid_Response_Get_Lanenum(CHAR *p_message)
{
	CHAR	szLaneNum[MAX_LANENUM + 1]	= { 0 };
	DWORD	dwLaneNum					= 0;

	strncpy_s(szLaneNum, sizeof(szLaneNum), p_message, MAX_LANENUM);
	dwLaneNum = atoi(szLaneNum);

	if (dwLaneNum < 0)
		return 0;

	return dwLaneNum;
}

/*-------------------------------- END OF FILE ------------------------------*/