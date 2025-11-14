/* --------------------------------------------------------------------
 * (C) 1998 Sanef ITS Croatia - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Ihm_qt 
 * FILE       : Ihm_loc.h
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
#ifndef IHM_LOC_H
#define IHM_LOC_H

/*--------------- INCLUDES: ---------------*/


/*---------------------------- MACRO -------------------------*/
//Reg value names
#define IHM_REG_VAL_DEFAULT_LANGUAGE    "DefaultLanguage"
#define IHM_REG_VAL_LANGUAGES_CFG_FILE  "LanguageFilePath"

#define IHM_REG_VAL_IMAGES_CFG_DSK		"ImagesCfgDesktop"
#define IHM_REG_VAL_IMAGES_CFG_WEB		"ImagesCfgWeb"


#define IHM_REG_VAL_ACTIONS_CFG			"ActionsCfg"
#define IHM_REG_VAL_CURSOR_CFG_DSK		"CursorsCfgDesktop"

#define IHM_REG_VAL_DYNAMIC_CFG			"DynamicCfg" 

#define IHM_REG_VAL_DEFAULT_FONT		"DefaultFont"
#define IHM_REG_VAL_DEFAULT_DATE_FORMAT	"DefaultDateFormat"
#define IHM_REG_VAL_DEFAULT_TIME_FORMAT	"DefaultTimeFormat"

#define IHM_REG_VAL_DEFAULT_RES_PRECISION		"DefaultResultPrecision"
#define IHM_REG_VAL_DEFAULT_DECIMAL_SEPARATOR	"DefaultDecimalSeparator"
#define IHM_REG_VAL_INPUTDLGS_ERROR_FILEDS_STYLE "InputDlgsErrorFieldsStyle"
#define IHM_REG_VAL_INPUTDLGS_VALID_ENABLED		 "InputDlgsValidAlwaysEnabled"

#define IHM_REG_VAL_SHOW_CANCEL_ON_CONNECTING_DLG	"ShowCancelOnConnectingDlg"


#define IHM_REG_VAL_AUTHORISATION_REQ_TIMEOUT	"AuthReqTimeout_ms"
#define IHM_REG_VAL_ANI_THREAD_PRIORITY			"PriorityANIThread"
#define IHM_REG_VAL_WEB_THREAD_PRIORITY			"PriorityWEBThread"


#define IHM_REG_VAL_DISPLAY_ENTRY_GATE	"DisplayEntryGate"
#define IHM_REG_VAL_DISPLAY_ENTRY_LOOP	"DisplayEntryLoop"
#define IHM_REG_VAL_HIDE_CURSOR			"HideCursor"


#define IHM_REG_VAL_SHOW_SPLASH				"ShowSplashScreen"
#define IHM_REG_VAL_SPLASH_TEMPLATE			"SplashScreenTemplate"
#define IHM_REG_VAL_INPUT_DLG_TEMPLATE		"DefaultInputDialogTemplate"
#define IHM_REG_VAL_ABOUT_DLG_TEMPLATE		"AboutDialogTemplate"
#define IHM_REG_VAL_ABOUT_DLG_DEFAULT_CFG	"AboutDialogDefaultCfg"


#define IHM_REG_VAL_DYNAMIC_COLOR_FILE		"DynamicColorsFilePath"

#define IHM_REG_VAL_HTML_ENC_MAP_FILE		"HtmlEncMapFilePath"


#define IHM_REG_VAL_UI_TEMPLATE_ROOT	 "UITemplateRoot"
#define IHM_REG_VAL_UI_CSS_ROOT			 "UICSSRoot"
#define IHM_REG_VAL_UI_INPUT_EX_DEF_ROOT "UIInputExDefRoot"
#define IHM_REG_VAL_COMM_FILES_ROOT		 "CommFilesRoot"

