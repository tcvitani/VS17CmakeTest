#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "csr_aes.h"
#include "aes.h"

#include <memclass.h>

/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
EXPORT BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk )
{
	BOOL bOK = TRUE;
	
	switch ( dwWhy )
	{
		
		// Lorsque le process fait le premier LoadLibrary consernant cette DLL
	case DLL_PROCESS_ATTACH :
		break;
		
		// Lorsque le process fait le dernier FreeLibrary consernant cett DLL
	case DLL_PROCESS_DETACH :
		break;
		
	}
	
	return bOK;
}

EXPORT DWORD WINAPI AESEncryptFile(char *szInputFileName, char *szOutputFileName, unsigned char *key)
{
	FILE *fOutput;
	FILE *fInput;
	DWORD lFileLen;
	byte *pData = NULL;
	byte *pDataPointer = NULL;
	DWORD dwDataLen;
	DWORD dwWrittenData;
	int iNumWritten;

	__try
	{
		fInput = fopen(szInputFileName, "rb");
	}
	__finally
	{
		if (fInput == NULL)
		{
			return CRYPT_ERROR_OPEN_INPUT_FILE;
		}

	}

	fseek(fInput,0L,SEEK_END);
	lFileLen = ftell(fInput);
	fseek(fInput,0L,SEEK_SET);

	if( lFileLen <= 0 )
	{
		fclose(fInput);
		return CRYPT_ERROR_OPEN_INPUT_FILE;
	}
	
	
	dwDataLen = 0;

	__try
	{
		pData = (byte*)malloc(lFileLen + AES_BLOCK_SIZE);
		
		pDataPointer = pData;
		
		if(pData != NULL)
		{
			// read the data to a buffer
			while (!feof(fInput))
			{
				byte buffer[AES_BLOCK_SIZE];
				int iNumRead,j;
				
				iNumRead = fread(buffer, 1, AES_BLOCK_SIZE, fInput);
				
				if (iNumRead == 0)
					break;
				
				//Extend to full AES_BLOCK_SIZE with zeroes
				for (j = iNumRead; j < sizeof(buffer); j++)
					buffer[j] = '\0';

				dwDataLen += sizeof(buffer);
				memcpy(pDataPointer, buffer,sizeof(buffer));

				pDataPointer += sizeof(buffer);
			}
		}
	}
	__finally
	{
		fclose(fInput);
			
		if (pData == NULL)
		{
			return CRYPT_ERROR_INSUFFICIENT_MEMORY;
		}

		if(dwDataLen == 0)
		{
			free(pData);
			return CRYPT_ERROR_READ_FILE;
		}
		
	}
	

	AESEncryptBuffer( pData, &dwDataLen, key);

	//try write to file
	__try
	{
		fOutput = fopen(szOutputFileName, "wb");
	}
	__finally
	{
		if (fOutput == NULL)
		{
			free(pData);
			return CRYPT_ERROR_OPEN_OUTPUT_FILE;
		}
	}
	
	__try
	{
		dwWrittenData = 0;
		pDataPointer = pData;

		while ( dwWrittenData < dwDataLen)
		{
			iNumWritten = fwrite(pDataPointer, 1, AES_BLOCK_SIZE, fOutput);
			
			dwWrittenData += iNumWritten;
			pDataPointer += iNumWritten;
		}
	}
	__finally
	{
		free(pData);
		fclose(fOutput);
	}

	return CRYPT_OK;
}

