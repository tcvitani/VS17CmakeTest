/***************** (v) 2016 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 DIO_ioLogik1200, DIO_ioLogikE4200								 */
/* FILE:	 dio_Mutex.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*		This file contains functions for handling with Windows WINAPI mutex. */
/*		It is used to resolve problem with Windows freezing when several	 */
/*		instances of dlls that handles MOXA ioLogik DIO cards are started.	 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef DIO_MUTEX_H
#define DIO_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------- INCLUDES:  -------------------------------*/

#ifdef DIO_MUTEX_DEF
 	#include <public.h>
#else
 	#include <export.h>
#endif

#include <protect.h>
#include <memclass.h>

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

typedef struct 
{
    // infos de debug
    void *debug;
    void *traces;
} 
struct_dio_mutex_debug;

typedef enum
{
	DIO_MUTEX_NOK = 0,
	DIO_MUTEX_OK	
}
enum_dio_mutex;

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT enum_dio_mutex WINAPI DioMutexStartRegion();
EXPORT enum_dio_mutex WINAPI DioMutexEndRegion();

/*-------------------------------- VARIABLES: -------------------------------*/

#endif
/*-------------------------------- END OF FILE ------------------------------*/