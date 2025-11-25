/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : DES
 * FILE       : DES.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
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


//
// For CSR_DES.H.
// The macro DES_EXPORTS should be defined in the project file DES.DSP
//
#ifndef DES_EXPORTS
#   define DES_EXPORTS
#endif
#include <CSR_DES.h>


#define LOC_DEF
#include <DES_loc.h>
#undef LOC_DEF

//
// The file MEMCLASS.H must be included at the last position
//
#include <memclass.h>




//
// PRIVATE FUNCTIONS PROTOTYPES
//

PRIVATE void WINAPI _DESEncrypt(
        IN OUT  DES_LONG          * data,
        IN      des_key_schedule    ks,
        IN      int                 encrypt );

PRIVATE void WINAPI _DESEncrypt2(
        IN OUT  DES_LONG          * data,
        IN      des_key_schedule    ks,
        IN      int                 encrypt );

PRIVATE void WINAPI _DESEncrypt3(
        IN OUT  DES_LONG          * data,
        IN      des_key_schedule    ks1,
        IN      des_key_schedule    ks2,
        IN      des_key_schedule    ks3 );
        
PRIVATE void WINAPI _DESDecrypt3(
        IN OUT  DES_LONG          * data,
        IN      des_key_schedule    ks1,
        IN      des_key_schedule    ks2,
        IN      des_key_schedule    ks3 );

PRIVATE int WINAPI _DESCheckParity(
        IN      des_cblock        * key );

PRIVATE void WINAPI _DESEncryptEcb3(
        IN      des_cblock        * input,
        OUT     des_cblock        * output, 
        IN      des_key_schedule    ks1,
        IN      des_key_schedule    ks2,
        IN      des_key_schedule    ks3, 
        IN      int                 encrypt );

PRIVATE void WINAPI _DESEncryptEcb(
        IN      des_cblock        * input,
        OUT     des_cblock        * output, 
        IN      des_key_schedule    ks,
        IN      int                 encrypt );

PRIVATE void WINAPI _DESSetOddParity( 
        IN OUT  des_cblock        * key );

PRIVATE int WINAPI _DESIsWeakKey(
        IN      des_cblock        * key );

PRIVATE int WINAPI _DESSetKey(
        IN      int                 check,
        IN      des_cblock        * key,
        OUT     des_key_schedule schedule );




//
// EXPORTED FUNCTIONS CODE
//


EXPORT HDESKEY WINAPI DESOpenKey( 
        IN      BYTE        tbKeyBytes[8],
        IN      BOOL        bVerifyKey )
{
    DWORD           dwErr = NO_ERROR;
    int             iResult;
    DESKEY_STRUCT * psKey = NULL;
    DWORD         * pdwKeyCount = NULL;

    __try
    {
        pdwKeyCount = HeapAlloc( GetProcessHeap(), 0, sizeof(*pdwKeyCount) + sizeof(*psKey) );
        if ( pdwKeyCount == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        psKey = (DESKEY_STRUCT*)( pdwKeyCount + 1 );
        (*pdwKeyCount) = 1 ;

        iResult = _DESSetKey( bVerifyKey, (des_cblock*)tbKeyBytes, psKey->tsKeySched );
        if ( iResult == -1 )
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }
        else if ( iResult == -2 )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
			if (pdwKeyCount != NULL)
			{
				HeapFree(GetProcessHeap(), 0, pdwKeyCount);
				pdwKeyCount = NULL;
			}
        }

        SetLastError( dwErr );
    }

	return (HDESKEY)pdwKeyCount;
}




