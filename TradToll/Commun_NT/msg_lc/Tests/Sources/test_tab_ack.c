#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_tab_ack.h>
#include <stdio.h>
#include <conio.h>

void Test_Tab_Ack(void)
{

    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;

    struct MSG_TAB_ACK *p_tab;
    struct MSG_TAB_ACK tab_ack = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_TAB_ACK_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Time payment
		{ { 0 }, 3, 6, "121212", 2}

    };

    GetLocalTime (&systime);

	
	p_tab = MSG_TAB_ACK_New ();
    *p_tab = tab_ack;
    p_tab->body.time_of_tab_ack = systime;

    if (MSG_TAB_ACK_Write (p_tab, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = %s", string_t);
    }
    else
        printf ("\nErreur TabAck!");

    /*p_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {
        case MSG_PAYMENT_CD:
            p_pay_r = (struct MSG_PAYMENT *) p_header;
            // TO DO...
            break;

        case MSG_TAB_ACK_CD:
            p_tab = (struct MSG_TAB_ACK *)p_header;
            // TO DO...
            break;

        default:
            // INCONNU...
            break;
        }
    }
    else
        printf ("\nCD inconnu !");*/

	printf("\nCD of tab ack msg : %d\n", p_tab->header.cd);

    MSG_TAB_ACK_Delete_All (p_tab);
//    getch();
}