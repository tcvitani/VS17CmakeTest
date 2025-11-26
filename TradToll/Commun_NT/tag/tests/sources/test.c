//
// TAG library sample program. This program illustrates some aspects of the TAG
// and CENL7 APIs. This application will work with a tag placed in front of the
// beacon or in the ELI.
//
// MODULE : MAIN MODULE
//
// The tag must be configured with the DSRC attributes : 
//
//    - EFCContextMark       (id =  0, container = 32, 6 bytes, implicit size),
//    - ContractSerialNumber (id =  1, container = 33, 4 bytes, implicit size),
//    - ReceiptServicePart   (id =  5, container = 37, 13 bytes, implicit size).
//    - VehicleClass		 (id = 17, container = 49, 1 byte , implicit size),
//    - EquipmentStatus      (id = 26, container = 58, 2 bytes, implicit size).
//
/* <-- NBL-2003-07-11
// All these attributes have and implicit length. They mustn't be protected by
// access credentials.
   NBL-2003-07-11 --> */
// <++ NBL-2003-07-11
// All these attributes have an implicit length. They must be protected by access
// credentials.
// NBL-2003-07-11 ++>
//


// <++ NBL-2003-07-11
//
// 2003-07-11 : Changes description :
//    - Now, the security is coded using the GSS/A1 style.
//      The master keys used are the one used by combitech on their test OBEs.
//      In order to work properly, an OBE must be configured with these keys.
//    - The access credentials are know used for the requests that are accessing
//      to the attributes.
//    - The GET request has been replaced by a concatenation of a GETSTAMPED and
//      a GET resquests.
//    - The GET part now only reads 3 attributes (ContractSerialNumber,
//      ReceiptServicePart and VehicleClass). The ContextMark is now directly
//      extracted from the parameters of the application in the VST. The
//      EquipmentStatus attribute is read using the GET-STAMPED request.
//    - All the modification have been flaged, using <++ and ++> "tags" to
//      identify the code that has been added and <-- and --> "tag" to identify
//      the code that has been removed.
//
// NBL-2003-07-11 ++>



#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <conio.h>



#include <csr_tag_trsp.h>
#include <csr_tag_cenl7.h>
#include <csr_tag.h>





#define MAX_FRAME_BYTES 132
#define MAX_FRAMES      1024



typedef struct
{
    LONGLONG            llTick;
    DWORD               dwBytes;
    DWORD               dwType;
    DWORD               dwLost;
    BYTE                tbData[256];
}
    TRACE_FRAME;



typedef struct
{
    DWORD               dwNext;
    DWORD               dwLast;

    LONGLONG            llFreq;
    CRITICAL_SECTION    sProtect;
    HANDLE              hThread;
    DWORD               dwThread;
}
    TRACE_HISTORY;



typedef struct
{
    TAG_INSTANCE          * hTag;
    TAG_INTERFACE_STATUS    eLastStatus;
    DWORD                   dwLastErr;
    DWORD                   dwAsyncErr;
    HANDLE                  hEvent;
	DWORD                   dwRspTimeout;
	DWORD                   dwBSTRspTimeout;
    DWORD                   dwId;
    BOOL                    fGetOk;
    BOOL                    fSetOk;
    TRACE_HISTORY           sHist;

// <++ NBL
    BOOL                    fAppFound;
    BYTE                    bEID;
    BYTE                    tbRndOBE[4];
    BYTE                    tbOBEGroupId[2];
    BYTE                    tbRndRSE[4];
    BYTE                    tbContextMark[6];
    BYTE                    tbAC[4];
    BYTE                    tbContractSerialNumber[4];
    DWORD                   dwCountry;
    DWORD                   dwCompany;
    DWORD                   dwTypeOfContract;
    DWORD                   dwContextVersion;
// NBL ++>

}
    TEST_CTX;




// <++ NBL-2003-07-11

BYTE gtbSecurity[] = 
{
    // Master element access key (120)
    0x49, 0x74, 0x27, 0x73, 0x20, 0x6F, 0x6B, 0x20, 0x74, 0x6F, 0x20, 0x6C, 0x61, 0x75, 0x67, 0x68,

    // Master element authentication keys (111 to 118)
    0x49, 0x74, 0x20, 0x77, 0x61, 0x73, 0x20, 0x61, 0x20, 0x6E, 0x65, 0x77, 0x20, 0x61, 0x67, 0x65,
    0x3B, 0x20, 0x69, 0x74, 0x20, 0x77, 0x61, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x65, 0x6E, 0x64,
    0x20, 0x6F, 0x66, 0x20, 0x68, 0x69, 0x73, 0x74, 0x6F, 0x72, 0x79, 0x3B, 0x20, 0x69, 0x74, 0x20,
    0x77, 0x61, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x79, 0x65, 0x61, 0x72, 0x20, 0x65, 0x76, 0x65,
    0x72, 0x79, 0x74, 0x68, 0x69, 0x6E, 0x67, 0x20, 0x63, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x64, 0x2E,
    0x20, 0x54, 0x68, 0x65, 0x20, 0x79, 0x65, 0x61, 0x72, 0x20, 0x69, 0x73, 0x20, 0x32, 0x32, 0x36,
    0x31, 0x3B, 0x20, 0x74, 0x68, 0x65, 0x20, 0x70, 0x6C, 0x61, 0x63, 0x65, 0x3A, 0x20, 0x42, 0x35,
    0x49, 0x74, 0x20, 0x77, 0x61, 0x73, 0x20, 0x61, 0x20, 0x6E, 0x65, 0x77, 0x20, 0x61, 0x67, 0x65
};


//
// Which is equivalent to (except for the ending NUL char) :
//    "It's ok to laugh"
//    "It was a new age"
//    "; it was the end"
//    " of history; it "
//    "was the year eve"
//    "rything changed."
//    " The year is 226"
//    "1; the place: B5"
//    "It was a new age"
//
// For the ones interested in SF, except for the first
// sentence, this is the introduction text of the 4th
// season of "Babylon 5". The guys from Combitech seem
// to be real fans.
//

// NBL-2003-07-11 ++>


//////////////////////////////////////////////////////////
//
//  Win32ErrorText : 
//
//      Parameters :
//          dwErr : Win32 error code
//
//      Description :
//          The function return the text associated with
//          a Win32 error code.
//
//      Return :
//          A pointer to a buffer containing the text.
//          Note : this function must not be used by
//          many threads at the same time because it is
//          using a unique static buffer.
//
char * Win32ErrorText( IN DWORD dwErr )
{
    static char szText[1024];
    BOOL        fResult;

    fResult = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                             NULL,
                             dwErr,
                             0,
                             szText,
                             sizeof(szText),
                             NULL );

    if ( ! fResult )
		sprintf_s(szText, sizeof(szText), "Unknown system error %u", dwErr);

    return szText;
}






//////////////////////////////////////////////////////////
//
//  AnalyseHeap : 
//
//      Parameters :
//          None
//
//      Description :
//          Heap analysis function.
//
//      Return :
//          Nothing
//
void WINAPI AnalyseHeap()
{
    static BOOL         fIncrement = FALSE;
    static DWORD        dwBlockCountOld = 0;
    static DWORD        dwRegionCountOld = 0;
    static DWORD        dwOtherCountOld = 0;
    static DWORD        dwBlockDataOld = 0;
    static DWORD        dwRegionDataOld = 0;
    static DWORD        dwOtherDataOld = 0;
    DWORD               dwBlockCount = 0;
    DWORD               dwRegionCount = 0;
    DWORD               dwOtherCount = 0;
    DWORD               dwBlockData = 0;
    DWORD               dwRegionData = 0;
    DWORD               dwOtherData = 0;
    BOOL                fLocked = FALSE;

    DWORD               dwErr;
    PROCESS_HEAP_ENTRY  sHeap;

    __try
    {
        printf( "\nHeap enumeration :" );

        printf( "\n  Locking heap ..." );
        if ( ! HeapLock( GetProcessHeap() ) )
        {
            dwErr = GetLastError();
            __leave;
        }

        fLocked = TRUE;
        
        ZeroMemory( &sHeap, sizeof(sHeap) );

        printf( "\n  Walking heap ..." );
        while ( HeapWalk( GetProcessHeap(), &sHeap ) )
        {
            printf( "\n    HeapElement" );
            printf( "\n      OverHead    : %u", sHeap.cbOverhead );
            printf( "\n      Data        : %u", sHeap.cbData );
            printf( "\n      RegionIndex : %u", (DWORD)sHeap.iRegionIndex );

            if ( ( sHeap.wFlags & PROCESS_HEAP_REGION ) != 0 )
            {
                printf( "\n      REGION" );
                
                printf( "\n        Committed size   : %u", sHeap.Region.dwCommittedSize );
                printf( "\n        Uncommitted size : %u", sHeap.Region.dwUnCommittedSize );
                dwRegionCount ++;
                dwRegionData += sHeap.cbData;
            }
            else if ( ( ( sHeap.wFlags & PROCESS_HEAP_ENTRY_BUSY     ) != 0 ) &&
                      ( ( sHeap.wFlags & PROCESS_HEAP_ENTRY_MOVEABLE ) != 0 ) )
            {
                printf( "\n      BLOCK" );
                dwBlockCount ++;
                dwBlockData += sHeap.cbData;
            }
            else
            {
                printf( "\n      UNKOWN" );
                dwOtherCount ++;
                dwOtherData += sHeap.cbData;
            }
        }

        dwErr = GetLastError();
        if ( dwErr != ERROR_NO_MORE_ITEMS )
            __leave;
        printf( "\n  Heap report :" );

        printf( "\n    Current regions  : %u / %u bytes", dwRegionCount, dwRegionData );
        printf( "\n    Current blocks   : %u / %u bytes", dwBlockCount, dwBlockData );
        printf( "\n    Current other    : %u / %u bytes", dwOtherCount, dwOtherData );

        if ( fIncrement )
        {
            printf( "\n    Previous regions : %u / %u bytes", dwRegionCountOld, dwRegionDataOld );
            printf( "\n    Previous blocks  : %u / %u bytes", dwBlockCountOld, dwBlockDataOld );
            printf( "\n    Previous other   : %u / %u bytes", dwOtherCountOld, dwOtherDataOld );
        }
        else
        {
            fIncrement = TRUE;
            dwRegionCountOld = dwRegionCount;
            dwRegionDataOld  = dwRegionData;
            dwBlockCountOld  = dwBlockCount;
            dwBlockDataOld   = dwBlockData;
            dwOtherCountOld  = dwOtherCount;
            dwOtherDataOld   = dwOtherData;
        }

        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( fLocked )
        {
            printf( "\n  Unlocking heap ..." );
            if ( ! HeapUnlock( GetProcessHeap() ) )
                dwErr = GetLastError();
        }

        if ( dwErr != NO_ERROR )
        {
            fprintf( stderr, 
                     "\nUnexpected error %u in heap analysis : %s", dwErr, Win32ErrorText( dwErr ) );
        }
    }
}




//////////////////////////////////////////////////////////
//
//  FrameHook : 
//
//      Parameters :
//          Refer to CSR_TAG library documents.
//
//      Description :
//          This callback function is used to queue
//          each protocol frame in the loop buffer.
//
//      Return :
//          Nothing
//
void WINAPI FrameHook( void * pvHookContext, DWORD dwTypeDir, BYTE * pbData, DWORD dwBytes )
{
    DWORD dwIndex;
    printf( "\n%s%s : ", ( dwTypeDir & TAG_HOOK_INPUT    ) ? "[<<]" : "[>>]",
                         ( dwTypeDir & TAG_HOOK_PROTOCOL ) ? " " : "*" );

    for ( dwIndex = 0 ; dwIndex < dwBytes ; dwIndex ++ )
        printf( " %02X", (DWORD)pbData[dwIndex] );

    printf( "\n      : " );

    for ( dwIndex = 0 ; dwIndex < dwBytes ; dwIndex ++ )
    {
        if ( pbData[dwIndex] >= 32 )
            printf( "  %c", (int)pbData[dwIndex] );
        else
            printf( "  ." );
    }
}




