#ifndef ALARM_H
#define ALARM_H

#include <lane.h>
#include <msg_pv_ack_req.h>
#include <msg_pv_alar_rep.h>

#include <protect.h>

struct MSG_PV_ACK_REQ_REP
{
	struct MSG_PV_ACK_REQ ack_req;

	struct MSG_PV_ALAR_REP alar_rep;
};

PROTECTED BOOL ALARM_Set_Lane_Shift_And_Mode(struct LANE_CNF *p_lane_cnf,
	DWORD matricule,
	DWORD vault,
	DWORD type,
	DWORD sub_type,
	DWORD id);

PROTECTED BOOL ALARM_Set_Lane_Active_Vault(struct LANE_CNF *p_lane_cnf, DWORD vault, DWORD type);

PROTECTED BOOL ALARM_Search_And_Set_List(struct LANE_CNF *p_lane_cnf,
	DWORD type,
	DWORD sub_type,
	DWORD status,
	LPSYSTEMTIME time,
	CHAR *pDescription);

PROTECTED BOOL ALARM_Search_And_Ack(struct LANE_CNF *p_lane_cnf, struct MSG_PV_ACK_REQ *p_req);

PROTECTED BOOL ALARM_Search_And_Ack_Test(struct LANE_CNF *p_lane_cnf, struct MSG_PV_ACK_REQ *p_req);

PROTECTED BOOL ALARM_Ack_Test(struct LANE_CNF *p_lane_cnf, struct MSG_PV_ACK_REQ_REP *p_req_rep);

PROTECTED struct MSG_PV_ALAR_REP * ALARM_Build_msg_pv_alar_rep(struct LANE_CNF *p_lane_cnf, BOOL partial);

PROTECTED BOOL ALARM_Build_Conf_From_DB(DWORD plaza_number);

PROTECTED BOOL ALARM_Delete_All(void);

PROTECTED BOOL ALARM_Lane_Delete_All(struct LANE_CNF *p_lane_cnf);

PROTECTED BOOL TestAlarmAckType(void);

PROTECTED BOOL Delete_List(IN OUT HLIST *hList);

PROTECTED BOOL ACK_REQ_REP_Delete_All(struct MSG_PV_ACK_REQ_REP *p_req);

PROTECTED struct MSG_PV_ACK_REQ_REP * ACK_REQ_REP_Duplicate(struct MSG_PV_ACK_REQ *pAcqReq, struct MSG_PV_ALAR_REP *pAlarRep);

#endif