/********************************************
*  PROGRAM: waff.h
*  PURPOSE: definitions and global variables
*           of wafftest program
/********************************************

/*--------------- INCLUDES: ---------------*/

#include <csrlc32.h>
#include <run.h>
#include <noyau.h>


#include <aff_ext.h>

#include <protect.h>

/*--------------- RESERVED: --------------*/

/*--------------- EXTERNALS: -------------*/

/*--------------- DEFINES: ---------------*/

#ifdef _WIN64
	#define	FNC_AFF_LANCE "AFFLance"
	#define	FNC_AFF_ARRET "AFFArret"
#else
	#define	FNC_AFF_LANCE "_AFFLance@12"
	#define	FNC_AFF_ARRET "_AFFArret@4"
#endif

#define NB_TEST_LABELS	5


/*--------------- TYPEDEFS: --------------*/

typedef enum_instance_result (WINAPI* LPFNC_LANCE)( IN char * pcKey, IN char * pcBalName, OUT noyau_bal_id * piBalId );
typedef enum_instance_result (WINAPI* LPFNC_ARRET)( IN noyau_bal_id iBalId );

typedef struct
{
	noyau_bal_id bal_id;
	char bal_name[6];
}simu_multi_user;


typedef struct
{
	noyau_bal_id			bal_id;
	simu_multi_user			user[4];
	noyau_bal_id			aff_bal;
	struct_aff_message		message;
	int						espion_nature;
	int						espion_sens;
	HWND					hDlg;
	boolean					fin;
	HINSTANCE				hDLL;             // Handle to DLL
	LPFNC_LANCE				LpFncAFFLance;    // Function pointer
	LPFNC_ARRET				LpFncAFFArret;    // Function pointer
	char                    szTmp[MAX_PATH];
	BOOL					bPerfomTestStarted;
	UCHAR					aszTestLabels[NB_TEST_LABELS][AFF_MAX_LABEL];
}struct_simu;

/*--------------- FUNCTIONS: ---------------*/

PROTECTED void AddLBItem(HWND hDlg, char *item);

/*--------------- VARIABLES: ---------------*/

PROTECTED struct_simu SIMU;
PROTECTED char	dll_name[50];
