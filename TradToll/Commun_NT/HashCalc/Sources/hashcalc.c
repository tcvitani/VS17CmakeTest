///////////////////////////// INCLUDES /////////////////////////////
#include <hashcalc.h>
#include <resource.h>
#include <stdio.h>
///////////////////////////// RESERVED /////////////////////////////
///////////////////////////// DEFINES //////////////////////////////

#define SHA256_DIGEST_SIZE ( 256 / 8)
#define SHA256_BLOCK_SIZE  ( 512 / 8)

#define SHFR(x, n)    (x >> n)
#define ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define CH(x, y, z)  ((x & y) ^ (~x & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))

#define SHA256_F1(x) (ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SHA256_F2(x) (ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SHA256_F3(x) (ROTR(x,  7) ^ ROTR(x, 18) ^ SHFR(x,  3))
#define SHA256_F4(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10))


#define UNPACK32(x, str)                      \
{                                             \
    *((str) + 3) = (UCHAR) ((x)      );       \
    *((str) + 2) = (UCHAR) ((x) >>  8);       \
    *((str) + 1) = (UCHAR) ((x) >> 16);       \
    *((str) + 0) = (UCHAR) ((x) >> 24);       \
}

#define PACK32(str, x)                        \
{                                             \
    *(x) =   ((UINT) *((str) + 3)      )    \
	| ((UINT) *((str) + 2) <<  8)    \
	| ((UINT) *((str) + 1) << 16)    \
	| ((UINT) *((str) + 0) << 24);   \
}

#define SHA256_SCR(i)                         \
{                                             \
    w[i] =  SHA256_F4(w[i -  2]) + w[i -  7]  \
	+ SHA256_F3(w[i - 15]) + w[i - 16]; \
}
///////////////////////////// TYPEDEFS /////////////////////////////

typedef struct {
    unsigned int tot_len;
    unsigned int len;
    unsigned char block[2 * SHA256_BLOCK_SIZE];
    unsigned int h[8];
} sha256_ctx;

///////////////////////////// FUNCTIONS ////////////////////////////
void sha256_init(sha256_ctx * ctx);
void sha256_update(sha256_ctx *ctx, const unsigned char *message, unsigned int len);
void sha256_final(sha256_ctx *ctx, unsigned char *digest);
void sha256(const unsigned char *message, unsigned int len, unsigned char *digest);
void ConvertBytesToHexString(IN BYTE *bytes, IN DWORD dwLen, OUT UCHAR *szHex);
void ConvertHexStringToBytes(IN UCHAR *szHex, OUT BYTE *pBytes, OUT DWORD *dwLen);

///////////////////////////// VARIABLES/////////////////////////////

extern UINT sha256_k[64];

