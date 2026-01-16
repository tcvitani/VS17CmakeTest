/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_dirs.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des répertoires par l'intermédiaire des librairie
 *              d'extension.
 *              Le but de cette librairie est de minimiser les I/O pour
 *              le suivi de l'état des répertoires. En particulier pour
 *              éviter un trop grand nombre d'énumérations supplémentaire,
 *              on maintient pour chaque répertoire une image de son
 *              contenu qui est mise à jour à chauque opération de copie,
 *              déplacement ou effacement.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CM_DIRS_H
#define CM_DIRS_H

#include <protect.h>



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMDirInit( CM_DIR * psDir, CM_HOST * psHost, char * szPath, char * szNameMask )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire non initialisée
 *             psHost : Pointeur sur une struture descriptive d'un host initialisé
 *             szPath : Chemin d'accés au répertoire (dans la syntaxe du host).
 *             szNameMask : Si NULL ou chaine vide, tous les fichiers sont traités.
 *                          Sinon, masque de sélection des fichiers en utilisant
 *                          les métacaractère '*' et '?'
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Initialise une structure descriptive d'un répertoire
 * --------------------------------------------------------------------
 */
PROTECTED void CMDirInit( CM_DIR * psDir, CM_HOST * psHost, char * szPath, char * szNameMask );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMDirTerminate( CM_DIR * psDir )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire initialisée
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libère les ressources allouées pour une structure descriptive d'un répertoire
 * --------------------------------------------------------------------
 */
PROTECTED void CMDirTerminate( CM_DIR * psDir );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMDirLock( CM_DIR * psDir )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire initialisée
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Vérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMDirLock( CM_DIR * psDir );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMDirUnlock( CM_DIR * psDir )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire initialisée
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Dévérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMDirUnlock( CM_DIR * psDir );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD CMDirExist( CM_DIR * psDir, char * szFileName  )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir      : Pointeur sur une struture descriptive d'un répertoire initialisée
 *             szFileName : Nom du fichier (sans le chemin)
 * --------------------------------------------------------------------
 * RETURN    : CM_EXISTS si le fichier existe, CM_NOT_FOUND si le répertoire est
 *             online, mais que le fichier n'existe pas et CM_INVALID si une
 *             erreur fait qu'il est impossible de savoir si le fichier
 *             existe ou nom (par exemple, le répertoire est offline).
 * --------------------------------------------------------------------
 * ROLE      : Détermine si un fichier se trouve dans un répertoire.
 *             Attention, le répertoire doit être raffraichi et online.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD CMDirExist( CM_DIR * psDir, char * szFileName  );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED CM_FILE * CMDirFind( CM_DIR * psDir, char * szFileName, BOOL fLock )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir      : Pointeur sur une struture descriptive d'un répertoire initialisée
 *             szFileName : Nom du fichier (sans le chemin)
 *             fLock      : Si TRUE, lorsque le fichier est trouvé, le vérou sur le
 *                          répertoire est conservé.
 * --------------------------------------------------------------------
 * RETURN    : NULL si erreur, répertoire offline ou fichier non trouvé.
 *             Un pointeur sur une structure fichier si le fichier a été trouvé.
 * --------------------------------------------------------------------
 * ROLE      : Recherche un fichier dans un répertoire.
 * --------------------------------------------------------------------
 */
PROTECTED CM_FILE * CMDirFind( CM_DIR * psDir, char * szFileName, BOOL fLock );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMDirRefresh( CM_DIR * psDir )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir      : Pointeur sur une struture descriptive d'un répertoire initialisée
 * --------------------------------------------------------------------
 * RETURN    : TRUE si le répertoire est rafraichi et online
 * --------------------------------------------------------------------
 * ROLE      : Rafraichie l'image local d'un répertoire.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMDirRefresh( CM_DIR * psDir );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMDirTransmit( CM_DIR * psDirSrc, CM_DIR * psDirDst, char * szName )
 * --------------------------------------------------------------------
 * PARAMETERS: psDirSrc : Pointeur sur une struture descriptive d'un répertoire initialisée (source)
 *             psDirDst : Pointeur sur une struture descriptive d'un répertoire initialisée (destination)
 *             szName   : Nom du fichier.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si la transmission a eu lieu sans erreur, FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Effectue une transmission de fichier d'un répertoire vers un autre.
 *             Si le fichier destination existe déjà, la transmission échoue.
 *             Pour réussir, les répertoires sources et destination doivent vérifier
 *             une des conditions suivantes :
 *                 CONDITION 1 :     Source locale    et    Destination locale
 *                 CONDITION 2 :     Source distante  et    Destination locale
 *                 CONDITION 3 ;     Source locale    et    Destination distante
 *             La combinaison source distante et destination distante est donc interdite.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMDirTransmit( CM_DIR * psDirSrc, CM_DIR * psDirDst, char * szName );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMDirMove( CM_DIR * psDirSrc, CM_DIR * psDirDst, char * szName, BOOL bCopy )
 * --------------------------------------------------------------------
 * PARAMETERS: psDirSrc : Pointeur sur une struture descriptive d'un répertoire initialisée (source)
 *             psDirDst : Pointeur sur une struture descriptive d'un répertoire initialisée (destination)
 *                        ou NULL pour un effacement.
 *             szName   : Nom du fichier.
 *             bCopy    : TRUE pour effectuer une copie, FALSE pour un déplacement.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si le deplacement/copie/effacement a eu lieu sans erreur, FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Effectue une copie, un déplacement ou un effacement de fichier.
 *             En cas de copie ou de déplacement, si le fichier destination existe déjà,
 *             la fonction échoue.
 *             En cas de copie ou de déplacement, l'host de la source doit être le même
 *             que celui de la destination.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMDirMove( CM_DIR * psDirSrc, CM_DIR * psDirDst, char * szName, BOOL bCopy );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMAnalyseSortParams( char * szParams, CM_SORT * psSort )
 * --------------------------------------------------------------------
 * PARAMETERS: szParam : Paramètre de tri
 *             psSort  : structure de contexte de tri
 * --------------------------------------------------------------------
 * RETURN    : TRUE si OK, FALSE si erreur
 * --------------------------------------------------------------------
 * ROLE      : Préparation d'un contexte de tri en fonction d'une chaine
 *             de paramètres.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMAnalyseSortParams( char * szParams, CM_SORT * psSort );



#endif