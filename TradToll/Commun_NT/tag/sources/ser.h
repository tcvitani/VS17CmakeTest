/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : SERCom
 * FICHIER    : SERCom.h
 * LANGAGE    : C
 * --------------------------------------------------------------------
 * MOT-CLE    : 
 * --------------------------------------------------------------------
 * RESUME     : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORIQUE:
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef SER_H
#define SER_H

#pragma pack( push, SER_PACK, 1 )

typedef enum
{
    SER_COM,
    SER_UDP,
}
    SER_TYPE;


typedef struct _SER_INSTANCE 
{
    SER_TYPE eType;
}
    SER_INSTANCE;


#pragma pack( pop, SER_PACK )


#endif