UINT sha256_h0[8] =
{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

UINT sha256_k[64] =
{0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
             0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

///////////////////////////// CODE /////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk)
{
	BOOL bOK = TRUE;
	
	switch (dwWhy)
	{
	case DLL_PROCESS_ATTACH:
		break;
		
	case DLL_PROCESS_DETACH:
		break;
		
	case DLL_THREAD_ATTACH:  
		break; 
		
	case DLL_THREAD_DETACH:  
		break;
	}
	
	return bOK;
}

EXPORT sha256_err __stdcall CalculateSha256(IN BYTE *pInputData, IN int iInputDataLen, IN int iOutputBufferSize, OUT BYTE *pCalculatedHash)
{
	DWORD	dwLen						= 0;
	BYTE	bTmp[SHA256_LEN_BYTES + 1]	= {0};
	BYTE	*pbytes						= NULL;
	
	if(iOutputBufferSize < (SHA256_LEN_HEX_STRING + 1))
		return ERR_BUFFER_TOO_SMALL;

	pbytes = (BYTE *)calloc((iInputDataLen/2) + 1, sizeof(BYTE));

	if(pbytes == NULL)
		return ERR_MEMORY_ALLOCATION;

	ConvertHexStringToBytes(pInputData, pbytes, &dwLen);
	
	//SHA256(pbytes, dwLen, bTmp);
	sha256(pbytes, dwLen, bTmp);
	
	
	ConvertBytesToHexString(bTmp, SHA256_LEN_BYTES, pCalculatedHash);

	free(pbytes);
	pbytes = NULL;
	
	return ERR_OK;
}

EXPORT sha256_err __stdcall CalculateSha256_w(IN WCHAR *pInputData, IN int iInputDataLen, IN int iOutputBufferSize, OUT WCHAR *pCalculatedHash)
{
	int			size_needed	= 0;
	BYTE		*pInput		= NULL;
	BYTE		*pOutput	= NULL;
	sha256_err	err			= ERR_OK;

	do 
	{
		// convert from wide char to ASCII string
		size_needed = WideCharToMultiByte(CP_UTF8, 0, pInputData, (int)wcslen(pInputData), NULL, 0, NULL, NULL);
		pInput = calloc(size_needed + 1, sizeof(BYTE));

		if(pInput == NULL)
		{
			err = ERR_MEMORY_ALLOCATION;
			break;
		}

 		WideCharToMultiByte(CP_UTF8, 0, pInputData, (int)wcslen(pInputData), pInput, size_needed, NULL, NULL);
 		pInput[size_needed]='\0';

		pOutput = (BYTE *)calloc(iOutputBufferSize + 1, sizeof(BYTE));

		if(pOutput == NULL)
		{
			err = ERR_MEMORY_ALLOCATION;
			break;
		}

		err = CalculateSha256(pInput, (int)strlen(pInput), iOutputBufferSize, pOutput);

		if(err == ERR_OK)
		{
			// convert string back to wide char
			size_needed = MultiByteToWideChar(CP_UTF8, 0, pOutput, -1, NULL, 0);

			if(size_needed < iOutputBufferSize)
			{
				err = ERR_BUFFER_TOO_SMALL;
				break;
			}

			MultiByteToWideChar(CP_ACP, 0, pOutput, -1, pCalculatedHash, size_needed);
		}
	}
	while(FALSE);

	if(pInput != NULL)
	{
		free(pInput);
		pInput = NULL;
	}
	
	if(pOutput != NULL)
	{
		free(pOutput);
		pOutput = NULL;
	}

	return err;
}


EXPORT void __stdcall ConvertBytesToHexString(IN BYTE *bytes, IN DWORD dwLen, OUT UCHAR *szHex)
{
	DWORD	dwH, dwA; 
	UCHAR	chCh;
	
	for (dwH=0, dwA=0; dwH<dwLen; dwH++, dwA+=2)
	{
		szHex[dwA] = (chCh = (bytes[dwH] & 0xF0) >> 4) < 10 ? (chCh | 0x30) : (chCh + 55);
		szHex[dwA+1] = (chCh = bytes[dwH] & 0x0F) < 10 ? (chCh | 0x30) : (chCh + 55);
	}
	szHex[dwA] = '\0';
}

EXPORT void __stdcall ConvertHexStringToBytes(IN UCHAR *szHex, OUT BYTE *pBytes, OUT DWORD *dwLen)
{
	DWORD	dwH, dwA; 
	UCHAR	chCh;
	
	for (dwH=0, dwA=0; dwH<strlen(szHex)/2; dwH++, dwA+=2)
	{
		pBytes[dwH] = ((((chCh = szHex[dwA])  >= 'A') ? (chCh - 55) : (chCh & 0x0F)) << 4) |
			((((chCh = szHex[dwA+1])  >= 'A') ? (chCh - 55) : (chCh & 0x0F)));
	}
	*dwLen = dwH;
}

void sha256_transf(sha256_ctx *ctx, const unsigned char *message,
                   unsigned int block_nb)
{
    UINT w[64];
    UINT wv[8];
    UINT t1, t2;
    const unsigned char *sub_block;
    int i;
	
    int j;
	
    for (i = 0; i < (int) block_nb; i++) {
        sub_block = message + (i << 6);
		
        for (j = 0; j < 16; j++) {
            PACK32(&sub_block[j << 2], &w[j]);
        }
		
        for (j = 16; j < 64; j++) {
            SHA256_SCR(j);
        }
		
        for (j = 0; j < 8; j++) {
            wv[j] = ctx->h[j];
        }
		
        for (j = 0; j < 64; j++) {
            t1 = wv[7] + SHA256_F2(wv[4]) + CH(wv[4], wv[5], wv[6])
                + sha256_k[j] + w[j];
            t2 = SHA256_F1(wv[0]) + MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = t1 + t2;
        }
		
        for (j = 0; j < 8; j++) {
            ctx->h[j] += wv[j];
        }
    }
}

void sha256_init(sha256_ctx *ctx)
{
    int i;
    for (i = 0; i < 8; i++) {
        ctx->h[i] = sha256_h0[i];
    }
	
    ctx->len = 0;
    ctx->tot_len = 0;
}

void sha256_update(sha256_ctx *ctx, const unsigned char *message,
                   unsigned int len)
{
    unsigned int block_nb;
    unsigned int new_len, rem_len, tmp_len;
    const unsigned char *shifted_message;
	
    tmp_len = SHA256_BLOCK_SIZE - ctx->len;
    rem_len = len < tmp_len ? len : tmp_len;
	
    memcpy(&ctx->block[ctx->len], message, rem_len);
	
    if (ctx->len + len < SHA256_BLOCK_SIZE) {
        ctx->len += len;
        return;
    }
	
    new_len = len - rem_len;
    block_nb = new_len / SHA256_BLOCK_SIZE;
	
    shifted_message = message + rem_len;
	
    sha256_transf(ctx, ctx->block, 1);
    sha256_transf(ctx, shifted_message, block_nb);
	
    rem_len = new_len % SHA256_BLOCK_SIZE;
	
    memcpy(ctx->block, &shifted_message[block_nb << 6],
		rem_len);
	
    ctx->len = rem_len;
    ctx->tot_len += (block_nb + 1) << 6;
}

void sha256_final(sha256_ctx *ctx, unsigned char *digest)
{
    unsigned int block_nb;
    unsigned int pm_len;
    unsigned int len_b;
	
    int i;
	
    block_nb = (1 + ((SHA256_BLOCK_SIZE - 9)
		< (ctx->len % SHA256_BLOCK_SIZE)));
	
    len_b = (ctx->tot_len + ctx->len) << 3;
    pm_len = block_nb << 6;
	
    memset(ctx->block + ctx->len, 0, pm_len - ctx->len);
    ctx->block[ctx->len] = 0x80;
    UNPACK32(len_b, ctx->block + pm_len - 4);
	
    sha256_transf(ctx, ctx->block, block_nb);
	
    for (i = 0 ; i < 8; i++) {
        UNPACK32(ctx->h[i], &digest[i << 2]);
    }
}

void sha256(const unsigned char *message, unsigned int len, unsigned char *digest)
{
    sha256_ctx ctx;
	
    sha256_init(&ctx);
    sha256_update(&ctx, message, len);
    sha256_final(&ctx, digest);
}