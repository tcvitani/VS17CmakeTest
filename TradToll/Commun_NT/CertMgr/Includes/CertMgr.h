/*------ (v) 2020 EMOVIS -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : CERTMGR 
 * FICHIER    : CERTMGR.H
 * LANGAGE    : C 
 * -----------------------------------------------------------------
 * KEY WORDS  : CertMgr
 * -----------------------------------------------------------------
 * RESUME     : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORIQUE : 
 *
 * -------------------------------------------------------------------- 
 * $F_HEAD
 */
#ifndef CSR_CERTMGR_H
#define CSR_CERTMGR_H
/*--------------- INCLUDES: ---------------*/
/*--------------- RESERVED: ---------------*/

/*--------------- DEFINE: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
typedef enum _enum_certmgr_error
{
	eERR_CERTMGR_NO_ERROR = 0,
	eERR_CERTMGR_DECODING_ERROR = 1,
	eERR_CERTMGR_GENERAL_ERROR = 0xFFFFFFFF,

	
}
enum_certmgr_error;

/*--------------- FUNCTIONS: --------------*/
#ifdef __cplusplus
extern "C" {
#endif 
	__declspec(dllexport) void __stdcall CertMgr_InitializeBotan();
	__declspec(dllexport) const char * __stdcall CertMgr_GetLastError();

	__declspec(dllexport) unsigned long __stdcall CertMgr_CalculateMd5Hash(const unsigned char * pbData, unsigned long ulDataLen,
		unsigned char * pucHash, unsigned long *ulHashLen);

	__declspec(dllexport) unsigned long __stdcall CertMgr_GetSigniture_RSA_EMSA3_SHA_256(const unsigned char * pbData, unsigned long ulDataLen,
		const char *szPathp8, const char *szPassword,
		unsigned char * signiture, unsigned long *ulSigLen);

	__declspec(dllexport) void __stdcall CertMgr_ConvertBINToHEX(const unsigned char *tbBin, const unsigned long dwLen, char *szHex);

	__declspec(dllexport) void __stdcall CertMgr_ConvertHEXToBIN(const char *szHex, unsigned char *tbBin, unsigned long *pdwLen);


#ifdef __cplusplus
}
#endif 



#endif

