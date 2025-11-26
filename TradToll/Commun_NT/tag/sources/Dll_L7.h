//=================================================================
//  Company  :    THALES -e- transactions
//  Project  :    INTERFACE  TGB
//              Header File dll used with TGB dll
//  
//  Name file  :    DLL_L7.h
//
//  Author(s)  :  Doucet Christophe
//                Garoscio Michael (upgrade)
//  
//
//  Creation      : 26.11.2001
// 
//
//=================================================================

/////////////////////////////////////////////////////////////////////
// Description of return values. The header of each function
// allows to know function return values.
//
// Positive or null values : 0 : Function executed with success.
//                           1 : Response not received, OBE absent or broken link.
//                           3 : Request not sent, OBE absent or broken link.
//                           4 : Function executed with success but multi-execution is posssible.
// Negative values          -1 : The rate is different of 9600 bit/s or 115200 bit/s.
//                          -2 : The serial com port COM1 or COM2 is not opened by OpenComPort function.
//                          -3 : The execution of a system function failed.
//                          -5 : The execution of a DLL_L2 function into DLL_L7_OpenComPort failed.
//                          -6 : Invalid size of buffer to send.
//                          -7 : Error, first frame send as last frame (LastFrame = 1).
//                          -8 : The initialisation file was not downloaded with success. (Only for TGB).
//                          -9 : The initialisation file has one or several errors (Only for TGB).
//                          -10: The dll is already opened by an other application.
//                          -11: DLL_L7_OpenComPort service failed or was not called before.
//                          -12: Error from link equipment (Only TGB & PERTEL).
//                          -13: Illegal frame (Only TGB & PERTEL).
/////////////////////////////////////////////////////////////////////

#ifndef DLL_L7_H
#define DLL_L7_H

#define DLL_L7_MAX_SIZE_VERSION               120  // maximum size buffer version
#define DLL_L7_BEACONID_SIZE                    6  // size buffer BeaconId

#define DLL_L7_SUCCESS                          0  // Function executed with success.
#define DLL_L7_RECEIVE_TIMEOUT                  1  // Response not received, OBE absent or broken link.
#define DLL_L7_SEND_FAILED                      3  // Request not sent, OBE absent or broken link.
#define DLL_L7_SUCCESS_WITH_DOUBLE_RUN_POSSIBLE 4  // Function executed with success but multi-execution is posssible
#define DLL_L7_ILLEGAL_RATE                    -1  // The rate is different of 9600 bit/s or 115200 bit/s.
#define DLL_L7_PORTCOM_NOT_OPENED              -2  // The serial com port COM1 or COM2 is not opened by OpenComPort function.
#define DLL_L7_ERROR_SYSTEM                    -3  // The execution of a system function failed.
#define DLL_L7_ERROR_DLL_L2                    -5  // The execution of a DLL_L2 function into DLL_L7_OpenComPort failed.
#define DLL_L7_ERROR_MAX_SIZE                  -6  // Invalid size of buffer to send.
#define DLL_L7_ERROR_LAST_FRAME                -7  // Error, first frame send as last frame (LastFrame = 1).
#define DLL_L7_ERROR_DOWNLOAD_FAILED           -8  // The initialisation file was not downloaded with success. (Only for TGB)
#define DLL_L7_ERROR_DOWNLOAD_FILE             -9  // The initialisation file has one or several errors (Only for TGB).
#define DLL_L7_ERROR_IS_OPENED                 -10 // The dll is already opened by an other application.
#define DLL_L7_ERROR_NOT_OPENED                -11 // DLL_L7_OpenComPort service failed or was not called before.
#define DLL_L7_EQUIPMENT_ERROR                 -12 // Error from link equipment (Only TGB & PERTEL).
#define DLL_L7_ILLEGAL_FRAME                   -13 // Illegal frame (Only TGB & PERTEL).


#define FNOPEN            "DLL_L7_OpenComPort"       //  Open Port Function Name
#define FNCLOSE           "DLL_L7_CloseComPort"      //  Close Port Function Name
#define FNWRITEREAD       "DLL_L7_WriteReadData"     //  WriteRead Function Name
#define FNGETVERSION      "DLL_L7_GetVersion"        //  GetVersion Function Name


// Prototype functions

// DLL_L7_OpenComPort prototype function
typedef int (__stdcall* LPFNOPEN)       (char *port,DWORD rate);

// DLL_L7_CloseComPort prototype function
typedef int (__stdcall* LPFNCLOSE)      ();


// DLL_L7_WriteReadData prototype function
typedef int (__stdcall* LPFNWRITEREAD)  (
                                         unsigned char *datatowrite,
                                         int tailledatatowrite,
                                         unsigned char *dataread,
                                         int *tailledataread,
                                         char last_frame
                                        );

// DLL_L7_GetVersion prototype function
typedef int (__stdcall* LPFNGETVERSION) (
                                         char *versionDll_L7Para,
                                         char *versionDll_L2Para,
                                         char *versionEquipmentPara
                                        );


///////////////////// TGB SPECIFIC SERVICES /////////////////////////

#define FNGETBEACONID     "DLL_L7_GetBeaconId"       //  GetBeaconId Function Name
#define FNSETDOWNLOADFILE "DLL_L7_SetDownloadFile"   //  SetDownloadFile Function Name

// DLL_L7_GetBeaconId prototype function
typedef int (__stdcall* LPFNGETBEACONID)(unsigned char *beaconid);

// DLL_L7_SetDownloadFile prototype function
typedef int (__stdcall* LPFNSETDOWNLOADFILE)(const char *name);

#endif
