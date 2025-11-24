/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : BITBL
 * FILE       : BITBL.C
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
// For BITBL.H.
// The macro BITBL_EXPORTS should be defined in the project file BITBL.DSP
//
#ifndef BITBL_EXPORTS
#   define BITBL_EXPORTS
#endif
#include <CSR_BITBL.h>


#define LOC_DEF
#include <BITBL_loc.h>
#undef LOC_DEF

//
// The file MEMCLASS.H must be included at the last postion
//
#include <memclass.h>


//
// PRIVATE MACROS (ONLY VISIBLE FROM THIS SOURCE FILE)
//

#define GENERIC_CODE


//
// Tests de validité d'indexes
//
#define CHK_PTR(x)              if ( (x) == NULL ) __leave; else if ( ! BITBL_IS_VALID(*(x)) ) __leave;
#define CHK_VAL(x)              if ( ! BITBL_IS_VALID(x) ) __leave;


//
// Valeurs prédéfinies pour le traitement
//
#define BYTE_BITS   (sizeof(BYTE)*8)
#define BYTE_FULL   ((BYTE)(-1))


//
// PRIVATE DATA TYPES
//

//
// Insert comments for data types before each one
//
typedef char BITBL_STRING[256];




//
// PRIVATE GLOBAL VARIABLES
//

// Insert comments for global variables before each variable definition
PRIVATE HINSTANCE ghInstanceDll = NULL;




//
// PRIVATE FUNCTIONS PROTOTYPES
//

PRIVATE void WINAPI _BITBLBitBlt(
        OUT     BYTE     * pDst,
        IN      BYTE            bDstBit,
        IN      BYTE     * pSrc,
        IN      BYTE            bSrcBit,
        IN      DWORD           dwBitLen );




