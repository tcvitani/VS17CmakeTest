// Test for Event Message
#include <msg_lc_header.h>
//#include <testvarstr.h>
#include <stdio.h>
#include <conio.h>

void Test_VarStr(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_HEADER *p_header;
    struct TESTVARSTR *p_event_w, *p_event_r;

    struct TESTVARSTR event = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, TESTVARSTR_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : TimeOfEvent Category SubCategory Status
        { { 0 }, 55, 17,"VARSTR\0","FIXSTR\0" , 72 },
        
    };
 
    GetLocalTime (&systime);

    event.body.time_of_event = systime;

    p_event_w = TESTVARSTR_New ();
    *p_event_w = event;

    if (TESTVARSTR_Write (p_event_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
        printf ("\nErreur !");

/*    p_event_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_event_r != NULL)
    {
        if (TESTVARSTR_Write (p_event_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

    TESTVARSTR_Delete_All (p_event_r);*/

	printf("\nCD of event msg : %d\n", p_event_w->header.cd);
    TESTVARSTR_Delete_All (p_event_w);


    /*p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {
 
        case TESTVARSTR_CD:
            p_event_r = (struct TESTVARSTR *) p_header;
            // TO DO...
            break;
        default:
            // INCONNU...
            break;
        }
    }
    else
        printf ("\nCD unknown !");
*/
}