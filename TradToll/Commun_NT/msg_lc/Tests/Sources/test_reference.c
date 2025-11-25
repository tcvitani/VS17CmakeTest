// Test for Reference Message
#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_reference.h>
#include <stdio.h>
#include <conio.h>

void Test_Reference(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_HEADER *p_header = NULL;
	struct MSG_REFERENCE *p_reference_w = NULL;
	struct MSG_REFERENCE *p_reference_r = NULL;

    struct MSG_REFERENCE reference = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_REFERENCE_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Length of reference file name, Refrenece file name
        { 15, "petnaESTznakova" },
        
    };
 
    GetLocalTime (&systime);

    
    p_reference_w = MSG_REFERENCE_New ();
    *p_reference_w = reference;

    if (MSG_REFERENCE_Write (p_reference_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
        printf ("\nErreur !");

    p_reference_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_reference_r != NULL)
    {
        if (MSG_REFERENCE_Write (p_reference_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

	printf("\nCD of reference msg : %d\n", p_reference_r->header.cd);

    MSG_REFERENCE_Delete_All (p_reference_r);
    MSG_REFERENCE_Delete_All (p_reference_w);


    /*p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {
 
        case MSG_REFERENCE_CD:
            p_reference_r = (struct MSG_REFERENCE *) p_header;
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
