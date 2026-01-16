/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     conf_srvpv.h													 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifndef CONF_SRVPV_H
#define CONF_SRVPV_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <csr_list.h>
#include <lane.h>
#include <msg_pv_define.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef CONF_SRVPV_EXPORTS
	#include "public.h"
#else
	#include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

__declspec(dllexport) BOOL WINAPI CONF_NTSVC_Inherit_Handle(void *pvContext);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_First_Plaza(OUT DWORD *plazanum,
														  OUT CHAR plazaname[MSG_PV_MAX_PLAZA_NAME],
														  IN DWORD dwDefaultPlazaNumber);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_Next_Plaza(OUT DWORD *plazanum,
														 OUT CHAR plazaname[MSG_PV_MAX_PLAZA_NAME],
														 IN DWORD dwDefaultPlazaNumber);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_First_Lane(IN DWORD plazanum,
														 OUT DWORD *lanenum,
														 OUT CHAR lanename[MSG_PV_MAX_LANE_NAME],
														 OUT DWORD *lanetype);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_Next_Lane(OUT DWORD *lanenum,
														OUT CHAR lanename[MSG_PV_MAX_LANE_NAME],
														OUT DWORD *lanetype);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_First_Lane_Device(IN DWORD plazanum,
																IN DWORD lanenum,
																OUT DWORD *dev_id);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_Next_Lane_Device(OUT DWORD *dev_id);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_First_Display(IN DWORD plazanum, OUT DWORD *disp_id);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_Next_Display(OUT DWORD *disp_id);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_First_Display_Zone(IN DWORD plazanum,
																 IN DWORD disp_id,
																 OUT DWORD *zone_id);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_Next_Display_Zone(OUT DWORD *zone_id);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_First_Display_Zone_Lane(IN DWORD plazanum,
																	  IN DWORD disp_id,
																	  IN DWORD zone_id,
																	  OUT DWORD *lane_number);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_Next_Display_Zone_Lane(OUT DWORD *lane_number);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_User_Login(IN CHAR name_in[MSG_PV_MAX_USER_NAME],
														 IN DWORD matricule_in,
														 IN CHAR *password,
														 IN DWORD dwPasswordSize,
														 OUT CHAR name_out[MSG_PV_MAX_USER_NAME],
														 OUT DWORD *matricule_out,
														 OUT CHAR firstname[MSG_PV_MAX_USER_FIRSTNAME],
														 OUT DWORD *user_ok,
														 OUT DWORD *profile_max_connections,
														 OUT DWORD *profile_id,
														 OUT char profile_name[MSG_PV_MAX_USER_PROFILE_NAME]);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_User_Auth_Fun(IN CHAR name_in[MSG_PV_MAX_USER_NAME],
															IN DWORD matricule_in,
															IN CHAR *password,
															IN DWORD dwPasswordSize,
															IN DWORD func_id,
															OUT CHAR name_out[MSG_PV_MAX_USER_NAME],
															OUT DWORD *matricule_out,
															OUT CHAR firstname[MSG_PV_MAX_USER_FIRSTNAME],
															OUT DWORD *user_ok,
															OUT DWORD *profile_id,
															OUT char profile_name[MSG_PV_MAX_USER_PROFILE_NAME]);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_User_Auth_Cmd(IN CHAR name_in[MSG_PV_MAX_USER_NAME],
															IN DWORD matricule_in,
															IN CHAR *password,
															IN DWORD dwPasswordSize,
															IN DWORD cmd_id,
															OUT CHAR name_out[MSG_PV_MAX_USER_NAME],
															OUT DWORD *matricule_out,
															OUT CHAR firstname[MSG_PV_MAX_USER_FIRSTNAME],
															OUT DWORD *user_ok,
															OUT DWORD *profile_id,
															OUT char profile_name[MSG_PV_MAX_USER_PROFILE_NAME]);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_First_User_Forbidden_Function(IN CHAR name[MSG_PV_MAX_USER_NAME],
																			IN DWORD matricule,
																			OUT DWORD *func_id,
																			OUT DWORD *func_mode);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_Next_User_Forbidden_Function(OUT DWORD *func_id,
																		   OUT DWORD *func_mode);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_First_User_Allowed_Command(IN CHAR name[MSG_PV_MAX_USER_NAME],
																		 IN DWORD matricule,
																		 OUT DWORD *cmd_id,
																		 OUT DWORD *cmd_mode,
																		 OUT CHAR cmd_label[MSG_PV_MAX_COMMAND_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_Next_User_Allowed_Command(OUT DWORD *cmd_id,
																		OUT DWORD *cmd_mode,
																		OUT CHAR cmd_label[MSG_PV_MAX_COMMAND_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_First_User_Command_Lane_Type(IN CHAR name[MSG_PV_MAX_USER_NAME],
																		   IN DWORD matricule,
																		   IN DWORD cmd_id,
																		   OUT DWORD *type_id);

__declspec(dllexport) BOOL WINAPI CONF_PV_Get_Next_User_Command_Lane_Type(OUT DWORD *type_id);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_First_Plaza(OUT DWORD *plaza_number, IN DWORD dwDefaultPlazaNumber);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_Next_Plaza(OUT DWORD *plaza_number, IN DWORD dwDefaultPlazaNumber);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_First_Lane(IN DWORD plazanum, OUT DWORD *lane_number);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_Next_Lane(OUT DWORD *lane_number);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_First_Display(IN DWORD plazanum,
															   OUT DWORD *display_id,
															   OUT DISP_COUNT counter[LANE_MAX_COUNTERS]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_Next_Display(OUT DWORD *display_id,
															  OUT DISP_COUNT counter[LANE_MAX_COUNTERS]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_First_Currency(OUT DWORD *currency_id,
																OUT CHAR label[MSG_PV_MAX_CURRENCY_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_Next_Currency(OUT DWORD *currency_id,
															   OUT CHAR label[MSG_PV_MAX_CURRENCY_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_First_Payment(OUT DWORD *payment_id,
															   OUT CHAR label[MSG_PV_MAX_PAYMENT_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_Next_Payment(OUT DWORD *payment_id,
															  OUT CHAR label[MSG_PV_MAX_PAYMENT_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_First_Payment_Sub_Type(IN DWORD pay_id,
																		OUT DWORD *pay_sub_id,
																		OUT CHAR pay_name[MSG_PV_MAX_PAYMENT_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_Next_Payment_Sub_Type(OUT DWORD *pay_sub_id,
																	   OUT CHAR pay_name[MSG_PV_MAX_PAYMENT_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_First_Class(OUT DWORD *class_id,
															 OUT CHAR label[MSG_PV_MAX_CLASS_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_Next_Class(OUT DWORD *class_id,
															OUT CHAR label[MSG_PV_MAX_CLASS_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_First_Anomaly(OUT DWORD *anomaly_id,
															   OUT CHAR label[MSG_PV_MAX_ANOMALY_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_Next_Anomaly(OUT DWORD *anomaly_id,
															  OUT CHAR label[MSG_PV_MAX_ANOMALY_LABEL]);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_First_Coin(OUT DWORD *coin_id,
															OUT DWORD *currency_id,
															OUT CHAR coin_label[MSG_PV_MAX_COIN_LABEL],
															OUT DOUBLE *value,
															OUT DWORD *bag);

__declspec(dllexport) BOOL WINAPI CONF_PLAZA_Get_Next_Coin(OUT DWORD *coin_id,
															OUT DWORD *currency_id,
															OUT CHAR coin_label[MSG_PV_MAX_COIN_LABEL],
															OUT DOUBLE *value,
															OUT DWORD *bag);

__declspec(dllexport) BOOL WINAPI CONF_LANE_Get_First_Eqt(IN DWORD plaza_number,
														IN DWORD lane_number,
														OUT DWORD *eqt_id,
														OUT DWORD *alarm_event,
														OUT DWORD *alarm_sub_event);

__declspec(dllexport) BOOL WINAPI CONF_LANE_Get_Next_Eqt(OUT DWORD *eqt_id,
														OUT DWORD *alarm_event,
														OUT DWORD *alarm_sub_event);

__declspec(dllexport) BOOL WINAPI CONF_LANE_Get_Counter(IN DWORD plaza_number,
														IN DWORD lane_number,
														IN DWORD display_id,
														OUT BOOL counter[LANE_MAX_COUNTERS]);

__declspec(dllexport) BOOL WINAPI CONF_ALARM_Get_Shift_Mode_And_Vault_Events(OUT DWORD *pShiftAndModeEvtType,
																			OUT DWORD *pShiftEvtSubType,
																			OUT DWORD *pModeEvtSubType,
																			OUT DWORD *pVault1EvtType,
																			OUT DWORD *pVault2EvtType);

__declspec(dllexport) BOOL WINAPI CONF_ALARM_Get_First_Alarm(OUT DWORD *alarm_type,
															OUT DWORD *alarm_sub_type);

__declspec(dllexport) BOOL WINAPI CONF_ALARM_Get_Next_Alarm(OUT DWORD *alarm_type,
															OUT DWORD *alarm_sub_type);

__declspec(dllexport) BOOL WINAPI CONF_ALARM_Get_First_Event(IN DWORD alarm_type,
															IN DWORD alarm_sub_type,
															OUT DWORD *event_id,
															OUT DWORD *event_ack_mode,
															OUT DWORD *event_level,
															OUT CHAR event_wording[MSG_PV_MAX_ALARM_LABEL],
															OUT CHAR event_directive[MSG_PV_MAX_DETAIL_SIZE]);

__declspec(dllexport) BOOL WINAPI CONF_ALARM_Get_Next_Event(OUT DWORD *event_id,
															OUT DWORD *event_ack_mode,
															OUT DWORD *event_level,
															OUT CHAR event_wording[MSG_PV_MAX_ALARM_LABEL],
															OUT CHAR event_directive[MSG_PV_MAX_DETAIL_SIZE]);

__declspec(dllexport) BOOL WINAPI CONF_ALARM_Ack(struct MSG_PV_ALAR_REP *p_alar);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/