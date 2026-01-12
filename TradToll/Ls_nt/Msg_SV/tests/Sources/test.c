#include <csr_msg.h>
#include <msg_sv_header.h>
#include <msg_sv_filt_dec.h>
#include <msg_sv_con_req.h>
#include <msg_sv_con_rep.h>

#include <stdio.h>
#include <conio.h>

void __cdecl test_msg_sv_filt_dec (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_SV_HEADER *p_header;
	struct MSG_SV_FILT_DEC_Idcd *p_idcd;

	// msg_sv_filt_dec
    
	struct MSG_SV_FILT_DEC *p_filt_dec;
	struct MSG_SV_FILT_DEC filt_dec = 
    {
        // HEADER : Id
        { 1, MSG_SV_FILT_DEC_CD },

        // BODY : Name,matricule,password
        { NULL}

    };
    
	p_idcd = MSG_SV_FILT_DEC_Idcd_New (&filt_dec.list_idcd);
    p_idcd->id = 1;
	p_idcd->cd = 2;

	p_idcd = MSG_SV_FILT_DEC_Idcd_New (&filt_dec.list_idcd);
    p_idcd->id = 3;
	p_idcd->cd = 4;

	p_idcd = MSG_SV_FILT_DEC_Idcd_New (&filt_dec.list_idcd);
    p_idcd->id = 5;
	p_idcd->cd = 6;

	p_idcd = MSG_SV_FILT_DEC_Idcd_New (&filt_dec.list_idcd);
    p_idcd->id = 7;
	p_idcd->cd = 8;

	p_idcd = MSG_SV_FILT_DEC_Idcd_New (&filt_dec.list_idcd);
    p_idcd->id = 9;
	p_idcd->cd = 10;

    p_filt_dec = MSG_SV_FILT_DEC_New ();
    *p_filt_dec = filt_dec;
   
    if (MSG_SV_FILT_DEC_Write (p_filt_dec, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {

        case MSG_SV_FILT_DEC_CD:
            p_filt_dec = (struct MSG_SV_FILT_DEC *)p_header;
			if (MSG_SV_FILT_DEC_Write (p_filt_dec, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer w = [%s]", string_w);
			}
			else
				printf ("\nErreur !");

			printf (" Identique : %s\n", strcmp (string_t, string_w) == 0 ? "YES" : "NO" );

            break;

        default:
            // INCONNU...
            break;
        }
    }
    else
        printf ("\nCD inconnu !");

    MSG_SV_FILT_DEC_Delete_All (p_filt_dec);
    
    _getch();

}

void __cdecl test_msg_sv_con_req (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_SV_HEADER *p_header;

	// msg_sv_con_req
    
	struct MSG_SV_CON_REQ *p_con_req;
	struct MSG_SV_CON_REQ con_req = 
    {
        // HEADER : Id
        { 1, MSG_SV_CON_REQ_CD }

    };
    
    p_con_req = MSG_SV_CON_REQ_New ();
    *p_con_req = con_req;
   
    if (MSG_SV_CON_REQ_Write (p_con_req, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {

        case MSG_SV_CON_REQ_CD:
            p_con_req = (struct MSG_SV_CON_REQ *)p_header;
			if (MSG_SV_CON_REQ_Write (p_con_req, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer w = [%s]", string_w);
			}
			else
				printf ("\nErreur !");

			printf (" Identique : %s\n", strcmp (string_t, string_w) == 0 ? "YES" : "NO" );

            break;

        default:
            // INCONNU...
            break;
        }
    }
    else
        printf ("\nCD inconnu !");

    MSG_SV_CON_REQ_Delete_All (p_con_req);
    
    _getch();

}

void __cdecl test_msg_sv_con_rep (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_SV_HEADER *p_header;

	// msg_sv_con_req
    
	struct MSG_SV_CON_REP *p_con_req;
	struct MSG_SV_CON_REP con_req = 
    {
        // HEADER : Id
        { 1, MSG_SV_CON_REP_CD },

		// body

		{1,2,1}

    };
    
    p_con_req = MSG_SV_CON_REP_New ();
    *p_con_req = con_req;
   
    if (MSG_SV_CON_REP_Write (p_con_req, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_header->cd);
        switch (p_header->cd)
        {

        case MSG_SV_CON_REP_CD:
            p_con_req = (struct MSG_SV_CON_REP *)p_header;
			if (MSG_SV_CON_REP_Write (p_con_req, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer w = [%s]", string_w);
			}
			else
				printf ("\nErreur !");

			printf (" Identique : %s\n", strcmp (string_t, string_w) == 0 ? "YES" : "NO" );

            break;

        default:
            // INCONNU...
            break;
        }
    }
    else
        printf ("\nCD inconnu !");

    MSG_SV_CON_REP_Delete_All (p_con_req);
    
    _getch();

}

void __cdecl main (void)
{
	test_msg_sv_filt_dec();

	test_msg_sv_con_req();

	test_msg_sv_con_rep();
	
}