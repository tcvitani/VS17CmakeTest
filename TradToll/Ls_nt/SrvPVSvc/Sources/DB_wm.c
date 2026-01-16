/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     db_wm.c														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <hashcalc.h>
#include <db_connection.h>
#include <ntsvc.h>
#include <pv_wm.h>
#include <db_wm.h>
#include <conf_srvpv.h>
#include <plaza.h>
#include <srvpv_main.h>
#include <msg_pv_user_rep.h>
#include <msg_pv_alar_rep.h>
#include <msg_pv_log_req.h>
#include <msg_pv_user_req.h>
#include <msg_pv_auth_req.h>
#include <msg_pv_com_req.h>
#include <route_wm.h>
#include <alarm.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define DB_WM_EVENT_NAME					"WM_DB_EVENT"

#define DB_REG_VAL_DB_CONNECTION			"PermanentDbConnection"
#define DB_REG_VAL_DB_RECONNECTION_DELAY	"DbReconnectionDelay"
#define DB_REG_VAL_DB_ENCRYPTED_PWD			"DbEncryptedPassword"
#define DB_REG_VAL_DB_INSTANCE              "DbInstance"
#define DB_REG_VAL_DB_DBUSER				"DbUser"
#define DB_REG_VAL_DB_DBPASSWORD			"DbPassword"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

PRIVATE struct
{
    DWORD		dwPermanentDBConnection;
    DWORD		dwDBReconnectionDelay;
	BOOL		bDBEncryptedPassword;

	CHAR		szDbInst[MAX_PATH];
	CHAR		szDbUsr[MAX_PATH];
	CHAR		szDbPwd[MAX_PATH];

	HANDLE		hThread;
    DWORD		ThreadId;
    DWORD		WM_DB;

	__int64		iDbConnectTimerId;
} 
DB = {0};

/*-------------------------------- VARIABLES: -------------------------------*/

struct_link_status sDbLastLinkStatus;

/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE void CheckDBLinkStatus(BOOL bSendStatus);

