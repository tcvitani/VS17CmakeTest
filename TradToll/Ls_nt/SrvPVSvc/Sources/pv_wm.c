/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES PV
* FICHIER: pv_wm_dispatch.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter les window messages destiné
*         aux autoamte de PV  
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/pv_wm.c_v  $
 * 
 *    Rev 1.7   26 Apr 2002 11:44:54   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.5   26 Jan 2001 13:48:46   DSI
 * Ajout du message complémentaire de
 * transaction pour mise à jour temps réel
 * des classes + mise à niveau des dll
 * linkées
 * 
 *    Rev 1.4   19 Dec 2000 15:10:10   DSI
 * J-1 + filtre des classes et des paiements et
 * macro du strncpy
 * 
 *    Rev 1.3   13 Dec 2000 16:34:48   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.0   Nov 22 1999 11:00:36   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/
#include <windows.h>
#include <stdio.h>
#include <acom.h>
#include <ntsvc.h>
#include <pv_aut.h>
#include <pv_acom.h>
#include <pv_wm.h>
#include <pv_conf.h>
#include <csr_msg.h>
#include <srvpv_main.h>
#include <cmd.h>
#include <msg_pv_ack_req.h>
#include <msg_pv_user_req.h>
#include <msg_pv_log_req.h>
#include <msg_pv_com_req.h>
#include <msg_pv_auth_req.h>
#include <msg_pv_dec_req.h>
#include <msg_pv_gal_req.h>
#include <msg_pv_conf_req.h>
#include <msg_pv_plaz_rep.h>

#include <DB_Connection.h>
#include <srvpv_dbif.h>

/*--------------- EXTERNALS: ---------------*/
#include <memclass.h>

/*--------------- DEFINES: -----------------*/

// Nom texte du window message utilisé pour signaler les événements
#define PV_WM_EVENT_NAME				"WM_PV_EVENT"

#define	MSG_PV_CONN_STAT_REQ_CD			301
#define	MSG_PV_CONN_STAT_CD				"301"
#define MSG_PV_CONN_STAT_REP			"120301"

/*--------------- TYPEDEFS: ----------------*/

/*--------------- VARIABLES INITIALISEES (1): ---------------*/

PRIVATE struct
{
    // tableau des PV connectés
    aut_pv_id aut[PV_ACOM_MAX_CONNECTIONS];

    // liste des messages reconnus et en provenance des PV
    HLIST hlPVMsgList;

    // infos sur le thread avec sa file de "window messages"
    HANDLE hThread;
    DWORD ThreadId;
    DWORD WM_PV;
    DWORD WM_ACOM;
} 
PV = {0};

HLIST hLastLaneLinkStatus = NULL;

HLIST ghLinkStatus = NULL;

/*--------------- FUNCTIONS: ---------------*/

PRIVATE VOID FillMsgLinkStatus(ARG_EVT_PV_BUFFER *p_Buff);

PRIVATE BOOL PV_CMD_List_Users (ACOM_CNX_HANDLE hCnx);

PRIVATE BOOL PV_Connexion (ACOM_EVT_HANDLE hEvent)
{
	DWORD64 dwInstUsrKey;
    ACOM_CNX_HANDLE hCnxHandle;
    aut_pv_id *pPV;
    DWORD Cnx;
    

    // on récupère le handle de connexion
    AComGetConnectionEvtParameters (hEvent, &dwInstUsrKey, &hCnxHandle);

    // calcul d'un index dans le tableau des PV connecté 
    // a partir du handle de connexion
    if (hCnxHandle == 0)
    {
        NTSVCError( "PV_Connexion() => handle to connection is NULL" );

        return FALSE;
    }

    Cnx = ACOM_GET_SLOT_NUMBER (hCnxHandle);
    pPV = &PV.aut[Cnx];

	NTSVCInfo("PV_Connexion() => open connection hCnxHandle:%u Cnx:%u", hCnxHandle, Cnx);

	if (*pPV != NULL)
	{
		NTSVCError("PV_Connexion() => handle to connection %d is not NULL. Should clean up!", Cnx);
		//// Envoi de l'event de fin de session sur déconnexion
		//PV_AUT_Send(*pPV, EVT_PV_LOGOUT, NULL);

		// suppression de l'automate du PV 
		if (PV_AUT_Delete(pPV) == FALSE)
			return FALSE;
	}

    // initialisation de l'automate du PV connecté
	if (PV_AUT_New(pPV, hCnxHandle) == FALSE)
	{
		return FALSE;
	}

    return TRUE;
}

