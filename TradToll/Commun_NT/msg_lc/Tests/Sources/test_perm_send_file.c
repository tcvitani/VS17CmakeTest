#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_perm_send_file.h>
#include <stdio.h>
#include <conio.h>

void Test_Perm_Send_File(void)
{

    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    DWORD size;

	//Declaration of Permission to send file Message

    struct MSG_PERM_SEND_FILE *p_perm_send_file_w,*p_perm_send_file_r;
	struct MSG_PERM_SEND_FILE perm_send_file = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_PERM_SEND_FILE_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

    };


    p_perm_send_file_w = MSG_PERM_SEND_FILE_New ();
    *p_perm_send_file_w = perm_send_file;

    if (MSG_PERM_SEND_FILE_Write (p_perm_send_file_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w of Permission To Send File Message = %s", string_w);
    }
    else
        printf ("\nErreur !");

   p_perm_send_file_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_perm_send_file_r != NULL)
    {
        if (MSG_PERM_SEND_FILE_Write (p_perm_send_file_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r of Permission To Send File Message = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nPermission To Send File Message Buffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

	printf("\nCD of permission to send file msg : %d", p_perm_send_file_r->header.cd);

    if (MSG_PERM_SEND_FILE_Delete_All (p_perm_send_file_r) == TRUE)
		printf("\nDel read OK");
	else printf("\nDel read NOK");
    
    if (MSG_PERM_SEND_FILE_Delete_All (p_perm_send_file_w) == TRUE)
		printf("\nDel write OK\n");
	else printf("\nDel write NOK\n");

}