EXPORT DWORD WINAPI AESDecryptFile(char *szInputFileName, char *szOutputFileName, unsigned char *key)
{
	FILE *fOutput;
	FILE *fInput;
	DWORD lFileLen;
	byte *pData = NULL;
	byte *pDataPointer = NULL;
	DWORD dwDataLen;
	DWORD dwWrittenData;
	int iNumWritten;

	__try
	{
		fInput = fopen(szInputFileName, "rb");
	}
	__finally
	{
		if (fInput == NULL)
		{
			return CRYPT_ERROR_OPEN_INPUT_FILE;
		}

	}

	fseek(fInput,0L,SEEK_END);
	lFileLen = ftell(fInput);
	fseek(fInput,0L,SEEK_SET);

	if( lFileLen <= 0 )
	{
		fclose(fInput);
		return CRYPT_ERROR_OPEN_INPUT_FILE;
	}
	
	
	dwDataLen = 0;

	__try
	{
		pData = (byte*)malloc(lFileLen + AES_BLOCK_SIZE);
		
		pDataPointer = pData;
		
		if(pData != NULL)
		{
			// read the data to a buffer
			while (!feof(fInput))
			{
				byte buffer[AES_BLOCK_SIZE];
				int iNumRead,j;
				
				iNumRead = fread(buffer, 1, AES_BLOCK_SIZE, fInput);
				
				if (iNumRead == 0)
					break;
				
				//Extend to full AES_BLOCK_SIZE with zeroes
				for (j = iNumRead; j < sizeof(buffer); j++)
					buffer[j] = '\0';

				dwDataLen += sizeof(buffer);
				memcpy(pDataPointer, buffer,sizeof(buffer));

				pDataPointer += sizeof(buffer);
			}
		}
	}
	__finally
	{
		fclose(fInput);
			
		if (pData == NULL)
		{
			return CRYPT_ERROR_INSUFFICIENT_MEMORY;
		}

		if(dwDataLen == 0)
		{
			free(pData);
			return CRYPT_ERROR_READ_FILE;
		}
		
	}
	

	AESDecryptBuffer( pData, &dwDataLen, key);

	//try write to file
	__try
	{
		fOutput = fopen(szOutputFileName, "wb");
	}
	__finally
	{
		if (fOutput == NULL)
		{
			free(pData);
			return CRYPT_ERROR_OPEN_OUTPUT_FILE;
		}
	}
	
	__try
	{
		dwWrittenData = 0;
		pDataPointer = pData;

		while ( dwWrittenData < dwDataLen)
		{
			iNumWritten = fwrite(pDataPointer, 1, AES_BLOCK_SIZE, fOutput);
			
			dwWrittenData += iNumWritten;
			pDataPointer += iNumWritten;
		}
	}
	__finally
	{
		free(pData);
		fclose(fOutput);
	}

	return CRYPT_OK;
}


EXPORT DWORD WINAPI AESEncryptBuffer(IN OUT LPBYTE pINOUTData, IN OUT DWORD *pdwLength, IN unsigned char *key)
{
	CAesCbc sAesCbc;
	byte iv[AES_BLOCK_SIZE];	
	DWORD dwNumBytesToEncrypt;
	DWORD dwNumEncryptedBytes;

	//init AES
	AesGenTables();
	AesSetKeyEncode(&(sAesCbc.aes),key,AES_KEY_SIZE);
	AesCbcInit(&sAesCbc, iv);

	dwNumBytesToEncrypt = *pdwLength;
	dwNumEncryptedBytes = 0;

	dwNumEncryptedBytes = AesCbcEncode(&sAesCbc, pINOUTData, dwNumBytesToEncrypt);

	*pdwLength = dwNumEncryptedBytes;
	return CRYPT_OK;
}


EXPORT DWORD WINAPI AESDecryptBuffer(IN OUT LPBYTE pINOUTData, IN OUT DWORD *pdwLength, IN unsigned char *key)
{
	CAesCbc sAesCbc;
	byte iv[AES_BLOCK_SIZE];	
	DWORD dwNumBytesToEncrypt;
	DWORD dwNumEncryptedBytes;

	//init AES
	AesGenTables();
	AesSetKeyDecode(&(sAesCbc.aes),key,AES_KEY_SIZE);
	AesCbcInit(&sAesCbc, iv);

	dwNumBytesToEncrypt = *pdwLength;
	dwNumEncryptedBytes = 0;

	dwNumEncryptedBytes = AesCbcDecode(&sAesCbc, pINOUTData, dwNumBytesToEncrypt);

	*pdwLength = dwNumEncryptedBytes;
	return CRYPT_OK;
}


