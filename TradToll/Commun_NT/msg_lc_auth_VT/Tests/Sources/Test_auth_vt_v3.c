// Test for Backup by Date Message
#include <csr_msg.h>
#include <msg_lc_header.h>

#include <msg_lc_auth_vt_enl_req_v3.h>
#include <msg_lc_auth_vt_enl_rep_v3.h>
#include <msg_lc_auth_vt_exl_req_v3.h>
#include <msg_lc_auth_vt_exl_rep_v3.h>

#include <stdio.h>
#include <conio.h>


void Test_Auth_Vt_enl_req_v3(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_HEADER *p_header;

	struct MSG_LC_AUTH_VT_ENL_REQ_V3 * psAuthEnlReq = NULL;
	struct MSG_LC_AUTH_VT_ENL_REQ_V3 * psAuthEnlReq_r = NULL;

	printf("\n Test_Auth_Vt_enl_req_v3: Test MSG_LC_AUTH_VT_ENL_REQ_V3!");

	psAuthEnlReq = MSG_LC_AUTH_VT_ENL_REQ_V3_New();

	psAuthEnlReq->header.plaza_number = 111;
	psAuthEnlReq->header.lane_number = 1101;
	psAuthEnlReq->body.entry_trs_ref_num = 1;


	GetLocalTime (&systime);
	psAuthEnlReq->body.dte_ref_entry = systime;
	psAuthEnlReq->body.dte_entry = systime;
  
	psAuthEnlReq->body.entry_plaza_id = 1;
	psAuthEnlReq->body.entry_lane_id = 1;

	strcpy_s(psAuthEnlReq->body.entry_trs_id, sizeof(psAuthEnlReq->body.entry_trs_id), "000100001000005000020004");

	strcpy_s(psAuthEnlReq->body.entry_vrn, sizeof(psAuthEnlReq->body.entry_vrn), "ABCD2");
	strcpy_s(psAuthEnlReq->body.entry_vrn_country, sizeof(psAuthEnlReq->body.entry_vrn_country), "MNE");
	strcpy_s(psAuthEnlReq->body.entry_provider, sizeof(psAuthEnlReq->body.entry_provider), "999");

	psAuthEnlReq->body.flag_delete = 0;
	psAuthEnlReq->body.flag_no_insert = 0;
	psAuthEnlReq->body.request_id = 12;

	if (MSG_LC_AUTH_VT_ENL_REQ_V3_Write(psAuthEnlReq, string_w, sizeof(string_w), &size) == TRUE)
	{
		string_w[size] = '\0';
		printf("\nBuffer w = %s", string_w);
	}
	else
		printf("\nWrite Error !");


	psAuthEnlReq_r = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (psAuthEnlReq_r != NULL)
    {
		if (MSG_LC_AUTH_VT_ENL_REQ_V3_Write(psAuthEnlReq_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nRead Error !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

	MSG_LC_AUTH_VT_ENL_REQ_V3_Delete_All(psAuthEnlReq_r);
	MSG_LC_AUTH_VT_ENL_REQ_V3_Delete_All(psAuthEnlReq);


    p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD of message = %ld\n" , p_header->cd);
        switch (p_header->cd)
        {
 
		case MSG_LC_AUTH_VT_ENL_REQ_V3_CD:
			psAuthEnlReq_r = (struct MSG_LC_AUTH_VT_ENL_REQ_V3 *) p_header;

			printf("\n entry_trs_ref_num = %s", psAuthEnlReq_r->body.entry_trs_ref_num == psAuthEnlReq->body.entry_trs_ref_num ? "YES" : "NO");
			printf("\n entry_plaza_id = %s", psAuthEnlReq_r->body.entry_plaza_id == psAuthEnlReq->body.entry_plaza_id ? "YES" : "NO");
			printf("\n entry_lane_id = %s", psAuthEnlReq_r->body.entry_lane_id == psAuthEnlReq->body.entry_lane_id ? "YES" : "NO");
			printf("\n entry_trs_id = %s", strcmp(psAuthEnlReq_r->body.entry_trs_id, psAuthEnlReq_r->body.entry_trs_id) == 0 ? "YES" : "NO");
			printf("\n entry_vrn = %s", strcmp(psAuthEnlReq_r->body.entry_vrn, psAuthEnlReq_r->body.entry_vrn) == 0 ? "YES" : "NO");
			printf("\n entry_vrn_country = %s", strcmp(psAuthEnlReq_r->body.entry_vrn_country, psAuthEnlReq_r->body.entry_vrn_country) == 0 ? "YES" : "NO");
			printf("\n entry_provider = %s", strcmp(psAuthEnlReq_r->body.entry_provider, psAuthEnlReq_r->body.entry_provider) == 0 ? "YES" : "NO");
			printf("\n ticket_type = %s", psAuthEnlReq_r->body.ticket_type == psAuthEnlReq->body.ticket_type ? "YES" : "NO");
			printf("\n flag_delete = %s", psAuthEnlReq_r->body.flag_delete == psAuthEnlReq->body.flag_delete ? "YES" : "NO");
			printf("\n flag_no_insert = %s", psAuthEnlReq_r->body.flag_no_insert == psAuthEnlReq->body.flag_no_insert ? "YES" : "NO");
			printf("\n request_id = %s \n", psAuthEnlReq_r->body.request_id == psAuthEnlReq->body.request_id ? "YES" : "NO");
			printf("\n");
			MSG_LC_AUTH_VT_ENL_REQ_V3_Delete_All(psAuthEnlReq_r);            
			// TO DO...
            break;
        default:
            // INCONNU...
            break;
        }
    }
    else
        printf ("\nCD unknown !");



}


void Test_Auth_Vt_enl_rep_v3(void)
{
	SYSTEMTIME systime;
	CHAR string_w[5000] = { 0 };
	CHAR string_r[5000] = { 0 };
	CHAR string_t[5000] = { 0 };
	DWORD size;
	struct MSG_HEADER *p_header;
	struct MSG_LC_AUTH_VT_ENL_REP_V3 * psAuthEnlRep = NULL;
	struct MSG_LC_AUTH_VT_ENL_REP_V3 * psAuthEnlRep_r = NULL;

	printf("\n Test_Auth_Vt_enl_rep_v3: Test MSG_LC_AUTH_VT_ENL_REP_V3!");

	psAuthEnlRep = MSG_LC_AUTH_VT_ENL_REP_V3_New();

	psAuthEnlRep->header.plaza_number = 111;
	psAuthEnlRep->header.lane_number = 1101;
	psAuthEnlRep->body.entry_trs_ref_num = 1;

	GetLocalTime(&systime);
	psAuthEnlRep->body.dte_ref_entry = systime;
	psAuthEnlRep->body.dte_entry = systime;

	psAuthEnlRep->body.entry_plaza_id = 1;
	psAuthEnlRep->body.entry_lane_id = 1;

	strcpy_s(psAuthEnlRep->body.entry_trs_id, sizeof(psAuthEnlRep->body.entry_trs_id), "000100001000005000020004");

	strcpy_s(psAuthEnlRep->body.entry_vrn, sizeof(psAuthEnlRep->body.entry_vrn), "ABCD2");
	strcpy_s(psAuthEnlRep->body.entry_vrn_country, sizeof(psAuthEnlRep->body.entry_vrn_country), "MNE");
	strcpy_s(psAuthEnlRep->body.entry_provider, sizeof(psAuthEnlRep->body.entry_provider), "AOS");
	strcpy_s(psAuthEnlRep->body.accepted_days, sizeof(psAuthEnlRep->body.accepted_days), "FF");
	strcpy_s(psAuthEnlRep->body.account_balance, sizeof(psAuthEnlRep->body.account_balance), "-12012");
	strcpy_s(psAuthEnlRep->body.user_balance, sizeof(psAuthEnlRep->body.user_balance), "-12012");


	psAuthEnlRep->body.flag_result = 1;
	psAuthEnlRep->body.reason_id = 1;
	psAuthEnlRep->body.flag_delete = 0;
	psAuthEnlRep->body.flag_no_insert = 0;
	psAuthEnlRep->body.request_id = 123;
	
	if (MSG_LC_AUTH_VT_ENL_REP_V3_Write(psAuthEnlRep, string_w, sizeof(string_w), &size) == TRUE)
	{
		string_w[size] = '\0';
		printf("\nBuffer w = %s", string_w);
	}
	else
		printf("\nWrite Error !");


	psAuthEnlRep_r = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (psAuthEnlRep_r != NULL)
	{
		if (MSG_LC_AUTH_VT_ENL_REP_V3_Write(psAuthEnlRep_r, string_r, sizeof(string_r), &size) == TRUE)
		{
			string_r[size] = '\0';
			printf("\nBuffer r = %s", string_r);
		}
		else
			printf("\nRead Error !");
	}

	printf("\nBuffer w = buffer r : %s", strcmp(string_w, string_r) == 0 ? "YES" : "NO");

	MSG_LC_AUTH_VT_ENL_REP_V3_Delete_All(psAuthEnlRep_r);
	MSG_LC_AUTH_VT_ENL_REP_V3_Delete_All(psAuthEnlRep);


	p_header = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (p_header != NULL)
	{
		printf("\nCD of message = %ld\n", p_header->cd);
		switch (p_header->cd)
		{

		case MSG_LC_AUTH_VT_ENL_REP_V3_CD:
			psAuthEnlRep_r = (struct MSG_LC_AUTH_VT_ENL_REP_V3 *) p_header;

			printf("\n entry_trs_ref_num = %s", psAuthEnlRep_r->body.entry_trs_ref_num == psAuthEnlRep->body.entry_trs_ref_num ? "YES" : "NO");
			printf("\n entry_plaza_id = %s", psAuthEnlRep_r->body.entry_plaza_id == psAuthEnlRep->body.entry_plaza_id ? "YES" : "NO");
			printf("\n entry_lane_id = %s", psAuthEnlRep_r->body.entry_lane_id == psAuthEnlRep->body.entry_lane_id ? "YES" : "NO");
			printf("\n entry_trs_id = %s", strcmp(psAuthEnlRep_r->body.entry_trs_id, psAuthEnlRep->body.entry_trs_id) == 0 ? "YES" : "NO");
			printf("\n entry_vrn = %s", strcmp(psAuthEnlRep_r->body.entry_vrn, psAuthEnlRep->body.entry_vrn) == 0 ? "YES" : "NO");
			printf("\n entry_vrn_country = %s", strcmp(psAuthEnlRep_r->body.entry_vrn_country, psAuthEnlRep->body.entry_vrn_country) == 0 ? "YES" : "NO");
			printf("\n entry_provider = %s", strcmp(psAuthEnlRep_r->body.entry_provider, psAuthEnlRep->body.entry_provider) == 0 ? "YES" : "NO");
			printf("\n flag_result = %s", psAuthEnlRep_r->body.flag_result == psAuthEnlRep->body.flag_result ? "YES" : "NO");
			printf("\n reason_id = %s", psAuthEnlRep_r->body.reason_id == psAuthEnlRep->body.reason_id ? "YES" : "NO");
			printf("\n ticket_type = %s", psAuthEnlRep_r->body.ticket_type == psAuthEnlRep->body.ticket_type ? "YES" : "NO");
			printf("\n flag_delete = %s", psAuthEnlRep_r->body.flag_delete == psAuthEnlRep->body.flag_delete ? "YES" : "NO");
			printf("\n flag_no_insert = %s", psAuthEnlRep_r->body.flag_no_insert == psAuthEnlRep->body.flag_no_insert ? "YES" : "NO");
			printf("\n vehicle_class = %s", psAuthEnlRep_r->body.vehicle_class == psAuthEnlRep->body.vehicle_class ? "YES" : "NO");
			printf("\n payment_subtype = %s", psAuthEnlRep_r->body.payment_subtype == psAuthEnlRep->body.payment_subtype ? "YES" : "NO");
			printf("\n accepted_days = %s", strcmp(psAuthEnlRep_r->body.accepted_days, psAuthEnlRep->body.accepted_days) == 0 ? "YES" : "NO");
			printf("\n account_balance = %s", strcmp(psAuthEnlRep_r->body.account_balance,psAuthEnlRep->body.account_balance)==0 ? "YES" : "NO");
			printf("\n user_balance = %s", strcmp(psAuthEnlRep_r->body.user_balance, psAuthEnlRep->body.user_balance) == 0 ? "YES" : "NO");
			printf("\n contract_type = %s", psAuthEnlRep_r->body.contract_type == psAuthEnlRep->body.contract_type ? "YES" : "NO");
			printf("\n request_id = %s \n", psAuthEnlRep_r->body.request_id == psAuthEnlRep->body.request_id ? "YES" : "NO");
			printf("\n");
			MSG_LC_AUTH_VT_ENL_REP_V3_Delete_All(psAuthEnlRep_r);			
			break;
		default:
			// INCONNU...
			break;
		}
	}
	else
		printf("\nCD unknown !");




}



void Test_Auth_Vt_exl_req_v3(void)
{
	SYSTEMTIME systime;
	CHAR string_w[5000] = { 0 };
	CHAR string_r[5000] = { 0 };
	CHAR string_t[5000] = { 0 };
	DWORD size;
	struct MSG_HEADER *p_header;

	struct MSG_LC_AUTH_VT_EXL_REQ_V3 * psAuthExlReq = NULL;
	struct MSG_LC_AUTH_VT_EXL_REQ_V3 * psAuthExlReq_r = NULL;

	printf("\n Test_Auth_Vt_exl_req3: Test MSG_LC_AUTH_VT_EXL_REQ_V3!");

	psAuthExlReq = MSG_LC_AUTH_VT_EXL_REQ_V3_New();

	psAuthExlReq->header.plaza_number = 111;
	psAuthExlReq->header.lane_number = 1101;
	psAuthExlReq->body.exit_trs_ref_num = 1;


	GetLocalTime(&systime);
	psAuthExlReq->body.dte_ref_exit = systime;
	psAuthExlReq->body.dte_exit = systime;

	psAuthExlReq->body.exit_plaza_id = 1;
	psAuthExlReq->body.exit_lane_id = 1;

	strcpy_s(psAuthExlReq->body.exit_trs_id, sizeof(psAuthExlReq->body.exit_trs_id), "000100001000005000020004");

	strcpy_s(psAuthExlReq->body.exit_vrn, sizeof(psAuthExlReq->body.exit_vrn), "ABCD2");
	strcpy_s(psAuthExlReq->body.exit_vrn_country, sizeof(psAuthExlReq->body.exit_vrn_country), "MNE");
	strcpy_s(psAuthExlReq->body.exit_provider, sizeof(psAuthExlReq->body.exit_provider), "AOS");

	psAuthExlReq->body.flag_delete = 1;
	psAuthExlReq->body.flag_no_update = 0;
	psAuthExlReq->body.flag_open_system = 0;
	psAuthExlReq->body.request_id = 13;

	if (MSG_LC_AUTH_VT_EXL_REQ_V3_Write(psAuthExlReq, string_w, sizeof(string_w), &size) == TRUE)
	{
		string_w[size] = '\0';
		printf("\nBuffer w = %s", string_w);
	}
	else
		printf("\nWrite Error !");


	psAuthExlReq_r = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (psAuthExlReq_r != NULL)
	{
		if (MSG_LC_AUTH_VT_EXL_REQ_V3_Write(psAuthExlReq_r, string_r, sizeof(string_r), &size) == TRUE)
		{
			string_r[size] = '\0';
			printf("\nBuffer r = %s", string_r);
		}
		else
			printf("\nRead Error !");
	}

	printf("\nBuffer w = buffer r : %s", strcmp(string_w, string_r) == 0 ? "YES" : "NO");

	MSG_LC_AUTH_VT_EXL_REQ_V3_Delete_All(psAuthExlReq_r);
	MSG_LC_AUTH_VT_EXL_REQ_V3_Delete_All(psAuthExlReq);


	p_header = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (p_header != NULL)
	{
		printf("\nCD of message = %ld\n", p_header->cd);
		switch (p_header->cd)
		{

		case MSG_LC_AUTH_VT_EXL_REQ_V3_CD:
			psAuthExlReq_r = (struct MSG_LC_AUTH_VT_EXL_REQ_V3 *) p_header;

			printf("\n exit_trs_ref_num = %s", psAuthExlReq_r->body.exit_trs_ref_num == psAuthExlReq->body.exit_trs_ref_num ? "YES" : "NO");
			printf("\n exit_plaza_id = %s", psAuthExlReq_r->body.exit_plaza_id == psAuthExlReq->body.exit_plaza_id ? "YES" : "NO");
			printf("\n exit_lane_id = %s", psAuthExlReq_r->body.exit_lane_id == psAuthExlReq->body.exit_lane_id ? "YES" : "NO");
			printf("\n exit_trs_id = %s", strcmp(psAuthExlReq_r->body.exit_trs_id, psAuthExlReq->body.exit_trs_id) == 0 ? "YES" : "NO");
			printf("\n exit_vrn = %s", strcmp(psAuthExlReq_r->body.exit_vrn, psAuthExlReq->body.exit_vrn) == 0 ? "YES" : "NO");
			printf("\n exit_vrn_country = %s", strcmp(psAuthExlReq_r->body.exit_vrn_country, psAuthExlReq->body.exit_vrn_country) == 0 ? "YES" : "NO");
			printf("\n exit_provider = %s", strcmp(psAuthExlReq_r->body.exit_provider, psAuthExlReq->body.exit_provider) == 0 ? "YES" : "NO");
			printf("\n flag_eticket = %s", psAuthExlReq_r->body.flag_eticket == psAuthExlReq->body.flag_eticket ? "YES" : "NO");
			printf("\n ticket_type = %s", psAuthExlReq_r->body.ticket_type == psAuthExlReq->body.ticket_type ? "YES" : "NO");
			printf("\n flag_delete = %s", psAuthExlReq_r->body.flag_delete == psAuthExlReq->body.flag_delete ? "YES" : "NO");
			printf("\n flag_open_system = %s", psAuthExlReq_r->body.flag_open_system == psAuthExlReq->body.flag_open_system ? "YES" : "NO");
			printf("\n flag_no_update = %s", psAuthExlReq_r->body.flag_no_update == psAuthExlReq->body.flag_no_update ? "YES" : "NO");
			printf("\n request_id = %s \n", psAuthExlReq_r->body.request_id == psAuthExlReq->body.request_id ? "YES" : "NO");
			printf("\n");
			MSG_LC_AUTH_VT_EXL_REQ_V3_Delete_All(psAuthExlReq_r);
			// TO DO...
			break;
		default:
			// INCONNU...
			break;
		}
	}
	else
		printf("\nCD unknown !");



}


void Test_Auth_Vt_exl_rep_v3(void)
{
	SYSTEMTIME systime;
	CHAR string_w[5000] = { 0 };
	CHAR string_r[5000] = { 0 };
	CHAR string_t[5000] = { 0 };
	DWORD size;
	struct MSG_HEADER *p_header;
	struct MSG_LC_AUTH_VT_EXL_REP_V3 * psAuthExlRep = NULL;
	struct MSG_LC_AUTH_VT_EXL_REP_V3 * psAuthExlRep_r = NULL;

	printf("\n Test_Auth_Vt_exl_rep_v3: Test MSG_LC_AUTH_VT_EXL_REP_V3!");

	psAuthExlRep = MSG_LC_AUTH_VT_EXL_REP_V3_New();

	psAuthExlRep->header.plaza_number = 111;
	psAuthExlRep->header.lane_number = 1101;
	psAuthExlRep->body.exit_trs_ref_num = 1;

	GetLocalTime(&systime);
	psAuthExlRep->body.dte_ref_entry = systime;
	psAuthExlRep->body.dte_entry = systime;

	psAuthExlRep->body.exit_plaza_id = 1;
	psAuthExlRep->body.exit_lane_id = 1;

	strcpy_s(psAuthExlRep->body.exit_trs_id, sizeof(psAuthExlRep->body.exit_trs_id), "000100001000005000020004");

	strcpy_s(psAuthExlRep->body.exit_vrn, sizeof(psAuthExlRep->body.exit_vrn), "ABCD2");
	strcpy_s(psAuthExlRep->body.exit_vrn_country, sizeof(psAuthExlRep->body.exit_vrn_country), "MNE");
	strcpy_s(psAuthExlRep->body.exit_provider, sizeof(psAuthExlRep->body.exit_provider), "AOS");
	strcpy_s(psAuthExlRep->body.accepted_days, sizeof(psAuthExlRep->body.accepted_days), "7F");

	psAuthExlRep->body.flag_result = 1;
	psAuthExlRep->body.reason_id = 1;
	psAuthExlRep->body.flag_delete = 0;
	psAuthExlRep->body.flag_no_update = 0;
	psAuthExlRep->body.flag_open_system = 0;
	psAuthExlRep->body.request_id = 133;

	if (MSG_LC_AUTH_VT_EXL_REP_V3_Write(psAuthExlRep, string_w, sizeof(string_w), &size) == TRUE)
	{
		string_w[size] = '\0';
		printf("\nBuffer w = %s", string_w);
	}
	else
		printf("\nWrite Error !");


	psAuthExlRep_r = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (psAuthExlRep_r != NULL)
	{
		if (MSG_LC_AUTH_VT_EXL_REP_V3_Write(psAuthExlRep_r, string_r, sizeof(string_r), &size) == TRUE)
		{
			string_r[size] = '\0';
			printf("\nBuffer r = %s", string_r);
		}
		else
			printf("\nRead Error !");
	}

	printf("\nBuffer w = buffer r : %s", strcmp(string_w, string_r) == 0 ? "YES" : "NO");

	MSG_LC_AUTH_VT_EXL_REP_V3_Delete_All(psAuthExlRep_r);
	MSG_LC_AUTH_VT_EXL_REP_V3_Delete_All(psAuthExlRep);


	p_header = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (p_header != NULL)
	{
		printf("\nCD of message = %ld\n", p_header->cd);
		switch (p_header->cd)
		{

		case MSG_LC_AUTH_VT_EXL_REP_V3_CD:
			psAuthExlRep_r = (struct MSG_LC_AUTH_VT_EXL_REP_V3 *) p_header;

			printf("\n entry_trs_ref_num = %s", psAuthExlRep_r->body.entry_trs_ref_num == psAuthExlRep->body.entry_trs_ref_num ? "YES" : "NO");
			printf("\n entry_plaza_id = %s", psAuthExlRep_r->body.entry_plaza_id == psAuthExlRep->body.entry_plaza_id ? "YES" : "NO");
			printf("\n entry_lane_id = %s", psAuthExlRep_r->body.entry_lane_id == psAuthExlRep->body.entry_lane_id ? "YES" : "NO");
			printf("\n entry_trs_id = %s", strcmp(psAuthExlRep_r->body.entry_trs_id, psAuthExlRep->body.entry_trs_id) == 0 ? "YES" : "NO");
			printf("\n entry_vrn = %s", strcmp(psAuthExlRep_r->body.entry_vrn, psAuthExlRep->body.entry_vrn) == 0 ? "YES" : "NO");
			printf("\n entry_vrn_country = %s", strcmp(psAuthExlRep_r->body.entry_vrn_country, psAuthExlRep->body.entry_vrn_country) == 0 ? "YES" : "NO");
			printf("\n entry_provider = %s", strcmp(psAuthExlRep_r->body.entry_provider, psAuthExlRep->body.entry_provider) == 0 ? "YES" : "NO");
			printf("\n exit_trs_ref_num = %s", psAuthExlRep_r->body.exit_trs_ref_num == psAuthExlRep->body.exit_trs_ref_num ? "YES" : "NO");
			printf("\n exit_plaza_id = %s", psAuthExlRep_r->body.exit_plaza_id == psAuthExlRep->body.exit_plaza_id ? "YES" : "NO");
			printf("\n exit_lane_id = %s", psAuthExlRep_r->body.exit_lane_id == psAuthExlRep->body.exit_lane_id ? "YES" : "NO");
			printf("\n exit_trs_id = %s", strcmp(psAuthExlRep_r->body.exit_trs_id, psAuthExlRep->body.exit_trs_id) == 0 ? "YES" : "NO");
			printf("\n exit_vrn = %s", strcmp(psAuthExlRep_r->body.exit_vrn, psAuthExlRep->body.exit_vrn) == 0 ? "YES" : "NO");
			printf("\n exit_vrn_country = %s", strcmp(psAuthExlRep_r->body.exit_vrn_country, psAuthExlRep->body.exit_vrn_country) == 0 ? "YES" : "NO");
			printf("\n exit_provider = %s", strcmp(psAuthExlRep_r->body.exit_provider, psAuthExlRep->body.exit_provider) == 0 ? "YES" : "NO");
			printf("\n flag_eticket = %s", psAuthExlRep_r->body.flag_eticket == psAuthExlRep->body.flag_eticket ? "YES" : "NO");
			printf("\n flag_delete = %s", psAuthExlRep_r->body.flag_delete == psAuthExlRep->body.flag_delete ? "YES" : "NO");
			printf("\n flag_open_system = %s", psAuthExlRep_r->body.flag_open_system == psAuthExlRep->body.flag_open_system ? "YES" : "NO");
			printf("\n flag_no_update = %s", psAuthExlRep_r->body.flag_no_update == psAuthExlRep->body.flag_no_update ? "YES" : "NO");
			printf("\n flag_result = %s", psAuthExlRep_r->body.flag_result == psAuthExlRep->body.flag_result ? "YES" : "NO");
			printf("\n reason_id = %s", psAuthExlRep_r->body.reason_id == psAuthExlRep->body.reason_id ? "YES" : "NO");
			printf("\n ticket_type = %s", psAuthExlRep_r->body.ticket_type == psAuthExlRep->body.ticket_type ? "YES" : "NO");
			printf("\n vehicle_class = %s", psAuthExlRep_r->body.vehicle_class == psAuthExlRep->body.vehicle_class ? "YES" : "NO");
			printf("\n payment_subtype = %s", psAuthExlRep_r->body.payment_subtype == psAuthExlRep->body.payment_subtype ? "YES" : "NO");
			printf("\n accepted_days = %s", strcmp(psAuthExlRep_r->body.accepted_days, psAuthExlRep->body.accepted_days) == 0 ? "YES" : "NO");
			printf("\n exempt_acccount_number = %s", psAuthExlRep_r->body.exempt_acccount_number == psAuthExlRep->body.exempt_acccount_number ? "YES" : "NO");
			printf("\n exempt_user_number = %s", psAuthExlRep_r->body.exempt_user_number == psAuthExlRep->body.exempt_user_number ? "YES" : "NO");
			printf("\n exempt_renewal_number = %s", psAuthExlRep_r->body.exempt_renewal_number == psAuthExlRep->body.exempt_renewal_number ? "YES" : "NO");
			printf("\n account_balance = %s", psAuthExlRep_r->body.account_balance == psAuthExlRep->body.account_balance ? "YES" : "NO");
			printf("\n user_balance = %s", strcmp(psAuthExlRep_r->body.user_balance, psAuthExlRep->body.user_balance) == 0 ? "YES" : "NO");
			printf("\n contract_type = %s", psAuthExlRep_r->body.contract_type == psAuthExlRep->body.contract_type ? "YES" : "NO");
			printf("\n request_id = %s \n", psAuthExlRep_r->body.request_id == psAuthExlRep->body.request_id ? "YES" : "NO");
			printf("\n");
			MSG_LC_AUTH_VT_EXL_REP_V3_Delete_All(psAuthExlRep_r);
			break;
		default:
			// INCONNU...
			break;
		}
	}
	else
		printf("\nCD unknown !");




}

    