/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL CONVERT
* FICHIER: CONVERT.H
* LANGAGE: C
* --------------------------------------------------------------------
* HISTORIQUE: 
 * $Log:   T:/MODULO/VoieNt/Outils_C/Convert/Includes/CONVERT.H_v  $
 * 
 *    Rev 1.3   Aug 22 2000 11:32:10   bph
 *  
 * 
 *    Rev 1.2   Jan 20 1999 14:12:20   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:06:44   bph
 *  
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef CONVERT_H
#define CONVERT_H

/*--------------- INCLUDES: ---------------*/
#include "memclass.h"


/*--------------- RESERVED: ---------------*/

#ifdef CONVERT_DEF
   #include<public.h>
#else
   #include<export.h>
#endif

#include <windows.h>

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

/* Constantes utilisees pour les fonctions de conversions */

#define SRV_LEFT    0
#define SRV_RIGHT   1

#define SRV_BASE10  0
#define SRV_BASE16  1


/*--------------- TYPEDEFS: ---------------*/


/*--------------- FUNCTIONS: ---------------*/

EXPORT void WINAPI ConversionExpanse (unsigned char *  source ,
                                      unsigned char *  dest ,
                                      unsigned char  lg_dst ,
                                      unsigned char  alignment );

EXPORT void WINAPI ConversionExpanseEnBcd (unsigned char *   source ,
                                           unsigned char *   dest ,
                                           unsigned char  nb_bytes ,
                                           unsigned char  alignment );

EXPORT void WINAPI ConversionHexaEnBcdExpanse (unsigned long  hex_value ,
                                               unsigned char * p_expansed_array,
                                               unsigned char expansed_length);

EXPORT unsigned long WINAPI ConversionBcdEnHexa (unsigned char *  source , 
                                                 unsigned char  nb_bytes );

EXPORT void WINAPI ConversionHexaEnBcd (unsigned long  hexa_value , 
                                        unsigned char *  destination ,
                                        unsigned char  dest_length );

EXPORT void WINAPI ConversionBcdEnAscii (unsigned char *  source ,
                                         unsigned char *  destination ,
                                         unsigned short int  nb_bytes );

EXPORT void WINAPI ConversionAsciiEnBcdExpanse (unsigned char *str_ascii,
                                                unsigned char *str_bcd,
                                                unsigned short int bytes_ascii);

EXPORT long WINAPI ConversionAsciiEnHexa (unsigned char *  source ,
                                          unsigned short int  nb_bytes ,
                                          unsigned char  base );


/*--------------- VARIABLES: ---------------*/

#undef PUBLIC
#undef I
#undef INIT
#endif
