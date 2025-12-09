#ifndef DMV_GLOBAL_H
#define DMV_GLOBAL_H

#include <csrlc32.h>
#include <run.h>
#include <csr_srv.h>
#include <debug.h>
#include <fic_gere.h>
#include <aff_ext.h>
#include <aff_util.h>
#include <csr_tlm.h>

#include <protect.h>
#include "MQEventLoop.h"
#include "mbx2slot.h"

#define AFF_NB_THREAD  3
#define NB_AFF_MAX     3
#define NB_MAX_LABEL   64

#define NB_THREADS	   2
#define INSTANCE_MAX   999
#define NB_SERVICE	   2

#define AFF_TIMEOUT_INFINITE    NOYAU_ATTENTE_INFINIE
#define AFF_TIMEOUT_FAST        180  // 10s
#define AFF_TIMEOUT_SLOW        540  // 30s

#define AFF_REG_KEYv_DEVICE_ADDRESS     "DeviceIpAddress"
#define AFF_REG_KEYv_DEVICE_PORT	    "DeviceIpPort"
#define AFF_REG_KEYv_DEVICE_ECHO_PORT	"DeviceEchoPort"
#define AFF_REG_KEYv_LOCAL_ADDRESS		"LocalIpAddress"
#define AFF_REG_KEYv_LOCAL_PORT			"LocalIpPort"
#define AFF_REG_KEYv_LOCAL_ECHO_PORT	"LocalEchoPort"
#define AFF_REG_KEYv_FORMAT_FILE		"FormatFile"
#define AFF_REG_KEYv_UNIT_ADDRESS		"Unit_address"
#define AFF_REG_KEYv_CHARSET_CONVERSION "CharsetConversion"
#define AFF_REG_KEYv_DELAY_BETWEEN_CMD  "DelayCommandMs"
#define AFF_REG_KEYv_DEVICE_TYPE		"DeviceType"
#define AFF_REG_KEYv_FONT				"Font"
#define AFF_REG_KEYv_FONT_COLOR			"FontColor"
#define AFF_REG_KEYv_ALIGNMENT			"Alignment"
#define AFF_REG_KEYv_MAX_SPACING		"MaxSpacing"
#define AFF_REG_KEYv_BRIGHTNESS			"Brightness"
#define AFF_REG_KEYv_TEXT_XL_COORDINATE	"TextXLCoodrdinate"
#define AFF_REG_KEYv_TEXT_XR_COORDINATE	"TextXRCoodrdinate"
#define AFF_REG_KEYv_TEXT_Y_COORDINATE	"TextYCoodrdinate"
#define AFF_REG_KEYv_IMAGE_X_COORDINATE	"ImageXCoodrdinate"
#define AFF_REG_KEYv_IMAGE_Y_COORDINATE	"ImageYCoodrdinate"
#define AFF_REG_KEYv_TRACE_MAX_SIZE_MB  "TraceMaxSize_MB"
#define AFF_REG_KEYv_RESPONSE_TIMEOUT	"WaitForResponseTimeout"
#define AFF_REG_KEYv_STATUS_RESPONSE_TO	"StatusResponseTimeoutMs"
#define AFF_REG_KEYv_IS_TLM_INTEGRATED	"IsTlmServiceIntegrated"

typedef short int dmv_inst_id;

enum index_traces
{
	TRC,
	NB_TRACES
};

typedef enum
{
	AFF_NONE = -1, 
	AFF_NON_VALID = 0,
	AFF_VALID,

	AFF_FREE = 0,
	AFF_NOT_FREE = 1

}enum_aff_validity;

typedef enum enum_tlm_internal_message_types
{
	TLM_MESSAGE_STOP_REQUESTED = TLM_END_MESSAGE_TYPE,
	TLM_MESSAGE_STOP_EXECUTED,
	TLM_MESSAGE_SET_REQUESTED,
	TLM_MESSAGE_FLASHER_SET_REQUESTED,
	TLM_MESSAGE_GET_REQUESTED,
	TLM_MESSAGE_FLASHER_GET_REQUESTED,
	TLM_MESSAGE_STATE_RECEIVED_ACQ,
	TLM_MESSAGE_STATE_RECEIVED_NACQ,
	TLM_MESSAGE_ETAT_CHANGED,
} tlm_internal_message_type;