//////////////////////////////////////////////////////////
//
//  ControlInterface : 
//
//      Parameters :
//          pcCtx : Context structure of the application
//          eReq  : Type of the control request
//
//      Description :
//          This function sends a control request to the EFC
//          equipment.
//          Note : it has only effect if the equipment is a
//          beacon (no effect on ELI).
//
//      Return :
//          FALSE : An error has occured (error code stored in
//                  the context structure in psCtx->dwLastErr).
//          Other : success
//
BOOL WINAPI ControlInterface( TEST_CTX              * psCtx,
                              TAG_INTERFACE_REQUEST   eReq )
{
    DWORD       dwErr;
    BOOL        fResponse;
    DWORD       dwCount;
    CENL7_API   sFoo;

    //
    // Prepare the status of the context structure
    // of the application.
    //
    psCtx->eLastStatus = TAG_STATUS_NONE;
    psCtx->dwLastErr = NO_ERROR;

    //
    // Sends the request to the protocol stack
    //
    dwErr = TagInterfaceRequest( psCtx->hTag, 
                                 eReq,
                                 &fResponse, 
                                 psCtx->hEvent,
                                 &psCtx->dwAsyncErr );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Wait for the protocol stack to acknowledge the
    // transmission of the message. As the event is
    // automatically reset, we won't have to call
    // the ResetEvent() function later.
    //
    WaitForSingleObject( psCtx->hEvent, INFINITE );

    if ( psCtx->dwAsyncErr != NO_ERROR )
    {
        //
        // If an assynchronous error has occured, we have
        // to update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = 0x80000000 | psCtx->dwAsyncErr;
        return FALSE;
    }

    if ( ! fResponse )
    {
        //
        // This request does not expect a response from
        // the hardware (ELI), we can return immediately
        // a successfull result.
        //
        return TRUE;
    }

    //
    // Wait for a response according to the timeout
    // paremeter in the context structure.
    //
    dwCount = 1 | TAG_TRUNCATE_RESPONSE;
    dwErr = TagResponse( psCtx->hTag, 
                         &sFoo, 
                         &dwCount, 
                         &psCtx->eLastStatus, 
                         psCtx->dwRspTimeout );

    if (( dwErr != NO_ERROR )||(psCtx->eLastStatus && TAG_STATUS_ERROR != 0))
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    return TRUE;
}



//////////////////////////////////////////////////////////
//
//  AbortTransaction : 
//
//      Parameters :
//          pcCtx : Context structure of the application
//
//      Description :
//          This function sends an empty ending request to
//          the EFC equipment.
//          Note : it has only effect if the equipment is a
//          beacon (no effect on ELI).
//
//      Return :
//          FALSE : An error has occured (error code stored in
//                  the context structure in psCtx->dwLastErr).
//          Other : success
//
BOOL WINAPI AbortTransaction( TEST_CTX * psCtx )
{
    DWORD       dwErr;
    DWORD       dwCount;
    CENL7_API   sFoo;

    //
    // Prepare the status of the context structure
    // of the application.
    //
    psCtx->eLastStatus = TAG_STATUS_NONE;
    psCtx->dwLastErr = NO_ERROR;

    //
    // Sends the request to the protocol stack. It is sent
    // as an empty ending session request.
    //
    dwErr = TagSessionRequest( psCtx->hTag, 
                               TAG_SESSION_END,
                               &sFoo, 
                               0,
                               psCtx->hEvent,
                               &psCtx->dwAsyncErr );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Wait for the protocol stack to acknowledge the
    // transmission of the message. As the event is
    // automatically reset, we won't have to call
    // the ResetEvent() function later.
    //
    WaitForSingleObject( psCtx->hEvent, INFINITE );

    if ( psCtx->dwAsyncErr != NO_ERROR )
    {
        //
        // If an assynchronous error has occured, we have
        // to update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = 0x80000000 | psCtx->dwAsyncErr;
        return FALSE;
    }

    //
    // Wait for a response according to the timeout
    // paremeter in the context structure.
    //
    dwCount = 1 | TAG_TRUNCATE_RESPONSE;
    dwErr = TagResponse( psCtx->hTag, 
                         &sFoo, 
                         &dwCount, 
                         &psCtx->eLastStatus, 
                         psCtx->dwRspTimeout );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    return TRUE;
}



//////////////////////////////////////////////////////////
//
//  InitTransaction : 
//
//      Parameters :
//          pcCtx : Context structure of the application
//
//      Description :
//          This function sends an init request (BST)
//          the EFC equipment.
//
//      Return :
//          FALSE : An error has occured (error code stored in
//                  the context structure in psCtx->dwLastErr).
//          Other : success
//
BOOL WINAPI InitTransaction( TEST_CTX * psCtx )
{
    DWORD       dwErr;
    DWORD       dwCount;
    CENL7_API   sReq;
    CENL7_API   sRsp;
    //
    // Prepare the status of the context structure
    // of the application.
    //
    psCtx->eLastStatus = TAG_STATUS_NONE;
    psCtx->dwLastErr = NO_ERROR;

    //
    // Fill the init request structure
    //
    sReq.bPDU = 2;
    sReq.bFrag = 0;
    sReq.bBlockType = CENL7_APDU_INI_REQ;
    sReq.sIniReq.fOptAppPresent = FALSE;
    sReq.sIniReq.wManufacturer = 0;         // Beacon Manufacturer 4 = CSSI
    sReq.sIniReq.dwBeaconId = psCtx->dwId ++;
    time( (time_t*)&sReq.sIniReq.dwTime );
    sReq.sIniReq.bProf = 0;                 // Profile 0 = Sub-Carrier 1.5 MHz
    sReq.sIniReq.bAppCount = 1; 
    sReq.sIniReq.tsApps[0].fEIDPresent = FALSE;
    sReq.sIniReq.tsApps[0].fParmPresent = FALSE;
    sReq.sIniReq.tsApps[0].bAppId = 1;
    sReq.sIniReq.bOptAppCount = 0;
    sReq.sIniReq.bProfCount = 0;

 
	
	//
    // Sends the request to the protocol stack. It is sent
    // as a beginning session request.
    //
    dwErr = TagSessionRequest( psCtx->hTag, 
                               TAG_SESSION_BEGIN,
                               &sReq, 
                               1,
                               psCtx->hEvent,
                                &psCtx->dwAsyncErr );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Wait for the protocol stack to acknowledge the
    // transmission of the message. As the event is
    // automatically reset, we won't have to call
    // the ResetEvent() function later.
    //
    WaitForSingleObject( psCtx->hEvent, INFINITE );

    if ( psCtx->dwAsyncErr != NO_ERROR )
    {
        //
        // If an assynchronous error has occured, we have
        // to update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = 0x80000000 | psCtx->dwAsyncErr;
        return FALSE;
    }

    //
    // Wait for a response according to the timeout
    // paremeter in the context structure.
    //
    dwCount = 1 | TAG_TRUNCATE_RESPONSE;
    dwErr = TagResponse( psCtx->hTag,
                         &sRsp, 
                         &dwCount, 
                         &psCtx->eLastStatus, 
						 psCtx->dwBSTRspTimeout); // 
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Make sure the response has the appropriated
    // count of frames and that this is the expected
    // type.
    //
    if ( ( dwCount            != 1                  ) || 
         ( sRsp.bBlockType != CENL7_APDU_INI_RSP ) )
    {
        //
        // The response is not coherent regarding
        // the request.
        //
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }

    //
    // According to usual transaction processing, we should
    // analyse the application list in sRsp.sIniRsp.tsApps,
    // find the appropriate one, store its parameters and
    // associated EID ...
    // This is only a demo program, so ...
    //
    
// <++ NBL-2003-07-11

    //
    // Now the program is "smarter", so we have to find a
    // suitable application in the OBE.
    //
    {
        DWORD   bAppIdx;
        DWORD   dwFirst;
        DWORD   dwFullLen;
        DWORD   dwLen;

        psCtx->fAppFound = FALSE;

        for ( bAppIdx = 0 ; bAppIdx < sRsp.sIniRsp.bAppCount ; bAppIdx ++ )
        {
            if ( sRsp.sIniRsp.tsApps[bAppIdx].bAppId != 1 )
            {
                //
                // If the application ID is not 1 (which is the standard value
                // for EFC), we have to try with the remaining applications.
                //
                continue;
            }

            if ( ( ! sRsp.sIniRsp.tsApps[bAppIdx].fParmPresent ) ||
                 ( ! sRsp.sIniRsp.tsApps[bAppIdx].fEIDPresent  ) )
            {
                //
                // If the application has no parameter or EID, it can't be
                // used for EFC.
                //
                continue;
            }

            //
            // Then we have to decode the parameters. First, we have to
            // get the usefull data
            //
            dwFullLen = CENL7GetRecordSize( TagGetL7( psCtx->hTag ),
                                            sRsp.sIniRsp.tsApps[bAppIdx].tbParmsData,
                                            (DWORD)sRsp.sIniRsp.tsApps[bAppIdx].bParmsLen,
                                            &dwLen );
            if ( dwFullLen != (DWORD)sRsp.sIniRsp.tsApps[bAppIdx].bParmsLen )
            {
                //
                // The container header should describe the full bloc. If the size obtained
                // is different, we have a problem with the definition of the containers.
                //
                psCtx->dwLastErr = ERROR_INVALID_DATA;
                return FALSE;
            }

            if ( dwLen != 16 )
            {
                //
                // The parameter data must contain EFCContextMark (6 bytes), the
                // OBEGroupId (4 bytes : 2 for the container, 2 for the value), and
                // the RndOBE (6 bytes : 2 for the container, 4 for the value).
                // Note : This structure for the application parameter is specific to A1.
                // This application doesn't fit, so we have to try the next application.
                //
                continue;
            }

            //
            // Now let's store the information we'll need later
            //
            dwFirst = dwFullLen - dwLen;
            memcpy( psCtx->tbContextMark, sRsp.sIniRsp.tsApps[bAppIdx].tbParmsData + dwFirst     , 6 );
            memcpy( psCtx->tbOBEGroupId , sRsp.sIniRsp.tsApps[bAppIdx].tbParmsData + dwFirst +  8, 2 );
            memcpy( psCtx->tbRndOBE     , sRsp.sIniRsp.tsApps[bAppIdx].tbParmsData + dwFirst + 12, 4 );
            psCtx->bEID = sRsp.sIniRsp.tsApps[bAppIdx].bEID;

            //
            // At this point, we analyse the content of the EFCContextMark.
            // Be careful, the data are code with the high order bytes first
            // (unlike intel's coding).
            //

            //
            // The first 10 bits are coding the contry code of the tolling company that
            // has issued the OBE. The value should be "CN" which is coded 0111000110 in
            // binary using the 5 bits ITA2 alphabet coding (454 dec, 1C6 hex).
            //
            psCtx->dwCountry = ( (DWORD)psCtx->tbContextMark[0] << 2 ) | 
                               ( (DWORD)psCtx->tbContextMark[1] >> 6 );

            //
            // AT THE PRESENT TIME, THE COUNTRY VERIFICATION IS NOT PERFORMED
            //

            //
            // The next 14 bits are coding the identifier of the company that issued the OBE.
            //
            psCtx->dwCompany = ( ( (DWORD)psCtx->tbContextMark[1] & 0x03 ) << 12 ) | 
                                   (DWORD)psCtx->tbContextMark[2];

            //
            // The next 16 bits are coding the "TypeOfContract" value
            //
            psCtx->dwTypeOfContract = ( (DWORD)psCtx->tbContextMark[3] << 8 ) |
                                        (DWORD)psCtx->tbContextMark[4];

            //
            // The last 8 bits of the ContextMark are coding the "ContextVersion" value
            //
            psCtx->dwContextVersion = (DWORD)psCtx->tbContextMark[5];

            //
            // By the way, we initialize the RndRSE value we'll need later for authentication
            //
            *(DWORD*)(psCtx->tbRndRSE) = GetTickCount() ^ (DWORD)rand();

            psCtx->fAppFound = TRUE;
            break;
        }
    }

// NBL-2003-07-11 ++>

    return TRUE;
}


