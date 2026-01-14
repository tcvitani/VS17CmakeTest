/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : EVTLOG
 * FILE       : EVTLOG_TEST.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Unitary test program for EVTLOG
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
#include <time.h>
#include <reg.h>

#include <CSR_EVTLOG.h>

#define LOC_DEF
#include <EVTLOG_test.h>
#undef LOC_DEF

#include <memclass.h>


#ifdef _DEBUG
#define DLL_NAME "CSR_EVTLOGD.DLL"
#define SOURCE_NAME "LOGMOND"
#else
#define DLL_NAME "CSR_EVTLOG.DLL"
#define SOURCE_NAME "LOGMON"
#endif



#include "../resources/messages.h"


PROTECTED int __cdecl main(
    IN int iArgc,
    IN char ** ppcArgc )
{
    DWORD dwErr;
    DWORD dwIndex;
    DWORD dwCount;
    // DWORD dwPrev;
    // DWORD dwNew;

    DWORD dwRecordNbr;
    DWORD dwAnsiTime;
    DWORD dwEventId;
    WORD  wEventType;
    WORD  wEventCategory;
    char  szText[4096];
    char  szSource[4096];
	char  szTimeT[4096];
    DWORD dwDataBytes;
    BYTE  tbData[4096];
    DWORD dwFlags;

    HANDLE hLog;
    char * pcStr = "PARAM";
    char * pcCourant = "DEBUT";
    EVTLOG_ENTRY * psList;
    EVTLOG_ENTRY * psScan;

	time_t *pTimeT = NULL;

    __try
    {
/*

        for ( dwIndex = 0 ; dwIndex < 300 ; dwIndex ++ )
        {
            pcCourant = "OPEN";

            dwErr = EvtLogCreateSource( SOURCE_NAME, &hLog );
            if ( dwErr != NO_ERROR )
                __leave;

            pcCourant = "REPORT SUCCESS";
            dwErr = EvtLogReportText( hLog, EVTLOG_SUCCESS, 1, "Ceci est un texte de %s", pcCourant );
            if ( dwErr != NO_ERROR )
                __leave;


            pcCourant = "REPORT INFO";
            dwErr = EvtLogReportText( hLog, EVTLOG_INFO, 1, "Ceci est un texte de %s", pcCourant );
            if ( dwErr != NO_ERROR )
                __leave;

            pcCourant = "REPORT WARNING";
            dwErr = EvtLogReportText( hLog, EVTLOG_WARNING, 1, "Ceci est un texte de %s", pcCourant );
            if ( dwErr != NO_ERROR )
                __leave;

            pcCourant = "REPORT ERROR";
            dwErr = EvtLogReportText( hLog, EVTLOG_ERROR, 1, "Ceci est un texte de %s", pcCourant );
            if ( dwErr != NO_ERROR )
                __leave;
        
            pcCourant = "CLOSE"; 

            EvtLogClose( hLog );
        }

        pcCourant = "OPEN"; */

        EvtLogSetOptions( EVTLOG_OPTION_REMOTETEXTDEF );

        pcCourant = "REMOTE";
        dwErr = EvtLogRemoteLink( "CCI-PCS002-BASE", "", "LANE", "LANE" );
        if ( dwErr != NO_ERROR )
            __leave;

        pcCourant = "OPEN";
        dwErr = EvtLogCreateView( "CCI-PCS002-BASE", "Application", &hLog );
        if ( dwErr != NO_ERROR )
            __leave;

        pcCourant = "GETCOUNT";
        dwErr = EvtLogGetCount( hLog, &dwCount );
		printf("\nCount=%u", dwCount);

        while ( TRUE )
        {
            pcCourant = "ENUM";
            dwErr = EvtLogReadItems( hLog, EVTLOG_FROM_OLDEST, 0, &psList );
            if ( dwErr != NO_ERROR )
                __leave;


            psScan = psList;
            while ( psScan )
            {
                dwDataBytes = sizeof(tbData);
                EvtLogGetItemDetails( psScan,
                                      &dwRecordNbr,
                                      &dwAnsiTime,
                                      &dwEventId,
                                      &wEventType,
                                      &wEventCategory,
                                      sizeof(szSource),
                                      szSource,
                                      sizeof(szText),
                                      szText,
                                      &dwDataBytes,
                                      tbData,
                                      &dwFlags );

                printf( "\n" );
                if ( ( dwFlags & EVTLOG_RECORD_NBR_PRESENT ) != 0 ) printf( "Rec=%u", dwRecordNbr );
                if ( ( dwFlags & EVTLOG_ID_PRESENT         ) != 0 ) printf( " I=%u", dwEventId );
                if ( ( dwFlags & EVTLOG_SOURCE_PRESENT     ) != 0 ) printf( " S=%s", szSource );

				pTimeT = (time_t*)(&dwAnsiTime);
				ctime_s(szTimeT, sizeof(szTimeT), pTimeT);
				if ((dwFlags & EVTLOG_TIME_PRESENT) != 0) printf(" T=%s", szTimeT);
                printf( "\n" );
                if ( ( dwFlags & EVTLOG_TYPE_PRESENT       ) != 0 ) printf( " Y=%u", (DWORD)wEventType );
                if ( ( dwFlags & EVTLOG_CATEGORY_PRESENT   ) != 0 ) printf( " C=%u", (DWORD)wEventCategory );
                if ( ( dwFlags & EVTLOG_TEXT_PRESENT       ) != 0 ) printf( " X=[%s]", (DWORD)szText );
                if ( ( dwFlags & EVTLOG_DATA_PRESENT       ) != 0 )
                {
                    printf( " DATA=[ " );
                    for ( dwIndex = 0 ; dwIndex < dwDataBytes ; dwIndex ++ )
                        printf( "%02X", (DWORD)tbData[dwIndex] );
                    printf( " ]" );
                }

                psScan = psScan->psNext;
            }

            EvtLogFreeItems( psList );

			gets_s(szText, sizeof(szText) );
            if ( _strnicmp( szText, "QUIT", 4 ) == 0 )
                break;
        }

        EvtLogCloseView( hLog );

/*
        dwErr = EvtLogGetMostRecent( hLog, &dwPrev );
        if ( dwErr != NO_ERROR )
            __leave;

        while ( TRUE )
        {
            dwErr = EvtLogGetMostRecent( hLog, &dwNew );
            if ( dwErr != NO_ERROR )
                __leave;

            if ( dwNew != dwPrev )
            {
                printf( "\nNEW! %u", dwNew );
                dwPrev = dwNew;
            }

            Sleep(1000);
        }
*/
        pcCourant = "FIN";
        dwErr = NO_ERROR;


    }
    __finally
    {
        printf( "\nStatus: %u / %s", dwErr, pcCourant );
		gets_s(szText, sizeof(szText));
    }

	return dwErr;
}
