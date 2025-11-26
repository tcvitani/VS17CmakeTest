/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : TAG
 * FILE       : TAG.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>

#include <reg.h>
#include <trc.h>

#include "csr_tag_isa.h"
#include "csr_tag_qfl2.h"
#include "csr_tag_bac.h"
#include "csr_tag_iso.h"
#include "csr_tag_trsp.h"
#include "csr_tag_qfsl7.h"
#include "csr_tag_cenl7.h"
#include "csr_tag_ipgea.h"
#include "csr_tag_qfdmi.h"
#include "csr_tag.h"

#include <memclass.h>


typedef DWORD  WINAPI TAGL2_OPEN( OUT void ** ppvInst, IN void * pvParams );
typedef void   WINAPI TAGL2_CLOSE( IN void * pvInst, IN DWORD dwTimeout );
typedef DWORD  WINAPI TAGL2_SEND( IN void * pvInst, IN BYTE * pbMsg, IN DWORD dwMsgBytes, OPTIONAL IN HANDLE hEvent, OPTIONAL OUT DWORD * pdwErr );
typedef DWORD  WINAPI TAGL2_RECV( IN  void * pvInst, OUT BYTE ** ppbMsg, OUT DWORD * pdwMsgBytes, IN DWORD dwTimeout );
typedef void   WINAPI TAGL2_FREE( IN BYTE * pbMsg );
typedef HANDLE WINAPI TAGL2_WAIT( IN void * pvInst );
typedef void * WINAPI TAGL2_COMM( IN void * pvInst );
typedef DWORD  WINAPI TAGL2_SETOPT( IN void * pvInst, IN DWORD dwOption, IN DWORD dwValue, IN void * pvValue );
typedef DWORD  WINAPI TAGL2_GETOPT( IN void * pvInst, IN DWORD dwOption, IN DWORD * pdwValue, IN void * pvValue );



//
// Définition de l'objet tag
//
typedef struct _TAG_INSTANCE
{
    // Type d'interface avec le badge
    TAG_INTERFACE_TYPES eInterface;

    // Paramétrage générique de la couche L2
    union
    {
        // Paramétrage spécifique couche L2 ISA (filaire directe)
        ISA_PARAMS      sIsa;

        // Paramétrage spécifique couche L2 BAC (série indirect par communicateur)
        BAC_PARAMS      sBac;

        // Paramétrage spécifique couche L2 BAC (série indirect par communicateur)
        BAC_PARAMS_EX   sBacEx;

        // Paramétrage spécifique couche L2 ISO (série indirect par communicateur)
        ISO_PARAMS      sIso;
		
        // Paramétrage spécifique couche L2 QFL2 
        QFREE_PARAMS    sQFree;

        // Paramétrage spécifique couche L2 IPGEA
        IPGEA_PARAMS    sIPGEA;

		// Paramétrage spécifique couche L2 IPGEA
		QFR_DMI_PARAMS	sQFreeDmi;


        // Paramétrage brute
        BYTE            tbParams[1];
    }
        uParams;

    DWORD               dwConsoleTrace;
	DWORD				dwConsoleSaveToFile;
	char				szConsoleFilePath[1024];

    // Handle de la couche 1 (physique)
    void              * hL1;

    // Handle de la couche 2 (liaison)
    void              * hL2;

    // Handle de la sous-couche 7 (application niveau intermédiaire)
    void              * hSL7;

    // Handle de la couche 7 (application niveau supérieur)
    void              * hL7;

    // Contrôle de l'API de la sous-couche 7
    TRSP_API            sSL7Api;

	// Structures to be used in QFree beacon protocol messages
	// build / analyse procedure 
	QFSL7_API			sQFSL7Api;

    // Contrôle de l'API de la couche 7
    CENL7_API           sL7Api;

	//Structure to handle GEAIP L7 communication since the BST is to be activated/deactivated
	IPGEAL7_API			sGEAIPL7Api;

	//Structure to handle QFREE L7 communication since the BST is to be activated/deactivated
	QFDMI_INTERNAL_MSG		sQFDMIL7Api;

    // Pointeur de fonctions générique pour la couche 2 : Open
    TAGL2_OPEN        * pfL2Open;

    // Pointeur de fonctions générique pour la couche 2 : Close
    TAGL2_CLOSE       * pfL2Close;

    // Pointeur de fonctions générique pour la couche 2 : SendMessage
    TAGL2_SEND        * pfL2Send;

    // Pointeur de fonctions générique pour la couche 2 : ReceiveMessage
    TAGL2_RECV        * pfL2Recv;

    // Pointeur de fonctions générique pour la couche 2 : FreeMessage
    TAGL2_FREE        * pfL2Free;

    // Pointeur de fonctions générique pour la couche 2 : GetWaitableHandle
    TAGL2_WAIT        * pfL2Wait;

    // Pointeur de fonctions générique pour la couche 2 : GetCommInstance
    TAGL2_COMM        * pfL2Comm;

    // Pointeur de fonctions générique pour la couche 2 : SetOption
    TAGL2_SETOPT      * pfL2SetOpt;

    // Pointeur de fonctions générique pour la couche 2 : GetOption
    TAGL2_GETOPT      * pfL2GetOpt;

    // Nombre d'octets utilisés dans le buffer d'émission des requètes applicatives
    DWORD               dwOutBytes;

    // Buffer d'émission des requètes applicatives
    BYTE                tbOut[256];

    // Nombre d'octets utilisés dans le buffer d'émission des requètes d'interface
    DWORD               dwOutSvcBytes;

    // Buffer d'émission des requètes d'interface
    BYTE                tbOutSvc[256];
}
    TAG_INSTANCE;



typedef struct _DEFAULT_HOOK_CONTEXT
{
    LONGLONG        llFreq;
    HWND            hWnd;
    HANDLE          hFile;
	BOOL			bIsFile;
    DWORD           dwThreadId;
    TAG_HOOK_LOG  * psLog;
	TAG_INSTANCE          * psInst;
}
    DEFAULT_HOOK_CONTEXT;



typedef struct _TAG_HOOK_LOG_ITEM
{
    struct _TAG_HOOK_LOG_ITEM * psNext;
    struct _TAG_HOOK_LOG_ITEM * psPrev;
    DWORD                       dwBytes;
    BYTE                        tbData[0];
}
    TAG_HOOK_LOG_ITEM;


    
typedef struct _TAG_HOOK_LOG
{
    CRITICAL_SECTION    sProtect;
    BOOL                fSuspended;
    TAG_HOOK_LOG_ITEM * psFirst;
}
    TAG_HOOK_LOG;

#define TAG_HOOK_MAX_BYTES_SIZE 4096

    
PRIVATE void WINAPI TagDefaultHook( IN void * pvHookContext, 
                                    IN DWORD  dwTypeDir, 
                                    IN BYTE * pbData, 
                                    IN DWORD dwBytes );

PRIVATE BOOL WINAPI TagHookLogPushData( IN TAG_HOOK_LOG * psLog,
                                        IN DWORD          dwBytes,
                                        IN BYTE         * pbData );


PUBLIC BOOL WINAPI TagIsVersionCompatible(
                IN   DWORD  dwMajor, 
                IN   DWORD  dwMinor, 
                IN   DWORD  dwBuild )
{
#pragma message( "Ne pas oublier de mettre en conformité la fonction de validation de version" )
#define TAG_VERSION_MAJOR 10
#define TAG_VERSION_MINOR 3
#define TAG_VERSION_BUILD 0
    if ( dwMajor == TAG_VERSION_MAJOR )
        if ( dwMinor < TAG_VERSION_MINOR )
            return TRUE;
        else if ( dwMinor == TAG_VERSION_MINOR )
            return ( dwBuild <= TAG_VERSION_BUILD );
        else
            return FALSE;
    else
        return FALSE;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk )
 * PARAMETRES: cf Win32 API
 * RETOUR    : cf Win32 API
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Initialiser la DLL à son chargement, et terminer proprement à son
 *             déchargement.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk )
{
	BOOL bOK = TRUE;

	switch ( dwWhy )
	{
	// Lorsque le process fait le premier LoadLibrary consernant cette DLL
	case DLL_PROCESS_ATTACH :
			IPGEAInitGlobalInstanceHandling();
		break;

	// Lorsque le process fait le dernier FreeLibrary consernant cett DLL
	case DLL_PROCESS_DETACH :
			IPGEADeinitGlobalInstanceHandling();
		break;
	}

	return bOK;
}