// <++ NBL-2003-07-11

//////////////////////////////////////////////////////////
//
//  ComputeAccessCredentials : 
//
//      Parameters :
//          psCtx : Context structure of the application
//
//      Description :
//          This function computes the value of the access credentials.
//
//      Return :
//          FALSE : An error has occured (error code stored in
//                  the context structure in psCtx->dwLastErr).
//          Other : success
//
BOOL WINAPI ComputeAccessCredentials( TEST_CTX * psCtx )
{
    DWORD dwLen;

    //
    // Make the derivation for the access key. The size of the
    // buffer (third parameter) make possibile for the library
    // to know how to handle the data : In that case, 2 bytes
    // means it's OBEGroupID used to derive the master element
    // access key (first of the 9 provided master keys).
    //
    if ( ! CENL7DeriveSecurity( TagGetL7( psCtx->hTag ),
                                psCtx->tbOBEGroupId,
                                sizeof(psCtx->tbOBEGroupId) ) )
    {
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }

    dwLen = sizeof( psCtx->tbAC );

    //
    // Compute the access credentials using the derived access
    // key and the rndOBE value as inputs.
    //

    if ( ! CENL7GetAccessCredentials( TagGetL7( psCtx->hTag ),
                                      psCtx->tbRndOBE,
                                      sizeof(psCtx->tbRndOBE),
                                      120,
                                      psCtx->tbAC,
                                      &dwLen) )
    {
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }

    return TRUE;                
}

// NBL-2003-07-11 ++>


/* <-- NBL-2003-07-11
//////////////////////////////////////////////////////////
//
//  GetAttributes : 
//
//      Parameters :
//          pcCtx : Context structure of the application
//
//      Description :
//          This function sends a GET request
//          to the EFC equipment.
//
//      Return :
//          FALSE : An error has occured (error code stored in
//                  the context structure in psCtx->dwLastErr).
//          Other : success
//
BOOL WINAPI GetAttributes ( TEST_CTX * psCtx )
{
    DWORD       dwErr;
    DWORD       dwCount;
    CENL7_API   sReq;
    CENL7_API   sRsp;
    BYTE        i,j;

    //
    // Prepare the status of the context structure
    // of the application.
    //
    psCtx->eLastStatus = TAG_STATUS_NONE;
    psCtx->dwLastErr = NO_ERROR;

    //
    // Fill the Get request structure
    //
    sReq.bPDU = 2;
    sReq.bFrag = 0;
    sReq.bBlockType = CENL7_APDU_GET_REQ;
    sReq.sGetReq.fAccCredPresent = FALSE;
    sReq.sGetReq.fIIDPresent = FALSE;
    sReq.sGetReq.fAttrIdListPresent = TRUE;
    sReq.sGetReq.bEID = 1;      // we suppose that application ID 1 is associated
                                // ...to EID 1 (which is not mandatory).
                                // we should have analysed the application parameters
                                // ..in the VST in order to find the appropriate value
    sReq.sGetReq.bAccCredLen = 0;
    sReq.sGetReq.bAttrIdCount = 5;
    sReq.sGetReq.tbAttrIds[0] = 0;      // ContextMark
    sReq.sGetReq.tbAttrIds[1] = 1;      // ContractSerialNumber
    sReq.sGetReq.tbAttrIds[2] = 5;      // ReceiptServicePart
    sReq.sGetReq.tbAttrIds[3] = 17;     // VehicleClass
    sReq.sGetReq.tbAttrIds[4] = 26;     // EquipmentStatus
    
    //
    // Sends the Get request to the protocol stack. 
    //
    dwErr = TagSessionRequest( psCtx->hTag, 
                               TAG_SESSION_PROCESS,
                               &sReq, 
                               1,
                               psCtx->hEvent,
                               &psCtx->dwAsyncErr );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Wait for the protocol stack to acknowledge the
    // transmission of the message. As the event is
    // automatically reset, we won't have to call
    // the ResetEvent() function later.
    //
    WaitForSingleObject( psCtx->hEvent, INFINITE );

    if ( psCtx->dwAsyncErr != NO_ERROR )
    {
        //
        // If an assynchronous error has occured, we have
        // to update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = 0x80000000 | psCtx->dwAsyncErr;
        return FALSE;
    }

    //
    // Wait for a response according to the timeout
    // paremeter in the context structure.
    //
    dwCount = 1 | TAG_TRUNCATE_RESPONSE;
    dwErr = TagResponse( psCtx->hTag, 
                         &sRsp, 
                         &dwCount, 
                         &psCtx->eLastStatus, 
                         psCtx->dwRspTimeout );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Make sure the response has the appropriated
    // count of frames and that this is the expected
    // type.
    //
    if ( ( dwCount         != 1                  ) || 
         ( sRsp.bBlockType != CENL7_APDU_GET_RSP ) )
    {
        //
        // The response is not coherent regarding
        // the request.
        //
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }

    __try
    {
        if ( sRsp.sGetRsp.fStatusPresent && ( sRsp.sGetRsp.bStatus != 0 ) )
        {
            printf ("\n GET Response status %u returned by the OBE", (DWORD) sRsp.sGetRsp.bStatus);
            __leave;
        }

        if ( !sRsp.sGetRsp.fAttrListPresent )
        {
            printf ("\n missing attribute list in the Get Response");
            __leave;
        }


        psCtx->fGetOk = TRUE;

        printf ("\n attribute list in the Get Response:");
        for (i=0; i<sRsp.sGetRsp.bAttrCount; i++)
        {
            printf ("\n attribute ID = %u : " , (DWORD) sRsp.sGetRsp.tsAttrs[i].bAttrId );
            for (j=0; j<sRsp.sGetRsp.tsAttrs[i].bAttrLen ; j++)
            {
                printf (" %02x" , (DWORD) sRsp.sGetRsp.tsAttrs[i].tbAttrData[j] );
            }                
        }

    }
    __finally{};

    return TRUE;
}
   NBL-2003-07-11 --> */


// <++ NBL-2003-07-11

