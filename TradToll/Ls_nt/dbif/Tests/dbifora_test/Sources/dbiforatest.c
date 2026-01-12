#include <windows.h>
#include <stdio.h>
#include <cmdl.h>

#include <dbif.h>

#define TEST_NB_MAX_VARS  32
#define TEST_MAX_PARAM_SIZE 1000

typedef struct TEST_VAR
{
    DB_VAR * psVar;
    DWORD    dwType;
    DWORD    dwCount;
    DWORD    dwSize;
    BOOL     bIsPlaceHolder;
    char     szValues[TEST_MAX_PARAM_SIZE];
    char     szName[DB_VAR_NAME_SIZE+1];
}
TEST_VAR;

typedef struct _TEST_PARAMS
{
	int iThreadId;
    BOOL bHasColumns;
    DWORD dwNbVars;
    TEST_VAR tsVars[TEST_NB_MAX_VARS];
    char szInst[TEST_MAX_PARAM_SIZE];
    char szUsr[TEST_MAX_PARAM_SIZE];
    char szPwd[TEST_MAX_PARAM_SIZE];
    char szReq[TEST_MAX_PARAM_SIZE];
}
TEST_PARAMS;

int RunMultipleThreadTests(TEST_PARAMS *psTest);

DWORD TestError( DB_CNX * psCnx )
{
    DWORD dwLibType;
    DWORD dwLibErr;
	char szLibErr[256] = {0};

    DBGetLastError( psCnx, &dwLibType, &dwLibErr, szLibErr, sizeof(szLibErr) -1 );
    printf( "ERROR %s/%u:%s\n", DB_ERRCLASS_TEXT(dwLibType), dwLibErr, szLibErr );
    DBDisconnect( psCnx );

    return dwLibErr;
}

void TestAssign( TEST_VAR * psVar )
{
    DWORD dwIndex;
    char * pcSrc = psVar->szValues;
    char * pcDst;
    DWORD dwVal;
    char szVal[TEST_MAX_PARAM_SIZE];

    for ( dwIndex = 0 ; dwIndex < DBGetCurrentItemCount( psVar->psVar ) ; dwIndex ++ )
    {
        pcDst = szVal;
        while ( ( *pcSrc != '\0' ) && ( *pcSrc != '|' ) )
        {
            *pcDst = *pcSrc;
            pcDst ++;
            pcSrc ++;
        }
        *pcDst = '\0';
        if ( psVar->dwType == DB_TYPE_INT )
        {
            dwVal = atol(szVal);
            DBSetVariableItemValue( psVar->psVar, dwIndex, &dwVal, sizeof(dwVal) );
        }
        else
        {
            DBSetVariableItemValue( psVar->psVar, dwIndex, szVal, (DWORD)strlen(szVal)+1 );
        }
        if ( *pcSrc != '\0' )
            pcSrc ++;
    }
}   



void TestPrintVar( TEST_VAR * psVar )
{
    DWORD dwIndex;
    DWORD dwVal;
    DWORD dwLen;
    DWORD dwCount;
    char szVal[TEST_MAX_PARAM_SIZE];

    dwCount = ( psVar->dwCount == 0 ? 1 : DBGetCurrentItemCount( psVar->psVar ) );
    for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
    {
        //printf( "%s[%u]=", psVar->szName, dwIndex );
        if ( psVar->dwType == DB_TYPE_INT )
        {
            dwLen = sizeof( dwVal );
            DBGetVariableItemValue( psVar->psVar, dwIndex, &dwVal, &dwLen );
            //printf( "%d\n", dwVal );
        }
        else
        {
            dwLen = sizeof( szVal );
            DBGetVariableItemValue( psVar->psVar, dwIndex, szVal, &dwLen );
            //printf( "\"%s\"\n", szVal );
        }
    }
}



DWORD TestRun( TEST_PARAMS * psTest ) 
{
    DB_CNX * psCnx;
    DB_STMT * psStmt;
    DWORD dwIndex;
    BOOL bEnd;
    
    printf( "." );
    psCnx = DBConnect( psTest->szInst, psTest->szUsr, psTest->szPwd );
    if ( psCnx == NULL )
    {
        printf( "ERROR DBConnect - ERROR_INVALID_DATA\n" );
        return ERROR_INVALID_DATA;
    }
    else
        //printf( "OK\n" );

    //printf( "DBOpenStatement ..." );
    psStmt = DBOpenStatement( psCnx, psTest->szReq );
    if ( psStmt == NULL )
        return TestError( psCnx );
    //else
        //printf( "OK\n" );

    for ( dwIndex = 0 ; dwIndex < psTest->dwNbVars ; dwIndex ++ )
    {
        if ( psTest->tsVars[dwIndex].bIsPlaceHolder )
        {
            //printf( "DBBindPlaceHolderVariable ..." );
            psTest->tsVars[dwIndex].psVar = DBBindPlaceHolderVariable(
                    psStmt, 
                    psTest->tsVars[dwIndex].szName,
                    psTest->tsVars[dwIndex].dwType,
                    psTest->tsVars[dwIndex].dwSize,
                    psTest->tsVars[dwIndex].dwCount );
        }
        else
        {
            //printf( "DBBindRowItemVariable ..." );
            psTest->tsVars[dwIndex].psVar = DBBindRowItemVariable(
                    psStmt,
                    atol(psTest->tsVars[dwIndex].szName),
                    psTest->tsVars[dwIndex].dwType,
                    psTest->tsVars[dwIndex].dwSize );
        }
        if ( psTest->tsVars[dwIndex].psVar == NULL )
            return TestError( psCnx );
        else
            //printf( "OK\n" );
        TestAssign( &psTest->tsVars[dwIndex] );
    }

    //printf( "DBExecuteStatement ..." );
    if ( ! DBExecuteStatement( psStmt ) )
        return TestError( psCnx );
    else
        //printf( "OK\n" );

    if ( psTest->bHasColumns )
    {
        bEnd = FALSE;
        do
        {
            //printf( "DBGetNextRow ..." );
            if ( ! DBGetNextRow( psStmt, &bEnd ) )
                return TestError( psCnx );
            else
                //printf( "OK\n" );
            if ( ! bEnd )
            {
                for ( dwIndex = 0 ; dwIndex < psTest->dwNbVars ; dwIndex ++ )
                    if ( ! psTest->tsVars[dwIndex].bIsPlaceHolder )
                        TestPrintVar( &psTest->tsVars[dwIndex] );
            }
        }
        while ( ! bEnd );
    }
    
    for ( dwIndex = 0 ; dwIndex < psTest->dwNbVars ; dwIndex ++ )
        if ( psTest->tsVars[dwIndex].bIsPlaceHolder )
            TestPrintVar( &psTest->tsVars[dwIndex] );

    DBDisconnect( psCnx );

	printf("%d:OK\n", psTest->iThreadId);

    return NO_ERROR;
}