PUBLIC DWORD WINAPI TagOpen(
                IN   TAG_INTERFACE_TYPES     eInterface,
                IN   char                  * pcRegPath,
                IN   TAG_HOOK              * pfHook,
                IN   TAG_HOOK_CONTEXT        pvHookContext,
                OUT  TAG_INSTANCE         ** ppsInst )
{
    DWORD           dwErr = ERROR_NOT_ENOUGH_MEMORY;
    DWORD           dwLen;
    DWORD           dwFlags;
    HKEY            hKeyRoot = HKEY_LOCAL_MACHINE;
    TAG_INTERFACE_TYPES * peType;
    TAG_INSTANCE  * psInst = NULL;

#   define GET_DWORD(name,var,def) \
        dwErr = REG_Defaut_Entier( hKeyRoot, pcRegPath, name, &dwLen, def ); \
        if ( dwErr != NO_ERROR ) { dwErr = ERROR_INVALID_PARAMETER; __leave; } \
        (var) = dwLen;

#   define GET_STRING(name,var,def) \
        dwLen = sizeof(var); \
        dwErr = REG_Defaut_Chaine( hKeyRoot, pcRegPath, name, var, &dwLen, def ); \
        if ( dwErr != NO_ERROR ) { dwErr = ERROR_INVALID_PARAMETER; __leave; }

    __try
    {
        *ppsInst = NULL;

        //
        // Allouer la structure d'instance
        //
        psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psInst) );
        if ( psInst == NULL )
            __leave;

        dwFlags = ( eInterface & (~TAG_INTERFACE_MASK) );
        eInterface = ( eInterface & TAG_INTERFACE_MASK );

        //
        // Modification des conditions d'accés au registre en fonction
        // de ce par quoi commence la chaine contenant le chemin.
        //
        if ( ( dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS ) == 0 )
        {
            if ( _strnicmp( pcRegPath, "HKLM:", 5 ) == 0 )
            {
                hKeyRoot = HKEY_LOCAL_MACHINE;
                pcRegPath += 5;
            }
            else if ( _strnicmp( pcRegPath, "HKCU:", 5 ) == 0 )
            {
                hKeyRoot = HKEY_CURRENT_USER;
                pcRegPath += 5;
            }
            else if ( _strnicmp( pcRegPath, "HKU:", 4 ) == 0 )
            {
                pcRegPath += 4;
                hKeyRoot = HKEY_USERS;
            }
        }

        if ( ( dwFlags & TAG_INTERFACE_FLAG_CONSOLE_TRACE ) == 0 )
        {
            if ( ( dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS ) == 0 )
            {
                GET_DWORD ( "ConsoleTrace" , psInst->dwConsoleTrace, 0  );
				GET_DWORD ( "ConsoleSaveToFile" , psInst->dwConsoleSaveToFile, 0  );
				GET_STRING( "ConsoleFilePath", psInst->szConsoleFilePath, "");
            }
        }
        else
        {
			psInst->dwConsoleTrace = 1;
		}

	//-------------------------------------------------------------------------------

        if ( eInterface == TAG_INTERFACE_WIRED )
        {
            //
            // Il s'agit d'une liaison filaire directe avec le badge (protocole
            // ISA), on prend donc la structure de configuration de la couche 2
            // pour ce type de liaison.
            //

            if ( ( dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS ) == 0 )
            {
                //
                // Récupérer les paramètres du protocole ISA dans le registre à
                // l'endroit indiqué en paramètre
                //
                GET_DWORD ( "Timer"           , psInst->uParams.sIsa.dwTimer           , 10 )
                GET_DWORD ( "MaxPendingMsg"   , psInst->uParams.sIsa.dwMaxPendingMsg   , 8  )
                GET_DWORD ( "Priority"        , psInst->uParams.sIsa.dwPriority        , THREAD_PRIORITY_HIGHEST )
                GET_DWORD ( "Port"            , psInst->uParams.sIsa.dwPort            , 1  )
                GET_DWORD ( "AutoClearBuffers", psInst->uParams.sIsa.dwAutoClearBuffers, 1  )
                GET_STRING( "PortSettings"    , psInst->uParams.sIsa.szPortSettings    , "baud=19200 data=8 stop=1 parity=e to=off xon=off dtr=off rts=off odsr=off octs=off idsr=off" )
            }
            else
            {
                peType = (TAG_INTERFACE_TYPES*)pcRegPath;
                if ( *peType != TAG_INTERFACE_WIRED )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }

                psInst->uParams.sIsa = *(ISA_PARAMS*)( peType + 1 );
            }

            psInst->uParams.sIsa.dwStructBytes = sizeof(psInst->uParams.sIsa);

            //
            // Mettre en place l'espion
            //
            if ( psInst->dwConsoleTrace == 0 )
            {
                psInst->uParams.sIsa.pfHook = pfHook;
                psInst->uParams.sIsa.pvHookContext = pvHookContext;
            }
            else
            {
                psInst->uParams.sIsa.pfHook = TagHookGetDefaultHook();
                psInst->uParams.sIsa.pvHookContext = TagHookOpenDefaultContext(psInst,NULL,0, NULL );
            }

            //
            // Faire pointer les pointeurs de fonctions génériques de la couche
            // 2 sur les fonctions du protocole ISA.
            //
            psInst->pfL2Open    = (TAGL2_OPEN  *)&ISAOpen;
            psInst->pfL2Close   = (TAGL2_CLOSE *)&ISAClose;
            psInst->pfL2Send    = (TAGL2_SEND  *)&ISASendMessage;
            psInst->pfL2Recv    = (TAGL2_RECV  *)&ISAReceiveMessage;
            psInst->pfL2Free    = (TAGL2_FREE  *)&ISAFreeMessage;
            psInst->pfL2Wait    = (TAGL2_WAIT  *)&ISAGetWaitableHandle;
            psInst->pfL2Comm    = (TAGL2_COMM  *)&ISAGetCommInstance;
            psInst->pfL2SetOpt  = (TAGL2_SETOPT*)&ISASetOption;
            psInst->pfL2GetOpt  = (TAGL2_GETOPT*)&ISAGetOption;

            //
            // Ce type de liaison directe n'utilise aucune sous-couche applicative.
            //
            psInst->hSL7 = TAG_NO_SUBL7;
        }   
 	//-------------------------------------------------------------------------------
       else if ( eInterface == TAG_INTERFACE_COMMUNICATOR )
        {
            //
            // Il s'agit d'une liaison série avec un communicateur (protocole
            // BAC), on prend donc la structure de configuration de la couche 2
            // pour ce type de liaison.
            //

            if ( ( dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS ) == 0 )
            {
                //
                // Récupérer les paramètres du protocole BAC dans le registre à
                // l'endroit indiqué en paramètre
                //
                GET_DWORD ( "MaxEnqRetry"     , psInst->uParams.sBac.dwMaxEnqRetry     , 16 )
                GET_DWORD ( "MaxMsgRetry"     , psInst->uParams.sBac.dwMaxMsgRetry     , 16 )
                GET_DWORD ( "Timer1"          , psInst->uParams.sBac.dwTimer1          , 44 )
                GET_DWORD ( "Timer2"          , psInst->uParams.sBac.dwTimer2          , 16 )
                GET_DWORD ( "MaxPendingMsg"   , psInst->uParams.sBac.dwMaxPendingMsg   , 8  )
                GET_DWORD ( "Priority"        , psInst->uParams.sBac.dwPriority        , THREAD_PRIORITY_HIGHEST )
                GET_DWORD ( "Port"            , psInst->uParams.sBac.dwPort            , 1  )
                GET_DWORD ( "AutoClearBuffers", psInst->uParams.sBac.dwAutoClearBuffers, 1  )
                GET_STRING( "PortSettings"    , psInst->uParams.sBac.szPortSettings    , "baud=115200 data=8 stop=2 parity=e to=off xon=off dtr=on rts=on odsr=off octs=off idsr=off" )
            }
            else
            {
                peType = (TAG_INTERFACE_TYPES*)pcRegPath;
                if ( *peType != TAG_INTERFACE_COMMUNICATOR )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }

                psInst->uParams.sBac = *(BAC_PARAMS*)( peType + 1 );
            }


            if ( ( dwFlags & TAG_INTERFACE_FLAG_COLLISION_CANCEL ) != 0 )
                psInst->uParams.sBacEx.dwFlags |= BAC_CONTENSION_CANCEL;

            psInst->uParams.sBacEx.dwStructBytes = sizeof(psInst->uParams.sBacEx);

            //
            // Mettre en place l'espion
            //
            if ( psInst->dwConsoleTrace == 0 )
            {
                psInst->uParams.sBac.pfHook = pfHook;
                psInst->uParams.sBac.pvHookContext = pvHookContext;
            }
            else
            {
                psInst->uParams.sBac.pfHook = TagHookGetDefaultHook();
                psInst->uParams.sBac.pvHookContext = TagHookOpenDefaultContext(psInst,NULL,0, NULL );
            }

            //
            // Faire pointer les pointeurs de fonctions génériques de la couche
            // 2 sur les fonctions du protocole BAC.
            //
            psInst->pfL2Open    = (TAGL2_OPEN  *)&BACOpen;
            psInst->pfL2Close   = (TAGL2_CLOSE *)&BACClose;
            psInst->pfL2Send    = (TAGL2_SEND  *)&BACSendMessage;
            psInst->pfL2Recv    = (TAGL2_RECV  *)&BACReceiveMessage;
            psInst->pfL2Free    = (TAGL2_FREE  *)&BACFreeMessage;
            psInst->pfL2Wait    = (TAGL2_WAIT  *)&BACGetWaitableHandle;
            psInst->pfL2Comm    = (TAGL2_COMM  *)&BACGetCommInstance;
            psInst->pfL2SetOpt  = (TAGL2_SETOPT*)&BACSetOption;
            psInst->pfL2GetOpt  = (TAGL2_GETOPT*)&BACGetOption;

            //
            // Ce type de liaison indirecte utilise une sous-couche applicative.
            // Il s'agit de la couche implémentant le mode transparent.
            //
            psInst->hSL7 = TRSPOpen();
            if ( psInst->hSL7 == NULL )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
        }
  	//-------------------------------------------------------------------------------
		else if ( eInterface == TAG_INTERFACE_COMMUNICATOR_FAST )
        {
            //
            // Il s'agit d'une liaison série avec un communicateur (protocole
            // BAC), on prend donc la structure de configuration de la couche 2
            // pour ce type de liaison.
            //

            if ( ( dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS ) == 0 )
            {
                //
                // Récupérer les paramètres du protocole ISA dans le registre à
                // l'endroit indiqué en paramètre
                //
                GET_DWORD ( "Timer"           , psInst->uParams.sIsa.dwTimer           , 10 )
                GET_DWORD ( "MaxPendingMsg"   , psInst->uParams.sIsa.dwMaxPendingMsg   , 8  )
                GET_DWORD ( "Priority"        , psInst->uParams.sIsa.dwPriority        , THREAD_PRIORITY_HIGHEST )
                GET_DWORD ( "Port"            , psInst->uParams.sIsa.dwPort            , 1  )
                GET_DWORD ( "AutoClearBuffers", psInst->uParams.sIsa.dwAutoClearBuffers, 1  )
                GET_STRING( "PortSettings"    , psInst->uParams.sIsa.szPortSettings    , "baud=115200 data=8 stop=2 parity=e to=off xon=off dtr=on rts=on odsr=off octs=off idsr=off" )
            }
            else
            {
                peType = (TAG_INTERFACE_TYPES*)pcRegPath;
                if ( *peType != TAG_INTERFACE_COMMUNICATOR_FAST )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }

                psInst->uParams.sIsa = *(ISA_PARAMS*)( peType + 1 );
            }

            psInst->uParams.sIsa.dwStructBytes = sizeof(psInst->uParams.sIsa);

            //
            // Mettre en place l'espion
            //
            if ( psInst->dwConsoleTrace == 0 )
            {
                psInst->uParams.sIsa.pfHook = pfHook;
                psInst->uParams.sIsa.pvHookContext = pvHookContext;
            }
            else
            {
                psInst->uParams.sIsa.pfHook = TagHookGetDefaultHook();
                psInst->uParams.sIsa.pvHookContext = TagHookOpenDefaultContext(psInst,NULL,0, NULL );
            }

            //
            // Faire pointer les pointeurs de fonctions génériques de la couche
            // 2 sur les fonctions du protocole BAC.
            //
            psInst->pfL2Open    = (TAGL2_OPEN  *)&ISAOpen;
            psInst->pfL2Close   = (TAGL2_CLOSE *)&ISAClose;
            psInst->pfL2Send    = (TAGL2_SEND  *)&ISASendMessage;
            psInst->pfL2Recv    = (TAGL2_RECV  *)&ISAReceiveMessage;
            psInst->pfL2Free    = (TAGL2_FREE  *)&ISAFreeMessage;
            psInst->pfL2Wait    = (TAGL2_WAIT  *)&ISAGetWaitableHandle;
            psInst->pfL2Comm    = (TAGL2_COMM  *)&ISAGetCommInstance;
            psInst->pfL2SetOpt  = (TAGL2_SETOPT*)&ISASetOption;
            psInst->pfL2GetOpt  = (TAGL2_GETOPT*)&ISAGetOption;

            //
            // Ce type de liaison indirecte utilise une sous-couche applicative.
            // Il s'agit de la couche implémentant le mode transparent.
            //
            psInst->hSL7 = TRSPOpen();
            if ( psInst->hSL7 == NULL )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
        }
   	//-------------------------------------------------------------------------------
		else if ( eInterface == TAG_INTERFACE_ISO7816 )
        {
            //
            // Il s'agit d'une liaison série avec une balise en interface
            // ISO7816 (type carte à puce) du type de la liaison avec la
            // balise de table TG
            //

            if ( ( dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS ) == 0 )
            {
                //
                // Récupérer les paramètres du protocole ISA dans le registre à
                // l'endroit indiqué en paramètre
                //
                GET_DWORD ( "MaxPendingMsg"   , psInst->uParams.sIso.dwMaxPendingMsg   , 8  )
                GET_DWORD ( "Priority"        , psInst->uParams.sIso.dwPriority        , THREAD_PRIORITY_HIGHEST )
                GET_DWORD ( "Port"            , psInst->uParams.sIso.dwPort            , 1  )
                GET_DWORD ( "Debit"           , psInst->uParams.sIso.dwDebit           , 115200 )
                GET_DWORD ( "AutoClearBuffers", psInst->uParams.sIso.dwAutoClearBuffers, 1  )
                GET_STRING( "DownloadFile"    , psInst->uParams.sIso.szDownloadFile    , "download.bin" )
                GET_STRING( "TransportDll"    , psInst->uParams.sIso.szTransportDll    , "dll_l7.dll" )
                GET_STRING( "LowLevelDll"     , psInst->uParams.sIso.szLowLevelDll     , "dll_i2.dll" )
            }
            else
            {
                peType = (TAG_INTERFACE_TYPES*)pcRegPath;
                if ( *peType != TAG_INTERFACE_ISO7816 )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }

                psInst->uParams.sIso = *(ISO_PARAMS*)( peType + 1 );
            }

            psInst->uParams.sIso.dwStructBytes = sizeof(psInst->uParams.sIso);

            //
            // Mettre en place l'espion
            //
            if ( psInst->dwConsoleTrace == 0 )
            {
                psInst->uParams.sIso.pfHook = pfHook;
                psInst->uParams.sIso.pvHookContext = pvHookContext;
            }
            else
            {
                psInst->uParams.sIso.pfHook = TagHookGetDefaultHook();
                psInst->uParams.sIso.pvHookContext = TagHookOpenDefaultContext(psInst,NULL,0, NULL );
            }

            //
            // Faire pointer les pointeurs de fonctions génériques de la couche
            // 2 sur les fonctions du protocole BAC.
            //
            psInst->pfL2Open    = (TAGL2_OPEN  *)&ISOOpen;
            psInst->pfL2Close   = (TAGL2_CLOSE *)&ISOClose;
            psInst->pfL2Send    = (TAGL2_SEND  *)&ISOSendMessage;
            psInst->pfL2Recv    = (TAGL2_RECV  *)&ISOReceiveMessage;
            psInst->pfL2Free    = (TAGL2_FREE  *)&ISOFreeMessage;
            psInst->pfL2Wait    = (TAGL2_WAIT  *)&ISOGetWaitableHandle;
            psInst->pfL2Comm    = (TAGL2_COMM  *)&ISOGetCommInstance;
            psInst->pfL2SetOpt  = (TAGL2_SETOPT*)&ISOSetOption;
            psInst->pfL2GetOpt  = (TAGL2_GETOPT*)&ISOGetOption;
        }
	//-------------------------------------------------------------------------------
		else if ( eInterface == TAG_INTERFACE_QFREE )
        {
            //
            // QFL2 MD5826 beacon 
            //
			
            if ( ( dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS ) == 0 )
            {
                //
                // Récupérer les paramètres du protocole ISA dans le registre à
                // l'endroit indiqué en paramètre
                //
                GET_DWORD ( "MaxPendingMsg"   , psInst->uParams.sQFree.dwMaxPendingMsg   , 8  )
				GET_DWORD ( "Priority"        , psInst->uParams.sQFree.dwPriority        , THREAD_PRIORITY_HIGHEST )
				GET_DWORD ( "AutoClearBuffers", psInst->uParams.sQFree.dwAutoClearBuffers, 1  )
				GET_STRING( "PortSettings"    , psInst->uParams.sQFree.szPortSettings    , "COM1 57600 e 2" )
				GET_STRING( "L7DataPipeName"    , psInst->uParams.sQFree.szL7DataPipeName    , "\\\\.\\pipe\\Q-Free-L7pipe" )
				GET_STRING( "SecurityDataPipeName"     , psInst->uParams.sQFree.szSecurityDataPipeName     , "\\\\.\\pipe\\Q-Free-SecurityPipe" )
				
			
			}
            else
            {
                peType = (TAG_INTERFACE_TYPES*)pcRegPath;
                if ( *peType != TAG_INTERFACE_QFREE )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }
				
                psInst->uParams.sQFree = *(QFREE_PARAMS*)( peType + 1 );
            }
			
            psInst->uParams.sQFree.dwStructBytes = sizeof(psInst->uParams.sQFree);
			
            //
            // Mettre en place l'espion
            //
            if ( psInst->dwConsoleTrace == 0 )
            {
                psInst->uParams.sQFree.pfHook = pfHook;
                psInst->uParams.sQFree.pvHookContext = pvHookContext;
            }
            else
            {
                psInst->uParams.sQFree.pfHook = TagHookGetDefaultHook();
                psInst->uParams.sQFree.pvHookContext = TagHookOpenDefaultContext(psInst,NULL,0, NULL );
            }
			
            //
            // Faire pointer les pointeurs de fonctions génériques de la couche
            // 2 sur les fonctions du protocole BAC.
            //
            psInst->pfL2Open    = (TAGL2_OPEN  *)&QFL2Open;
            psInst->pfL2Close   = (TAGL2_CLOSE *)&QFL2Close;
            psInst->pfL2Send    = (TAGL2_SEND  *)&QFL2SendMessage;
            psInst->pfL2Recv    = (TAGL2_RECV  *)&QFL2ReceiveMessage;
            psInst->pfL2Free    = (TAGL2_FREE  *)&QFL2FreeMessage;
            psInst->pfL2Wait    = (TAGL2_WAIT  *)&QFL2GetWaitableHandle;
            psInst->pfL2Comm    = (TAGL2_COMM  *)&QFL2GetCommInstance;
            psInst->pfL2SetOpt  = (TAGL2_SETOPT*)&QFL2SetOption;
            psInst->pfL2GetOpt  = (TAGL2_GETOPT*)&QFL2GetOption;
        
			
            psInst->hSL7 = QFSL7Open();
            if ( psInst->hSL7 == NULL )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
			
		}
	//-------------------------------------------------------------------------------
		else if ( eInterface == TAG_INTERFACE_IPGEA )
		{
            
			if ( ( dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS ) == 0 )
            {
                // Récupérer les paramètres du IPGEA dans le registre à
                GET_DWORD ( "MaxPendingMsg"		, psInst->uParams.sIPGEA.dwMaxPendingMsg   , 8  )
				GET_DWORD ( "Priority"			, psInst->uParams.sIPGEA.dwPriority        , THREAD_PRIORITY_HIGHEST )
				GET_DWORD ( "AutoClearBuffers"	, psInst->uParams.sIPGEA.dwAutoClearBuffers, 1  )
				GET_STRING( "IPAddress"			, psInst->uParams.sIPGEA.szIpAdress			, "192.168.100.71")
				GET_DWORD ( "IPPort"			, psInst->uParams.sIPGEA.dwPort				, 10001)
				GET_DWORD ( "CheckBeaconTimer"	, psInst->uParams.sIPGEA.dwCheckPoolingPeriod, 3000) // 				CheckBeaconTimer  -- DWORD argCheckBeacon - default 1000 ms
				GET_DWORD ( "ConnectionTimeout" , psInst->uParams.sIPGEA.dwConnectionTimeout ,5000)
				GET_DWORD ( "ChangeBeaconID"	, psInst->uParams.sIPGEA.bChangeBeaconID	,FALSE)
				GET_DWORD ( "ResetAfterTimeouts", psInst->uParams.sIPGEA.dwResetAfterTimeouts,3);
			}
            else
            {
                peType = (TAG_INTERFACE_TYPES*)pcRegPath;
                if ( *peType != TAG_INTERFACE_IPGEA )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }
				
                psInst->uParams.sIPGEA = *(IPGEA_PARAMS*)( peType + 1 );
            }
			
            psInst->uParams.sIPGEA.dwStructBytes = sizeof(psInst->uParams.sIPGEA);
			
            //
            // Mettre en place l'espion
            //
            if ( psInst->dwConsoleTrace == 0 )
            {
                psInst->uParams.sIPGEA.pfHook = pfHook;
                psInst->uParams.sIPGEA.pvHookContext = pvHookContext;
            }
            else
            {
                psInst->uParams.sIPGEA.pfHook = TagHookGetDefaultHook();
				psInst->uParams.sIPGEA.pvHookContext = TagHookOpenDefaultContext(psInst,NULL,0, NULL );
            }
			
            //
            // Faire pointer les pointeurs de fonctions génériques de la couche
            // 2 sur les fonctions du protocole BAC.
            //
            psInst->pfL2Open    = (TAGL2_OPEN  *)&IPGEAOpen;
            psInst->pfL2Close   = (TAGL2_CLOSE *)&IPGEAClose;
            psInst->pfL2Send    = (TAGL2_SEND  *)&IPGEASendMessage;
            psInst->pfL2Recv    = (TAGL2_RECV  *)&IPGEAReceiveMessage;
            psInst->pfL2Free    = (TAGL2_FREE  *)&IPGEAFreeMessage;
            psInst->pfL2Wait    = (TAGL2_WAIT  *)&IPGEAGetWaitableHandle;
            psInst->pfL2Comm    = (TAGL2_COMM  *)&IPGEAGetCommInstance;
            psInst->pfL2SetOpt  = (TAGL2_SETOPT*)&IPGEASetOption;
            psInst->pfL2GetOpt  = (TAGL2_GETOPT*)&IPGEAGetOption;
        
		
		}
		//-------------------------------------------------------------------------------
		else if (eInterface == TAG_INTERFACE_QFREE_DMI)
		{
			//
			// QFREE MD5850 beacon with DMI protocol 
			//

			if ((dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS) == 0)
			{
				//
				// Récupérer les paramètres du protocole ISA dans le registre à
				// l'endroit indiqué en paramètre
				//
				GET_DWORD( "MaxPendingMsg",		psInst->uParams.sQFreeDmi.dwMaxPendingMsg, 8)
				GET_DWORD( "Priority",			psInst->uParams.sQFreeDmi.dwPriority, THREAD_PRIORITY_HIGHEST)
				GET_DWORD( "AutoClearBuffers",	psInst->uParams.sQFreeDmi.dwAutoClearBuffers, 1)
				GET_DWORD( "Port",				psInst->uParams.sQFreeDmi.dwListenPort, 8080)
				GET_DWORD( "BeaconHTTP_Port",	psInst->uParams.sQFreeDmi.dwHTTPServer_Port, 80)
				GET_STRING( "BeaconHTTP_Address", psInst->uParams.sQFreeDmi.szHTTPServerIP, "192.168.0.1")
				GET_DWORD(	"DMITimerDelayMs",	psInst->uParams.sQFreeDmi.dwDMITimerDelayMs, 7000);
				GET_DWORD(	"TrsTimerDelayMs",	psInst->uParams.sQFreeDmi.dwTrsTimerDelayMs, 400);
				GET_DWORD(	"HighBeaconId",		psInst->uParams.sQFreeDmi.dwHighBeaconId, 10);
				GET_DWORD(	"LowBeaconId",		psInst->uParams.sQFreeDmi.dwLowBeaconId, 1);
				GET_DWORD( "UseMdrEnable",		psInst->uParams.sQFreeDmi.dwUseMDREnable, 1);
				GET_STRING("MdrEnable_ParamUrl", psInst->uParams.sQFreeDmi.szMdrEnable_ParamUrl, "GET /MuninBrowse/ParamDB/Set?Param=DsrcStack.Mdr.Enable&Value=")
				GET_STRING("ChangeBeaconID_ParamUrl", psInst->uParams.sQFreeDmi.szChangeBeaconID_ParamUrl, "GET /MuninBrowse/ParamDB/Set?Param=DsrcStack.Application.BeaconId&Value=")

			}
			else
			{
				peType = (TAG_INTERFACE_TYPES*)pcRegPath;
				if (*peType != TAG_INTERFACE_QFREE_DMI)
				{
					dwErr = ERROR_INVALID_PARAMETER;
					__leave;
				}

				psInst->uParams.sQFreeDmi = *(QFR_DMI_PARAMS*)(peType + 1);
			}

			psInst->uParams.sQFreeDmi.dwStructBytes = sizeof(psInst->uParams.sQFreeDmi);

			//
			// Mettre en place l'espion
			//
			if (psInst->dwConsoleTrace == 0)
			{
				psInst->uParams.sQFreeDmi.pfHook = pfHook;
				psInst->uParams.sQFreeDmi.pvHookContext = pvHookContext;
			}
			else
			{
				psInst->uParams.sQFreeDmi.pfHook = TagHookGetDefaultHook();
				psInst->uParams.sQFreeDmi.pvHookContext = TagHookOpenDefaultContext(psInst, NULL, 0, NULL);
			}

			//
			// Faire pointer les pointeurs de fonctions génériques de la couche
			// 2 sur les fonctions du protocole BAC.
			//
			psInst->pfL2Open = (TAGL2_OPEN  *)&QFDMI_L2Open;
			psInst->pfL2Close = (TAGL2_CLOSE *)&QFDMI_L2Close;
			psInst->pfL2Send = (TAGL2_SEND  *)&QFDMI_L2SendMessage;
			psInst->pfL2Recv = (TAGL2_RECV  *)&QFDMI_L2ReceiveMessage;
			psInst->pfL2Free = (TAGL2_FREE  *)&QFDMI_L2FreeMessage;
			psInst->pfL2Wait = (TAGL2_WAIT  *)&QFDMI_L2GetWaitableHandle;
			psInst->pfL2Comm = (TAGL2_COMM  *)&QFDMI_L2GetCommInstance;
			psInst->pfL2SetOpt = (TAGL2_SETOPT*)&QFDMI_L2SetOption;
			psInst->pfL2GetOpt = (TAGL2_GETOPT*)&QFDMI_L2GetOption;

		}
		//-------------------------------------------------------------------------------
		else
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }

        //
        // Mémoriser le type de l'interface dans les données de l'instance
        //
        psInst->eInterface = eInterface;

        //
        // Créer une instance de la couche 2 (BAC ou ISA en fonction des
        // paramètres d'appel de la fonction).
        //
        dwErr = psInst->pfL2Open( &psInst->hL2, &psInst->uParams );
        if ( dwErr != NO_ERROR )
            __leave;
        psInst->hL1 = psInst->pfL2Comm( psInst->hL2 );

        //
        // Créer une instance de la couche 7 (CENL7).
        //
        psInst->hL7 = CENL7Open();
        if ( psInst->hL7 == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        //
        // Préparer les paramètres de sortie
        //
        *ppsInst = psInst;
        psInst = NULL;
        dwErr = NO_ERROR;

        if ( ( dwFlags & TAG_INTERFACE_FLAG_BUILT_PARAMS ) != 0 )
            HeapFree( GetProcessHeap(), 0, pcRegPath );
    }
    __finally
    {
        //
        // En cas d'échec de création de l'instance, si quelquechose
        // a été alloué, on le désalloue.
        if ( psInst != NULL )
            TagClose( psInst, 0 );
        
        
    }

	return dwErr;
}