//////////////////////////////////////////////////////////
//
//  GetStampedAndGet : 
//
//      Parameters :
//          pcCtx : Context structure of the application
//
//      Description :
//          This function sends a GETSTAMPED and a GET request
//          to the EFC equipment (the requests are concatened).
//
//      Return :
//          FALSE : An error has occured (error code stored in
//                  the context structure in psCtx->dwLastErr).
//          Other : success
//
BOOL WINAPI GetStampedAndGet( TEST_CTX * psCtx )
{
    DWORD            dwErr;
    DWORD            dwCount;
    DWORD            dwPos;
    DWORD            dwLen;
    DWORD            dwFullLen;
    DWORD            dwMACLen;
    BYTE             bAttrIdx;
    BYTE             bByteIdx;
    BOOL             fResult;
    BOOL             fContractSerialNumber = FALSE;
    CENL7_API        tsReq[2];
    CENL7_API        tsRsp[2];
    CENL7_ACT_PARAMS sActPrm;
    BYTE             tbDerive[7];
    BYTE             tbMsg[16];
    BYTE             tbMAC[4];

    //
    // Prepare the status of the context structure
    // of the application.
    //
    psCtx->eLastStatus = TAG_STATUS_NONE;
    psCtx->dwLastErr = NO_ERROR;

    //
    // ///////////////////////////////////////////
    //
    // Fill the GET-STAMPED action request structure
    //
    tsReq[0].bPDU = 2;
    tsReq[0].bFrag = 0;
    tsReq[0].bBlockType = CENL7_APDU_ACT_REQ;
    tsReq[0].sActReq.fAccCredPresent = FALSE; // TRUE;
    tsReq[0].sActReq.bAccCredLen = 0; // sizeof(psCtx->tbAC);
    // memcpy( tsReq[0].sActReq.tbAccCred, psCtx->tbAC, sizeof(psCtx->tbAC) );
    tsReq[0].sActReq.fParmsPresent = TRUE;
    tsReq[0].sActReq.fIIDPresent = FALSE;
    tsReq[0].sActReq.fConfirmed = TRUE;
    tsReq[0].sActReq.bEID = psCtx->bEID;
    tsReq[0].sActReq.bActionType = CENL7_ACTCODE_GET_STAMPED;

    //
    // Buid the action parameters to make a GET-STAMPED request
    //
    sActPrm.sGStReq.bAttrIdCount = 1;
    sActPrm.sGStReq.tbAttrIds[0] = 16;  // EquipmentStatus Attribute
    sActPrm.sGStReq.bKeyId = 111;       // First element authentication key
    sActPrm.sGStReq.bRndLen = sizeof( psCtx->tbRndRSE );
    memcpy( sActPrm.sGStReq.tbRnd, psCtx->tbRndRSE, sizeof( psCtx->tbRndRSE ) );
    
    dwPos = 0;
    dwLen = sizeof(tsReq[0].sActReq.tbParmsData);
    fResult = CENL7BuildActionParams( TagGetL7( psCtx->hTag ),
                                      tsReq[0].sActReq.tbParmsData,
                                      &dwPos,
                                      &dwLen,
                                      CENL7_ACTCODE_GET_STAMPED,
                                      &sActPrm );
    if ( ! fResult )
    {
        //
        // Update the status in the context structure
        // and return a generic error.
        //
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }                                 

    //
    // Finish the filling of the action request structure
    //
    tsReq[0].sActReq.bParmsLen = (BYTE)dwPos;

    //
    // ///////////////////////////////////////////
    //
    // Fill the Get request structure
    //
    tsReq[1].bPDU = 3; // Previous concatened message + 1
    tsReq[1].bFrag = 0;
    tsReq[1].bBlockType = CENL7_APDU_GET_REQ;

    //
    // Now we are using the access credentials we have calculated
    //
    tsReq[1].sGetReq.fAccCredPresent = FALSE; // TRUE;
    tsReq[1].sGetReq.bAccCredLen = 0; // sizeof(psCtx->tbAC);
    // memcpy( tsReq[1].sGetReq.tbAccCred, psCtx->tbAC, sizeof(psCtx->tbAC) );
    tsReq[1].sGetReq.fIIDPresent = FALSE;
    tsReq[1].sGetReq.fAttrIdListPresent = TRUE;
    tsReq[1].sGetReq.bEID = psCtx->bEID;  // Now we use the EID associated to the
    tsReq[1].sGetReq.bAttrIdCount = 3;
    // tsReq[1].sGetReq.tbAttrIds[0] = 1;      // ContractSerialNumber
    // tsReq[1].sGetReq.tbAttrIds[1] = 5;      // ReceiptServicePart
    tsReq[1].sGetReq.tbAttrIds[0] = 16;     // VehicleLicencePlateNumber
    tsReq[1].sGetReq.tbAttrIds[1] = 17;     // VehicleClass
    tsReq[1].sGetReq.tbAttrIds[2] = 26;     // VehicleClass

    //
    // Sends the concatened GetStamped and Get requests to the protocol stack. 
    //
    dwErr = TagSessionRequest( psCtx->hTag, 
                               TAG_SESSION_PROCESS,
                               tsReq, 
                               2,
                               psCtx->hEvent,
                               &psCtx->dwAsyncErr );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Wait for the protocol stack to acknowledge the
    // transmission of the message. As the event is
    // automatically reset, we won't have to call
    // the ResetEvent() function later.
    //
    WaitForSingleObject( psCtx->hEvent, INFINITE );

    if ( psCtx->dwAsyncErr != NO_ERROR )
    {
        //
        // If an assynchronous error has occured, we have
        // to update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = 0x80000000 | psCtx->dwAsyncErr;
        return FALSE;
    }

    //
    // Wait for a response according to the timeout
    // paremeter in the context structure.
    //
    dwCount = 2 | TAG_TRUNCATE_RESPONSE;
    dwErr = TagResponse( psCtx->hTag, 
                         tsRsp, 
                         &dwCount, 
                         &psCtx->eLastStatus, 
                         psCtx->dwRspTimeout );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Make sure the response has the appropriated
    // count of frames and that this is the expected
    // type.
    //
    if ( ( dwCount             != 2                  ) || 
         ( tsRsp[0].bBlockType != CENL7_APDU_ACT_RSP ) ||
         ( tsRsp[1].bBlockType != CENL7_APDU_GET_RSP ) )
    {
        //
        // The response is not coherent regarding
        // the request.
        //
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }

    __try
    {
        //
        // Make sure the content of the GET-STAMPED response is ok and contains
        // some parameters
        //

        if ( tsRsp[0].sActRsp.fStatusPresent && ( tsRsp[0].sActRsp.bStatus != 0 ) )
        {
            printf ("\n GET-STAMPED Response status %u returned by the OBE", (DWORD) tsRsp[0].sActRsp.bStatus);
            __leave;
        }

        if ( ! tsRsp[0].sActRsp.fParmsPresent )
        {
            printf ("\n Missing attribute list in the GET-STAMPED Response");
            __leave;
        }

        //
        // Decode the parameters of the GET-STAMPED reponse
        //

        dwPos = 0;
        dwLen = tsRsp[0].sActRsp.bParmsLen;
        fResult = CENL7AnalyseActionParams( TagGetL7( psCtx->hTag ),
                                            tsRsp[0].sActRsp.tbParmsData,
                                            &dwPos,
                                            &dwLen,
                                            CENL7_ACTCODE_GET_STAMPED,
                                            &sActPrm );
        if ( ! fResult )
        {
            printf ("\n Struture error in the GET-STAMPED result");
            __leave;
        }

        //
        // Print the content
        //

        printf ("\nAttribute list in the GET-STAMPED response:");
        for ( bAttrIdx = 0 ; bAttrIdx < sActPrm.sGStRsp.bAttrCount ; bAttrIdx++ )
        {
            printf ("\n Attribute ID = %u : " , (DWORD) sActPrm.sGStRsp.tsAttrs[bAttrIdx].bAttrId );
            for ( bByteIdx =0 ; bByteIdx < sActPrm.sGStRsp.tsAttrs[bAttrIdx].bAttrLen ; bByteIdx++ )
                printf (" %02x" , (DWORD) sActPrm.sGStRsp.tsAttrs[bAttrIdx].tbAttrData[bByteIdx] );
        }


        //
        // Make sure the content of the GET response is ok and contains
        // some parameters
        //

        if ( tsRsp[1].sGetRsp.fStatusPresent && ( tsRsp[1].sGetRsp.bStatus != 0 ) )
        {
            printf ("\n GET Response status %u returned by the OBE", (DWORD) tsRsp[1].sGetRsp.bStatus);
            __leave;
        }

        if ( !tsRsp[1].sGetRsp.fAttrListPresent )
        {
            printf ("\n Missing attribute list in the Get Response");
            __leave;
        }

        printf ("\nAttribute list in the Get Response:");
        for ( bAttrIdx = 0 ; bAttrIdx < tsRsp[1].sGetRsp.bAttrCount ; bAttrIdx++)
        {
            printf ("\n Attribute ID = %u : " , (DWORD) tsRsp[1].sGetRsp.tsAttrs[bAttrIdx].bAttrId );
            for ( bByteIdx = 0 ; bByteIdx < tsRsp[1].sGetRsp.tsAttrs[bAttrIdx].bAttrLen ; bByteIdx++ )
                printf (" %02x" , (DWORD) tsRsp[1].sGetRsp.tsAttrs[bAttrIdx].tbAttrData[bByteIdx] );

            //
            // By the way, find the ContractSerialNumber in order to store it, we need it
            // to derive the master authentication key in order to verify the MAC.
            //
            if ( tsRsp[1].sGetRsp.tsAttrs[bAttrIdx].bAttrId == 1 )
            {
                memcpy( psCtx->tbContractSerialNumber, 
                        tsRsp[1].sGetRsp.tsAttrs[bAttrIdx].tbAttrData,
                        sizeof(psCtx->tbContractSerialNumber) );
                fContractSerialNumber = TRUE;
            }
        }

        //
        //////////////////////////////////////////////////
        //
        // NOW THIS IS THE HARD CRYPTOGRAPHIC PROCESSING FOR
        // THE AUTHENTICATION OF THE OBE
        //

        //
        // In order to perform the derivation of the master key,
        // we need the value of ContractSerialNumber
        //
        if ( ! fContractSerialNumber )
        {
            printf ("\n ContractSerialNumber attribute not found in the Get response");
            __leave;
        }

        //
        // The derivation input data are made out of the 4 bytes of
        // the ContractSerialNumber, and the 3 first bytes of the
        // ContextMark (ContracProvider = ProviderCountry+ProviderId).
        //
        memcpy( tbDerive + 0, psCtx->tbContractSerialNumber, 4 );
        memcpy( tbDerive + 4, psCtx->tbContextMark         , 3 );

        //
        // Make the derivation for the authentication keys.
        // The size of the buffer (third parameter) make possibile for
        // the library to know how to handle the data : In that case,
        // 7 bytes means it's ContractSerialNumber and ContractProvider
        // used to derive the master element autentication keys
        // (8 last keys of the 9 provided master keys).
        //
        if ( ! CENL7DeriveSecurity( TagGetL7( psCtx->hTag ),
                                    tbDerive,
                                    sizeof(tbDerive) ) )
        {
            printf ("\n Master key derivation error");
            __leave;
        }

        //
        // Know we have to find the data over which the signature (MAC)
        // is going to be calculated
        //
        dwFullLen = CENL7GetRecordSize( TagGetL7( psCtx->hTag ),
                                        tsRsp[0].sActRsp.tbParmsData,
                                        (DWORD)tsRsp[0].sActRsp.bParmsLen,
                                        &dwLen );
        if ( ( dwFullLen != (DWORD)tsRsp[0].sActRsp.bParmsLen ) ||
             ( dwLen < 4                                      ) ||
             ( dwLen > sizeof(tbMsg)                          ) )

        {
            printf ("\n GetStamped verification error (structure analysis)");
            __leave;
        }

        //
        // We just copy the data to be authenticated and we replace the MAC
        // value (returned by the OBE) by the RndRSE value
        //
        memcpy( tbMsg, tsRsp[0].sActRsp.tbParmsData + dwFullLen - dwLen, dwLen );
        memcpy( tbMsg + dwLen - 4, psCtx->tbRndRSE, 4 );

        //
        // Then we can calculate the MAC
        //
        dwMACLen = sizeof(tbMAC);
        if ( ! CENL7GetAuthenticator( TagGetL7( psCtx->hTag ),
                                      tbMsg,
                                      dwLen,
                                      111,
                                      tbMAC,
                                      &dwMACLen ) )
        {
            printf ("\n Authenticator calculation error");
            __leave;
        }

        //
        // Finally, we compare our MAC with the one provided by the OBE.
        // If they match, the OBE is authentic.
        //
        if ( ( dwMACLen != (DWORD)sActPrm.sGStRsp.bMacLen            ) ||
             ( memcmp( tbMAC, sActPrm.sGStRsp.tbMac, dwMACLen ) != 0 ) )
            printf( "\n THE MAC DOES NOT MATCH, THE AUTHENTICATION FAILED !" );
        else
            printf( "\n THE OBE IS AUTHENTICATED !" );

        psCtx->fGetOk = TRUE;

    }
    __finally{};

    return TRUE;
}
// NBL-2003-07-11 ++>



//////////////////////////////////////////////////////////
//
//  SetAttributes : 
//
//      Parameters :
//          pcCtx : Context structure of the application
//
//      Description :
//          This function sends a SET request
//          to the EFC equipment.
//
//      Return :
//          FALSE : An error has occured (error code stored in
//                  the context structure in psCtx->dwLastErr).
//          Other : success
//
BOOL WINAPI SetAttributes ( TEST_CTX * psCtx )
{
    DWORD       dwErr;
    DWORD       dwCount;
    CENL7_API   sReq;
    CENL7_API   sRsp;

    //
    // Prepare the status of the context structure
    // of the application.
    //
    psCtx->eLastStatus = TAG_STATUS_NONE;
    psCtx->dwLastErr = NO_ERROR;

    //
    // Fill the Set request structure
    //
    sReq.bPDU = 2;
    sReq.bFrag = 0;
    sReq.bBlockType = CENL7_APDU_SET_REQ;

/* <-- NBL-2003-07-11
    sReq.sSetReq.fAccCredPresent = FALSE;
   NBL-2003-07-11 --> */
// <++ NBL-2003-07-11
    //
    // Now we are using the access credentials we have calculated
    //
    sReq.sSetReq.fAccCredPresent = TRUE;
    sReq.sSetReq.bAccCredLen = sizeof(psCtx->tbAC);
    memcpy( sReq.sSetReq.tbAccCred, psCtx->tbAC, sizeof(psCtx->tbAC) );
// NBL-2003-07-11 ++>
    sReq.sSetReq.fIIDPresent = FALSE;
    sReq.sSetReq.fConfirmed = TRUE;
    sReq.sSetReq.bEID = 1;      // we suppose that application ID 1 is associated
                                // ...to EID 1 (which is not mandatory).
                                // we should have analysed the application parameters
                                // ..in the VST in order to find the appropriate value

/* <-- NBL-2003-07-11
    sReq.sSetReq.bAccCredLen = 0; 
// NBL-2003-07-11 --> */
    sReq.sSetReq.bAttrCount = 2;

    sReq.sSetReq.tsAttrs[0].bAttrId = 5;            // ReceiptServicePart
    sReq.sSetReq.tsAttrs[0].bAttrLen = 13;          // 
    memcpy( sReq.sSetReq.tsAttrs[0].tbAttrData, "ABCDEFGHIJKLM", 13 );

    sReq.sSetReq.tsAttrs[1].bAttrId = 26;           // EquipmentStatus
    sReq.sSetReq.tsAttrs[1].bAttrLen = 2;           // 
    sReq.sSetReq.tsAttrs[1].tbAttrData[0] = '1';    // 
    sReq.sSetReq.tsAttrs[1].tbAttrData[1] = '2';    // 



    //
    // Sends the Get request to the protocol stack. 
    //
    dwErr = TagSessionRequest( psCtx->hTag, 
                               TAG_SESSION_PROCESS,
                               &sReq, 
                               1,
                               psCtx->hEvent,
                               &psCtx->dwAsyncErr );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Wait for the protocol stack to acknowledge the
    // transmission of the message. As the event is
    // automatically reset, we won't have to call
    // the ResetEvent() function later.
    //
    WaitForSingleObject( psCtx->hEvent, INFINITE );

    if ( psCtx->dwAsyncErr != NO_ERROR )
    {
        //
        // If an assynchronous error has occured, we have
        // to update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = 0x80000000 | psCtx->dwAsyncErr;
        return FALSE;
    }

    //
    // Wait for a response according to the timeout
    // paremeter in the context structure.
    //
    dwCount = 1 | TAG_TRUNCATE_RESPONSE;
    dwErr = TagResponse( psCtx->hTag, 
                         &sRsp, 
                         &dwCount, 
                         &psCtx->eLastStatus, 
                         psCtx->dwRspTimeout );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Make sure the response has the appropriated
    // count of frames and that this is the expected
    // type.
    //
    if ( ( dwCount         != 1                  ) || 
         ( sRsp.bBlockType != CENL7_APDU_SET_RSP ) )
    {
        //
        // The response is not coherent regarding
        // the request.
        //
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }

    __try
    {
        if ( sRsp.sSetRsp.fStatusPresent && ( sRsp.sSetRsp.bStatus != 0 ) )
        {
            printf ("\n SET Response status %u returned by the OBE", (DWORD) sRsp.sSetRsp.bStatus);
            __leave;
        }

        psCtx->fSetOk = TRUE;

        printf ("\n SET request succesfull");

    }
    __finally{};

    return TRUE;
}




