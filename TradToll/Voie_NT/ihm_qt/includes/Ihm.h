/* --------------------------------------------------------------------
 * --------------------------------------------------------------------
 * MODULE     : Ihm qt
 * FILE       : Ihm.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
  *
 * --------------------------------------------------------------------
 */
#ifndef IHM_H
#define IHM_H

/*--------------- INCLUDES: ---------------*/

#include <WINDOWS.H>

#include <CSRLC32.H>
#include <run.h>
#include <module.h>   

#ifdef IHM_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*---------------------------- MACRO -------------------------*/

// LONGUEURS DE CHAINES

// Longueurs des chaines cournantes
#define IHM_LG_CHAINE           256

// Longueurs des chaines longues
#define IHM_LG_LIGNE_MAX        256   // 255 + car. de fin

// Longueurs des chaines courtes
#define IHM_SH_LIGNE_MAX        41   // 40 car. + car. de fin

// longueur des chaines qui transitent dans les messages de service IN
#define IHM_LG_CHAINES_SRV      IHM_LG_LIGNE_MAX

#define IHM_MAX_XML_BUFFER_SIZE	50000


// longueur des chaines de donnees renvoyees a l'appli (saisies)
// Attention, doit etre un multiple de 4
#define IHM_LG_SAISIE_MAX       2048

//tmo 29/11/2000
#define STRUCT_IHM_SRV_DETECTION_LENGHT		2 + IHM_LG_CHAINES_SRV + IHM_LG_CHAINES_SRV + IHM_LG_SAISIE_MAX


// ACCES AU REGISTRE

// Cle du registre contenant les configurations des instances IHM
#define IHM_REG_KEYn_ModIHM     "IHM\\"

#define IHM_REG_KEY_BASE        CSR_REG_KEYn_CSRBASE \
                                CSR_REG_KEYn_LANE_BASE \
                                CSR_REG_KEYn_CONFIG \
                                MOD_REG_KEYn_MODULES \
                                IHM_REG_KEYn_ModIHM

// VALEURS DES PARAMETRES DES MESSAGES DE SERVICES

// Define pour valeurs par defaut
#define IHM_DONT_CARE           0

// Define pour ne pas changer des valeurs (ex service config)
#define IHM_NOCHANGE_S          "\1"        // pour une chaine
#define IHM_NOCHANGE_E          -1          // pour un enum

#define IHM_DO_NOT_CHANGE_S(x) \
		{\
		strncpy_s(x, sizeof(x), IHM_NOCHANGE_S, sizeof(x));\
		x[sizeof(x) - 1] = '\0';\
		}

#define IHM_DO_NOT_CHANGE_E(x)  (x=IHM_NOCHANGE_E);

#define IHM_NOT_EMPTY_S(x) ((x)[0]!='\0')
#define IHM_NOT_EMPTY_E(x) ((x)!=IHM_NOCHANGE_E)

#define IHM_STR_IS_DO_NOT_CHANGE(x) ((x)[0]=='\1' && (x)[1]=='\0')

#define IHM_STR_NEW_LINE "\xF5"
#define IHM_REG_VAL_DEFAULT_VIDEO_DIR		"DefaultVideoDirectory"

