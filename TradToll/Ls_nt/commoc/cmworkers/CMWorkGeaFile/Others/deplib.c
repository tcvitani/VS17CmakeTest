/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     deplib.c                                                        */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#pragma comment( lib, "imagehlp.lib" )
#pragma comment( lib, "shlwapi.lib" )

#ifdef _DEBUG
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\EXCPT\\10.0.3\\CSR_EXCPT10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\MSG\\10.0.2\\CSR_MSG10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\MSG_LC\\11.0.0\\CSR_MSGLC11_VS12_x64.lib" )
	#pragma comment( lib, "N:\\COMMUN_NT\\Europe\\Hr\\Hac\\Modules\\Msg_Gea_Lc\\10.0.1\\CSR_MSGLC10_GEA_VS12_x64.lib" )
	
	#ifdef CMWORK_DB_CONNECTION
		#pragma comment( lib, "L:\\LS_NT\\DBIF\\10.0.1\\CSR_DBIFORA10_OCI11_VS12_x64.lib" )
	#endif
#else
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\EXCPT\\10.0.3\\CSR_EXCPT10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\MSG\\10.0.2\\CSR_MSG10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\MSG_LC\\11.0.0\\CSR_MSGLC11_VS12_x64_r.lib" )
	#pragma comment( lib, "N:\\COMMUN_NT\\Europe\\Hr\\Hac\\Modules\\Msg_Gea_Lc\\10.0.1\\CSR_MSGLC10_GEA_VS12_x64_r.lib" )
	
	#ifdef CMWORK_DB_CONNECTION
		#pragma comment( lib, "L:\\LS_NT\\DBIF\\10.0.1\\CSR_DBIFORA10_OCI11_VS12_x64_r.lib" )
	#endif
#endif


