#include <windows.h>
#include <run.h>
#include <noyau.h>
#include <module.h>
#define TLM_PROTOTYPES
#include <csr_tlm.h>
#undef TLM_PROTOTYPES

#include "protect.h"

#ifndef TEST_H
#define TEST_H

/*--------------- RESERVED: ---------------*/
/*--------------- EXTERNALS: --------------*/
/*--------------- DEFINES: ----------------*/
#ifdef _WIN64
#define	FNC_LANCE "MODLance"
#define	FNC_ARRET "MODArret"
#else
#define	FNC_LANCE "_MODLance@12"
#define	FNC_ARRET "_MODArret@4"	
#endif
/*--------------- TYPEDEFS: ---------------*/

typedef enum_instance_result(WINAPI* TLM_LANCE)(IN char * pcKey, IN char * pcBalName, OUT noyau_bal_id * piBalId);
typedef enum_instance_result(WINAPI* TLM_ARRET)(IN noyau_bal_id iBalId);


typedef struct
{
	HANDLE				hInst;
	noyau_bal_id		module_bal;
	noyau_bal_id		test_bal;
	char				szBalName[ MAX_PATH + 1 ];
	struct_tlm_message	message;
	int					dop_nature;
	int					dop_sens;
	HWND				hDlg;
	boolean				fin;
	UCHAR				szDllName[50];
	HINSTANCE			hDLL;
	TLM_LANCE			LpFncLance;
	TLM_ARRET			LpFncArret;
	short int			inst_id;
	BOOL				bCyclicStarted;
}struct_simu;

/*--------------- FUNCTIONS: --------------*/
PROTECTED int LanceModule(HWND hDlg);
PROTECTED int ArretModule(HWND hDlg);
PROTECTED void Envoi(short int service, short int type_message, struct_tlm_message *p_message);
PROTECTED void AddLBItem(HWND hDlg, char *item);

LRESULT CALLBACK StartEspion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SetImageRequest(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CyclicSetImageRequest(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/*--------------- VARIABLES: ---------------*/
PROTECTED struct_simu SIMU;
#endif