// inclure ce fichier avant et apres la déclaration en PROTECTED de vos
// variables à partager entre plusieurs instance d'une DLL
// ATTENTION : 
//    - ces variables partagées doivent etre initialisées (meme les structures) 
//      grace à la macro 'INIT(valeur)' (cf protect.h)
//    - ces variables ne peuvent être des pointeurs (pas meme des pointeurs vers
//      d'autres variables dans cette meme zone partagée car l'adresse de base
//      de la zone peut varier d'une instance à l'autre -> 'relocation' du systeme)


#ifdef LOC_DEF
	#pragma message ("GLOBAL.H INCLUDED WITH LOC_DEF!")
	
   #ifndef SECTION_GLOBALE

      // début de déclaration de la section
      #pragma data_seg("MySharedData")    
         
      // bascule dans le 'else' à la prochaine inclusion de ce fichier
      #define SECTION_GLOBALE 

      #pragma message ("Declaration of data segment MySharedData START")

   #else
   
      // fin de déclaration de la section
      #pragma data_seg()       

      // attribution des acces en lecture/ecriture et partage
      #pragma comment(linker, "/SECTION:MySharedData,RWS")
	  #pragma message ("Declaration of data segment MySharedData END")

	  #pragma message ("WARRNING!!! make sure that you added ""/SECTION:MySharedData,RWS"" to project Properties>Linker>Command Line>Additional Options")	  

      #undef SECTION_GLOBALE

   #endif

#endif