/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : BITBL
 * FILE       : BITBL.H
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
// The macro BITBL_H is defined to avoid multiple file inclusion
//
#ifndef BITBL_H
#define BITBL_H


//
// The macro BITBL_EXPORTS is defined in the DLL project and
// should not be defined in any other project using the DLL.
//
#ifdef BITBL_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif



//
// EXPORTED MACROS
//

#define BITBL_IS_VALID(x)                   ((x)<0x80000000)
#define BITBL_INVALID                       0xFFFFFFFF




//
// EXPORTED FUNCTIONS PROTOTYPES
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
        IN      DWORD       dwLen );




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
        IN      DWORD       dwSrcBitLen );




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
        IN OUT  DWORD     * pdwSrcBitLen );




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
        IN OUT  DWORD     * pdwSrcBitLen );




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
        IN OUT  DWORD     * pdwDstBitLen );




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
        IN      WORD        wVal );




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
        IN      DWORD       dwVal );




//
// In reference to BITBL_H
//
#endif