#define IHM_REG_VAL_NUMLOCK_PERIOD		"NumLockPeriod"
#define IHM_REG_VAL_NUMLOCK_SYNCHKEY	"NumLockSynchroKey"
#define IHM_REG_VAL_REFRESH_KEY			"KeyRefresh"
#define IHM_REG_VAL_ABOUT_KEY			"KeyAbout"
#define IHM_REG_VAL_MENU_KEY			"KeyMenu"
#define IHM_REG_VAL_RETURNCTRL_KEY		"KeyReturnControl"
#define IHM_REG_VAL_TAKECTRL_KEY		"KeyTakeControl"
#define IHM_REG_VAL_VALID_KEY			"KeyValid"

#define IHM_REG_VAL_SYSTEM_ENCODING		"SystemEncoding"
#define IHM_REG_VAL_LANE_TYPES_CFG_FILE	"LaneTypesFilePath"

#define IHM_REG_VAL_SHOW_TITLE_BAR				"ShowTitleBar"
#define IHM_REG_VAL_NO_RESOLUTION_CHECK			"NoResolutionCheck"
#define IHM_REG_VAL_DO_UI_TEMPL_VERIFICATION	"DoUITemplatesVerification"

#define IHM_REG_VAL_RESOLUTION_X		"ResolutionX"
#define IHM_REG_VAL_RESOLUTION_Y		"ResolutionY"
#define IHM_REG_VAL_MANUAL_STARTUP_POSITION		"ManualStartupPosition"
#define IHM_REG_VAL_STARTUP_POSITION_LEFT		"StartupPositionLeft"
#define IHM_REG_VAL_STARTUP_POSITION_TOP		"StartupPositionTop"
#define IHM_REG_VAL_STARTUP_SCREEN_INDEX		"StartupScreenIndex"
#define IHM_REG_VAL_STARTUP_HIDE_IF_SINGLE_SCREEN	"StartupHideIfSingleScreen"


#define IHM_REG_VAL_MAX_INPUT_MSGS_PER_SEC "MaximumInputMsgsPerSeccond"

#define IHM_REG_VAL_WEB_ENABLED			"WebInterfaceEnabled"

#define IHM_REG_VAL_START_TIMEOUT			"StartTimeout"
#define IHM_REG_VAL_TAKE_CONTROL_TIMEOUT	"TakeCtrlTimeout_ms"
#define IHM_REG_VAL_AUTH_AT_LANE			"AuthorisationAtLaneApp"

#define IHM_REG_VAL_DEFAULT_VIDEO_PIC		"DefaultVideoPicture"
#define IHM_REG_VAL_DEFAULT_VIDEO_SOURCE	"DefaultVideoSource"
#define IHM_REG_VAL_DEFAULT_VIDEO_ZOOM		"DefaultVideoZoom"
#define IHM_REG_VAL_VIDEO_ZOOM_X			"VideoZoom"
#define IHM_REG_VAL_VIDEO_ZOOM_NO			"VideoZoomNo"

#define IHM_REG_VAL_VIDEO_MAX_TABLE_ROWS_IN_MEMORY	"MaxTableRowsInMemory"

#define IHM_REG_VAL_TEST_KEYBOARD_MASK_REGEX	"TestKeyboardMaskRegex"
#define IHM_REG_VAL_TEST_KEYBOARD_MASK_ENABLED	"TestKeyboardMaskEnabled"
#define IHM_REG_VAL_TEST_KEYBOARD_MASK_CHAR		"TestKeyboardMaskChar"
#define IHM_REG_VAL_FILTER_STRING_DETECTED_KEYS	"FilterStringDetectedKeys"


#define IHM_REG_VAL_WEB_PORT			"WebServerPort"
#define IHM_REG_VAL_INIT_MAIN_PAGE		"WebInitMainPage"
#define IHM_REG_VAL_MINI_WEB_ERROR_PAGE	"WebMiniWebErrorPage"