PUBLIC void * WINAPI TagBuildCommunicatorParams( 
                IN   DWORD        dwMaxEnqRetry,
                IN   DWORD        dwMaxMsgRetry,
                IN   DWORD        dwTimer1,
                IN   DWORD        dwTimer2,
                IN   DWORD        dwMaxPendingMsg,
                IN   DWORD        dwPort,
                IN   DWORD        dwPriority,
                IN   DWORD        dwAutoClearBuffer,
                IN   const char * szPortSettings )
{
    BAC_PARAMS          * psParams = NULL;
    TAG_INTERFACE_TYPES * peType = NULL;
    
    peType = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psParams) + sizeof(*peType) );
    psParams = (BAC_PARAMS*)(peType+1);
    if ( psParams != NULL )
    {
        *peType = TAG_INTERFACE_COMMUNICATOR;
        psParams->dwMaxEnqRetry = dwMaxEnqRetry;
        psParams->dwMaxMsgRetry = dwMaxMsgRetry;
        psParams->dwTimer1 = dwTimer1;
        psParams->dwTimer2 = dwTimer2;
        psParams->dwMaxPendingMsg = dwMaxPendingMsg;
        psParams->dwPort = dwPort;
        psParams->dwPriority = dwPriority;
        psParams->dwAutoClearBuffers = dwAutoClearBuffer;
        strncpy( psParams->szPortSettings, szPortSettings, sizeof(psParams->szPortSettings) );
        psParams->szPortSettings[sizeof(psParams->szPortSettings)-1] = 0;
    }

    return (void*)peType;
}



PUBLIC void * WINAPI TagBuildWiredParams( 
                IN   DWORD        dwTimer,
                IN   DWORD        dwMaxPendingMsg,
                IN   DWORD        dwPort,
                IN   DWORD        dwPriority,
                IN   DWORD        dwAutoClearBuffer,
                IN   const char * szPortSettings )
{
    ISA_PARAMS          * psParams = NULL;
    TAG_INTERFACE_TYPES * peType = NULL;
    
    peType = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psParams) + sizeof(*peType) );
    psParams = (ISA_PARAMS*)(peType+1);
    if ( psParams != NULL )
    {
        *peType = TAG_INTERFACE_WIRED;
        psParams->dwTimer = dwTimer;
        psParams->dwMaxPendingMsg = dwMaxPendingMsg;
        psParams->dwPort = dwPort;
        psParams->dwPriority = dwPriority;
        psParams->dwAutoClearBuffers = dwAutoClearBuffer;
        strncpy( psParams->szPortSettings, szPortSettings, sizeof(psParams->szPortSettings) );
        psParams->szPortSettings[sizeof(psParams->szPortSettings)-1] = 0;
    }

    return (void*)peType;
}




PUBLIC void * WINAPI TagBuildCommunicatorFastParams( 
                IN   DWORD        dwTimer,
                IN   DWORD        dwMaxPendingMsg,
                IN   DWORD        dwPort,
                IN   DWORD        dwPriority,
                IN   DWORD        dwAutoClearBuffer,
                IN   const char * szPortSettings )
{
    ISA_PARAMS          * psParams = NULL;
    TAG_INTERFACE_TYPES * peType = NULL;
    
    peType = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psParams) + sizeof(*peType) );
    psParams = (ISA_PARAMS*)(peType+1);
    if ( psParams != NULL )
    {
        *peType = TAG_INTERFACE_COMMUNICATOR_FAST;
        psParams->dwTimer = dwTimer;
        psParams->dwMaxPendingMsg = dwMaxPendingMsg;
        psParams->dwPort = dwPort;
        psParams->dwPriority = dwPriority;
        psParams->dwAutoClearBuffers = dwAutoClearBuffer;
        strncpy( psParams->szPortSettings, szPortSettings, sizeof(psParams->szPortSettings) );
        psParams->szPortSettings[sizeof(psParams->szPortSettings)-1] = 0;
    }

    return (void*)peType;
}




