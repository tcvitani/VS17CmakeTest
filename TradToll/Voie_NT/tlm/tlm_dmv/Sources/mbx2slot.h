#ifndef MBX_TO_SLOT_H
#define MBX_TO_SLOT_H


//
// The header file PROTECT.H will behave differently if the macro
// LOC_DEF is defined or not. In a project, this file should only
// be included once with LOC_DEF defined.
//
#include <protect.h>


typedef struct _mbx2queue_wrapper
{
	char                szBalName[MAX_PATH];
	noyau_bal_id        hBal;
	void*				pQObject;
	struct_tache        tsTasks[2];
	HANDLE              hEvtTerminateMbx2Slot;
} mbx2slot_wrapper;


void * WINAPI PublieBALToQueue(char *nom_bal, unsigned long nb_messages_en_attente_max, mbx2slot_wrapper * psMbx2queue);
noyau_enum_retour WINAPI SupprimeBALToQueue(mbx2slot_wrapper * psMbx2queue);

#endif