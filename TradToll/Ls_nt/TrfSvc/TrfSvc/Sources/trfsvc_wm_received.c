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
PROTECTED DWORD WINAPI WM_Received (PVOID param)
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
	
	NTSVCInfo("WM_Received()");
	
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
                NTSVC_ERR ("WM_Received() : ACOM_WM_SHUTDOWN");
                ret = FALSE;
                break;
			case ACOM_WM_CONNECTION:
				NTSVCInfo("WM_Received() ACOM_WM_CONNECTION");
				AComGetConnectionEvtParameters(msg.lParam, &dwInstUsrKey, &hCnxHandle);
				SrvCallbackConnection(dwInstUsrKey, hCnxHandle);
				
				// on libère l'event associé au message de ACOM
				AComReleaseEvt (msg.lParam);
                break;
				
            case ACOM_WM_DISCONNECTION:
				NTSVCInfo("WM_Received() ACOM_WM_DISCONNECTION");
				AComGetDisconnectionEvtParameters(msg.lParam, &dwInstUsrKey, &dwCnxUsrKey);
				// Get the item handle based on the connection handle in dwCnxUsrKey
				hItem = ColItemFind( gsSvcWork.hColServers, &dwCnxUsrKey );
				
				if ( hItem != NULL )
					SrvCallbackDisconnection(dwInstUsrKey, (DWORD64)hItem);
				
				// on libère l'event associé au message de ACOM
				AComReleaseEvt (msg.lParam);
                break;
				
            case ACOM_WM_RECEIVED:
				NTSVCInfo("WM_Received() -> ACOM_WM_RECEIVED");
				AComGetReceivedEvtParameters(msg.lParam, &dwInstUsrKey, &dwCnxUsrKey, &dwDataSize, &poDataBuffer);
				// Get the item handle based on the connection handle in dwCnxUsrKey
				hItem = ColItemFind( gsSvcWork.hColServers, &dwCnxUsrKey );
				
				if ( hItem != NULL )
					SrvCallbackReceived(dwInstUsrKey, (DWORD64)hItem, dwDataSize, (BYTE  *)poDataBuffer);
				
				// on libère l'event associé au message de ACOM
				AComReleaseEvt (msg.lParam);
                break;
				
            default:
                NTSVC_ERR ("WM_Received : WM_ACOM inconnu !!");
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
            NTSVC_ERR ("WM_Received : WM inconnu !!");
        }
		
    }
    // ret == 0 sur réception de WM_QUIT
    while (ret > 0);
	
	return 0;
}
// end of TMA