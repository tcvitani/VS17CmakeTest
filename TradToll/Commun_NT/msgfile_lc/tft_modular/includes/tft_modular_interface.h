/***************** (v) 2012 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 																 */
/* FILE:     tft_modular_inteface.h											 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*																			 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef TFT_MODULAR_INTERFACE_H
#define TFT_MODULAR_INTERFACE_H

/*--------------------------- INCLUDES:  --------------------------*/


/*--------------------------- RESERVED:  --------------------------*/

#ifdef TFT_MODULAR_INTERFACE_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*--------------------------- EXTERNALS: --------------------------*/


/*--------------------------- DEFINES:   --------------------------*/

#define MAX_TFT_SIZE	4096

/*--------------------------- TYPEDEFS:  --------------------------*/

typedef enum
{
	M_TFT_STATUS,
	M_TFT_FILE,
	M_TFT_LOCK,
	M_TFT_DATETIME,

	BUTEE_TFT_SERVICE,
	DERNIER_TFT_SERVICE = BUTEE_TFT_SERVICE - 1,
	NB_TFT_SERVICE = BUTEE_TFT_SERVICE

}
enum_tft_services;

typedef enum
{
	TFT_DEBUT,
	TFT_DEBUT_ACK,
	TFT_DEBUT_NACK,

	TFT_FIN,
	TFT_FIN_ACK,
	TFT_FIN_NACK,

	TFT_DEMANDE,
	TFT_DEMANDE_NACK,

	TFT_NEW_FILE,

	TFT_LOCK_REQUEST,
	TFT_LOCK_UNLOCK,
	TFT_LOCK_DONE,

	TFT_NEW_STATUS,

	TFT_DATETIME_CHANGE

}
enum_tft_type_services;

// messages exchanged through mailbox
typedef struct
{
	struct_neutre			neutre;
	enum_tft_services		service;
	enum_tft_type_services	type;
}
struct_tft_entete;

typedef struct
{
	char	Files[MAX_TFT_SIZE];
	char	ActiveTft[MAX_TFT_SIZE];
}
struct_tft_status;

typedef struct
{
	char NewFiles[MAX_TFT_SIZE];
}
struct_tft_newfile;

typedef struct
{
	boolean	FareModification;
}
struct_tft_lock;


typedef struct
{
   struct_tft_entete entete;
   
   union
   {
	   struct_tft_status	status;
	   struct_tft_newfile	new_file;
	   struct_tft_lock		lock;
   } 
   u;
} 
struct_tft_message;

/*--------------------------- FUNCTIONS: --------------------------*/


/*--------------------------- VARIABLES: --------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* TFT_MODULAR_INTERFACE_H */
