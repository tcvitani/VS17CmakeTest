/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     :   
 * FILE       : deplib.c
 * LANGAGE    : C
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Options de linkage - librairies
 * --------------------------------------------------------------------
 * DESCRIPTION: Définition centralisée des librairies nécessaires
 *              pour le linkage et suceptibles d'évoluer.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */


#ifdef _DEBUG
	#pragma comment( lib, "L:\\VOIE_NT\\NOY\\10.0.4\\CSR_NOY10_VS12_X64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_X64.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\HRD\\10.1.1\\CSR_HRD10_VS12_X64.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\FONCT_C\\11.0.1\\CSR_FONCT_C11_VS12_X64.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\FIC_GERE\\10.1.1\\CSR_FIC_GERE10_VS12_X64.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\FIC_CONF\\10.1.1\\CSR_FIC_CONF10_VS12_X64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\FILES_TOOLS\\10.2.0\\CSR_FILES_TOOLS10_VS12_X64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64.LIB" )
#else
	#pragma comment( lib, "L:\\VOIE_NT\\NOY\\10.0.4\\CSR_NOY10_VS12_X64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_X64_r.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\HRD\\10.1.1\\CSR_HRD10_VS12_X64_r.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\FONCT_C\\11.0.1\\CSR_FONCT_C11_VS12_X64_r.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\FIC_GERE\\10.1.1\\CSR_FIC_GERE10_VS12_X64_r.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\FIC_CONF\\10.1.1\\CSR_FIC_CONF10_VS12_X64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\FILES_TOOLS\\10.2.0\\CSR_FILES_TOOLS10_VS12_X64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64_r.LIB" )
#endif
