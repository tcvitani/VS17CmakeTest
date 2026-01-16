/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     pv_conf.c														 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <ntsvc.h>
#include <srvpv_main.h>
#include <conf_srvpv.h>
#include <pv_conf.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define PV_CONF_REG_VAL_DEFAULT_PLAZA_NUMBER	"DefaultPlazaNumber"
#define PLAZA_REG_VAL_DEFAULT_PLAZA_NAME		"DefaultPlazaName"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

struct PV_CNF
{
	struct MSG_PV_CONF_REP *msg_plz_cnf;
};

/*-------------------------------- VARIABLES: -------------------------------*/

PRIVATE struct PV_CNF PV_CNF = { 0 };

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PRIVATE BOOL PV_Get_Lane_Conf_From_DB(DWORD plazanum, HLIST *list_lane)
{
	struct MSG_PV_CONF_REP_Lane lane = { 0 };
	struct MSG_PV_CONF_REP_Lane *p_lane_cnf;
	DWORD dev_id;
	struct MSG_PV_CONF_REP_Equipment * p_dev_cnf;
	BOOL ret;

	// get first lane from database
	ret = CONF_PV_Get_First_Lane(plazanum, &lane.lanenum, lane.name, &lane.type);
	while (ret == TRUE)
	{
		// allocate memory for message
		p_lane_cnf = MSG_PV_CONF_REP_Lane_New(list_lane);
		if (p_lane_cnf == NULL)
			return FALSE;

		// recopy data
		*p_lane_cnf = lane;

		// get first lane equipment from  database
		ret = CONF_PV_Get_First_Lane_Device(plazanum, lane.lanenum, &dev_id);
		while (ret == TRUE)
		{
			// allocate memory for message
			p_dev_cnf = MSG_PV_CONF_REP_Equipment_New(&p_lane_cnf->list_equipment);
			if (p_dev_cnf == NULL)
				return FALSE;

			p_dev_cnf->id = dev_id;

			// get next lane equipment from  database
			ret = CONF_PV_Get_Next_Lane_Device(&dev_id);
		}

		// get next lane from database
		ret = CONF_PV_Get_Next_Lane(&lane.lanenum, lane.name, &lane.type);
	}

	return TRUE;
}

PRIVATE BOOL PV_Get_Disp_Conf_From_DB(DWORD plazanum, HLIST *list_display)
{
	DWORD disp_id;
	struct MSG_PV_CONF_REP_Display *p_disp_cnf;
	DWORD zone_id;
	struct MSG_PV_CONF_REP_Zone * p_zone_cnf;
	DWORD lane_number;
	struct MSG_PV_CONF_REP_Lanenum * p_lane_cnf;
	BOOL ret;

	// get first display from database
	ret = CONF_PV_Get_First_Display(plazanum, &disp_id);
	while (ret == TRUE)
	{
		// allocate memory for message
		p_disp_cnf = MSG_PV_CONF_REP_Display_New(list_display);
		if (p_disp_cnf == NULL)
			return FALSE;

		p_disp_cnf->id = disp_id;

		// get first display zone from database
		ret = CONF_PV_Get_First_Display_Zone(plazanum, disp_id, &zone_id);
		while (ret == TRUE)
		{
			// allocate memory for messsage
			p_zone_cnf = MSG_PV_CONF_REP_Zone_New(&p_disp_cnf->list_zone);
			if (p_zone_cnf == NULL)
				return FALSE;

			p_zone_cnf->id = zone_id;

			// get first display zone lane from database
			ret = CONF_PV_Get_First_Display_Zone_Lane(plazanum, disp_id, zone_id, &lane_number);
			while (ret == TRUE)
			{
				// allocate memory for messsage
				p_lane_cnf = MSG_PV_CONF_REP_Lanenum_New(&p_zone_cnf->list_lanenum);
				if (p_lane_cnf == NULL)
					return FALSE;

				p_lane_cnf->id = lane_number;

				// get next display zone lane from database
				ret = CONF_PV_Get_Next_Display_Zone_Lane(&lane_number);
			}

			// get next display zone lane from database
			ret = CONF_PV_Get_Next_Display_Zone(&zone_id);
		}

		// get next display from database
		ret = CONF_PV_Get_Next_Display(&disp_id);
	}

	return TRUE;
}

PROTECTED BOOL PV_Build_Msg_Conf_From_DB(void)
{
	struct MSG_PV_CONF_REP *p_plz_cnf						= NULL;
	NTSVC_PARAMETER_DEF		*psParams						= NULL;
	DWORD					dwErr							= 0;
	DWORD					dwDefaultPlazaNumber			= 0;
	CHAR					szDefaultPlazaName[MAX_PATH]	= { 0 };

	// registry parameters
	psParams = NTSVCOpenParameters(
		PV_CONF_REG_VAL_DEFAULT_PLAZA_NUMBER,	REG_DWORD,	4,			0,		&dwDefaultPlazaNumber,
		PLAZA_REG_VAL_DEFAULT_PLAZA_NAME,		REG_SZ,		MAX_PATH,	" ",	szDefaultPlazaName,
		NULL);

	if (psParams == NULL)
		return FALSE;

	if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
		return FALSE;

	NTSVCCloseParameters(psParams);

	// allocate memory for message
	p_plz_cnf = MSG_PV_CONF_REP_New();
	if (p_plz_cnf == NULL)
		return FALSE;

	// get message id
	p_plz_cnf->header.id = SVC_Get_Msg_Id();

	// get first plaza number from database
	if (!CONF_PV_Get_First_Plaza(&p_plz_cnf->body.plazanum, p_plz_cnf->body.name, dwDefaultPlazaNumber))
		return FALSE;

	if (dwDefaultPlazaNumber != 0)
		strncpy_s(p_plz_cnf->body.name, sizeof(p_plz_cnf->body.name), szDefaultPlazaName, sizeof(p_plz_cnf->body.name) - 1);

	// get lane configuration from database
	if (!PV_Get_Lane_Conf_From_DB(p_plz_cnf->body.plazanum, &p_plz_cnf->list_lane))
	{
		MSG_PV_CONF_REP_Delete_All(p_plz_cnf);
		return FALSE;
	}

	// get display configuration from database
	if (!PV_Get_Disp_Conf_From_DB(p_plz_cnf->body.plazanum, &p_plz_cnf->list_display))
	{
		MSG_PV_CONF_REP_Delete_All(p_plz_cnf);
		return FALSE;
	}

	// always returns FALSE but allows to terminate the connection with the database
	CONF_PV_Get_Next_Plaza(&p_plz_cnf->body.plazanum, p_plz_cnf->body.name, dwDefaultPlazaNumber);

	PV_CNF.msg_plz_cnf = p_plz_cnf;

	return TRUE;
}

PROTECTED struct MSG_PV_CONF_REP *PV_Build_msg_pv_conf_rep(void)
{
	// "deep copy"
	return MSG_Duplicate(PV_CNF.msg_plz_cnf);
}

PROTECTED BOOL PV_Delete_Conf(void)
{
	// delete plaza configuration message
	return MSG_PV_CONF_REP_Delete_All(PV_CNF.msg_plz_cnf);
}

/*-------------------------------- END OF FILE ------------------------------*/