/*---------------------------- TYPES -------------------------*/
// Objets definis dans l'Ihm et accessibles de l'exterieur
typedef enum
{
    IHM_OBJ = 0,

	//specific for labels
	IHM_OBJ_All_Object = 1,			
 
	
    // Labels
	IHM_OBJ_PlazaName = 10,         
	IHM_OBJ_PlazaName1,         
	IHM_OBJ_PlazaName2,         
	IHM_OBJ_PlazaName3,         
	IHM_OBJ_PlazaNameTitle,
    IHM_OBJ_PlazaNbr,
	IHM_OBJ_PlazaNbrTitle,
    IHM_OBJ_LaneNbr,
	IHM_OBJ_LaneNbrTitle,
    IHM_OBJ_CollectorID,
    IHM_OBJ_CollectorName,      

	IHM_OBJ_TxtSpare1,		
	IHM_OBJ_TxtSpare2,			
	IHM_OBJ_TxtSpare3,
	IHM_OBJ_TxtSpare4,
	IHM_OBJ_TxtSpare5,
	IHM_OBJ_TxtSpare6,
	IHM_OBJ_TxtSpare7,
	IHM_OBJ_TxtSpare8,
	IHM_OBJ_TxtSpare9,
	IHM_OBJ_TxtSpare10,
	IHM_OBJ_TxtSpare11,
	IHM_OBJ_TxtSpare12,
	IHM_OBJ_TxtSpare13,
	IHM_OBJ_TxtSpare14,
	IHM_OBJ_TxtSpare15,
	IHM_OBJ_TxtSpare16,
	IHM_OBJ_TxtSpare17,
	IHM_OBJ_TxtSpare18,
	IHM_OBJ_TxtSpare19,
	IHM_OBJ_TxtSpare20,
	IHM_OBJ_TxtSpare21,		
	IHM_OBJ_TxtSpare22,			
	IHM_OBJ_TxtSpare23,
	IHM_OBJ_TxtSpare24,
	IHM_OBJ_TxtSpare25,
	IHM_OBJ_TxtSpare26,
	IHM_OBJ_TxtSpare27,
	IHM_OBJ_TxtSpare28,
	IHM_OBJ_TxtSpare29,
	IHM_OBJ_TxtSpare30,
	IHM_OBJ_TxtSpare31,		
	IHM_OBJ_TxtSpare32,			
	IHM_OBJ_TxtSpare33,
	IHM_OBJ_TxtSpare34,
	IHM_OBJ_TxtSpare35,
	IHM_OBJ_TxtSpare36,
	IHM_OBJ_TxtSpare37,
	IHM_OBJ_TxtSpare38,
	IHM_OBJ_TxtSpare39,
	IHM_OBJ_TxtSpare40,
	IHM_OBJ_TxtSpare41,		
	IHM_OBJ_TxtSpare42,			
	IHM_OBJ_TxtSpare43,
	IHM_OBJ_TxtSpare44,
	IHM_OBJ_TxtSpare45,
	IHM_OBJ_TxtSpare46,
	IHM_OBJ_TxtSpare47,
	IHM_OBJ_TxtSpare48,
	IHM_OBJ_TxtSpare49,
	IHM_OBJ_TxtSpare50,
	IHM_OBJ_TxtSpare51,		
	IHM_OBJ_TxtSpare52,			
	IHM_OBJ_TxtSpare53,
	IHM_OBJ_TxtSpare54,
	IHM_OBJ_TxtSpare55,
	IHM_OBJ_TxtSpare56,
	IHM_OBJ_TxtSpare57,
	IHM_OBJ_TxtSpare58,
	IHM_OBJ_TxtSpare59,
	IHM_OBJ_TxtSpare60,

    IHM_OBJ_TransactionNbr,		
    IHM_OBJ_TransactionNbrTitle,
    IHM_OBJ_ReceiptNbr,
	IHM_OBJ_ReceiptNbrTitle,
    IHM_OBJ_Axles,
	IHM_OBJ_AxlesTitle,
    IHM_OBJ_Balance,	
	IHM_OBJ_BalanceTitle,
    IHM_OBJ_Class,              
	IHM_OBJ_ClassTitle, 
    IHM_OBJ_AdditionalRevenue,              
	IHM_OBJ_AdditionalRevenueTitle, 

    IHM_OBJ_NbCar,				
    IHM_OBJ_Entry,
	IHM_OBJ_EntryTitle,
    IHM_OBJ_Currency,
	IHM_OBJ_CurrencyTitle,
    IHM_OBJ_Fare,
	IHM_OBJ_FareTitle,		
    IHM_OBJ_Sale,
	IHM_OBJ_SaleTitle,
    IHM_OBJ_SaleDue,			
	IHM_OBJ_SaleDueTitle,	
    IHM_OBJ_SaleDuePaid,			
    IHM_OBJ_TransactionType,
	IHM_OBJ_TransactionTypeTitle,
	IHM_OBJ_LaneName,          
	IHM_OBJ_LaneName1,          
	IHM_OBJ_LaneName2,          
	IHM_OBJ_LaneNameTitle,	
    IHM_OBJ_FareType,
	IHM_OBJ_FareTypeTitle,
	
	IHM_OBJ_TxtModeTitle,		
	IHM_OBJ_TxtMode,		
	IHM_OBJ_TxtCollectorTitle,		
	IHM_OBJ_TxtInCtrlUserTitle,		
	IHM_OBJ_TxtInCtrlUser,	
	IHM_OBJ_TxtInCtrlUser1,	
	IHM_OBJ_TxtTrsContainerTitle,		
	IHM_OBJ_TxtAlarmsTitle,		
	IHM_OBJ_TxtTrsHistoryTitle,		
	IHM_OBJ_TxtAlertesTitle,		
	IHM_OBJ_TxtCamNumPic,
	IHM_OBJ_TxtScanNumPic,
	IHM_OBJ_TxtNbCarSAS,

    // Icons
    IHM_OBJ_IcoLaneStatus = 200,
    IHM_OBJ_IcoLaneMode,
    IHM_OBJ_IcoCollector,       
    IHM_OBJ_IcoEntryGate,		
    IHM_OBJ_IcoExitGate,		
    IHM_OBJ_IcoCar,
    IHM_OBJ_IcoTrafficLight,
    IHM_OBJ_IcoViolation,       
    IHM_OBJ_IcoEntryLoop,		
    IHM_OBJ_IcoExitLoop,
	IHM_OBJ_IcoBeacon,
	IHM_OBJ_IcoEntOpticalBarrier,
	IHM_OBJ_IcoSpare1,
	IHM_OBJ_IcoSpare2,
	IHM_OBJ_IcoSpare3,
	IHM_OBJ_IcoSpare4,
	IHM_OBJ_IcoSpare5,
	IHM_OBJ_IcoSpare6,
	IHM_OBJ_IcoSpare7,
	IHM_OBJ_IcoSpare8,
	IHM_OBJ_IcoSpare9,
	IHM_OBJ_IcoSpare10,
	IHM_OBJ_IcoSpare11,
	IHM_OBJ_IcoSpare12,
	IHM_OBJ_IcoSpare13,
	IHM_OBJ_IcoSpare14,
	IHM_OBJ_IcoSpare15,
	IHM_OBJ_IcoSpare16,
	IHM_OBJ_IcoSpare17,
	IHM_OBJ_IcoSpare18,
	IHM_OBJ_IcoSpare19,
	IHM_OBJ_IcoSpare20,
	IHM_OBJ_IcoSpare21,
	IHM_OBJ_IcoSpare22,
	IHM_OBJ_IcoSpare23,
	IHM_OBJ_IcoSpare24,
	IHM_OBJ_IcoSpare25,
	IHM_OBJ_IcoSpare26,
	IHM_OBJ_IcoSpare27,
	IHM_OBJ_IcoSpare28,
	IHM_OBJ_IcoSpare29,
	IHM_OBJ_IcoSpare30,
	IHM_OBJ_IcoSpare31,
	IHM_OBJ_IcoSpare32,
	IHM_OBJ_IcoSpare33,
	IHM_OBJ_IcoSpare34,
	IHM_OBJ_IcoSpare35,
	IHM_OBJ_IcoSpare36,
	IHM_OBJ_IcoSpare37,
	IHM_OBJ_IcoSpare38,
	IHM_OBJ_IcoSpare39,
	IHM_OBJ_IcoSpare40,
	IHM_OBJ_IcoSpare41,
	IHM_OBJ_IcoSpare42,
	IHM_OBJ_IcoSpare43,
	IHM_OBJ_IcoSpare44,
	IHM_OBJ_IcoSpare45,
	IHM_OBJ_IcoSpare46,
	IHM_OBJ_IcoSpare47,
	IHM_OBJ_IcoSpare48,
	IHM_OBJ_IcoSpare49,
	IHM_OBJ_IcoSpare50,

	IHM_OBJ_IcoCamera1,
	IHM_OBJ_IcoCamera2,

	IHM_OBJ_IcoBeacon2,
	IHM_OBJ_IcoSignalLight,
	IHM_OBJ_IcoExtOpticalBarrier,


    // Instruction 
    IHM_OBJ_ErrPayment = 300,		
    IHM_OBJ_Payment,
	IHM_OBJ_Remark,
    IHM_OBJ_Instruction,   
	IHM_OBJ_Instruction2,
	IHM_OBJ_CurrentDate,	

	//lists
    IHM_OBJ_LstAlarms = 310,
    IHM_OBJ_LstWarnings,

	//dynamic image (specific icon)
	IHM_OBJ_Image = 400,	
	IHM_OBJ_Image2,	
	
	//buttons
	IHM_OBJ_btnClass	= 420,
	IHM_OBJ_btnEntryPoint,
	IHM_OBJ_btnEntryPointNbr,
	IHM_OBJ_btnMode,
	IHM_OBJ_btnArrowOn,
	IHM_OBJ_btnArrowOff,
	IHM_OBJ_btnCrossOn,
	IHM_OBJ_btnCrossOff,
	IHM_OBJ_btnDsrcOn,
	IHM_OBJ_btnDsrcOff,
	IHM_OBJ_btnMagneticOn,
	IHM_OBJ_btnMagneticOff,
	IHM_OBJ_btnCollectorOn,
	IHM_OBJ_btnCollectorOff,

	IHM_OBJ_btnCamera,
	IHM_OBJ_btnScanner,
	IHM_OBJ_btnComment,

	IHM_OBJ_btnGabarite,
	IHM_OBJ_btnEntryGate,
	IHM_OBJ_btnExitGate,
	IHM_OBJ_btnCar3,
	IHM_OBJ_btnCar2,
	IHM_OBJ_btnCar,
	IHM_OBJ_btnEntryLoop,
	IHM_OBJ_btnExitLoop,
	IHM_OBJ_btnTblBeacon,

	IHM_OBJ_btnCamGrab,
	IHM_OBJ_btnCamHide,
	IHM_OBJ_btnScanGrab,
	IHM_OBJ_btnScanHide,	
	IHM_OBJ_btnScanSend,
	IHM_OBJ_btnScanSave,
	IHM_OBJ_btnCommHide,
	IHM_OBJ_btnExternalDisplay,
	IHM_OBJ_btnExternalDisplay2,
	IHM_OBJ_btnTakeControl,
	IHM_OBJ_btnReturnControl,

	//spare buttons
	IHM_OBJ_Product1 ,
	IHM_OBJ_Product2,
	IHM_OBJ_Product3,
	IHM_OBJ_Product4,
	IHM_OBJ_Product5,	
	
	IHM_OBJ_btnSpeedLimitOn,	
	IHM_OBJ_btnSpeedLimitOff,	
	IHM_OBJ_btnWarningOn,	
	IHM_OBJ_btnWarningOff,	
	IHM_OBJ_btnDsrcTSAOn,
	IHM_OBJ_btnDsrcTSAOff,

	IHM_OBJ_btnSpare1,
	IHM_OBJ_btnSpare2,
	IHM_OBJ_btnSpare3,
	IHM_OBJ_btnSpare4,
	IHM_OBJ_btnSpare5,
	IHM_OBJ_btnSpare6,
	IHM_OBJ_btnSpare7,
	IHM_OBJ_btnSpare8,
	IHM_OBJ_btnSpare9,
	IHM_OBJ_btnSpare10,

	IHM_OBJ_btnSpare11,
	IHM_OBJ_btnSpare12,
	IHM_OBJ_btnSpare13,
	IHM_OBJ_btnSpare14,
	IHM_OBJ_btnSpare15,
	IHM_OBJ_btnSpare16,
	IHM_OBJ_btnSpare17,
	IHM_OBJ_btnSpare18,
	IHM_OBJ_btnSpare19,
	IHM_OBJ_btnSpare20,

	IHM_OBJ_btnSpare21,
	IHM_OBJ_btnSpare22,
	IHM_OBJ_btnSpare23,
	IHM_OBJ_btnSpare24,
	IHM_OBJ_btnSpare25,
	IHM_OBJ_btnSpare26,
	IHM_OBJ_btnSpare27,
	IHM_OBJ_btnSpare28,
	IHM_OBJ_btnSpare29,
	IHM_OBJ_btnSpare30,

	IHM_OBJ_TableView1 = 520,
	IHM_OBJ_TableView2,
	IHM_OBJ_TableView3,
	
	//horizontal menu
	IHM_OBJ_HMenuView1,
	IHM_OBJ_HMenuView2,

	//Video objects using restream_server 
	//These objects are also available for the web interface
	//It uses MJPEG stream to show video. Commands are sent to the restream server
	// using HTTP command requests
	IHM_OBJ_RVideoView1 = 540,
	IHM_OBJ_RVideoView2,

	//to be able to dynamicaly show hide containers
	IHM_OBJ_CNT_HeaderGroup = 600,
	IHM_OBJ_CNT_InstructionGroup,
	IHM_OBJ_CNT_ModeGroup,	
	IHM_OBJ_CNT_PaymentGroup,	
	IHM_OBJ_CNT_TrsGroup,	

	//We cannot use spare containers since these are 
	// not visible when not in control
	IHM_OBJ_CNT_Comments,
	IHM_OBJ_CNT_Camera,
	IHM_OBJ_CNT_Scanner,

	IHM_OBJ_CNT_Spare1, 	
	IHM_OBJ_CNT_Spare2, 	
	IHM_OBJ_CNT_Spare3, 	
	IHM_OBJ_CNT_Spare4, 	
	IHM_OBJ_CNT_Spare5, 	
	IHM_OBJ_CNT_Spare6,
	IHM_OBJ_CNT_Spare7,
	IHM_OBJ_CNT_Spare8,
	IHM_OBJ_CNT_Spare9,
	IHM_OBJ_CNT_Spare10,
	IHM_OBJ_CNT_Spare11,
	IHM_OBJ_CNT_Spare12,
	IHM_OBJ_CNT_Spare13,
	IHM_OBJ_CNT_Spare14,
	IHM_OBJ_CNT_Spare15,
	IHM_OBJ_CNT_Spare16,
	IHM_OBJ_CNT_Spare17,
	IHM_OBJ_CNT_Spare18,
	IHM_OBJ_CNT_Spare19,
	IHM_OBJ_CNT_Spare20,

	IHM_OBJ_CNT_MiniWeb,
	IHM_OBJ_CNT_AlarmsGroup,

	IHM_OBJ_TabControl1 = 700,
	IHM_OBJ_TabControl2,
	IHM_OBJ_TabControl3,
	IHM_OBJ_TabControl4,

	IHM_OBJ_OpenLink,
	IHM_OBJ_OpenLoginDlg, //visible only at desktop, invariant to in control status
	IHM_OBJ_AboutDlg,		//visible only at desktop, invariant to in control status 

	IHM_OBJ_LastObject
}
enum_ihm_objets;

