/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
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
#include <acom.h>
#include <col.h>
#include <ntsvc.h>

#include <csr_rfa_svc.h>
#include <csr_rfa.h>

#include <trfsvc_glob.h>
#include <trfsvc_text.h>
#include <trfsvc_protocol.h>

#define LOC_DEF
#include <trfsvc_file_callback.h>
#undef LOC_DEF

#include <memclass.h>

//#pragma warning (disable : 4996)

PRIVATE DWORD FileSendFull( TRFSVC_SERVER * psServer, RFA_STRUCT * psHandle );


PROTECTED void CALLBACK FileCallback( RFA_STRUCT * psHandle, ULONGLONG ullPos, DWORD dwSize, BYTE * pbBuffer )
{
    HCOLLECTIONITEM         hItem;
    TRFSVC_SERVER         * psServer;
    TRFSVC_FILE           * psFile;
    FSYNC_MSG_PARTIAL_NF  * psNF;
    DWORD                   dwDataSize;
    DWORD                   dwMaxDataSize;
    DWORD                   dwErr;
    DWORD                   dwWorkSize;
    BYTE                  * pbWorkBuffer;
    ULONGLONG               ullWorkPos;

    NTSVCInfo( "FileCallback(), %s - pos=%I64u - datablock=%u - pbBuffer=0x%08X", psHandle->szKey, ullPos, dwSize, pbBuffer );

    // Marquer la date de dernière mise à jour

    psFile = NULL;
    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColFiles, &hItem ) )
    {

        // Utiliser le vérou de la collection comme exclusion pour
        // serialiser les comparaisons et mises à jour des dates
        ColLock( gsSvcWork.hColFiles );

        psFile = ColItemData( hItem );
        if ( _stricmp( psHandle->szKey, psFile->szFileKey ) == 0 ) 
        {
            psFile->ullLastTime = psHandle->ullLastWrite;
            NTSVCInfo( "FileCallback(), last write =0x%016X", psFile->ullLastTime );
        }

        ColUnlock( gsSvcWork.hColFiles );
    }

    if ( psFile == NULL )
    {
        NTSVCInfo( "FileCallback(), problème, le fichier %s ne figure pas dans la collection", psHandle->szKey );
    }


    // A propos des acces concurents :
    //  - Un autre callback portant sur ce fichier est impossible puisque le callback est
    //    protégé par le mutex nommé associé au fichier.
    //  - Pour éviter un deadlock entre un lock sur les connexions et un lock sur la collection,
    //    seule la collection est lockée. Un échec des fonction AComXXX permet de savoir que la
    //    collection n'existe plus. De plus, la protection du callback fait que des notifications
    //    ne peuvent pas être mélangées.

    ColLock( gsSvcWork.hColServers );

    NTSVCInfo( "FileCallback(), Scan des clients" );

    hItem = COL_SCAN_BEGIN;
    while ( ColItemScan( gsSvcWork.hColServers, &hItem ) )
    {
        psServer = ColItemData( hItem );
        if ( _stricmp( psHandle->szKey, psServer->psFile->szFileKey ) != 0 ) 
        {
            NTSVCInfo( "FileCallback(), Client filtré sur %s", psServer->psFile->szFileKey );
            continue;
        }

        NTSVCInfo( "FileCallback(), Client OK / hCnx = 0x%08X / Id = %s", psServer->hCnx, psServer->szClientId );

        psNF = (FSYNC_MSG_PARTIAL_NF*)psServer->tbBuffer;
        psNF->dwMessageCode = FSYNC_CODE_PARTIAL_NF;

        if ( pbBuffer != NULL )
        {
            NTSVCInfo( "FileCallback(), La notification contient des données" );

            psNF->ullLastWrite = psHandle->ullLastWrite;
            psNF->ullFileSize = psHandle->ullSize;
            dwMaxDataSize = psServer->dwMaxBufferBytes - sizeof(*psNF);

            NTSVCInfo( "FileCallback(), Début boucle envoi des données en PARTIAL_NF" );

            dwWorkSize = dwSize;
            pbWorkBuffer = pbBuffer;
            ullWorkPos = ullPos;

            while ( dwWorkSize > 0 )
            {
                dwDataSize = __min( dwMaxDataSize, dwWorkSize );

                psNF->ullOffset = ullWorkPos;
                psNF->dwDataSize = dwDataSize;

                memcpy( psNF->tbData, pbWorkBuffer, dwDataSize );

                NTSVCInfo( "FileCallback(), Send PARTIAL_NF lastwrite=%I64u filesize=%I64u offset=%I64u size=%u", 
                           psNF->ullLastWrite,
                           psNF->ullFileSize,
                           psNF->ullOffset,
                           psNF->dwDataSize );

                dwErr = AComSendMessage( gsSvcWork.hWks, psServer->hCnx, 0, FSYNC_SIZEOF_PARTIAL_NF( psNF ), psNF );
                if ( dwErr != NO_ERROR )
                {
                    NTSVCInfo( "FileCallback(), Erreur %u de AComSendMessage()", dwErr );
                    AComDisconnectPeer( gsSvcWork.hWks, psServer->hCnx, FALSE );
                }

                dwWorkSize -= dwDataSize;
                ullWorkPos += (ULONGLONG)dwDataSize;
                pbWorkBuffer += dwDataSize;
            }

            NTSVCInfo( "FileCallback(), Fin boucle envoi des données en PARTIAL_NF" );
        }
        else
        {
            NTSVCInfo( "FileCallback(), La notification ne contient pas de données" );

            switch ( ullPos )
            {
            case RFA_CALLBACK_DATE :
            case RFA_CALLBACK_CUT :
            case RFA_CALLBACK_NEW :

                NTSVCInfo( "FileCallback(), Il s'agit soit d'un chgt de date, d'une coupure ou d'une création" );

                psNF->ullLastWrite = psHandle->ullLastWrite;
                psNF->ullFileSize = psHandle->ullSize;
                psNF->ullOffset = 0;
                psNF->dwDataSize = 0;
                NTSVCInfo( "FileCallback(), Send PARTIAL_NF lastwrite=%I64u filesize=%I64u offset=%I64u size=%u", 
                           psNF->ullLastWrite,
                           psNF->ullFileSize,
                           psNF->ullOffset,
                           psNF->dwDataSize );
                dwErr = AComSendMessage( gsSvcWork.hWks, psServer->hCnx, 0, FSYNC_SIZEOF_PARTIAL_NF( psNF ), psNF );
                if ( dwErr != NO_ERROR )
                {
                    NTSVCInfo( "FileCallback(), Erreur %u de AComSendMessage()", dwErr );
                    AComDisconnectPeer( gsSvcWork.hWks, psServer->hCnx, FALSE );
                }
                break;

            case RFA_CALLBACK_DEL :

                NTSVCInfo( "FileCallback(), Il s'agit d'un effacement" );

                psNF->ullLastWrite = 0;
                psNF->ullFileSize = (ULONGLONG)(-1);
                psNF->ullOffset = 0;
                psNF->dwDataSize = 0;
                NTSVCInfo( "FileCallback(), Send PARTIAL_NF lastwrite=%I64u filesize=%I64u offset=%I64u size=%u", 
                           psNF->ullLastWrite,
                           psNF->ullFileSize,
                           psNF->ullOffset,
                           psNF->dwDataSize );
                dwErr = AComSendMessage( gsSvcWork.hWks, psServer->hCnx, 0, FSYNC_SIZEOF_PARTIAL_NF( psNF ), psNF );
                if ( dwErr != NO_ERROR )
                {
                    NTSVCInfo( "FileCallback(), Erreur %u de AComSendMessage()", dwErr );
                    AComDisconnectPeer( gsSvcWork.hWks, psServer->hCnx, FALSE );
                }
                break;

            case RFA_CALLBACK_REPLACE :

                NTSVCInfo( "FileCallback(), Il s'agit d'un remplacement" );

                if ( FileSendFull( psServer, psHandle ) != NO_ERROR )
                {
                    AComDisconnectPeer( gsSvcWork.hWks, psServer->hCnx, FALSE );
                }
                break;

            default :

                NTSVCInfo( "FileCallback(), Il s'agit d'un cas NON GERE" );
            }
        }
    }

    ColUnlock( gsSvcWork.hColServers );

    NTSVCInfo( "FileCallback(), Fin" );
}

        