PUBLIC void * WINAPI TagBuildIso7816Params( 
                IN   DWORD        dwMaxPendingMsg,
                IN   DWORD        dwPort,
                IN   DWORD        dwDebit,
                IN   DWORD        dwPriority,
                IN   DWORD        dwAutoClearBuffer,
                IN   const char * szDownloadFile,
                IN   const char * szTransportDll,
                IN   const char * szLowLevelDll )
{
    ISO_PARAMS          * psParams = NULL;
    TAG_INTERFACE_TYPES * peType = NULL;
    
    peType = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psParams) + sizeof(*peType) );
    psParams = (ISO_PARAMS*)(peType+1);
    if ( psParams != NULL )
    {
        *peType = TAG_INTERFACE_ISO7816;
        psParams->dwMaxPendingMsg = dwMaxPendingMsg;
        psParams->dwPort = dwPort;
        psParams->dwDebit = dwDebit;
        psParams->dwPriority = dwPriority;
        psParams->dwAutoClearBuffers = dwAutoClearBuffer;
        strncpy( psParams->szDownloadFile, szDownloadFile, sizeof(psParams->szDownloadFile) );
        psParams->szDownloadFile[sizeof(psParams->szDownloadFile)-1] = 0;
        strncpy( psParams->szTransportDll, szTransportDll, sizeof(psParams->szTransportDll) );
        psParams->szTransportDll[sizeof(psParams->szTransportDll)-1] = 0;
        strncpy( psParams->szLowLevelDll, szLowLevelDll, sizeof(psParams->szLowLevelDll) );
        psParams->szLowLevelDll[sizeof(psParams->szLowLevelDll)-1] = 0;
    }

    return (void*)peType;
}



PUBLIC void * WINAPI TagBuildQFREEParams( 
										   IN   DWORD        dwMaxPendingMsg,
										   IN   DWORD        dwPriority,
										   IN   DWORD        dwAutoClearBuffer,
										   IN   const char * szPortSettings,
										   IN   const char * szL7DataPipeName,
										   IN   const char * szSecurityDataPipeName )
{
    QFREE_PARAMS          * psParams = NULL;
    TAG_INTERFACE_TYPES * peType = NULL;
    
    peType = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psParams) + sizeof(*peType) );
    psParams = (QFREE_PARAMS*)(peType+1);
    
	if ( psParams != NULL )
    {
        *peType = TAG_INTERFACE_QFREE;
        psParams->dwMaxPendingMsg = dwMaxPendingMsg;
        psParams->dwPriority = dwPriority;
        psParams->dwAutoClearBuffers = dwAutoClearBuffer;
        
		strncpy( psParams->szPortSettings, szPortSettings, sizeof(psParams->szPortSettings) );
        psParams->szPortSettings[sizeof(psParams->szPortSettings)-1] = 0;
        
		strncpy( psParams->szL7DataPipeName, szL7DataPipeName, sizeof(psParams->szL7DataPipeName) );
        psParams->szL7DataPipeName[sizeof(psParams->szL7DataPipeName)-1] = 0;
        
		strncpy( psParams->szSecurityDataPipeName, szSecurityDataPipeName, sizeof(psParams->szSecurityDataPipeName) );
        psParams->szSecurityDataPipeName[sizeof(psParams->szSecurityDataPipeName)-1] = 0;
    }
	
    return (void*)peType;
}



PUBLIC void * WINAPI TagBuildIpGEAParams(	IN   DWORD        dwMaxPendingMsg,
											IN   DWORD        dwPriority,
											IN   DWORD        dwAutoClearBuffer,
											IN   const char * szIpAddres,
											IN   DWORD		  dwPort,
											IN   DWORD		  dwCheckPoolingPeriod,
											IN   DWORD		  dwConnectionTimeout,
											IN	 DWORD		  dwChangeBeaconID,
											IN	DWORD		  dwUseFrequency)
{

    IPGEA_PARAMS          * psParams = NULL;
    TAG_INTERFACE_TYPES * peType = NULL;
    
    peType = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psParams) + sizeof(*peType) );
    psParams = (IPGEA_PARAMS*)(peType+1);
    
	if ( psParams != NULL )
    {
        *peType = TAG_INTERFACE_IPGEA;
        psParams->dwMaxPendingMsg = dwMaxPendingMsg;
        psParams->dwPriority = dwPriority;
        psParams->dwAutoClearBuffers = dwAutoClearBuffer;
        
		strncpy_s(psParams->szIpAdress, sizeof(psParams->szIpAdress), szIpAddres, sizeof(psParams->szIpAdress));
        psParams->szIpAdress[sizeof(psParams->szIpAdress)-1] = 0;
		
		psParams->dwPort = dwPort;
		psParams->dwCheckPoolingPeriod = dwCheckPoolingPeriod;
		psParams->dwConnectionTimeout = dwConnectionTimeout;
		psParams->bChangeBeaconID = (dwChangeBeaconID!=0);

    }
	
    return (void*)peType;
}

PUBLIC void * WINAPI TagBuildQFDMIParams(IN   DWORD        dwMaxPendingMsg,
											IN   DWORD        dwPriority,
											IN   DWORD        dwAutoClearBuffer,
											IN   DWORD		  dwListenPort,
											IN   const char * szHTTPServerIP,
											IN   DWORD		  dwHTTPServer_Port,
											IN   DWORD		  dwDMITimerDelayMs,
											IN   DWORD		  dwTrsTimerDelayMs,
											IN   DWORD		  dwHighBeaconId,
											IN   DWORD		  dwLowBeaconId,
											IN   DWORD		  dwUseMDREnable,
											IN   const char * szMdrEnable_ParamUrl,
											IN   const char * szChangeBeaconID_ParamUrl)
{

	QFR_DMI_PARAMS      * psParams = NULL;
	TAG_INTERFACE_TYPES * peType = NULL;

	peType = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psParams) + sizeof(*peType));
	psParams = (QFR_DMI_PARAMS*)(peType + 1);

	if (psParams != NULL)
	{
		*peType = TAG_INTERFACE_QFREE_DMI;
		psParams->dwMaxPendingMsg = dwMaxPendingMsg;
		psParams->dwPriority = dwPriority;
		psParams->dwAutoClearBuffers = dwAutoClearBuffer;
		psParams->dwListenPort = dwListenPort;

		strncpy_s(psParams->szHTTPServerIP, sizeof(psParams->szHTTPServerIP), szHTTPServerIP, sizeof(psParams->szHTTPServerIP));
		psParams->szHTTPServerIP[sizeof(psParams->szHTTPServerIP) - 1] = 0;

		psParams->dwHTTPServer_Port = dwHTTPServer_Port;

		psParams->dwDMITimerDelayMs = dwDMITimerDelayMs;
		psParams->dwTrsTimerDelayMs = dwTrsTimerDelayMs;
		psParams->dwHighBeaconId = dwHighBeaconId;
		psParams->dwLowBeaconId = dwLowBeaconId;
		psParams->dwUseMDREnable = dwUseMDREnable;

		if(szMdrEnable_ParamUrl!=NULL)
		{
			strncpy_s(psParams->szMdrEnable_ParamUrl, sizeof(psParams->szMdrEnable_ParamUrl), szMdrEnable_ParamUrl, sizeof(psParams->szMdrEnable_ParamUrl));
			psParams->szMdrEnable_ParamUrl[sizeof(psParams->szMdrEnable_ParamUrl) - 1] = 0;
		}
		else
		{
			strcpy_s(psParams->szMdrEnable_ParamUrl, sizeof(psParams->szMdrEnable_ParamUrl), "GET /MuninBrowse/ParamDB/Set?Param=DsrcStack.Mdr.Enable&Value=");
			psParams->szMdrEnable_ParamUrl[sizeof(psParams->szMdrEnable_ParamUrl) - 1] = 0;
		}



		if (szChangeBeaconID_ParamUrl != NULL)
		{
			strncpy_s(psParams->szChangeBeaconID_ParamUrl, sizeof(psParams->szChangeBeaconID_ParamUrl), szChangeBeaconID_ParamUrl, sizeof(psParams->szChangeBeaconID_ParamUrl));
			psParams->szChangeBeaconID_ParamUrl[sizeof(psParams->szChangeBeaconID_ParamUrl) - 1] = 0;
		}
		else
		{
			if(psParams->dwUseMDREnable)
			{
				strcpy_s(psParams->szChangeBeaconID_ParamUrl, sizeof(psParams->szChangeBeaconID_ParamUrl), "GET /MuninBrowse/ParamDB/Set?Param=DsrcStack.Application.BeaconId&Value=");
				psParams->szChangeBeaconID_ParamUrl[sizeof(psParams->szChangeBeaconID_ParamUrl) - 1] = 0;
			}
			else
			{
				strcpy_s(psParams->szChangeBeaconID_ParamUrl, sizeof(psParams->szChangeBeaconID_ParamUrl), "GET /MuninBrowse/ParamDB/Set?Param=DsrcStack.BeaconId&Value=");
				psParams->szChangeBeaconID_ParamUrl[sizeof(psParams->szChangeBeaconID_ParamUrl) - 1] = 0;
			}
		}
	}

	return (void*)peType;

}


PUBLIC void WINAPI TagDestroyParams( 
                IN   void * pvParams )
{
    HeapFree( GetProcessHeap(), 0, pvParams );
}




PUBLIC void * WINAPI TagGetL1(
                IN  TAG_INSTANCE  * psInst )
{
    return psInst->hL1;
}



PUBLIC void * WINAPI TagGetL2(
                IN  TAG_INSTANCE  * psInst )
{
    return psInst->hL2;
}


PUBLIC void * WINAPI TagGetL7(
                IN  TAG_INSTANCE  * psInst )
{
    return psInst->hL7;
}


PUBLIC void * WINAPI TagGetSL7(
                IN  TAG_INSTANCE  * psInst )
{
    return psInst->hSL7;
}



PUBLIC void WINAPI TagClose(
                IN  TAG_INSTANCE  * psInst,
                IN  DWORD           dwTimeout )
{
    //
    // Désallouer tout ce qui a été alloué.
    // Attention, dans la mesure où cette fonction est également
    // appelée en interne pour nettoyer une instance créée partiellement,
    // certains des objets peuvent ne pas avoir été créé. Pour ne
    // pas avoir de problème, avant toute libération, on vérifier que
    // la valeur n'est pas NULL.
    //
    if ( ( psInst->hSL7 != NULL ) && ( psInst->hSL7 != TAG_NO_SUBL7 ) )
    {    
		if(psInst->eInterface != TAG_INTERFACE_QFREE) 
								TRSPClose( psInst->hSL7 );
		else //if(psInst->eInterface == TAG_INTERFACE_QFREE) 
			QFSL7Close(psInst->hSL7);
	}

	if ( psInst->hL7 != NULL )
        CENL7Close( psInst->hL7 );

    if ( psInst->hL2 != NULL )
        psInst->pfL2Close( psInst->hL2, dwTimeout );

    if ( ( psInst->dwConsoleTrace != 0 ) && ( psInst->eInterface == TAG_INTERFACE_WIRED ) )
    {
        if ( psInst->uParams.sIsa.pvHookContext != NULL )
            TagHookCloseDefaultContext( psInst->uParams.sIsa.pvHookContext );
    }
    else if ( ( psInst->dwConsoleTrace != 0 ) && ( psInst->eInterface == TAG_INTERFACE_COMMUNICATOR ) )
    {
        if ( psInst->uParams.sBac.pvHookContext != NULL )
            TagHookCloseDefaultContext( psInst->uParams.sBac.pvHookContext );
    }
    else if ( ( psInst->dwConsoleTrace != 0 ) && ( psInst->eInterface == TAG_INTERFACE_COMMUNICATOR_FAST ) )
    {
        if ( psInst->uParams.sIsa.pvHookContext != NULL )
            TagHookCloseDefaultContext( psInst->uParams.sIsa.pvHookContext );
    }
    else if ( ( psInst->dwConsoleTrace != 0 ) && ( psInst->eInterface == TAG_INTERFACE_IPGEA ) )
    {
        if ( psInst->uParams.sIPGEA.pvHookContext != NULL )
            TagHookCloseDefaultContext( psInst->uParams.sIPGEA.pvHookContext );
    }
	else if ((psInst->dwConsoleTrace != 0) && (psInst->eInterface == TAG_INTERFACE_QFREE_DMI))
	{
		if (psInst->uParams.sIPGEA.pvHookContext != NULL)
			TagHookCloseDefaultContext(psInst->uParams.sQFreeDmi.pvHookContext);
	}
	


    HeapFree( GetProcessHeap(), 0, psInst );
}


PUBLIC HANDLE WINAPI TagGetWaitableHandle(
                IN  TAG_INSTANCE  * psInst )
{
    //
    // L'objet permettant des attentes est celui de la couche 2. Il signale
    // donc la réception de données. A charge à l'utilisateur de la librairie
    // des les traiter.
    //
    return psInst->pfL2Wait( psInst->hL2 );
}





