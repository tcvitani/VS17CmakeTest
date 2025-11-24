/******************* (v) 2008 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   												 */
/* FILE:     									 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:  					 */
/*             											 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY: September 2006 - created                                         */
/*****************************************************************************/
#ifndef AES_CRYPT_MAIN_H
#define AES_CRYPT_MAIN_H


#include <windows.h>
#include "csrlc32.h"
/*

*/

#ifdef CSR_AES_DEF
#	include <public.h>
#else
#	include <export.h>
#endif

#define KEYLENGTH(keybits) ((keybits)/8)
#define RKLENGTH(keybits)  ((keybits)/8+28)
#define NROUNDS(keybits)   ((keybits)/32+6)

#ifndef AES_BLOCK_SIZE
	#define AES_BLOCK_SIZE 16
#endif

#define AES_KEY_SIZE 32
#define MD5_LENGTH		33

enum CRYPT_ERROR_MSG
{
	CRYPT_OK = 0,
	CRYPT_ERROR_OPEN_INPUT_FILE,
	CRYPT_ERROR_OPEN_OUTPUT_FILE,
	CRYPT_ERROR_READ_FILE,
	CRYPT_ERROR_WRITE_FILE,
	CRYPT_ERROR_INSUFFICIENT_MEMORY,
}; 

//
// EXPORTED FUNCTIONS PROTOTYPES
//

EXPORT DWORD WINAPI AESEncryptFile(
		IN	char			*szInputFileName, 
		IN	char			*szOutputFileName, 
		IN	unsigned char	*key);

EXPORT DWORD WINAPI AESDecryptFile(
		IN	char			*szInputFileName, 
		IN	char			*szOutputFileName, 
		IN	unsigned char	*key);

EXPORT DWORD WINAPI AESEncryptBuffer(IN OUT LPBYTE pINOUTData, IN OUT DWORD *pdwLength, IN unsigned char *key);
EXPORT DWORD WINAPI AESDecryptBuffer(IN OUT LPBYTE pINOUTData, IN OUT DWORD *pdwLength, IN unsigned char *key);


EXPORT int	WINAPI MD5_File(
		IN	const char		*sFilename, 
		OUT	unsigned char	digest[MD5_LENGTH]);

EXPORT void WINAPI MD5_Buffer(
		IN	const unsigned char *buffer, 
		IN	unsigned int		iBufLength, 
		OUT	unsigned char		digest[MD5_LENGTH]);

EXPORT void WINAPI MD5_DecryptedBuffer(
		IN	unsigned char		*buffer, 
		IN	unsigned int		iBufLength, 
		IN	unsigned int		iEncBlockSize, 
		OUT	unsigned char		digest[MD5_LENGTH]);

EXPORT int	WINAPI MD5_DecryptedFile(
		IN	const char		*sFilename, 
		IN	unsigned int		iEncBlockSize, 
		OUT	unsigned char	digest[MD5_LENGTH]);



#endif 