#define IHM_REG_VAL_LOGIN_FAIL_PAGE		"WebLoginFailPage"
#define IHM_REG_VAL_LOGOUT_PAGE			"WebLogoutPage"
#define IHM_REG_VAL_WEB_SESSION_TIMEOUT	"WebSessionTimeoutSec"
#define IHM_REG_VAL_MINI_WEB_SESSION_TIMEOUT	"MiniWebSessionTimeoutSec"

#define IHM_REG_VAL_VIRTUAL_KEYBOARD_ENABLED		"VirtualKeyboardEnabled"
#define IHM_REG_VAL_VIRTUAL_KEYBOARD_MBOX_NAME		"VirtualKeyboardMboxName"
#define IHM_REG_VAL_VIRTUAL_KEYBOARD_DEFAULT_CFG		"VirtualKeyboardDefaultConfig"



//Parameters found in ConfigDynamic.xml
#define IHM_DYNAMIC_CONF_PARAM_MAX_WEB_CLIENTS "MaxWebClients"
#define IHM_DYNAMIC_CONF_PARAM_MAX_MINIWEB_CLIENTS "MaxMiniWebClients"
#define IHM_DYNAMIC_CONF_PARAM_IDLE_TIMEOUT_DSK "IdleTimeoutDskSec"
#define IHM_DYNAMIC_CONF_PARAM_IDLE_TIMEOUT_WEB "IdleTimeoutWebSec"
#define IHM_DYNAMIC_CONF_PARAM_TAKE_CTRL_TIMEOUT "TakeCtrlTimeoutSec"

// OTHER CONSTANTS 
#define IHM_REG_FONT_SEPARATOR ","

//------------------------------------------------------------------
//Error messages initiated from MIhmSrv for the authorization response
//------------------------------------------------------------------
#define IHM_LABEL_MSG_INVALID_USER_OR_PWD 		"LABEL_INVALID_USER_OR_PWD" // the default authorization rejected messsage
#define IHM_LABEL_MSG_NO_AUTHORIZATION_SERVICE 	"LABEL_NO_AUTHORIZATION_SERVICE"


//---------------------------------------------------------------
//--- For ihm objects: 	
//		IHM_OBJ_FileView1 = 400,
//		IHM_OBJ_FileView2,
//		IHM_OBJ_FileView3,
//   Specific type of xml structures used for initialization format and data
//  Format file example:
//---------------------------------------------------------------
// <?xml version="1.0" encoding="UTF-8"?>
// <TABLE_DATA>
// 	<HEADER HEIGHT="26" RESIZE="false">
// 		<COLUMN TEXT="IDP" WIDTH="70" ALIGN="center"/>
// 		<COLUMN TEXT="Details" WIDTH="387" ALIGN="center"/>
// 		<COLUMN TEXT="Montant" WIDTH="131" ALIGN="center"/>
// 	</HEADER>
// 	<BODY HEIGHT="25" CAN_GET_FOCUS="0" VSCROLL_POLICY="1" HSCROLL_POLICY="1">
// 	</BODY>
// </TABLE_DATA>
//---------------------------------------------------------------
//  Data file example:
//---------------------------------------------------------------
// <TABLE_DATA>
// 		<ROW>
// 			<COLUMN align="center" text="TLP"  action="TRANSACTION_HIS_EDIT" action_params="0099101" tooltip="Trs.Num: 0099101 Hour:12:55:58  Class:1 Payment:TLP"/>
// 			<COLUMN align="center" text="0099101" action="TRANSACTION_HIS_EDIT" action_params="0099101"   tooltip="Trs.Num: 0099101 Hour:12:55:58  Class:1 Payment:TLP"/>
// 			<COLUMN align="center" text="1,00"  action="TRANSACTION_HIS_EDIT_CLASS" action_params="0099101"  tooltip="Trs.Num: 0099101 Hour:12:55:58  Class:1 Payment:TLP"/>
// 		</ROW>
// </TABLE_DATA>
// ---- To parse the xml we use these predefined names:

