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

#ifdef _DEBUG
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\EXCPT\\10.0.3\\CSR_EXCPT10_VS12_x64.lib" )
#else
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\EXCPT\\10.0.3\\CSR_EXCPT10_VS12_x64_r.lib" )
#endif