typedef enum
{
	IHM_VIDEO_CMD_UNKNOWN=0,
	IHM_VIDEO_ON=1,
	IHM_VIDEO_OFF,
	IHM_VIDEO_FREEZE,
	IHM_VIDEO_UNFREEZE,
	IHM_VIDEO_ZOOM_IN,
	IHM_VIDEO_ZOOM_OUT,
	IHM_VIDEO_ZOOM,
	IHM_VIDEO_SELECT_SRC,
	IHM_VIDEO_SAVE_BMP,
	IHM_VIDEO_SAVE_JPG,

	IHM_VIDEO_SAVE_OK,
	IHM_VIDEO_SAVE_NOK,
	IHM_VIDEO_FREEZE_OK,
	IHM_VIDEO_FREEZE_NOK,
		
	// in case of response this value is used with video.cmdval.dwValeur for result
	IHM_VIDEO_CMD_RSP 
}
enum_ihm_video_cmd;



// Liste des services disponibles pour IHM
typedef enum
{
    PREMIER_IHM_SERVICE = 0,
    M_IHM_CONFIG = M_SRV_USER,
    M_IHM_AFFICHAGE,
    M_IHM_TOUCHES,
    M_IHM_SAISIE_VISU,
    M_IHM_SAISIE_VISU_EX,
    M_IHM_INDICATEURS,
	M_IHM_POLICE,
	M_IHM_DETECTION_CHAINE,
	//TIC 2010/03/10
	M_IHM_AUTHORIZATION,
	M_IHM_ASYNC_INPUT,
	//TIC 2014/03/07
	M_IHM_AFFICHAGE_STATUS,


    BUTEE_IHM_SERVICE,
    DERNIER_IHM_SERVICE = BUTEE_IHM_SERVICE - 1,
    NB_IHM_SERVICE = BUTEE_IHM_SERVICE - PREMIER_IHM_SERVICE
}
enum_ihm_service;

