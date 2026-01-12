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

#ifdef _DEBUG
	#pragma comment( lib, "L:\\COMMUN_NT\\CMDL\\10.0.0\\CSR_CMDL10_VS12_x64.LIB" )
	#pragma comment( lib, "..\\..\\Debug_12_x64\\CSR_DBIFORA_MAP\\CSR_DBIFORA10_VS12_x64.LIB" )
#else
	#pragma comment( lib, "L:\\COMMUN_NT\\CMDL\\10.0.0\\CSR_CMDL10_VS12_x64_r.LIB" )
	#pragma comment( lib, "..\\..\\Release_12_x64\\CSR_DBIFORA_MAP\\CSR_DBIFORA10_VS12_x64_r.LIB" )
#endif
	