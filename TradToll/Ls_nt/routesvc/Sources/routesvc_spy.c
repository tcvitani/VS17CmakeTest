/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_spy.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des espions de com
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

#include <acom.h>
#include <ntsvc.h>
#include <col.h>
#include <csr_list.h>

#include <routesvc_glob.h>
#include <routesvc_text.h>

#define LOC_DEF
#include <routesvc_spy.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI SpyCnxAdd( DWORD64 dwCmdIndex, BOOL bState )
 * PARAMETERS: dwCmdIndex : Index de la connexion de commande demandant l'activation de l'espion
 *             bState     : TRUE si actif, FALSE si inactif
 * RETURN    : NO_ERROR si succès
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Met en place un espion auditant les connexions / déconnexions
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI SpyCnxAdd(DWORD64 dwCmdIndex, BOOL bState)
{
    DWORD dwErr = NO_ERROR;     // Code d'erreur à renvoyer

    // Vérifier le bon type de la connexion demandeuse
    if ( ( gsSvcWork.psList[dwCmdIndex].dwType == ROUTESVC_PIPE_CMD_TYPE ) && 
         ( gsSvcWork.psList[dwCmdIndex].bBusy ) )
        gsSvcWork.psList[dwCmdIndex].uCnx.sCmd.bSpyCnx = bState;
    else
        dwErr = ERROR_INVALID_PARAMETER;

    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI SpyAdd( DWORD64 dwCmdIndex, DWORD dwSpyIndex )
 * PARAMETERS: dwCmdIndex : Index de la connexion de commande demandant l'activation de l'espion
 *             dwSpyIndex : Index de la connexion à espionner
 * RETURN    : NO_ERROR si succès
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Met en place un espion sur une connexion quelconque, pour le compte d'un
 *             connexion de type interface de commande.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI SpyAdd(DWORD64 dwCmdIndex, DWORD dwSpyIndex)
{
    DWORD dwErr = NO_ERROR;     // Code d'erreur à renvoyer
    ROUTESVC_CMD * psCmd;       // Pointer sur les données de la connexion interface de commande
    DWORD dwNewCount;           // Pour le redimensionnement de la liste des espions
    DWORD * pdwNewList;         // Liste redimensionnées

    // Vérifier le bon type de la connexion demandeuse
    if ( ( gsSvcWork.psList[dwCmdIndex].dwType == ROUTESVC_PIPE_CMD_TYPE ) && 
         ( gsSvcWork.psList[dwCmdIndex].bBusy ) &&
         ( dwCmdIndex != dwSpyIndex ) )
    {
        // Eviter de dupliquer les espions
        if ( ! SpyIsActive( dwCmdIndex, dwSpyIndex ) )
        {
            psCmd = &gsSvcWork.psList[dwCmdIndex].uCnx.sCmd;

            if ( psCmd->pdwSpyList == NULL )
            {
                dwNewCount = 1;
                pdwNewList = HeapAlloc( GetProcessHeap(), 0, dwNewCount * sizeof( *pdwNewList ) );
            }
            else
            {
                 dwNewCount = psCmd->dwSpyCount + 1;
                 pdwNewList = HeapReAlloc( GetProcessHeap(), 0, psCmd->pdwSpyList, dwNewCount * sizeof( *pdwNewList ) );
            }
            if ( pdwNewList == NULL )
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                psCmd->pdwSpyList = pdwNewList;
                psCmd->dwSpyCount = dwNewCount;
                psCmd->pdwSpyList[ dwNewCount - 1 ] = dwSpyIndex;
            }
        }
    }
    else
        dwErr = ERROR_INVALID_PARAMETER;

    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL WINAPI SpyIsActive( DWORD64 dwCmdIndex, DWORD64 dwSpyIndex )
 * PARAMETERS: dwCmdIndex : Index de la connexion de commande demandant l'activation de l'espion
 *             dwSpyIndex : Index de la connexion à espionner
 * RETURN    : TRUE si un espion est activé sur la connexion d'index dwSpyIndex pour 
 *             la connexion d'index dwCmdIndex.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Teste la présence d'un espion
 * --------------------------------------------------------------------
 */
PROTECTED BOOL WINAPI SpyIsActive(DWORD64 dwCmdIndex, DWORD64 dwSpyIndex)
{
    DWORD bSpying = FALSE;      // Par défaut, il n'y a pas d'espion
    ROUTESVC_CMD * psCmd;       // Pointer sur les données de la connexion interface de commande
    DWORD dwScan;               // Pour scanner la liste des espions

    if ( ( gsSvcWork.psList[dwCmdIndex].dwType == ROUTESVC_PIPE_CMD_TYPE ) && ( gsSvcWork.psList[dwCmdIndex].bBusy ) )
    {
        psCmd = &gsSvcWork.psList[dwCmdIndex].uCnx.sCmd;

        if ( psCmd->pdwSpyList != NULL )
        {
            for ( dwScan = 0 ; dwScan < psCmd->dwSpyCount ; dwScan ++ )
                if ( psCmd->pdwSpyList[dwScan] == dwSpyIndex )
                {
                    bSpying = TRUE;
                    break;
                }
        }
    }

    return bSpying;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL WINAPI SpyCnxIsActive( DWORD dwCmdIndex )
 * PARAMETERS: dwCmdIndex : Index de la connexion de commande demandant l'activation de l'espion
 * RETURN    : TRUE si un espion est activé sur les connexions / déconnexions
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Teste la présence d'un espion
 * --------------------------------------------------------------------
 */
PROTECTED BOOL WINAPI SpyCnxIsActive( DWORD dwCmdIndex )
{
    if ( ( gsSvcWork.psList[dwCmdIndex].dwType == ROUTESVC_PIPE_CMD_TYPE ) && ( gsSvcWork.psList[dwCmdIndex].bBusy ) )
        return gsSvcWork.psList[dwCmdIndex].uCnx.sCmd.bSpyCnx;
    else
        return FALSE;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI SpyRemove( DWORD64 dwCmdIndex, DWORD dwSpyIndex )
 * PARAMETERS: dwCmdIndex : Index de la connexion de commande demandant l'arret de l'espion
 *             dwSpyIndex : Index de la connexion espionnée
 * RETURN    : NO_ERROR si espion enlevé
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Supprime un espion
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI SpyRemove(DWORD64 dwCmdIndex, DWORD dwSpyIndex)
{
    DWORD dwErr = NO_ERROR;     // Code d'erreur à renvoyer
    ROUTESVC_CMD * psCmd;       // Pointer sur les données de la connexion interface de commande
    DWORD dwScan;               // Pour scanner la liste des espions

    if ( ( gsSvcWork.psList[dwCmdIndex].dwType == ROUTESVC_PIPE_CMD_TYPE ) && ( gsSvcWork.psList[dwCmdIndex].bBusy ) )
    {
        psCmd = &gsSvcWork.psList[dwCmdIndex].uCnx.sCmd;

        if ( psCmd->pdwSpyList != NULL )
        {
            for ( dwScan = 0 ; dwScan < psCmd->dwSpyCount ; dwScan ++ )
                if ( psCmd->pdwSpyList[dwScan] == dwSpyIndex )
                {
                    if ( psCmd->dwSpyCount == 1 )
                    {
                        HeapFree( GetProcessHeap(), 0, psCmd->pdwSpyList );
                        psCmd->pdwSpyList = NULL;
                        psCmd->dwSpyCount = 0;
                    }
                    else
                    {
                        psCmd->dwSpyCount --;
                        psCmd->pdwSpyList[dwScan] = psCmd->pdwSpyList[psCmd->dwSpyCount];
                        // Pas de HeapRealloc : inutile
                    }
                    break;
                }
        }

    }

    return NO_ERROR;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void SpySendMessage( DWORD64 dwIndex, char * pcText, DWORD dwMsgSize, void * pvMsg )
 * PARAMETERS: dwIndex   : Index de la connexion sur laquelle un message est passé
 *             pcText    : Text d'entête du message d'espionnage
 *             dwMsgSize : Taille du message
 *             pvMsg     : Pointe sur le message
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Soumet un message à l'espionnage
 * --------------------------------------------------------------------
 */
PROTECTED void SpySendMessage(DWORD64 dwIndex, char * pcText, DWORD dwMsgSize, void * pvMsg)
{
	DWORD64 dwCmdIndex;           // Pour scanner les interfaces de commande connectées
    DWORD dwDumpSize;           // Taille du dump texte des données du message
    DWORD dwScan;               // Pour scanner les octets du message
    DWORD dwStart;              // Taille de l'entête du message d'espionnage
    BOOL bSend;                 // Flag d'activation d'un espion
    char cScan;                 // Caractère scanné courrant du message
    char * pcDump = NULL;       // Pointe sur le buffer recevant le dump texte des données du message
    ACOM_CNX_HANDLE hCnx;       // Handle des connexion d'interface de commande

    for ( dwCmdIndex = gsSvcWork.dwCmdStart ; dwCmdIndex <= gsSvcWork.dwCmdEnd ; dwCmdIndex ++ )
    {
        EnterCriticalSection( &gsSvcWork.sCritical );
        bSend = SpyIsActive( dwCmdIndex, dwIndex );
        hCnx = gsSvcWork.psList[dwCmdIndex].hCnx;
        LeaveCriticalSection( &gsSvcWork.sCritical );
 
        if ( bSend )
        {
            if ( pcDump == NULL )
            {
                dwDumpSize = (DWORD)strlen( pcText ) + dwMsgSize + 32;
                pcDump = HeapAlloc( GetProcessHeap(), 0, dwDumpSize );
                if ( pcDump == NULL )
                {
                    AComDisconnectPeer( gsSvcWork.hWks, hCnx, FALSE );
                    break;
                }
                dwStart = sprintf_s( pcDump, dwDumpSize, "%s %u,%u,[", pcText, dwIndex, dwMsgSize );
                
                for ( dwScan = 0 ; dwScan < dwMsgSize ; dwScan ++ )
                {
                    cScan = ((BYTE*)pvMsg)[dwScan];
                    pcDump[dwScan+dwStart] = ( cScan >= ' ' ? cScan : '.' );
                }
                pcDump[dwScan+dwStart] = ']';
                dwScan ++;

            }
            AComSendMessage( gsSvcWork.hWks, hCnx, 0, dwScan + dwStart, pcDump );
        }   
    }
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void SpyCnx( DWORD dwIndex, BOOL bState )
 * PARAMETERS: dwIndex   : Index de la connexion
 *             bState    : TRUE = connexion, FALSE = déconnexion
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Soumet une connexion / déconnexion message à l'espionnage
 * --------------------------------------------------------------------
 */
PROTECTED void SpyCnx(DWORD64 dwIndex, BOOL bState)
{
    DWORD dwCmdIndex;           // Pour scanner les interfaces de commande connectées
    DWORD dwSize;               // Taille du message
    BOOL bSend;                 // Flag d'activation d'un espion
    BOOL bFirst = TRUE;         // Premier envoi
    ACOM_CNX_HANDLE hCnx;       // Handle des connexion d'interface de commande
    char szMsg[128];

    for ( dwCmdIndex = gsSvcWork.dwCmdStart ; dwCmdIndex <= gsSvcWork.dwCmdEnd ; dwCmdIndex ++ )
    {
        EnterCriticalSection( &gsSvcWork.sCritical );
        bSend = SpyCnxIsActive( dwCmdIndex ) && ( dwCmdIndex != dwIndex );
        hCnx = gsSvcWork.psList[dwCmdIndex].hCnx;
        LeaveCriticalSection( &gsSvcWork.sCritical );
 
        if ( bSend )
        {
            if ( bFirst )
            {
                dwSize = sprintf_s( szMsg, sizeof(szMsg), "SPYCNX %u,%s %s",
                             dwIndex, 
                             bState ? "CONNECTION" : "DISCONNECTION",
                             ROUTESVC_PIPE_TYPE_TEXT( gsSvcWork.psList[dwIndex].dwType ) );
                bFirst = FALSE;
            }

            AComSendMessage( gsSvcWork.hWks, hCnx, 0, dwSize, szMsg );
        }
    }
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void SpyCnxLane( DWORD64 dwLCIndex )
 * PARAMETERS: dwLCIndex   : Index de la connexion de type LC
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Soumet une identification de voie à l'espionnage
 * --------------------------------------------------------------------
 */
PROTECTED void SpyCnxLane(DWORD64 dwLCIndex)
{
    DWORD dwCmdIndex;           // Pour scanner les interfaces de commande connectées
    DWORD dwSize;               // Taille du message
    BOOL bSend;                 // Flag d'activation d'un espion
    BOOL bFirst = TRUE;         // Premier envoi
    ACOM_CNX_HANDLE hCnx;       // Handle des connexion d'interface de commande
    char szMsg[128];

    for ( dwCmdIndex = gsSvcWork.dwCmdStart ; dwCmdIndex <= gsSvcWork.dwCmdEnd ; dwCmdIndex ++ )
    {
        EnterCriticalSection( &gsSvcWork.sCritical );
        bSend = SpyCnxIsActive( dwCmdIndex );
        hCnx = gsSvcWork.psList[dwCmdIndex].hCnx;
        LeaveCriticalSection( &gsSvcWork.sCritical );
 
        if ( bSend )
        {
            if ( bFirst )
            {
                if ( ! gsSvcWork.psList[dwLCIndex].uCnx.sLC.fQualified )
                {
                    dwSize = sprintf_s( szMsg, sizeof(szMsg), "SPYCNX %u,LC UNQUALIFIED",
                                 dwLCIndex );
                }
                else if ( ( gsSvcWork.psList[dwLCIndex].uCnx.sLC.dwCount == 1 ) &&
                          ! gsSvcWork.psList[dwLCIndex].uCnx.sLC.fMultiple )
                {
                    dwSize = sprintf_s( szMsg, sizeof(szMsg), "SPYCNX %u,IDENTIFICATION Plaza=%u Lane=%u",
                                 dwLCIndex, 
                                 gsSvcWork.psList[dwLCIndex].uCnx.sLC.tdwPlaza[0],
                                 gsSvcWork.psList[dwLCIndex].uCnx.sLC.tdwLane[0] );
                }
                else
                {
                    dwSize = sprintf_s( szMsg, sizeof(szMsg), "SPYCNX %u,IDENTIFICATION PRE-CONCENTRATED",
                                 dwLCIndex );
                }
                bFirst = FALSE;
            }

            AComSendMessage( gsSvcWork.hWks, hCnx, 0, dwSize, szMsg );
        }
    }
}
