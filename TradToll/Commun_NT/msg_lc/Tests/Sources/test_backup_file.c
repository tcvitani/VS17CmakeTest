// Test for Backup by File Message
#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_backup_file.h>
#include <stdio.h>
#include <conio.h>

void Test_Backup_File(void)
{
    SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_HEADER *p_header;
    struct MSG_BACKUP_FILE *p_backup_file_w, *p_backup_file_r;

    struct MSG_BACKUP_FILE backup_file = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_BACKUP_FILE_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : BeginFileNumber EndFileNumber
        { 7, 7272 },
        
    };
 

    GetLocalTime (&systime);


    
    p_backup_file_w = MSG_BACKUP_FILE_New ();
    *p_backup_file_w = backup_file;
	
	if (MSG_BACKUP_FILE_Write (p_backup_file_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = %s", string_w);
    }
    else
        printf ("\nError !");

    p_backup_file_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_backup_file_r != NULL)
    {
        if (MSG_BACKUP_FILE_Write (p_backup_file_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r = %s", string_r);
        }
        else
            printf ("\nError !");
    }

    printf ("\nBuffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");

    MSG_BACKUP_FILE_Delete_All (p_backup_file_r);
    MSG_BACKUP_FILE_Delete_All (p_backup_file_w);


    p_header = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_header != NULL)
    {
        printf ("\nCD of message = %ld\n" , p_header->cd);
        switch (p_header->cd)
        {
 
        case MSG_BACKUP_FILE_CD:
            p_backup_file_r = (struct MSG_BACKUP_FILE *) p_header;
            // TO DO...
            break;
        default:
            // INCONNU...
            break;
        }
    }
    else
        printf ("\nCD unknown !");

}
    