// Liste des types de messages
typedef enum
{
    PREMIER_IHM_TYPE = 0,
    IHM_CONFIG_ACTUELLE = M_SRV_USER,  //  Privees
    IHM_NOUVELLE_TOUCHE,
    IHM_NOUVELLE_SAISIE,
    IHM_SAISIE_ANNULEE,
	
	//IHM_RESULT_DEMANDE_ANNULATION - response message for SRV_TYP_DEMANDE_ANNULATION 
	// The dialog_id member of the structure struct_ihm_srv_saisie_visu_ex; can be used to specify 
	// which dialog to cancel. If dialog_id is empty string, all dialogs are canceled.
	// For each canceled dialog the message of type IHM_RESULT_DEMANDE_ANNULATION is generated
	// NOTE: Only mailboxes that actually opened the service with M_IHM_SAISIE_VISU_EX, SRV_TYP_DEBUT 
	// would receive the IHM_RESULT_DEMANDE_ANNULATION messages
	IHM_RESULT_DEMANDE_ANNULATION, 
	IHM_NOUVELLE_CHAINE,	
	IHM_VIDEO_CMD_RESULT,	

    BUTEE_IHM_TYPE,
    DERNIER_IHM_TYPE = BUTEE_IHM_TYPE - 1,
    NB_IHM_TYPE = BUTEE_IHM_TYPE - PREMIER_IHM_TYPE
}
enum_ihm_type;