PRIVATE BOOL PV_Disconnexion (ACOM_EVT_HANDLE hEvent)
{
	DWORD64 dwInstUsrKey;
	DWORD64 hCnxHandle;
    aut_pv_id *pPV;
    DWORD Cnx;


    // on récupère le handle de connexion
    AComGetDisconnectionEvtParameters (hEvent, &dwInstUsrKey, &hCnxHandle);

    // calcul d'un index dans le tableau des PV connecté 
    // a partir du handle de connexion
    if (hCnxHandle == 0)
    {
        NTSVCError( "PV_Disconnexion() => handle to connection is NULL" );

        return FALSE;
    }

    Cnx = ACOM_GET_SLOT_NUMBER (hCnxHandle);
    pPV = &PV.aut[Cnx];

	NTSVCInfo("PV_Disconnexion(%u) => close connection hCnxHandle:%u ", Cnx, hCnxHandle);

	if (*pPV != NULL)
	{
		// Envoi de l'event de fin de session sur déconnexion
		PV_AUT_Send(*pPV, EVT_PV_LOGOUT, NULL);

		// suppression de l'automate du PV 
		if (PV_AUT_Delete(pPV) == FALSE)
			return FALSE;
	}
	else
	{
		NTSVCInfo("PV_Disconnexion() => %u already deleted! ", Cnx);
	}

    return TRUE;
}
    
PRIVATE BOOL PV_Translate_WM_To_Event_HMSG(void *pvDataBuffer, 
											DWORD dwDataSize,
											aut_event_id *aut_event,
											HMSG *hMsg)
{
    struct MSG_PV_HEADER	*pHeader					= NULL;
	struct MSG_PV_HEADER	sHeader						= {0};
	CHAR					string[SVC_MSG_BUFFER_SIZE] = {0};
	CHAR					szTmp[MAX_PATH]				= {0};

    strncpy_s(string, sizeof(string), pvDataBuffer, dwDataSize);
    string[dwDataSize] = '\0';
    
	NTSVCInfo ("PV_WM Receive (%lu / %s)", dwDataSize, string);

	*hMsg = NULL;

	if (memcmp(&string[3], MSG_PV_CONN_STAT_CD, 3) == 0)
	{
		pHeader = &sHeader;
		memcpy(szTmp, &string[3], 3);
		pHeader->cd = atol(szTmp);
	}
	else
	{
		// interprétation du message à l'aide du moteur MSG
		pHeader = MSG_New_Read_If_Found(PV.hlPVMsgList, pvDataBuffer, dwDataSize);
		if (pHeader == NULL)
			return FALSE;

		*hMsg = pHeader;
	}

    // on convertit ensuite le CD du message en event pour l'automate
    switch (pHeader->cd)
    {
		// demande d'ouverture de session d'un user
		case MSG_PV_LOG_REQ_CD:
			*aut_event = EVT_PV_LOGIN_REQ;
			break;

		// demande de profil d'un user
		case MSG_PV_USER_REQ_CD:
			*aut_event = EVT_PV_PROFILE_REQ;
			break;

		// demande de configuration de la gare
		case MSG_PV_CONF_REQ_CD:
			*aut_event = EVT_PV_PLAZA_CONF;
			break;

		// demande d'état courant de toute la gare
		case MSG_PV_GAL_REQ_CD:
			*aut_event = EVT_PV_GENERAL_STATE;
			break;

		// fin de session
		case MSG_PV_DEC_REQ_CD:
			*aut_event = EVT_PV_LOGOUT;
			break;

		// commande
		case MSG_PV_COM_REQ_CD:
			*aut_event = EVT_PV_COMMAND;
			break;

		// demande d'autorisation
		case MSG_PV_AUTH_REQ_CD:
			*aut_event = EVT_PV_AUTH_REQ;
			break;

		// demande d'acquittement d'alarmes
		case MSG_PV_ACK_REQ_CD:
			*aut_event = EVT_PV_ACK_ALARM;
			break;

		// connection (link) status request
		case MSG_PV_CONN_STAT_REQ_CD:
			*aut_event = EVT_PV_CONN_STAT_REQ;
			break;

		default:
			NTSVCInfo("PV_WM Receive: unknown PV msg cd:%u", pHeader->cd);
			// libération du Msg 
			MSG_Delete_All(hMsg);
			return FALSE;
    }

    return TRUE;
}

