#include <csr_msg.h>
#include <msg_lc_header.h>
#include <msg_lc_comp_inf_tr.h>
#include <stdio.h>
#include <conio.h>

void Test_Comp_Inf_Tr(void)
{

	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    DWORD size;


	//Declaration of Complementary Information Transaction Message

    struct MSG_COMP_INF_TR *p_comp_w;
    struct MSG_COMP_INF_TR_Elem_Info *p_elem_info;
	struct MSG_COMP_INF_TR_Info_Type *p_info_type;
	struct MSG_COMP_INF_TR comp_inf = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_COMP_INF_TR_CD, 1, 1, { 12121 }, { 0 }, 100010UL, 0 },

        // BODY : Time payment
        { { 0 } },

        // TRS REF : Id Start time
        { 12345, { 0 } },

 		// INFORMATION TYPE : List
        { NULL },

    };

	GetLocalTime (&systime);

    comp_inf.body.time_of_comp_inf = systime;
    comp_inf.ref_trs.start_time = systime;

    p_info_type = MSG_COMP_INF_TR_Info_Type_New (&comp_inf.list_comp_inf);
    p_info_type->type = 22;
	p_info_type->list_info_type=NULL;

	p_elem_info = MSG_COMP_INF_TR_Elem_Info_New (&p_info_type->list_info_type);
    p_elem_info->code = 1;
    p_elem_info->info_value.type = MSG_FIELD_VARSTR;
	strcpy_s(p_elem_info->info_value.String, sizeof(p_elem_info->info_value.String), "Hello World");

	p_elem_info = MSG_COMP_INF_TR_Elem_Info_New (&p_info_type->list_info_type);
    p_elem_info->code = 2;
    p_elem_info->info_value.type = MSG_FIELD_VARSTR;
	strcpy_s(p_elem_info->info_value.String, sizeof(p_elem_info->info_value.String), "Info");
	
    p_comp_w = MSG_COMP_INF_TR_New ();
    *p_comp_w = comp_inf;

    if (MSG_COMP_INF_TR_Write (p_comp_w, string_w, sizeof(string_w), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w of Complementary Information Transaction Message = %s", string_w);
    }
    else
        printf ("\nErreur !");

/*    p_comp_r = MSG_New_Read_If_Found (NULL, string_w, sizeof(string_w));

    if (p_comp_r != NULL)
    {
        if (MSG_COMP_INF_TR_Write (p_comp_r, string_r, sizeof(string_r), &size) == TRUE)
        {
            string_r[size] = '\0';
            printf ("\nBuffer r of Complementary Information Transaction Message = %s", string_r);
        }
        else
            printf ("\nErreur !");
    }

    printf ("\nComplementary Information Transaction Message Buffer w = buffer r : %s", strcmp (string_w, string_r) == 0 ? "YES" : "NO");
	printf ("\nCD du message = %ld" , p_comp_r->header.cd);

    MSG_COMP_INF_TR_Delete_All (p_comp_r);*/
	printf("CD of complementary information transaction msg : %d\n", p_comp_w->header.cd);

    MSG_COMP_INF_TR_Delete_All (p_comp_w);

}