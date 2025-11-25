// Test file for Payment message
#include <msg_lc_header.h>
#include <csr_msg.h>
#include <msg_lc_payment.h>
#include <stdio.h>
#include <conio.h>


void Test_Payment(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    
    struct MSG_PAYMENT						*p_pay_w;
    struct MSG_PAYMENT_Sold_Product			*p_sold;
    struct MSG_PAYMENT_Official_Currency	*p_cur;
    struct MSG_PAYMENT_Anomaly				*p_ano;
    struct MSG_PAYMENT_Elem_Info			*p_elem_info;
	struct MSG_PAYMENT_Info_Type			*p_info_type;
	struct MSG_PAYMENT_Anomaly_Description	*p_evt_desc = NULL;

    struct MSG_PAYMENT payment = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_PAYMENT_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Time payment
        { { 0 } },

        // TRS REF : Id Start time
        { 12345, { 0 } },

        // SOLD PRODUCT : List
        { NULL },

        // VALUE
        { 10, 3.51266, 1, 2956.22511, 1 },

        // TYPE
        { 11, 12, 8, "COUCOU", 6, "SALUT", 55 },

        // ANOMALY : List
        { NULL },

        // COMPLEMENTARY_INFO : List
        { NULL },
        
	
	};
    GetLocalTime (&systime);

    payment.body.time_of_payment = systime;
    payment.ref_trs.start_time = systime;

    p_sold = MSG_PAYMENT_Sold_Product_New (&payment.list_sold_products);
    p_sold->id = 12;
    p_sold->quantity = 200;
    p_sold->sign = -1;
    p_cur = MSG_PAYMENT_Official_Currency_New (&p_sold->list_official_currencies);
    p_cur->id = 3;
    p_cur->unit_price = 1234;
    p_cur = MSG_PAYMENT_Official_Currency_New (&p_sold->list_official_currencies);
    p_cur->id = 4;
    p_cur->unit_price = 5678;
    
    p_sold = MSG_PAYMENT_Sold_Product_New (&payment.list_sold_products);
    p_sold->id = 13;
    p_sold->quantity = 300;
    p_sold->sign = 1;
    p_cur = MSG_PAYMENT_Official_Currency_New (&p_sold->list_official_currencies);
    p_cur->id = 5;
    p_cur->unit_price = 920.65663;
    
    p_ano = MSG_PAYMENT_Anomaly_New (&payment.list_anomalies);
    p_ano->id = 6;
    p_ano->value = 92;

	p_evt_desc = MSG_PAYMENT_Anomaly_Description_New(&p_ano->list_anomaly_descritpion);
	p_evt_desc->description.type = MSG_FIELD_VARSTR;
	strcpy_s(p_evt_desc->description.String, sizeof(p_evt_desc->description.String), "Event Description 1");

    p_ano = MSG_PAYMENT_Anomaly_New (&payment.list_anomalies);
    p_ano->id = 8;
    p_ano->value = 20;
    
	p_evt_desc = MSG_PAYMENT_Anomaly_Description_New(&p_ano->list_anomaly_descritpion);
	p_evt_desc->description.type = MSG_FIELD_LONG;
	p_evt_desc->description.Long = 123;

//------------------------------------------------------------------------    
    p_info_type = MSG_PAYMENT_Info_Type_New (&payment.list_comp_inf);
    p_info_type->type = 22;
	p_info_type->list_info_type=NULL;

	p_elem_info = MSG_PAYMENT_Elem_Info_New (&p_info_type->list_info_type);
    p_elem_info->code = 1;
    p_elem_info->info_value.type = MSG_FIELD_RAW;
    p_elem_info->info_value.RawSize = 4;
	memcpy(p_elem_info->info_value.Raw, "\xb3\x06\x01\x70", p_elem_info->info_value.RawSize);

	p_elem_info = MSG_PAYMENT_Elem_Info_New (&p_info_type->list_info_type);
    p_elem_info->code = 2;
    p_elem_info->info_value.type = MSG_FIELD_RAWHEX;
    p_elem_info->info_value.RawSize = 4;
	memcpy(p_elem_info->info_value.Raw, "\x12\x34\x56\x78", p_elem_info->info_value.RawSize);
//------------------------------------------------------------------------

    p_pay_w = MSG_PAYMENT_New ();
    *p_pay_w = payment;

    if (MSG_PAYMENT_Write (p_pay_w, string_w, sizeof(string_w), &size) == TRUE)
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
	
	printf("\nCD of payment msg : %d\n", p_pay_w->header.cd);
    
	MSG_PAYMENT_Delete_All (p_pay_w);
//	getch();
}