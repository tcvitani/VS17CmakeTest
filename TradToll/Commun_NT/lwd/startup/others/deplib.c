/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : lwd
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

// Pour chaque librairie :
// #pragma comment( lib, "DISQUE:\\CHEMIN\\FICHIER.LIB" )

#ifdef _DEBUG
	#pragma comment( lib, "L:\\Commun_NT\\reg\\10.0.2\\csr_reg10_VS12_x64.lib" )
	#pragma comment( lib, "l:\\commun_nt\\trc\\10.0.3\\CSR_trc10_VS12_x64.LIB" )
#else
	#pragma comment( lib, "L:\\Commun_NT\\reg\\10.0.2\\csr_reg10_VS12_x64_r.lib" )
	#pragma comment( lib, "l:\\commun_nt\\trc\\10.0.3\\CSR_trc10_VS12_x64_r.LIB" )
#endif