PRIVATE BOOL PV_Receive (ACOM_EVT_HANDLE hEvent)
{
	DWORD64 dwInstUsrKey;
	DWORD64 hCnxHandle;
    DWORD  dwDataSize;
    void * pvDataBuffer;
    aut_pv_id *pPV;
    aut_event_id aut_event;
    HMSG hMsg;
    DWORD Cnx;
    BOOL bRet = TRUE;

  
    // on récupère le handle de connexion et les données associées
    AComGetReceivedEvtParameters (hEvent, 
                                  &dwInstUsrKey, 
                                  &hCnxHandle,
                                  &dwDataSize,
                                  &pvDataBuffer);

    // calcul d'un index dans le tableau des PV connecté 
    // a partir du handle de connexion
    if (hCnxHandle == 0)
    {
        NTSVCError( "PV_Receive() => handle to connection is NULL" );
        return FALSE;
    }

    Cnx = ACOM_GET_SLOT_NUMBER (hCnxHandle);
    pPV = &PV.aut[Cnx];

	NTSVCInfo("PV_Receive(%u) => message received at hCnxHandle:%u", Cnx, hCnxHandle);

    // interprétation du msg du PV en événement d'automate et données associées
	bRet &= PV_Translate_WM_To_Event_HMSG(pvDataBuffer, dwDataSize, &aut_event, &hMsg);

    // Envoi du msg de l'event de déconnexion
    if (bRet)
    {
		bRet &= PV_AUT_Send(*pPV, aut_event, hMsg);

		if (hMsg != NULL)
			MSG_Delete_All(&hMsg);
    }

    return bRet;
}

PRIVATE BOOL PV_Dispatch (aut_event event, PVOID args)
{
    DWORD i;
    DWORD bRet = TRUE;

    // Pour tous les PV connectés
    // envoyé l'EVT avec ses arguments
    for (i = 0; i < PV_ACOM_MAX_CONNECTIONS; i++)
    {
        if (PV.aut[i] != NULL)
        {
            // Envoi d'un event d'etat gare totale
            bRet &= PV_AUT_Send (PV.aut[i], event, args);
        }
    }

    return bRet;
}

PRIVATE BOOL PV_Dispatch_Message (aut_event event, HMSG p_msg)
{
    ARG_EVT_PV_BUFFER args;
	DWORD bRet = TRUE;

    // on envoie directement un buffer pour des raisons d'efficacité :
    // le msg peut etre tres gros et serait donc long a généré
    // par chaque PV avant d'être envoyé sur le réseau...

    // conversion du msg en un buffer "ASCII"
	args.buffer_size = MSG_Write (p_msg, args.buffer, sizeof(args.buffer)); 
    
    // Pour tous les PV connectés
    // envoyé l'EVT de mise a jour de l'état complet de la gare
    if (args.buffer_size > 0)
        bRet &= PV_Dispatch (event, &args);

    // libération du message
    bRet &= MSG_Delete_All (&p_msg);

    return bRet;
}

PRIVATE BOOL PV_Dispatch_Message_Linka_Status(aut_event event, HMSG p_msg)
{
	ARG_EVT_PV_BUFFER	args	= {0};
	DWORD				bRet	= TRUE;

	memcpy(&args, p_msg, sizeof(ARG_EVT_PV_BUFFER));

	if (args.buffer_size > 0)
		bRet &= PV_Dispatch(event, &args);

	// free message
	free(p_msg);
	p_msg = NULL;

	return bRet;
}

PRIVATE BOOL PV_Send_To (aut_event event, PVOID args)
{
    ACOM_CNX_HANDLE *hCnxHandle = args;
    aut_pv_id *pPV;
    DWORD Cnx;
    DWORD bRet = TRUE;
    
    // on suppose que le message passé en argument
    // contient comme premier champ le handle de connexion du PV
    Cnx = ACOM_GET_SLOT_NUMBER (*hCnxHandle);
    pPV = &PV.aut[Cnx];

    // si le PV ne s'est pas déconnecté entre temps...
    if (*pPV != NULL)
    {
        // Envoi d'un event au PV
        bRet = PV_AUT_Send (*pPV, event, args);
    }



    return bRet;
}
 
