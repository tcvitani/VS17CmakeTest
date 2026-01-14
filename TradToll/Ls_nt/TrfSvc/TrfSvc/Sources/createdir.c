#include <windows.h>
#include <stdio.h>

#define LOC_DEF
#include <createdir.h>
#undef LOC_DEF

#include <memclass.h>


PUBLIC void CreateFileDirectories( char * pcFilePath )
{
    DWORD dwErr;
    char * pcCur;
    char szPath[MAX_PATH];

    __try
    {
        // Recopier le chemin du fichier dans un buffer de travail
        strncpy_s( szPath, MAX_PATH, pcFilePath, sizeof( szPath ) );
        szPath[sizeof(szPath)-1] = '\0';
        
        // Rechercher le dernier backslash pour enlever du chemin
        // le nom du ficher.
        if ( ( pcCur = strrchr( szPath, '\\' ) ) == NULL )
            __leave;
        (*pcCur) = '\0';
 

        // recherche du premier '\' normalement juste apres le ':' avec un path absolu
        if ( ( pcCur = strchr(szPath, '\\') ) == NULL )
            __leave;
        pcCur++;
 
        // création des sous-répertoires si inexistant
        while ( ( pcCur = strchr( pcCur, '\\' ) ) != NULL )
        {
            (*pcCur) = '\0';
 
            if ( ! CreateDirectory( szPath, NULL) )
            {
                dwErr = GetLastError();
                if ( ( dwErr != ERROR_ALREADY_EXISTS ) &&
                     ( dwErr != ERROR_FILE_EXISTS )
                   )
                    __leave;
            }
     
            (*(pcCur++)) = '\\';
        }
 
        // création du répertoire final si inexistant
        CreateDirectory( szPath, NULL );
    }
    __finally
    {
    }
}