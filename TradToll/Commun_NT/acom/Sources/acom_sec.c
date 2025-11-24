/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_sec.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion de la sécurité sur les IPC
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <acom_dmem.h>

#include <acom.h>
#include <acom_priv.h>
#include <acom_dbg.h>
#define LOC_DEF
#include <acom_sec.h>
#undef LOC_DEF

#include <memclass.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED SECURITY_ATTRIBUTES * AComSecOpen (  )
 * PARAMETERS: Aucun
 * RETURN    : Un pointeur sur une structure d'attributs de sécurité nulle.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Créer une structure d'attributs de sécurité nulle prète
 *             à être utilisée par un pipe ou un mailslot.
 * --------------------------------------------------------------------
 */
PROTECTED SECURITY_ATTRIBUTES * AComSecOpen()
{
    SECURITY_ATTRIBUTES * psSA = NULL;
    SECURITY_DESCRIPTOR * psSD = NULL;
    BOOL bErr = FALSE;
    DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComSecOpen()" );

    // Allouer la mémoire pour le security descriptor.
    if ( DMEM_ZALLOC_BUFFER( psSD, SECURITY_DESCRIPTOR_MIN_LENGTH ) != NULL )
    {
        if ( DMEM_ZALLOC( psSA ) != NULL )
        {
            // Initialiser le security descriptor.
            if ( InitializeSecurityDescriptor( psSD, SECURITY_DESCRIPTOR_REVISION ) )
            {
                // Ajouter une access control list (ACL) avec descripteur NULL au security descriptor.
                if ( SetSecurityDescriptorDacl( psSD, TRUE, (PACL) NULL, FALSE ) )
                {
                    psSA->nLength = sizeof(*psSA);
                    psSA->lpSecurityDescriptor = psSD;
                    psSA->bInheritHandle = TRUE;
                }
                else
                {
                    bErr = TRUE;
                    dwErr = GetLastError();
                    AComDbgError( dwErr, __FILE__, __LINE__, "SetSecurityDescriptorDacl" );
                }
            }
            else
            {
                bErr = TRUE;
                dwErr = GetLastError();
                AComDbgError( dwErr, __FILE__, __LINE__, "InitializeSecurityDescriptor" );
            }
        }
        else
        {
            bErr = TRUE;
            AComDbgError( 0, __FILE__, __LINE__, "DMEM_ZALLOC" );
        }
    }
    else
    {
        bErr = TRUE;
        AComDbgError( 0, __FILE__, __LINE__, "DMEM_ZALLOC_BUFFER" );
    }

    if ( bErr )
    {
        if ( psSA != NULL ) DMEM_FREE(psSA);
        if ( psSD != NULL ) DMEM_FREE(psSD);
    }

    AComDbgInfo( __FILE__, __LINE__, "AComSecOpen return 0x%016X", psSA );
    return psSA;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComSecClose ( SECURITY_ATTRIBUTES * psSA )
 * PARAMETERS: SECURITY_ATTRIBUTES * psSA : Pointe sur une structure d'attributs de sécurité
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Désalloue une structure d'attributs de sécurités ouverte avec AComSecOpen().
 * --------------------------------------------------------------------
 */
PROTECTED void AComSecClose( SECURITY_ATTRIBUTES * psSA )
{
    AComDbgInfo( __FILE__, __LINE__, "AComSecClose()" );
    DMEM_FREE( psSA->lpSecurityDescriptor );
    DMEM_FREE( psSA );
}


/* -------------  FIN DU FICHIER : acom_sec.c ------------- */ 