PRIVATE DWORD WINAPI PV_WM_Dispatch (PVOID param)
{
    MSG msg;
    INT ret;
    DWORD i;

    // Création d'une file de window messages pour ce thread
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    // boucle de réception des messages
    do
    {
        // attente d'un message
        ret = GetMessage (&msg, NULL, 0, 0);

        // erreur de réception
        if (ret < 0)
            continue;

        // un message a traiter venant des PV via ACOM ?
        if (msg.message == PV.WM_ACOM)
        {
            switch (msg.wParam)
            {
				case ACOM_WM_SHUTDOWN:
					NTSVC_ERR ("PV_WM_Dispatch() => ACOM_WM_SHUTDOWN");
					ret = FALSE;
					break;

				case ACOM_WM_CONNECTION:
					ret = PV_Connexion (msg.lParam);
					break;

				case ACOM_WM_DISCONNECTION:
					ret = PV_Disconnexion (msg.lParam);
					break;

				case ACOM_WM_RECEIVED:
					ret = PV_Receive (msg.lParam);
					break;

				case ACOM_WM_SENT:
					// tant mieux...
					break;

				case ACOM_WM_TIME_SHIFT:
					// TO DO : mise a l'heure ??
					break;

				default:
					NTSVC_ERR ("PV_WM_Dispatch() => WM_ACOM unknown!");
					break;
            }
            
            // on libère l'event associé au message de ACOM
            AComReleaseEvt (msg.lParam);
        }
        // un message a traiter pour SRV PV ?
        else if (msg.message == PV.WM_PV)
        {
            switch (msg.wParam)
            {
				case PV_WM_CONNECTION_STATE:
					ret = PV_Dispatch_Message_Linka_Status(EVT_PV_CONN_STAT, (HMSG)msg.lParam);
					break;

				case PV_WM_PLAZA_STATE:
					ret = PV_Dispatch_Message (EVT_PV_PLAZA_STATE, (HMSG) msg.lParam);
					break;

				case PV_WM_ALARM_STATE:
					ret = PV_Dispatch_Message (EVT_PV_ALARM_STATE, (HMSG) msg.lParam);
					break;

				case PV_WM_COMP_INFO:
					ret = PV_Dispatch_Message (EVT_PV_COMP_INFO, (HMSG) msg.lParam);
					break;

				case PV_WM_BOWL_STATUS:
					ret = PV_Dispatch_Message (EVT_PV_BOWL_STATUS, (HMSG) msg.lParam);
					break;

				case PV_WM_VAULT_STATUS:
					ret = PV_Dispatch_Message (EVT_PV_VAULT_STATUS, (HMSG) msg.lParam);
					break;

				case PV_WM_PARTLY:
					ret = PV_Dispatch_Message (EVT_PV_PARTLY, (HMSG) msg.lParam);
					break;

				case PV_WM_TRACE:
					ret = PV_Dispatch_Message (EVT_PV_TRACE, (HMSG) msg.lParam);
					break;

				case PV_WM_DISK:
					ret = PV_Dispatch_Message (EVT_PV_DISK, (HMSG) msg.lParam);
					break;
	
				case PV_WM_SERVER_COM_FAILURE:
					if ((PVOID) msg.lParam == NULL)
						ret = PV_Dispatch (EVT_PV_COM_SERVER_FAILURE, NULL);
					else
					{
						ret = PV_Send_To(EVT_PV_COM_SERVER_FAILURE, (PVOID)msg.lParam);

						// free message ??
						free((PVOID)msg.lParam);
					}
					break;

				case PV_WM_DATA_BASE_FAILURE:
					if ((PVOID) msg.lParam == NULL)
						ret = PV_Dispatch (EVT_PV_DATA_BASE_FAILURE, NULL);
					else
					{
						ret = PV_Send_To(EVT_PV_DATA_BASE_FAILURE, (PVOID)msg.lParam);
						// free message ??
						free((PVOID)msg.lParam);

					}
					break;

				case PV_WM_RELOAD:
					ret = PV_Dispatch (EVT_PV_RELOAD_SERVER, NULL);
					break;

				case PV_WM_USER_LOGIN_REP:
				{		struct PV_WM_PARAM_USER_LOGIN_REP *p_login = (struct PV_WM_PARAM_USER_LOGIN_REP *)msg.lParam;
						ret = PV_Send_To(EVT_PV_LOGIN_REP, (PVOID)p_login);
						// free message ??
						free(p_login);
					}
					break;

				case PV_WM_USER_PROFILE_REP:
				{		struct PV_WM_PARAM_USER_PROFILE_REP	* p_rep = (struct PV_WM_PARAM_USER_PROFILE_REP	*)msg.lParam;
						struct MSG_PV_USER_REP	*p_profile = p_rep->p_profile;

						ret = PV_Send_To (EVT_PV_PROFILE_REP, (PVOID) msg.lParam);
						
						// suppression du message
						MSG_PV_USER_REP_Delete_All(p_rep->p_profile);
						free(p_rep);
					}
					break;

				case PV_WM_USER_AUTH_REP:
				{		struct PV_WM_PARAM_USER_AUTH_REP *p_auth = (struct PV_WM_PARAM_USER_AUTH_REP *)msg.lParam;
						ret = PV_Send_To (EVT_PV_AUTH_REP, (PVOID) msg.lParam);
						// free message ??
						free(p_auth);
					}
					break;

				case PV_WM_DB_CMD_REP:
					{
						ret = PV_Send_To (EVT_PV_CMD_REP, (PVOID) msg.lParam);
						// free message ??
						free((PVOID)msg.lParam);
					}
					break;

				case PV_WM_CMD_LIST_USERS:
					ret = PV_CMD_List_Users ((ACOM_CNX_HANDLE) msg.lParam); 
					break;

				default:
					NTSVC_ERR ("PV_WM_Dispatch() => WM_PV unknown!");
					break;
            }
        } 
        else if (msg.message == WM_QUIT)
        {
            ret = 0;
            break;
        }
        else
        {
            NTSVC_ERR ("PV_WM_Dispatch() => WM unknown!");
        }

        // pour que le serveur soit plus tolérant et robuste
        ret = TRUE;
    }
    // ret == 0 sur réception de WM_QUIT
    while (ret > 0);

    // Libérer tous les PV connectés
    for (i = 0; i < PV_ACOM_MAX_CONNECTIONS; i++)
    {
        if (PV.aut[i] != NULL)
        {
            // Envoi de l'event de fin de session sur déconnexion
            PV_AUT_Send (PV.aut[i], EVT_PV_ABORT, NULL);

            // suppression de l'automate du PV 
            PV_AUT_Delete (&PV.aut[i]);
        }
    }

    NTSVCInfo ("PV_WM_Dispatch() => thread stop!");

    return 0;
}

