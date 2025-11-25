#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_vault_ins.h>
#include <stdio.h>
#include <conio.h>

void Test_Vault_Ins(void)
{
	
	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    DWORD size;

	//Declaration of Valut Insertion

	struct MSG_VAULT_INS *p_vault_ins_w = NULL;
	struct MSG_VAULT_INS *p_vault_ins_r = NULL;

    struct MSG_VAULT_INS vault_ins = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_VAULT_INS_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Time payment
        { { 0 },1,2,3 },

    };

    GetLocalTime (&systime);

	//  Msg_Vault_Ins

    vault_ins.body.time_of_vault_insertion = systime;

     p_vault_ins_w = MSG_VAULT_INS_New ();
    *p_vault_ins_w = vault_ins;

    if (MSG_VAULT_INS_Write (p_vault_ins_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w of Vault Insertion Message = %s", string_w);
    }
    else
        printf ("\nError !");

/*    p_vault_ins_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_vault_ins_r != NULL)
    {
	    if (MSG_VAULT_INS_Write (p_vault_ins_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r of Vault Insertion Message = %s", string_r);
        }
        else
            printf ("\nErrour !");
    }

    printf ("\nVault Insertion Message Buffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");
	printf ("\nCD du message = %ld" , p_vault_ins_r->header.cd);

    MSG_VAULT_INS_Delete_All (p_vault_ins_r);*/

	printf("\nCD of vault ins msg : %d\n", p_vault_ins_w->header.cd);
    MSG_VAULT_INS_Delete_All (p_vault_ins_w);

}