EXPORT HDESKEY WINAPI DESOpenTripleKey( 
        IN      BYTE        ttbKeyBytes[3][8],
        IN      BOOL        bVerifyKey )
{
    DWORD           dwErr = NO_ERROR;
    DWORD           dwIndex;
    int             iResult;
    DESKEY_STRUCT * psKey = NULL;
    DWORD         * pdwKeyCount = NULL;

    __try
    {
        pdwKeyCount = HeapAlloc( GetProcessHeap(), 0, sizeof(*pdwKeyCount) + sizeof(*psKey) * 3 );
        if ( pdwKeyCount == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        psKey = (DESKEY_STRUCT*)( pdwKeyCount + 1 );
        (*pdwKeyCount) = 3 ;

        for ( dwIndex = 0 ; dwIndex < 3 ; dwIndex ++ )
        {
            iResult = _DESSetKey( bVerifyKey, (des_cblock*)(ttbKeyBytes[dwIndex]), psKey[dwIndex].tsKeySched );
            if ( iResult == -1 )
            {
                dwErr = ERROR_INVALID_DATA;
                __leave;
            }
            else if ( iResult == -2 )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }
        }
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            if ( pdwKeyCount != NULL )
            {
                HeapFree( GetProcessHeap(), 0, pdwKeyCount );
                pdwKeyCount = NULL;
            }
        }

        SetLastError( dwErr );
    }

	return (HDESKEY)pdwKeyCount;
}







EXPORT void WINAPI DESCloseKey( 
        IN      HDESKEY     hDesKey )
{
    HeapFree( GetProcessHeap(), 0, hDesKey );
}


EXPORT BOOL WINAPI DESEncryptEcb( 
        IN      BOOL        bTriple,
        IN      BOOL        bEncrypt,
        IN      BYTE      * pbSrcBlock, 
        IN      DWORD       dwSrcBlockBytes,
        OUT     BYTE      * pbDstBlock,
        IN OUT  DWORD     * pdwDstBlockBytes,
        IN      HDESKEY     hDesKey )
{
    DWORD           dwErr = NO_ERROR;
    DWORD           dwIndex;
    DWORD           dwNeeded;
    DWORD           dwLast;
    BOOL            bIncomplete;
    DWORD         * pdwKeyCount;
    BYTE          * pbData;
    DESKEY_STRUCT * psKey;
    BYTE            tbBuffer[8];

    __try
    {
        if ( dwSrcBlockBytes == 0 )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }

        pdwKeyCount = hDesKey;
        if ( ( ( ! bTriple ) && ( (*pdwKeyCount) != 1 ) ) ||
             ( (   bTriple ) && ( (*pdwKeyCount) != 3 ) )
           )
        {
            dwErr = ERROR_INVALID_HANDLE;
            __leave;
        }

        dwNeeded = ( ( dwSrcBlockBytes + 7 ) / 8 ) * 8;
        if ( (*pdwDstBlockBytes) < dwNeeded )
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            __leave;
        }

        *pdwDstBlockBytes = dwNeeded;

        dwLast = dwNeeded - 8;
        bIncomplete = ( dwNeeded != dwSrcBlockBytes );

        psKey = (DESKEY_STRUCT*)( pdwKeyCount + 1 );
        
        for ( dwIndex = 0 ; dwIndex < dwNeeded ; dwIndex += 8 )
        {
            if ( ( dwIndex == dwLast ) && bIncomplete )
            {
                ZeroMemory( tbBuffer, sizeof(tbBuffer) );
                memcpy( tbBuffer, pbSrcBlock + dwIndex, dwSrcBlockBytes - dwIndex );
                pbData = tbBuffer;
            }
            else
            {
                pbData = pbSrcBlock + dwIndex;
            }

            if ( bTriple )
            {
                _DESEncryptEcb3( 
                    (des_cblock*)pbData, 
                    (des_cblock*)(pbDstBlock + dwIndex), 
                    psKey[0].tsKeySched, 
                    psKey[1].tsKeySched, 
                    psKey[2].tsKeySched, 
                    bEncrypt );
            }
            else
            {
                _DESEncryptEcb(
                    (des_cblock*)pbData, 
                    (des_cblock*)(pbDstBlock + dwIndex), 
                    psKey->tsKeySched, 
                    bEncrypt );
            }
        }
    }
    __finally
    {
        SetLastError( dwErr );
    }

	return (dwErr == NO_ERROR);
   
}




