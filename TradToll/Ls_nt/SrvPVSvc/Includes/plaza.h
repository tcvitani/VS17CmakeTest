#ifndef PLAZA_H
#define PLAZA_H

#include <windows.h>
#include <lane.h>
#include <msg_sv_con_rep.h>

#include <protect.h>

// INIT

PROTECTED BOOL PLAZA_Init(void);

// ACCESS

PROTECTED struct LANE_CNF * PLAZA_Search_Lane (DWORD lane_number);

PROTECTED DWORD PLAZA_Get_Number (void);

PROTECTED DWORD PLAZA_Count_Lanes (void);

PROTECTED BOOL PLAZA_Counter (DWORD disp, 
                              DWORD cnt,
                              struct MSG_PV_LANE_REP *p_lane);

PROTECTED struct MSG_PV_PLAZ_REP *PLAZA_Build_msg_pv_plz_rep (BOOL partial);

PROTECTED BOOL PLAZA_Delete_All (void);

PROTECTED BOOL PLAZA_Get_Currency (DWORD currency_id, CHAR label[MSG_PV_MAX_CURRENCY_LABEL]);

PROTECTED BOOL PLAZA_Get_Payment_Filter (void);

PROTECTED BOOL PLAZA_Get_Payment (DWORD payment_id, DWORD payment_sub_id, CHAR label[MSG_PV_MAX_PAYMENT_LABEL]);

PROTECTED BOOL PLAZA_Get_Payment_Sub_Type_Use (void);

PROTECTED BOOL PLAZA_Get_Transaction_Date_Use (void);

PROTECTED BOOL PLAZA_Get_Transaction_Id_Use (void);

PROTECTED BOOL PLAZA_Get_Class_Filter (void);

PROTECTED BOOL PLAZA_Get_Class (DWORD class_id, CHAR label[MSG_PV_MAX_CLASS_LABEL]);

PROTECTED BOOL PLAZA_Get_Anomaly (DWORD anomaly_id, CHAR label[MSG_PV_MAX_ANOMALY_LABEL]);

PROTECTED BOOL PLAZA_Get_Coin (IN DWORD coin_id,
                               OUT CHAR currency_label[MSG_PV_MAX_CURRENCY_LABEL], 
	                           OUT CHAR coin_label[MSG_PV_MAX_COIN_LABEL],
                               OUT DOUBLE *value,
                               OUT DWORD *bag);

PROTECTED BOOL PLAZA_Search_For_Manual_Lane(void);

PROTECTED BOOL PLAZA_Search_For_Busy_Lane(void);

PROTECTED struct LANE_CNF * PLAZA_Search_For_ActiveFree_Lane(void);

// MESSAGES DES VOIES

PROTECTED BOOL PLAZA_Disconnect_All_Lanes (ACOM_CNX_HANDLE hCnxHandle);

PROTECTED BOOL PLAZA_Reset_Traffic_Counters_For_All_Disconnected_Lanes (struct LANE_CNF *p_lane_cnf, BOOL reset);

PROTECTED BOOL PLAZA_Set_Lane_Connexion_State (struct MSG_SV_CON_REP *p_con, ACOM_CNX_HANDLE hCnxHandle);

// DATA BASE

PROTECTED BOOL PLAZA_Build_Conf_From_DB (void);

PROTECTED void GetLanesLinkStatus(CHAR *pszBuffer);

#endif