// inclure ce fichier avant et apres la declaration en PROTECTED de vos
// variables apartager entre plusieurs instance d'une DLL
// ATTENTION : 
//    - ces variables partagees doivent etre initialisees (meme les structures) 
//      grace a la macro 'INIT(valeur)' (cf protect.h)
//    - ces variables ne peuvent -tre des pointeurs (pas meme des pointeurs vers
//      d'autres variables dans cette meme zone partagee car l'adresse de base
//      de la zone peut varier d'une instance a l'autre -> 'relocation' du systeme)


#ifdef LOC_DEF
	#pragma message ("GLOBAL.H INCLUDED WITH LOC_DEF!")
	
   #ifndef SECTION_GLOBALE

      // debut de declaration de la section
      #pragma data_seg("MySharedData")    
         
      // bascule dans le 'else' a la prochaine inclusion de ce fichier
      #define SECTION_GLOBALE 

      #pragma message ("Declaration of data segment MySharedData START")

   #else
   
      // fin de declaration de la section
      #pragma data_seg()       

      // attribution des acces en lecture/ecriture et partage
      #pragma comment(linker, "/SECTION:MySharedData,RWS")
	  #pragma message ("Declaration of data segment MySharedData END")

	  #pragma message ("WARRNING!!! make sure that you added ""/SECTION:MySharedData,RWS"" to project Properties>Linker>Command Line>Additional Options")	  

      #undef SECTION_GLOBALE

   #endif

#endif