// STRUCTURES POUR PARAMETRES SERVICES IHM

/*
// Service Arret
typedef struct
{
    BYTE    dummy;
}
struct_ihm_srv_arret;

*/  


// Service Config
typedef struct
{
    BYTE    projet[IHM_LG_CHAINES_SRV];
    BYTE    no_version[IHM_LG_CHAINES_SRV];
    BYTE    langue[IHM_LG_CHAINES_SRV];
	BYTE    type_voie[IHM_LG_CHAINES_SRV];
	//the shift type names are defined in registry in DynamicColors.xml file 
    BYTE    type_poste[IHM_LG_CHAINES_SRV]; 
	BYTE    main_dlg_hidden[IHM_LG_CHAINES_SRV];//use "1" to hide and "0" or "" to show
}
struct_ihm_srv_config;

typedef struct
{
    enum_ihm_video_cmd      eCmd;
	union
	{
		char                szValeur[IHM_LG_CHAINES_SRV];
		DWORD               dwValeur;
	} cmdval;
}
struct_ihm_video_affichage;

//Buttons affichage manipulation
typedef enum
{
	enuIHM_OBJECT_UNCHANGED = -1,
	enuIHM_OBJECT_DISABLED = 0,
	enuIHM_OBJECT_ENABLED = 1,
}
enum_ihm_object_enabled;