EXPORT BOOL WINAPI DESEncryptCbc( 
        IN      BOOL        bTriple,
        IN      BOOL        bEncrypt,
        IN      BYTE      * pbSrcBlock, 
        IN      DWORD       dwSrcBlockBytes,
        OUT     BYTE      * pbDstBlock,
        IN OUT  DWORD     * pdwDstBlockBytes,
        IN      HDESKEY     hDesKey )
{
    DWORD           dwErr = NO_ERROR;
    DWORD           dwIndex;
    DWORD           dwNeeded;
    DWORD           dwLast;
    BOOL            bIncomplete;
    DWORD         * pdwKeyCount;
    BYTE          * pbData;
    DESKEY_STRUCT * psKey;
    BYTE            tbBuffer[8];
    BYTE            tbWork[8];

    __try
    {
        if ( dwSrcBlockBytes == 0 )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }

        pdwKeyCount = hDesKey;
        if ( ( ( ! bTriple ) && ( (*pdwKeyCount) != 1 ) ) ||
             ( (   bTriple ) && ( (*pdwKeyCount) != 3 ) )
           )
        {
            dwErr = ERROR_INVALID_HANDLE;
            __leave;
        }

        dwNeeded = ( ( dwSrcBlockBytes + 7 ) / 8 ) * 8;
        if ( (*pdwDstBlockBytes) < dwNeeded )
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            __leave;
        }

        *pdwDstBlockBytes = dwNeeded;

        dwLast = dwNeeded - 8;
        bIncomplete = ( dwNeeded != dwSrcBlockBytes );

        psKey = (DESKEY_STRUCT*)( pdwKeyCount + 1 );

        *(ULONGLONG*)tbWork = (ULONGLONG)0;

        if ( bEncrypt )
        {
            for ( dwIndex = 0 ; dwIndex < dwNeeded ; dwIndex += 8 )
            {
                if ( ( dwIndex == dwLast ) && bIncomplete )
                {
                    ZeroMemory( tbBuffer, sizeof(tbBuffer) );
                    memcpy( tbBuffer, pbSrcBlock + dwIndex, dwSrcBlockBytes - dwIndex );
                    pbData = tbBuffer;
                }
                else
                {
                    pbData = pbSrcBlock + dwIndex;
                }

                *(ULONGLONG*)tbWork = *(ULONGLONG*)tbWork ^ *(ULONGLONG*)pbData;

                if ( bTriple )
                {
                    _DESEncryptEcb3( 
                        (des_cblock*)tbWork, 
                        (des_cblock*)(pbDstBlock + dwIndex), 
                        psKey[0].tsKeySched, 
                        psKey[1].tsKeySched, 
                        psKey[2].tsKeySched, 
                        TRUE );
                }
                else
                {
                    _DESEncryptEcb(
                        (des_cblock*)tbWork, 
                        (des_cblock*)(pbDstBlock + dwIndex), 
                        psKey->tsKeySched, 
                        TRUE );
                }
                
                *(ULONGLONG*)tbWork = *(ULONGLONG*)(pbDstBlock + dwIndex);
            }
        }
        else
        {
            for ( dwIndex = 0 ; dwIndex < dwNeeded ; dwIndex += 8 )
            {
                if ( ( dwIndex == dwLast ) && bIncomplete )
                {
                    ZeroMemory( tbBuffer, sizeof(tbBuffer) );
                    memcpy( tbBuffer, pbSrcBlock + dwIndex, dwSrcBlockBytes - dwIndex );
                    pbData = tbBuffer;
                }
                else
                {
                    pbData = pbSrcBlock + dwIndex;
                }

                if ( bTriple )
                {
                    _DESEncryptEcb3( 
                        (des_cblock*)pbData, 
                        (des_cblock*)(pbDstBlock + dwIndex), 
                        psKey[0].tsKeySched, 
                        psKey[1].tsKeySched, 
                        psKey[2].tsKeySched, 
                        FALSE );
                }
                else
                {
                    _DESEncryptEcb(
                        (des_cblock*)pbData, 
                        (des_cblock*)(pbDstBlock + dwIndex), 
                        psKey->tsKeySched, 
                        FALSE );
                }

               *(ULONGLONG*)(pbDstBlock + dwIndex) = *(ULONGLONG*)(pbDstBlock + dwIndex) ^ *(ULONGLONG*)tbWork;
               *(ULONGLONG*)tbWork = *(ULONGLONG*)pbData;
            }
        }
    }
    __finally
    {
        SetLastError( dwErr );
    }  

	return (dwErr == NO_ERROR);
}


