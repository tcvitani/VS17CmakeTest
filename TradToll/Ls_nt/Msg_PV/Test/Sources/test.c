#include <msg_pv_header.h>
#include <msg_pv_log_req.h>
#include <msg_pv_log_rep.h>
#include <msg_pv_user_req.h>
#include <msg_pv_user_rep.h>
#include <msg_pv_auth_req.h>
#include <msg_pv_auth_rep.h>
#include <msg_pv_dec_req.h>
#include <msg_pv_conf_req.h>
#include <msg_pv_conf_rep.h>
#include <msg_pv_gal_req.h>
#include <msg_pv_plaz_rep.h>
#include <msg_pv_lane_rep.h>
#include <msg_pv_alar_rep.h>
#include <msg_pv_ack_req.h>
#include <msg_pv_com_req.h>
#include <msg_pv_bowl_rep.h>
#include <msg_pv_vaul_rep.h>
#include <msg_pv_serv_rep.h>
#include <msg_pv_trac_rep.h>

#include <stdio.h>
#include <conio.h>

void __cdecl test_msg_pv_log_req (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;

	// msg_pv_con_req
    
	struct MSG_PV_LOG_REQ *p_log_req;
	struct MSG_PV_LOG_REQ log_req = 
    {
        // HEADER : Id
        { 1, MSG_PV_LOG_REQ_CD },

        // BODY : Name,matricule,password
        { "didier",40472,"bonjour"}

    };
    
    p_log_req = MSG_PV_LOG_REQ_New ();
    *p_log_req = log_req;
   
    if (MSG_PV_LOG_REQ_Write (p_log_req, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {

        case MSG_PV_LOG_REQ_CD:
            p_log_req = (struct MSG_PV_LOG_REQ *)p_pv_header;
			if (MSG_PV_LOG_REQ_Write (p_log_req, string_w, sizeof(string_w), &size) == TRUE)
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

    MSG_PV_LOG_REQ_Delete_All (p_log_req);
    
    _getch();

}
    

void __cdecl test_msg_pv_log_rep (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;

	// msg_pv_log_rep
    
	struct MSG_PV_LOG_REP *p_log_rep;
	struct MSG_PV_LOG_REP log_rep = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_PV_LOG_REP_CD },

        // BODY : reply
        { "silber", "didier", 123456,"maitre", "Y"}

    };
    
    p_log_rep = MSG_PV_LOG_REP_New ();
    *p_log_rep = log_rep;
   
    if (MSG_PV_LOG_REP_Write (p_log_rep, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {

        case MSG_PV_LOG_REP_CD:
            p_log_rep = (struct MSG_PV_LOG_REP *)p_pv_header;
			if (MSG_PV_LOG_REP_Write (p_log_rep, string_w, sizeof(string_w), &size) == TRUE)
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

    MSG_PV_LOG_REP_Delete_All (p_log_rep);
    
    _getch();

}

void __cdecl test_msg_pv_user_req (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;

	// msg_pv_user_req
    
	struct MSG_PV_USER_REQ *p_user_req;
	struct MSG_PV_USER_REQ user_req = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_PV_USER_REQ_CD },

		// BODY : Name,matricule,password
        { "didier",40472,"bonjour","maitre"}

    };
    
    p_user_req = MSG_PV_USER_REQ_New ();
    *p_user_req = user_req;
   
    if (MSG_PV_USER_REQ_Write (p_user_req, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {

        case MSG_PV_USER_REQ_CD:
            p_user_req = (struct MSG_PV_USER_REQ *)p_pv_header;
			if (MSG_PV_USER_REQ_Write (p_user_req, string_w, sizeof(string_w), &size) == TRUE)
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

    MSG_PV_USER_REQ_Delete_All (p_user_req);
    
    _getch();

}


void __cdecl test_msg_pv_user_rep (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;
    struct MSG_PV_USER_REP *p_user_rep_w, *p_user_rep_r;
    struct MSG_PV_USER_REP_Function *p_fun;
	struct MSG_PV_USER_REP_Command *p_com;
    struct MSG_PV_USER_REP_Lanetype *p_typ;
    

    struct MSG_PV_USER_REP user_rep = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_USER_REP_CD },

        // FUNCTION : List
        { NULL },

        // COMMAND : List
        { NULL }

    };

	p_fun = MSG_PV_USER_REP_Function_New (&user_rep.list_function);
    p_fun->id = 1;
	p_fun->mode = 1;
	p_fun = MSG_PV_USER_REP_Function_New (&user_rep.list_function);
    p_fun->id = 2;
	p_fun->mode = 0;
	

    p_com = MSG_PV_USER_REP_Command_New (&user_rep.list_command);
    p_com->id = 1;
	p_com->mode = 1;
	strcpy_s(p_com->label, MSG_PV_MAX_COMMAND_LABEL, "Demande de restitution");
    p_typ = MSG_PV_USER_REP_Lanetype_New (&p_com->list_lanetype);
    p_typ->id = 1;
        
    p_com = MSG_PV_USER_REP_Command_New (&user_rep.list_command);
    p_com->id=2;
	p_com->mode = 1;
	strcpy_s(p_com->label, MSG_PV_MAX_COMMAND_LABEL, "Remplissage des bols");
    p_typ = MSG_PV_USER_REP_Lanetype_New (&p_com->list_lanetype);
    p_typ->id = 2;

	p_com = MSG_PV_USER_REP_Command_New (&user_rep.list_command);
    p_com->id=3;
	p_com->mode = 0;
	strcpy_s(p_com->label, MSG_PV_MAX_COMMAND_LABEL, "Consignes d'exploitation");
    p_typ = MSG_PV_USER_REP_Lanetype_New (&p_com->list_lanetype);
    p_typ->id = 1;
	p_typ = MSG_PV_USER_REP_Lanetype_New (&p_com->list_lanetype);
    p_typ->id = 2;
    
    p_user_rep_w = MSG_PV_USER_REP_New ();
    *p_user_rep_w = user_rep;

    if (MSG_PV_USER_REP_Write (p_user_rep_w, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {
        case MSG_PV_USER_REP_CD:
            p_user_rep_r = (struct MSG_PV_USER_REP *) p_pv_header;
            if (MSG_PV_USER_REP_Write (p_user_rep_w, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer r = [%s]", string_w);
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

    MSG_PV_USER_REP_Delete_All (p_user_rep_r);
    MSG_PV_USER_REP_Delete_All (p_user_rep_w);
    
    _getch();
}


void __cdecl test_msg_pv_auth_req (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;

	// msg_pv_auth_req
    
	struct MSG_PV_AUTH_REQ *p_auth_req;
	struct MSG_PV_AUTH_REQ auth_req = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_PV_AUTH_REQ_CD },

        // BODY : Name,matricule,password
        { "didier",40472,"bonjour",0,1}

    };
    
    p_auth_req = MSG_PV_AUTH_REQ_New ();
    *p_auth_req = auth_req;
   
    if (MSG_PV_AUTH_REQ_Write (p_auth_req, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {

        case MSG_PV_AUTH_REQ_CD:
            p_auth_req = (struct MSG_PV_AUTH_REQ *)p_pv_header;
			if (MSG_PV_AUTH_REQ_Write (p_auth_req, string_w, sizeof(string_w), &size) == TRUE)
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

    MSG_PV_AUTH_REQ_Delete_All (p_auth_req);
    
    _getch();

}

void __cdecl test_msg_pv_auth_rep (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;

	// msg_pv_auth_rep
    
	struct MSG_PV_AUTH_REP *p_auth_rep;
	struct MSG_PV_AUTH_REP auth_rep = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_PV_AUTH_REP_CD },

        // BODY : reply
        { "nom","prenom", 12345,"maitre","Y"}

    };
    
    p_auth_rep = MSG_PV_AUTH_REP_New ();
    *p_auth_rep = auth_rep;
   
    if (MSG_PV_AUTH_REP_Write (p_auth_rep, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {

        case MSG_PV_AUTH_REP_CD:
            p_auth_rep = (struct MSG_PV_AUTH_REP *)p_pv_header;
			if (MSG_PV_AUTH_REP_Write (p_auth_rep, string_w, sizeof(string_w), &size) == TRUE)
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

    MSG_PV_AUTH_REP_Delete_All (p_auth_rep);
    
    _getch();

}


void __cdecl test_msg_pv_dec_req (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;

	// msg_pv_dec_req
    
	struct MSG_PV_DEC_REQ *p_dec_req;
	struct MSG_PV_DEC_REQ dec_req = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_PV_DEC_REQ_CD },

        // BODY : Name,matricule,password
        { "didier",40472,"bonjour"}

    };
    
    p_dec_req = MSG_PV_DEC_REQ_New ();
    *p_dec_req = dec_req;
   
    if (MSG_PV_DEC_REQ_Write (p_dec_req, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {

        case MSG_PV_DEC_REQ_CD:
            p_dec_req = (struct MSG_PV_DEC_REQ *)p_pv_header;
			if (MSG_PV_DEC_REQ_Write (p_dec_req, string_w, sizeof(string_w), &size) == TRUE)
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

    MSG_PV_DEC_REQ_Delete_All (p_dec_req);
    
    _getch();

}


void __cdecl test_msg_pv_conf_req (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;

	// msg_pv_conf_req
    
	struct MSG_PV_CONF_REQ *p_conf_req;
	struct MSG_PV_CONF_REQ conf_req = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_PV_CONF_REQ_CD }

    };
    
    p_conf_req = MSG_PV_CONF_REQ_New ();
    *p_conf_req = conf_req;
   
    if (MSG_PV_CONF_REQ_Write (p_conf_req, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {

        case MSG_PV_CONF_REQ_CD:
            p_conf_req = (struct MSG_PV_CONF_REQ *)p_pv_header;
			if (MSG_PV_CONF_REQ_Write (p_conf_req, string_w, sizeof(string_w), &size) == TRUE)
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

    MSG_PV_CONF_REQ_Delete_All (p_conf_req);
    
    _getch();

}


void __cdecl test_msg_pv_conf_rep (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;
    struct MSG_PV_CONF_REP *p_conf_rep_w, *p_conf_rep_r;
    struct MSG_PV_CONF_REP_Lane *p_lan;
	struct MSG_PV_CONF_REP_Equipment *p_equi;
    struct MSG_PV_CONF_REP_Display *p_dis;
	struct MSG_PV_CONF_REP_Zone *p_zon;
	struct MSG_PV_CONF_REP_Lanenum *p_lnu;
    

    struct MSG_PV_CONF_REP conf_rep = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_CONF_REP_CD },

		// BODY : Name,password
        { 5,"CLAMART"},

        // LANE : List
        { NULL },

        // DISPLAY : List
        { NULL }

    };

	p_lan = MSG_PV_CONF_REP_Lane_New (&conf_rep.list_lane);
    p_lan->lanenum = 1;
	strcpy_s(p_lan->name, MSG_PV_MAX_LANE_NAME, "VM01");
	p_lan->type = 1;
	p_equi = MSG_PV_CONF_REP_Equipment_New (&p_lan->list_equipment);
    p_equi->id = 1;
	p_equi = MSG_PV_CONF_REP_Equipment_New (&p_lan->list_equipment);
    p_equi->id = 1;

    p_lan = MSG_PV_CONF_REP_Lane_New (&conf_rep.list_lane);
    p_lan->lanenum = 2;
	strcpy_s(p_lan->name, MSG_PV_MAX_LANE_NAME, "VM02");
	p_lan->type = 1;
	p_equi = MSG_PV_CONF_REP_Equipment_New (&p_lan->list_equipment);
    p_equi->id = 1;
	p_equi = MSG_PV_CONF_REP_Equipment_New (&p_lan->list_equipment);
    p_equi->id = 1;
	
	p_lan = MSG_PV_CONF_REP_Lane_New (&conf_rep.list_lane);
    p_lan->lanenum = 3;
	strcpy_s(p_lan->name, MSG_PV_MAX_LANE_NAME, "VM03");
	p_lan->type = 1;
	p_equi = MSG_PV_CONF_REP_Equipment_New (&p_lan->list_equipment);
    p_equi->id = 1;
	p_equi = MSG_PV_CONF_REP_Equipment_New (&p_lan->list_equipment);
    p_equi->id = 1;

	p_dis = MSG_PV_CONF_REP_Display_New (&conf_rep.list_display);
	p_dis->id = 1;
	p_zon = MSG_PV_CONF_REP_Zone_New (&p_dis->list_zone);
	p_zon->id = 1;
	p_lnu = MSG_PV_CONF_REP_Lanenum_New (&p_zon->list_lanenum);
	p_lnu->id = 1;
	p_zon = MSG_PV_CONF_REP_Zone_New (&p_dis->list_zone);
	p_zon->id = 2;
	p_lnu = MSG_PV_CONF_REP_Lanenum_New (&p_zon->list_lanenum);
	p_lnu->id = 2;
	
	p_dis = MSG_PV_CONF_REP_Display_New (&conf_rep.list_display);
	p_dis->id = 2;
	p_zon = MSG_PV_CONF_REP_Zone_New (&p_dis->list_zone);
	p_zon->id = 3;
	p_lnu = MSG_PV_CONF_REP_Lanenum_New (&p_zon->list_lanenum);
	p_lnu->id = 3;
	
    p_conf_rep_w = MSG_PV_CONF_REP_New ();
    *p_conf_rep_w = conf_rep;

    if (MSG_PV_CONF_REP_Write (p_conf_rep_w, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {
        case MSG_PV_CONF_REP_CD:
            p_conf_rep_r = (struct MSG_PV_CONF_REP *) p_pv_header;
            if (MSG_PV_CONF_REP_Write (p_conf_rep_w, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer r = [%s]", string_w);
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

    MSG_PV_CONF_REP_Delete_All (p_conf_rep_r);
    MSG_PV_CONF_REP_Delete_All (p_conf_rep_w);
    
    _getch();
}

void __cdecl test_msg_pv_gal_req (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;

	// msg_pv_gal_req
    
	struct MSG_PV_GAL_REQ *p_gal_req;
	struct MSG_PV_GAL_REQ gal_req = 
    {
        // HEADER : Id Plaza Lane Time Tci Avi
        { 1, MSG_PV_GAL_REQ_CD }

    };
    
    p_gal_req = MSG_PV_GAL_REQ_New ();
    *p_gal_req = gal_req;
   
    if (MSG_PV_GAL_REQ_Write (p_gal_req, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {

        case MSG_PV_GAL_REQ_CD:
            p_gal_req = (struct MSG_PV_GAL_REQ *)p_pv_header;
			if (MSG_PV_GAL_REQ_Write (p_gal_req, string_w, sizeof(string_w), &size) == TRUE)
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

    MSG_PV_GAL_REQ_Delete_All (p_gal_req);
    
    _getch();

}


void __cdecl test_msg_pv_plaz_rep (void)
{
	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;
    struct MSG_PV_PLAZ_REP *p_plaz_rep_w, *p_plaz_rep_r;
    struct MSG_PV_PLAZ_REP_Lane *p_lan;
	struct MSG_PV_PLAZ_REP_Alarm *p_alar;
	struct MSG_PV_PLAZ_REP_Display *p_dis;
    struct MSG_PV_PLAZ_REP plaz_rep = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_PLAZ_REP_CD },

		// BODY : plaza number
        { 1},

		// LANE
		{ NULL },

		// ALARM
		{ NULL },

        // DISPLAY : List
        { NULL }

    };

    struct MSG_PV_LANE_REP_Equipment *p_equi;
	struct MSG_PV_LANE_REP_Class *p_cla;
    struct MSG_PV_LANE_REP_Anomaly *p_ano;
    
    struct MSG_PV_LANE_REP lane_rep = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_LANE_REP_CD },

		// BODY : 
        { 1,{ 0 },1,1,1,222,333,444,"F","CASH",555555,666666},

        // EQUIPMENT : List
        { NULL },

        // CLASS : List
        { NULL },

		//ANOMALY : List
		{ NULL }

    };

    struct MSG_PV_ALAR_REP_Event *p_even;
    
    struct MSG_PV_ALAR_REP alar_rep = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_ALAR_REP_CD },

		// BODY : Name,password
        { 1,1},

        // EVENT : List
        { NULL }

    };

	GetLocalTime (&systime);

	// LANE

	lane_rep.body.date = systime;

	p_equi = MSG_PV_LANE_REP_Equipment_New (&lane_rep.list_equipment);
    p_equi->id = 1;
	p_equi->state = 1;
	p_equi = MSG_PV_LANE_REP_Equipment_New (&lane_rep.list_equipment);
    p_equi->id = 2;
	p_equi->state = 0;
	

    p_cla = MSG_PV_LANE_REP_Class_New (&lane_rep.list_class);
    p_cla->value = 1;
    p_cla = MSG_PV_LANE_REP_Class_New (&lane_rep.list_class);
    p_cla->value = 2;
    p_cla = MSG_PV_LANE_REP_Class_New (&lane_rep.list_class);
    p_cla->value = 3;
        
    p_ano = MSG_PV_LANE_REP_Anomaly_New (&lane_rep.list_anomaly);
    p_ano->id=1;
	strcpy_s(p_ano->label, MSG_PV_MAX_ANOMALY_LABEL, "simulation de boucle");

	p_ano = MSG_PV_LANE_REP_Anomaly_New (&lane_rep.list_anomaly);
    p_ano->id=2;
	strcpy_s(p_ano->label, MSG_PV_MAX_ANOMALY_LABEL, "annulation de classe");
    

	// ALARME

	p_even = MSG_PV_ALAR_REP_Event_New (&alar_rep.list_event);
    p_even->plazanum=1;
	p_even->lanenum=1;
	p_even->type=3;
	p_even->subtype=4;
	p_even->id=5;
	p_even->appdate = systime;
	p_even->ackdate = systime;
	p_even->matricule = 666666;
	p_even->level = 1;
	p_even->state = 1;
	strcpy_s(p_even->label, MSG_PV_MAX_ALARM_LABEL, "lecteur en panne");
	strcpy_s(p_even->instruction, MSG_PV_MAX_DETAIL_SIZE, "Instruction 0");
	strcpy_s(p_even->description, MSG_PV_MAX_ALARM_DESCRIPTION, "Description 0");

	p_even = MSG_PV_ALAR_REP_Event_New (&alar_rep.list_event);
    p_even->plazanum=1;
	p_even->lanenum=1;
	p_even->type=4;
	p_even->subtype=4;
	p_even->id=5;
	p_even->appdate = systime;
	p_even->ackdate = systime;
	p_even->level = 2;
	p_even->state = 0;
	strcpy_s(p_even->label, MSG_PV_MAX_ALARM_LABEL, "barriere en panne");
	strcpy_s(p_even->instruction, MSG_PV_MAX_DETAIL_SIZE, "Instruction 1");
	strcpy_s(p_even->description, MSG_PV_MAX_ALARM_DESCRIPTION, "Description 1");

	p_even = MSG_PV_ALAR_REP_Event_New (&alar_rep.list_event);
    p_even->plazanum=1;
	p_even->lanenum=1;
	p_even->type=5;
	p_even->subtype=4;
	p_even->id=5;
	p_even->appdate = systime;
	p_even->ackdate = systime;
	p_even->level = 3;
	p_even->state = 2;
	strcpy_s(p_even->label, MSG_PV_MAX_ALARM_LABEL, "voie ouverte");
	strcpy_s(p_even->instruction, MSG_PV_MAX_DETAIL_SIZE, "Instruction 2");
	strcpy_s(p_even->description, MSG_PV_MAX_ALARM_DESCRIPTION, "Description 2");

	// PLAZA

	p_lan= MSG_PV_PLAZ_REP_Lane_New (&plaz_rep.list_lane);
	p_lan->msg_pv_lane_rep=MSG_PV_LANE_REP_New ();
	*p_lan->msg_pv_lane_rep = lane_rep;
	
	
	p_alar = MSG_PV_PLAZ_REP_Alarm_New (&plaz_rep.list_alarm);
	p_alar->msg_pv_alar_rep=MSG_PV_ALAR_REP_New ();
	*p_alar->msg_pv_alar_rep = alar_rep;

	p_dis = MSG_PV_PLAZ_REP_Display_New (&plaz_rep.list_display);
    p_dis->id = 1;
	p_dis->total1 = 2;
	p_dis->total2 = 20000;
	p_dis->total3 = 15000;
	p_dis->total4 = 5000;
	p_dis->total5 = 0;

	p_dis = MSG_PV_PLAZ_REP_Display_New (&plaz_rep.list_display);
    p_dis->id = 2;
	p_dis->total1 = 3;
	p_dis->total2 = 200;
	p_dis->total3 = 0;
	p_dis->total4 = 0;
	p_dis->total5 = 0;

    p_plaz_rep_w = MSG_PV_PLAZ_REP_New ();
    *p_plaz_rep_w = plaz_rep;
	

    if (MSG_PV_PLAZ_REP_Write (p_plaz_rep_w, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {
        case MSG_PV_PLAZ_REP_CD:
            p_plaz_rep_r = (struct MSG_PV_PLAZ_REP *) p_pv_header;
            if (MSG_PV_PLAZ_REP_Write (p_plaz_rep_w, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer r = [%s]", string_w);
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

    MSG_PV_PLAZ_REP_Delete_All (p_plaz_rep_r);
    MSG_PV_PLAZ_REP_Delete_All (p_plaz_rep_w);
    
    _getch();
}

void __cdecl test_msg_pv_lane_rep (void)
{
	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;
    struct MSG_PV_LANE_REP *p_lane_rep_w, *p_lane_rep_r;
    struct MSG_PV_LANE_REP_Equipment *p_equi;
	struct MSG_PV_LANE_REP_Class *p_cla;
    struct MSG_PV_LANE_REP_Anomaly *p_ano;
    
    struct MSG_PV_LANE_REP lane_rep = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_LANE_REP_CD },

		// BODY : Name,password
        { 1,{ 0 },1,1,1,222,333,444,"F","CASH",555555,666666},

        // EQUIPMENT : List
        { NULL },

        // CLASS : List
        { NULL },

		//ANOMALY : List
		{ NULL }

    };

	GetLocalTime (&systime);

	lane_rep.body.date = systime;

	p_equi = MSG_PV_LANE_REP_Equipment_New (&lane_rep.list_equipment);
    p_equi->id = 12;
	p_equi->state = 1;
	p_equi = MSG_PV_LANE_REP_Equipment_New (&lane_rep.list_equipment);
    p_equi->id = 13;
	p_equi->state = 0;
	

    p_cla = MSG_PV_LANE_REP_Class_New (&lane_rep.list_class);
    p_cla->value = 1;
    p_cla = MSG_PV_LANE_REP_Class_New (&lane_rep.list_class);
    p_cla->value = 2;
    p_cla = MSG_PV_LANE_REP_Class_New (&lane_rep.list_class);
    p_cla->value = 3;
        
    p_ano = MSG_PV_LANE_REP_Anomaly_New (&lane_rep.list_anomaly);
    p_ano->id=1;
	strcpy_s(p_ano->label, MSG_PV_MAX_ANOMALY_LABEL, "simulation de boucle");

	p_ano = MSG_PV_LANE_REP_Anomaly_New (&lane_rep.list_anomaly);
    p_ano->id=2;
	strcpy_s(p_ano->label, MSG_PV_MAX_ANOMALY_LABEL, "annulation de transaction");

	p_ano = MSG_PV_LANE_REP_Anomaly_New (&lane_rep.list_anomaly);
    p_ano->id=3;
	strcpy_s(p_ano->label, MSG_PV_MAX_ANOMALY_LABEL, "annulation de classe");

    p_lane_rep_w = MSG_PV_LANE_REP_New ();
    *p_lane_rep_w = lane_rep;

    if (MSG_PV_LANE_REP_Write (p_lane_rep_w, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer w = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {
        case MSG_PV_LANE_REP_CD:
            p_lane_rep_r = (struct MSG_PV_LANE_REP *) p_pv_header;
            if (MSG_PV_LANE_REP_Write (p_lane_rep_w, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer r = [%s]", string_w);
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

    MSG_PV_LANE_REP_Delete_All (p_lane_rep_r);
    MSG_PV_LANE_REP_Delete_All (p_lane_rep_w);
    
    _getch();
}


void __cdecl test_msg_pv_alar_rep (void)
{
	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;
    struct MSG_PV_ALAR_REP *p_alar_rep_w, *p_alar_rep_r;
    struct MSG_PV_ALAR_REP_Event *p_even;
    
    struct MSG_PV_ALAR_REP alar_rep = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_ALAR_REP_CD },

		// BODY : Name,password
        { 1,66},

        // EVENT : List
        { NULL }

    };

	GetLocalTime (&systime);

	p_even = MSG_PV_ALAR_REP_Event_New (&alar_rep.list_event);
    p_even->plazanum=1;
	p_even->lanenum=2;
	p_even->type=3;
	p_even->subtype=4;
	p_even->id=5;
	p_even->appdate = systime;
	p_even->ackdate = systime;
	p_even->matricule = 666666;
	p_even->level = 7;
	p_even->state = 1;
	strcpy_s(p_even->label, MSG_PV_MAX_ALARM_LABEL, "lecteur machin en panne");
	strcpy_s(p_even->instruction, MSG_PV_MAX_ALARM_LABEL, "Instruction 0");
	strcpy_s(p_even->description, MSG_PV_MAX_ALARM_DESCRIPTION, "Description 0");

	p_even = MSG_PV_ALAR_REP_Event_New (&alar_rep.list_event);
    p_even->plazanum=1;
	p_even->lanenum=2;
	p_even->type=3;
	p_even->subtype=4;
	p_even->id=5;
	p_even->appdate = systime;
	p_even->ackdate = systime;
	p_even->matricule = 666666;
	p_even->level = 7;
	p_even->state = 1;
	strcpy_s(p_even->label, MSG_PV_MAX_ALARM_LABEL, "lecteur truc en panne");
	strcpy_s(p_even->instruction, MSG_PV_MAX_DETAIL_SIZE, "Appeler la maintenance");
	strcpy_s(p_even->description, MSG_PV_MAX_ALARM_DESCRIPTION, "Description 1");

    p_alar_rep_w = MSG_PV_ALAR_REP_New ();
    *p_alar_rep_w = alar_rep;

    if (MSG_PV_ALAR_REP_Write (p_alar_rep_w, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {
        case MSG_PV_ALAR_REP_CD:
            p_alar_rep_r = (struct MSG_PV_ALAR_REP *) p_pv_header;
            if (MSG_PV_ALAR_REP_Write (p_alar_rep_w, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer r = [%s]", string_w);
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

    MSG_PV_ALAR_REP_Delete_All (p_alar_rep_r);
    MSG_PV_ALAR_REP_Delete_All (p_alar_rep_w);
    
    _getch();
}

void __cdecl test_msg_pv_ack_req (void)
{
	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;
    struct MSG_PV_ACK_REQ *p_ack_req_w, *p_ack_req_r;
    struct MSG_PV_ACK_REQ_Event *p_even;
    
    struct MSG_PV_ACK_REQ ack_req = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_ACK_REQ_CD },

		// BODY : Name,password
        { 1,40472},

        // EVENT : List
        { NULL }

    };

	GetLocalTime (&systime);

	p_even = MSG_PV_ACK_REQ_Event_New (&ack_req.list_event);
    p_even->plazanum=1;
	p_even->lanenum=2;
	p_even->type=3;
	p_even->subtype=4;
	p_even->id=5;
	p_even->appdate = systime;

	p_even = MSG_PV_ACK_REQ_Event_New (&ack_req.list_event);
    p_even->plazanum=1;
	p_even->lanenum=2;
	p_even->type=3;
	p_even->subtype=4;
	p_even->id=5;
	p_even->appdate = systime;

    p_ack_req_w = MSG_PV_ACK_REQ_New ();
    *p_ack_req_w = ack_req;

    if (MSG_PV_ACK_REQ_Write (p_ack_req_w, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {
        case MSG_PV_ACK_REQ_CD:
            p_ack_req_r = (struct MSG_PV_ACK_REQ *) p_pv_header;
            if (MSG_PV_ACK_REQ_Write (p_ack_req_w, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer r = [%s]", string_w);
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

    MSG_PV_ACK_REQ_Delete_All (p_ack_req_r);
    MSG_PV_ACK_REQ_Delete_All (p_ack_req_w);
    
    _getch();
}


#define COMMAND "ici le corps\0 de la commande"
void __cdecl test_msg_pv_com_req (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;
    struct MSG_PV_COM_REQ *p_com_req_w, *p_com_req_r;
    struct MSG_PV_COM_REQ_Lanenum *p_lnu;
    
    struct MSG_PV_COM_REQ com_req = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_COM_REQ_CD },

		// BODY : Name,password
        { 1,2, sizeof(COMMAND), COMMAND},

        // LANENUM : List
        { NULL }

    };

	p_lnu = MSG_PV_COM_REQ_Lanenum_New (&com_req.list_lanenum);
	p_lnu->lanenum=1;

	p_lnu = MSG_PV_COM_REQ_Lanenum_New (&com_req.list_lanenum);
	p_lnu->lanenum=2;

	p_lnu = MSG_PV_COM_REQ_Lanenum_New (&com_req.list_lanenum);
	p_lnu->lanenum=3;

	p_lnu = MSG_PV_COM_REQ_Lanenum_New (&com_req.list_lanenum);
	p_lnu->lanenum=4;

    p_com_req_w = MSG_PV_COM_REQ_New ();
    *p_com_req_w = com_req;

    if (MSG_PV_COM_REQ_Write (p_com_req_w, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {
        case MSG_PV_COM_REQ_CD:
            p_com_req_r = (struct MSG_PV_COM_REQ *) p_pv_header;
            if (MSG_PV_COM_REQ_Write (p_com_req_w, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer r = [%s]", string_w);
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

    MSG_PV_COM_REQ_Delete_All (p_com_req_r);
    MSG_PV_COM_REQ_Delete_All (p_com_req_w);
    
    _getch();
}

void __cdecl test_msg_pv_bowl_rep (void)
{
	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;
    struct MSG_PV_BOWL_REP *p_bowl_rep_w, *p_bowl_rep_r;
    struct MSG_PV_BOWL_REP_Bowl *p_bow;
    
    struct MSG_PV_BOWL_REP bowl_rep = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_BOWL_REP_CD },

		// BODY : lanenum
        { 1 , {0},40472,656565},

        // BOWL : List
        { NULL }

    };

	GetLocalTime (&systime);
	bowl_rep.body.date = systime;

	p_bow = MSG_PV_BOWL_REP_Bowl_New (&bowl_rep.list_bowl);
    p_bow->position = 1;
	strcpy_s(p_bow->currency_label, MSG_PV_MAX_CURRENCY_LABEL, "F");
	strcpy_s(p_bow->coin_label, MSG_PV_MAX_COIN_LABEL, "1F");
	p_bow->remaining_coin_counter = 1111;
	p_bow->value = 1;
	p_bow->bag = 1000;
	p_bow->returned_coin_counter = 87654321;

	p_bow = MSG_PV_BOWL_REP_Bowl_New (&bowl_rep.list_bowl);
    p_bow->position = 2;
	strcpy_s(p_bow->currency_label, MSG_PV_MAX_CURRENCY_LABEL, "F");
	strcpy_s(p_bow->coin_label, MSG_PV_MAX_COIN_LABEL, "2F");
	p_bow->remaining_coin_counter = 2222;
	p_bow->value = 2;
	p_bow->bag = 100;
	p_bow->returned_coin_counter = 12345678;
	
	p_bow = MSG_PV_BOWL_REP_Bowl_New (&bowl_rep.list_bowl);
    p_bow->position = 3;
	strcpy_s(p_bow->currency_label, MSG_PV_MAX_CURRENCY_LABEL, "F");
	strcpy_s(p_bow->coin_label, MSG_PV_MAX_COIN_LABEL, "5F");
	p_bow->remaining_coin_counter = 3333;
	p_bow->value = 5;
	p_bow->bag = 500;
	p_bow->returned_coin_counter = 88888888;

    p_bowl_rep_w = MSG_PV_BOWL_REP_New ();
    *p_bowl_rep_w = bowl_rep;

    if (MSG_PV_BOWL_REP_Write (p_bowl_rep_w, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {
        case MSG_PV_BOWL_REP_CD:
            p_bowl_rep_r = (struct MSG_PV_BOWL_REP *) p_pv_header;
            if (MSG_PV_BOWL_REP_Write (p_bowl_rep_w, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer r = [%s]", string_w);
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

    MSG_PV_BOWL_REP_Delete_All (p_bowl_rep_r);
    MSG_PV_BOWL_REP_Delete_All (p_bowl_rep_w);
    
    _getch();
}

void __cdecl test_msg_pv_vaul_rep (void)
{
	SYSTEMTIME systime;
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;
    struct MSG_PV_VAUL_REP *p_vaul_rep_w, *p_vaul_rep_r;
    struct MSG_PV_VAUL_REP_Vault *p_vau;
	struct MSG_PV_VAUL_REP_Currency *p_mon;
	struct MSG_PV_VAUL_REP_Coin *p_coi;
    
    struct MSG_PV_VAUL_REP vaul_rep = 
    {
        // HEADER : Id CD
        { 1, MSG_PV_VAUL_REP_CD },

		// BODY : lanenum date collector_id active_vault rejected_coin_counter
		{ 5,{0},404072,555555 },

        // VAULT : List
        { NULL }

    };

	GetLocalTime (&systime);
	vaul_rep.body.date = systime;

	p_vau = MSG_PV_VAUL_REP_Vault_New (&vaul_rep.list_vault);
	p_vau->date=systime;
	p_vau->position = 1;
	p_vau->state = 1;
	p_vau->type = 3;
	p_vau->id = 666666;
	p_vau->percent=99;

	p_mon = MSG_PV_VAUL_REP_Currency_New (&p_vau->list_currency);
	strcpy_s(p_mon->label, MSG_PV_MAX_CURRENCY_LABEL, "F");
	p_mon->value = 6000;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "1F");
	p_coi->counter = 10;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "2F");
	p_coi->counter = 5;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "5F");
	p_coi->counter = 2;

	p_vau = MSG_PV_VAUL_REP_Vault_New (&vaul_rep.list_vault);
	p_vau->date=systime;
	p_vau->position = 2;
	p_vau->state = 0;
	p_vau->type = 3;
	p_vau->id = 555555;
	p_vau->percent=50;

	p_mon = MSG_PV_VAUL_REP_Currency_New (&p_vau->list_currency);
	strcpy_s(p_mon->label, MSG_PV_MAX_CURRENCY_LABEL, "F");
	p_mon->value = 1000;
	
	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "1F");
	p_coi->counter = 4;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "2F");
	p_coi->counter = 2;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "5F");
	p_coi->counter = 1;

	p_vau = MSG_PV_VAUL_REP_Vault_New (&vaul_rep.list_vault);
	p_vau->date=systime;
	p_vau->position = 3;
	p_vau->state = 0;
	p_vau->type = 3;
	p_vau->id = 555555;
	p_vau->percent=50;

	p_mon = MSG_PV_VAUL_REP_Currency_New (&p_vau->list_currency);
	strcpy_s(p_mon->label, MSG_PV_MAX_CURRENCY_LABEL, "F");
	p_mon->value = 2000;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "1F");
	p_coi->counter = 4;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "2F");
	p_coi->counter = 2;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "5F");
	p_coi->counter = 1;

	p_vau = MSG_PV_VAUL_REP_Vault_New (&vaul_rep.list_vault);
	p_vau->date=systime;
	p_vau->position = 4;
	p_vau->state = 0;
	p_vau->type = 3;
	p_vau->id = 555555;
	p_vau->percent=50;

	p_mon = MSG_PV_VAUL_REP_Currency_New (&p_vau->list_currency);
	strcpy_s(p_mon->label, MSG_PV_MAX_CURRENCY_LABEL, "F");
	p_mon->value = 5000;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "1F");
	p_coi->counter = 4;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "2F");
	p_coi->counter = 2;

	p_coi = MSG_PV_VAUL_REP_Coin_New (&p_mon->list_coin);
	strcpy_s(p_coi->label, MSG_PV_MAX_COIN_LABEL, "5F");
	p_coi->counter = 1;

    p_vaul_rep_w = MSG_PV_VAUL_REP_New ();
    *p_vaul_rep_w = vaul_rep;

    if (MSG_PV_VAUL_REP_Write (p_vaul_rep_w, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_w[size] = '\0';
        printf ("\nBuffer w = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {
        case MSG_PV_VAUL_REP_CD:
            p_vaul_rep_r = (struct MSG_PV_VAUL_REP *) p_pv_header;
            if (MSG_PV_VAUL_REP_Write (p_vaul_rep_w, string_w, sizeof(string_w), &size) == TRUE)
			{
				string_w[size] = '\0';
				printf ("\nBuffer r = [%s]", string_w);
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

    MSG_PV_VAUL_REP_Delete_All (p_vaul_rep_r);
    MSG_PV_VAUL_REP_Delete_All (p_vaul_rep_w);
    
    _getch();
}


void __cdecl test_msg_pv_serv_rep (void)
{
    CHAR string_w[5000] = {0};
    CHAR string_r[5000] = {0};
    CHAR string_t[5000] = {0};
    DWORD size;
    struct MSG_PV_HEADER *p_pv_header;

	// msg_pv_serv_rep
    
	struct MSG_PV_SERV_REP *p_serv_rep;
	struct MSG_PV_SERV_REP serv_rep = 
    {
        // HEADER : 
        { 1, MSG_PV_SERV_REP_CD },

        // BODY : message
        { "Attention, shutdown du serveur de PV dans 30s"}

    };
    
    p_serv_rep = MSG_PV_SERV_REP_New ();
    *p_serv_rep = serv_rep;
   
    if (MSG_PV_SERV_REP_Write (p_serv_rep, string_t, sizeof(string_t), &size) == TRUE)
    {
        string_t[size] = '\0';
        printf ("\nBuffer t = [%s]", string_t);
    }
    else
        printf ("\nErreur !");

    p_pv_header = MSG_New_Read_If_Found (NULL, string_t, sizeof(string_t));

    if (p_pv_header != NULL)
    {
        printf ("\nCD du message = %ld" , p_pv_header->cd);
        switch (p_pv_header->cd)
        {

        case MSG_PV_SERV_REP_CD:
            p_serv_rep = (struct MSG_PV_SERV_REP *)p_pv_header;
			if (MSG_PV_SERV_REP_Write (p_serv_rep, string_w, sizeof(string_w), &size) == TRUE)
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

    MSG_PV_SERV_REP_Delete_All (p_serv_rep);
    
    _getch();

}

void __cdecl main (void)
{
	test_msg_pv_log_req();
	test_msg_pv_log_rep();

	test_msg_pv_user_req();
	test_msg_pv_user_rep();

	test_msg_pv_auth_req();
	test_msg_pv_auth_rep();

	test_msg_pv_dec_req();

	test_msg_pv_conf_req();
	test_msg_pv_conf_rep();

	test_msg_pv_gal_req();

	test_msg_pv_plaz_rep();

	test_msg_pv_lane_rep();

	test_msg_pv_alar_rep();

	test_msg_pv_ack_req();

	test_msg_pv_com_req();

	test_msg_pv_bowl_rep();

	test_msg_pv_vaul_rep();

	test_msg_pv_serv_rep();
}