// Test for Event Message
#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_event.h>
#include <stdio.h>
#include <conio.h>

void Test_Event(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_HEADER					*p_header	= NULL;
	struct MSG_EVENT					*p_event_w	= NULL;
	struct MSG_EVENT					*p_event_r	= NULL;
	struct MSG_EVENT_Event_Description	*p_evt_desc = NULL;

    struct MSG_EVENT event = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_EVENT_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : TimeOfEvent Category SubCategory Status
        { { 0 }, 55, 17, 72, NULL },
        
    };
 
    GetLocalTime (&systime);

    event.body.time_of_event = systime;

	p_evt_desc = MSG_EVENT_Event_Description_New(&event.body.list_event_descritpion);
	p_evt_desc->description.type = MSG_FIELD_VARSTR;
	strcpy_s(p_evt_desc->description.String, sizeof(p_evt_desc->description.String), "Event Description 1");

	p_evt_desc = MSG_EVENT_Event_Description_New(&event.body.list_event_descritpion);
	p_evt_desc->description.type = MSG_FIELD_LONG;
	p_evt_desc->description.Long = 123;

    p_event_w = MSG_EVENT_New ();
    *p_event_w = event;

    if (MSG_EVENT_Write (p_event_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
        printf ("\nErreur !");

/*    p_event_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_event_r != NULL)
    {
        if (MSG_EVENT_Write (p_event_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

    MSG_EVENT_Delete_All (p_event_r);*/

	printf("\nCD of event msg : %d\n", p_event_w->header.cd);
    MSG_EVENT_Delete_All (p_event_w);


    /*p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {
 
        case MSG_EVENT_CD:
            p_event_r = (struct MSG_EVENT *) p_header;
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
    