PUBLIC DWORD WINAPI TagInterfaceRequestEx( 
                IN              TAG_INSTANCE          * psInst,
                IN              TAG_INTERFACE_REQUEST   eReq,
                OUT             BOOL                  * pfResponse,
                OPTIONAL IN     HANDLE                  hEvent, 
                OPTIONAL OUT    DWORD                 * pdwErr,
                IN              DWORD                   dwDataBytes,
                OPTIONAL IN     BYTE                  * pbDataBytes )
{
    DWORD       dwErr = ERROR_INVALID_DATA;
    DWORD       dwPos;
    DWORD       dwLen;
    BOOL        fResult;
    BYTE      * pbData;

    __try
    {
        if ( psInst->eInterface == TAG_INTERFACE_WIRED )
        {
            //
            // Il n'y a pas de sous-couche 7, les requêtes d'interface ne peuvent
            // être satisfaites.
            // 

            if ( ( eReq != TAG_REQUEST_MODE    ) &&
                 ( eReq != TAG_REQUEST_ABORT   ) &&
                 ( eReq != TAG_REQUEST_STATUS  ) )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }

            if ( pdwErr != NULL )
                *pdwErr = NO_ERROR;

            if ( hEvent != NULL )
            {
                if ( ! SetEvent( hEvent ) )
                {
                    dwErr = GetLastError();
                    __leave;
                }
            }

            //
            // Rien n'a été fait, on n'attend donc pas de réponse.
            //
            *pfResponse = FALSE;

            dwErr = NO_ERROR;
        }

        else if ( ( psInst->eInterface == TAG_INTERFACE_COMMUNICATOR      ) ||
                  ( psInst->eInterface == TAG_INTERFACE_COMMUNICATOR_FAST ) )
        {
            //
            // S'il s'agit d'un protocole avec sous-couche 7, on génére un message
            // correspondant à la fonction demandée
            //
            if ( eReq == TAG_REQUEST_MODE    )
            {
                psInst->sSL7Api.bActionType = TRSP_ACTION_SET_MODE;
                psInst->sSL7Api.sSetModeReq.bMode = TRSP_MODE_TRANSPARENT;
            }
            else if ( eReq == TAG_REQUEST_ABORT )
            {
                psInst->sSL7Api.bActionType = TRSP_ACTION_BEACON_OFF;
            }
            else if ( eReq == TAG_REQUEST_STATUS )
            {
                psInst->sSL7Api.bActionType = TRSP_ACTION_GET_STATUS;
            }
            else if ( ( eReq == TAG_REQUEST_CUSTOM ) && 
                      ( pbDataBytes != NULL ) &&
                      ( dwDataBytes == sizeof( psInst->sSL7Api ) ) )
            {
                psInst->sSL7Api = *(TRSP_API*)pbDataBytes;
            }
            else
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }
                                                                                
            //
            // On fait ensuite générer le message par la sous-couche 7 dans le
            // buffer d'émission.
            //
            dwPos = 0;
            dwLen = sizeof( psInst->tbOutSvc );
            pbData = psInst->tbOutSvc;
            fResult = TRSPBuildMessage(
                    psInst->hSL7,
                    pbData,
                    &dwPos,
                    &dwLen,
                    &psInst->sSL7Api );
            if ( ! fResult )
            {
                dwErr = ERROR_INVALID_DATA;
                __leave;
            }

            //
            // Mettre à jour la taille des données utiles dans le buffer d'émission et
            // envoyer le message.
            //
            psInst->dwOutSvcBytes = dwPos;
            dwErr = psInst->pfL2Send(
                    psInst->hL2,
                    psInst->tbOutSvc,
                    psInst->dwOutSvcBytes,
                    hEvent,
                    pdwErr );

            //
            // L'envoie effectif du message suppose qu'une réponse va être renvoyée
            //
            *pfResponse = TRUE; //this one sais there is response to expect
        }

        else if ( psInst->eInterface == TAG_INTERFACE_ISO7816 )
        {
            if ( ( eReq == TAG_REQUEST_MODE   ) ||
                 ( eReq == TAG_REQUEST_ABORT  ) ||
                 ( eReq == TAG_REQUEST_STATUS ) )
            {
                dwErr = psInst->pfL2GetOpt( psInst->hL2, ISO_OPTION_DW_BEACON_ID, &dwLen, NULL );
                if ( pdwErr != NULL )
                    *pdwErr = dwErr;
                if ( dwErr != NO_ERROR )
                    __leave;
            }
            else
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }


			if ( hEvent != NULL )
            {
                if ( ! SetEvent( hEvent ) )
                {
                    dwErr = GetLastError();
                    __leave;
                }
            }

            //
            // Rien n'a été fait, on n'attend donc pas de réponse.
            //
            *pfResponse = FALSE;

            dwErr = NO_ERROR;
        }
		else if ( psInst->eInterface == TAG_INTERFACE_QFREE )
        {
 
            if ( eReq == TAG_REQUEST_ABORT )
            {
                psInst->sQFSL7Api.bMessageType = QFSL7_ACTION_QFSL7_STOP;
                psInst->sQFSL7Api.dwParamBytes = 0;
            }
            else if ( eReq == TAG_REQUEST_STATUS )
            {
                psInst->sQFSL7Api.bMessageType = QFSL7_ACTION_QFSL7_QUERY;
                psInst->sQFSL7Api.dwParamBytes = 2;
                strcpy( psInst->sQFSL7Api.tbParams, "MR" );

            }
            else if ( eReq == TAG_REQUEST_MODE )
            {
                psInst->sQFSL7Api.bMessageType = QFSL7_ACTION_QFSL7_CONFIG;
                _snprintf( psInst->sQFSL7Api.tbParams, 
                           sizeof(psInst->sQFSL7Api.tbParams),
                           "SERIAL %s",
                           psInst->uParams.sQFree.szPortSettings );
                psInst->sQFSL7Api.tbParams[sizeof(psInst->sQFSL7Api.tbParams)-1] = 0;
				psInst->sQFSL7Api.dwParamBytes = (DWORD)strlen(psInst->sQFSL7Api.tbParams);
            }
            else if ( ( eReq == TAG_REQUEST_CUSTOM ) && 
                      ( pbDataBytes != NULL ) &&
                      ( dwDataBytes == sizeof( psInst->sQFSL7Api ) ) )
            {
                psInst->sQFSL7Api = *(QFSL7_API*)pbDataBytes;
            }
			else
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }

            //
            // Generate the sub SL7 message 
            //
            dwPos = 0;
            dwLen = sizeof( psInst->tbOutSvc );
            pbData = psInst->tbOutSvc;
            fResult = QFSL7BuildMessage(
				psInst->hSL7,
				pbData,
				&dwPos,
				&dwLen,
				&psInst->sQFSL7Api );
            if ( ! fResult )
            {
                dwErr = ERROR_INVALID_DATA;
                __leave;
            }
			
            //
            // Update the length of package and send 
            //
            psInst->dwOutSvcBytes = dwPos;
            dwErr = psInst->pfL2Send(
				psInst->hL2,
				psInst->tbOutSvc,
				psInst->dwOutSvcBytes,
				hEvent,
				pdwErr );
			
            //
            // L'envoie effectif du message suppose qu'une réponse va être renvoyée
            //
            *pfResponse = TRUE; //this one sais there is response to expect
        }
 		else if ( psInst->eInterface == TAG_INTERFACE_IPGEA )
        {

			if ( eReq == TAG_REQUEST_ABORT )
            {
                psInst->sGEAIPL7Api.bMessageType = GEAIP_INTF_REQ_ABORT;
				*pfResponse = FALSE;
            }
            else if ( eReq == TAG_REQUEST_STATUS )
            {
                psInst->sGEAIPL7Api.bMessageType = GEAIP_INTF_REQ_GETSTATUS;
				*pfResponse = TRUE;
            }
            else if ( eReq == TAG_REQUEST_MODE )
            {
                psInst->sGEAIPL7Api.bMessageType = GEAIP_INTF_REQ_MODE_TRANSPARENT;
				*pfResponse = TRUE;
            }
			else
            {
              *pfResponse = FALSE;
               dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }

			//Implement coding and decoding of the message
			//do I can use the pfL2Send function and the queue to handle the request in worker thread
			//All the messages are to be sent the same way, but internally the working thread will decode the message and 
			// decide how to process it
			
            dwPos = 0;
            dwLen = sizeof( psInst->tbOutSvc );
			//generate a message in a byte array to be able to send it to working thread
			fResult = IPGEAL7_EncodeMessage(
				psInst->hL2,
				psInst->tbOutSvc,
				&dwPos,
				&dwLen,
				&psInst->sGEAIPL7Api );
            if ( ! fResult )
            {
                dwErr = ERROR_INVALID_DATA;
                __leave;
            }
			
            //
            // Update the length of package and send 
            //
            psInst->dwOutSvcBytes = dwPos;
            dwErr = psInst->pfL2Send(
				psInst->hL2,
				psInst->tbOutSvc,
				psInst->dwOutSvcBytes,
				hEvent,
				pdwErr );


		}
		else if (psInst->eInterface == TAG_INTERFACE_QFREE_DMI)
		{
			if (eReq == TAG_REQUEST_ABORT)
			{
				psInst->sQFDMIL7Api.bMessageType = QFDMI_INTF_REQ_ABORT;
				*pfResponse = FALSE;
			}
			else if (eReq == TAG_REQUEST_STATUS)
			{
				psInst->sQFDMIL7Api.bMessageType = QFDMI_INTF_REQ_GETSTATUS;
				*pfResponse = TRUE;
			}
			else if (eReq == TAG_REQUEST_MODE)
			{
				psInst->sQFDMIL7Api.bMessageType = QFDMI_INTF_REQ_MODE_TRANSPARENT;
				*pfResponse = TRUE;
			}
			else
			{
				*pfResponse = FALSE;
				dwErr = ERROR_INVALID_PARAMETER;
				__leave;
			}

			//Implement coding and decoding of the message
			//do I can use the pfL2Send function and the queue to handle the request in worker thread
			//All the messages are to be sent the same way, but internally the working thread will decode the message and 
			// decide how to process it

			dwPos = 0;
			dwLen = sizeof(psInst->tbOutSvc);
			//generate a message in a byte array to be able to send it to working thread (INTERNAL MESSAGE)
			fResult = QFDMI_BuildInternalMessage(
				psInst->hL2,
				psInst->tbOutSvc,
				&dwPos,
				&dwLen,
				&psInst->sQFDMIL7Api);
			if (!fResult)
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}

			//
			// Update the length of package and send 
			//
			psInst->dwOutSvcBytes = dwPos;
			dwErr = psInst->pfL2Send(
				psInst->hL2,
				psInst->tbOutSvc,
				psInst->dwOutSvcBytes,
				hEvent,
				pdwErr);
		}
		else
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }
    }
    __finally
    {
		;
    }

	return dwErr;
}



PUBLIC DWORD WINAPI TagInterfaceRequest( 
                IN              TAG_INSTANCE          * psInst,
                IN              TAG_INTERFACE_REQUEST   eReq,
                OUT             BOOL                  * pfResponse,
                OPTIONAL IN     HANDLE                  hEvent, 
                OPTIONAL OUT    DWORD                 * pdwErr )
{
    return TagInterfaceRequestEx( psInst, eReq, pfResponse, hEvent, pdwErr, 0, NULL );
}



