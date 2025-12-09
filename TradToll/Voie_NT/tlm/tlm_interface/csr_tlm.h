//****************** (c) 2024 emovis - All rights reserved
// MODULE:		TLM - Traffick light management
// FILE:		csr_tlm.h
// DESCRIPTION:	-

#ifndef TLM_H
#define TLM_H

//-------------------------------- INCLUDES
//-------------------------------- RESERVED
#ifdef CSR_TLM_EXPORTS
	#include "public.h"
#else
	#include "export.h"
#endif

//-------------------------------- EXTERNALS
//-------------------------------- DEFINES
#define MOD_TLM_REGKEY		"TLM\\"


//-------------------------------- TYPEDEFS
typedef enum 
{
	SRV_TYP_TLM_STATE_RECEIVED	= SRV_TYP_USER,

	TLM_END_MESSAGE_TYPE,
	TLM_LAST_MESSAGE_TYPE		= TLM_END_MESSAGE_TYPE - 1,
	NB_TLM_MESSAGE_TYPE			= TLM_END_MESSAGE_TYPE - SRV_TYP_USER
} enum_tlm_type;

typedef enum 
{
	M_TLM_MESSENGER_SERVICE	= M_SRV_USER + 10, // The +10 is because this interface is used in the same module as AFF interface
	M_TLM_FLASHER_SERVICE,

	END_TLM_SERVICE,
	LAST_TLM_SERVICE		= END_TLM_SERVICE - 1,
	NB_TLM_SERVICE			= END_TLM_SERVICE - M_SRV_USER
} enum_tlm_service;

typedef enum
{
	TLM_FLASHER_ON,
	TLM_FLASHER_OFF,
} enum_flasher_state;

typedef struct
{
	DWORD		lane_id;
	DWORD		state;
	CHAR		type[MAX_PATH+1];
} struct_tlm_service;

typedef struct
{
	enum_flasher_state	state;
} struct_tlm_flasher;

typedef struct
{
	UINT	lane_id;
} struct_tlm_start;

typedef struct
{
	union
	{
		struct_tlm_start	display_debut;
		struct_tlm_service	display_srv;
		struct_tlm_flasher  flasher_srv;
	} u;
} struct_tlm_display;

typedef struct
{
	unsigned int link_failure		: 1;
	unsigned int state_error		: 1;
	unsigned int reserved			: 6;
} struct_tlm_status;

typedef struct
{
	struct_srv_entete	entete;
	union
	{
		struct_srv_arret	srv_arret;
		struct_srv_etat		srv_etat;
		struct_tlm_display	srv_tlm;
		struct_srv_espion   espion;		

	} srv;
} struct_tlm_message;

//-------------------------------- FUNCTIONS
EXPORT enum_instance_result WINAPI TLMLance(char * pcKey, 
											char * pcBalNam, 
											noyau_bal_id * piBalId);

EXPORT enum_instance_result WINAPI TLMArret(
				noyau_bal_id	mailbox_id
			);

//-------------------------------- VARIABLES

#include <undef.h>
#endif // #ifndef TLM_H
