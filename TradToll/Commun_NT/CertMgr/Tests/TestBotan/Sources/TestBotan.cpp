#include <iostream>
#include <string>
#include <conio.h>
#include <botan\botan.h>
#include <botan\alg_id.h>
#include <botan\pubkey.h>
#include <botan\rsa.h>
//#include <external\pkcs11t.h>
//#include <external\pkcs11f.h>
//#include <external\pkcs11.h>

//#include <p11.h>

using namespace std;
using namespace Botan;


bool CalculateMd5Hash(unsigned char * xml, unsigned long ulXmlLen, unsigned char * pucHash, unsigned long *ulHashLen)
{
	Botan::Pipe pipe;
	pipe.append(new Botan::Hash_Filter("MD5"));
	pipe.append(new Botan::Hex_Encoder);

	pipe.start_msg();

	// trickiness: the >> op reads until EOF, but seekg won't work
	// unless we're in the "good" state (which EOF is not).
	pipe.write(xml, ulXmlLen);
	pipe.end_msg();
	//std::string out = pipe.read_all_as_string();
	*ulHashLen = pipe.read(pucHash, *ulHashLen);

	return true;
}



bool Signature2(unsigned char * xml, unsigned long ulXmlLen, std::string &pathp8, std::string& password,
				unsigned char * signiture, unsigned long *ulSigLen)
{
	AutoSeeded_RNG rng;

	try
	{
		std::auto_ptr<PKCS8_PrivateKey> key(PKCS8::load_key(pathp8, rng, password));
		RSA_PrivateKey *rsakey = dynamic_cast<RSA_PrivateKey*>(key.get());
		if (!rsakey) {
			std::cout << "The loaded key is not a RSA key!\n";
		}

		PK_Signer signer(*rsakey, "EMSA3(SHA-256)");
	
		for (int i = 0; i < ulXmlLen; ++i) {
			signer.update(xml[i]);
		}


		SecureVector<byte> v = signer.signature(rng);

		int i;
		for (i = 0; i < v.size(); i++)
		{
			signiture[i] = v[i];
		}

		*ulSigLen = i;


		return true;
	}
	catch (Exception e) {

		printf(e.what());
		return false; // "greska potpisa nema privatnog kljuca";

	}
}

void ConvertBINToHEX( const unsigned char *tbBin,  const unsigned long dwLen,  char *szHex)
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

void ConvertHEXToBIN( const char *szHex, unsigned char *tbBin, unsigned long *pdwLen)
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



int main() 
{
	unsigned char szData[] = "12345678|2019-06-12T17:05:43+02:00|9952|bb123bb123|cc123cc123|ss123ss123|99.01";
	unsigned int  uiDataSize = sizeof(szData)-1;

	unsigned char szSigniture[512 + 1] = { 0 };
	unsigned long  uiSignitureSize = sizeof(szSigniture);
	char szSignitureHex[2*512 + 1] = { 0 };

	unsigned char szMd5[512 + 1] = { 0 };
	unsigned long  uiMd5Size = sizeof(szMd5)-1;

	string keyPath = "cert/p8key.pem";
	string password = "123456";

	Botan::LibraryInitializer init;

	if (Signature2(szData, uiDataSize, keyPath, password, szSigniture, &uiSignitureSize))
	{

		ConvertBINToHEX(szSigniture, uiSignitureSize, szSignitureHex);

		CalculateMd5Hash(szSigniture, uiSignitureSize, szMd5, &uiMd5Size);

		printf("Signiture size [bytes]:%d \n", uiSignitureSize);
		printf("Signiture HEX:[%s] \n", szSignitureHex);
		printf("Md5 :[%s] \n", szMd5);

		printf("OK");
	}
	else
	{
		printf("NOK");

	}


	_getch();

// 	Botan::Dynamically_Loaded_Library pkcs11_module("C:\\pkcs11-middleware\\library.dll");
// 	Botan::PKCS11::FunctionListPtr func_list = nullptr;
// 	Botan::PKCS11::LowLevel::C_GetFunctionList(pkcs11_module, &func_list);
// 	Botan::PKCS11::LowLevel p11_low_level(func_list);


}