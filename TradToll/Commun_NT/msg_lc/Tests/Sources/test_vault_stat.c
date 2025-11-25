#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_Vault_Stat.h>
#include <stdio.h>
#include <conio.h>

void Test_Vault_Stat(void)
{

	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    DWORD size;


	//Declaration of Vault Status Message

    struct MSG_VAULT_STAT *p_vstat_w;
    struct MSG_VAULT_STAT_Vault_Info *p_vault_info;
	struct MSG_VAULT_STAT_Coin_Info *p_coin_info;
	struct MSG_VAULT_STAT vault_stat = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_VAULT_STAT_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

		// BODY : Rejected Coin Counter
        { { 999 } },

 		// VAULT INFORMATION : List
        { NULL },

    };

	GetLocalTime (&systime);

 
    p_vault_info = MSG_VAULT_STAT_Vault_Info_New (&vault_stat.list_vaults_in_lane);
    p_vault_info->time_of_insert = systime;
    p_vault_info->position = 1;
    p_vault_info->type = 1;
	p_vault_info->id = 1;
	p_vault_info->fill_percent=20;
	p_vault_info->list_coin_info=NULL;

	p_coin_info = MSG_VAULT_STAT_Coin_Info_New (&p_vault_info->list_coin_info);
    p_coin_info->coin_id = 1;
    p_coin_info->coin_counter = 99;

	p_coin_info = MSG_VAULT_STAT_Coin_Info_New (&p_vault_info->list_coin_info);
    p_coin_info->coin_id = 2;
    p_coin_info->coin_counter = 22;
    
	p_vault_info = MSG_VAULT_STAT_Vault_Info_New (&vault_stat.list_vaults_in_lane);
    p_vault_info->time_of_insert = systime;
    p_vault_info->position = 2;
    p_vault_info->type = 1;
	p_vault_info->id = 2;
	p_vault_info->fill_percent=30;
	p_vault_info->list_coin_info=NULL;

    p_coin_info = MSG_VAULT_STAT_Coin_Info_New (&p_vault_info->list_coin_info);
    p_coin_info->coin_id = 2;
    p_coin_info->coin_counter = 99999;
    
    p_vstat_w = MSG_VAULT_STAT_New ();
    *p_vstat_w = vault_stat;

    if (MSG_VAULT_STAT_Write (p_vstat_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w of Vault Status Message = %s", string_w);
    }
    else
        printf ("\nErreur !");

/*    p_vstat_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_vstat_r != NULL)
    {
        if (MSG_VAULT_STAT_Write (p_vstat_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r of Vault Status Message = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nVault Status Message Buffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");
	printf ("\nCD du message = %ld" , p_vstat_r->header.cd);

    MSG_VAULT_STAT_Delete_All (p_vstat_r);*/

	printf("\nCD of vault stat msg : %d\n", p_vstat_w->header.cd);

    MSG_VAULT_STAT_Delete_All (p_vstat_w);
}