typedef struct
{
	char      szValeurImg[IHM_LG_CHAINES_SRV]; 
	char      szValeurText[IHM_LG_CHAINES_SRV]; 
}
struct_ihm_button_affichage;

typedef struct
{
	int		iTargetDialog; 
	char    szUrlValue[IHM_LG_CHAINES_SRV]; 
}
struct_ihm_open_link;



typedef struct
{
	BYTE	bLDAPOffline; 
	BYTE	bManualLogin; 
	char    szLogin[IHM_LG_CHAINES_SRV]; 
}
struct_ihm_open_login_dlg;

typedef struct
{
	char    szMenuContent[IHM_MAX_XML_BUFFER_SIZE]; 
	BYTE	bUseExternalFile;
	BYTE	bDoNotReloadMenu;

}
struct_ihm_menu_affichage;

//-----------------------------------------
//table view commands
//-----------------------------------------
typedef enum
{
	enuIHM_AFF_TABLE_CMD_UNKNOWN = -1,
	enuIHM_AFF_TABLE_CMD_PREPEND_ROWS,
    enuIHM_AFF_TABLE_CMD_APPEND_ROWS,
    enuIHM_AFF_TABLE_CMD_RESET_ROWS,
    enuIHM_AFF_TABLE_CMD_RESET_AND_APPEND_ROWS,
    enuIHM_AFF_TABLE_CMD_COUNT
}
enum_aff_table_command;

typedef struct
{
	char    szContent[IHM_MAX_XML_BUFFER_SIZE]; 
	BYTE	bUseExternalFile;
	enum_aff_table_command eTableCmd;
}
struct_ihm_table_view_affichage;
//-----------------------------------------


//all objects visibility 
typedef enum
{
    enuIHM_AFF_OBJ_HIDDEN = 0xFF,
    enuIHM_AFF_OBJ_VISIBLE
}
enum_ihm_obj_visibility;


// Service Affichage
typedef struct
{
    enum_ihm_objets             objet;

	union
	{
		char                        szValeur[IHM_MAX_XML_BUFFER_SIZE];
		struct_ihm_video_affichage  video;
		struct_ihm_button_affichage button;
		struct_ihm_menu_affichage	menu;
		struct_ihm_open_link		link;
		struct_ihm_open_login_dlg   login_dlg; //IHM_OBJ_OpenLoginDlg
		struct_ihm_table_view_affichage table_view; 	//IHM_OBJ_TableView1 = 400,	IHM_OBJ_TableView2,	IHM_OBJ_TableView3
	} valeur;

	char        szToolTip[IHM_LG_CHAINES_SRV];

	//property:
	// -for the list objects: 1 - add item; 0 - remove item
	BYTE                        propriete; 

    enum_ihm_object_enabled     is_enabled;

	//for all the objects if enuIHM_AFF_OBJ_HIDDEN hide else show the item 
	enum_ihm_obj_visibility		visibility;
}
struct_ihm_srv_affichage;


// Service Police
#define IHM_REG_FONT_SEPARATOR ","