PROTECTED BOOL PV_Start (void)
{
    // RAZ de la strucutre interne
    memset (&PV, 0, sizeof (PV));

    // Création de la liste des messages reconnu des PV
    MSG_PV_ACK_REQ_New_Record (&PV.hlPVMsgList);
    MSG_PV_AUTH_REQ_New_Record (&PV.hlPVMsgList);
    MSG_PV_COM_REQ_New_Record (&PV.hlPVMsgList);
    MSG_PV_CONF_REQ_New_Record (&PV.hlPVMsgList);
    MSG_PV_LOG_REQ_New_Record (&PV.hlPVMsgList);
    MSG_PV_DEC_REQ_New_Record (&PV.hlPVMsgList);
    MSG_PV_GAL_REQ_New_Record (&PV.hlPVMsgList);
    MSG_PV_USER_REQ_New_Record (&PV.hlPVMsgList);


    // init des paramètres du registre utilisé par les automates de PV
    if (PV_AUT_Init() == FALSE)
        return FALSE;

    // recherche de la configureation des PV dans la base
    if (PV_Build_Msg_Conf_From_DB () == FALSE)
        return FALSE;

    // recherche des WM du module de COM réseau
    PV.WM_ACOM = RegisterWindowMessage (ACOM_WM_EVENT_NAME);
    if (PV.WM_ACOM == 0)
        return FALSE;

    // déclaration des WM du module lui-meme
    PV.WM_PV = RegisterWindowMessage (PV_WM_EVENT_NAME);
    if (PV.WM_ACOM == 0)
        return FALSE;

    // création du thread du module
    PV.hThread = NTSVCCreateThread (NULL, 0, PV_WM_Dispatch, NULL, 0, &PV.ThreadId, "SRVPVSVC PV Thread");
    if (PV.hThread == NULL)
        return FALSE;

    // Démarrage de la COM réseau
    if (PV_ACOM_Start (PV.ThreadId) == FALSE)
        return FALSE;

    return TRUE;
}
    
