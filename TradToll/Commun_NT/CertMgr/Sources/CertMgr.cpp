/*------ (v) 2020 EMOVIS -------- Droits reserves ------- */
/*
/*--------------- INCLUDES: ---------------*/
#include <certmgr.h>

#include <iostream>
#include <string>
#include <conio.h>
#include <botan\botan.h>
#include <botan\alg_id.h>
#include <botan\pubkey.h>
#include <botan\rsa.h>



using namespace Botan;

std::string g_sLastError = "";
unsigned long g_ulLastError = 0;


void __stdcall CertMgr_InitializeBotan()
{
	Botan::LibraryInitializer init;
}

const char * __stdcall CertMgr_GetLastError()
{
	return g_sLastError.c_str();
}




unsigned long __stdcall CertMgr_CalculateMd5Hash(const unsigned char * pbData, unsigned long ulDataLen,
	unsigned char * pucHash, unsigned long *ulHashLen)
{
	Botan::Pipe pipe;
	g_ulLastError = eERR_CERTMGR_NO_ERROR; //reset last error

	try
	{
		pipe.append(new Botan::Hash_Filter("MD5"));
		pipe.append(new Botan::Hex_Encoder);

		pipe.start_msg();

		// trickiness: the >> op reads until EOF, but seekg won't work
		// unless we're in the "good" state (which EOF is not).
		pipe.write(pbData, ulDataLen);
		pipe.end_msg();
		//std::string out = pipe.read_all_as_string();
		*ulHashLen = pipe.read(pucHash, *ulHashLen);
	}
	catch (Botan::Exception e) {

		g_ulLastError = eERR_CERTMGR_GENERAL_ERROR;
		g_sLastError = e.what();
	}

	return g_ulLastError;
}


unsigned long __stdcall CertMgr_GetSigniture_RSA_EMSA3_SHA_256(const unsigned char * pbData, unsigned long ulDataLen,
	const char *szPathp8, const char *szPassword,
	unsigned char * signiture, unsigned long *ulSigLen)
{
	Botan::AutoSeeded_RNG rng;
	g_ulLastError = eERR_CERTMGR_NO_ERROR; //reset last error

	try
	{
		std::string pathp8 = szPathp8;
		std::string password = szPassword;

		Botan::Private_Key *PKey = PKCS8::load_key(pathp8, rng, password);
		if (!PKey) {
			throw Exception("Error loading key file");
		}

		std::auto_ptr<PKCS8_PrivateKey> key(PKey);
		RSA_PrivateKey *rsakey = dynamic_cast<RSA_PrivateKey*>(key.get());
		if (!rsakey) {
			throw Exception("The loaded key is not a RSA key!\n");
		}

		PK_Signer signer(*rsakey, "EMSA3(SHA-256)");

		for (unsigned int i = 0; i < ulDataLen; ++i) {
			signer.update(pbData[i]);
		}

		SecureVector<byte> v = signer.signature(rng);

		unsigned int i;
		for (i = 0; i < v.size(); i++)
		{
			signiture[i] = v[i];
		}

		*ulSigLen = i;

		
	}
	catch (Botan::Decoding_Error e) 
	{
		g_sLastError = e.what();
		g_ulLastError = eERR_CERTMGR_DECODING_ERROR;

	}
	catch (Exception e) 
	{

		g_sLastError = e.what();
		g_ulLastError = eERR_CERTMGR_GENERAL_ERROR;
	}

	return g_ulLastError;

}

void __stdcall CertMgr_ConvertBINToHEX(const unsigned char *tbBin, const unsigned long dwLen, char *szHex)
{
	unsigned long	dwH, dwA;
	unsigned char	chCh;

	for (dwH = 0, dwA = 0; dwH < dwLen; dwH++, dwA += 2)
	{
		szHex[dwA] = (chCh = (tbBin[dwH] & 0xF0) >> 4) < 10 ? (chCh | 0x30) : (chCh + 55);
		szHex[dwA + 1] = (chCh = tbBin[dwH] & 0x0F) < 10 ? (chCh | 0x30) : (chCh + 55);
	}
	szHex[dwA] = '\0';
}

void __stdcall CertMgr_ConvertHEXToBIN(const char *szHex, unsigned char *tbBin, unsigned long *pdwLen)
{
	size_t	dwH, dwA, dwStrLen;
	unsigned char	chCh;
	char *szUpperCaseHex;
	size_t iHexSize = 0;

	iHexSize = strlen(szHex) + 1;

	szUpperCaseHex = (char*)malloc(iHexSize);
	strcpy_s(szUpperCaseHex, iHexSize, szHex);

	_strupr_s(szUpperCaseHex, iHexSize);

	dwStrLen = strlen(szUpperCaseHex) % 2 ? strlen(szUpperCaseHex) + 1 : strlen(szUpperCaseHex);

	for (dwH = 0, dwA = 0; dwA <= dwStrLen; dwH++, dwA += 2)
	{
		if (szUpperCaseHex[dwA] > 'F' || szUpperCaseHex[dwA + 1] > 'F')
		{
			memset(tbBin, 0, (strlen(szUpperCaseHex) + 1) % 2);
			*pdwLen = -1;
			return;
		}

		tbBin[dwH] = ((((chCh = szUpperCaseHex[dwA]) >= 'A') ? (chCh - 55) : (chCh & 0x0F)) << 4) |
			((((chCh = szUpperCaseHex[dwA + 1]) >= 'A') ? (chCh - 55) : (chCh & 0x0F)));
	}

	*pdwLen = (unsigned long)dwH - 1;

	free(szUpperCaseHex);
}





/*---------------------------- END OF FILE -------------------------*/
