/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : DES
 * FILE       : DES.H
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

//
// The macro DES_H is defined to avoid multiple file inclusion
//
#ifndef DES_H
#define DES_H


#ifdef DES_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif





//
// EXPORTED DATA TYPES
//

typedef void * HDESKEY;
typedef void * HDESKEYTRIPLE;




//
// EXPORTED FUNCTIONS PROTOTYPES
//

EXPORT HDESKEY WINAPI DESOpenKey( 
        IN      BYTE        tbKeyBytes[8],
        IN      BOOL        bVerifyKey );

EXPORT HDESKEY WINAPI DESOpenTripleKey( 
        IN      BYTE        ttbKeyBytes[3][8],
        IN      BOOL        bVerifyKey );

EXPORT void WINAPI DESCloseKey( 
        IN      HDESKEY     hDesKey );

EXPORT BOOL WINAPI DESEncryptEcb( 
        IN      BOOL        bTriple,
        IN      BOOL        bEncrypt,
        IN      BYTE      * pbSrcBlock, 
        IN      DWORD       dwSrcBlockBytes,
        OUT     BYTE      * pbDstBlock,
        IN OUT  DWORD     * pdwDstBlockBytes,
        IN      HDESKEY     hDesKey );

EXPORT BOOL WINAPI DESEncryptCbc( 
        IN      BOOL        bTriple,
        IN      BOOL        bEncrypt,
        IN      BYTE      * pbSrcBlock, 
        IN      DWORD       dwSrcBlockBytes,
        OUT     BYTE      * pbDstBlock,
        IN OUT  DWORD     * pdwDstBlockBytes,
        IN      HDESKEY     hDesKey );

//
// In reference to DES_H
//
#endif