typedef struct
{
    enum_ihm_objets             objet;
    union
    {
		// Used to handle police value format which is a comma separated string, with values: 
		// 1.	Font family name, 
		// 2.	point size, 
		// 3.	font weight: <50 - normal, >50-Bold
		// 4.	underline: 0,1
		// 5.	italic:0,1
		// 6.	strike out:0,1
		// 7.	foreground color: hex number 
		// 8.	background color: hex number
		//example: Arial,12,70,0,0,0,000000,FFFFFF
		// If some values are omitted these properties should not be changed. 
		// This is correct for the foreground and background color and if none of the font parameter is used. 
		// If one of the font parameter is used the default font is updated with provided parameters and set to the widget. 
		// The problem is that it is possible that the font style on a specific widget is inherited
		// and that it is not known to the widget. 
		// That is why when the command is issued for the first time the missing font parameters are replaced 
		// with the ones from the default font defined in registry key DefaultFont. 
		// After that first POLICE command for the specific widget the missing parameters are not updated to the widget.
		// NOTE: If all values relative to font are omitted - the font is not changed!
		// If some values are omitted the values from default font are used! 
		// If you have no intention to change font, but only colors assure to send all the font parameters empty: 
		//  i.e.: ",,,,,,00FF00,FFFFFF"
		// If you need to change font or any of the font parameters assure to send all the font parameters 
		// at least first time when the command is issued for a widget. 
        char                    szValeur[IHM_LG_CHAINES_SRV];

    } valeur;
    BYTE                        propriete;
}
struct_ihm_srv_police;






// Service Touches
#define KB_DONT_CARE        0       // Pas d'enum pour reduire la taille du 
#define KB_WAIT             1       // tableau de controle de touches
#define KB_NM_WAIT          2
#define KB_GRP_NOTUSED      "\0"
#define KB_KEY_NOTUSED      0

//This IHM_COMMANDS_ACTIVATE_KEY key is used to identify the status for all commands
//If the Key IHM_COMMANDS_ACTIVATE_KEY is KB_DONT_CARE (default) commands will not be emit to the user
//To receive commands from the IHM set KB_WAIT for Key 300
//Warning: Do not use the scancode 300 in configuration 
// otherwise the key that is using it will be enabled/disabled with commands
// and vice versa
#define IHM_COMMANDS_ACTIVATE_KEY		300

#define SPECIFIC_ACTION_BTN_IDENTIFICATION	"ACTION_BTN_IDENTIFICATION" //used to identify DESKTOP identification request 

typedef enum
{
    enuIHM_TOUCHE_Key = 0,
    enuIHM_TOUCHE_Command = 1
}
enum_ihm_touche_type;


typedef struct
{
    char    nomgroupe[IHM_LG_CHAINES_SRV];
    int     touche;
    BYTE    etat;
	enum_ihm_touche_type type;	
	char    command[IHM_LG_CHAINES_SRV];
	char    command_param[IHM_LG_CHAINES_SRV];
}
struct_ihm_srv_touches;

// Service M_IHM_SAISIE_VISU
#define ihmBoutonOK         1
#define ihmBoutonCANCEL     2
#define ihmBoutonOKCANCEL   (ihmBoutonOK + ihmBoutonCANCEL)

#define IHM_SEPARATOR_CHAR    '|'
#define IHM_SEPARATOR_STRING  "|"

#define IHM_SEPARATOR2_CHAR    '~'
#define IHM_SEPARATOR2_STRING  "~"

// tmo 27/11/2000
#define IHMSAISIE_VISUSEUL		0												
#define IHMSAISIE_SAISIE		1
// saisie clavier + tous peripheriques de saisie externe (douchette, ZIP...)
#define IHMSAISIE_MARQUEUR_TOUS		2 
// saisie clavier + peripheriques de saisie externe actives par le service string detection (douchette, ZIP...)
#define IHMSAISIE_MARQUEUR_ACTIF	3 

typedef struct 
{ 
    BYTE    boutons;
    DWORD   b_saisie;		// tmo 27/11/2000 BOOL devient DWORD
    BYTE    string[IHM_LG_SAISIE_MAX];
} 
struct_ihm_srv_saisie_visu;



// When struct_ihm_srv_saisie_visu_ex is used with SRV_TYP_DEMANDE_ANNULATION msg type
// the dialog_id is considered to be the target dialog to be canceled
// If the dialog_id is empty string it will cancel all the dialogs.
// For each of the dialogs canceled by the main application the module will return IHM_RESULT_DEMANDE_ANNULATION
// with this structure dialog_id set to the canceled dialog_id
typedef struct 
{ 
    BYTE    dialog_id[IHM_LG_CHAINES_SRV];
	BYTE	is_file;   //if the data content is file name or message itself
    BYTE    data[IHM_MAX_XML_BUFFER_SIZE]; 
} 
struct_ihm_srv_saisie_visu_ex;


