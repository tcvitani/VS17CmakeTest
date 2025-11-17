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
	#pragma comment( lib, "..\\Debug_12_x64\\csr_lwd10_VS12_x64.lib" )
#else
	#pragma comment( lib, "..\\Release_12_x64\\csr_lwd10_VS12_x64_r.lib" )
#endif