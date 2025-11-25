// Test for Start of Shift Message
#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_startshift.h>
#include <stdio.h>
#include <conio.h>

void Test_Start_Shift(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_HEADER *p_header = NULL;
	struct MSG_START_SHIFT *p_startshift_w = NULL;
	struct MSG_START_SHIFT *p_startshift_r = NULL;

    struct MSG_START_SHIFT startshift = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_START_SHIFT_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : TimeOfStartShift StartShiftId TollFare FarePoint FareStrip OpenType
        { { 0 }, 1, "dvadeset zbankonbann kdsjfkajb", 1, 7, 1 },
        
    };
 
    GetLocalTime (&systime);

    startshift.body.time_of_start_shift = systime;

    
    p_startshift_w = MSG_START_SHIFT_New ();
    *p_startshift_w = startshift;

    if (MSG_START_SHIFT_Write (p_startshift_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
        printf ("\nErreur !");

/*    p_startshift_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_startshift_r != NULL)
    {
        if (MSG_START_SHIFT_Write (p_startshift_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

    MSG_START_SHIFT_Delete_All (p_startshift_r);*/

	printf("\nCD of start shift msg : %d\n", p_startshift_w->header.cd);
    MSG_START_SHIFT_Delete_All (p_startshift_w);


/*    p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {
        case MSG_START_SHIFT_CD:
            p_startshift_r = (struct MSG_START_SHIFT *) p_header;
            // TO DO...
            break;

        default:
            // UNKNOWN...
            break;
        }
    }
    else
        printf ("\nCD unknown !");
*/
}
    