#define IHM_CFG_VAL_TABLEVIEW_ROOT_ELEMENT "TABLE_DATA"
#define IHM_CFG_VAL_TABLEVIEW_HEADER_ELEMENT "HEADER" 
#define IHM_CFG_VAL_TABLEVIEW_BODY_ELEMENT "BODY" 
#define IHM_CFG_VAL_TABLEVIEW_ROW_ELEMENT "ROW" 
#define IHM_CFG_VAL_TABLEVIEW_COLUMN_ELEMENT "COLUMN" 
#define IHM_CFG_VAL_TABLEVIEW_TOOLTIP_ATTRIBUTE "TOOLTIP" 
#define IHM_CFG_VAL_TABLEVIEW_RESIZE_ATTRIBUTE "RESIZE" 
#define IHM_CFG_VAL_TABLEVIEW_CAN_GET_FOCUS_ATTRIBUTE "CAN_GET_FOCUS" 

#define IHM_CFG_VAL_TABLEVIEW_VSCROLL_POLICY_ATTRIBUTE "VSCROLL_POLICY" 
#define IHM_CFG_VAL_TABLEVIEW_HSCROLL_POLICY_ATTRIBUTE "HSCROLL_POLICY" 

#define IHM_CFG_VAL_TABLEVIEW_HEIGHT_ATTRIBUTE "HEIGHT" 
#define IHM_CFG_VAL_TABLEVIEW_WIDTH_ATTRIBUTE "WIDTH" 
#define IHM_CFG_VAL_TABLEVIEW_BOLD_ATTRIBUTE "BOLD"  // is font of type bold -- 1 - font-weight:bold; 0 -font-weight:normal; 
#define IHM_CFG_VAL_TABLEVIEW_TEXT_STRIKEOUT_ATTRIBUTE "TEXT-STRIKEOUT" //  is text strikeout
#define IHM_CFG_VAL_TABLEVIEW_TEXT_ATTRIBUTE "TEXT" 
#define IHM_CFG_VAL_TABLEVIEW_ICO_ATTRIBUTE "ICO" 
#define IHM_CFG_VAL_TABLEVIEW_ACTION_ATTRIBUTE "ACTION" 
#define IHM_CFG_VAL_TABLEVIEW_ALIGN_ATTRIBUTE "ALIGN" 
#define IHM_CFG_VAL_TABLEVIEW_ACTION_PARAMS_ATTRIBUTE "ACTION_PARAMS" 
#define IHM_CFG_VAL_TABLEVIEW_ACTION_KEY_CODE_ATTRIBUTE "ACTION_KEY_CODE" //indicates key for the column that is executing its selected cell


//----------------------------------------------------------------------------
//---------------------------------------------------------------
//--- For ihm objects: 	
// 	IHM_OBJ_HMenuView1,
// 	IHM_OBJ_HMenuView2,
//   Specific type of xml structure is used for initialization! 
// Example:
//---------------------------------------------------------------
// <MENU LABEL="Main menu" TOOLTIP="Main menu">
// 	<ITEM LABEL="MODE" ICON="" TOOLTIP="Commandes de changement de mode">
// 		<SUBMENU submenuID="MNU_MODE">
// 			<ITEM LABEL="Arr?t" ACTION="MDARRET" ICON="" TOOLTIP="" ENABLED="1"/>
// 			<ITEM LABEL="Exploitation" ICON="" TOOLTIP="Exploitation">
// 				<SUBMENU submenuID="MDEXP">
// 					<ITEM LABEL="Libre" ACTION="MDEXPLIBRE" ICON="" TOOLTIP="" ENABLED="0"/>
// 					<ITEM LABEL="Formation" ACTION="MDEXPFORM" ICON="" TOOLTIP=""/>
// 				</SUBMENU>
// 			</ITEM>
// 			<ITEM LABEL="Maintenance" ICON="" TOOLTIP="Commandes de changement de mode / etat  maintenance ">
// 				<SUBMENU submenuID="MDMNT">
// 					<ITEM LABEL="Specialise telepeage" ACTION="MDMNTST" ICON="" TOOLTIP=""/>
// 				</SUBMENU>
// 			</ITEM>
// 		</SUBMENU>
// 	</ITEM>
// 	<ITEM LABEL="PORTES" ICON="" TOOLTIP="Actions liees au controle des portes" ENABLED="1">
// 		<SUBMENU submenuID="MNU_PORTES">
// 			<ITEM LABEL="Porte maintenance" ACTION="PORMAINT" ICON="" TOOLTIP=""/>
// 			<ITEM LABEL="Tiroir lecteurs" ACTION="PORLECT" ICON="" TOOLTIP=""/>
// 			<ITEM LABEL="Coffre, sac et bol" ACTION="PORESP" ICON="" TOOLTIP=""/>
// 		</SUBMENU>
// 	</ITEM>
// </MENU>
// ---- To parse this xml we use these predefined names:

