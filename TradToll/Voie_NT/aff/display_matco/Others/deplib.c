/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : DEPLIB.C
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

#ifdef _DEBUG
	#pragma comment( lib, "L:\\Commun_NT\\LIST\\10.0.0\\CSR_LIST10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Commun_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Commun_NT\\SRV\\10.0.0\\CSR_SRV10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Commun_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64.lib" )
	
	#pragma comment( lib, "l:\\Voie_nt\\fic_gere\\10.1.2\\CSR_FIC_GERE10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\fonct_c\\11.0.1\\CSR_FONCT_C11_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\hrd\\10.1.1\\CSR_HRD10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\noy\\10.0.4\\CSR_NOY10_VS12_x64.lib" )
	#pragma comment( lib, "l:\\Voie_nt\\msg_fic\\10.1.1\\CSR_MSG_FIC10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Voie_nt\\ser\\10.0.2\\CSR_SER10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\Voie_nt\\aff\\_aff_util\\11.0.0\\CSR_AFF_UTIL11_VS12_x64.lib" )
#else
	#pragma comment( lib, "L:\\Commun_NT\\LIST\\10.0.0\\CSR_LIST10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Commun_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Commun_NT\\SRV\\10.0.0\\CSR_SRV10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Commun_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64_r.lib" )

	#pragma comment( lib, "l:\\Voie_nt\\fic_gere\\10.1.2\\CSR_FIC_GERE10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\fonct_c\\11.0.1\\CSR_FONCT_C11_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\hrd\\10.1.1\\CSR_HRD10_VS12_x64_r.lib" )
	#pragma comment( lib, "l:\\Voie_nt\\msg_fic\\10.1.1\\CSR_MSG_FIC10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Voie_NT\\noy\\10.0.4\\CSR_NOY10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Voie_nt\\ser\\10.0.2\\CSR_SER10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\Voie_nt\\aff\\_aff_util\\11.0.0\\CSR_AFF_UTIL11_VS12_x64_r.lib" )
#endif
