#ifndef TLM_DMV_GLOBAL_H
#define TLM_DMV_GLOBAL_H

/*--------------- INCLUDES: ---------------*/
#include <csrlc32.h>
#include <run.h>
#include <csr_srv.h>
#include <protect.h>
#include <module.h>
#include <csr_tlm.h>
#include "MQEventLoop.h"
#include "mbx2slot.h"
/*--------------- RESERVED: ---------------*/


/*----------------DEFINES:-----------------*/

#define TLM_REG_KEYv_RESPONSE_TO		"ResponseTimeoutMs"
#define TLM_REG_KEYv_STATUS_RESPONSE_TO	"StatusResponseTimeoutMs"
#define TLM_REG_KEYv_MAX_LINK_ERROR		"MaxLinkError"
#define TLM_REG_KEYv_DEVICE_TYPE		"DeviceType"
#define TLM_REG_KEYv_DEVICE_ADDRESS     "DeviceIpAddress"
#define TLM_REG_KEYv_DEVICE_PORT	    "DeviceIpPort"
#define TLM_REG_KEYv_DEVICE_ECHO_PORT	"DeviceEchoPort"
#define TLM_REG_KEYv_LOCAL_ADDRESS		"LocalIpAddress"
#define TLM_REG_KEYv_LOCAL_PORT			"LocalIpPort"
#define TLM_REG_KEYv_LOCAL_ECHO_PORT	"LocalEchoPort"
#define TLM_REG_KEYv_IMAGE_X_COORDINATE "ImageXCoordinate"
#define TLM_REG_KEYv_IMAGE_Y_COORDINATE "ImageYCoordinate"
#define TLM_REG_KEYv_TRACE_MAX_SIZE_MB  "TraceMaxSize_MB"

#define TLM_REG_SLEEP_MS				"SleepMs"

#define TLM_NB_TACHE					2
#define TLM_INSTANCE_MAX				3
#define NB_TLM_SERVICE					5
/*--------------- TYPEDEFS ENUM: ---------------*/

enum index_traces
{
	TLM_TRC,
	TLM_NB_TRACES
};

typedef short int tlm_inst_id;

typedef struct
{
	tlm_inst_id			inst_id;
	struct_tlm_message	*p_msg;
}
struct_tlm_envoi_service;

typedef enum
{
	POOLING_CMD_NOT_SENT = 0,
	POOLING_WAIT_RESPONSE,
	POOLING_RECEIVED,
	POOLING_PAUSE
}
enum_pooling_status;

/**----- list of messages between ANI and IOS thread -------------**/
typedef enum
{
	M_PREMIER_TYPE_MESSAGE = M_SRV_USER,
	M_ETAT,
	M_ARRET_DEMANDE,
	M_ARRET_EFFECTUE,
}
enum_tlm_ani_ios;

typedef enum enum_tlm_internal_message_types
{
	TLM_MESSAGE_STOP_REQUESTED = TLM_END_MESSAGE_TYPE,
	TLM_MESSAGE_STOP_EXECUTED,
	TLM_MESSAGE_SET_REQUESTED,
	TLM_MESSAGE_GET_REQUESTED,
	TLM_MESSAGE_STATE_RECEIVED_ACQ,
	TLM_MESSAGE_STATE_RECEIVED_NACQ,
	TLM_MESSAGE_ETAT_CHANGED,
} tlm_internal_message_type;

/**********************************************/
/*  structure globale						  */
/**********************************************/
typedef struct _struct_global
{
	char                        pcKey[MAX_PATH];

	mbx2slot_wrapper			sMbx2Slot;
	TWorkerStructure			sInitStructure;

	noyau_bal_id				ulArretBalId;
	noyau_priorite_tache		priorite_max;

	noyau_bal_id				ani_bal;
	noyau_bal_id				ios_bal;
	char						nom_bal_ani[MAX_PATH + 1];
	char						nom_bal_ios[MAX_PATH + 1];

	noyau_pool_id				pool;
	HANDLE						service[M_SRV_USER + NB_TLM_SERVICE];
	dbg_struct_debug			dbg;
	dbg_struct_trace			trace;
	struct_tache				taches[TLM_NB_TACHE + 1];

	struct_tlm_status			status;

	CRITICAL_SECTION			criticalSectionInstance;
}
struct_global;


/*--------------- VARIABLES: ---------------*/
// Begin of shared data section (data must be initialised)
#include "global.h"

/* Array of structures (one public structure for each instance) */
PROTECTED struct_global gsTLM[TLM_INSTANCE_MAX] INIT(0);

/* Instancies counter */
PROTECTED WORD TLM_NB_INSTANCES INIT(0);

// End of shared data section
#include "global.h"

/*------------------FUNCTIONS:-----------------*/
PROTECTED void DebutRegionModule(IN tlm_inst_id inst_id);
PROTECTED void FinRegionModule(IN tlm_inst_id inst_id);
/*------------------------------------------*/
#include <undef.h>
#endif