//
// PRIVATE FUNCTIONS CODE
//


PRIVATE void WINAPI _DESEncryptEcb3(
        IN      des_cblock        * input,
        OUT     des_cblock        * output, 
        IN      des_key_schedule    ks1,
        IN      des_key_schedule    ks2,
        IN      des_key_schedule    ks3, 
        IN      int                 encrypt )
{
	register DES_LONG l0;
    register DES_LONG l1;
	register unsigned char * in;
    register unsigned char * out;
	DES_LONG ll[2];

	in=(unsigned char *)input;
	out=(unsigned char *)output;
	c2l(in,l0);
	c2l(in,l1);
	ll[0]=l0;
	ll[1]=l1;
	if (encrypt)
		_DESEncrypt3(ll,ks1,ks2,ks3);
	else
		_DESDecrypt3(ll,ks1,ks2,ks3);
	l0=ll[0];
	l1=ll[1];
	l2c(l0,out);
	l2c(l1,out);
}




PRIVATE void WINAPI _DESEncryptEcb(
        IN      des_cblock        * input,
        OUT     des_cblock        * output, 
        IN      des_key_schedule    ks,
        IN      int                 encrypt )
{
	register DES_LONG l;
	register unsigned char *in,*out;
	DES_LONG ll[2];

	in=(unsigned char *)input;
	out=(unsigned char *)output;
	c2l(in,l); ll[0]=l;
	c2l(in,l); ll[1]=l;
	_DESEncrypt(ll,ks,encrypt);
	l=ll[0]; l2c(l,out);
	l=ll[1]; l2c(l,out);
	l=ll[0]=ll[1]=0;
}




PRIVATE void WINAPI _DESSetOddParity( 
        IN OUT  des_cblock        * key )
{
	int i;

	for (i=0; i<DES_KEY_SZ; i++)
		(*key)[i]=odd_parity[(*key)[i]];
}




PRIVATE int WINAPI _DESIsWeakKey(
        IN      des_cblock        * key )
{
	int i;

	for (i=0; i<NUM_WEAK_KEY; i++)
		/* Added == 0 to comparision, I obviously don't run
		 * this section very often :-(, thanks to
		 * engineering@MorningStar.Com for the fix
		 * eay 93/06/29 */
		if (memcmp(weak_keys[i],key,sizeof(key)) == 0) return(1);
	return(0);
}




/* return 0 if key parity is odd (correct),
 * return -1 if key parity error,
 * return -2 if illegal weak key.
 */
