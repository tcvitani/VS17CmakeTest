#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_transaction.h>
#include <stdio.h>
#include <conio.h>

void Test_Transaction(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    DWORD size;
    struct MSG_HEADER *p_header = NULL;
	struct MSG_TRANSACTION *p_transaction_w = NULL;
	struct MSG_TRANSACTION *p_transaction_r = NULL;
    struct MSG_TRANSACTION_Anomaly *p_anomaly = NULL;
    struct MSG_TRANSACTION_Entry_Info *p_entry_info = NULL;
	struct MSG_TRANSACTION_Anomaly_Description *p_anomaly_desc = NULL;

	struct MSG_TRANSACTION transaction = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_TRANSACTION_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Transaction Time
        { { 0 } },

        
		//START OF SHIFT REFERENCE Start of Shift msg id,Time
		{ 99999, { 0 } },
		
		//TRANSACTION REFERENCE
		{ 99999, { 0 }, 12121212, 34343434 },

		//TRANSACTION DETAILS
		{ 15, 16, 17, "VARSTR", 14, 14, 2, 88.959, 6, -1, 12.56, 88.959, 12.66, 3.55, 1, 0, "ST 123 AH" },

		//ENTRY INFO : List
        { NULL },

		// ANOMALIES : List
        { NULL },
    };

    GetLocalTime (&systime);

    transaction.body.time_of_transaction = systime;
	transaction.ref_shift.start_time = systime;
	transaction.ref_trans.start_time = systime;
	

    p_anomaly = MSG_TRANSACTION_Anomaly_New (&transaction.list_anomalies);
    p_anomaly->id = 85;
    p_anomaly->value = 99;

	p_anomaly_desc = MSG_TRANSACTION_Anomaly_Description_New(&p_anomaly->list_anomaly_descritpion);
	p_anomaly_desc->description.type = MSG_FIELD_VARSTR;
	strcpy_s(p_anomaly_desc->description.String, sizeof(p_anomaly_desc->description.String), "Event Description 1");

	p_anomaly_desc = MSG_TRANSACTION_Anomaly_Description_New(&p_anomaly->list_anomaly_descritpion);
	p_anomaly_desc->description.type = MSG_FIELD_LONG;
	p_anomaly_desc->description.Long = 123;
    
	p_entry_info = MSG_TRANSACTION_Entry_Info_New (&transaction.entry_info);

    p_entry_info->plaza_number = 2;               // 0-9999
    p_entry_info->lane_number = 3;                // 0-9999
    p_entry_info->time_of_entry_trans = systime;         
    p_entry_info->collector_id = 11234;               // 0-999999
    p_entry_info->transaction_entry_class = 1;    // 0-99
    p_entry_info->keyed_entry_class = 1;          // 0-99
    p_entry_info->detected_entry_class = 2;       // 0-99
    p_entry_info->entry_fare_point = 120;           
    p_entry_info->destination_fare_point = 121;     // 0-9999
    p_entry_info->destination_plaza_number = 2;   //0-9999    
    p_entry_info->prepaid_amount = 150.59;             //0-99 999 999
    p_entry_info->country_id = 33;
	strcpy_s(p_entry_info->company_id, sizeof(p_entry_info->company_id), "17");
    p_entry_info->info_getting_mode = 5;          // 0-99
    p_entry_info->distribution_level = 1;         //0 ou 1
	p_entry_info->enl_trs_id = 123456;			  // 0-99999999
	strcpy_s(p_entry_info->entry_lic_plate, sizeof(p_entry_info->entry_lic_plate), "ST 123 AH");
		
    p_transaction_w = MSG_TRANSACTION_New ();
    *p_transaction_w = transaction;

    if (MSG_TRANSACTION_Write (p_transaction_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
        printf ("\nErreur !");
    
/*	p_transaction_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_transaction_r != NULL)
    {
        if (MSG_TRANSACTION_Write (p_transaction_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

    MSG_TRANSACTION_Delete_All (p_transaction_r);*/

	printf("\nCD of transaction msg : %d\n", p_transaction_w->header.cd);
    MSG_TRANSACTION_Delete_All (p_transaction_w);


    /*p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {
        case MSG_TRANSACTION_CD:
            p_transaction_r = (struct MSG_TRANSACTION *) p_header;
            // TO DO...
            break;

 
        default:
            // INCONNU...
            break;
        }
    }
    else
        printf ("\nCD UNKNOWN !");
*/
}
    