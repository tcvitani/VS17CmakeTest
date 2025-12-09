/***************** (v) 2016 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     Deplib.c												         */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:     Creation of file for project								 */
/*****************************************************************************/

#ifdef _DEBUG
	#pragma comment( lib, "L:\\Commun_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\noy\\10.0.4\\CSR_NOY10_VS12_x64.lib" )	
#else
	#pragma comment( lib, "L:\\Commun_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\noy\\10.0.4\\CSR_NOY10_VS12_x64_r.lib" )
#endif