//////////////////////////////////////////////////////////
//
//  SetMMI : 
//
//      Parameters :
//          pcCtx : Context structure of the application
//
//      Description :
//          This function sends an set-mmi action request
//          to the EFC equipment.
//
//      Return :
//          FALSE : An error has occured (error code stored in
//                  the context structure in psCtx->dwLastErr).
//          Other : success
//
BOOL WINAPI SetMMI( TEST_CTX * psCtx )
{
    BOOL        fResult;
    DWORD       dwErr;
    DWORD       dwCount;
    DWORD       dwPos;
    DWORD       dwLen;
    CENL7_API   sReq;
    CENL7_API   sRsp;
    CENL7_ACT_PARAMS sActPrm;

    //
    // Prepare the status of the context structure
    // of the application.
    //
    psCtx->eLastStatus = TAG_STATUS_NONE;
    psCtx->dwLastErr = NO_ERROR;

    //
    // Fill the set-mmi action request structure
    //
    sReq.bPDU = 2;
    sReq.bFrag = 0;
    sReq.bBlockType = CENL7_APDU_ACT_REQ;
    sReq.sActReq.fAccCredPresent = FALSE;
    sReq.sActReq.fParmsPresent = TRUE;
    sReq.sActReq.fIIDPresent = FALSE;
    sReq.sActReq.fConfirmed = TRUE;
    sReq.sActReq.bEID = 0;
    sReq.sActReq.bActionType = CENL7_ACTCODE_SET_MMI;

    //
    // Buid the action parameters to make a SET_MMI request
    //

    if (psCtx->fGetOk)
    {
        if (psCtx->fSetOk)
        {
            sActPrm.sMmiReq.bMMISignal = 0;         // full success
        }
        else
        {
            sActPrm.sMmiReq.bMMISignal = 2;         // partially failed
        }
    }
    else
    {
        if (psCtx->fSetOk)
        {
            sActPrm.sMmiReq.bMMISignal = 2;         // partially failed
        }
        else
        {
            sActPrm.sMmiReq.bMMISignal = 1;         // totally failed
        }
    }

    
    dwPos = 0;
    dwLen = sizeof(sReq.sActReq.tbParmsData);
    fResult = CENL7BuildActionParams( TagGetL7( psCtx->hTag ),
                                      sReq.sActReq.tbParmsData,
                                      &dwPos,
                                      &dwLen,
                                      CENL7_ACTCODE_SET_MMI,
                                      &sActPrm );
    if ( ! fResult )
    {
        //
        // Update the status in the context structure
        // and return a generic error.
        //
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }                                 

    //
    // Finish the filling of the action request structure
    //
    sReq.sActReq.bParmsLen = (BYTE)dwPos;

    //
    // Sends the request to the protocol stack. 
    //
    dwErr = TagSessionRequest( psCtx->hTag, 
                               TAG_SESSION_PROCESS,
                               &sReq, 
                               1,
                               psCtx->hEvent,
                               &psCtx->dwAsyncErr );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Wait for the protocol stack to acknowledge the
    // transmission of the message. As the event is
    // automatically reset, we won't have to call
    // the ResetEvent() function later.
    //
    WaitForSingleObject( psCtx->hEvent, INFINITE );

    if ( psCtx->dwAsyncErr != NO_ERROR )
    {
        //
        // If an assynchronous error has occured, we have
        // to update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = 0x80000000 | psCtx->dwAsyncErr;
        return FALSE;
    }

    //
    // Wait for a response according to the timeout
    // paremeter in the context structure.
    //
    dwCount = 1 | TAG_TRUNCATE_RESPONSE;
    dwErr = TagResponse( psCtx->hTag, 
                         &sRsp, 
                         &dwCount, 
                         &psCtx->eLastStatus, 
                         psCtx->dwRspTimeout );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Make sure the response has the appropriated
    // count of frames and that this is the expected
    // type.
    //
    if ( ( dwCount         != 1                  ) || 
         ( sRsp.bBlockType != CENL7_APDU_ACT_RSP ) )
    {
        //
        // The response is not coherent regarding
        // the request.
        //
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }

    return TRUE;
}



//////////////////////////////////////////////////////////
//
//  TerminateTransaction : 
//
//      Parameters :
//          pcCtx : Context structure of the application
//
//      Description :
//          This function sends an event-report request
//          to the EFC equipment.
//
//      Return :
//          FALSE : An error has occured (error code stored in
//                  the context structure in psCtx->dwLastErr).
//          Other : success
//
BOOL WINAPI TerminateTransaction( TEST_CTX * psCtx )
{
    DWORD       dwErr;
    DWORD       dwCount;
    CENL7_API   sReq;
    CENL7_API   sRsp;

    //
    // Prepare the status of the context structure
    // of the application.
    //
    psCtx->eLastStatus = TAG_STATUS_NONE;
    psCtx->dwLastErr = NO_ERROR;

    //
    // Fill the event-report action request structure
    //
    sReq.bPDU = 2;
    sReq.bFrag = 0;
    sReq.bBlockType = CENL7_APDU_EVT_REQ;
    sReq.sEvtReq.fAccCredPresent = FALSE;
    sReq.sEvtReq.fParmsPresent = FALSE;
    sReq.sEvtReq.fIIDPresent = FALSE;
    sReq.sEvtReq.fConfirmed = FALSE;
    sReq.sEvtReq.bEID = 0;
    sReq.sEvtReq.bEventType = CENL7_EVENT_CLOSE_LINK;

    //
    // Sends the request to the protocol stack. It is sent
    // as a ending session request.
    //
    dwErr = TagSessionRequest( psCtx->hTag, 
                               TAG_SESSION_END,
                               &sReq, 
                               1,
                               psCtx->hEvent,
                               &psCtx->dwAsyncErr );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Wait for the protocol stack to acknowledge the
    // transmission of the message. As the event is
    // automatically reset, we won't have to call
    // the ResetEvent() function later.
    //
    WaitForSingleObject( psCtx->hEvent, INFINITE );

    if ( psCtx->dwAsyncErr != NO_ERROR )
    {
        //
        // If an assynchronous error has occured, we have
        // to update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = 0x80000000 | psCtx->dwAsyncErr;
        return FALSE;
    }

    //
    // Wait for a response according to the timeout
    // paremeter in the context structure.
    //
    dwCount = 1 | TAG_TRUNCATE_RESPONSE;
    dwErr = TagResponse( psCtx->hTag, 
                         &sRsp, 
                         &dwCount, 
                         &psCtx->eLastStatus, 
                         5 );
    if ( dwErr != NO_ERROR )
    {
        //
        // Update the status in the context structure
        // and return an error.
        //
        psCtx->dwLastErr = dwErr;
        return FALSE;
    }

    //
    // Make sure the response has the appropriated
    // count of frames and that this is the expected
    // type.
    //
    if ( ( dwCount         != 1                  ) || 
         ( sRsp.bBlockType != CENL7_APDU_EVT_RSP ) )
    {
        psCtx->dwLastErr = ERROR_INVALID_DATA;
        return FALSE;
    }

    return TRUE;
}



