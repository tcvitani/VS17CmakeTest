// Test for Backup by Date Message
#include <csr_msg.h>
#include <msg_lc_header.h>

#include <msg_lc_auth_vt_enl_req.h>
#include <msg_lc_auth_vt_enl_rep.h>
#include <msg_lc_auth_vt_exl_req.h>
#include <msg_lc_auth_vt_exl_rep.h>

#include <stdio.h>
#include <conio.h>


void Test_Auth_Vt_enl_req(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_HEADER *p_header;

	struct MSG_LC_AUTH_VT_ENL_REQ * psAuthEnlReq = NULL;
	struct MSG_LC_AUTH_VT_ENL_REQ * psAuthEnlReq_r = NULL;

	printf("\nTest_Auth_Vt_enl_req: Test MSG_LC_AUTH_VT_ENL_REQ!");

	psAuthEnlReq = MSG_LC_AUTH_VT_ENL_REQ_New();

	psAuthEnlReq->header.plaza_number = 1;
	psAuthEnlReq->header.lane_number = 1;
	psAuthEnlReq->body.entry_trs_ref_num = 1;


	GetLocalTime (&systime);
	psAuthEnlReq->body.dte_ref_entry = systime;
	psAuthEnlReq->body.dte_entry = systime;
  
	psAuthEnlReq->body.entry_plaza_id = 1;
	psAuthEnlReq->body.entry_lane_id = 1;

	strcpy_s(psAuthEnlReq->body.entry_trs_id, sizeof(psAuthEnlReq->body.entry_trs_id), "0001");

	strcpy_s(psAuthEnlReq->body.entry_vrn, sizeof(psAuthEnlReq->body.entry_vrn), "ABCD2");
	strcpy_s(psAuthEnlReq->body.entry_vrn_country, sizeof(psAuthEnlReq->body.entry_vrn_country), "PL");
	strcpy_s(psAuthEnlReq->body.entry_provider, sizeof(psAuthEnlReq->body.entry_provider), "AOS");

	if (MSG_LC_AUTH_VT_ENL_REQ_Write(psAuthEnlReq, string_w, sizeof(string_w), &size) == TRUE)
	{
		string_w[size] = '\0';
		printf("\nBuffer w = %s", string_w);
	}
	else
		printf("\nWrite Error !");


	psAuthEnlReq_r = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (psAuthEnlReq_r != NULL)
    {
		if (MSG_LC_AUTH_VT_ENL_REQ_Write(psAuthEnlReq_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nRead Error !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

	MSG_LC_AUTH_VT_ENL_REQ_Delete_All(psAuthEnlReq_r);
	MSG_LC_AUTH_VT_ENL_REQ_Delete_All(psAuthEnlReq);


    p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD of message = %ld\n" , p_header->cd);
        switch (p_header->cd)
        {
 
		case MSG_LC_AUTH_VT_ENL_REQ_CD:
			psAuthEnlReq_r = (struct MSG_LC_AUTH_VT_ENL_REQ *) p_header;
			MSG_LC_AUTH_VT_ENL_REQ_Delete_All(psAuthEnlReq_r);            
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


void Test_Auth_Vt_enl_rep(void)
{
	SYSTEMTIME systime;
	CHAR string_w[5000] = { 0 };
	CHAR string_r[5000] = { 0 };
	CHAR string_t[5000] = { 0 };
	DWORD size;
	struct MSG_HEADER *p_header;
	struct MSG_LC_AUTH_VT_ENL_REP * psAuthEnlRep = NULL;
	struct MSG_LC_AUTH_VT_ENL_REP * psAuthEnlRep_r = NULL;

	printf("\n Test_Auth_Vt_enl_rep: Test MSG_LC_AUTH_VT_ENL_REP!");

	psAuthEnlRep = MSG_LC_AUTH_VT_ENL_REP_New();

	psAuthEnlRep->header.plaza_number = 1;
	psAuthEnlRep->header.lane_number = 1;
	psAuthEnlRep->body.entry_trs_ref_num = 1;

	GetLocalTime(&systime);
	psAuthEnlRep->body.dte_ref_entry = systime;
	psAuthEnlRep->body.dte_entry = systime;

	psAuthEnlRep->body.entry_plaza_id = 1;
	psAuthEnlRep->body.entry_lane_id = 1;

	strcpy_s(psAuthEnlRep->body.entry_trs_id, sizeof(psAuthEnlRep->body.entry_trs_id), "0001");

	strcpy_s(psAuthEnlRep->body.entry_vrn, sizeof(psAuthEnlRep->body.entry_vrn), "ABCD2");
	strcpy_s(psAuthEnlRep->body.entry_vrn_country, sizeof(psAuthEnlRep->body.entry_vrn_country), "PL");
	strcpy_s(psAuthEnlRep->body.entry_provider, sizeof(psAuthEnlRep->body.entry_provider), "AOS");
	
	psAuthEnlRep->body.flag_result = 1;
	psAuthEnlRep->body.reason_id = 1;
	
	if (MSG_LC_AUTH_VT_REP_ENL_Write(psAuthEnlRep, string_w, sizeof(string_w), &size) == TRUE)
	{
		string_w[size] = '\0';
		printf("\nBuffer w = %s", string_w);
	}
	else
		printf("\nWrite Error !");


	psAuthEnlRep_r = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (psAuthEnlRep_r != NULL)
	{
		if (MSG_LC_AUTH_VT_REP_ENL_Write(psAuthEnlRep_r, string_r, sizeof(string_r), &size) == TRUE)
		{
			string_r[size] = '\0';
			printf("\nBuffer r = %s", string_r);
		}
		else
			printf("\nRead Error !");
	}

	printf("\nBuffer w = buffer r : %s", strcmp(string_w, string_r) == 0 ? "YES" : "NO");

	MSG_LC_AUTH_VT_ENL_REP_Delete_All(psAuthEnlRep_r);
	MSG_LC_AUTH_VT_ENL_REP_Delete_All(psAuthEnlRep);


	p_header = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (p_header != NULL)
	{
		printf("\nCD of message = %ld\n", p_header->cd);
		switch (p_header->cd)
		{

		case MSG_LC_AUTH_VT_ENL_REP_CD:
			psAuthEnlRep_r = (struct MSG_LC_AUTH_VT_ENL_REP *) p_header;
			MSG_LC_AUTH_VT_ENL_REP_Delete_All(psAuthEnlRep_r);			
			break;
		default:
			// INCONNU...
			break;
		}
	}
	else
		printf("\nCD unknown !");




}



void Test_Auth_Vt_exl_req(void)
{
	SYSTEMTIME systime;
	CHAR string_w[5000] = { 0 };
	CHAR string_r[5000] = { 0 };
	CHAR string_t[5000] = { 0 };
	DWORD size;
	struct MSG_HEADER *p_header;

	struct MSG_LC_AUTH_VT_EXL_REQ * psAuthExlReq = NULL;
	struct MSG_LC_AUTH_VT_EXL_REQ * psAuthExlReq_r = NULL;

	printf("\n Test_Auth_Vt_exl_req: Test MSG_LC_AUTH_VT_EXL_REQ!");

	psAuthExlReq = MSG_LC_AUTH_VT_EXL_REQ_New();

	psAuthExlReq->header.plaza_number = 1;
	psAuthExlReq->header.lane_number = 1;
	psAuthExlReq->body.exit_trs_ref_num = 1;


	GetLocalTime(&systime);
	psAuthExlReq->body.dte_ref_exit = systime;
	psAuthExlReq->body.dte_exit = systime;

	psAuthExlReq->body.exit_plaza_id = 1;
	psAuthExlReq->body.exit_lane_id = 1;

	strcpy_s(psAuthExlReq->body.exit_trs_id, sizeof(psAuthExlReq->body.exit_trs_id), "0001");

	strcpy_s(psAuthExlReq->body.exit_vrn, sizeof(psAuthExlReq->body.exit_vrn), "ABCD2");
	strcpy_s(psAuthExlReq->body.exit_vrn_country, sizeof(psAuthExlReq->body.exit_vrn_country), "PL");
	strcpy_s(psAuthExlReq->body.exit_provider, sizeof(psAuthExlReq->body.exit_provider), "AOS");

	if (MSG_LC_AUTH_VT_EXL_REQ_Write(psAuthExlReq, string_w, sizeof(string_w), &size) == TRUE)
	{
		string_w[size] = '\0';
		printf("\nBuffer w = %s", string_w);
	}
	else
		printf("\nWrite Error !");


	psAuthExlReq_r = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (psAuthExlReq_r != NULL)
	{
		if (MSG_LC_AUTH_VT_EXL_REQ_Write(psAuthExlReq_r, string_r, sizeof(string_r), &size) == TRUE)
		{
			string_r[size] = '\0';
			printf("\nBuffer r = %s", string_r);
		}
		else
			printf("\nRead Error !");
	}

	printf("\nBuffer w = buffer r : %s", strcmp(string_w, string_r) == 0 ? "YES" : "NO");

	MSG_LC_AUTH_VT_EXL_REQ_Delete_All(psAuthExlReq_r);
	MSG_LC_AUTH_VT_EXL_REQ_Delete_All(psAuthExlReq);


	p_header = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (p_header != NULL)
	{
		printf("\nCD of message = %ld\n", p_header->cd);
		switch (p_header->cd)
		{

		case MSG_LC_AUTH_VT_EXL_REQ_CD:
			psAuthExlReq_r = (struct MSG_LC_AUTH_VT_EXL_REQ *) p_header;
			MSG_LC_AUTH_VT_EXL_REQ_Delete_All(psAuthExlReq_r);
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


void Test_Auth_Vt_exl_rep(void)
{
	SYSTEMTIME systime;
	CHAR string_w[5000] = { 0 };
	CHAR string_r[5000] = { 0 };
	CHAR string_t[5000] = { 0 };
	DWORD size;
	struct MSG_HEADER *p_header;
	struct MSG_LC_AUTH_VT_EXL_REP * psAuthExlRep = NULL;
	struct MSG_LC_AUTH_VT_EXL_REP * psAuthExlRep_r = NULL;

	printf("\n Test_Auth_Vt_exl_rep: Test MSG_LC_AUTH_VT_EXL_REP!");

	psAuthExlRep = MSG_LC_AUTH_VT_EXL_REP_New();

	psAuthExlRep->header.plaza_number = 1;
	psAuthExlRep->header.lane_number = 1;
	psAuthExlRep->body.exit_trs_ref_num = 1;

	GetLocalTime(&systime);
	psAuthExlRep->body.dte_ref_entry = systime;
	psAuthExlRep->body.dte_entry = systime;

	psAuthExlRep->body.exit_plaza_id = 1;
	psAuthExlRep->body.exit_lane_id = 1;

	strcpy_s(psAuthExlRep->body.exit_trs_id, sizeof(psAuthExlRep->body.exit_trs_id), "0001");

	strcpy_s(psAuthExlRep->body.exit_vrn, sizeof(psAuthExlRep->body.exit_vrn), "ABCD2");
	strcpy_s(psAuthExlRep->body.exit_vrn_country, sizeof(psAuthExlRep->body.exit_vrn_country), "PL");
	strcpy_s(psAuthExlRep->body.exit_provider, sizeof(psAuthExlRep->body.exit_provider), "AOS");

	psAuthExlRep->body.flag_result = 1;
	psAuthExlRep->body.reason_id = 1;

	if (MSG_LC_AUTH_VT_REP_EXL_Write(psAuthExlRep, string_w, sizeof(string_w), &size) == TRUE)
	{
		string_w[size] = '\0';
		printf("\nBuffer w = %s", string_w);
	}
	else
		printf("\nWrite Error !");


	psAuthExlRep_r = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (psAuthExlRep_r != NULL)
	{
		if (MSG_LC_AUTH_VT_REP_EXL_Write(psAuthExlRep_r, string_r, sizeof(string_r), &size) == TRUE)
		{
			string_r[size] = '\0';
			printf("\nBuffer r = %s", string_r);
		}
		else
			printf("\nRead Error !");
	}

	printf("\nBuffer w = buffer r : %s", strcmp(string_w, string_r) == 0 ? "YES" : "NO");

	MSG_LC_AUTH_VT_EXL_REP_Delete_All(psAuthExlRep_r);
	MSG_LC_AUTH_VT_EXL_REP_Delete_All(psAuthExlRep);


	p_header = MSG_New_Read_If_Found(NULL, string_w, sizeof(string_w));

	if (p_header != NULL)
	{
		printf("\nCD of message = %ld\n", p_header->cd);
		switch (p_header->cd)
		{

		case MSG_LC_AUTH_VT_EXL_REP_CD:
			psAuthExlRep_r = (struct MSG_LC_AUTH_VT_EXL_REP *) p_header;
			MSG_LC_AUTH_VT_EXL_REP_Delete_All(psAuthExlRep_r);
			break;
		default:
			// INCONNU...
			break;
		}
	}
	else
		printf("\nCD unknown !");




}

    