PRIVATE DWORD FileSendFull( TRFSVC_SERVER * psServer, RFA_STRUCT * psHandle )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwMaxDataSize;
    DWORD dwDataSize;
    DWORD dwRead;
    BOOL bResult;
    BOOL bClose = FALSE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    FSYNC_MSG_FULL_NF * psNF = (FSYNC_MSG_FULL_NF*)psServer->tbBuffer;
    ULONGLONG ullFileSize;
    ULONGLONG ullOffset;

    // ATTENTION : Cette fonction doit être appelé dans du code ayant l'accés
    // exclusif sur le fichier (mutex nommé).

    __try
    {
        NTSVCInfo( "FileSendFull(), %s", psServer->szFileKey );

        dwMaxDataSize = psServer->dwMaxBufferBytes - sizeof(*psNF);
        ullFileSize = psHandle->ullSize;

        psNF->dwMessageCode = FSYNC_CODE_FULL_NF;
        psNF->ullLastWrite = psHandle->ullLastWrite;
        psNF->ullFileSize = psHandle->ullSize;
        ullOffset = 0;

        hFile = CreateFile(
            psHandle->szFile,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( hFile == INVALID_HANDLE_VALUE ) { dwErr = GetLastError(); __leave; }

        while ( ullFileSize != 0 )
        {

            dwDataSize = ( ullFileSize > (ULONGLONG)dwMaxDataSize ? dwMaxDataSize : RFA_LL_LO(ullFileSize) );

            bResult = ReadFile(
                hFile,
                psNF->tbData,
                dwDataSize,
                &dwRead,
                NULL );
            if ( ! bResult ) { dwErr = GetLastError(); __leave; }

            ullFileSize -= dwDataSize;

            psNF->ullOffset = ullOffset;
            psNF->dwDataSize = dwDataSize;

            NTSVCInfo( "FileCallback(), Send FULL_NF lastwrite=%I64u filesize=%I64u offset=%I64u size=%u", 
                       psNF->ullLastWrite,
                       psNF->ullFileSize,
                       psNF->ullOffset,
                       psNF->dwDataSize );

			// TMA
            //dwErr = AComSendMessage( gsSvcWork.hWks, psServer->hCnx, 0, FSYNC_SIZEOF_FULL_NF( psNF ), psNF );
			dwErr = AComSendMessageCb( gsSvcWork.hWks, psServer->hCnx, 1, FSYNC_SIZEOF_FULL_NF( psNF ), psNF, psServer );
			// end of TMA

            if ( dwErr != NO_ERROR ) __leave;

            ullOffset += dwDataSize;
        }
    }
    __finally
    {
        NTSVCInfo( "FileSendFull(), retourne %u", dwErr );

        if ( hFile != INVALID_HANDLE_VALUE ) CloseHandle( hFile );

    }

	return dwErr;
}






    
