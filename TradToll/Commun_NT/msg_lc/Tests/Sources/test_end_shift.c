// Test for End of Shift Message
#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_endshift.h>
#include <stdio.h>
#include <conio.h>

void Test_End_Shift(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_HEADER *p_header = NULL;
	struct MSG_END_SHIFT *p_endshift_w = NULL;
	struct MSG_END_SHIFT *p_endshift_r = NULL;
    struct MSG_END_SHIFT_Coin_Information *p_coin = NULL;
    struct MSG_END_SHIFT_Bowl_Information *p_bowl = NULL;
    struct MSG_END_SHIFT endshift = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_END_SHIFT_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : TimeEndShift CloseType
        { { 0 }, 1 },

        // START SHIFT REF : StartShiftId StartShiftTime
        { 12345, { 0 } },

        // COIN INFORMATION : List
        { NULL },

        // BOWL INFORMATION : List
        { NULL },
      
    };

    GetLocalTime (&systime);

    endshift.body.time_of_end_shift = systime;
    endshift.start_ref.start_shift_time = systime;

    p_coin = MSG_END_SHIFT_Coin_Information_New (&endshift.list_coin_information);
    p_coin->coin_id = 12;
    p_coin->coin_counter = 200;
	p_bowl= MSG_END_SHIFT_Bowl_Information_New (&endshift.list_bowl_information);
	p_bowl->bowl_position = 3;
	p_bowl->coin_change_counter = 7777;
	p_bowl->coin_id = 72;
	p_bowl= MSG_END_SHIFT_Bowl_Information_New (&endshift.list_bowl_information);
	p_bowl->bowl_position = 4;
	p_bowl->coin_change_counter = 8888;
	p_bowl->coin_id = 86;

    p_coin = MSG_END_SHIFT_Coin_Information_New (&endshift.list_coin_information);
    p_coin->coin_id = 14;
    p_coin->coin_counter = 50;
	p_bowl= MSG_END_SHIFT_Bowl_Information_New (&endshift.list_bowl_information);
	p_bowl->bowl_position = 5;
	p_bowl->coin_change_counter = 5555;
	p_bowl->coin_id = 59;

    
    //p_trk = MSG_PAYMENT_Track_New (&payment.list_tracks);
    //p_trk->size = 10;
    //memcpy(p_trk->track, "1\1\2\3\4\5\6ABCDE", p_trk->size);
    
    p_endshift_w = MSG_END_SHIFT_New ();
    *p_endshift_w = endshift;

    if (MSG_END_SHIFT_Write (p_endshift_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
        printf ("\nError !");

 /*   p_endshift_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_endshift_r != NULL)
    {
        if (MSG_END_SHIFT_Write (p_endshift_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nError !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

    MSG_END_SHIFT_Delete_All (p_endshift_r);*/

	
	printf("\nCD of end shift msg : %d\n", p_endshift_w->header.cd);
    MSG_END_SHIFT_Delete_All (p_endshift_w);
		

/*    p_header = MSG_New_Read_If_Found (NULL, string_r, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD of message = %ld" , p_header->cd);
        switch (p_header->cd)
        {
        case MSG_END_SHIFT_CD:
            p_endshift_r = (struct MSG_END_SHIFT *) p_header;
            // TO DO...
            break;

        default:
            // INCONNU...
            break;
        }
    }
    else
        printf ("\nCD Unknown !");*/

}
    