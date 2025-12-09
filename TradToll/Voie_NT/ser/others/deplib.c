/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     :   
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

#ifdef _DEBUG
	#pragma comment( lib, "L:\\Commun_NT\\REG\\10.0.2\\csr_reg10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\NOY\\10.0.4\\csr_Noy10_VS12_x64.lib" )
#else
	#pragma comment( lib, "L:\\Commun_NT\\REG\\10.0.2\\csr_reg10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\NOY\\10.0.4\\csr_Noy10_VS12_x64_r.lib" )
#endif
