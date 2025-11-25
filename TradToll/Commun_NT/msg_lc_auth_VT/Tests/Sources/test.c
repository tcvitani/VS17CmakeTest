#include <csr_msg.h>
#include <msg_lc_header.h>
#include <stdio.h>
#include <conio.h>

extern void Test_Auth_Vt_enl_req(void);
extern void Test_Auth_Vt_enl_rep(void);
extern void Test_Auth_Vt_exl_req(void);
extern void Test_Auth_Vt_exl_rep(void);
extern void Test_Auth_Vt_enl_req_v2(void);
extern void Test_Auth_Vt_enl_rep_v2(void);
extern void Test_Auth_Vt_exl_req_v2(void);
extern void Test_Auth_Vt_exl_rep_v2(void);
extern void Test_Auth_Vt_enl_req_v3(void);
extern void Test_Auth_Vt_enl_rep_v3(void);
extern void Test_Auth_Vt_exl_req_v3(void);
extern void Test_Auth_Vt_exl_rep_v3(void);


void _cdecl main (void)
{
	Test_Auth_Vt_enl_req();
	_getch();
	Test_Auth_Vt_enl_rep();
	_getch();
	Test_Auth_Vt_exl_req();
	_getch();
	Test_Auth_Vt_exl_rep();
	_getch();
	Test_Auth_Vt_enl_req_v2();
	_getch();			
	Test_Auth_Vt_enl_rep_v2();
	_getch();			
	Test_Auth_Vt_exl_req_v2();
	_getch();			
	Test_Auth_Vt_exl_rep_v2();
	_getch();
	Test_Auth_Vt_enl_req_v3();
	_getch();
	Test_Auth_Vt_enl_rep_v3();
	_getch();
	Test_Auth_Vt_exl_req_v3();
	_getch();
	Test_Auth_Vt_exl_rep_v3();
	_getch();
}
    