PRIVATE int WINAPI _DESSetKey(
        IN      int                 check,
        IN      des_cblock        * key,
        OUT     des_key_schedule schedule )
{
	static int shifts2[16]={0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0};
	register DES_LONG c,d,t,s,t2;
	register unsigned char *in;
	register DES_LONG *k;
	register int i;

	if (check)
		{
		if (!_DESCheckParity(key))
			return(-1);

		if (_DESIsWeakKey(key))
			return(-2);
		}

	k=(DES_LONG *)schedule;
	in=(unsigned char *)key;

	c2l(in,c);
	c2l(in,d);

	/* do PC1 in 60 simple operations */ 
/*	PERM_OP(d,c,t,4,0x0f0f0f0fL);
	HPERM_OP(c,t,-2, 0xcccc0000L);
	HPERM_OP(c,t,-1, 0xaaaa0000L);
	HPERM_OP(c,t, 8, 0x00ff0000L);
	HPERM_OP(c,t,-1, 0xaaaa0000L);
	HPERM_OP(d,t,-8, 0xff000000L);
	HPERM_OP(d,t, 8, 0x00ff0000L);
	HPERM_OP(d,t, 2, 0x33330000L);
	d=((d&0x00aa00aaL)<<7L)|((d&0x55005500L)>>7L)|(d&0xaa55aa55L);
	d=(d>>8)|((c&0xf0000000L)>>4);
	c&=0x0fffffffL; */

	/* I now do it in 47 simple operations :-)
	 * Thanks to John Fletcher (john_fletcher@lccmail.ocf.llnl.gov)
	 * for the inspiration. :-) */
	PERM_OP (d,c,t,4,0x0f0f0f0fL);
	HPERM_OP(c,t,-2,0xcccc0000L);
	HPERM_OP(d,t,-2,0xcccc0000L);
	PERM_OP (d,c,t,1,0x55555555L);
	PERM_OP (c,d,t,8,0x00ff00ffL);
	PERM_OP (d,c,t,1,0x55555555L);
	d=	(((d&0x000000ffL)<<16L)| (d&0x0000ff00L)     |
		 ((d&0x00ff0000L)>>16L)|((c&0xf0000000L)>>4L));
	c&=0x0fffffffL;

	for (i=0; i<ITERATIONS; i++)
		{
		if (shifts2[i])
			{ c=((c>>2L)|(c<<26L)); d=((d>>2L)|(d<<26L)); }
		else
			{ c=((c>>1L)|(c<<27L)); d=((d>>1L)|(d<<27L)); }
		c&=0x0fffffffL;
		d&=0x0fffffffL;
		/* could be a few less shifts but I am to lazy at this
		 * point in time to investigate */
		s=	des_skb[0][ (c    )&0x3f                ]|
			des_skb[1][((c>> 6)&0x03)|((c>> 7L)&0x3c)]|
			des_skb[2][((c>>13)&0x0f)|((c>>14L)&0x30)]|
			des_skb[3][((c>>20)&0x01)|((c>>21L)&0x06) |
						  ((c>>22L)&0x38)];
		t=	des_skb[4][ (d    )&0x3f                ]|
			des_skb[5][((d>> 7L)&0x03)|((d>> 8L)&0x3c)]|
			des_skb[6][ (d>>15L)&0x3f                ]|
			des_skb[7][((d>>21L)&0x0f)|((d>>22L)&0x30)];

		/* table contained 0213 4657 */
		t2=((t<<16L)|(s&0x0000ffffL))&0xffffffffL;
		*(k++)=ROTATE(t2,30)&0xffffffffL;

		t2=((s>>16L)|(t&0xffff0000L));
		*(k++)=ROTATE(t2,26)&0xffffffffL;
		}
	return(0);
}




PRIVATE void WINAPI _DESEncrypt(
        IN OUT  DES_LONG          * data,
        IN      des_key_schedule    ks,
        IN      int                 encrypt )
{
    register DES_LONG l;
    register DES_LONG r;
    register DES_LONG t;
    register DES_LONG u;
    register int i;
    register DES_LONG * s;

    r=data[0];
    l=data[1];

    IP(r,l);
    /* clear the top bits on machines with 8byte longs */
    /* shift left by 2 */
    r=ROTATE(r,29)&0xffffffffL;
    l=ROTATE(l,29)&0xffffffffL;

    s=(DES_LONG *)ks;
    if (encrypt)
    {
        for (i=0; i<32; i+=8)
        {
            D_ENCRYPT(l,r,i+0); /*  1 */
            D_ENCRYPT(r,l,i+2); /*  2 */
            D_ENCRYPT(l,r,i+4); /*  3 */
            D_ENCRYPT(r,l,i+6); /*  4 */
        }
    }
    else
    {
        for (i=30; i>0; i-=8)
        {
            D_ENCRYPT(l,r,i-0); /* 16 */
            D_ENCRYPT(r,l,i-2); /* 15 */
            D_ENCRYPT(l,r,i-4); /* 14 */
            D_ENCRYPT(r,l,i-6); /* 13 */
        }
    }

    /* rotate and clear the top bits on machines with 8byte longs */
    l=ROTATE(l,3)&0xffffffffL;
    r=ROTATE(r,3)&0xffffffffL;

    FP(r,l);
    data[0]=l;
    data[1]=r;
    l=r=t=u=0;
}




