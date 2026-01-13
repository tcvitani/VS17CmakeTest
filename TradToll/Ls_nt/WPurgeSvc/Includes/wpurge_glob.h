#ifndef WPURGE_GLOB_H
#define WPURGE_GLOB_H

#include <protect.h>

#include "..\resources\version.h"


// ----------------- MACROS ----------------------

// Nom du service
#define WPURGE_SERVICE_NAME       RESINFO_PRODUCT

// Prompt du service
#define WPURGE_PROMPT_TITLE       RESINFO_PRODUCT " - " RESINFO_COPYRIGHT
#define WPURGE_PROMPT_VERSION     "Version " RESINFO_VERSION_STRING
#define WPURGE_PROMPT_DATE        "Compiled " __DATE__ " - " __TIME__
#define WPURGE_PROMPT_COMMENTS    RESINFO_FILEDESC

// Timeout toléré pour l'arret d'un working set
#define WPURGE_WORKINGSET_TIMEOUT 5000
#define CSR_REG_KEYi_ROOT			HKEY_LOCAL_MACHINE

// Définition des codes de types des clients correspondant
#define WPURGE_PIPE_CMD_TYPE       0
#define WPURGE_PIPE_COM_TYPE       1
#define WPURGE_PIPE_SERVER_TYPE    2
#define WPURGE_PIPE_CLIENT_TYPE    3


#define WPURGE_REG_ROOT            HKEY_LOCAL_MACHINE
#define WPURGE_REG_KEY             NTSVC_REG_KEY_SERVICE "\\" WPURGE_SERVICE_NAME "\\" NTSVC_REG_KEY_PARAM

#define WPURGE_REG_KEYn_Target_CONFIG	"ConfigTarget\\"
#define WPURGE_REG_KEYn_SourceDir	"SourceDir"
#define WPURGE_REG_KEYn_BackupDir	"BackupDir"
#define WPURGE_REG_KEYn_DoBackup	"DoBackup"
#define WPURGE_REG_KEYn_RetentionPeriodHours	"RetentionPeriodHours"
#define WPURGE_REG_KEYn_ExtFilter	"ExtFilter"
#define WPURGE_REG_KEYn_MaxFilesEnumStep	"MaxFilesEnumStep"


#define WPURGE_REG_VAL_MAINPOLLING         "MainPolling"
#define WPURGE_REG_VAL_NumWorkers         "NumWorkers"

#define WPURGE_MSG_BUFFER_SIZE     4024

#define MAX_TARGETS	20
#define MAX_EXTENSION_LEN	6
#define MAX_WORKERS 500
#define MAX_TODO_TASKS 10000


#define SVC_ERR(err,txt) \
    if ( FALSE );\
    else { \
        char * pcTxtErr = txt;\
        NTSVC_ERR2( "#ERR%u# : %s", (err), pcTxtErr );\
        Sleep( 500 );\
    }


// ----------------- TYPES ----------------------


typedef struct
{
	CHAR szSourceDir[MAX_PATH];
	CHAR szBackupDir[MAX_PATH];
	CHAR szExtFilter[MAX_EXTENSION_LEN];

	BOOL bDoBackup;
	DWORD dwRetentionPeriodHours;
	DWORD dwMaxFilesEnumStep;

}struct_config_target;

typedef struct WPURGE_PARAMS
{
	DWORD                       dwMainPolling;
	DWORD						dwNumWorkers;

	DWORD						dwNbDefinedTargets;
	struct_config_target		asConfigTarget[MAX_TARGETS];

}
WPURGE_PARAMS;

typedef struct
{
	CHAR m_szFileName[MAX_PATH];
	CHAR m_szRelativePath[MAX_PATH];
	CHAR m_szCurrentRootPath[MAX_PATH];
	CHAR m_szTargetRootPath[MAX_PATH];

	BOOL bDoBackup;
	BOOL bIsFolder;
}struct_task_data;

typedef struct WPURGE_WORK
{
    BOOL bIsDebug;
    HANDLE hEvent;

    WPURGE_PARAMS sParmWork;

	DWORD adwThreadId[MAX_WORKERS];
	HANDLE ahThread[MAX_WORKERS];


	CRITICAL_SECTION csForTasks;
	int iFirstTask;
	int iLastTask;
	struct_task_data asToDoTasks[MAX_TODO_TASKS]; //circular buffer
}
WPURGE_WORK;




// ----------------- VARIABLES GLOBALES ----------------------


PROTECTED WPURGE_WORK   gsSvcWork INIT(0);

PROTECTED char gszSvcName[MAX_PATH]
#ifdef LOC_DEF
 = WPURGE_SERVICE_NAME
#endif
;




#endif
