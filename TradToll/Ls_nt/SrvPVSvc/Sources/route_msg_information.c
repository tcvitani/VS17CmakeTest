/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     route_msg_information.c										 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <stdio.h>
#include <plaza.h>
#include <msg_lc_comp_inf_tr.h>
#include <route_msg_information.h>
#include <ntsvc.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/

#define CLASS_MAX_SIZE		3

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

PROTECTED BOOL ROUTE_Process_Msg_Information(struct MSG_COMP_INF_TR *p_info)
{
	struct LANE_CNF						*p_lane_cnf			= NULL;
	struct MSG_COMP_INF_TR_Info_Type	*p_lc_type			= NULL;
	struct MSG_COMP_INF_TR_Elem_Info	*p_lc_info			= NULL;
	char								szStatement[1024]	= { 0 };
	DWORD								Class				= 0;
	BOOL								bRet				= TRUE;

	// search for the channel concerned by the transaction
	p_lane_cnf = PLAZA_Search_Lane(p_info->header.lane_number);
	if (p_lane_cnf == NULL)
		return FALSE;

	p_lc_type = MSG_COMP_INF_TR_Get_First_Info_Type(p_info->list_comp_inf);
	while (p_lc_type != NULL)
	{
		p_lc_info = MSG_COMP_INF_TR_Get_First_Elem_Info(p_lc_type->list_info_type);
		while (p_lc_info != NULL)
		{
			if (p_lc_type->type == CMPL_INFO_MSG_TYPE_VEHICLE)
			{
				if (p_lc_info->code == CMPL_INFO_MSG_CODE_CLS_MOTO_A || p_lc_info->code == CMPL_INFO_MSG_CODE_CLS_MOTO_B)
				{
					LANE_Set_List_Other_Element(p_lane_cnf, p_lc_type->type, p_lc_info);
				}
				else if (p_lc_info->info_value.type == MSG_FIELD_VARSTR)
				{
					if (p_lc_info->code == CMPL_INFO_MSG_CODE_CLS_ON_FLY_STYPE)
					{
						memcpy(szStatement, &p_lc_info->info_value.String, CLASS_MAX_SIZE);
						szStatement[CLASS_MAX_SIZE] = '\0';
						Class = atoi(szStatement);
						bRet = LANE_Set_Comp_Classes(p_lane_cnf, Class, CLASS_ONTHEFLY1_SUBTYPE);

						memcpy(szStatement, &p_lc_info->info_value.String[CLASS_MAX_SIZE], CLASS_MAX_SIZE);
						szStatement[CLASS_MAX_SIZE] = '\0';
						Class = atoi(szStatement);
						bRet = LANE_Set_Comp_Classes(p_lane_cnf, Class, CLASS_ONTHEFLY2_SUBTYPE);
					}
					else if (p_lc_info->code == CMPL_INFO_MSG_CODE_CLS_RESET_STYPE)
					{
						memcpy(szStatement, &p_lc_info->info_value.String, CLASS_MAX_SIZE);
						szStatement[CLASS_MAX_SIZE] = '\0';
						Class = atoi(szStatement);
						bRet = LANE_Set_Comp_Classes(p_lane_cnf, Class, CLASS_RESET1_SUBTYPE);

						memcpy(szStatement, &p_lc_info->info_value.String[CLASS_MAX_SIZE], CLASS_MAX_SIZE);
						szStatement[CLASS_MAX_SIZE] = '\0';
						Class = atoi(szStatement);
						bRet = LANE_Set_Comp_Classes(p_lane_cnf, Class, CLASS_RESET2_SUBTYPE);

						LANE_Set_List_Other_Element(p_lane_cnf, p_lc_type->type, p_lc_info);
					}
					else if (p_lc_info->code == CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE)
					{
						memcpy(szStatement, &p_lc_info->info_value.String, CLASS_MAX_SIZE);
						szStatement[CLASS_MAX_SIZE] = '\0';
						Class = atoi(szStatement);
						bRet = LANE_Set_Comp_Classes(p_lane_cnf, Class, CMPL_INFO_MSG_CODE_CLS_KEYIN_STYPE);

						LANE_Set_List_Other_Element(p_lane_cnf, p_lc_type->type, p_lc_info);
					}
				}
			}
			else if (p_lc_type->type == CMPL_INFO_MSG_TYPE_PV)
			{
				if (p_lc_info->code == CMPL_INFO_MSG_CODE_PV_FARE)
				{
					memset(p_lane_cnf->msg_lane->body.currency, 0x00, MSG_PV_MAX_CURRENCY_LABEL);
					PLAZA_Get_Currency(1, p_lane_cnf->msg_lane->body.currency);

					LANE_Set_List_Other_Element(p_lane_cnf, p_lc_type->type, p_lc_info);
				}
				else if (p_lc_info->code == CMPL_INFO_MSG_CODE_PV_PAYMENT_TYPE)
				{
					char   szTmp[MAX_PATH]	= { 0 };
					DWORD  dwPayType		= 0;
					DWORD  dwPaySubType		= 0;
					DWORD  dwPayTypeCorr	= 0;
					DWORD  dwPaySubTypeCorr = 0;

					memcpy(szTmp, p_lc_info->info_value.String, 2);
					dwPayType = atol(szTmp);

					ZeroMemory(szTmp, sizeof(szTmp));
					memcpy(szTmp, &p_lc_info->info_value.String[2], 2);
					dwPaySubType = atol(szTmp);

					ZeroMemory(szTmp, sizeof(szTmp));
					memcpy(szTmp, &p_lc_info->info_value.String[4], 2);
					dwPayTypeCorr = atol(szTmp);

					ZeroMemory(szTmp, sizeof(szTmp));
					memcpy(szTmp, &p_lc_info->info_value.String[6], 2);
					dwPaySubTypeCorr = atol(szTmp);
			
					memset(p_lane_cnf->msg_lane->body.payment_type, 0x00, MSG_PV_MAX_PAYMENT_LABEL);
					PLAZA_Get_Payment(dwPayType, dwPaySubType, p_lane_cnf->msg_lane->body.payment_type);

					memset(p_lc_info->info_value.String, 0, sizeof(p_lc_info->info_value.String));
					p_lc_info->info_value.type = MSG_FIELD_VARSTR;

					if (dwPayTypeCorr != 0)
					{
						ZeroMemory(szTmp, sizeof(szTmp));
						PLAZA_Get_Payment(dwPayTypeCorr, dwPaySubTypeCorr, szTmp);

						sprintf_s(p_lc_info->info_value.String,
								sizeof(p_lc_info->info_value.String),
								"%02d%s%02d%s",
								strlen(p_lane_cnf->msg_lane->body.payment_type),
								p_lane_cnf->msg_lane->body.payment_type,
								strlen(szTmp),
								szTmp);
					}
					else
					{
						sprintf_s(p_lc_info->info_value.String,
								sizeof(p_lc_info->info_value.String),
								"%02d%s%02d%s",
								strlen(p_lane_cnf->msg_lane->body.payment_type),
								p_lane_cnf->msg_lane->body.payment_type,
								0,
								"");
					}

					LANE_Set_List_Other_Element(p_lane_cnf, p_lc_type->type, p_lc_info);
				}
				else if (p_lc_info->code == CMPL_INFO_MSG_CODE_PV_CLASS || 
						p_lc_info->code == CMPL_INFO_MSG_CODE_PV_TRAFFIC_LIGHT)
				{
					LANE_Set_List_Other_Element(p_lane_cnf, p_lc_type->type, p_lc_info);
				}
			}
			else if (p_lc_type->type == CMPL_INFO_MSG_TYPE_FARE_INFO)
			{
				if (p_lc_info->code == CMPL_INFO_MSG_CODE_FARE_AMOUNT_PAID)
					LANE_Set_List_Other_Element(p_lane_cnf, p_lc_type->type, p_lc_info);
			}

			p_lc_info = MSG_COMP_INF_TR_Get_Next_Elem_Info(p_lc_type->list_info_type, p_lc_info);
		}

		p_lc_type = MSG_COMP_INF_TR_Get_Next_Info_Type(p_info->list_comp_inf, p_lc_type);
	}

	return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/