PROTECTED BOOL PV_Stop (DWORD TimeOut)
{
    // Arret de la COM réseau
    if (PV_ACOM_Terminate () == FALSE)
        return FALSE;

    // on envoie un message au thread lui demandant de se terminer
    if (PostThreadMessage (PV.ThreadId, WM_QUIT, 0, 0) == FALSE)
        return FALSE;

    // on attend la disparition du thread avec délai
    if (WaitForSingleObject (PV.hThread, TimeOut) != WAIT_OBJECT_0)
        return FALSE;

    // libération du handle de thread
    CloseHandle( PV.hThread );

    // libération de la configureation des PV dans la base
    if (PV_Delete_Conf () == FALSE)
        return FALSE;

    // Destruction de la liste des messages
    MSG_Delete_All_Records (&PV.hlPVMsgList);

    return TRUE;
}

PROTECTED BOOL PV_PostMessage (enum PV_WM_TYPE type, PVOID Param)
{
	return PostThreadMessage(PV.ThreadId, PV.WM_PV, (WPARAM)type, (LPARAM)Param);
}

PROTECTED DWORD PV_Count_User_Connections (CHAR profile_name[MSG_PV_MAX_USER_PROFILE_NAME])
{
    struct MSG_PV_USER_REQ_Body *p_user;
    DWORD i;
    DWORD dwCount = 0;

    // Pour tous les PV connectés
    // récupérer le user courant
    for (i = 0; i < PV_ACOM_MAX_CONNECTIONS; i++)
    {
        if (PV.aut[i] != NULL)
        {
            // comparaison des users
            p_user = PV_AUT_Get_Current_User (PV.aut[i]);

            // pas de comptage par user mais par profile
            if (strcmp (profile_name, p_user->profilename) == 0)
                dwCount ++;
        }
    }

    return dwCount;
}

