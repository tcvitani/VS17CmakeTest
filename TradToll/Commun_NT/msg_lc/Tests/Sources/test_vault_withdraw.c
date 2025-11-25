#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_vlt_withdraw.h>
#include <stdio.h>
#include <conio.h>

void Test_Vault_Witdraw(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    DWORD size;
    struct MSG_HEADER *p_header = NULL;
	struct MSG_VLT_WITHDRAW *p_vlt_withdraw_w = NULL;
	struct MSG_VLT_WITHDRAW *p_vlt_withdraw_r = NULL;
    struct MSG_VLT_WITHDRAW_Coin_Information *p_coin = NULL;
	struct MSG_VLT_WITHDRAW_Bowl_Information *p_bowl = NULL;
    struct MSG_VLT_WITHDRAW vlt_withdraw = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_VLT_WITHDRAW_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Time Vault Withdraw Time Vault Insertion Withdrawn Vault Identifier
        { { 0 },{ 0 },123456 },

        // COIN INFORMATION : List
        { NULL },

		//BOWL INFORMATION :List
		{ NULL },

    };

    GetLocalTime (&systime);

    vlt_withdraw.body.time_of_vault_withdraw = systime;
	vlt_withdraw.body.time_of_vault_insertion = systime;

    p_coin = MSG_VLT_WITHDRAW_Coin_Information_New (&vlt_withdraw.list_coin_information);
    p_coin->coin_id = 85;
    p_coin->coin_counter = 55555555;

    p_coin = MSG_VLT_WITHDRAW_Coin_Information_New (&vlt_withdraw.list_coin_information);
    p_coin->coin_id = 86;
    p_coin->coin_counter = 66666666;

    p_bowl = MSG_VLT_WITHDRAW_Bowl_Information_New (&vlt_withdraw.list_bowl_information);
    p_bowl->bowl_pos = 8;
	p_bowl->coin_id = 77;
    p_bowl->coin_change_counter = 44444444;

    p_bowl = MSG_VLT_WITHDRAW_Bowl_Information_New (&vlt_withdraw.list_bowl_information);
    p_bowl->bowl_pos = 7;
	p_bowl->coin_id = 76;
    p_bowl->coin_change_counter = 33333333;

    p_bowl = MSG_VLT_WITHDRAW_Bowl_Information_New (&vlt_withdraw.list_bowl_information);
    p_bowl->bowl_pos = 6;
    p_bowl->coin_id = 75;
    p_bowl->coin_change_counter = 22222222;

    
    p_vlt_withdraw_w = MSG_VLT_WITHDRAW_New ();
    *p_vlt_withdraw_w = vlt_withdraw;

    if (MSG_VLT_WITHDRAW_Write (p_vlt_withdraw_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
        printf ("\nErreur !");
    
/*	p_vlt_withdraw_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_vlt_withdraw_r != NULL)
    {
        if (MSG_VLT_WITHDRAW_Write (p_vlt_withdraw_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

    MSG_VLT_WITHDRAW_Delete_All (p_vlt_withdraw_r);*/

	printf("\nCD of vault withdraw msg : %d\n", p_vlt_withdraw_w->header.cd);
    MSG_VLT_WITHDRAW_Delete_All (p_vlt_withdraw_w);


    /*p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {
        case MSG_VLT_WITHDRAW_CD:
            p_vlt_withdraw_r = (struct MSG_VLT_WITHDRAW *) p_header;
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
    