PRIVATE void WINAPI _DESEncrypt2(
        IN OUT  DES_LONG          * data,
        IN      des_key_schedule    ks,
        IN      int                 encrypt )
{
    register DES_LONG l;
    register DES_LONG r;
    register DES_LONG t;
    register DES_LONG u;
    register int i;
    register DES_LONG * s;

    r=data[0];
    l=data[1];

    /* clear the top bits on machines with 8byte longs */
    r=ROTATE(r,29)&0xffffffff;
    l=ROTATE(l,29)&0xffffffff;

    s=(DES_LONG *)ks;
    if (encrypt)
    {
        for (i=0; i<32; i+=8)
        {
            D_ENCRYPT(l,r,i+0); /*  1 */
            D_ENCRYPT(r,l,i+2); /*  2 */
            D_ENCRYPT(l,r,i+4); /*  3 */
            D_ENCRYPT(r,l,i+6); /*  4 */
        }
    }
    else
    {
        for (i=30; i>0; i-=8)
        {
            D_ENCRYPT(l,r,i-0); /* 16 */
            D_ENCRYPT(r,l,i-2); /* 15 */
            D_ENCRYPT(l,r,i-4); /* 14 */
            D_ENCRYPT(r,l,i-6); /* 13 */
        }
    }
    /* rotate and clear the top bits on machines with 8byte longs */
    data[0]=ROTATE(l,3)&0xffffffff;
    data[1]=ROTATE(r,3)&0xffffffff;
    l=r=t=u=0;
}




PRIVATE void WINAPI _DESEncrypt3(
        IN OUT  DES_LONG          * data,
        IN      des_key_schedule    ks1,
        IN      des_key_schedule    ks2,
        IN      des_key_schedule    ks3 )
{
    register DES_LONG l;
    register DES_LONG r;

    l=data[0];
    r=data[1];
    IP(l,r);
    data[0]=l;
    data[1]=r;
    _DESEncrypt2((DES_LONG *)data,ks1,DES_ENCRYPT);
    _DESEncrypt2((DES_LONG *)data,ks2,DES_DECRYPT);
    _DESEncrypt2((DES_LONG *)data,ks3,DES_ENCRYPT);
    l=data[0];
    r=data[1];
    FP(r,l);
    data[0]=l;
    data[1]=r;
}




PRIVATE void WINAPI _DESDecrypt3(
        IN OUT  DES_LONG          * data,
        IN      des_key_schedule    ks1,
        IN      des_key_schedule    ks2,
        IN      des_key_schedule    ks3 )
{
    register DES_LONG l;
    register DES_LONG r;

    l=data[0];
    r=data[1];
    IP(l,r);
    data[0]=l;
    data[1]=r;
    _DESEncrypt2((DES_LONG *)data,ks3,DES_DECRYPT);
    _DESEncrypt2((DES_LONG *)data,ks2,DES_ENCRYPT);
    _DESEncrypt2((DES_LONG *)data,ks1,DES_DECRYPT);
    l=data[0];
    r=data[1];
    FP(r,l);
    data[0]=l;
    data[1]=r;
}




PRIVATE int WINAPI _DESCheckParity(
        IN      des_cblock        * key )
{
	int i;

	for (i=0; i<DES_KEY_SZ; i++)
	{
		if ((*key)[i] != odd_parity[(*key)[i]])
			return(0);
	}
	return(1);
}