//
// EXPORTED FUNCTIONS CODE
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI BITBLCopy( 
 *                      IN OUT  void      * pvDst, 
 *                      IN OUT  DWORD     * pdwDstBitPos,
 *                      IN OUT  DWORD     * pdwDstBitLen,
 *                      IN      void      * pvSrc, 
 *                      IN OUT  DWORD     * pdwSrcBitPos, 
 *                      IN OUT  DWORD     * pdwSrcBitLen,
 *                      IN      DWORD       dwLen );
 * PARAMETERS: pvDst        : Pointe sur le premier octet du buffer destination
 *             pdwDstBitPos : En entrée, Position du premier bit dans le buffer de destination.
 *                            En sortie, Position du bit immédiatement après le bloc copie.
 *             pdwDstBitLen : En entrée, nombre de bits disponibles à partir de la position donnée en entrée.
 *                            En sortie, nombre de bits restant disponibles à partir de la position donnée en sortie.
 *             pvSrc        : Pointe sur le premier octet du buffer source
 *             pdwSrcBitPos : En entrée, Position du premier bit dans le buffer source.
 *                            En sortie, Position du bit immédiatement après le bloc copie.
 *             pdwSrcBitLen : En entrée, nombre de bits disponibles à partir de la position donnée en entrée.
 *                            En sortie, nombre de bits restant disponibles à partir de la position donnée en sortie.
 *             dwLen        : Nombre de bits à copier.
 * RETURN    : TRUE succes, FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Recopie d'une série de bits d'un buffer vers un autre, avec
 *             gestion de curseur sur la source et la destination.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI BITBLCopy( 
        IN OUT  void      * pvDst, 
        IN OUT  DWORD     * pdwDstBitPos,
        IN OUT  DWORD     * pdwDstBitLen,
        IN      void      * pvSrc, 
        IN OUT  DWORD     * pdwSrcBitPos, 
        IN OUT  DWORD     * pdwSrcBitLen,
        IN      DWORD       dwLen )
{
    BOOL bReturn = FALSE;
    BYTE * pbSrc;
    BYTE * pbDst;
    BYTE   bSrc;
    BYTE   bDst;

    __try
    {
        CHK_PTR( pdwDstBitPos )
        CHK_PTR( pdwDstBitLen )
        CHK_PTR( pdwSrcBitPos )
        CHK_PTR( pdwSrcBitLen )
        CHK_VAL( dwLen )

        if ( (*pdwSrcBitLen) < dwLen ) __leave;
        if ( (*pdwDstBitLen) < dwLen ) __leave;

        pbSrc = ((BYTE*)pvSrc) + ( (*pdwSrcBitPos) / 8 );
        bSrc = (BYTE)( (*pdwSrcBitPos) % 8 );

        pbDst = ((BYTE*)pvDst) + ( (*pdwDstBitPos) / 8 );
        bDst = (BYTE)( (*pdwDstBitPos) % 8 );

        _BITBLBitBlt( pbDst, bDst, pbSrc, bSrc, dwLen );

        (*pdwDstBitLen) -= dwLen;
        (*pdwDstBitPos) += dwLen;
        (*pdwSrcBitLen) -= dwLen;
        (*pdwSrcBitPos) += dwLen;

        bReturn = TRUE;
    }
    __finally
    {
		;
    }

	return bReturn;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI BITBLCopyToBuffer(
 *                      IN OUT  void      * pvDst, 
 *                      IN OUT  DWORD     * pdwDstBitPos,
 *                      IN OUT  DWORD     * pdwDstBitLen,
 *                      IN      void      * pvSrc, 
 *                      IN      DWORD       dwSrcBitPos, 
 *                      IN      DWORD       dwSrcBitLen );
 * PARAMETERS: pvDst        : Pointe sur le premier octet du buffer destination
 *             pdwDstBitPos : En entrée, Position du premier bit dans le buffer de destination.
 *                            En sortie, Position du bit immédiatement après le bloc copie.
 *             pdwDstBitLen : En entrée, nombre de bits disponibles à partir de la position donnée en entrée.
 *                            En sortie, nombre de bits restant disponibles à partir de la position donnée en sortie.
 *             pvSrc        : Pointe sur le premier octet du buffer source
 *             dwSrcBitPos  : Position du premier bit dans le buffer source.
 *             dwSrcBitLen  : Nombre de bits à recopier à partir de la position donnée.
 * RETURN    : TRUE succes, FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Recopie d'une série de bits d'un buffer vers un autre, avec
 *             gestion de curseur sur la destination uniquement.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI BITBLCopyToBuffer(
        IN OUT  void      * pvDst, 
        IN OUT  DWORD     * pdwDstBitPos,
        IN OUT  DWORD     * pdwDstBitLen,
        IN      void      * pvSrc, 
        IN      DWORD       dwSrcBitPos, 
        IN      DWORD       dwSrcBitLen )
{
    return BITBLCopy( pvDst, pdwDstBitPos, pdwDstBitLen, pvSrc, &dwSrcBitPos, &dwSrcBitLen, dwSrcBitLen );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI BITBLCopyFromBuffer(
 *                      IN OUT  void      * pvDst, 
 *                      IN      DWORD       dwDstBitPos,
 *                      IN      DWORD       dwDstBitLen,
 *                      IN      void      * pvSrc, 
 *                      IN OUT  DWORD     * pdwSrcBitPos, 
 *                      IN OUT  DWORD     * pdwSrcBitLen );
 * PARAMETERS: pvDst        : Pointe sur le premier octet du buffer destination
 *             dwDstBitPos  : Position du premier bit dans le buffer de destination.
 *             dwDstBitLen  : Nombre de bits à recopier.
 *             pvSrc        : Pointe sur le premier octet du buffer source
 *             pdwSrcBitPos : En entrée, Position du premier bit dans le buffer source.
 *                            En sortie, Position du bit immédiatement après le bloc copie.
 *             pdwSrcBitLen : En entrée, nombre de bits disponibles à partir de la position donnée en entrée.
 *                            En sortie, nombre de bits restant disponibles à partir de la position donnée en sortie.
 * RETURN    : TRUE succes, FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Recopie d'une série de bits d'un buffer vers un autre, avec
 *             gestion de curseur sur la source uniquement.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI BITBLCopyFromBuffer(
        IN OUT  void      * pvDst, 
        IN      DWORD       dwDstBitPos,
        IN      DWORD       dwDstBitLen,
        IN      void      * pvSrc, 
        IN OUT  DWORD     * pdwSrcBitPos, 
        IN OUT  DWORD     * pdwSrcBitLen )
{
    return BITBLCopy( pvDst, &dwDstBitPos, &dwDstBitLen, pvSrc, pdwSrcBitPos, pdwSrcBitLen, dwDstBitLen );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI BITBLGetNumber( 
 *                      OUT     DWORD     * pdwVal,
 *                      IN      DWORD       dwValBits,
 *                      IN      void      * pvSrc, 
 *                      IN OUT  DWORD     * pdwSrcBitPos, 
 *                      IN OUT  DWORD     * pdwSrcBitLen );
 * PARAMETERS: pdwVal       : Récupère la valeur numérique à lire
 *             dwValBits    : Nombre de bits constituant la valeur numérique (de 1 à 32)
 *             pvSrc        : Pointe sur le premier octet du buffer source
 *             pdwSrcBitPos : En entrée, Position du premier bit dans le buffer source.
 *                            En sortie, Position du bit immédiatement après le bloc copie.
 *             pdwSrcBitLen : En entrée, nombre de bits disponibles à partir de la position donnée en entrée.
 *                            En sortie, nombre de bits restant disponibles à partir de la position donnée en sortie.
 * RETURN    : TRUE succes, FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Lecture d'une valeur numérique d'après une serie de bits avec
 *             gestion de curseur sur la source.
 *             La lecture est faite poid fort en tête.
 *             La valeur (*pdwVal) est codée poid faible en tête.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI BITBLGetNumber( 
        OUT     DWORD     * pdwVal,
        IN      DWORD       dwValBits,
        IN      void      * pvSrc, 
        IN OUT  DWORD     * pdwSrcBitPos, 
        IN OUT  DWORD     * pdwSrcBitLen )
{   
    BOOL bReturn = FALSE;
    DWORD dwVal = 0;
    __try
    {
        if ( ! BITBLCopyFromBuffer( &dwVal, 32 - dwValBits, dwValBits, pvSrc, pdwSrcBitPos, pdwSrcBitLen ) )
            __leave;

        dwVal = BITBLSwitchDW( dwVal );
        memcpy( pdwVal, &dwVal, ( dwValBits + 7 ) / 8 );
        bReturn = TRUE;
    }
    __finally
    {
		;
    }

	return bReturn;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI BITBLSetNumber( 
 *                      IN      DWORD       dwVal,
 *                      IN      DWORD       dwValBits,
 *                      OUT     void      * pvDst, 
 *                      IN OUT  DWORD     * pdwDstBitPos, 
 *                      IN OUT  DWORD     * pdwDstBitLen );
 * PARAMETERS: dwVal        : Valeur numérique à écrire
 *             dwValBits    : Nombre de bits constituant la valeur numérique (de 1 à 32)
 *             pvDst        : Pointe sur le premier octet du buffer destination
 *             pdwDstBitPos : En entrée, Position du premier bit dans le buffer destination.
 *                            En sortie, Position du bit immédiatement après le bloc copié.
 *             pdwDstBitLen : En entrée, nombre de bits disponibles à partir de la position donnée en entrée.
 *                            En sortie, nombre de bits restant disponibles à partir de la position donnée en sortie.
 * RETURN    : TRUE succes, FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Ecriture d'une serie de bits d'après une valeur numérique avec
 *             gestion de curseur sur la destination.
 *             L'écriture est faite poid fort en tête.
 *             La valeur (dwVal) est lue poid faible en tête.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI BITBLSetNumber( 
        IN      DWORD       dwVal,
        IN      DWORD       dwValBits,
        OUT     void      * pvDst, 
        IN OUT  DWORD     * pdwDstBitPos, 
        IN OUT  DWORD     * pdwDstBitLen )
{   
    BOOL bReturn = FALSE;
    __try
    {
        dwVal = BITBLSwitchDW( dwVal );
        if ( ! BITBLCopyToBuffer( pvDst, pdwDstBitPos, pdwDstBitLen, &dwVal, 32-dwValBits, dwValBits ) )
            __leave;
        bReturn = TRUE;
    }
    __finally
    {
		;
    }

	return bReturn;
}




#ifdef GENERIC_CODE




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT WORD WINAPI BITBLSwitchW( 
 *                      IN      WORD        wVal );
 * PARAMETERS: wVal         : Valeur numérique à convertir
 * RETURN    : Valeur numérique convertie
 * --------------------------------------------------------------------
 * ROLE      : Effectue la permutation des octets d'un mot 16 bits.
 *             Equivaut à la conversion :
 *                 BIG ENDIAN > SMALL ENDIAN
 *             ou  SMALL ENDIAN > BIG ENDIAN
 * --------------------------------------------------------------------
 */
EXPORT WORD WINAPI BITBLSwitchW( 
        IN      WORD        wVal )
{
    register BYTE    bVal;

    bVal = ((BYTE*)&wVal)[0];
    ((BYTE*)&wVal)[0] = ((BYTE*)&wVal)[1];
    ((BYTE*)&wVal)[1] = bVal;

    return wVal;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI BITBLSwitchDW( 
 *                      IN      DWORD       dwVal );
 * PARAMETERS: dwVal         : Valeur numérique à convertir
 * RETURN    : Valeur numérique convertie
 * --------------------------------------------------------------------
 * ROLE      : Effectue la permutation des octets d'un mot 32 bits.
 *             Equivaut à la conversion :
 *                 BIG ENDIAN > SMALL ENDIAN
 *             ou  SMALL ENDIAN > BIG ENDIAN
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI BITBLSwitchDW( 
        IN      DWORD       dwVal )
{
    register BYTE    bVal;

    bVal = ((BYTE*)&dwVal)[0];
    ((BYTE*)&dwVal)[0] = ((BYTE*)&dwVal)[3];
    ((BYTE*)&dwVal)[3] = bVal;
    bVal = ((BYTE*)&dwVal)[1];
    ((BYTE*)&dwVal)[1] = ((BYTE*)&dwVal)[2];
    ((BYTE*)&dwVal)[2] = bVal;

    return dwVal;
}




#else




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT WORD WINAPI BITBLSwitchW( 
 *                      IN      WORD        wVal );
 * PARAMETERS: wVal         : Valeur numérique à convertir
 * RETURN    : Valeur numérique convertie
 * --------------------------------------------------------------------
 * ROLE      : Effectue la permutation des octets d'un mot 16 bits.
 *             Equivaut à la conversion :
 *                 BIG ENDIAN > SMALL ENDIAN
 *             ou  SMALL ENDIAN > BIG ENDIAN
 * --------------------------------------------------------------------
 */
EXPORT WORD WINAPI BITBLSwitchW( 
        IN      WORD        wVal )
{
    __asm 
    { 
        mov     ax              ,   wVal
        xchg    ah              ,   al
        mov     wVal            ,   ax
    }
    return wVal;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI BITBLSwitchDW( 
 *                      IN      DWORD       dwVal );
 * PARAMETERS: dwVal         : Valeur numérique à convertir
 * RETURN    : Valeur numérique convertie
 * --------------------------------------------------------------------
 * ROLE      : Effectue la permutation des octets d'un mot 32 bits.
 *             Equivaut à la conversion :
 *                 BIG ENDIAN > SMALL ENDIAN
 *             ou  SMALL ENDIAN > BIG ENDIAN
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI BITBLSwitchDW( 
        IN      DWORD       dwVal )
{
    __asm 
    { 
        mov     ax              ,   word ptr dwVal
        xchg    ah              ,   al
        xchg    ax              ,   word ptr dwVal + 2
        xchg    ah              ,   al
        mov     word ptr dwVal  ,   ax
    }
    return dwVal;
}




#endif



//
// PROJECT WIDE FUNCTIONS CODE
//




//
// PRIVATE FUNCTIONS CODE
//

#ifdef GENERIC_CODE


PRIVATE void WINAPI _BITBLBitBlt(
        OUT     BYTE     * pDst,
        IN      BYTE            bDstBit,
        IN      BYTE     * pSrc,
        IN      BYTE            bSrcBit,
        IN      DWORD           dwBitLen )
{
    BYTE       wMskFst;
    BYTE       wMskLst;
    BYTE       wMsk;
    BYTE       bShift;
    BYTE       bNShift;
    BYTE       bEnd;
    DWORD      dwPos;
    DWORD      dwLst;
    BYTE       wVal;

    // Remarque :   Dans un bitmap, le premier bit est le bit de poids fort
    //              du premier mot.
    // Par exemple sur des octets :
    // Octet          : ----------0-----------  ----------1-----------  ----------2-----------  
    // Bit de l'octet : 7  6  5  4  3  2  1  0  7  6  5  4  3  2  1  0  7  6  5  4  3  2  1  0
    // Bit du bitmap  : 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23

    bEnd    = (BYTE)( ( (DWORD)bDstBit + dwBitLen ) % BYTE_BITS );
    dwPos   = 0;
    dwLst   = ( bDstBit + dwBitLen ) / BYTE_BITS;
    wMskFst = BYTE_FULL >> bDstBit;
    wMskLst = BYTE_FULL << ( BYTE_BITS - bEnd );

    if ( bDstBit == bSrcBit )
    {
        // 
        // L'OFFSET DE BIT EST LE MÊME POUR LA SOURCE ET LA DESTINATION
        // 

        if ( dwLst == 0 )
        {
            // La copie tient intégralement dans le premier mot

            wMsk        = wMskFst & wMskLst;
            pDst[dwPos] = ( pDst[dwPos] & ~wMsk ) | ( pSrc[dwPos] & wMsk );
        }

        else
        {
            // La copie est au moins à cheval sur deux mots

            // Permier mot
            pDst[dwPos] = ( pDst[dwPos] & ~wMskFst ) | ( pSrc[dwPos] & wMskFst );

            // Recopier tels quels les mots intermédiaires
            while ( (++dwPos) != dwLst ) pDst[dwPos] = pSrc[dwPos];

            // Dernier mot
            if ( wMskLst != 0 ) 
            {
                wVal = pSrc[dwPos];
                pDst[dwPos] = ( pDst[dwPos] & ~wMskLst ) | ( wVal & wMskLst );
            }
        }
    }

    else if ( bDstBit < bSrcBit )
    {
        //
        // L'OFFSET DE BIT DE DESTINATION EST LE PLUS PETIT
        //

        // Décalage positif entre source et destination
        bShift = bSrcBit - bDstBit;
        bNShift = BYTE_BITS - bShift;

        if ( dwLst == 0 )
        {
            // La copie tient intégralement dans le premier mot

            wMsk        = wMskFst & wMskLst;
            wVal        = ( pSrc[dwPos] << bShift ) | ( pSrc[dwPos+1] >> bNShift );
            pDst[dwPos] = ( pDst[dwPos] & ~wMsk ) | ( wVal & wMsk );
        }

        else
        {
            // La copie est au moins à cheval sur deux mots

            // Permier mot
            wVal        = ( pSrc[dwPos] << bShift ) | ( pSrc[dwPos+1] >> bNShift );
            pDst[dwPos] = ( pDst[dwPos] & ~wMskFst ) | ( wVal & wMskFst );

            // Recopier tels quels les mots intermédiaires
            while ( (++dwPos) != dwLst ) pDst[dwPos] = ( pSrc[dwPos] << bShift ) | ( pSrc[dwPos+1] >> bNShift );

            // Dernier mot
            if ( wMskLst != 0 )
            {
                wVal = ( pSrc[dwPos] << bShift ) | ( pSrc[dwPos+1] >> bNShift );
                pDst[dwPos] = ( pDst[dwPos] & ~wMskLst ) | ( wVal & wMskLst );
            }
        }


    }
    else
    {
        //
        // L'OFFSET DE BIT DE DESTINATION EST LE PLUS GRAND
        //

        bShift = bDstBit - bSrcBit;
        bNShift = BYTE_BITS - bShift;

        // La copie tient intégralement dans le premier mot
        if ( dwLst == 0 )
        {
            wMsk        = wMskFst & wMskLst;
            wVal        = ( pSrc[dwPos] >> bShift ); 
            pDst[dwPos] = ( pDst[dwPos] & ~wMsk ) | ( wVal & wMsk );
        }
        else
        {
            // Permier mot
            pDst[dwPos] = ( pDst[dwPos] & ~wMskFst ) | ( ( pSrc[dwPos] >> bShift ) & wMskFst );

            // Recopier tels quels les mots intermédiaires
            while ( (++dwPos) != dwLst ) pDst[dwPos] = ( pSrc[dwPos-1] << bNShift ) | ( pSrc[dwPos] >> bShift );

            // Dernier mot
            if ( wMskLst != 0 )
            {
                wVal = ( pSrc[dwPos-1] << bNShift ) | ( pSrc[dwPos] >> bShift );
                pDst[dwPos] = ( pDst[dwPos] & ~wMskLst ) | ( wVal & wMskLst );
            }
        }
    }
}

#else

#pragma message( "LA VERSION OPTIMISEE UTILISANT L'ASSEMBLEUR 386 EST BUGGEE, UTILISER LA VERSION NON OPTIMISEE" )


PRIVATE void WINAPI _BITBLBitBlt(
        OUT     BYTE          * pbDst,
        IN      BYTE            bDstBit,
        IN      BYTE          * pbSrc,
        IN      BYTE            bSrcBit,
        IN      DWORD           dwBitLen )
{
    BYTE            bMsk;
    BYTE            bNMsk;
    BYTE            bEnd;
    DWORD           dwLst;

    // Remarque :   Dans un bitmap, le premier bit est le bit de poids fort
    //              du premier mot.
    // Par exemple sur des octets :
    // Octet          : ----------0-----------  ----------1-----------  ----------2-----------  
    // Bit de l'octet : 7  6  5  4  3  2  1  0  7  6  5  4  3  2  1  0  7  6  5  4  3  2  1  0
    // Bit du bitmap  : 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23

    __asm
    {
        // bl = bSrcBit
        // bh = bDstBit
        mov     bl              ,   bSrcBit
        mov     bh              ,   bDstBit

        // Mise à jour de bEnd
        xor     eax             ,   eax
        mov     al              ,   bh
        add     eax             ,   dwBitLen
        and     al              ,   07h
        mov     bEnd            ,   al

        // Mise à jour des indexes
        mov     esi             ,   pbSrc
        mov     edi             ,   pbDst
        
        // Mise à jour de dwLst
        xor     eax             ,   eax
        mov     al              ,   bh
        add     eax             ,   dwBitLen
        shr     eax             ,   03h
        add     eax             ,   edi
        mov     dwLst           ,   eax

        // Mise à jour des masques
        // dl = bMskFst
        // dh = bMskLst

        mov     dx              ,   0FFFFh
        mov     cl              ,   bh
        shr     dl              ,   cl
        mov     cl              ,   08h
        sub     cl              ,   bEnd
        shl     dh              ,   cl
        mov     ax              ,   dx
        and     al              ,   ah
        mov     bMsk            ,   al
        not     al
        mov     bNMsk           ,   al

        // En fonction du décalage de bit entre source et destination
        cmp     bh              , bl
        jl      DEST_IS_SMALLER
        jg      DEST_IS_GREATER

// DEST_IS_EQUAL:
        // L'OFFSET DE BIT EST LE MÊME POUR LA SOURCE ET LA DESTINATION
        
        cmp     edi             ,   dwLst
        jne     EQUAL_MULTI_BYTES

// EQUAL_ONE_BYTE:
        // La copie tient intégralement dans le premier mot
        mov     al              ,   ds:[esi]
        mov     ah              ,   ds:[edi]
        and     al              ,   bMsk
        and     ah              ,   bNMsk
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        jmp     END_OF_PROCESSING

EQUAL_MULTI_BYTES:

        // La copie est au moins à cheval sur deux mots

        // Permier mot
        mov     al              ,   ds:[esi]
        mov     ah              ,   ds:[edi]
        and     al              ,   dl
        not     dl
        and     ah              ,   dl
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        // Recopier tels quels les mots intermédiaires
EQUAL_MULTI_BYTES_LOOP:
        inc     esi
        inc     edi
        cmp     edi             ,   dwLst
        je EQUAL_MULTI_BYTES_LAST

        mov     al              ,   ds:[esi]
        mov     ds:[edi]        ,   al

        jmp     EQUAL_MULTI_BYTES_LOOP

EQUAL_MULTI_BYTES_LAST:
        // Dernier mot
        mov     al              ,   ds:[esi]
        mov     ah              ,   ds:[edi]
        and     al              ,   dh
        not     dh
        and     ah              ,   dh
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        jmp END_OF_PROCESSING

DEST_IS_SMALLER:
        // L'OFFSET DE BIT DE DESTINATION EST LE PLUS PETIT
        
        mov     cl              ,   bSrcBit
        sub     cl              ,   bDstBit
        mov     ch              ,   8
        sub     ch              ,   cl

        cmp     edi             ,   dwLst
        jne     SMALLER_MULTI_BYTES

// SMALLER_ONE_BYTE:
        // La copie tient intégralement dans le premier mot 

        mov     ax              ,   ds:[esi]
        shl     al              ,   cl
        xchg    ch              ,   cl
        shr     ah              ,   cl
        or      al              ,   ah
        xchg    ch              ,   cl
        mov     ah              ,   ds:[edi]
        and     al              ,   bMsk
        and     ah              ,   bNMsk
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        jmp     END_OF_PROCESSING

SMALLER_MULTI_BYTES:

        // La copie est au moins à cheval sur deux mots

        // Permier mot
        mov     ax              ,   ds:[esi]
        shl     al              ,   cl
        xchg    ch              ,   cl
        shr     ah              ,   cl
        or      al              ,   ah
        xchg    ch              ,   cl
        mov     ah              ,   ds:[edi]
        and     al              ,   dl
        not     dl
        and     ah              ,   dl
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        // Recopier tels quels les mots intermédiaires
SMALLER_MULTI_BYTES_LOOP:
        inc     esi
        inc     edi
        cmp     edi             ,   dwLst
        je      SMALLER_MULTI_BYTES_LAST

        mov     ax              ,   ds:[esi]
        shl     al              ,   cl
        xchg    ch              ,   cl
        shr     ah              ,   cl
        xchg    ch              ,   cl
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        jmp     SMALLER_MULTI_BYTES_LOOP

SMALLER_MULTI_BYTES_LAST:
        // Dernier mot
        cmp     dh              ,   0
        je      END_OF_PROCESSING
        mov     ax              ,   ds:[esi]
        shl     al              ,   cl
        xchg    ch              ,   cl
        shr     ah              ,   cl
        xchg    ch              ,   cl
        or      al              ,   ah
        mov     ah              ,   ds:[edi]
        and     al              ,   dh
        not     dh
        and     ah              ,   dh
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        jmp END_OF_PROCESSING

DEST_IS_GREATER:
        // L'OFFSET DE BIT DE DESTINATION EST LE PLUS GRAND
        
        mov     cl              ,   bDstBit
        sub     cl              ,   bSrcBit
        mov     ch              ,   8
        sub     ch              ,   cl

        cmp     edi             ,   dwLst
        jne     GREATER_MULTI_BYTES

// GREATER_ONE_BYTE:
        // La copie tient intégralement dans le premier mot 

        mov     al              ,   ds:[esi]
        shr     al              ,   cl
        mov     ah              ,   ds:[edi]
        and     al              ,   bMsk
        and     ah              ,   bNMsk
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        jmp     END_OF_PROCESSING

GREATER_MULTI_BYTES:

        // La copie est au moins à cheval sur deux mots

        // Permier mot
        mov     al              ,   ds:[esi]
        shr     al              ,   cl
        mov     ah              ,   ds:[edi]
        and     al              ,   dl
        not     dl
        and     ah              ,   dl
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        // Recopier tels quels les mots intermédiaires
GREATER_MULTI_BYTES_LOOP:
        inc     esi
        inc     edi
        cmp     edi             ,   dwLst
        je      GREATER_MULTI_BYTES_LAST

        mov     ax              ,   ds:[esi-1]
        xchg    ch              ,   cl
        shl     al              ,   cl
        xchg    ch              ,   cl
        shr     ah              ,   cl
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        jmp     GREATER_MULTI_BYTES_LOOP

GREATER_MULTI_BYTES_LAST:
        // Dernier mot
        cmp     dh              ,   0
        je      END_OF_PROCESSING
        mov     ax              ,   ds:[esi]
        xchg    ch              ,   cl
        shl     al              ,   cl
        xchg    ch              ,   cl
        shr     ah              ,   cl
        or      al              ,   ah
        mov     ah              ,   ds:[edi]
        and     al              ,   dh
        not     dh
        and     ah              ,   dh
        or      al              ,   ah
        mov     ds:[edi]        ,   al

        jmp END_OF_PROCESSING

END_OF_PROCESSING:
    }
}


#endif