#define IHM_CFG_VAL_MENU_FILE_ROOT_ELEMENT "MENU"
#define IHM_CFG_VAL_MENU_FILE_ITEM_ELEMENT "ITEM"
#define IHM_CFG_VAL_MENU_FILE_SUBMENU_ELEMENT "SUBMENU"

#define IHM_CFG_VAL_MENU_FILE_LABEL_ATTRIBUTE "LABEL"
#define IHM_CFG_VAL_MENU_FILE_ICON_ATTRIBUTE "ICON"
#define IHM_CFG_VAL_MENU_FILE_TOOLTIP_ATTRIBUTE "TOOLTIP"
#define IHM_CFG_VAL_MENU_FILE_SUBMENU_ID_ATTRIBUTE "submenuID"
#define IHM_CFG_VAL_MENU_FILE_ACTION_ATTRIBUTE "ACTION"
#define IHM_CFG_VAL_MENU_FILE_ACTION_PARAMS_ATTRIBUTE "action_params"
#define IHM_CFG_VAL_MENU_FILE_ENABLED_ATTRIBUTE "ENABLED"

#define IHM_REG_SHIFT_COL_PREFIX  '#'


// Service M_IHM_SAISIE_VISU_EX,
#define IHM_SAISIE_EX_INPUT_CHK_BOX			"CheckBox"
#define IHM_SAISIE_EX_INPUT_RADIO_BTN		"RadioButton"
#define IHM_SAISIE_EX_INPUT_COMBO_BOX		"ComboBox"
#define IHM_SAISIE_EX_INPUT_LIST_WIDGET		"ListWidget"
#define IHM_SAISIE_EX_INPUT_LINE_EDIT		"LineEdit"
#define IHM_SAISIE_EX_INPUT_LINE_EDIT_EXT	"LineEditExt"
#define IHM_SAISIE_EX_INPUT_HIDDEN_EDIT		"HiddenLineEdit"

#define IHM_SAISIE_EX_INPUT_TEXT_EDIT			"TextEdit"
#define IHM_SAISIE_EX_INPUT_LABEL				"Label"
#define IHM_SAISIE_EX_INPUT_LABEL_LUHN_CHECK	"LabelLuhnCheck"


#define IHM_SAISIE_EX_INPUT_BUTTON_OK		"PushButtonOK"
#define IHM_SAISIE_EX_INPUT_BUTTON_CANCEL	"PushButtonCancel"
#define IHM_SAISIE_EX_INPUT_BUTTON			"PushButton" // for any additional generic button


#define IHM_SAISIE_EX_FIELD_FORMAT_FILTER				"FILTER"		// format_detail= i.e. "lstData"
#define IHM_SAISIE_EX_FIELD_FORMAT_FILTER_BEGINS_WITH	"FILTER_BEGINS_WITH"		// format_detail= i.e. "lstData"
#define IHM_SAISIE_EX_MAX_ROWS_IN_OPTION_LIST	200		// format_detail= i.e. "lstData"

