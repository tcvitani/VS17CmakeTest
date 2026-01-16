/******************* (v) 2003 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKGEAFILE                                                   */
/* FILE:     cmworkgeafile.h                                                 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The extension module of the ComMOCSvc service. This module    */
/*             converts the GEA PAQ file to CSSI files.                      */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMWORKGEAFILE_H
#define CMWORKGEAFILE_H
/*-------------------------------- INCLUDES:  -------------------------------*/
/*-------------------------------- RESERVED:  -------------------------------*/
#ifdef CMWORKGEAFILE_EXPORTS
#    include <public.h>
#else
#    include <export.h>
#endif
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
#define CMWORK_MAX_STR			512
#define CMWORK_MAX_PARAM_STR	50
#define CMWORK_MAX_FILE_NAME	MAX_PATH

#define ERROR_BAD_FILE			1001
#define ERROR_MAX_LANES			1002
#define ERROR_MAX_TRS_ITEMS		1003
#define ERROR_NO_DB_CONNECTION	1004
#define ERROR_CONVERTING_MSG	1005

#define MAX_GEA_LINE_LENGTH		11000
#define PCS_MAX_MSG_LENGTH		4096
#define MSG_PREFIX_LENGTH		4
/*-------------------------------- TYPEDEFS:  -------------------------------*/
typedef struct CMW_PARAMS
{
	char  szOutDir[CMWORK_MAX_PARAM_STR];
	char  szTempDir[CMWORK_MAX_PARAM_STR];
	char  szCtxDir[CMWORK_MAX_PARAM_STR];

	char szDbUsr[MAX_PATH];
	char szDbPwd[MAX_PATH];
	char szDbInst[MAX_PATH];

	char szWrkName[MAX_PATH];
	DWORD dwEnsureOrder;
	DWORD dwStoreInBakFiles;

	char szLaneList[MAX_PATH];
	DWORD dwNetMult;

	TRC_EMETTEUR hTrc;
	TRC_EMETTEUR hErr;
}CMW_PARAMS;
/*-------------------------------- FUNCTIONS: -------------------------------*/

/*****************************************************************************/
/*SYNTAX: EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig )  */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service at service  */
/*            startup. The function initializes the module. It reads the     */
/*            module parameters from the registry and initializes the trace  */
/*            file.                                                          */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) PVOID param  - Pointer to the instance ID.                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  HANDLE                If the initialization is successful the function   */
/*                        returns the module handle (pointer to module       */
/*                        parameters).                                       */
/*  NULL                  The function returns NULL if the initialization is */
/*                        not successful.                                    */
/*****************************************************************************/
EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig );

/*****************************************************************************/
/*SYNTAX: EXPORT void WINAPI WorkerClose( HANDLE hWrk )                      */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service at the      */
/*            service exit.                                                  */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HANDLE hWrk  - Pointer to the module handle.                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
EXPORT void WINAPI WorkerClose( HANDLE hWrk );

/*****************************************************************************/
/*SYNTAX: EXPORT DWORD WINAPI WorkerProcessFileEx( HANDLE hWrk,              */
/*                                                 char * szFilePath,        */
/*                                                 void * pvReserved )       */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service when it     */
/*            detects the file in the appropriate directory. The function    */
/*            converts the GEA PAQ file to CSSI message files.               */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HANDLE hWrk       - Worker handle (pointer to module           */
/*                                parameters).                               */
/*      (IN ) char * szFilePath - File name (full path)                      */
/*      (IN ) void * pvReserved - Reserved                                   */
/*===========================================================================*/
/*  Return                   Description                                     */
/*---------------------------------------------------------------------------*/
/* Result of the processing:                                                 */
/*  CM_WORKER_ACK_NONE       (0x00000000) : processing canceled/file ignored */
/*  CM_WORKER_ACK_OK         (0x00000001) : processing done successfully     */
/*  CM_WORKER_ACK_RETRY_ERR  (0x00000002) : error during processing,         */
/*                                          retry later                      */
/*  CM_WORKER_ACK_RETRY_BUSY (0x00000003) : processing busy, retry later     */
/*  CM_WORKER_ACK_ABANDON    (0x00000004) : error during processing, abandon */
/*                                          file                             */
/* Combined with the service action after processing:                        */
/*  CM_WORKER_ACT_NONE       (0x00000000) : nothing to do                    */
/*  CM_WORKER_ACT_MOVE_ACK   (0x00000100) : move file to acknowledgement     */
/*                                          directory                        */
/*  CM_WORKER_ACT_COPY_ERR   (0x00000200) : copy file to the error           */
/*                                          directory and move it to         */
/*                                          the acknowledgement directory    */
/*  CM_WORKER_ACT_MOVE_ERR   (0x00000300) : move the file to the             */
/*                                          error directory                  */
/*  CM_WORKER_ACT_DELETE     (0x00000400) : simply delete the file           */
/*****************************************************************************/
EXPORT DWORD WINAPI WorkerProcessFileEx( HANDLE hWrk,
										 char * szFilePath,
										 void * pvReserved );

/*-------------------------------- VARIABLES: -------------------------------*/
#endif
/*-------------------------------- END OF FILE ------------------------------*/