PUBLIC DWORD WINAPI TagSessionRequest(
                IN              TAG_INSTANCE      * psInst,
                IN              TAG_SESSION_STATES  eSession,
                IN              CENL7_API         * psReqTable,
                IN              DWORD               dwReqCount,
                OPTIONAL IN     HANDLE              hEvent, 
                OPTIONAL OUT    DWORD             * pdwErr )
{
    DWORD       dwErr = NO_ERROR;
    DWORD       dwIndex;
    DWORD       dwPos;
    DWORD       dwLen;
    BOOL        fResult;
    BYTE      * pbData;

	__try
	{
		if ( psReqTable == TAG_RETRY_LAST_REQUEST ) //use the previous request
		{
			if ( psInst->dwOutBytes == 0 )
			{
				dwErr = ERROR_INVALID_PARAMETER;
				__leave;
			}
			dwPos = psInst->dwOutBytes;
		}
		else
		{
			if ( psInst->eInterface == TAG_INTERFACE_WIRED )
			{
				//
				// S'il s'agit d'un protocole sans sous-couche 7, on génére directement
				// les messages dans le buffer d'émission.
				//
				dwPos = 0;
				dwLen = sizeof( psInst->tbOut );
				pbData = psInst->tbOut;
				
                for ( dwIndex = 0 ; dwIndex < dwReqCount ; dwIndex ++ )
                {
                    fResult = CENL7BuildApdu(
                        psInst->hL7,
                        pbData,
                        &dwPos,
                        &dwLen,
                        &psReqTable[dwIndex] );
                    if ( ! fResult )
                    {
                        dwErr = ERROR_INVALID_DATA;
                        __leave;
                    }
                }
            }

            else if ( ( psInst->eInterface == TAG_INTERFACE_COMMUNICATOR      ) ||
                      ( psInst->eInterface == TAG_INTERFACE_COMMUNICATOR_FAST ) )
            {
                //
                // S'il s'agit d'un protocole avec sous-couche 7, on génére tout d'abord
                // les messages dans le buffer de la sous couche 7.
                //
                dwPos = 0;
                dwLen = sizeof( psInst->sSL7Api.sTrspReq.tbReq );
                pbData = psInst->sSL7Api.sTrspReq.tbReq;

                for ( dwIndex = 0 ; dwIndex < dwReqCount ; dwIndex ++ )
                {
                    fResult = CENL7BuildApdu(
                            psInst->hL7,
                            pbData,
                            &dwPos,
                            &dwLen,
                            &psReqTable[dwIndex] );
                    if ( ! fResult )
                    {
                        dwErr = ERROR_INVALID_DATA;
                        __leave;
                    }
                }

                //
                // On finit ensuite de préparer la définition du message pour la
                // sous couche L7.
                //
                psInst->sSL7Api.bActionType = ( eSession == TAG_SESSION_BEGIN ) ? TRSP_ACTION_TRSP_INI :
                                                ( eSession == TAG_SESSION_END   ) ? TRSP_ACTION_TRSP_END :
                                                                                    TRSP_ACTION_TRSP_CMD;
                psInst->sSL7Api.sTrspReq.dwReqSize = dwPos;
                                                                                
                //
                // On fait ensuite générer le message par la sous-couche 7 dans le
                // buffer d'émission.
                //
                dwPos = 0;
                dwLen = sizeof( psInst->tbOut );
                pbData = psInst->tbOut;
                fResult = TRSPBuildMessage(
                        psInst->hSL7,
                        pbData,
                        &dwPos,
                        &dwLen,
                        &psInst->sSL7Api );
                if ( ! fResult )
                {
                    dwErr = ERROR_INVALID_DATA;
                    __leave;
                }
            }

            else if ( psInst->eInterface == TAG_INTERFACE_ISO7816 )
            {
                //
                // S'il s'agit d'un protocole sans sous-couche 7, on génére directement
                // les messages dans le buffer d'émission.
                //
                dwPos = 0;
                dwLen = sizeof( psInst->tbOut );
                pbData = psInst->tbOut;

                for ( dwIndex = 0 ; dwIndex < dwReqCount ; dwIndex ++ )
                {
                    fResult = CENL7BuildApdu(
                        psInst->hL7,
                        pbData,
                        &dwPos,
                        &dwLen,
                        &psReqTable[dwIndex] );
                    if ( ! fResult )
                    {
                        dwErr = ERROR_INVALID_DATA;
                        __leave;
                    }
                }

                dwErr = psInst->pfL2SetOpt( psInst->hL2, 
                                            ISO_OPTION_F_LAST_MESSAGE, 
                                            eSession == TAG_SESSION_END ? 1 : 0,
                                            NULL );
                if ( dwErr != NO_ERROR )
                    __leave;
            }
            else if ( psInst->eInterface == TAG_INTERFACE_QFREE )
            {

                //
                // S'il s'agit d'un protocole avec sous-couche 7, on génére tout d'abord
                // les messages dans le buffer de la sous couche 7.
                //
				if ( eSession ==  TAG_SESSION_BEGIN )
                {
                    if ( dwReqCount > 0 )
                    {
                        if ( psReqTable->bBlockType == CENL7_APDU_INI_REQ )
                        {
                            dwErr = psInst->pfL2SetOpt( psInst->hL2,
                                                        QFL2_OPTION_DW_BEACONID,
                                                        psReqTable->sIniReq.dwBeaconId,
                                                        NULL );
                            if ( dwErr != NO_ERROR )
                                __leave;
                        }
                    }

                    //
                    // No content is given for the BST, it is fully built by the
                    // beacon itself
                    //
				    psInst->sQFSL7Api.bMessageType = QFSL7_ACTION_QFSL7_INIT;
                    dwLen = sizeof( psInst->sQFSL7Api.tbParams );
                }
                else
                {
                    if ( dwReqCount == 0 )
                    {
                        //
                        // No content is given for the last request
                        //
    				    psInst->sQFSL7Api.bMessageType = QFSL7_ACTION_QFSL7_STOP;
                        dwLen = 0;
                    }
                    else
                    {
                        //
                        // A content is given for the last request
                        //
    				    psInst->sQFSL7Api.bMessageType = QFSL7_ACTION_QFSL7_FRAME;
                        dwLen = sizeof( psInst->sQFSL7Api.tbParams );
                    }
                }

                dwPos = 0;
                if ( dwLen != 0 )
                {
					for ( dwIndex = 0 ; dwIndex < dwReqCount ; dwIndex ++ )
					{
						fResult = CENL7BuildApdu(
							psInst->hL7,
							psInst->sQFSL7Api.tbParams,
							&dwPos,
							&dwLen,
							&psReqTable[dwIndex] );
						if ( ! fResult )
						{
							dwErr = ERROR_INVALID_DATA;
							__leave;
						}
					}
                }
                psInst->sQFSL7Api.dwParamBytes = dwPos;

                //
                // On fait ensuite générer le message par la sous-couche 7 dans le
                // buffer d'émission.
                //
                dwPos = 0;
                dwLen = sizeof( psInst->tbOut );
                pbData = psInst->tbOut;
                fResult = QFSL7BuildMessage(
					psInst->hSL7,
					pbData,
					&dwPos,
					&dwLen,
					&psInst->sQFSL7Api );
                if ( ! fResult )
                {
                    dwErr = ERROR_INVALID_DATA;
                    __leave;
                }
				
			}
			else if ( psInst->eInterface == TAG_INTERFACE_IPGEA )
			{
				//TO DO Generate what to send to the read write thread
				// and put it to psInst->tbOut and psInst->dwOutBytes
                dwPos = 0;
                dwLen = sizeof( psInst->sGEAIPL7Api.tbL2Bytes);
                pbData = psInst->sGEAIPL7Api.tbL2Bytes;

                for ( dwIndex = 0 ; dwIndex < dwReqCount ; dwIndex ++ )
                {
                    fResult = CENL7BuildApdu(
                            psInst->hL7,
                            pbData,
                            &dwPos,
                            &dwLen,
                            &psReqTable[dwIndex] );
                    if ( ! fResult )
                    {
                        dwErr = ERROR_INVALID_DATA;
                        __leave;
                    }
                }


				if(eSession == TAG_SESSION_BEGIN)
                {   
					if ( dwReqCount > 0 )
                    {
                        if ( psReqTable->bBlockType == CENL7_APDU_INI_REQ )
                        {
                            dwErr = psInst->pfL2SetOpt( psInst->hL2,
                                                        IPGEA_OPTION_DW_BEACON_ID,
                                                        psInst->uParams.sIPGEA.bChangeBeaconID,
                                                        NULL );
                            if ( dwErr != NO_ERROR )
                                __leave;
                        }
                    }
					psInst->sGEAIPL7Api.bMessageType =  GEAIP_SEND_L2_FRAME_BST;
				}
				else 
                {
                    if ( dwReqCount == 0 )
                    {
                        // No content is given for the request
    				    psInst->sGEAIPL7Api.bMessageType = GEAIP_SEND_L2_FRAME_END;
                        dwLen = 0;
                    }
                    else
                    {
                        // A content is given for the request
    				    psInst->sGEAIPL7Api.bMessageType = GEAIP_SEND_L2_FRAME;
                        dwLen = sizeof( psInst->sQFSL7Api.tbParams );
                    }
                }


				psInst->sGEAIPL7Api.dwNumL2Bytes = dwPos;
                                                                                
                dwPos = 0;
                dwLen = sizeof( psInst->tbOut );
                pbData = psInst->tbOut;
                fResult = IPGEAL7_EncodeMessage(
                        psInst->hSL7,
                        pbData,
                        &dwPos,
                        &dwLen,
                        &psInst->sGEAIPL7Api );
                if ( ! fResult )
                {
                    dwErr = ERROR_INVALID_DATA;
                    __leave;
                }
				
                dwErr = psInst->pfL2SetOpt( psInst->hL2, 
                                            IPGEA_OPTION_F_LAST_MESSAGE, 
                                            eSession == TAG_SESSION_END ? 1 : 0,
                                            NULL );
					
			}
			else if (psInst->eInterface == TAG_INTERFACE_QFREE_DMI)
			{
				//TO DO Generate what to send to the read write thread
				// and put it to psInst->tbOut and psInst->dwOutBytes
				dwPos = 0;
				dwLen = sizeof(psInst->sQFDMIL7Api.tbL2Bytes);
				pbData = psInst->sQFDMIL7Api.tbL2Bytes;

				for (dwIndex = 0; dwIndex < dwReqCount; dwIndex++)
				{
					fResult = CENL7BuildApdu(
						psInst->hL7,
						pbData,
						&dwPos,
						&dwLen,
						&psReqTable[dwIndex]);
					if (!fResult)
					{
						dwErr = ERROR_INVALID_DATA;
						__leave;
					}
				}


				if (eSession == TAG_SESSION_BEGIN)
				{
					psInst->sQFDMIL7Api.bMessageType = QFDMI_SEND_L2_FRAME_BST;
				}
				else
				{
					if (dwReqCount == 0)
					{
						// No content is given for the request
						psInst->sQFDMIL7Api.bMessageType = QFDMI_SEND_L2_FRAME_END;
						dwLen = 0;
					}
					else
					{
						// A content is given for the request
						psInst->sQFDMIL7Api.bMessageType = QFDMI_SEND_L2_FRAME;
						dwLen = sizeof(psInst->sQFSL7Api.tbParams);
					}
				}


				psInst->sQFDMIL7Api.dwL2BytesLen = dwPos;

				dwPos = 0;
				dwLen = sizeof(psInst->tbOut);
				pbData = psInst->tbOut;
				fResult = QFDMI_BuildInternalMessage(
					psInst->hL2,
					pbData,
					&dwPos,
					&dwLen,
					&psInst->sQFDMIL7Api);
				if (!fResult)
				{
					dwErr = ERROR_INVALID_DATA;
					__leave;
				}

				dwErr = psInst->pfL2SetOpt(psInst->hL2,
					QFDMI_L2_OPTION_F_LAST_MESSAGE,
					eSession == TAG_SESSION_END ? 1 : 0,
					NULL);

			}
			else
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}
		}

		//
        // Mettre à jour la taille des données utiles dans le buffer d'émission et
        // envoyer le message.
        //
		if ( dwPos != 0 )
		{
			psInst->dwOutBytes = dwPos;
			dwErr = psInst->pfL2Send(
					psInst->hL2,
					psInst->tbOut,
					psInst->dwOutBytes,
					hEvent,
					pdwErr );

		}
		else
		{
			if ( hEvent != NULL )
			{
				if ( pdwErr != NULL )
					*pdwErr = NO_ERROR;

				if ( SetEvent( hEvent ) )
					dwErr = NO_ERROR;
				else
					dwErr = GetLastError();
			}
            else
                dwErr = NO_ERROR;
		}
    }
    __finally
    {
		;
    }

	return dwErr;
}




