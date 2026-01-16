/******************* (v) 2017 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     route_msg_trs.c												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <plaza.h>
#include <msg_lc_transaction.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

/*|*/
/*****************************************************************************/
/*SYNTAX: BOOL ROUTE_Process_Msg_Transaction(struct MSG_TRANSACTION *p_trs)	 */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED BOOL ROUTE_Process_Msg_Transaction(struct MSG_TRANSACTION *p_trs)
{
    struct LANE_CNF								*p_lane_cnf			= NULL;
    struct MSG_TRANSACTION_Anomaly				*p_ano				= NULL;
	struct MSG_TRANSACTION_Anomaly_Description	*p_ano_desc			= NULL;
	char										szStatement[1024]	= {0};
	CHAR										szDesc[MSG_TRANSACTION_ANOMALY_DESCRIPTION_LENGTH] = { 0 };
	DWORD										SubTypeOfPayment	= 0;
    BOOL										bRet				= TRUE;

    // search for the lane according to the transaction
    p_lane_cnf = PLAZA_Search_Lane(p_trs->header.lane_number);
    if (p_lane_cnf == NULL)
        return FALSE;

    // To avoid to see the transactions on LSDU app. if the channel is disconnected or in closed or unknown state
    if (!LANE_Get_Connection_State(p_lane_cnf) || (LANE_Get_Shift(p_lane_cnf) == LANE_UNKNOWN_SHIFT_STATE)) 
        return TRUE;

	// get payment subtype
	if (PLAZA_Get_Payment_Sub_Type_Use())
		SubTypeOfPayment = p_trs->det_trans.sub_type_of_payment;
	else
		SubTypeOfPayment = 0;

	// filter for payment type
	if (PLAZA_Get_Payment_Filter())
		if (!PLAZA_Get_Payment (p_trs->det_trans.type_of_payment, SubTypeOfPayment, szStatement))
			return TRUE;

	// filter for vehicle class
	if (PLAZA_Get_Class_Filter())
		if (!PLAZA_Get_Class(p_trs->det_trans.transaction_class, szStatement))
			return TRUE;

	// get licence plate
	LANE_Set_Enl_Plate(p_lane_cnf, p_trs->entry_info);

	if (PLAZA_Get_Transaction_Date_Use() == TRUE)
		bRet &= LANE_Set_Trs_Body(p_lane_cnf,
								p_trs->header.toll_collector_id,
								p_trs->header.active_vault_id,
								&p_trs->body.time_of_transaction,
								p_trs->ref_trans.total_traffic, 
								p_trs->ref_trans.total_violation, 
								p_trs->det_trans.type_of_payment,
								SubTypeOfPayment,
								p_trs->det_trans.toll_fare,
								p_trs->det_trans.currency_id,
								p_trs->det_trans.exit_lic_plate);
	else
		bRet &= LANE_Set_Trs_Body(p_lane_cnf,
                               p_trs->header.toll_collector_id,
                               p_trs->header.active_vault_id,
                               &p_trs->ref_trans.start_time,
                               p_trs->ref_trans.total_traffic, 
                               p_trs->ref_trans.total_violation, 
                               p_trs->det_trans.type_of_payment,
							   SubTypeOfPayment,
                               p_trs->det_trans.toll_fare,
                               p_trs->det_trans.currency_id,
							   p_trs->det_trans.exit_lic_plate);

    // update classes in a predefined list of 3 elements: used, detected, keyed
    bRet &= LANE_Set_Trs_Classes(p_lane_cnf,
                                 p_trs->det_trans.transaction_class,
                                 p_trs->det_trans.detected_class,
                                 p_trs->det_trans.keyed_class);
	
	// management of transaction anomalies

    // deleting the list of the previous transaction anomalies
    LANE_Reset_Trs_Anomaly(p_lane_cnf);

    // creation of the new list
    p_ano = MSG_TRANSACTION_Get_First_Anomaly(p_trs->list_anomalies);
    while (p_ano != NULL)
    {
		memset(szDesc, 0, MSG_TRANSACTION_ANOMALY_DESCRIPTION_LENGTH);
		
		p_ano_desc = MSG_TRANSACTION_Get_First_Anomaly_Description(p_ano->list_anomaly_descritpion);
		
		if (p_ano_desc != NULL && p_ano_desc->description.type == MSG_FIELD_VARSTR)
			strcpy_s(szDesc, sizeof(szDesc), p_ano_desc->description.String);

        bRet &= LANE_Set_Trs_Anomaly(p_lane_cnf, p_ano->id, p_ano->value, szDesc);
        
        p_ano = MSG_TRANSACTION_Get_Next_Anomaly (p_trs->list_anomalies, p_ano);
    }

	if (PLAZA_Get_Transaction_Id_Use())
		bRet &= LANE_Set_Trs_Id(p_lane_cnf, p_trs->ref_trans.id);							

    return bRet;
}

/*-------------------------------- END OF FILE ------------------------------*/