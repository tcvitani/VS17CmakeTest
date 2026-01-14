#include <windows.h>
#include <stdio.h>
#include <acom.h>
#include <ntsvc.h>
#include <col.h>
#include <reg.h>

#include <trfsvc_res.h>

#include <trfsvc_glob.h>

#include <trfsvc_srv_callback.h>

#define LOC_DEF
#include <trfsvc_main.h>
#undef LOC_DEF

#include <memclass.h>

// TMA
PROTECTED DWORD WINAPI WM_Dispatch (PVOID param)
{
    MSG msg;
    INT ret;
	DWORD dwError = 0;
	DWORD dwDataSize = 0;
	DWORD64 dwInstUsrKey = 0;
	DWORD64 dwCnxUsrKey = 0;
	DWORD64 dwMsgUsrKey = 0;
	ACOM_CNX_HANDLE hCnxHandle = 0;
	void * poDataBuffer = NULL;
	HCOLLECTIONITEM hItem;
	
	NTSVCInfo("WM_Dispatch()");
	
    // Création d'une file de window messages pour ce thread
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	
    // boucle de réception des messages
    do
    {
        // attente d'un message
        ret = GetMessage (&msg, NULL, 0, 0);
		
        // erreur de réception
        if (ret < 0)
            continue;
		
        if (msg.message == gsSvcWork.WM_ACOM)
        {
			
            switch (msg.wParam)
            {
            case ACOM_WM_SHUTDOWN:
                NTSVC_ERR ("WM_Dispatch() : ACOM_WM_SHUTDOWN");
				PostThreadMessage(gsSvcWork.dwThreadReceivedId, gsSvcWork.WM_ACOM, ACOM_WM_SHUTDOWN, msg.lParam);
                ret = FALSE;
                break;
				
            case ACOM_WM_CONNECTION:
				NTSVCInfo("WM_Dispatch() ACOM_WM_CONNECTION");
				PostThreadMessage(gsSvcWork.dwThreadReceivedId, gsSvcWork.WM_ACOM, ACOM_WM_CONNECTION, msg.lParam);
                break;
				
            case ACOM_WM_DISCONNECTION:
				NTSVCInfo("WM_Dispatch() ACOM_WM_DISCONNECTION");
				PostThreadMessage(gsSvcWork.dwThreadReceivedId, gsSvcWork.WM_ACOM, ACOM_WM_DISCONNECTION, msg.lParam);
                break;
				
            case ACOM_WM_RECEIVED:
				NTSVCInfo("WM_Dispatch() ACOM_WM_RECEIVED");
				PostThreadMessage(gsSvcWork.dwThreadReceivedId, gsSvcWork.WM_ACOM, ACOM_WM_RECEIVED, msg.lParam);
                break;
				
            case ACOM_WM_SENT:
				NTSVCInfo("WM_Dispatch() ACOM_WM_SENT");
                AComGetSentEvtParameters(msg.lParam, &dwInstUsrKey, &dwCnxUsrKey, &dwMsgUsrKey, &dwError);
				// Get the item handle based on the connection handle in dwCnxUsrKey
				hItem = ColItemFind( gsSvcWork.hColServers, &dwCnxUsrKey );
				
				if ( hItem != NULL )
					SrvCallbackSent( dwInstUsrKey, (DWORD64)hItem, dwMsgUsrKey, dwError);
				
				// on libère l'event associé au message de ACOM
				AComReleaseEvt (msg.lParam);
                break;
				
            case ACOM_WM_TIME_SHIFT:
                break;
				
            default:
                NTSVC_ERR ("CMD_WM_Dispatch : WM_ACOM inconnu !!");
                break;
            }
            
        }
        else if (msg.message == WM_QUIT)
        {
            ret = 0;
            break;
        }
        else
        {
            NTSVC_ERR ("WM_Dispatch : WM inconnu !!");
        }
		
    }
    // ret == 0 sur réception de WM_QUIT
    while (ret > 0);
	
	return 0;
}
// end of TMA