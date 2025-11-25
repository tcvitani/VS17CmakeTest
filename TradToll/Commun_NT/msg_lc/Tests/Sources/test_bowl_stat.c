#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_bowl_stat.h>
#include <stdio.h>
#include <conio.h>

void Test_Bowl_Stat(void) 
{ 
	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    DWORD size;


	//Declaration of Bowl Status Message

    struct MSG_BOWL_STAT *p_bowl_stat_w;
    struct MSG_BOWL_STAT_Bowl_Information *p_stat,*p_stat_element;
	struct MSG_BOWL_STAT bowl_stat = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_BOWL_STAT_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Time of last filling
        { { 0 } },

		// STATUS INFORMATION : List
        { NULL },

    };

    GetLocalTime (&systime);

	// Msg_Bowl_Stat

    bowl_stat.body.time_of_last_fill = systime;

    p_stat = MSG_BOWL_STAT_Bowl_Information_New (&bowl_stat.list_bowl_informations);
    p_stat->bowl_position = 2;
    p_stat->coin_id = 20;
	//p_stat->bowl_fill_percent = 20;
    p_stat->remaining_coin_counter = 99999999;
     
    p_stat = MSG_BOWL_STAT_Bowl_Information_New (&bowl_stat.list_bowl_informations);
    p_stat->bowl_position = 1;
    p_stat->coin_id = 2;
	//p_stat->bowl_fill_percent = 99;
    p_stat->remaining_coin_counter = 99999;
    
    p_bowl_stat_w = MSG_BOWL_STAT_New ();
    *p_bowl_stat_w = bowl_stat;

	//Jump on first element of chain list
	p_stat_element=MSG_BOWL_STAT_Get_First_Bowl_Information(bowl_stat.list_bowl_informations);
	printf("\nCoin ID of first element :%d",p_stat_element->coin_id);

	//Skip on next element of chain list
	p_stat_element=MSG_BOWL_STAT_Get_Next_Bowl_Information(bowl_stat.list_bowl_informations,p_stat_element);
	printf("\nCoin ID of next element :%d",p_stat_element->coin_id);

    if (MSG_BOWL_STAT_Write (p_bowl_stat_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w of Bowl Status Message = %s", string_w);
    }
    else
        printf ("\nErreur !");

/*    p_bowl_stat_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_bowl_stat_r != NULL)
    {
        if (MSG_BOWL_STAT_Write (p_bowl_stat_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r of Bowl Status Message = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nBowl Status Message Buffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");
	printf ("\nCD du message = %ld" , p_bowl_stat_r->header.cd);

    MSG_BOWL_STAT_Delete_All (p_bowl_stat_r);*/
	printf("\nCD of bowl status msg : %d\n", p_bowl_stat_w->header.cd);

    MSG_BOWL_STAT_Delete_All (p_bowl_stat_w);

	}
