/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BUFFER
* FICHIER: BUFFER.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: D‚finition et traitement d'un buffer
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Outils_C/Fonct_c/Includes/buffer.h_v  $
 * 
 *    Rev 1.0   11 May 1999 17:31:12   afx
 *  
 * 
 *    Rev 1.4   Jul 28 1997 17:17:48   DPI
 * Gestion de la version de la librairie
 * 
 *    Rev 1.3   Apr 25 1997 16:11:22   PDU
 * La taille d'un buffer est maintenant un int.
 * Dans la version precedente, c'etait un long.
 * 
 *    Rev 1.2   Apr 11 1997 10:47:16   DPI
 * Pb avec l'historique 
 *
 *    Rev 1.1   Apr 11 1997 10:14:24   DPI
 * Evolution apres creation
 *
 *    Rev 1.0   Apr 11 1997 10:14:24   DPI
 * Creation
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef BUFFER_H
#define BUFFER_H

/*--------------- RESERVED: ---------------*/
#ifdef MOD_BUFFER
#include <public.h>
#else
#include <export.h>
#endif

#include <windows.h>

/*--------------- VERSION: --------------*/

/*--------------- DEFINES: ----------------*/

#define TAILLE_MAX_BUFFER 255

/*--------------- FUNCTIONS: --------------*/

// Definition d'un buffer :
//typedef struct struct_buffer TBuffer;

typedef struct struct_buffer
{
    char *contenu; // le contenu d'un buffer
    int  taille;  // le nombre de caracteres significatifs du buffer
} TBuffer;


EXPORT void WINAPI InitBuffer( TBuffer *buff );
EXPORT void WINAPI InitialiserBuffer( TBuffer *buff,
                               char    *contenu,
                               int      taille);
EXPORT void WINAPI AfficherBuffer( TBuffer *buff);

EXPORT void WINAPI AffecterBuffer( TBuffer *dest,
                            TBuffer *source);
EXPORT boolean WINAPI CopierBuffer( TBuffer *dest,
                             TBuffer *source,
                             int      posDest,
                             int      tailleMaxDest );

EXPORT void WINAPI DecalerBuffer( TBuffer *buff,
                           int      offset);

EXPORT boolean WINAPI ChangerTailleBuffer( TBuffer *buff,
                                    int      nvTaille,
                                    int      tailleMaxBuff);

EXPORT long WINAPI LireEntierDansBuffer( TBuffer *buff,
                                  int      tailleEntier );
EXPORT boolean WINAPI EcrireEntierDansBuffer( TBuffer *buff,
                                       int      tailleEntier,
                                       long     entier,
                                       int      positionEcriture,
                                       int      tailleMaxBuff );

/*--------------- VARIABLES: --------------*/

#endif
