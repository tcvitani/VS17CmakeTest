#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_status.h>
#include <stdio.h>
#include <conio.h>

void Test_Status (void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    DWORD size;
    struct MSG_STATUS *p_status_w;
    struct MSG_STATUS_Event_Information *p_event;
    struct MSG_STATUS status = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_STATUS_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Time event
        { { 0 } },

        // EVENT INFORMATION : List
        { NULL },

    };

    GetLocalTime (&systime);

    status.body.time_of_event = systime;

    p_event = MSG_STATUS_Event_Information_New (&status.list_event_info);
    p_event->event_category = 12;
    p_event->event_sub_category = 76;
    p_event->current_event_status = 25;
// 	p_event->comp_info.type = MSG_FIELD_VARSTR;
// 	strcpy(p_event->comp_info.String, "Event information");  

    
    p_status_w = MSG_STATUS_New ();
    *p_status_w = status;

    if (MSG_STATUS_Write (p_status_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
	{
        printf ("\nErreur !");
		printf ("\nBuffer w = %s", string_w);
	}

/*    p_status_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_status_r != NULL)
    {
        if (MSG_STATUS_Write (p_status_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

    MSG_STATUS_Delete_All (p_status_r);*/

	printf("\nCD of status msg : %d\n", p_status_w->header.cd);

    MSG_STATUS_Delete_All (p_status_w);


    /*p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {
        case MSG_STATUS_CD:
            p_status_r = (struct MSG_STATUS *) p_header;
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
    