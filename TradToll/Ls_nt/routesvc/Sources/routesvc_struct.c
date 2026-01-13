/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : routesvc
 * FILE       : routesvc_struct.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : LS
 * --------------------------------------------------------------------
 * SUMMARY    : Manipulation de la liste des connexion et des structures associées
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
#include <msg_sv_filt_dec.h>
#include <msg_lc_header.h>

#include <routesvc_glob.h>
#include <routesvc_text.h>

#define LOC_DEF
#include <routesvc_struct.h>
#undef LOC_DEF

#include <memclass.h>




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI StructOpenList( DWORD dwLCCount, DWORD dwLSCount, DWORD dwCmdCount )
 * PARAMETERS: dwLCCount  : Nombre max de voies connectées
 *             dwLSCount  : Nombre max de clients LS connectés
 *             dwCmdCount : Nombre max d'interfaces de commande connectées
 * RETURN    : NO_ERROR si succès
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise la liste des connexions
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI StructOpenList( DWORD dwLCCount, DWORD dwLSCount, DWORD dwCmdCount )
{
    DWORD dwIndex = 0;              // Pour scanner la liste créée

    // S'il existe déjà une liste
    if ( gsSvcWork.dwListCount != 0 )
        return ERROR_INVALID_DATA;

    // Nombre total d'entrées dans la liste
    gsSvcWork.dwListCount = dwLCCount + dwLSCount + dwCmdCount;

    gsSvcWork.psList = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( ROUTESVC_CONNECTION ) * gsSvcWork.dwListCount );
    if ( gsSvcWork.psList == NULL )
    {
        gsSvcWork.dwListCount = 0;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    // Définition des bornes
    gsSvcWork.dwLCStart = 0;
    gsSvcWork.dwLCEnd = dwLCCount - 1;
    gsSvcWork.dwLSStart = dwLCCount;
    gsSvcWork.dwLSEnd = dwLCCount + dwLSCount - 1;
    gsSvcWork.dwCmdStart = dwLCCount + dwLSCount;
    gsSvcWork.dwCmdEnd = dwLCCount + dwLSCount + dwCmdCount - 1;

    // Initialisation des types
    for ( dwIndex = gsSvcWork.dwLCStart ; dwIndex <= gsSvcWork.dwLCEnd ; dwIndex ++ )
        gsSvcWork.psList[dwIndex].dwType = ROUTESVC_PIPE_LC_TYPE;
    for ( dwIndex = gsSvcWork.dwLSStart ; dwIndex <= gsSvcWork.dwLSEnd ; dwIndex ++ )
        gsSvcWork.psList[dwIndex].dwType = ROUTESVC_PIPE_LS_TYPE;
    for ( dwIndex = gsSvcWork.dwCmdStart ; dwIndex <= gsSvcWork.dwCmdEnd ; dwIndex ++ )
        gsSvcWork.psList[dwIndex].dwType = ROUTESVC_PIPE_CMD_TYPE;

    return NO_ERROR;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED VOID WINAPI StructCloseList()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Detruit la liste des connexions
 * --------------------------------------------------------------------
 */
PROTECTED VOID WINAPI StructCloseList()
{
    HeapFree( GetProcessHeap(), 0, gsSvcWork.psList );
    gsSvcWork.psList = NULL;
    gsSvcWork.dwListCount = 0;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD64 WINAPI StructAllocate( DWORD64 dwType )
 * PARAMETERS: dwType : type de l'élement (ROUTESVC_PIPE_LC_TYPE, ROUTESVC_PIPE_LS_TYPE ou ROUTESVC_PIPE_CMD_TYPE)
 * RETURN    : Index de connexion ou STRUCT_NOT_ALLOCATED si erreur
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Reserve un emplacement d'un type donnée dans la liste des connexion
 * --------------------------------------------------------------------
 */
PROTECTED DWORD64 WINAPI StructAllocate(DWORD64 dwType)
{
	DWORD64 dwIndex = 0;                      // Pour scanner la liste
	DWORD64 dwAlloc = STRUCT_NOT_ALLOCATED;   // Index d'emplacement libre

    EnterCriticalSection( &gsSvcWork.sCritical );

    // Rechercher un emplacement libre et du bon type
    for ( dwIndex = 0 ; ( dwIndex <= gsSvcWork.dwListCount ) && ( dwAlloc == STRUCT_NOT_ALLOCATED ) ; dwIndex ++ )
    {
        if ( ( gsSvcWork.psList[dwIndex].dwType == dwType ) && ( ! gsSvcWork.psList[dwIndex].bBusy ) )
        {
            gsSvcWork.psList[dwIndex].bBusy = TRUE;
            ZeroMemory( &gsSvcWork.psList[dwIndex].uCnx, sizeof( gsSvcWork.psList[dwIndex].uCnx ) );
            dwAlloc = dwIndex;
        }
    }

    LeaveCriticalSection( &gsSvcWork.sCritical );

    return dwAlloc;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD64 WINAPI StructFree( DWORD64 dwIndex )
 * PARAMETERS: dwIndex : Index de l'emplacement de la liste à libérer
 * RETURN    : NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Libère un emplacement de la liste des connexions
 * --------------------------------------------------------------------
 */
PROTECTED DWORD64 WINAPI StructFree(DWORD64 dwIndex)
{
	DWORD64 dwErr = NO_ERROR;         // Code d'erreur de retour

    EnterCriticalSection( &gsSvcWork.sCritical );

    if ( gsSvcWork.psList[dwIndex].bBusy )
    {

        if ( gsSvcWork.psList[dwIndex].dwType == ROUTESVC_PIPE_LC_TYPE )
        {
            // Rien de spécial à faire
        }

        else if ( gsSvcWork.psList[dwIndex].dwType == ROUTESVC_PIPE_LS_TYPE )
        {
            // Libérer le filtre
            if ( gsSvcWork.psList[dwIndex].uCnx.sLS.dwFilterCount > 0 )
                HeapFree( GetProcessHeap(), 0, gsSvcWork.psList[dwIndex].uCnx.sLS.psFilterList );
        }

        else
        {
            // Libérer les espions
            if ( gsSvcWork.psList[dwIndex].uCnx.sCmd.dwSpyCount > 0 )
                HeapFree( GetProcessHeap(), 0, gsSvcWork.psList[dwIndex].uCnx.sCmd.pdwSpyList );
        }

        gsSvcWork.psList[dwIndex].bBusy = FALSE;

    }
    else
    {
        // Y a un gros probleme
        dwErr = ERROR_INVALID_DATA;
    }

    LeaveCriticalSection( &gsSvcWork.sCritical );

    return dwErr;
}


PROTECTED DWORD WINAPI StructSetFilter(DWORD64 dwIndex, HLIST hList)
{
    struct MSG_SV_FILT_DEC_Idcd * psIDCD;

    DWORD dwErr = NO_ERROR;         // Code d'erreur de retour
    DWORD dwCount = 0;              // Nombre de paires ID/CD
    DWORD dwScan;                   // Scan des paires ID/CD
    ROUTESVC_LS * psLS;             // Pointe sur la connexion LS
    ROUTESVC_FILTER * psNewList;    // Nouvelle table du filtre

    if ( ( gsSvcWork.psList[dwIndex].dwType == ROUTESVC_PIPE_LS_TYPE ) && ( gsSvcWork.psList[dwIndex].bBusy ) )
    {
        psLS = &gsSvcWork.psList[dwIndex].uCnx.sLS;

        psIDCD = MSG_SV_FILT_DEC_Get_First_Idcd( hList );
        dwCount = List_GetCount( hList );
        if ( psIDCD != NULL && dwCount > 0)
        {
            // Préparer le buffer recevant la liste dans le bon format
            psNewList = HeapAlloc( GetProcessHeap(), 0, dwCount * sizeof( *psNewList ) );
            if ( psNewList != NULL )
            {
                dwScan = 0;
                do
                {
                    if ( ( psIDCD->id > MSG_SV_FILT_DEC_ID_ANY ) || ( psIDCD->id > MSG_SV_FILT_DEC_CD_ANY ) )
                    {
                        HeapFree( GetProcessHeap(), 0, psNewList );
                        psNewList = NULL;
                        dwErr = ERROR_INVALID_DATA;
                        break;
                    }
                    psNewList[dwScan].dwID = psIDCD->id;
                    psNewList[dwScan].dwCD = psIDCD->cd;
                    dwScan ++;
                    NTSVCInfo( "StructSetFilter(), ajout du filtre %u/%u", psIDCD->id, psIDCD->cd );
                }
                while ( ( psIDCD = MSG_SV_FILT_DEC_Get_Next_Idcd( hList, psIDCD ) ) != NULL );
            }
            else 
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
            psNewList = NULL;

        if ( dwErr == NO_ERROR )
        {
            // Mettre à jour la nouvelle liste
            if ( psLS->psFilterList != NULL )
                HeapFree( GetProcessHeap(), 0, psLS->psFilterList );
            psLS->psFilterList = psNewList;
            psLS->dwFilterCount = dwCount;
        }
    }
    else
    {
        // Y a un gros probleme
        dwErr = ERROR_INVALID_DATA;
    }

    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL WINAPI StructInFilter( DWORD64 dwIndex, DWORD dwID, DWORD dwCD )
 * PARAMETERS: dwIndex : Index de l'emplacement de la connexion de type LS
 *             dwId    : Id à tester
 *             dwCd    : Cd à tester
 * RETURN    : NO_ERROR si OK
 * --------------------------------------------------------------------
 * VARIABLES : TRUE si la paire ID/CD passée en paramètre est dans le filtre de la connexion
 *             correspondant à l'index dwIndex
 * --------------------------------------------------------------------
 * ROLE      : Test la présence d'une paire ID/CD dans le filtre d'une connexion
 * --------------------------------------------------------------------
 */
PROTECTED BOOL WINAPI StructInFilter(DWORD64 dwIndex, DWORD dwID, DWORD dwCD)
{
    BOOL bFound = FALSE;            // Flag indiquant le succès de la recherche
    ROUTESVC_LS * psLS;             // Pointe sur la connexion LS
    DWORD dwScan;                   // Scan des paires ID/CD

    if ( ( gsSvcWork.psList[dwIndex].dwType == ROUTESVC_PIPE_LS_TYPE ) &&
         ( gsSvcWork.psList[dwIndex].bBusy ) )
    {
        psLS = &gsSvcWork.psList[dwIndex].uCnx.sLS;

        for ( dwScan = 0 ; ( dwScan < psLS->dwFilterCount ) && ( ! bFound ); dwScan ++ )
            if ( ( ( dwID == psLS->psFilterList[dwScan].dwID ) || ( psLS->psFilterList[dwScan].dwID == MSG_SV_FILT_DEC_ID_ANY ) ) &&
                 ( ( dwCD == psLS->psFilterList[dwScan].dwCD ) || ( psLS->psFilterList[dwScan].dwCD == MSG_SV_FILT_DEC_CD_ANY ) ) )
                bFound = TRUE;
    }

    return bFound;
}



