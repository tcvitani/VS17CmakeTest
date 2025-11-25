#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <certmgr.h>


int __cdecl main( int iArgc, char * * ppcArgv )
{
	unsigned char szData[] = "12345678|2019-06-12T17:05:43+02:00|9952|bb123bb123|cc123cc123|ss123ss123|99.01";
	unsigned int  uiDataSize = sizeof(szData) - 1;

	unsigned char ucSigniture[512 + 1] = { 0 };
	unsigned long  uiSignitureSize = sizeof(ucSigniture);
	char szSignitureHex[2 * 512 + 1] = { 0 };

	unsigned char szMd5[512 + 1] = { 0 };
	unsigned long  uiMd5Size = sizeof(szMd5) - 1;

	unsigned char keyPath[] = "cert/p8key.pem";
	unsigned char password[] = "123456";

	CertMgr_InitializeBotan();

	//OK
	if (CertMgr_GetSigniture_RSA_EMSA3_SHA_256(szData, uiDataSize, "cert/p8key.pem", "123456", ucSigniture, &uiSignitureSize) == eERR_CERTMGR_NO_ERROR)
	{
		CertMgr_ConvertBINToHEX(ucSigniture, uiSignitureSize, szSignitureHex);

		CertMgr_CalculateMd5Hash(ucSigniture, uiSignitureSize, szMd5, &uiMd5Size);

		printf("Test1 \n");
		printf("Signiture size [bytes]:%d \n", uiSignitureSize);
		printf("Signiture HEX:[%s] \n", szSignitureHex);
		printf("Md5 :[%s] \n", szMd5);

		printf("OK \n");
	}
	else
	{
		printf("Error:%s \n", CertMgr_GetLastError());

	}


	//NOK
	if (CertMgr_GetSigniture_RSA_EMSA3_SHA_256(szData, uiDataSize, "cert/p8key.pem", "123456", ucSigniture, &uiSignitureSize) == eERR_CERTMGR_NO_ERROR)
	{
		CertMgr_ConvertBINToHEX(ucSigniture, uiSignitureSize, szSignitureHex);

		CertMgr_CalculateMd5Hash(ucSigniture, uiSignitureSize, szMd5, &uiMd5Size);

		printf("Test2 \n");
		printf("Signiture size [bytes]:%d \n", uiSignitureSize);
		printf("Signiture HEX:[%s] \n", szSignitureHex);
		printf("Md5 :[%s] \n", szMd5);

		printf("OK \n");
	}
	else
	{
		printf("Error:%s \n", CertMgr_GetLastError());

	}

	//NOK
	if (CertMgr_GetSigniture_RSA_EMSA3_SHA_256(szData, uiDataSize, "cert/p8key.pem", "123456", ucSigniture, &uiSignitureSize) == eERR_CERTMGR_NO_ERROR)
	{
		CertMgr_ConvertBINToHEX(ucSigniture, uiSignitureSize, szSignitureHex);

		CertMgr_CalculateMd5Hash(ucSigniture, uiSignitureSize, szMd5, &uiMd5Size);

		printf("Test3 \n");
		printf("Signiture size [bytes]:%d \n", uiSignitureSize);
		printf("Signiture HEX:[%s] \n", szSignitureHex);
		printf("Md5 :[%s] \n", szMd5);

		printf("OK \n");
	}
	else
	{
		printf("Error:%s \n", CertMgr_GetLastError());

	}
	_getch();
    return 0;
}