//////////////////////////////////////////////////////////
//
//  main : 
//
//      Parameters :
//          Refer to C documentation
//
//      Description :
//          Application entry point
//
//      Return :
//          0 : no error
//          Other : error code
//
#define OTHERSTRUCT(x)    printf("\n     :%s", #x );
#define BEGINSTRUCT(x)    printf("\nBegin:%s", #x ); dwPos = (DWORD)0;
#define STRUCTFIELD(x,y)  if ( dwPos != (DWORD)(&(((x*)(NULL))->y)) )\
                          printf("\n  %-20s : %4u", "#Gap", (DWORD)(&(((x*)(NULL))->y)) - dwPos );\
                          printf("\n  %-20s : %4u", #y, sizeof( ((x*)(NULL))->y ) );\
                          dwPos=(DWORD)(&(((x*)(NULL))->y))+sizeof( ((x*)(NULL))->y );
#define STRUCTTOEND(x,y)  if ( dwPos != (DWORD)(&(((x*)(NULL))->y)) )\
                          printf("\n  %-20s : %4u", "#Gap", (DWORD)(&(((x*)(NULL))->y)) - dwPos );\
                          dwPos=(DWORD)(&(((x*)(NULL))->y));\
                          printf("\n  %-20s : %4u", "union", sizeof(x) - dwPos );\
                          dwPos = sizeof(x);
#define UNIONITEM(x,y)    if ( dwPos != (DWORD)(&(((x*)(NULL))->y)) )\
                          printf("\n  %-20s : %4u", "#Gap", (DWORD)(&(((x*)(NULL))->y)) - dwPos );\
                          dwPos=(DWORD)(&(((x*)(NULL))->y));\
                          printf("\n  + %-18s : %4u (union)", #y, sizeof( ((x*)(NULL))->y ) );
#define ENDSTRUCT(x)      if ( dwPos != sizeof(x) )\
                          printf("\n  %-20s : %4u", "#Gap", sizeof(x) - dwPos );\
                          printf("\nEnd (%u bytes)\n\n", sizeof(x) );


int main( IN int iArgc, 
          IN char ** ppcArgv )
{

    DWORD       dwErr;
    DWORD       dwPos;
    TEST_CTX    sCtx;
    BOOL        fByPass;
	
	int i;

    BEGINSTRUCT(TRSP_API)
    STRUCTFIELD(TRSP_API, bActionType );
    STRUCTFIELD(TRSP_API, sTrspReq );
    // UNIONITEM(TRSP_API, sTrspRsp );
    // STRUCTTOEND(TRSP_API, sTrspRsp );
    ENDSTRUCT(TRSP_API)

    _getch();


    /*    CENL7_API sAPI;

    BEGINSTRUCT(CENL7_APPLICATION)
    STRUCTFIELD(CENL7_APPLICATION,fEIDPresent)
    STRUCTFIELD(CENL7_APPLICATION,fParmPresent)
    STRUCTFIELD(CENL7_APPLICATION,bAppId)
    STRUCTFIELD(CENL7_APPLICATION,bEID)
    STRUCTFIELD(CENL7_APPLICATION,bParmsLen)
    STRUCTFIELD(CENL7_APPLICATION,tbParmsData)
    ENDSTRUCT  (CENL7_APPLICATION)

    BEGINSTRUCT(CENL7_ATTRIBUTE)
    STRUCTFIELD(CENL7_ATTRIBUTE,bAttrId)
    STRUCTFIELD(CENL7_ATTRIBUTE,bAttrLen)
    STRUCTFIELD(CENL7_ATTRIBUTE,tbAttrData)
    ENDSTRUCT  (CENL7_ATTRIBUTE)


    BEGINSTRUCT(CENL7_INI_REQ)
    STRUCTFIELD(CENL7_INI_REQ,fOptAppPresent)
    STRUCTFIELD(CENL7_INI_REQ,wManufacturer)
    STRUCTFIELD(CENL7_INI_REQ,dwBeaconId)
    STRUCTFIELD(CENL7_INI_REQ,dwTime)
    STRUCTFIELD(CENL7_INI_REQ,bProf)
    STRUCTFIELD(CENL7_INI_REQ,bAppCount)
    STRUCTFIELD(CENL7_INI_REQ,tsApps)
    STRUCTFIELD(CENL7_INI_REQ,bOptAppCount)
    STRUCTFIELD(CENL7_INI_REQ,tsOptApps)
    STRUCTFIELD(CENL7_INI_REQ,bProfCount)
    STRUCTFIELD(CENL7_INI_REQ,tbProfs)
    ENDSTRUCT  (CENL7_INI_REQ)
    
    BEGINSTRUCT(CENL7_INI_RSP)
    STRUCTFIELD(CENL7_INI_RSP,bProf)
    STRUCTFIELD(CENL7_INI_RSP,bAppCount)
    STRUCTFIELD(CENL7_INI_RSP,tsApps)
    STRUCTFIELD(CENL7_INI_RSP,fOBEStatusPresent)
    STRUCTFIELD(CENL7_INI_RSP,wEqtClass)
    STRUCTFIELD(CENL7_INI_RSP,wManufacturerId)
    STRUCTFIELD(CENL7_INI_RSP,bOBEStatus)
    STRUCTFIELD(CENL7_INI_RSP,bPrivate)
    ENDSTRUCT  (CENL7_INI_RSP)

    BEGINSTRUCT(CENL7_GET_REQ)
    STRUCTFIELD(CENL7_GET_REQ,fAccCredPresent)
    STRUCTFIELD(CENL7_GET_REQ,fIIDPresent)
    STRUCTFIELD(CENL7_GET_REQ,fAttrIdListPresent)
    STRUCTFIELD(CENL7_GET_REQ,bEID)
    STRUCTFIELD(CENL7_GET_REQ,bAccCredLen)
    STRUCTFIELD(CENL7_GET_REQ,tbAccCred )
    STRUCTFIELD(CENL7_GET_REQ,bIID)
    STRUCTFIELD(CENL7_GET_REQ,bAttrIdCount)
    STRUCTFIELD(CENL7_GET_REQ,tbAttrIds)
    ENDSTRUCT  (CENL7_GET_REQ)

    BEGINSTRUCT(CENL7_GET_RSP)
    STRUCTFIELD(CENL7_GET_RSP,fIIDPresent)
    STRUCTFIELD(CENL7_GET_RSP,fAttrListPresent)
    STRUCTFIELD(CENL7_GET_RSP,fStatusPresent)
    STRUCTFIELD(CENL7_GET_RSP,bEID)
    STRUCTFIELD(CENL7_GET_RSP,bIID)
    STRUCTFIELD(CENL7_GET_RSP,bAttrCount)
    STRUCTFIELD(CENL7_GET_RSP,tsAttrs)
    STRUCTFIELD(CENL7_GET_RSP,bStatus)
    ENDSTRUCT  (CENL7_GET_RSP)

    BEGINSTRUCT(CENL7_SET_REQ)
    STRUCTFIELD(CENL7_SET_REQ,fAccCredPresent)
    STRUCTFIELD(CENL7_SET_REQ,fIIDPresent)
    STRUCTFIELD(CENL7_SET_REQ,fConfirmed)
    STRUCTFIELD(CENL7_SET_REQ,bEID)
    STRUCTFIELD(CENL7_SET_REQ,bAccCredLen)
    STRUCTFIELD(CENL7_SET_REQ,tbAccCred)
    STRUCTFIELD(CENL7_SET_REQ,bIID)
    STRUCTFIELD(CENL7_SET_REQ,bAttrCount)
    STRUCTFIELD(CENL7_SET_REQ,tsAttrs)
    ENDSTRUCT  (CENL7_SET_REQ)

    BEGINSTRUCT(CENL7_SET_RSP)
    STRUCTFIELD(CENL7_SET_RSP,fIIDPresent)
    STRUCTFIELD(CENL7_SET_RSP,fStatusPresent)
    STRUCTFIELD(CENL7_SET_RSP,bEID)
    STRUCTFIELD(CENL7_SET_RSP,bIID)
    STRUCTFIELD(CENL7_SET_RSP,bStatus)
    ENDSTRUCT  (CENL7_SET_RSP)

    BEGINSTRUCT(CENL7_ACT_REQ)
    STRUCTFIELD(CENL7_ACT_REQ,fAccCredPresent)
    STRUCTFIELD(CENL7_ACT_REQ,fParmsPresent)
    STRUCTFIELD(CENL7_ACT_REQ,fIIDPresent)
    STRUCTFIELD(CENL7_ACT_REQ,fConfirmed)
    STRUCTFIELD(CENL7_ACT_REQ,bEID)
    STRUCTFIELD(CENL7_ACT_REQ,bAccCredLen)
    STRUCTFIELD(CENL7_ACT_REQ,tbAccCred)
    STRUCTFIELD(CENL7_ACT_REQ,bActionType)
    STRUCTFIELD(CENL7_ACT_REQ,bParmsLen)
    STRUCTFIELD(CENL7_ACT_REQ,tbParmsData)
    STRUCTFIELD(CENL7_ACT_REQ,bIID)
    ENDSTRUCT  (CENL7_ACT_REQ)

    BEGINSTRUCT(CENL7_ACT_RSP)
    STRUCTFIELD(CENL7_ACT_RSP,fIIDPresent)
    STRUCTFIELD(CENL7_ACT_RSP,fParmsPresent)
    STRUCTFIELD(CENL7_ACT_RSP,fStatusPresent)
    STRUCTFIELD(CENL7_ACT_RSP,bEID)
    STRUCTFIELD(CENL7_ACT_RSP,bIID)
    STRUCTFIELD(CENL7_ACT_RSP,bParmsLen)
    STRUCTFIELD(CENL7_ACT_RSP,tbParmsData)
    STRUCTFIELD(CENL7_ACT_RSP,bStatus)
    ENDSTRUCT  (CENL7_ACT_RSP)
    
    BEGINSTRUCT(CENL7_EVT_REQ)
    STRUCTFIELD(CENL7_EVT_REQ,fAccCredPresent)
    STRUCTFIELD(CENL7_EVT_REQ,fParmsPresent)
    STRUCTFIELD(CENL7_EVT_REQ,fIIDPresent)
    STRUCTFIELD(CENL7_EVT_REQ,fConfirmed)
    STRUCTFIELD(CENL7_EVT_REQ,bEID)
    STRUCTFIELD(CENL7_EVT_REQ,bAccCredLen)
    STRUCTFIELD(CENL7_EVT_REQ,tbAccCred)
    STRUCTFIELD(CENL7_EVT_REQ,bEventType)
    STRUCTFIELD(CENL7_EVT_REQ,bParmsLen)
    STRUCTFIELD(CENL7_EVT_REQ,tbParmsData)
    STRUCTFIELD(CENL7_EVT_REQ,bIID)
    ENDSTRUCT  (CENL7_EVT_REQ)
    
    BEGINSTRUCT(CENL7_EVT_RSP)
    STRUCTFIELD(CENL7_EVT_RSP,fIIDPresent)
    STRUCTFIELD(CENL7_EVT_RSP,fStatusPresent)
    STRUCTFIELD(CENL7_EVT_RSP,bEID)
    STRUCTFIELD(CENL7_EVT_RSP,bIID)
    STRUCTFIELD(CENL7_EVT_RSP,bStatus)
    ENDSTRUCT  (CENL7_EVT_REQ)

    BEGINSTRUCT(CENL7_API)
    STRUCTFIELD(CENL7_API,bPDU)
    STRUCTFIELD(CENL7_API,bFrag)
    STRUCTFIELD(CENL7_API,bBlockType)
    UNIONITEM  (CENL7_API,sIniReq)
    UNIONITEM  (CENL7_API,sIniRsp)
    UNIONITEM  (CENL7_API,sGetReq)
    UNIONITEM  (CENL7_API,sGetRsp)
    UNIONITEM  (CENL7_API,sSetReq)
    UNIONITEM  (CENL7_API,sSetRsp)
    UNIONITEM  (CENL7_API,sActReq)
    UNIONITEM  (CENL7_API,sActRsp)
    UNIONITEM  (CENL7_API,sEvtReq)
    UNIONITEM  (CENL7_API,sEvtRsp)
    STRUCTTOEND(CENL7_API,sIniReq)
    ENDSTRUCT  (CENL7_API)

    BEGINSTRUCT(CENL7_ACT_MMI_REQ)
    STRUCTFIELD(CENL7_ACT_MMI_REQ,bMMISignal)
    ENDSTRUCT  (CENL7_ACT_MMI_REQ)

    BEGINSTRUCT(CENL7_ACT_CHN_REQ)
    OTHERSTRUCT(CENL7_ACT_CHN_RSP)
    STRUCTFIELD(CENL7_ACT_CHN_REQ,bChannelId)
    STRUCTFIELD(CENL7_ACT_CHN_REQ,bChannelLen)
    STRUCTFIELD(CENL7_ACT_CHN_REQ,tbChannelData)
    ENDSTRUCT  (CENL7_ACT_CHN_REQ)
    
    BEGINSTRUCT(CENL7_ACT_ECH_REQ)
    OTHERSTRUCT(CENL7_ACT_ECH_RSP)
    STRUCTFIELD(CENL7_ACT_ECH_REQ,bEchoLen)
    STRUCTFIELD(CENL7_ACT_ECH_REQ,tbEcho)
    ENDSTRUCT  (CENL7_ACT_ECH_REQ)
    
    BEGINSTRUCT(CENL7_ACT_GNO_RSP)
    STRUCTFIELD(CENL7_ACT_GNO_RSP,bRndLen)
    STRUCTFIELD(CENL7_ACT_GNO_RSP,tbRnd)
    ENDSTRUCT  (CENL7_ACT_GNO_RSP)

    BEGINSTRUCT(CENL7_ACT_GST_REQ)
    STRUCTFIELD(CENL7_ACT_GST_REQ,bAttrIdCount)
    STRUCTFIELD(CENL7_ACT_GST_REQ,tbAttrIds)
    STRUCTFIELD(CENL7_ACT_GST_REQ,bRndLen)
    STRUCTFIELD(CENL7_ACT_GST_REQ,tbRnd)
    STRUCTFIELD(CENL7_ACT_GST_REQ,bKeyId)
    ENDSTRUCT  (CENL7_ACT_GST_REQ)

    BEGINSTRUCT(CENL7_ACT_GST_RSP)
    STRUCTFIELD(CENL7_ACT_GST_RSP,bAttrCount)
    STRUCTFIELD(CENL7_ACT_GST_RSP,tsAttrs)
    STRUCTFIELD(CENL7_ACT_GST_RSP,bMacLen)
    STRUCTFIELD(CENL7_ACT_GST_RSP,tbMac)
    ENDSTRUCT  (CENL7_ACT_GST_RSP)

    BEGINSTRUCT(CENL7_ACT_SUB_REQ)
    OTHERSTRUCT(CENL7_ACT_ADD_REQ)
    OTHERSTRUCT(CENL7_ACT_SSC_REQ)
    OTHERSTRUCT(CENL7_ACT_GSC_RSP)
    STRUCTFIELD(CENL7_ACT_SUB_REQ,bAttrCount)
    STRUCTFIELD(CENL7_ACT_SUB_REQ,tsAttrs)
    ENDSTRUCT  (CENL7_ACT_SUB_REQ)

    BEGINSTRUCT(CENL7_ACT_GSC_REQ)
    STRUCTFIELD(CENL7_ACT_GSC_REQ,bAttrIdCount)
    STRUCTFIELD(CENL7_ACT_GSC_REQ,tbAttrIds)
    ENDSTRUCT  (CENL7_ACT_GSC_REQ)

    BEGINSTRUCT(CENL7_ACT_MMI_RSP)
    OTHERSTRUCT(CENL7_ACT_GNO_REQ)
    OTHERSTRUCT(CENL7_ACT_SUB_RSP)
    OTHERSTRUCT(CENL7_ACT_ADD_RSP)
    OTHERSTRUCT(CENL7_ACT_SSC_RSP)
    STRUCTFIELD(CENL7_ACT_MMI_RSP,bFoo)
    ENDSTRUCT  (CENL7_ACT_MMI_RSP)

    BEGINSTRUCT(CENL7_ACT_PARAMS)
    UNIONITEM  (CENL7_ACT_PARAMS,sMmiReq)
    UNIONITEM  (CENL7_ACT_PARAMS,sMmiRsp)
    UNIONITEM  (CENL7_ACT_PARAMS,sGNoReq)
    UNIONITEM  (CENL7_ACT_PARAMS,sGNoRsp)
    UNIONITEM  (CENL7_ACT_PARAMS,sGStReq)
    UNIONITEM  (CENL7_ACT_PARAMS,sGStRsp)
    UNIONITEM  (CENL7_ACT_PARAMS,sEchReq)
    UNIONITEM  (CENL7_ACT_PARAMS,sEchRsp)
    UNIONITEM  (CENL7_ACT_PARAMS,sChnReq)
    UNIONITEM  (CENL7_ACT_PARAMS,sChnRsp)
    UNIONITEM  (CENL7_ACT_PARAMS,sSubReq)
    UNIONITEM  (CENL7_ACT_PARAMS,sSubRsp)
    UNIONITEM  (CENL7_ACT_PARAMS,sAddReq)
    UNIONITEM  (CENL7_ACT_PARAMS,sAddRsp)
    UNIONITEM  (CENL7_ACT_PARAMS,sGScReq)
    UNIONITEM  (CENL7_ACT_PARAMS,sGScRsp)
    UNIONITEM  (CENL7_ACT_PARAMS,sSScReq)
    UNIONITEM  (CENL7_ACT_PARAMS,sSScRsp)
    STRUCTTOEND(CENL7_ACT_PARAMS,sMmiReq)
    ENDSTRUCT  (CENL7_ACT_PARAMS)

    SIZEOF  ( sAPI );
    OFFSETOF( sAPI.bPDU );
    OFFSETOF( sAPI.bFrag );
    OFFSETOF( sAPI.bBlockType );
    SIZEOF  ( sAPI.sIniReq );
    OFFSETOF( sAPI.sIniReq );
    OFFSETOF( sAPI.sIniReq.fOptAppPresent );
    OFFSETOF( sAPI.sIniReq.wManufacturer );
    OFFSETOF( sAPI.sIniReq.dwBeaconId );
    OFFSETOF( sAPI.sIniReq.dwTime );
    OFFSETOF( sAPI.sIniReq.bProf );
    OFFSETOF( sAPI.sIniReq.bAppCount );
    SIZEOF  ( sAPI.sIniReq.tsApps[0] );
    OFFSETOF( sAPI.sIniReq.tsApps[0] );
    OFFSETOF( sAPI.sIniReq.tsApps[0].fEIDPresent  );
    OFFSETOF( sAPI.sIniReq.tsApps[0].fParmPresent );
    OFFSETOF( sAPI.sIniReq.tsApps[0].bAppId );
    OFFSETOF( sAPI.sIniReq.tsApps[0].bEID );
    OFFSETOF( sAPI.sIniReq.tsApps[0].bParmsLen );
    OFFSETOF( sAPI.sIniReq.tsApps[0].tbParmsData[0] );
    OFFSETOF( sAPI.sIniReq.tsApps[1] );
    ENDOF   ( sAPI.sIniReq.tsApps[CENL7_MAX_APPS-1] );
    OFFSETOF( sAPI.sIniReq.bOptAppCount );
    OFFSETOF( sAPI.sIniReq.tsOptApps[0] );
    OFFSETOF( sAPI.sIniReq.bProfCount );
    OFFSETOF( sAPI.sIniReq.tbProfs[0] );
    
    return 0;

*/


    AnalyseHeap();

    while ( TRUE )
    {
        __try
        {
            printf( "\nCSR_TAG3.DLL demonstration program" );
            printf( "\nInitializing the software ..." );

            //
            // In order to be as reactive as possible, we increase
            // the priority class of the process so the layer 2
            // thread will be able to reach the highest priority
            // of the operating system.
            //
            SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS );

            //
            // We do not use other multimedia functions, but this one
            // has an interesting side effect : if changes the kernel
            // preemption time from 10 ms to 1 ms. Allowing our
            // high priority process to be activate faster when another
            // lower priority process is using a lot of CPU.
            //
            timeBeginPeriod( 1 );

            //
            // This event is used for the asynchronous I/O of
            // the TAG library.
            //
            sCtx.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
            if ( sCtx.hEvent == NULL )
            {
                dwErr = GetLastError();
                fprintf( stderr, 
                         "\nError %u in CreateEvent : %s", 
                         dwErr, Win32ErrorText(dwErr) );
                __leave;
            }

            //
            // Create an instance of TAG using the registry to
            // define the settings.
            //

			////UDP serial port emulatin for Armonhly B4E
			//dwErr = TagOpen(TAG_INTERFACE_COMMUNICATOR | TAG_INTERFACE_FLAG_REGISTRY_PARAMS,
			//                          "SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\PPT_PRT\\Prt_bal\\CommParams",
			//                          &FrameHook,
			//                          (void*)&sCtx.sHist,
			//                          &sCtx.hTag );  



			//dwErr = TagOpen( TAG_INTERFACE_IPGEA | TAG_INTERFACE_FLAG_REGISTRY_PARAMS,
   //                          "SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\PPT_PRT\\Prt_bal\\CommParams",
   //                          &FrameHook,
   //                          (void*)&sCtx.sHist,
   //                          &sCtx.hTag );  

      /*    dwErr = TagOpen( TAG_INTERFACE_COMMUNICATOR | TAG_INTERFACE_FLAG_REGISTRY_PARAMS,
                             "SOFTWARE\\CSRoute\\DsrcMT",
                             &FrameHook,
                             (void*)&sCtx.sHist,
                             &sCtx.hTag ); 

	      
			dwErr = TagOpen( TAG_INTERFACE_WIRED | TAG_INTERFACE_FLAG_REGISTRY_PARAMS,
                             "SOFTWARE\\CSRoute\\DsrcELI",
                             &FrameHook,
                             (void*)&sCtx.sHist,
                             &sCtx.hTag ); 

		    dwErr = TagOpen( TAG_INTERFACE_QFREE | TAG_INTERFACE_FLAG_REGISTRY_PARAMS,
			    "SOFTWARE\\CSRoute\\DsrcQFree",
			    &FrameHook,
			    (void*)&sCtx.sHist,
			    &sCtx.hTag ); 

    */


			dwErr = TagOpen(TAG_INTERFACE_QFREE_DMI | TAG_INTERFACE_FLAG_REGISTRY_PARAMS,
				"SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\PPT_PRT\\Prt_bal\\CommParams",
				&FrameHook,
				(void*)&sCtx.sHist,
				&sCtx.hTag);

			//dwErr = TagOpen(TAG_INTERFACE_COMMUNICATOR | TAG_INTERFACE_FLAG_REGISTRY_PARAMS,
			//	"SOFTWARE\\CSRoute\\LaneController\\Config\\Modules\\PPT_PRT\\Prt_bal\\CommParams",
			//	&FrameHook,
			//	(void*)&sCtx.sHist,
			//	&sCtx.hTag);

			

		    
		    if ( dwErr != NO_ERROR )
            {
                fprintf( stderr, 
                         "\nError %u in TagOpen : %s", 
                         dwErr, Win32ErrorText(dwErr) );
                __leave;
            }


            //
            // Prepare the container codes and attributes.
            //
            //    - EFCContextMark       (id =  0, container = 32, 6 bytes, implicit size),
            //    - ContractSerialNumber (id =  1, container = 33, 4 bytes, implicit size),
            //    - ReceiptServicePart   (id =  5, container = 37, 13 bytes, implicit size),
            //    - VehicleLPN   		 (id = 16, container = 47, special),
            //    - VehicleClass		 (id = 17, container = 49, 1 byte , implicit size),
            //    - EquipmentStatus      (id = 26, container = 58, 2 bytes, implicit size).

            if ( ( ! CENL7DefineContainer( TagGetL7(sCtx.hTag), 32, FALSE, 6) ) ||
                 ( ! CENL7DefineContainer( TagGetL7(sCtx.hTag), 33, FALSE, 4) ) ||
                 ( ! CENL7DefineContainer( TagGetL7(sCtx.hTag), 37, FALSE, 13) ) ||
                 ( ! CENL7DefineContainer( TagGetL7(sCtx.hTag), 47, FALSE, 0) ) ||
                 ( ! CENL7DefineContainer( TagGetL7(sCtx.hTag), 49, FALSE, 1) ) ||
                 ( ! CENL7DefineContainer( TagGetL7(sCtx.hTag), 58, FALSE, 2) ) )
            {
                fprintf( stderr, 
                         "\nError in DefineContainer");
                __leave;
            }                                   
    

            if ( ( ! CENL7DefineAttribute( TagGetL7(sCtx.hTag),  0, 32) ) ||
                 ( ! CENL7DefineAttribute( TagGetL7(sCtx.hTag),  1, 33) ) ||
                 ( ! CENL7DefineAttribute( TagGetL7(sCtx.hTag),  5, 37) ) ||
                 ( ! CENL7DefineAttribute( TagGetL7(sCtx.hTag), 16, 47) ) ||
                 ( ! CENL7DefineAttribute( TagGetL7(sCtx.hTag), 17, 49) ) ||
                 ( ! CENL7DefineAttribute( TagGetL7(sCtx.hTag), 26, 58) ) )
            {
                fprintf( stderr, 
                         "\nError in DefineAttribute");
                __leave;
            }                                   

    // <++ NBL-2003-07-11

            //
            // Define the value of the master keys
            //
            if ( ! CENL7SetMasterSecurity( TagGetL7(sCtx.hTag),
                                           CENL7_SECURITY_DES,
                                           gtbSecurity,
                                           sizeof(gtbSecurity) ) )
            {
                fprintf( stderr, 
                         "\nError in CENL7SetMasterSecurity");
                __leave;
            }

    // NBL-2003-07-11 ++> 


            //
            // Prepare the running context of the application.
            //
            sCtx.dwRspTimeout = 200;
			sCtx.dwBSTRspTimeout = 10000;
            sCtx.dwId = 0;
            sCtx.dwLastErr = NO_ERROR;
            sCtx.eLastStatus = TAG_STATUS_NONE;
			printf("OK! - hit any key to continue -");
			_getch();

            //
            // The "ByPass" flag indicates if the initialization
            // of the interface must be by-passed or not.
            //
            fByPass = FALSE;
			printf("\nInitializing the equipment...");

            while ( ! _kbhit() )
            {
			    int iOK = 0;
			    
			    fprintf( stderr, "." );
			    
    //          iOK = MessageBox( NULL,"Read next?", "Q", MB_OKCANCEL );
    //			if(iOK == IDCANCEL)
    //			{
    //				break;
    //			}
			    
			    //or
			    Sleep(10);
			    
				
			    
                if ( ! fByPass )
			    {
					printf(".");
                    //
                    // We have to initialize the equipement interface.
                    // These actions have no effect on a direct connection
                    // using the ELI equipment, but they are executed
                    // anyway to keep code genericity.
                    //

                    //
                    // First, we have to make sure the current operation
                    // mode of the equipment is correct. So we call the
                    // function that forces it. We do not handle any error
                    // for this request (to many possible cases). The next
                    // control command are easier to handle.
				    if(!ControlInterface( &sCtx, TAG_REQUEST_MODE ))
				    {
					    continue;
				    }
				    
				    //
                    // Now we are sure the mode is correct, we must ensure
                    // the beacon is not making BST polling.
                    //
				    if ( ! ControlInterface( &sCtx, TAG_REQUEST_ABORT) )
                    {
                        if ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TRANSACTING ) != 0 )
                        {
                            //
                            // The command failed because a transaction was
                            // already started. So we need to terminate it
                            //
                            if ( ! AbortTransaction( &sCtx ) )
                            {
                                if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                                {
                                    //
                                    // The equipment refused to end the transaction,
                                    // there is a problem.
                                    //
                                    fprintf( stderr, 
                                             "\nInvalid status in AbortTransaction(AbortPolling) : 0x%08X", sCtx.eLastStatus );
                                }
                                else if ( sCtx.dwLastErr != NO_ERROR )
                                {
                                    //
                                    // The library reports an error.
                                    //
                                    fprintf( stderr, 
                                             "\nError %u in AbortTransaction(AbortPolling) : %s", 
                                             sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                                }
                                else
                                {
                                    //
                                    // The software should never reach that point.
                                    //
                                    fprintf( stderr, 
                                             "\nUnexpected error in AbortTransaction(AbortPolling)" );
                                }
                            }
                            else
                            {
                                //
                                // The current status is correct to begin a transaction, we
                                // can now bypass the equipment initialization phase.
                                //
								printf("OK!\n");
                                fByPass = TRUE;
                            }
                        }
                        else if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                        {
                            //
                            // The equipment refused to stop the BST polling.
                            //
                            fprintf( stderr, 
                                     "\nInvalid status in AbortPolling : 0x%08X", sCtx.eLastStatus );
                        }
                        else if ( sCtx.dwLastErr != NO_ERROR )
                        {
                            //
                            // The library reports an error.
                            //
                            fprintf( stderr, 
                                     "\nError %u in AbortPolling : %s", 
                                     sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                        }
                        else
                        {
                            //
                            // The software should never reach that point.
                            //
                            fprintf( stderr, 
                                     "\nUnexpected error in AbortPolling" );
                        }

                        Sleep( 1000 );
                        continue;
                    }

                    if ( ! ControlInterface( &sCtx, TAG_REQUEST_STATUS ) )
				    {
					    continue;
				    }
                }

                //
                // By default, we'll have to initialize the equipment
                // at the next loop. If the procession is correct, the flag
                // will be changed so the equipment initialization is
                // by-passed.
                //
			    
			    fByPass = FALSE;
			    sCtx.fGetOk = FALSE;
                sCtx.fSetOk = FALSE;
				printf("\nInitTransaction...");

                //
                // Do BST/VST exchange
                //
                if ( ! InitTransaction( &sCtx ) )
                {
                    //
                    // BST/VST exchange failed
                    //
                    if ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TRANSACTING ) != 0 )
                    {
                        //
                        // A transaction was already started, so we have to abort it.
                        //
                        if ( ! AbortTransaction( &sCtx ) )
                        {
                            //
                            // Abortion has failed, so get a diagnositc
                            //
                            if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                            {
                                fprintf( stderr, 
                                         "\nInvalid status in AbortTransaction(InitTransaction) : 0x%08X", sCtx.eLastStatus );
                            }
                            else if ( sCtx.dwLastErr != NO_ERROR )
                            {
                                fprintf( stderr, 
                                         "\nError %u in AbortTransaction(InitTransaction) : %s", 
                                         sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                            }
                            else
                            {
                                fprintf( stderr, 
                                         "\nUnexpected error in AbortTransaction(InitTransaction)" );
                            }
                        }
                        else
                            fByPass = TRUE;
                    }

                    else if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                    {
                        fprintf( stderr, 
                                 "\nInvalid status in InitTransaction : 0x%08X", sCtx.eLastStatus );
                    }
                    else if ( sCtx.dwLastErr == WAIT_TIMEOUT )
                    {
                        //
                        // Response time means no tag is present, this is not really an error
                        //
                        fByPass = TRUE;
                    }
                    else if ( sCtx.dwLastErr != NO_ERROR )
                    {
                        fprintf( stderr, 
                                 "\nError %u in InitTransaction : %s", 
                                 sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                    }
                    else
                    {
                        fprintf( stderr, 
                                 "\nUnexpected error in InitTransaction" );
                    }

				    TagSessionRequest( sCtx.hTag, 
					    TAG_SESSION_END,
					    NULL, 
					    0,
					    NULL,
					    NULL );
				    
                    continue;
                }

    // <++ NBL-2003-07-11
                if ( sCtx.fAppFound )
                {
                    //
                    // Make security calculation about the access credentials
                    //
                    ComputeAccessCredentials( &sCtx );

    // NBL-2003-07-11 ++>

                    //
                    // Read Attributes from the OBE
                    //

    /* <-- NBL-2003-07-11      if ( ! GetAttributes( &sCtx ) )
       NBL-2003-07-11 --> */
    // <++ NBL-2003-07-11
                    if ( ! GetStampedAndGet( &sCtx ) )
    // NBL-2003-07-11 ++>
                    {
                        //
                        // If the command failed
                        //
                        if ( ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TRANSACTING ) != 0 ) ||
                             ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TIMEOUT     ) != 0 ) )
                        {
                            //
                            // The request failed, we have to command the equipment
                            // to abort the current transaction.
                            //
                            if ( ! AbortTransaction( &sCtx ) )
                            {
                                if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                                {
                                    fprintf( stderr, 
                                             "\nInvalid status in AbortTransaction(GET) : 0x%08X", sCtx.eLastStatus );
                                }
                                else if ( sCtx.dwLastErr != NO_ERROR )
                                {
                                    fprintf( stderr, 
                                             "\nError %u in AbortTransaction(GET) : %s", 
                                             sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                                }
                                else
                                {
                                    fprintf( stderr, 
                                             "\nUnexpected error in AbortTransaction(GET)" );
                                }
                            }
                            else
                                fByPass = TRUE;
                        }

                        else if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                        {
                            fprintf( stderr, 
                                     "\nInvalid status in GET : 0x%08X", sCtx.eLastStatus );
                        }
                        else if ( sCtx.dwLastErr != NO_ERROR )
                        {
                            fprintf( stderr, 
                                     "\nError %u in GET : %s", 
                                     sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                        }
                        else
                        {
                            fprintf( stderr, 
                                     "\nUnexpected error in GET" );
                        }
                        continue;
                    }


					    
				    //TIC repeat set to test the speed
				    for(i=0;i<10;i++)
				    {

					    //
					    // Set Attributes to the OBE
					    //

					    if ( ! SetAttributes( &sCtx ) )
					    {
						    //
						    // If the command failed
						    //
						    if ( ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TRANSACTING ) != 0 ) ||
							     ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TIMEOUT     ) != 0 ) )
						    {
							    //
							    // The request failed, we have to command the equipment
							    // to abort the current transaction.
							    //
							    if ( ! AbortTransaction( &sCtx ) )
							    {
								    if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
								    {
									    fprintf( stderr, 
											     "\nInvalid status in AbortTransaction(SET) : 0x%08X", sCtx.eLastStatus );
								    }
								    else if ( sCtx.dwLastErr != NO_ERROR )
								    {
									    fprintf( stderr, 
											     "\nError %u in AbortTransaction(SET) : %s", 
											     sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
								    }
								    else
								    {
									    fprintf( stderr, 
											     "\nUnexpected error in AbortTransaction(SET)" );
								    }
							    }
							    else
								    fByPass = TRUE;
						    }

						    else if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
						    {
							    fprintf( stderr, 
									     "\nInvalid status in SET : 0x%08X", sCtx.eLastStatus );
						    }
						    else if ( sCtx.dwLastErr != NO_ERROR )
						    {
							    fprintf( stderr, 
									     "\nError %u in SET : %s", 
									     sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
						    }
						    else
						    {
							    fprintf( stderr, 
									     "\nUnexpected error in SET" );
						    }
						    continue;
					    }

				    
				    }


                    //
                    // Command the OBE to beep
                    //
                    if ( ! SetMMI( &sCtx ) )
                    {
                        //
                        // If the command failed
                        //
                        if ( ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TRANSACTING ) != 0 ) ||
                             ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TIMEOUT     ) != 0 ) )
                        {
                            //
                            // The request failed, we have to command the equipment
                            // to abort the current transaction.
                            //
                            if ( ! AbortTransaction( &sCtx ) )
                            {
                                if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                                {
                                    fprintf( stderr, 
                                             "\nInvalid status in AbortTransaction(SetMMI) : 0x%08X", sCtx.eLastStatus );
                                }
                                else if ( sCtx.dwLastErr != NO_ERROR )
                                {
                                    fprintf( stderr, 
                                             "\nError %u in AbortTransaction(SetMMI) : %s", 
                                             sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                                }
                                else
                                {
                                    fprintf( stderr, 
                                             "\nUnexpected error in AbortTransaction(SetMMI)" );
                                }
                            }
                            else
                                fByPass = TRUE;
                        }

                        else if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                        {
                            fprintf( stderr, 
                                     "\nInvalid status in SetMMI : 0x%08X", sCtx.eLastStatus );
                        }
                        else if ( sCtx.dwLastErr != NO_ERROR )
                        {
                            fprintf( stderr, 
                                     "\nError %u in SetMMI : %s", 
                                     sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                        }
                        else
                        {
                            fprintf( stderr, 
                                     "\nUnexpected error in SetMMI" );
                        }
                        continue;
                    }

    // <++ NBL-2003-07-11
                }
    // NBL-2003-07-11 ++>


                //
                // Terminate the transaction with an event-report
                //
                        if ( ! TerminateTransaction( &sCtx ) )
                        {
                            //
                            // If the command failed
                            //
                            if ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TRANSACTING ) != 0 )
                            {
                                //
                                // The request failed, we have to command the equipment
                                // to abort the current transaction.
                                //
                                if ( ! AbortTransaction( &sCtx ) )
                                {
                                    if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                                    {
                                        fprintf( stderr, 
                                                 "\nInvalid status in AbortTransaction(TerminateTransaction) : 0x%08X", sCtx.eLastStatus );
                                    }
                                    else if ( sCtx.dwLastErr != NO_ERROR )
                                    {
                                        fprintf( stderr, 
                                                 "\nError %u in AbortTransaction(TerminateTransaction) : %s", 
                                                 sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                                    }
                                    else
                                    {
                                        fprintf( stderr, 
                                                 "\nUnexpected error in AbortTransaction(TerminateTransaction)" );
                                    }
                                }
                                else
                                    fByPass = TRUE;
                            }
        
                            else if ( ( sCtx.eLastStatus & TAG_STATUS_ERR_TIMEOUT     ) != 0 )
                            {
                                //
                                // For the beacon equipment, we can receive a return status "TIMEOUT"
                                //
                                fByPass = TRUE;
                            }
        
                            else if ( ( sCtx.eLastStatus & TAG_STATUS_ERROR ) != 0 )
                            {
                                fprintf( stderr, 
                                         "\nInvalid status in TerminateTransaction : 0x%08X", sCtx.eLastStatus );
                            }
                            else if ( sCtx.dwLastErr == WAIT_TIMEOUT )
                            {
                                //
                                // For the ELI equipment, we do not expect any answer
                                //
                                fByPass = TRUE;
                            }
                            else if ( sCtx.dwLastErr != NO_ERROR )
                            {
                                fprintf( stderr, 
                                         "\nError %u in TerminateTransaction : %s", 
                                         sCtx.dwLastErr, Win32ErrorText(sCtx.dwLastErr) );
                            }
                            else
                            {
                                fprintf( stderr, 
                                         "\nUnexpected error in TerminateTransaction" );
                            }
        
                            continue;
                        }
        
                        fByPass = TRUE;

                    }
        
        }
        __finally
        {
            //
            // At the end, never forget to close everything
            // that was open.
            //

            if ( sCtx.hTag != NULL )
                TagClose( sCtx.hTag, 1000 );
            if ( sCtx.hEvent != NULL )
                CloseHandle( sCtx.hEvent );
        }

        while ( _kbhit() )
            _getch();

        if ( MessageBox( NULL, "Recommencer ?", "Question", MB_YESNO ) != IDYES )
            break;

    }

    AnalyseHeap();

    MessageBox( NULL, "FINI", "INFO", MB_OK );
	_getch();

    return 0;
}



