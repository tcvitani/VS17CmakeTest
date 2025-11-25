/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
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

#ifdef _DEBUG
	#pragma comment( lib, "l:\\commun_nt\\msg\\10.0.2\\csr_msg10_VS12_x64.lib" )
	#pragma comment( lib, "..\\Debug_12_x64\\CSR_MSGLC11_VS12_x64.lib" )
#else
	#pragma comment( lib, "l:\\commun_nt\\msg\\10.0.2\\csr_msg10_VS12_x64_r.lib" )
	#pragma comment( lib, "..\\Release_12_x64\\CSR_MSGLC11_VS12_x64_r.lib" )
#endif