// Service Indicateurs (Flags)
typedef struct
{   
    BYTE    indicateur;
    BYTE    etat;
}
struct_ihm_srv_indicateurs;

// Service Detection de chaine
typedef struct
{   
	BYTE	ChaineComplete;		// indique si la chaine transmise est complete ou coupee par timeout
    BYTE	EnvoiePrePostambule;	// Dword Indiquant si les pre et post ambuyle sont renvoyes a l'application.
	BYTE    CoupleId[IHM_LG_CHAINES_SRV];	 // identifiant du couple pre/postambul (a compare avec la registerie
	BYTE	ChaineSaisie[IHM_LG_SAISIE_MAX];	// chaine saisie en cas de retour
}
struct_ihm_srv_detection;



//TIC 2010/03/10
//	M_IHM_AUTHORIZATION,
typedef enum
{
	IHM_AUTH_UNKNOWN=0,
	IHM_AUTH_REQ=1,
	IHM_AUTH_RSP,	
	IHM_TAKE_CTRL_REQ,
	IHM_TAKE_CTRL_RSP,
	IHM_RETURN_CTRL_REQ,
	IHM_RETURN_CTRL_RSP,
	IHM_AUTH_LOGOFF_REQ,
	IHM_AUTH_DLG_CANCELED
	
}
enum_ihm_auth_cmd;


typedef enum
{
	enuAUTH_NOK=0,
	enuAUTH_OK=1,
	enuAUTH_LDAP_OFFLINE=2
}
enum_ihm_auth_ret_val;

typedef enum
{
	enuRETURN_CTRL_UNKNOWN = 0,
	enuRETURN_CTRL_VOLUNTARY,
	enuRETURN_CTRL_FORCED,
	enuRETURN_CTRL_IDLE_TIMEOUT,
	enuRETURN_CTRL_TAKEOVER_TIMEOUT
}
enum_ihm_auth_ret_ctrl_reason;


//NOTE: enum_srv_type  type_message for_
//		-requests is to be set to SRV_TYP_DEMANDE
//      -responses is to be set SRV_TYP_EFFECTUE
typedef struct  
{
    enum_ihm_auth_cmd             cmd_type;

	char    user_id[IHM_LG_CHAINES_SRV];	
	char    user_name[IHM_LG_CHAINES_SRV];	//name to be filled in response which is used at dialogs
	char	sec_code[IHM_LG_CHAINES_SRV]; //pasword or security code
	BYTE	is_sec_code_pwd; //1 - pasword; 0 - security code
	BYTE	is_user_id_auth; //1 - authentificator or web hash ; 0 - PAN or user name
	char	session_id[IHM_LG_CHAINES_SRV]; //IHM session ID to be used to identify auth request(not the web session)
	char	errmsg[IHM_LG_SAISIE_MAX]; //error message for the NOK response if not set the default is used (IHM_LABEL_MSG_INVALID_USER_OR_PWD) 
	
	enum_ihm_auth_ret_ctrl_reason	reason;		//flag used for additional information on return control
	
	BYTE	is_desktop;   //flag to know if the user is logging on to WEB or to DESKTOP interface
	enum_ihm_auth_ret_val	ret_val; // 0-NOK, 1-OK, 2-Try security code (LDAP offline)
}
struct_ihm_authorization;



//M_IHM_ASYNC_INPUT 

typedef struct
{
	BYTE	is_file;   //if the data content is file name or message itself
    BYTE    data[IHM_LG_SAISIE_MAX]; 
}
struct_ihm_async_input;




// DeFINITION DU MESSAGE INTER-TACHE

// structure des messages concernant le dialogue avec l'application (services)
typedef struct
{
    struct_srv_entete               entete;

    union
    {
        struct_srv_arret				srv_arret;
        struct_ihm_srv_config			srv_config;
        struct_ihm_srv_affichage		srv_affichage;
        struct_ihm_srv_touches			srv_touches;
        struct_ihm_srv_saisie_visu		srv_saisie_visu;
		struct_ihm_srv_saisie_visu_ex	srv_saisie_visu_ex;
        struct_ihm_srv_indicateurs		srv_indic;
		struct_ihm_srv_police			srv_police;		
		struct_ihm_srv_detection		srv_detection;		
		struct_ihm_authorization		srv_auth;
		struct_ihm_async_input			srv_async_input;
    } u;
}
struct_ihm_message;


/*---------------------------- PROTOTYPES -------------------------*/


#include <undef.h>
#endif   // IHM_H
/*---------------------------- END OF FILE -------------------------*/
