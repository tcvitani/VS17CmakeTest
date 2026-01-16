/******************* (v) 2002 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKORA                                                       */
/* FILE:     cmworkora.h                                                     */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             The extension module of the ComMOCSvc service. This module    */
/*             checks if the file is completely exported (the file has the   */
/*             #ERF terminator).                                             */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMWORKORA_H
#define CMWORKORA_H
/*-------------------------------- INCLUDES:  -------------------------------*/
/*-------------------------------- RESERVED:  -------------------------------*/
#ifdef CMWORKCHKEXP_EXPORTS
#    include <public.h>
#else
#    include <export.h>
#endif
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/

/*****************************************************************************/
/*SYNTAX: EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig )  */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service at service  */
/*            startup. The function initializes the module. It reads the     */
/*            module parameters from the registry and initializes the trace  */
/*            file.                                                          */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) PVOID param  - Pointer to the instance ID.                     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  HANDLE                If the initialization is successful the function   */
/*                        returns the module handle (pointer to module       */
/*                        parameters).                                       */
/*  NULL                  The function returns NULL if the initialization is */
/*                        not successful.                                    */
/*****************************************************************************/
EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig );

/*****************************************************************************/
/*SYNTAX: EXPORT void WINAPI WorkerClose( HANDLE hWrk )                      */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service at the      */
/*            service exit.                                                  */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HANDLE hWrk  - Pointer to the module handle.                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
EXPORT void WINAPI WorkerClose( HANDLE hWrk );

/*****************************************************************************/
/*SYNTAX: EXPORT BOOL WINAPI WorkerProcessFile( HANDLE hWrk,                 */
/*                                              char * szFilePath )          */
/*===========================================================================*/
/*TYPE:   Exported function.                                                 */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function is called from the ComMOCSvc service when it     */
/*            detects the file in the appropriate directory.                 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      (IN ) HANDLE hWrk       - Worker handle (pointer to module           */
/*                                parameters).                               */
/*      (IN ) char * szFilePath - File name (full path)                      */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  The file is successfully converted.                */
/*  FALSE                 Error converting the file.                         */
/*****************************************************************************/
EXPORT BOOL WINAPI WorkerProcessFile( HANDLE hWrk, char * szFilePath );

/*-------------------------------- VARIABLES: -------------------------------*/
#endif
/*-------------------------------- END OF FILE ------------------------------*/