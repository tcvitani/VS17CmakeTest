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
	#pragma comment( lib, "l:\\voie_nt\\fic_gere\\10.1.1\\csr_Fic_gere10_VS12_x64.lib" )
	#pragma comment( lib, "l:\\voie_nt\\fonct_c\\11.0.1\\csr_Fonct_c11_VS12_x64.lib" )
#else
	#pragma comment( lib, "l:\\voie_nt\\fic_gere\\10.1.1\\csr_Fic_gere10_VS12_x64_r.lib" )
	#pragma comment( lib, "l:\\voie_nt\\fonct_c\\11.0.1\\csr_Fonct_c11_VS12_x64_r.lib" )
#endif
