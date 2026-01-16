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

//#pragma comment( lib, "imagehlp.lib" )

#ifdef _DEBUG
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\EXCPT\\10.0.3\\CSR_EXCPT10_VS12_x64.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\XML_TOOLS\\10.0.0\\csr_xml_tools10_vs12_x64.lib" )
	#pragma comment( lib, "L:\\_Third Party Libraries\\Xerces\\3.1.2\\x64\\debug\\xerces-c_3D.lib")
#else
	#pragma comment( lib, "L:\\COMMUN_NT\\REG\\10.0.2\\CSR_REG10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\TRC\\10.0.3\\CSR_TRC10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\EXCPT\\10.0.3\\CSR_EXCPT10_VS12_x64_r.lib" )
	#pragma comment( lib, "L:\\COMMUN_NT\\XML_TOOLS\\10.0.0\\csr_xml_tools10_vs12_x64_r.lib" )
	#pragma comment( lib, "L:\\_Third Party Libraries\\Xerces\\3.1.2\\x64\\release\\xerces-c_3.lib")
#endif