PRIVATE BOOL PV_CMD_List_Users (ACOM_CNX_HANDLE hCnx)
{
    struct MSG_PV_USER_REQ_Body *p_user;
    DWORD i;

    CmdResponse( hCnx, "LISTUSERSBEGIN CONNECTION NAME (MATRICULE)" );

    // Pour tous les PV connectés
    // récupérer le user courant
    for (i = 0; i < PV_ACOM_MAX_CONNECTIONS; i++)
    {
        if (PV.aut[i] != NULL)
        {
            // comparaison des users
            p_user = PV_AUT_Get_Current_User (PV.aut[i]);

            CmdResponse (hCnx, "CON %lu USER %s (%lu)", i, p_user->name,p_user->matricule);
        }
    }

    CmdResponse( hCnx, "LISTUSERSEND" );

    return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED VOID CheckLaneLinkStatus(DWORD dwLaneNum, BOOL bLaneState)
{
	struct_link_status *pTmp;

	pTmp = (struct_link_status *)List_Get_First_Item(hLastLaneLinkStatus);

	while (pTmp != NULL)
	{
		if ((DWORD)atol(pTmp->szName) == dwLaneNum)
		{
			if (pTmp->bConnected != bLaneState)
			{
				pTmp->bConnected = bLaneState;
				GetLocalTime(&pTmp->stTime);

				AddItemTo_LinkStatusList(pTmp);

				SendMsgLinkStatus();
			}

			break;
		}

		pTmp = (struct_link_status *)List_Get_Next_Item(hLastLaneLinkStatus, pTmp);
	}
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE VOID FillMsgLinkStatus(ARG_EVT_PV_BUFFER *pMsg)
{
	CHAR				szTmp[MAX_PATH] = { 0 };
	struct_link_status	*pTmp;

	pTmp = (struct_link_status *)List_Get_First_Item(ghLinkStatus);

	sprintf_s(pMsg->buffer, sizeof(pMsg->buffer), "%s%03d", MSG_PV_CONN_STAT_REP, List_GetCount(ghLinkStatus));

	while (pTmp != NULL)
	{
		memset(szTmp, 0, sizeof(szTmp));

		sprintf_s(szTmp, 
			sizeof(szTmp), 
			"%02d%s%d%02d/%02d/%04d%02d:%02d:%02d",
			strlen(pTmp->szName),
			pTmp->szName,
			pTmp->bConnected,
			pTmp->stTime.wDay,
			pTmp->stTime.wMonth,
			pTmp->stTime.wYear,
			pTmp->stTime.wHour,
			pTmp->stTime.wMinute,
			pTmp->stTime.wSecond);

		strcat_s(pMsg->buffer, sizeof(pMsg->buffer), szTmp);

		pTmp = (struct_link_status *)List_Get_Next_Item(ghLinkStatus, pTmp);
	}

	pMsg->buffer_size = (DWORD)strlen(pMsg->buffer);
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED VOID SendMsgLinkStatus()
{
	ARG_EVT_PV_BUFFER *pMsg = NULL;

	pMsg = (ARG_EVT_PV_BUFFER *)calloc(1, sizeof(ARG_EVT_PV_BUFFER));
	

	if (pMsg != NULL)
	{
		pMsg->buffer[0] = 0;
		pMsg->buffer_size = 0;
		FillMsgLinkStatus(pMsg);
		PV_PostMessage(PV_WM_CONNECTION_STATE, pMsg);
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
PVOID New_List_Item(IN OUT HLIST *hList, IN DWORD dwSizeOfMessage)
{
	PVOID *p_header;

	if (*hList == NULL)
		*hList = List_New();

	if (List_GetCount(*hList) >= SVC_Get_Max_Nb_Stat_Items())
	{
		p_header = List_GetTail(*hList);
		if (p_header != NULL)
		{
			List_RemoveTail(*hList);
			List_ItemDelete(*hList, &(PVOID)p_header);
		}
	}

	p_header = List_ItemNew(*hList, dwSizeOfMessage);
	if (p_header == NULL)
		return NULL;

	memset(p_header, 0, dwSizeOfMessage);

	if (List_AddHead(*hList, p_header) == FALSE)
	{
		List_ItemDelete(*hList, &(PVOID)p_header);
		return NULL;
	}

	return p_header;
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
PVOID List_Get_First_Item(IN HLIST hList)
{
	PVOID *p_header;

	p_header = List_GetHead(hList);

	if (p_header == NULL)
		return NULL;

	return p_header;
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
PVOID List_Get_Next_Item(IN HLIST hList, IN PVOID hMsg)
{
	PVOID *p_header;

	p_header = List_GetNext(hList, hMsg);

	if (p_header == NULL)
		return NULL;

	return p_header;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
BOOL AddItemTo_LinkStatusList(struct_link_status *pLinkStatus)
{
	CHAR				szTmp[MAX_PATH] = { 0 };
	struct_link_status	*tmp = NULL;

	tmp = (struct_link_status *)New_List_Item(&ghLinkStatus, sizeof(struct_link_status));

	if (tmp == NULL)
		return FALSE;

	memcpy(tmp, pLinkStatus, sizeof(struct_link_status));

	sprintf_s(szTmp, 
		sizeof(szTmp),
		"Name: %10s, Link Status: %3s, DHM: %02d/%02d/%04d %02d:%02d:%02d",
		pLinkStatus->szName,
		(pLinkStatus->bConnected) ? ("OK") : ("NOK"),
		pLinkStatus->stTime.wDay,
		pLinkStatus->stTime.wMonth,
		pLinkStatus->stTime.wYear,
		pLinkStatus->stTime.wHour,
		pLinkStatus->stTime.wMinute,
		pLinkStatus->stTime.wSecond);

	NTSVCInfo(szTmp);

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED HLIST LaneLinkStatusListNew()
{
	hLastLaneLinkStatus = List_New();

	return hLastLaneLinkStatus;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED BOOL LaneLinkStatusListDelete()
{
	PVOID pHead;

	if (hLastLaneLinkStatus == NULL)
		return TRUE;

	while ((pHead = List_RemoveHead(hLastLaneLinkStatus)) != NULL)
		List_ItemDelete(hLastLaneLinkStatus, &pHead);

	if (!List_Delete(hLastLaneLinkStatus))
		return FALSE;

	return TRUE;
}


PROTECTED VOID Delete_LinkStatusList()
{
	PVOID pHead;

	if (ghLinkStatus == NULL)
		return;

	while ((pHead = List_RemoveHead(ghLinkStatus)) != NULL)
		List_ItemDelete(ghLinkStatus, &pHead);

	if (!List_Delete(ghLinkStatus))
		return;

	return;
}