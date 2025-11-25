// Test file for Payment message
#include <msg_lc_header.h>
#include <csr_msg.h>
#include <msg_lc_hourly.h>
#include <stdio.h>
#include <conio.h>


void Test_Hourly(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    
    struct MSG_HOURLY *p_hour_w = NULL;
	//, *p_pay_r;
	struct MSG_HOURLY_Detailed_Traffic *p_det_traffic = NULL;
	struct MSG_HOURLY_Currency *p_cur = NULL;
	struct MSG_HOURLY_Traffic_Det_Payment *p_tdet_payment = NULL;
	struct MSG_HOURLY_Currency_Det_Payment *p_cdet_payment = NULL;
	
    struct MSG_HOURLY hourly = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_HOURLY_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Time limit, primary_currency_id, primary revenue
		{ { 0 }, { 0 }, 1, 1.0 },

        // GENERAL TRAFFIC:	total traffic, total violation, maintenance traffic
        { 52, 8, 69, 32.6},

        // DETAILED TRAFFIC : List
        { NULL },

        
        // CURRENCY : List
        { NULL },
	};

    GetLocalTime (&systime);

    hourly.body.time_of_inferior_limit = systime;
    
    p_det_traffic = MSG_HOURLY_Detailed_Traffic_New (&hourly.detailed_traffic);
    p_det_traffic->class_id = 12;
    p_tdet_payment = MSG_HOURLY_Traffic_Det_Payment_New (&p_det_traffic->detailed_payment);
    p_tdet_payment->payment_id = 3;
    p_tdet_payment->traffic = 4567;
    p_tdet_payment = MSG_HOURLY_Traffic_Det_Payment_New (&p_det_traffic->detailed_payment);
    p_tdet_payment->payment_id = 4;
    p_tdet_payment->traffic = 5678;
    
    p_det_traffic = MSG_HOURLY_Detailed_Traffic_New (&hourly.detailed_traffic);
    p_det_traffic->class_id = 13;
    p_tdet_payment = MSG_HOURLY_Traffic_Det_Payment_New (&p_det_traffic->detailed_payment);
    p_tdet_payment->payment_id = 3;
    p_tdet_payment->traffic = 3333;
    p_tdet_payment = MSG_HOURLY_Traffic_Det_Payment_New (&p_det_traffic->detailed_payment);
    p_tdet_payment->payment_id = 4;
    p_tdet_payment->traffic = 4444;

    p_cur = MSG_HOURLY_Currency_New (&hourly.currency);
    p_cur->currency_id = 6;
    p_cdet_payment = MSG_HOURLY_Currency_Det_Payment_New (&p_cur->detailed_payment);
    p_cdet_payment->payment_id = 92;
    p_cdet_payment->revenue = 4.45678912;
    
    
    p_hour_w = MSG_HOURLY_New ();
    *p_hour_w = hourly;

    if (MSG_HOURLY_Write (p_hour_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
        printf ("\nErreur !");

/*    p_pay_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_pay_r != NULL)
    {
        if (MSG_PAYMENT_Write (p_pay_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

    MSG_PAYMENT_Delete_All (p_pay_r);*/
	
	printf("\nCD of hourly msg : %d\n", p_hour_w->header.cd);
    
	MSG_HOURLY_Delete_All (p_hour_w);
//	getch();
}