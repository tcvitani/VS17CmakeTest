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
	#pragma comment( lib, "L:\\VOIE_NT\\NOY\\10.0.4\\CSR_NOY10_VS12_x64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\FONCT_C\\11.0.1\\CSR_FONCT_C11_VS12_x64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\FILES_TOOLS\\10.2.0\\CSR_FILES_TOOLS10_VS12_x64.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\RFR\\REFERENC\\11.0.0\\CSR_REFERENC11_VS12_x64.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64.LIB" )
	#pragma comment( lib, "l:\\commun_nt\\list\\10.0.0\\csr_list10_VS12_x64.lib" )
#else
	#pragma comment( lib, "L:\\VOIE_NT\\NOY\\10.0.4\\CSR_NOY10_VS12_x64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64_r.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\FONCT_C\\11.0.1\\CSR_FONCT_C11_VS12_x64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\FILES_TOOLS\\10.2.0\\CSR_FILES_TOOLS10_VS12_x64_r.LIB" )
	#pragma comment( lib, "L:\\VOIE_NT\\RFR\\REFERENC\\11.0.0\\CSR_REFERENC11_VS12_x64_r.LIB" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64_r.LIB" )
	#pragma comment( lib, "l:\\commun_nt\\list\\10.0.0\\csr_list10_VS12_x64_r.LIB")
#endif