/*-------------------------------- CODE:      -------------------------------*/

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
PRIVATE BOOL PV_Send_DB_HS (ACOM_CNX_HANDLE hCnxHandle)
{
    ACOM_CNX_HANDLE *pCnx;
    
    // allocation du handle de connexion du PV
    if ((pCnx = malloc (sizeof(ACOM_CNX_HANDLE))) == NULL)
        return FALSE;
    
    *pCnx = hCnxHandle;
    
    PV_PostMessage (PV_WM_DATA_BASE_FAILURE, pCnx);
    
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
PRIVATE BOOL Recv_user_login_req(struct MSG_PV_LOG_REQ *p_req)
{
    BOOL								bRet								= FALSE;
    struct	PV_WM_PARAM_USER_LOGIN_REP	*p_rep								= NULL;
	CHAR								szHash[SHA256_LEN_HEX_STRING + 1]	= { 0 };
	CHAR								szPasswordHex[2048]					= { 0 };
	DWORD								dwHashSize							= 0;
    DWORD								profile_id							= 0;
    
    if ((p_rep = malloc (sizeof(struct PV_WM_PARAM_USER_LOGIN_REP))) == NULL)
    {
        NTSVC_ERR ("Recv_user_login_req() => Error memory allocation: malloc()");
        
        return FALSE;
    }
    
	if (DB.bDBEncryptedPassword && (p_req->body.password[0] != '\0'))
	{
		ConvertBytesToHexString(p_req->body.password, (DWORD)strlen(p_req->body.password), szPasswordHex);

		dwHashSize = sizeof(szHash);
		CalculateSha256(szPasswordHex, (int)strlen(szPasswordHex), dwHashSize, szHash);
		
		if (szHash[0] == '\0')
			return FALSE;
	}
	else
	{
		sprintf_s(szHash, sizeof(szHash), "%s", p_req->body.password);
		dwHashSize = (DWORD)strlen(szHash) + 1;
	}

    bRet = CONF_PV_Get_User_Login(p_req->body.name,
								p_req->body.matricule,
								szHash,
								dwHashSize,
								p_rep->user.name,
								&p_rep->user.matricule,
								p_rep->user.firstname,
								&p_rep->user_ok,
								&p_rep->profile_max_connections,
								&profile_id,
								p_rep->user.profilename);
    
    if (!bRet || p_rep->user_ok != 1)
    {
        strzcpy(p_rep->user.name, MSG_PV_MAX_USER_NAME, p_req->body.name, sizeof(p_rep->user.name));
        p_rep->user.matricule = p_req->body.matricule;
        p_rep->user.firstname[0] = '\0';
        p_rep->user.profilename[0] = '\0';
        p_rep->profile_max_connections = 0;
        
        if (!bRet)
        {
            PV_Send_DB_HS (p_req->header.id);
            p_rep->user_ok = 3;
        }
    }
    
    p_rep->hCnxHandle = p_req->header.id;
    p_rep->req = p_req->body;
    
    PV_PostMessage (PV_WM_USER_LOGIN_REP, p_rep);
    
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
PRIVATE struct MSG_PV_USER_REP * Get_User_Conf_From_DB(CHAR name[MSG_PV_MAX_USER_NAME], DWORD matricule)
{    
    struct MSG_PV_USER_REP *p_profil;
    struct MSG_PV_USER_REP_Function *p_func;
    DWORD func_id;
    DWORD func_mode;
    struct MSG_PV_USER_REP_Command *p_cmd;
    DWORD cmd_id;
    DWORD cmd_mode;
    CHAR  cmd_label[MSG_PV_MAX_COMMAND_LABEL];
    struct MSG_PV_USER_REP_Lanetype *p_type;
    DWORD type_id;
    BOOL next;
    
    // allocation du message
    p_profil = MSG_PV_USER_REP_New ();
    if (p_profil == NULL)
        return NULL;
    
    p_profil->header.id = SVC_Get_Msg_Id ();
    
    // appel d'une procédure stockée en base
    next = CONF_PV_Get_First_User_Forbidden_Function(name, matricule, &func_id, &func_mode); 
    while (next == TRUE)
    {
        // allocation d'un element de liste 
        p_func = MSG_PV_USER_REP_Function_New (&p_profil->list_function);
        if (p_func == NULL)
        {
            // on libère tout
            MSG_PV_USER_REP_Delete_All (p_profil);
            
            NTSVC_ERR ("Get_User_Conf_From_DB() => Error memory allocation: MSG_PV_USER_REP_Function_New()!");                    
            
            return NULL;
        }
        
        p_func->id = func_id;
        p_func->mode = func_mode;
        
        // appel de la fonction suivante
        next = CONF_PV_Get_Next_User_Forbidden_Function (&func_id, &func_mode);
    }
    
    // appel d'une procédure stockée en base
    next = CONF_PV_Get_First_User_Allowed_Command(name, matricule, &cmd_id, &cmd_mode, cmd_label);
    while (next == TRUE)
    {
        // allocation d'un element de liste 
        p_cmd = MSG_PV_USER_REP_Command_New (&p_profil->list_command);
		if (p_cmd == NULL)
        {
            // on libère tout
            MSG_PV_USER_REP_Delete_All (p_profil);
            
            NTSVC_ERR ("Get_User_Conf_From_DB() => Error memory allocation: MSG_PV_USER_REP_Command_New()!");                    
            
            return NULL;
        }
        
        p_cmd->id = cmd_id;
        p_cmd->mode = cmd_mode;
        strzcpy (p_cmd->label, MSG_PV_MAX_COMMAND_LABEL, cmd_label, sizeof(p_cmd->label));
        
        // appel d'une procédure stockée en base
        next = CONF_PV_Get_First_User_Command_Lane_Type(name, matricule, cmd_id, &type_id);    
        while (next == TRUE)
        {
            // allocation d'un element de liste 
            p_type = MSG_PV_USER_REP_Lanetype_New (&p_cmd->list_lanetype);
            if (p_type == NULL)
            {
                // on libère tout
                MSG_PV_USER_REP_Delete_All (p_profil);
                
                NTSVC_ERR ("Get_User_Conf_From_DB() => Error memory allocation: MSG_PV_USER_REP_Lanetype_New()!");                    
                
                return NULL;
            }
            
            p_type->id = type_id;
            
            // appel de la fonction suivante
            next = CONF_PV_Get_Next_User_Command_Lane_Type (&type_id);
        }
        
        // appel de la commande suivante
        next = CONF_PV_Get_Next_User_Allowed_Command (&cmd_id, &cmd_mode, cmd_label);
    }
    
    return p_profil;
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
PRIVATE BOOL Recv_user_profile_req(struct MSG_PV_USER_REQ *p_req)
{    
    struct PV_WM_PARAM_USER_PROFILE_REP	*p_rep		= NULL;
    struct MSG_PV_USER_REP				*p_profile	= NULL;
    
    p_profile = Get_User_Conf_From_DB(p_req->body.name, p_req->body.matricule);   
    if (p_profile == NULL)
    {
        // informer le PV que la com. avec la base est HS
        PV_Send_DB_HS(p_req->header.id);
        
        return TRUE;
    }
    
    // allocation de la reponse
    if ((p_rep = malloc (sizeof(struct PV_WM_PARAM_USER_PROFILE_REP))) == NULL)
    {
        NTSVC_ERR ("Recv_user_profile_req() => Error memory allocation: malloc()!");
        
        return FALSE;
    }
    
    // recopie des infos complémentaires du msg source
    p_rep->hCnxHandle = p_req->header.id;
    p_rep->req = p_req->body;
    p_rep->p_profile = p_profile;
    
    // Envoi du msg
    PV_PostMessage (PV_WM_USER_PROFILE_REP, p_rep);
    
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
PRIVATE BOOL Recv_alarm_ack_req(struct MSG_PV_ALAR_REP *p_req)
{
    DWORD bRet = TRUE;
    
    // acquittement d'une liste d'alarmes auprès de la base
    bRet &= CONF_ALARM_Ack(p_req);
    
    // l'acces a la base a echoué
    if (bRet == FALSE)
    {
        // informer le PV que la com. avec la base est HS
        PV_Send_DB_HS(p_req->header.id);
        
        return TRUE;
    }
    
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
PRIVATE BOOL Recv_alarm_ack_req_test(struct MSG_PV_ACK_REQ_REP *p_req)
{
    DWORD						bRet	= TRUE;
	struct MSG_PV_ACK_REQ_REP	*p_dup	= NULL;

    // acquittement d'une liste d'alarmes auprès de la base
    bRet &= CONF_ALARM_Ack(&(p_req->alar_rep));
    
    // l'acces a la base a echoué
    if (bRet == FALSE)
    {
        // informer le PV que la com. avec la base est HS
        PV_Send_DB_HS(p_req->alar_rep.header.id);
        
        return TRUE;
    }
    else
	{
		if (p_req->ack_req.list_event == NULL)
			return TRUE;
		
		// prepare data to be sent on DB thread
		p_dup = ACK_REQ_REP_Duplicate(&p_req->ack_req, &p_req->alar_rep);
		if (p_dup == NULL)
		{
			NTSVCError("Recv_alarm_ack_req_test() => Error memory allocation: ACK_REQ_REP_Duplicate()!");
			return FALSE;
		}

		ROUTE_PostMessage(ROUTE_WM_ACK_ALARM_TEST, p_dup);
	}

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
PRIVATE BOOL Recv_cmd_req(struct MSG_PV_COM_REQ *p_req)
{
    DWORD bRet;
    
    NTSVCInfo ("Recv_cmd_req() => command: %lu", p_req->body.cd);
    
    switch (p_req->body.cd)
    {
        
    default:
        bRet = FALSE;
        break;
    };
    
    // l'acces a la base a echoué
    if (bRet == FALSE)
    {
        // informer le PV que la com. avec la base est HS
        PV_Send_DB_HS (p_req->header.id);
    }
    
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
PRIVATE BOOL Recv_user_auth_req(struct MSG_PV_AUTH_REQ *p_req)
{
    BOOL								bRet								= FALSE;
    struct PV_WM_PARAM_USER_AUTH_REP	*p_rep								= NULL;
	char								szHash[SHA256_LEN_HEX_STRING + 1]	= { 0 };
	char								szPasswordHex[2048]					= { 0 };
	DWORD								dwHashSize							= 0;
	DWORD								user_ok								= 0;
	DWORD								profile_id							= 0;

    // allocation de la réponse
    if ((p_rep = malloc (sizeof(struct PV_WM_PARAM_USER_AUTH_REP))) == NULL)
    {
        NTSVC_ERR ("Recv_user_auth_req() => Error memory allocation: malloc()!");
        
        return FALSE;
    }
    
	if (DB.bDBEncryptedPassword && (p_req->body.password[0] != '\0'))
	{
		ConvertBytesToHexString(p_req->body.password, (DWORD)strlen(p_req->body.password), szPasswordHex);

		dwHashSize = sizeof(szHash);
		CalculateSha256(szPasswordHex, (int)strlen(szPasswordHex), dwHashSize, szHash);
		
		if (szHash[0] == '\0')
			return FALSE;
	}
	else
	{
		sprintf_s(szHash, sizeof(szHash), "%s", p_req->body.password);
		dwHashSize = (DWORD)strlen(szHash) + 1;
	}

    // recherche d'autorisation sur de la base a partir des elements connus :
    // nom et/ou matricule et/ou password
    
    // commande ou fonction ?
    switch (p_req->body.type)
    {
		case  0: // fonction
			bRet = CONF_PV_Get_User_Auth_Fun(p_req->body.name,
											p_req->body.matricule,
											szHash,
											dwHashSize,
											p_req->body.id,
											p_rep->user.name,
											&p_rep->user.matricule,
											p_rep->user.firstname,
											&user_ok,
											&profile_id,
											p_rep->user.profilename);
			break;
        
		case 1: // commande
			bRet = CONF_PV_Get_User_Auth_Cmd(p_req->body.name,
											p_req->body.matricule,
											szHash,
											dwHashSize,
											p_req->body.id,
											p_rep->user.name,
											&p_rep->user.matricule,
											p_rep->user.firstname,
											&user_ok,
											&profile_id,
											p_rep->user.profilename);
			break;
        
		default:
			NTSVC_ERR ("Recv_user_auth_req() => identification type unknown!");
        
			bRet = TRUE;
			user_ok = 0;
			break;
    }
    
    // access to base failed
    if (!bRet)
    {
        // inform the PV that communication with data base is HS
        PV_Send_DB_HS (p_req->header.id);
        
        // negative response
        strzcpy (p_rep->user.reply, MSG_PV_MAX_USER_REPLY, "N", sizeof (p_rep->user.reply));
    }
    else if (user_ok == 0)
    {
		// negative response
        strzcpy (p_rep->user.reply, MSG_PV_MAX_USER_REPLY, "N", sizeof (p_rep->user.reply));
    }
	else if (user_ok == 1)
    {
		// positive response
        strzcpy (p_rep->user.reply, MSG_PV_MAX_USER_REPLY, "Y", sizeof (p_rep->user.reply));
    }
	else if (user_ok == 2)
	{
		// password expired (deactivated)
		strzcpy(p_rep->user.reply, MSG_PV_MAX_USER_REPLY, "D", sizeof(p_rep->user.reply));
	}
	else
	{
		// negative response
		strzcpy(p_rep->user.reply, MSG_PV_MAX_USER_REPLY, "N", sizeof(p_rep->user.reply));
	}

	// get additional info from the source msg
    p_rep->hCnxHandle = p_req->header.id;
    
    // send the message
    PV_PostMessage (PV_WM_USER_AUTH_REP, p_rep);
    
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
PRIVATE BOOL Recv_connection_status_req(void)
{
	if (SVC_Is_Link_Status_Cheking_Used())
		CheckDBLinkStatus(FALSE);

	SendMsgLinkStatus();

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
PRIVATE __int64 DB_StartTimer(UINT uiElapse)
{
	__int64 iTimerId = 0;

	iTimerId = SetTimer(NULL, 0, uiElapse, NULL);

	return iTimerId;
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
PRIVATE BOOL DB_StopTimer(__int64 iTimerId)
{
	return KillTimer(NULL, iTimerId);
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
PRIVATE void CheckDBLinkStatus(BOOL bSendStatus)
{
	struct_link_status	sDbLinkStatus		= {0};
	BOOL				bStatusChanged		= FALSE;
	DB_CNX				*hDbCnx				= NULL;

	GetLocalTime(&sDbLinkStatus.stTime);

	strcpy_s(sDbLinkStatus.szName, sizeof(sDbLinkStatus.szName), DB.szDbInst);

	hDbCnx = _DBConnect(DB.szDbInst, DB.szDbUsr, DB.szDbPwd);

	if (hDbCnx != NULL)
	{
		sDbLinkStatus.bConnected = TRUE;
		_DBDisconnect(hDbCnx);
		hDbCnx = NULL;
	}
	else
		sDbLinkStatus.bConnected = FALSE;

	if (sDbLinkStatus.bConnected != sDbLastLinkStatus.bConnected)
	{
		AddItemTo_LinkStatusList(&sDbLinkStatus);
		bStatusChanged = TRUE;
	}

	memcpy(&sDbLastLinkStatus, &sDbLinkStatus, sizeof(sDbLastLinkStatus));

	if (bStatusChanged && bSendStatus)
		SendMsgLinkStatus();
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
DWORD WINAPI DB_WM_Dispatch (PVOID param)
{
    BOOL	RealTimeUpdate = (BOOL) param;
    MSG		msg;
    INT		ret;
    
	if (SVC_Is_Link_Status_Cheking_Used())
		DB.iDbConnectTimerId = DB_StartTimer(SVC_Get_DB_Link_Status_Time());

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
        
        // un message a traiter pour SRV DB ?
        if (msg.message == DB.WM_DB)
        {
            switch (msg.wParam)
            {
				case DB_WM_ACK_ALARM :
					if (TestAlarmAckType())
						ret = Recv_alarm_ack_req_test((PVOID) msg.lParam);
					else
						ret = Recv_alarm_ack_req((PVOID) msg.lParam);
					break;
                
				case DB_WM_USER_LOGIN_REQ:
					// demande d'envoi d'une commande
					ret = Recv_user_login_req((PVOID) msg.lParam);
					break;
                
				case DB_WM_USER_PROFILE_REQ:
					// demande d'envoi d'une commande
					ret = Recv_user_profile_req((PVOID) msg.lParam);
					break;
                
				case DB_WM_USER_AUTH_REQ:
					// demande d'envoi d'une commande
					ret = Recv_user_auth_req((PVOID) msg.lParam);
					break;
                
				case DB_WM_CMD_REQ:
					// demande d'envoi d'une commande
					ret = Recv_cmd_req((PVOID) msg.lParam);
					break;
                
				case DB_WM_CONNECT_STAUS_REQ:
					ret = Recv_connection_status_req();
					break;

				default:
					NTSVC_ERR ("DB_WM_Dispatch() => WM_DB unknown!");
					break;
            }
            
			if (msg.wParam == DB_WM_ACK_ALARM && TestAlarmAckType())
				ACK_REQ_REP_Delete_All((PVOID)msg.lParam);
			else
				MSG_Delete_All((PVOID)&msg.lParam);
        }
        else if (msg.message == WM_QUIT)
        {
            ret = 0;
            break;
        }
		else if (msg.message == WM_TIMER)
		{
			if (msg.wParam == DB.iDbConnectTimerId)
				CheckDBLinkStatus(TRUE);
		}
        else
        {
            NTSVC_ERR ("DB_WM_Dispatch() => WM unknown!");
        }
    }
    // ret == 0 sur réception de WM_QUIT
    while (ret > 0);
    
	if (SVC_Is_Link_Status_Cheking_Used())
		DB_StopTimer(DB.iDbConnectTimerId);

    NTSVCInfo ("DB_WM_Dispatch() => thread stop!");
    
    return 0;
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
PROTECTED BOOL DB_Start (void)
{
    DWORD dwErr = 0;
    NTSVC_PARAMETER_DEF * psParams;
    DWORD dwWait;
    
    // Définition des paramètres
    psParams = NTSVCOpenParameters(
		DB_REG_VAL_DB_CONNECTION,			REG_DWORD,	4,			1,								&DB.dwPermanentDBConnection,
        DB_REG_VAL_DB_RECONNECTION_DELAY,	REG_DWORD,	4,			60000,							&DB.dwDBReconnectionDelay,
        DB_REG_VAL_DB_ENCRYPTED_PWD,		REG_DWORD,	4,			0,								&DB.bDBEncryptedPassword,
		DB_REG_VAL_DB_INSTANCE,				REG_SZ,		MAX_PATH,	"ENTER DATA BASE NAME HERE !",	&DB.szDbInst,
		DB_REG_VAL_DB_DBUSER,				REG_SZ,		MAX_PATH,	"PVUSER",						&DB.szDbUsr,
		DB_REG_VAL_DB_DBPASSWORD,			REG_SZ,		MAX_PATH,	"PVPWD",						&DB.szDbPwd,
		NULL);
    
	if (psParams == NULL)
        return FALSE;
    
    if (NTSVCLoadParameters (psParams, &dwErr) != ERROR_SUCCESS)
        return FALSE;
    
    NTSVCCloseParameters( psParams );

    // !!!!! ATTENTION BLOQUANT SI BASE HS !!!!!
    
    // on essaye périodiquement jusqu'a ce que ca marche ou
    // qu'on demande l'arret du service !
    while (DB_CONNECTION_Open () == NULL)
    {
        NTSVC_ERR( "DB_Start() => Impossible to open connection to database: blocked until the next attempt !!!" );
        
        // Attendre la demande de fin ou l'écoulement du délai de
        // polling de la base de données
        dwWait = NTSVCWaitForEnd(DB.dwDBReconnectionDelay);
        
        // Demande d'arret ? 
        if (dwWait == WAIT_OBJECT_0)
            return FALSE;
    }
    
    // si l'on ne doit pas etablir une connection permanente avec la base
    if (DB.dwPermanentDBConnection == FALSE)
        DB_CONNECTION_Close();
    
    // déclaration des WM du module lui-meme
    DB.WM_DB = RegisterWindowMessage (DB_WM_EVENT_NAME);
    if (DB.WM_DB == 0)
        return FALSE;
    
    // création du thread du module
    DB.hThread = NTSVCCreateThread (NULL, 0, DB_WM_Dispatch, NULL, 0, &DB.ThreadId, "SRVPVSVC DB Thread");
    if (DB.hThread == NULL)
        return FALSE;
    
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
PROTECTED BOOL DB_Stop (DWORD TimeOut)
{
    // on envoie un message au thread lui demandant de se terminer
    if (PostThreadMessage (DB.ThreadId, WM_QUIT, 0, 0) == FALSE)
        return FALSE;
    
    // on attend la disparition du thread avec délai
    if (WaitForSingleObject (DB.hThread, TimeOut) != WAIT_OBJECT_0)
        return FALSE;
    
    // libération du handle de thread
    CloseHandle( DB.hThread );
    
    // si l'on doit libérer une connection permanente avec la base
    if (DB.dwPermanentDBConnection == TRUE)
        DB_CONNECTION_Close ();

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
PROTECTED BOOL DB_PostMessage (enum DB_WM_TYPE type, PVOID Param)
{
	return PostThreadMessage(DB.ThreadId, DB.WM_DB, (WPARAM)type, (LPARAM)Param);
}

/*-------------------------------- END OF FILE ------------------------------*/