#define IHM_SAISIE_EX_FIELD_FORMAT_LUHN_KEY				"LUHN_KEY"		 // format_detail= i.e. "lblLuhnKeyValidator"
#define IHM_SAISIE_EX_FIELD_FORMAT_LUHN_KEY_MOD16		"LUHN_KEY_MOD16"		// format_detail= i.e. "lblLuhnKeyValidator"
#define IHM_SAISIE_EX_FIELD_FORMAT_LUHN_KEY_MOD16_M3	"LUHN_KEY_MOD16_M3"		// format_detail= i.e. "lblLuhnKeyValidator"

#define IHM_SAISIE_EX_FIELD_FORMAT_NUMBER			"N"		// format_detail= N+03
#define IHM_SAISIE_EX_FIELD_FORMAT_TEXT				"C"		// format_detail= ""
#define IHM_SAISIE_EX_FIELD_FORMAT_MASK				"M"		// format_detail= MM/DD/YYYY
#define IHM_SAISIE_EX_FIELD_FORMAT_RIGHTMASK		"R"		// format_detail= R06
#define IHM_SAISIE_EX_FIELD_FORMAT_CALCULATION		"F"		// format_detail= "(txtVal + txtVal3) * txtVal2"

#define IHM_SAISIE_EX_SEPARATEUR_CHAMP "|"
#define IHM_SAISIE_EX_SEPARATEUR_PARAM "~"

#define IHM_FIELD_FORMAT_CHAR_SIGNED '+'
#define IHM_FIELD_FORMAT_CHAR_ZERO	 '0'



#define IHM_SAISIE_EX_ROOT_DEF	"input_definition"
#define IHM_SAISIE_EX_ROOT_DESC "input_description"

#define IHM_SAISIE_EX_ELEMENT "widget"

#define IHM_SAISIE_EX_ATT_DSKCSS_FILE "desktop_css_file"
#define IHM_SAISIE_EX_ATT_TYPE "type"
#define IHM_SAISIE_EX_ATT_NAME "name"
#define IHM_SAISIE_EX_ATT_FORMAT "format"
#define IHM_SAISIE_EX_ATT_FORMAT_DETAIL "format_detail"
#define IHM_SAISIE_EX_ATT_VALIDATOR "validator"
#define IHM_SAISIE_EX_ATT_MIN_LEN "min_len"
#define IHM_SAISIE_EX_ATT_MAX_LEN "max_len"
#define IHM_SAISIE_EX_ATT_READONLY "readonly"
#define IHM_SAISIE_EX_ATT_SOURCE "src"
#define IHM_SAISIE_EX_ATT_IS_HTML "is_html"
#define IHM_SAISIE_EX_ATT_VALUE "value"
#define IHM_SAISIE_EX_ATT_TEXT "text"
#define IHM_SAISIE_EX_ATT_MULTISELECTION "multiselection"
#define IHM_SAISIE_EX_ATT_BTN_ID "btn_id"


//attributes for calculation field configuration
#define IHM_SAISIE_EX_ATT_RESULT_PRECISION "result_precision"
#define IHM_SAISIE_EX_ATT_ENABLE_LABEL_SWITCHING "enable_label_switching"
#define IHM_SAISIE_EX_ATT_LABEL_TRUE "label_true"
#define IHM_SAISIE_EX_ATT_LABEL_FALSE "label_false"
#define IHM_SAISIE_EX_ATT_LABEL_SWITCH_FORMULA "lbl_switch_formula"



#define IHM_SAISIE_EX_LIST_OPT_ROOT "option_list"
#define IHM_SAISIE_EX_LIST_OPTION "option"




#endif   // IHM_LOC_H
/*---------------------------- END OF FILE -------------------------*/
