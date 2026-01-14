/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:      MSG_PV_LANE_REP.H                                              */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MSG_PV_LANE_REP_H
#define MSG_PV_LANE_REP_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <csr_msg.h>

#include <msg_pv_define.h>
#include <msg_pv_header.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#ifdef MSG_PV_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MSG_PV_LANE_REP_CD 212L

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct MSG_PV_LANE_REP_Equipment
{
    DWORD	id;                     // 0-99
	DWORD	state;					// 0-99
};

struct MSG_PV_LANE_REP_Class
{
    DWORD	value;     // 0-99         
};

struct MSG_PV_LANE_REP_Anomaly
{
    DWORD	id;	// 0-9
	CHAR	label[MSG_PV_MAX_ANOMALY_LABEL];
	CHAR	description[MSG_PV_MAX_ALARM_DESCRIPTION];
};

struct MSG_PV_LANE_REP_Other
{
    DWORD	id;	// 0-9
	CHAR	label[MSG_PV_MAX_OTHER_LABEL];
};

struct MSG_PV_LANE_REP
{
    // Message Header
    struct MSG_PV_HEADER header;
    
	struct MSG_PV_LANE_REP_Body
    {
		DWORD		lanenum;	//0-9999
		SYSTEMTIME	date;	//YYYYMMDDHHMMSS
		DWORD		connection_state;	//0-9
		DWORD		lane_state;		//0-9
		DWORD		lane_mode;		//0-99
		DWORD		daily_traffic;	//0-9999 9999
		DWORD		daily_violation;	//0-9999 9999
		DOUBLE		tollfare;		// 9 999 999.99
		CHAR		currency[MSG_PV_MAX_CURRENCY_LABEL];
		CHAR		payment_type[MSG_PV_MAX_PAYMENT_LABEL];
		DWORD		collector_id;	//0-999 999
		DWORD		active_vault;	//0-999 999
		CHAR        exl_licence_plate[MSG_PV_MAX_LANE_LIC_PLATE_LEN];
		CHAR        enl_licence_plate[MSG_PV_MAX_LANE_LIC_PLATE_LEN];
    } 
    body;

    // Equipment detail
    HLIST	list_equipment;

	//Class detail
	HLIST	list_class;

	//Anomaly dteil
	HLIST	list_anomaly;

	//Other
	HLIST	list_other;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT struct MSG_PV_LANE_REP * WINAPI MSG_PV_LANE_REP_New(void);

EXPORT struct MSG_PV_LANE_REP_Equipment * WINAPI MSG_PV_LANE_REP_Equipment_New(HLIST *hList);

EXPORT struct MSG_PV_LANE_REP_Equipment * WINAPI MSG_PV_LANE_REP_Get_First_Equipment(HLIST hList);

EXPORT struct MSG_PV_LANE_REP_Equipment * WINAPI MSG_PV_LANE_REP_Get_Next_Equipment(HLIST hList, 
																					struct MSG_PV_LANE_REP_Equipment *p_ptr);

EXPORT struct MSG_PV_LANE_REP_Class * WINAPI MSG_PV_LANE_REP_Class_New(HLIST *hList);

EXPORT struct MSG_PV_LANE_REP_Class * WINAPI MSG_PV_LANE_REP_Get_First_Class(HLIST hList);

EXPORT struct MSG_PV_LANE_REP_Class * WINAPI MSG_PV_LANE_REP_Get_Next_Class(HLIST hList, 
																			struct MSG_PV_LANE_REP_Class *p_ptr);

EXPORT struct MSG_PV_LANE_REP_Anomaly * WINAPI MSG_PV_LANE_REP_Anomaly_New(HLIST *hList);

EXPORT struct MSG_PV_LANE_REP_Anomaly * WINAPI MSG_PV_LANE_REP_Get_First_Anomaly(HLIST hList);

EXPORT struct MSG_PV_LANE_REP_Anomaly * WINAPI MSG_PV_LANE_REP_Get_Next_Anomaly(HLIST hList, 
																				struct MSG_PV_LANE_REP_Anomaly *p_ptr);

EXPORT struct MSG_PV_LANE_REP_Other * WINAPI MSG_PV_LANE_REP_Other_New(HLIST *hList);

EXPORT struct MSG_PV_LANE_REP_Other * WINAPI MSG_PV_LANE_REP_Get_First_Other(HLIST hList);

EXPORT struct MSG_PV_LANE_REP_Other * WINAPI MSG_PV_LANE_REP_Get_Next_Other(HLIST hList, 
																			struct MSG_PV_LANE_REP_Other *p_ptr);

EXPORT BOOL WINAPI MSG_PV_LANE_REP_Delete_All(struct MSG_PV_LANE_REP *p_ptr);

EXPORT BOOL WINAPI MSG_PV_LANE_REP_Write(struct MSG_PV_LANE_REP *p_ptr, 
										 BYTE *p_msg, 
										 DWORD msg_size_max, 
										 DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_LANE_REP_Read(struct MSG_PV_LANE_REP *p_ptr, 
										BYTE *p_msg, 
										DWORD msg_size_max, 
										DWORD *final_msg_size);

EXPORT BOOL WINAPI MSG_PV_LANE_REP_New_Record(HLIST *hList);

/*-------------------------------- VARIABLES: -------------------------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_PV_LANE_REP_H */

/*-------------------------------- END OF FILE ------------------------------*/