typedef enum
{
   M_ARRET = 0,

   /* Messages to AFF_IOS */
   M_ETAT = 1,
   M_AFFICHAGE,
   M_ALLUME,

   /* Messages to AFF_ANI */
   M_CHANGEMENT_ETAT = 1,
   M_ACK_AFFICHAGE,
   M_ERR_AFFICHAGE,

   M_CHANGEMENT_VISU,

   M_RESET_HW,
   M_VOYANT,
   M_ARRET_EFFECTUE,
   M_DEVICE_INFO_COMPLETE,	 // Device info successfully get from device

   M_TLM_MESSAGE_SET = M_SRV_USER,
   M_TLM_MESSAGE_GET,
   M_TLM_FLASHER_GET,
   M_TLM_FLASHER_SET
}
enum_ani_ios;

typedef struct
{
	short modif_priorite;
}struct_aff_item;

typedef struct
{
	union
	{
		struct_aff_item		contenu;
		struct_aff_allume	allume;
		struct_aff_voyant	voyant;
		struct_tlm_service	tlm;
		struct_tlm_flasher  flasher;
	}u;
} struct_ani_to_ios;

typedef struct
{
	char current_view[AFF_MAX_LIGNES][AFF_MAX_DONNEES_AFFICHAGE];
	struct_b_etat_aff sStatus;
} struct_ios_to_ani;

typedef struct
{
	struct_neutre		neutre;
	enum_ani_ios		message_id;

	union
	{
		struct_ani_to_ios  ani_ios;
		struct_ios_to_ani  ios_ani;
	}u;
}struct_ani_ios;

typedef struct
{
	dmv_inst_id			inst_id;
	struct_aff_message  *p_msg;
}struct_aff_send_service;

typedef struct
{
	dmv_inst_id			inst_id;
	struct_tlm_message	*p_msg;
}struct_tlm_send_service;

typedef struct
{
	struct_srv_entete entete;
	union
	{
		struct_srv_arret				srv_arret;
		struct_srv_etat					srv_etat;
		struct_srv_espion				srv_espion;
	} u;
} struct_message_generic;

typedef struct _struct_global
{
	char                        pcKey[MAX_PATH];

	boolean						arret_demande;
	noyau_priorite_tache		priority_max;
	noyau_bal_id				ani_mbox;
	noyau_bal_id				ios_mbox;
	noyau_bal_id				stop_mbox;
	char						ani_mbox_name[MAX_PATH + 1];
	char						ios_mbox_name[MAX_PATH + 1];
	noyau_delai					timeout_mbox;
	noyau_pool_id				pool;
	DWORD						polling_timeout;

	BOOL						stop_ios_request;

	mbx2slot_wrapper			sMbx2Slot;
	TWorkerStructure			sInitStructure;
	DWORD						dwMboxReadInterval; //Check this

	HANDLE						service[NB_AFF_SERVICE];
	dbg_struct_debug			sDbg;
	dbg_struct_trace			sTrace[NB_AFF_SERVICE];

	struct_tache				threads[AFF_NB_THREAD + 1];

	struct_b_etat_aff			sStatus;

	DWORD						MaxLinkError;

	struct_label                LabelTable[NB_MAX_LABEL];
	char                        szCurrentLineTable[AFF_MAX_LIGNES][AFF_MAX_DONNEES_AFFICHAGE];
	struct_screen				sScreen;
	unsigned char				char_conv_array[256];
	unsigned char				inv_char_conv_array[256];
	long						position;

	unsigned long				tab_credit[NB_MAX_AFF_PRIO];
	unsigned long				tab_paid[NB_MAX_AFF_PRIO];
}
struct_global;


// Begin of shared data section (data must be initialised)
#include <global.h>

/* Array of structures (one public structure for each instance) */
PROTECTED struct_global gsDMV[NB_AFF_MAX] INIT(0);

/* Instancies counter */
PROTECTED WORD NB_INSTANCES INIT(0);

// End of shared data section
#include "global.h"

/*------------------FUNCTIONS:-----------------*/
/*------------------------------------------*/
#include <undef.h>


#endif
