/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     srvpv_main_wm.h												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef SVC_MAIN_H
#define SVC_MAIN_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csrlc32.h>
#include <reg.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <protect.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define strzcpy(dst,len,src,lg)    if ( TRUE ) { int i = (lg); int j = (len); strncpy_s( dst, j, src, i ); dst[i-1] = 0 ; } else

#define SVC_REG_VAL_COM_SERVER_NB           "NbComServer"
#define SVC_REG_VAL_COM_SERVER_NB_DEFAULT   1

#define SVC_REG_VAL_WORKERS                 "Workers"
#define SVC_REG_VAL_WORKERS_DEFAULT         3

#define SVC_REG_VAL_RECONNECTTIME           "ReconnectTime"
#define SVC_REG_VAL_RECONNECTTIME_DEFAULT   1000

#define SVC_REG_VAL_QUEUESIZE               "QueueSize"
#define SVC_REG_VAL_QUEUESIZE_DEFAULT       256

#define SVC_REG_VAL_MAXMSGSIZE              "MaxMsgSize"
#define SVC_REG_VAL_MAXMSGSIZE_DEFAULT      16384

#define SVC_REG_VAL_BUFFERSIZE              "BufferSize"
#define SVC_REG_VAL_BUFFERSIZE_DEFAULT      32000 

#define SVC_REG_VAL_MAXLIFE                 "MaxLife"
#define SVC_REG_VAL_MAXLIFE_DEFAULT         3

#define SVC_REG_VAL_LIFETIME                "LifeTime"
#define SVC_REG_VAL_LIFETIME_DEFAULT        5000


#define SVC_REG_VAL_ACOM_TRACE_ERR				"EnableACOMTraceDebug"
#define SVC_REG_VAL_ACOM_TRACE_ERR_DEFAULT     	0

#define SVC_REG_VAL_ACOM_TRACE_DBG				"EnableACOMTraceError"
#define SVC_REG_VAL_ACOM_TRACE_DBG_DEFAULT     	0


#define SVC_REG_VAL_COM_SERVER_NAME         "ComServer%dMachineName"
#define SVC_REG_VAL_COM_SERVER_NAME_DEFAULT "."

#define SVC_MSG_BUFFER_SIZE					SVC_REG_VAL_BUFFERSIZE_DEFAULT

/*-------------------------------- TYPEDEFS:  -------------------------------*/
PROTECTED char *SRVSVC_DBG_FILE;
PROTECTED int SRVSVC_DBG_LINE;

/*-------------------------------- FUNCTIONS: -------------------------------*/

PROTECTED DWORD SVC_Get_Msg_Id(void);

PROTECTED void SVC_Get_Time(LPSYSTEMTIME time);

PROTECTED void MAIN_Set_Reload(BOOL bReload);

PROTECTED BOOL MAIN_Wake_Up(void);

PROTECTED DWORD SVC_Get_DB_Link_Status_Time(void);

PROTECTED DWORD SVC_Get_Max_Nb_Stat_Items(void);

PROTECTED BOOL SVC_Is_Link_Status_Cheking_Used(void);

PROTECTED DWORD SVC_Get_Clean_Trs_Stat_Interval(void);
/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
