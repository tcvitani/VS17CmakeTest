/*------------- Test file for Bowl Movement Message ---------------*/

#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_bowl_movement.h>
#include <stdio.h>
#include <conio.h>

void Test_Bowl_Movement(void)
{

	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    DWORD size;


	//Declaration of Bowl Movement Message

    struct MSG_BOWL_MOVEMENT *p_bowl_w;
    struct MSG_BOWL_MOVEMENT_Filling_Information *p_movement;
	struct MSG_BOWL_MOVEMENT bowl_movement = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_BOWL_MOVEMENT_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Time_of_movement Agent_Identifier Movement_Type 
        { { 0 }, 7, 2 },

		// FILLING INFORMATION : List
        { NULL },

    };

	GetLocalTime (&systime);

    bowl_movement.body.time_of_movement = systime;

    p_movement = MSG_BOWL_MOVEMENT_Filling_Information_New (&bowl_movement.list_fill_informations);
    p_movement->bowl_position = 2;
    p_movement->coin_id = 20;
    p_movement->coin_counter = 99999999;
     
    p_movement = MSG_BOWL_MOVEMENT_Filling_Information_New (&bowl_movement.list_fill_informations);
    p_movement->bowl_position = 1;
    p_movement->coin_id = 2;
    p_movement->coin_counter = 99999;
    
    p_bowl_w = MSG_BOWL_MOVEMENT_New ();
    *p_bowl_w = bowl_movement;

    if (MSG_BOWL_MOVEMENT_Write (p_bowl_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w of Bowl Movement Message = %s", string_w);
    }
    else
        printf ("\nErreur !");


	printf("\nCD of bowl movement msg : %d\n", p_bowl_w->header.cd);

    MSG_BOWL_MOVEMENT_Delete_All (p_bowl_w);
}