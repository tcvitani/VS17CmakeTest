/***************** (v) 2012 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_UNIDAQ														 */
/* FILE:	 dio_unidaq.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef DIO_UNIDAQ_GLOB_H
#define DIO_UNIDAQ_GLOB_H

/*--------------- INCLUDES: ---------------*/

#include "protect.h"

/*--------------- RESERVED: ---------------*/


/*--------------- EXTERNALS: ---------------*/


/*--------------- DEFINES: ---------------*/




/*--------------- TYPEDEFS: ---------------*/
typedef struct _struct_global_outputs
{
 	BYTE	abOutputStatus[DIO_NB_OUTPUTS_MAX];

}
struct_global_outputs;


typedef struct _struct_dio_global
{
    struct_global_outputs	sBoardDetails[DIO_NB_BOARDS_MAX];
}
struct_dio_global;

/*--------------- FUNCTIONS: ---------------*/

#include "global.h"

   PROTECTED struct_dio_global DIO_GLOB INIT(0);

   PROTECTED WORD UNIDAQ_NB_INSTANCES INIT(0);

// fin de section de données partagées
#include "global.h"




#endif