PUBLIC DWORD WINAPI TagResponseEx(
                IN              TAG_INSTANCE          * psInst,
                IN              CENL7_API             * psRspTable,
                IN OUT          DWORD                 * pdwReqCount,
                OUT             TAG_INTERFACE_STATUS  * peStatus,
                IN              DWORD                   dwTimeout,
                IN OUT          DWORD                 * pdwDataBytes,
                OUT             BYTE                  * pbDataBytes)
{
    DWORD                   dwErr = ERROR_INVALID_DATA;
    DWORD                   dwIndex;
    DWORD                   dwPos;
    DWORD                   dwMsgBytes;
    DWORD                   dwMaxReq;
    DWORD                   dwLen;
    BOOL                    fResult;
    BOOL                    fTruncate;
    TAG_INTERFACE_STATUS    eStatus;
    TRSP_ERROR_CODES        eTrspErr;
    BYTE                  * pbData;

    __try
    {
        pbData = NULL;
        dwIndex = 0;
        eStatus = TAG_STATUS_NONE;
        *peStatus = TAG_STATUS_NONE;
        dwMaxReq = ( (*pdwReqCount) & (~(TAG_TRUNCATE_RESPONSE)) );
        fTruncate = ( ( (*pdwReqCount) & TAG_TRUNCATE_RESPONSE ) != 0 );
        *pdwReqCount = 0;

        dwErr = psInst->pfL2Recv(
            psInst->hL2,
            &pbData,
            &dwMsgBytes,
            dwTimeout );
        if ( dwErr != NO_ERROR )
            __leave;
        
        if ( pbData == NULL )
        {
            dwErr = WAIT_TIMEOUT;
            __leave;
        }

        if ( ( psInst->eInterface == TAG_INTERFACE_WIRED   ) ||
             ( psInst->eInterface == TAG_INTERFACE_ISO7816 ) )
        {
            //
            // S'il s'agit d'un protocole sans sous-couche 7, on exploite directement
            // le message du buffer de reception.
            //
            dwLen = dwMsgBytes;
            dwPos = 0;

            while ( dwLen > 0 )
            {
                if ( dwIndex >= dwMaxReq )
                {
                    if ( fTruncate )
                        break;

                    dwErr = ERROR_BUFFER_OVERFLOW;
                    __leave;
                }

                fResult = CENL7AnalyseApdu(
                    psInst->hL7,
                    pbData,
                    &dwPos,
                    &dwLen,
                    &psRspTable[dwIndex] );
                if ( ! fResult )
                {
                    dwErr = ERROR_INVALID_DATA;
                    __leave;
                }

                dwIndex ++;
            }

            if ( ( pdwDataBytes != NULL ) && 
                 ( pbDataBytes  != NULL ) )
            {
                if ( *pdwDataBytes >= dwMsgBytes )
                {
                    memcpy( pbDataBytes, pbData, dwMsgBytes );
                    *pdwDataBytes = dwMsgBytes;
                }
                else
                {
                    dwErr = ERROR_BUFFER_OVERFLOW;
                    __leave;
                }
            }
        }

        else if ( ( psInst->eInterface == TAG_INTERFACE_COMMUNICATOR      ) ||
                  ( psInst->eInterface == TAG_INTERFACE_COMMUNICATOR_FAST ) )
        {
            //
            // S'il s'agit d'un protocole avec sous-couche 7, on exploite tout d'abord
            // le messages du buffer de réception pour en remplir la structure d'API de
            // la sous couche 7.
            //
            dwLen = dwMsgBytes;
            dwPos = 0;

            fResult = TRSPAnalyseMessage(
                    psInst->hSL7,
                    pbData,
                    &dwPos,
                    &dwLen,
                    &psInst->sSL7Api );
            if ( ! fResult )
            {
                dwErr = ERROR_INVALID_DATA;
                __leave;
            }

            if ( ( pdwDataBytes != NULL ) && 
                 ( pbDataBytes  != NULL ) )
            {
                if ( *pdwDataBytes >= sizeof(TRSP_API) )
                {
                    *(TRSP_API*)(pbDataBytes) = psInst->sSL7Api;
                    *pdwDataBytes = sizeof(TRSP_API);
                }
                else
                {
                    dwErr = ERROR_BUFFER_OVERFLOW;
                    __leave;
                }
            }

            if( psInst->sSL7Api.bActionType == TRSP_ACTION_SET_MODE )
            {
                //
                // Il s'agit d'une réponse à une demande de changement de mode 
                //
                eTrspErr = psInst->sSL7Api.sSetModeRsp.bError;
            }

            else if ( psInst->sSL7Api.bActionType == TRSP_ACTION_BEACON_OFF )
            {
                //
                // Il s'agit d'une réponse à une demande d'arret balise
                //
                eTrspErr = psInst->sSL7Api.sBeaconOffRsp.bError;
            }

            else if ( psInst->sSL7Api.bActionType == TRSP_ACTION_GET_STATUS )
            {
                //
                // Il s'agit d'une réponse à une demande d'état
                //
                eTrspErr = psInst->sSL7Api.sGetStatusRsp.bError;

                if ( psInst->sSL7Api.sGetStatusRsp.bMode != TRSP_MODE_TRANSPARENT )
                    eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_MODE;
                if ( psInst->sSL7Api.sGetStatusRsp.bTransacting != 0 )
                    eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_TRANSACTING;
            }

            else if ( ( psInst->sSL7Api.bActionType == TRSP_ACTION_TRSP_INI ) ||
                      ( psInst->sSL7Api.bActionType == TRSP_ACTION_TRSP_END ) ||
                      ( psInst->sSL7Api.bActionType == TRSP_ACTION_TRSP_CMD ) )
            {
                //
                // Il s'agit d'une réponse à une demande de transmission transparente L7
                //
                eTrspErr = psInst->sSL7Api.sTrspRsp.bError;

                if  ( eTrspErr == TRSP_ERROR_SUCCESS  )
                {
                    //
                    // On exploite le buffer de la sous-couche L7 pour décoder les réponses L7
                    //
                    dwLen = psInst->sSL7Api.sTrspRsp.dwRspSize;
                    dwPos = 0;

                    while ( dwLen > 0 )
                    {
                        if ( dwIndex >= dwMaxReq )
                        {
                            if ( fTruncate )
                                break;

                            dwErr = ERROR_BUFFER_OVERFLOW;
                            __leave;
                        }

                        fResult = CENL7AnalyseApdu(
                            psInst->hL7,
                            psInst->sSL7Api.sTrspRsp.tbRsp,
                            &dwPos,
                            &dwLen,
                            &psRspTable[dwIndex] );
                        if ( ! fResult )
                        {
                            dwErr = ERROR_INVALID_DATA;
                            __leave;
                        }

                        dwIndex ++;
                    }
                }
            }
            else
            {
                //
                // Il s'agit d'une réponse inconnue
                //
                eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_UNKNOWN;
                eTrspErr = TRSP_ERROR_SUCCESS;
            }

            //
            // Effectuer la translation des erreurs du mode
            // transparent vers les bits de status.
            //
            if  ( eTrspErr != TRSP_ERROR_SUCCESS  )
            {
                eStatus |= TAG_STATUS_ERROR;

                switch ( eTrspErr )
                {
                case TRSP_ERROR_REFUSED :
                    eStatus |= TAG_STATUS_ERR_REFUSED;
                    break;

                case TRSP_ERROR_TRANSACT :
                    eStatus |= TAG_STATUS_ERR_TRANSACTING;
                    break;

                case TRSP_ERROR_BEACON :
                    eStatus |= TAG_STATUS_ERR_HARDWARE;
                    break;

                case TRSP_ERROR_TIMEOUT :
                    eStatus |= TAG_STATUS_ERR_TIMEOUT;
                    break;

                case TRSP_ERROR_RESET :
                    eStatus |= TAG_STATUS_ERR_RESET;
                    break;

                case TRSP_ERROR_INVALID_PARAM :
                    eStatus |= TAG_STATUS_ERR_PARAM;
                    break;

                case TRSP_ERROR_CONFIG_FILE :
                    eStatus |= TAG_STATUS_ERR_CONFIG;
                    break;

                default :
                    eStatus |= TAG_STATUS_ERR_UNKNOWN;
                    break;
                }
            }
        }
        else if ( psInst->eInterface == TAG_INTERFACE_QFREE) 
        {
            //
            dwLen = dwMsgBytes;
            dwPos = 0;

            fResult = QFSL7AnalyseMessage(
                    psInst->hSL7,
                    pbData,
                    &dwPos,
                    &dwLen,
                    &psInst->sQFSL7Api );
            if ( ! fResult )
            {
                dwErr = ERROR_INVALID_DATA;
                __leave;
            }

            if ( ( pdwDataBytes != NULL ) && 
                 ( pbDataBytes  != NULL ) )
            {
                if ( *pdwDataBytes >= sizeof(QFSL7_API) )
                {
                    *(QFSL7_API*)(pbDataBytes) = psInst->sQFSL7Api;
                    *pdwDataBytes = sizeof(QFSL7_API);
                }
                else
                {
                    dwErr = ERROR_BUFFER_OVERFLOW;
                    __leave;
                }
            }

			if ( ( psInst->sQFSL7Api.bMessageType == QFSL7_ACTION_QFSL7_QUERY  ) ||
                 ( psInst->sQFSL7Api.bMessageType == QFSL7_ACTION_QFSL7_STOP   ) ||
                 ( psInst->sQFSL7Api.bMessageType == QFSL7_ACTION_QFSL7_CONFIG ) )
            {
                if ( strncmp( "OK", psInst->sQFSL7Api.tbParams, psInst->sQFSL7Api.dwParamBytes ) != 0 )
                {
                    if ( strncmp( "NOT OK", psInst->sQFSL7Api.tbParams, psInst->sQFSL7Api.dwParamBytes ) == 0 )
                        eStatus = TAG_STATUS_ERROR | TAG_STATUS_ERR_HARDWARE;
                    else
                        eStatus = TAG_STATUS_ERROR | TAG_STATUS_ERR_UNKNOWN;
                }
            }
			else if ( psInst->sQFSL7Api.bMessageType == QFSL7_ACTION_QFSL7_TIMEOUT )
            {
                eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_TIMEOUT;
            }

			else if	(psInst->sQFSL7Api.bMessageType == QFSL7_ACTION_QFSL7_ERROR )
            {
                eStatus |= TAG_STATUS_ERROR;

                if ( strncmp( "NoOBUorNoMRconnected", psInst->sQFSL7Api.tbParams, psInst->sQFSL7Api.dwParamBytes ) == 0 )
                    eStatus |= TAG_STATUS_ERR_TIMEOUT;
                else if ( strncmp( "COMportNotOpen", psInst->sQFSL7Api.tbParams, psInst->sQFSL7Api.dwParamBytes ) == 0 )
                    eStatus |= TAG_STATUS_ERR_HARDWARE;
                else if ( strncmp( "InvalidDataFromL7App", psInst->sQFSL7Api.tbParams, psInst->sQFSL7Api.dwParamBytes ) == 0 )
                    eStatus |= TAG_STATUS_ERR_HARDWARE;
                else
                    eStatus |= TAG_STATUS_ERR_UNKNOWN;
            }

			else if ( psInst->sQFSL7Api.bMessageType == QFSL7_ACTION_QFSL7_FRAME ) 
            {
                //
                // On exploite le buffer de la sous-couche L7 pour décoder les réponses L7
                //
                dwLen = psInst->sQFSL7Api.dwParamBytes;
                dwPos = 0;

                while ( dwLen > 0 )
                {
                    if ( dwIndex >= dwMaxReq )
                    {
                        if ( fTruncate )
                            break;

                        dwErr = ERROR_BUFFER_OVERFLOW;
                        __leave;
                    }

                    fResult = CENL7AnalyseApdu(
                        psInst->hL7,
                        psInst->sQFSL7Api.tbParams,
                        &dwPos,
                        &dwLen,
                        &psRspTable[dwIndex] );
                    if ( ! fResult )
                    {
                        dwErr = ERROR_INVALID_DATA;
                        __leave;
                    }

                    dwIndex ++;
                }
            }

            else if	(psInst->sQFSL7Api.bMessageType == QFSL7_ACTION_QFSL7_CONFIG )
            {
                //
                // Even if we use config command, we use them at lower level only so this
                // layer should not see config responses
                //
                eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_UNKNOWN;
            }

            else
            {
                //
                // Il s'agit d'une réponse inconnue
                //
                eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_UNKNOWN;
            }
        }
        else if ( psInst->eInterface == TAG_INTERFACE_IPGEA) 
        {
            dwLen = dwMsgBytes;
            dwPos = 0;

            fResult = IPGEAL7_DecodeMessage(
                    psInst->hSL7,
                    pbData,
                    &dwPos,
                    &dwLen,
                    &psInst->sGEAIPL7Api);
            if ( ! fResult )
            {
                dwErr = ERROR_INVALID_DATA;
                __leave;
            }

            if ( ( pdwDataBytes != NULL ) && 
                 ( pbDataBytes  != NULL ) )
            {
                if ( *pdwDataBytes >= sizeof(IPGEAL7_API) )
                {
                    *(IPGEAL7_API*)(pbDataBytes) = psInst->sGEAIPL7Api;
                    *pdwDataBytes = sizeof(IPGEAL7_API);
                }
                else
                {
                    dwErr = ERROR_BUFFER_OVERFLOW;
                    __leave;
                }
            }

			if( psInst->sGEAIPL7Api.bMessageType == GEAIP_INTF_REQ_GETSTATUS )
            {
				if(psInst->sGEAIPL7Api.dwNumL2Bytes == 3)
				{	
					BYTE state, mode, trxInProgress;

					state = psInst->sGEAIPL7Api.tbL2Bytes[0];
					mode = psInst->sGEAIPL7Api.tbL2Bytes[1];
					trxInProgress = psInst->sGEAIPL7Api.tbL2Bytes[2];
					
					if(state != 0)
						eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_HARDWARE;
					else 
					{
						if(mode != 1)	// BCM_MOD_Transparent
							eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_MODE;
						if(trxInProgress == 1)
							eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_TRANSACTING;
					}
				}
				else
					eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_UNKNOWN;
            }
			else if(psInst->sGEAIPL7Api.bMessageType == GEAIP_INTF_REQ_MODE_TRANSPARENT)
            {
				if(psInst->sGEAIPL7Api.dwNumL2Bytes == 3)
				{	
					BYTE state, mode, trxInProgress;
					
					state = psInst->sGEAIPL7Api.tbL2Bytes[0];
					mode = psInst->sGEAIPL7Api.tbL2Bytes[1];
					trxInProgress = psInst->sGEAIPL7Api.tbL2Bytes[2];
					
					if(state != 0)
						eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_HARDWARE;
					
					if(trxInProgress == 1)
						eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_TRANSACTING;

					if(mode != 1)	// BCM_MOD_Transparent
						eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_MODE;
				}
				else
					eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_UNKNOWN;
            }
			else if(psInst->sGEAIPL7Api.bMessageType == GEAIP_L2_TIMEOUT)
			{
				eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_TIMEOUT;
			}
			else if ( psInst->sGEAIPL7Api.bMessageType == GEAIP_SEND_L2_FRAME ) 
            {
                // On exploite le buffer de la sous-couche L7 pour décoder les réponses L7
                dwLen = psInst->sGEAIPL7Api.dwNumL2Bytes;
                dwPos = 0;

                while ( dwLen > 0 )
                {
                    if ( dwIndex >= dwMaxReq )
                    {
                        if ( fTruncate )
                            break;

                        dwErr = ERROR_BUFFER_OVERFLOW;
                        __leave;
                    }

                    fResult = CENL7AnalyseApdu(
                        psInst->hL7,
                        psInst->sGEAIPL7Api.tbL2Bytes,
                        &dwPos,
                        &dwLen,
                        &psRspTable[dwIndex] );
                    if ( ! fResult )
                    {
                        dwErr = ERROR_INVALID_DATA;
                        __leave;
                    }

                    dwIndex ++;
                }
            }
            else
            {
                // In case of response unknown 
                eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_UNKNOWN;
            }
        }
		else if (psInst->eInterface == TAG_INTERFACE_QFREE_DMI)
		{
			dwLen = dwMsgBytes;
			dwPos = 0;

			fResult = QFDMI_DecodeInternalMessage(
				psInst->hL2,
				pbData,
				&dwPos,
				&dwLen,
				&psInst->sQFDMIL7Api);
			if (!fResult)
			{
				dwErr = ERROR_INVALID_DATA;
				__leave;
			}

			if ((pdwDataBytes != NULL) &&
				(pbDataBytes != NULL))
			{
				if (*pdwDataBytes >= sizeof(QFDMI_INTERNAL_MSG))
				{
					*(QFDMI_INTERNAL_MSG *)(pbDataBytes) = psInst->sQFDMIL7Api;
					*pdwDataBytes = sizeof(QFDMI_INTERNAL_MSG);
				}
				else
				{
					dwErr = ERROR_BUFFER_OVERFLOW;
					__leave;
				}
			}
			if (psInst->sQFDMIL7Api.bMessageType == QFDMI_INTF_REQ_GETSTATUS || 
				psInst->sQFDMIL7Api.bMessageType == QFDMI_INTF_REQ_MODE_TRANSPARENT)
			{
				if (psInst->sQFDMIL7Api.dwL2BytesLen == 1)
				{
					BYTE bCommError, mode, trxInProgress;

					bCommError = psInst->sQFDMIL7Api.tbL2Bytes[0]; //QFDMI_isCommunicationOK
					//mode = psInst->sQFDMIL7Api.tbL2Bytes[1];
					//trxInProgress = psInst->sQFDMIL7Api.tbL2Bytes[2];

					if (bCommError != 0)
						eStatus |= TAG_STATUS_ERROR ;//| TAG_STATUS_ERR_HARDWARE
					//else
					//{
					//	if (mode != 1)	// BCM_MOD_Transparent
					//		eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_MODE;
					//	if (trxInProgress == 1)
					//		eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_TRANSACTING;
					//}
				}
				else
					eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_UNKNOWN;
			}
			else if (psInst->sQFDMIL7Api.bMessageType == QFDMI_L2_TIMEOUT)
			{
				eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_TIMEOUT;
			}
			else if (psInst->sQFDMIL7Api.bMessageType == QFDMI_SEND_L2_FRAME)
			{
				// On exploite le buffer de la sous-couche L7 pour décoder les réponses L7
				dwLen = psInst->sQFDMIL7Api.dwL2BytesLen;
				dwPos = 0;

				while (dwLen > 0)
				{
					if (dwIndex >= dwMaxReq)
					{
						if (fTruncate)
							break;

						dwErr = ERROR_BUFFER_OVERFLOW;
						__leave;
					}

					fResult = CENL7AnalyseApdu(
						psInst->hL7,
						psInst->sQFDMIL7Api.tbL2Bytes,
						&dwPos,
						&dwLen,
						&psRspTable[dwIndex]);
					if (!fResult)
					{
						dwErr = ERROR_INVALID_DATA;
						__leave;
					}

					dwIndex++;
				}
			}
			else
			{
				// In case of response unknown 
				eStatus |= TAG_STATUS_ERROR | TAG_STATUS_ERR_UNKNOWN;
			}

		}
		

        if ( dwIndex > 0 )
            eStatus |= TAG_STATUS_SESSION_DATA;

        *peStatus = eStatus;
        *pdwReqCount = dwIndex;

        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( pbData != NULL )
            psInst->pfL2Free( pbData );

  //      	TRACE_CONSOLE2( "\nTagResponseEx...Returns:%d, Status:%x\n", dwErr, eStatus);

        
    }

	return dwErr;
}





