/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : TAG
 * FILE       : TAG_DEP.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Library dependency file
 * --------------------------------------------------------------------
 * DESCRIPTION: This file is used to generate an object files that
 *              contains linker options, especially library
 *              references.
 * --------------------------------------------------------------------
 * HISTORY    :
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

//
// For each library that must be linked to the project :
//
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "Mswsock.lib" )
#pragma comment(lib, "Ws2_32.lib")

#pragma message("Visual Studio 2013 - 64bit")

#ifdef _DEBUG
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.0\\CSR_REG10_VS12_x64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\EXCPT\\10.0.1\\CSR_EXCPT10_VS12_x64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\QUEUE\\10.0.0\\CSR_QUEUEMT10_VS12_x64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TIMER\\10.0.0\\CSR_TIMERMT10_VS12_x64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\BITBL\\10.0.1\\CSR_BITBL10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\DES\\10.0.0\\CSR_DES10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.4\\CSR_TRC10_VS12_x64.lib" )
#else
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.0\\CSR_REG10_VS12_x64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\EXCPT\\10.0.1\\CSR_EXCPT10_VS12_x64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\QUEUE\\10.0.0\\CSR_QUEUEMT10_VS12_x64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TIMER\\10.0.0\\CSR_TIMERMT10_VS12_x64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\BITBL\\10.0.1\\CSR_BITBL10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\DES\\10.0.0\\CSR_DES10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.4\\CSR_TRC10_VS12_x64_r.lib" )
#endif