void TestAddVar( TEST_PARAMS * psTest, char * pcParam, DWORD dwType, DWORD dwSize, BOOL bIsPlaceHolder )
{
    char * pcSrc = pcParam;
    TEST_VAR * psVar = &psTest->tsVars[psTest->dwNbVars];
    char * pcDst = psVar->szName;

    while ( ( *pcSrc != '\0' ) && ( *pcSrc != '=' ) && ( *pcSrc != '[' ))
    {
        *pcDst = *pcSrc;
        pcDst ++;
        pcSrc ++;
    }
    *pcDst = '\0';
    if ( *pcSrc == '[' )
    {
        pcSrc ++;
        psVar->dwCount = atol(pcSrc);
        while ( ( *pcSrc != '\0' ) && ( *pcSrc != '=' ) && ( *pcSrc != ']' ))
            pcSrc ++;
        if ( *pcSrc != ']' )
            while ( ( *pcSrc != '\0' ) && ( *pcSrc != '=' ) )
                pcSrc ++;
    }
    else
        psVar->dwCount = 0;
    if ( *pcSrc == '=' )
        pcSrc ++;
    strcpy( psVar->szValues, pcSrc );

    psVar->bIsPlaceHolder = bIsPlaceHolder;
    psVar->dwType = dwType;
    psVar->dwSize = dwSize;
    psTest->dwNbVars ++;
}


int main( int iArgc, char * * ppcArgv )
{
    DWORD dwIndex;
    TEST_PARAMS sTest = {0};
    char szParam[TEST_MAX_PARAM_SIZE];

    for ( dwIndex = 1 ; dwIndex < (DWORD)iArgc ; dwIndex ++ )
    {
        if ( CMDLParamAsString( "/INST=", ppcArgv[dwIndex], sizeof(sTest.szInst), sTest.szInst ) )
            continue;
        if ( CMDLParamAsString( "/USR=", ppcArgv[dwIndex], sizeof(sTest.szUsr), sTest.szUsr ) )
            continue;
        if ( CMDLParamAsString( "/PWD=", ppcArgv[dwIndex], sizeof(sTest.szPwd), sTest.szPwd ) )
            continue;
        if ( CMDLParamAsString( "/REQ=", ppcArgv[dwIndex], sizeof(sTest.szReq), sTest.szReq ) )
            continue;
        if ( CMDLParamAsString( "/INTCOL=", ppcArgv[dwIndex], sizeof(szParam), szParam ) )
        {
            TestAddVar( &sTest, szParam, DB_TYPE_INT, sizeof(DWORD), FALSE );
            sTest.bHasColumns = TRUE;
            continue;
        }
        if ( CMDLParamAsString( "/STRCOL=", ppcArgv[dwIndex], sizeof(szParam), szParam ) )
        {
            TestAddVar( &sTest, szParam, DB_TYPE_STR, TEST_MAX_PARAM_SIZE, FALSE );
            sTest.bHasColumns = TRUE;
            continue;
        }
        if ( CMDLParamAsString( "/INTPLC=", ppcArgv[dwIndex], sizeof(szParam), szParam ) )
        {
            TestAddVar( &sTest, szParam, DB_TYPE_INT, sizeof(DWORD), TRUE );
            continue;
        }
        if ( CMDLParamAsString( "/STRPLC=", ppcArgv[dwIndex], sizeof(szParam), szParam ) )
        {
            TestAddVar( &sTest, szParam, DB_TYPE_STR, TEST_MAX_PARAM_SIZE, TRUE );
            continue;
        }
    }
    

	
	RunMultipleThreadTests(&sTest);
	Sleep(5000);

	TestRun(&sTest);
	_getch();

	return 0;

}




struct Thread_data
{
	TEST_PARAMS sTest;
};

DWORD WINAPI solo_thread(void* arg)
{
	struct _TEST_PARAMS* psTest = arg;

	/* Use 'data'. */
	TestRun(psTest);

	free(psTest);
	return 0;
}

int RunMultipleThreadTests(TEST_PARAMS *psTest)
{
	for (int i = 0; i < 1000;i++)
	{
		HANDLE thread;
		struct _TEST_PARAMS* data = malloc(sizeof(TEST_PARAMS));

		if (data)
		{
			memcpy(data, psTest, sizeof(TEST_PARAMS));
			data->iThreadId = i;
			thread = CreateThread(NULL, 0, solo_thread, data, 0, NULL);
		}
	}

	return 0;
}