PUBLIC DWORD WINAPI TagResponse(
                IN              TAG_INSTANCE          * psInst,
                IN              CENL7_API             * psRspTable,
                IN OUT          DWORD                 * pdwReqCount,
                OUT             TAG_INTERFACE_STATUS  * peStatus,
                IN              DWORD                   dwTimeout )
{
    return TagResponseEx( psInst, psRspTable, pdwReqCount, peStatus, dwTimeout, NULL, NULL );
}



void FichierTrace(TRC_EMETTEUR emetTrace, char *szString)
{
	//va_list args;
	//char szString[1024] = {0};

	//va_start(args, szFmt);
	//_vsnprintf_s(szString, sizeof(szString), sizeof(szString), szFmt, args);

	TRC_Trace_V(emetTrace, TRC_OPT_MASK, NULL, 0, szString, NULL);

	//va_end(args);
}

PUBLIC void * WINAPI TagHookOpenDefaultContext( IN  TAG_INSTANCE          * psInst, 
                                                OPTIONAL IN HWND            hWnd,
                                                OPTIONAL IN DWORD           dwThreadId,
                                                OPTIONAL IN TAG_HOOK_LOG  * psLog )
{
    DEFAULT_HOOK_CONTEXT * psCtx = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;

    __try
    {
        psCtx = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psCtx) );
        if ( psCtx == NULL )
            __leave;

        if ( ! QueryPerformanceFrequency( (LARGE_INTEGER*)&psCtx->llFreq ) )
            psCtx->llFreq = 0;

		psCtx->hFile = INVALID_HANDLE_VALUE;

		if(psInst->dwConsoleSaveToFile &&
			strlen(psInst->szConsoleFilePath)!=0) 
		{
			//hFile = CreateFile(psInst->szConsoleFilePath,
			//	GENERIC_WRITE|FILE_SHARE_READ, 
			//	FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			//	NULL,
			//	CREATE_ALWAYS,
			//	FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, 
			//	NULL);


			if (TRC_Initialise_Trace("TRACE", psInst->szConsoleFilePath, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, (TRC_EMETTEUR *)&hFile) == ERROR_SUCCESS)
			{
				if (TRC_Taille_Max_Fichier((TRC_EMETTEUR)hFile, 1000000) != ERROR_SUCCESS)
				{
					TRC_Termine_Trace((TRC_EMETTEUR)hFile);
					hFile = INVALID_HANDLE_VALUE;
				}
			}
			else
				hFile = INVALID_HANDLE_VALUE;

		}
			
		if(hFile == INVALID_HANDLE_VALUE)
		{
			AllocConsole();
			hFile = GetStdHandle( STD_OUTPUT_HANDLE );
			psCtx->bIsFile = FALSE;
			psCtx->hFile = hFile;
		}
		else
		{
			psCtx->bIsFile = TRUE;
			psCtx->hFile = hFile;

		}
  		
        psCtx->hWnd = hWnd;
        psCtx->dwThreadId = dwThreadId;
        psCtx->psLog = psLog;
		psCtx->psInst = psInst;
    }
    __finally
    {
		;
    }

	return psCtx;
}



PUBLIC void WINAPI TagHookCloseDefaultContext( void * pvHookContext )
{
	DEFAULT_HOOK_CONTEXT * psCtx = (DEFAULT_HOOK_CONTEXT *)pvHookContext;

	if(psCtx->hFile != INVALID_HANDLE_VALUE)
	{
		if(psCtx->bIsFile == FALSE)
			FreeConsole();
		else
		{
			//CloseHandle(psCtx->hFile);
			TRC_Termine_Trace((TRC_EMETTEUR)psCtx->hFile);
			psCtx->hFile = INVALID_HANDLE_VALUE;
		}
	}

    HeapFree( GetProcessHeap(), 0, pvHookContext );
}



PUBLIC UINT WINAPI TagHookGetDefaultWM()
{
    return RegisterWindowMessage( "TAG_HOOK_MSG" );
}



PUBLIC TAG_HOOK * WINAPI TagHookGetDefaultHook()
{
    return &TagDefaultHook;
}



PRIVATE void WINAPI TagDefaultHook( IN void * pvHookContext, 
                                      IN DWORD  dwTypeDir, 
                                      IN BYTE * pbData, 
                                      IN DWORD dwBytes )
{
    DWORD   dwDone;
    DWORD   dwIndex;
    int     iPos;
    int     iLen;
    int     iRemain;
    LPARAM  lParam;
    WPARAM  wParam;
	char    szText[TAG_HOOK_MAX_BYTES_SIZE] = { 0 };
	char    szPrefix[256] = { 0 };
    LONGLONG llTime;
    DEFAULT_HOOK_CONTEXT * psCtx = pvHookContext;

	if (psCtx->hFile != INVALID_HANDLE_VALUE)
    {
        if ( psCtx->llFreq == 0 )
            llTime = GetTickCount();
        else
        {
            QueryPerformanceCounter( (LARGE_INTEGER*)&llTime );
            llTime = ( llTime * (LONGLONG)1000 ) / psCtx->llFreq;
        }

		if(( dwTypeDir & TAG_HOOK_COMMENT))
		{
			int iSize = (dwBytes < sizeof(szText)-1) ? dwBytes : sizeof(szText) - 1;

			//TIC added to be able to send a comment from tag code
			strncpy_s(szText, sizeof(szText), pbData, _TRUNCATE);
			
			szText[iSize] = '\0';
			iPos = iSize;
		}
		else
		{
			BOOL bDisplayAllToHex = (dwTypeDir & TAG_HOOK_DATA) || (dwTypeDir & TAG_HOOK_PROTOCOL);

			_snprintf_s(szPrefix, sizeof(szPrefix), sizeof(szPrefix)-1, "%s %s :",
				( ( dwTypeDir & TAG_HOOK_INPUT    ) != 0 ) ? "IN "  : "OUT",
				( ( dwTypeDir & TAG_HOOK_PROTOCOL ) != 0 ) ? "SYST" : "DATA" );

			iPos = _snprintf_s(szText, sizeof(szText), sizeof(szText)-1, "\r\n%I64u %s",
				llTime, szPrefix);

			dwIndex = 0;
			iRemain = sizeof(szText) - iPos;

			while ( ( iPos < sizeof(szText) ) && ( dwIndex < dwBytes ) )
			{
				if (bDisplayAllToHex)
					iLen = _snprintf_s(szText + iPos, (sizeof(szText) - iPos), iRemain, "%02X", (DWORD)pbData[dwIndex++]);
				else
				{
					if (isprint((DWORD)pbData[dwIndex]))
						iLen = _snprintf_s(szText + iPos, (sizeof(szText) - iPos), iRemain, "%c", (DWORD)pbData[dwIndex++]);
					else
						iLen = _snprintf_s(szText + iPos, (sizeof(szText) - iPos), iRemain, " %02X", (DWORD)pbData[dwIndex++]);

				}


				if ( iLen < 0 )
				{
					iPos = sizeof(szText);
					iRemain = 0;
					_snprintf_s(szText + (sizeof(szText) - 4), (sizeof(szText) - 4), 4, " ...");
					break;
				}

				iPos += iLen;
				iRemain -= iLen;
			}
		}

		if (!psCtx->bIsFile) //trace to console
		{
			WriteFile(psCtx->hFile, szText, iPos, &dwDone, NULL);
		}
		else
		{
			
			FichierTrace((TRC_EMETTEUR)psCtx->hFile, szText);
		}
		

    }

    if ( psCtx->hWnd != NULL )
    {
        lParam = (LPARAM)HeapAlloc( GetProcessHeap(), 0, dwBytes );
        if ( lParam != 0 )
        {
            CopyMemory( (void*)lParam, pbData, dwBytes );
            wParam = ( dwBytes & 0x00FFFFFF ) | ( dwTypeDir & 0xFF000000 );

            PostMessage( psCtx->hWnd, RegisterWindowMessage( "TAG_HOOK_MSG" ), wParam, lParam );
        }
    }

    if ( psCtx->dwThreadId != 0 )
    {
        lParam = (LPARAM)HeapAlloc( GetProcessHeap(), 0, dwBytes );
        if ( lParam != 0 )
        {
            CopyMemory( (void*)lParam, pbData, dwBytes );
            wParam = ( dwBytes & 0x00FFFFFF ) | ( dwTypeDir & 0xFF000000 );

            PostThreadMessage( psCtx->dwThreadId, RegisterWindowMessage( "TAG_HOOK_MSG" ), wParam, lParam );
        }
    }

    if ( psCtx->psLog != NULL )
        TagHookLogPushData( psCtx->psLog, ( dwBytes & 0x00FFFFFF ) | ( dwTypeDir & 0xFF000000 ), pbData );
}



PUBLIC TAG_HOOK_LOG * WINAPI TagHookLogOpen()
{
    TAG_HOOK_LOG * psLog;

    psLog = HeapAlloc( GetProcessHeap(), 
                       HEAP_ZERO_MEMORY,
                       sizeof( TAG_HOOK_LOG ) );
    if ( psLog != NULL )
        InitializeCriticalSection( &psLog->sProtect );

    return psLog;
}




PRIVATE BOOL WINAPI TagHookLogPushData( IN TAG_HOOK_LOG * psLog,
                                       IN DWORD          dwBytes,
                                       IN BYTE         * pbData )
{
    TAG_HOOK_LOG_ITEM * psItem;

    psItem = HeapAlloc( GetProcessHeap(), 0, sizeof(*psItem) + ( dwBytes & 0x00FFFFFF ) );
    if ( psItem != NULL )
    {
        psItem->dwBytes = dwBytes;
        CopyMemory( psItem->tbData, pbData, dwBytes & 0x00FFFFFF );

        EnterCriticalSection( &psLog->sProtect );

        if ( ! psLog->fSuspended )
        {
            if ( psLog->psFirst == NULL )
            {
                psItem->psNext = psItem;
                psItem->psPrev = psItem;
                psLog->psFirst = psItem;
            }
            else
            {
                psItem->psNext = psLog->psFirst;
                psItem->psPrev = psLog->psFirst->psPrev;
                psLog->psFirst->psPrev->psNext = psItem;
                psLog->psFirst->psPrev = psItem;
                psLog->psFirst = psItem;
            }
        }

        LeaveCriticalSection( &psLog->sProtect );
    }

    return ( psItem != NULL );
}



PUBLIC BOOL WINAPI TagHookLogPullData( IN     TAG_HOOK_LOG * psLog,
                                       IN OUT DWORD        * pdwBytes,
                                       OUT    BYTE         * pbData )
{
    TAG_HOOK_LOG_ITEM * psItem;
    BOOL                fResult;

    EnterCriticalSection( &psLog->sProtect );
    
    if ( psLog->psFirst != NULL )
    {
        psItem = psLog->psFirst->psPrev;

        if ( pdwBytes != NULL )
        {
            if ( ( psItem->dwBytes & 0x00FFFFFF ) <= (*pdwBytes) )
            {
                CopyMemory( pbData, psItem->tbData, psItem->dwBytes & 0x00FFFFFF );
                *pdwBytes = psItem->dwBytes;
                fResult = TRUE;
            }
            else
            {
                fResult = FALSE;
                *pdwBytes = psItem->dwBytes & 0x00FFFFFF;
            }
        }
        else 
            fResult = TRUE;

        if ( fResult )
        {
            if ( psItem == psLog->psFirst )
                psLog->psFirst = NULL;
            else
            {
                psItem->psPrev->psNext = psItem->psNext;
                psItem->psNext->psPrev = psItem->psPrev;
            }

            HeapFree( GetProcessHeap(), 0, psItem );
        }
    }
    else
    {
        if ( pdwBytes != NULL )
            *pdwBytes = 0;
        fResult = FALSE;
    }

    LeaveCriticalSection( &psLog->sProtect );

    return fResult;
}


PUBLIC void WINAPI TagHookLogClear( IN TAG_HOOK_LOG * psLog )
{
    EnterCriticalSection( &psLog->sProtect );

    while ( TagHookLogPullData( psLog, NULL, NULL ) );

    LeaveCriticalSection( &psLog->sProtect );
}



PUBLIC void WINAPI TagHookLogSuspend( IN TAG_HOOK_LOG * psLog,
                                      IN BOOL           fSuspended )
{
    EnterCriticalSection( &psLog->sProtect );
    psLog->fSuspended = fSuspended;
    LeaveCriticalSection( &psLog->sProtect );
}




PUBLIC void WINAPI TagHookLogClose( IN TAG_HOOK_LOG * psLog )
{
    TagHookLogClear( psLog );

    DeleteCriticalSection( &psLog->sProtect );
    HeapFree( GetProcessHeap(), 0, psLog );
}



