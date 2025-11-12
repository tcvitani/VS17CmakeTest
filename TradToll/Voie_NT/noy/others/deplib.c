/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Noyau   
 * FILE       : deplib.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Options de linkage - librairies
 * --------------------------------------------------------------------
 * DESCRIPTION: Définition centralisée des librairies nécessaires pour le
 *              linkage et suceptibles d'évoluer.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#pragma comment( lib, "WININET.LIB" )
#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
	#pragma comment( lib, "l:\\commun_nt\\reg\\10.0.2\\CSR_reg10_VS12_x64.LIB" )
	#pragma comment( lib, "l:\\commun_nt\\trc\\10.0.3\\CSR_trc10_VS12_x64.LIB" )
	#pragma comment( lib, "l:\\commun_nt\\excpt\\10.0.3\\CSR_excpt10_VS12_x64.LIB" )
#else
	#pragma comment( lib, "l:\\commun_nt\\reg\\10.0.2\\CSR_reg10_VS12_x64_R.LIB" )
	#pragma comment( lib, "l:\\commun_nt\\trc\\10.0.3\\CSR_trc10_VS12_x64_R.LIB" )
	#pragma comment( lib, "l:\\commun_nt\\excpt\\10.0.3\\CSR_excpt10_VS12_x64_R.LIB" )
#endif


