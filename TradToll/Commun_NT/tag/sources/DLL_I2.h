//=================================================================
//  Company  :		THALES -e- transactions
//  Project  :		INTERFACE  THP
//				    Header File dll used with THP dll
//  
//  Name file  :    DLL_I2.h
//
//  Author(s)  :	Doucet Christophe
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
// Positive or null values : 0 : function called with SUCCESS
//                           0 to 7 :
//				     1 to 3 bit values ar as follow:
//
//                   			b0 0 : response received
//                      		   1 : response not received
//                   
//                  			b1 0 : L7 command was send
//                                 1 : L7 command was not send
//
//                               b2 0: double L7 command run is not enable
//                                  1: double L7 command run is enable
//
// Negative values          -1 : Invalid rate
//                          -2 : Port com is not open
//                          -3 : System function failed
//////////////////////////////////////////////////////////////////////




// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DLL_I2_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DLL_I2_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef DLL_I2_EXPORTS
#define DLL_I2_API //__declspec(dllexport)
#else
#define DLL_I2_API //__declspec(dllimport)
#endif

#define DLL_I2_SUCCESS                           0
#define DLL_I2_RECEIVE_TIMEOUT                   1
#define DLL_I2_SEND_FAILED                       3
#define DLL_I2_SUCCESS_WITH_DOUBLE_RUN_POSSIBLE  4
#define DLL_I2_ILLEGAL_RATE	                    -1
#define DLL_I2_PORTCOM_NOT_OPENED               -2
#define DLL_I2_ERROR_SYSTEM                     -3

//===================================================================
// POUR LA SUPERVISION DE LA COUCHE I2
typedef struct { long appel_fct;// nombre d'appel à la fct WriteRead
				long AP,AS;		// Activation pattern reçus et emis
				long RR,RE;		// blocs R  reçus et emis
				long SR,SE;		// blocs Synchro  reçus et emis
				long WR,WE;     // blocs S(WTX) reçus et emis
				long IR,IE;		// blocs I  reçus et emis
				long reprise;
				long TO;		// Time-out global
				long EchecWU1;	// Time-out wakeup au 1er essai
				long EchecWU2;	// Time-out au 2 essai wakeup
				 } cpt;
// Cette structure est initialisée par l'application cliente.
// Ces données sont également affichées par l'App cliente.
extern DLL_I2_API cpt compteurs;
extern DLL_I2_API char  Version_DLL_I2[];


const char * FNOPENI2 = "DLL_I2_OpenComPort";        //  Open Port Fonction Name
const char * FNCLOSEI2 = "DLL_I2_CloseComPort";      //  Close Port Fonction Name
const char * FNWRITEREADI2 = "DLL_I2_WriteReadData"; //  WriteRead Fonction Name



//===================================================================
//   Ouverture et fermeture de la liaison série


//=====================================================================
//  Function : DLL_I2_OpenComport	
//          	Open a communication
//  Parameters   : 
//  (Character String) *port   : Communication Port
//	 (DWORD)            rate    : Data Transfert Rate
//				
//  Return value :  0  Success
//                 -1  Illegal Rate
//                 -2  Port com is not open 
//                 -3  System Error  
//====================================================================


typedef int  (__stdcall* LPFNOPENI2)  (char *port,DWORD rate);

//===================================================================
//  Function : DLL_I2_CloseComport		
//            close the Communication
//  Parameters   : None
//					    	
//  Return value :  0  Success
//				   -2  Port Com is not open
//                 -3  System Error
//=================================================================== 

typedef int (__stdcall* LPFNCLOSEI2) ();

//===================================================================
//  Function :		DLL_I2_WriteReadData
//					Layer 2 Frame Emission and Reception
//
//  Parameters   :                  
//	 (Unsigned char) *datatowrite      : Data to write
//  (Integer)       tailledatatowrite : Size in octet of data to write
//  (Unsigned char) *dataread         : Read Data pointer
//  (Integer)       *tailledataread   : Read Data pointer size ( in octet ) 
//	 (Char)          last_frame        : End Data Transaction Flag
//					 	
//										
//  Return value : 
//					   0 Ok  Transmission du bloc I garantie.
//					   1 to 3 bit values are as follow:
//
//                   b0 0 : response received
//                      1 : response not received
//                   
//                   b1 0: L7 command was send
//                      1: L7 command was not send
//
//                   b2 0: double L7 command run is not enable
//                      1: double L7 command run is enable
//
//                     -2 : Port com is not open
//=========================================================================

typedef int (__stdcall* LPFNWRITEREADI2) (
				  unsigned char *datatowrite,
				  int tailledatatowrite,
				  unsigned char *dataread,
				  int *tailledataread,
				  char last_frame);
