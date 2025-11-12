/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Liste générique
* FICHIER: Liste.C
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <csr_srv.h>
#include <csr_list.h>

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- DEFINES: ---------------*/

#define SRV_GET_TOKEN(i)  ((struct_jeton *) (((BYTE *) i) - sizeof(struct_jeton)))
#define SRV_GET_DATA(i)    ((struct_jeton *) (((BYTE *) i) + sizeof(struct_jeton)))

/*--------------- TYPEDEFS: ---------------*/

typedef struct
{
    srv_id_demandeur IdDemandeur;
}
struct_jeton;

typedef struct
{
    HLIST hList;
    DWORD NbJetonsMax;
    list_struct_find find;
}
struct_service;

/*--------------- FUNCTIONS: ---------------*/

/*--------------- CODE: ---------------*/

/*--------------- RESERVED: ---------------*/

PRIVATE BOOL WINAPI ChercheJeton(struct_jeton *pJeton1, struct_jeton *pJeton2)
{
    return pJeton1->IdDemandeur == pJeton2->IdDemandeur;
}

PUBLIC HANDLE WINAPI SrvLance (DWORD NbJetonsMax)
{
    struct_service *pSrv;

    // allocation du service
    if ((pSrv = malloc(sizeof(struct_service))) == NULL)
        return NULL;

    // allocation de la liste des demandeurs
    if ((pSrv->hList = List_New()) == NULL)
    {
        free (pSrv);
        return NULL;
    }

    // préparation de la struct de recherche
    pSrv->find.hList = pSrv->hList;

    // allocation de l'item de recherche
    if ((pSrv->find.ItemToFind = List_ItemNew(pSrv->hList, sizeof(struct_jeton))) == NULL)
    {
        List_Delete (&pSrv->hList);
        free(pSrv);
        return FALSE;
    }

    // fonction de recherche d'un jeton
    pSrv->find.Equal = ChercheJeton;

    // le reste des infos
    pSrv->NbJetonsMax = NbJetonsMax;

    return pSrv;
}

PUBLIC BOOL WINAPI SrvArret (HANDLE *phSrv)
{
    struct_service *pSrv = *phSrv;

    // suppression de la liste et de ses jetons
    List_DeleteAll (&pSrv->hList, NULL, NULL);

    // suppression du jeton de recherche
    List_ItemDelete (pSrv->hList, &pSrv->find.ItemToFind);
    
    // supression du service
    free(pSrv);

    return TRUE;
}

PUBLIC BOOL WINAPI SrvEstVide(HANDLE hSrv)
{
    struct_service *pSrv = hSrv;

    return List_GetCount(pSrv->hList) == 0;
}

PUBLIC DWORD WINAPI SrvCompteDemandeurs(HANDLE hSrv)
{
    struct_service *pSrv = hSrv;

    return List_GetCount(pSrv->hList);
}

PUBLIC BOOL WINAPI SrvEstDisponible(HANDLE hSrv)
{
    struct_service *pSrv = hSrv;

    // si le nombre de jetons max est atteind
    // (pour infini, max = -1 non signé)
    if (List_GetCount(pSrv->hList) >= pSrv->NbJetonsMax)
        return FALSE;

    return TRUE;
}

PUBLIC PVOID WINAPI SrvAjouteDemandeur (HANDLE hSrv, srv_id_demandeur IdDemandeur, DWORD SizeOfData)
{
    BOOL bRet;
    struct_service *pSrv = hSrv;
    struct_jeton *pJeton;

    List_EnterCriticalSection (pSrv->hList);

    // si le nombre de jetons max est atteind, stop
    // (pour infini, max = -1 non signé)
    if (List_GetCount(pSrv->hList) >= pSrv->NbJetonsMax)
    {
        List_LeaveCriticalSection (pSrv->hList);
        return NULL;
    }

    // création d'un nouveau jeton de service
    if ((pJeton = List_ItemNew (pSrv->hList, sizeof(struct_jeton) + SizeOfData)) == NULL)
    {
        List_LeaveCriticalSection (pSrv->hList);
        return NULL;
    }

    // préparation du jeton
    pJeton->IdDemandeur = IdDemandeur;

    // insertion en tete
    bRet = List_AddHead (pSrv->hList, pJeton);

    // si l'insertion a echouée, faisait pas les cons liberer le jeton !
    if (bRet == FALSE)
    {
        List_ItemDelete (pSrv->hList, &pJeton);

        List_LeaveCriticalSection (pSrv->hList);
        return NULL;
    }

    List_LeaveCriticalSection (pSrv->hList);
 
    return SRV_GET_DATA(pJeton);
}

PUBLIC BOOL WINAPI SrvEnleveDemandeur (HANDLE hSrv, PVOID *Jeton)
{
    struct_service *pSrv = hSrv;
    struct_jeton *pJeton = SRV_GET_TOKEN(*Jeton);

    List_EnterCriticalSection (pSrv->hList);

    // Extrait le jeton de la liste
    List_Remove (pSrv->hList, pJeton);

    // libérer le jeton (ou on casse la zonzon ! - slogan -:)
    List_ItemDelete (pSrv->hList, &pJeton);

    // RAZ du jeton
    *Jeton = NULL;

    List_LeaveCriticalSection (pSrv->hList);

    return TRUE;
}

PUBLIC BOOL WINAPI SrvEstDemandeur (HANDLE hSrv, srv_id_demandeur IdDemandeur)
{
    struct_service *pSrv = hSrv;
    struct_jeton *pJeton = pSrv->find.ItemToFind;

    // on remplit le jeton de recherche préalloué
    pJeton->IdDemandeur =  IdDemandeur;

    // recherche du 1er
    pJeton = List_FindHeadFirst(&pSrv->find);
    
    // si jeton n'existe pas dans la liste, 
    // la BAL n'est pas demandeur du service
    if (pJeton == NULL)
        return FALSE;

    // brrr ! j'ai les jetons
    return TRUE;
}

PUBLIC PVOID WINAPI SrvRechercheDemandeur (HANDLE hSrv, srv_id_demandeur IdDemandeur, srv_func_recherche Recherche, PVOID Param)
{
    struct_service *pSrv = hSrv;
    struct_jeton *pJeton = pSrv->find.ItemToFind;

    List_EnterCriticalSection (pSrv->hList);

    // on remplit le jeton de recherche préalloué
    pJeton->IdDemandeur =  IdDemandeur;

    // recherche du 1er
    pJeton = List_FindHeadFirst(&pSrv->find);

    // si Recherche est NULL, on renvoie le 1er
    if (Recherche == NULL && pJeton != NULL)
    {
        List_LeaveCriticalSection (pSrv->hList);
        return SRV_GET_DATA(pJeton);
    }

    // sinon on cherche celui qui correspond
    while (pJeton != NULL)
    {
        if (Recherche (Param, SRV_GET_DATA(pJeton)) == TRUE)
        {
            List_LeaveCriticalSection (pSrv->hList);
            return SRV_GET_DATA(pJeton);
        }

        pJeton = List_FindNext(&pSrv->find);
    }

    List_LeaveCriticalSection (pSrv->hList);

    return NULL;
}

PUBLIC BOOL WINAPI SrvPourChaqueDemandeur (HANDLE hSrv, srv_func_action fAction, PVOID Param)
{
    struct_service *pSrv = hSrv;
    struct_jeton *pJeton, *pTemp;

    List_EnterCriticalSection (pSrv->hList);
    
    pJeton = List_GetHead(pSrv->hList);
    while (pJeton != NULL)
    {
        // on utilise une variable intermédiaire pour pouvoir
        // passer au jeton suivant AVANT d'appeler la fonction de l'utilisateur
        // Cette derniere pourrait tres bien supprimer l'element courant
        // d'ou probleme pour en chercher le suivant en retour
        pTemp = pJeton;
        pJeton = List_GetNext(pSrv->hList, pJeton);

        if (fAction(Param, pTemp->IdDemandeur, SRV_GET_DATA(pTemp)) == FALSE)
        {
            List_LeaveCriticalSection (pSrv->hList);
            return FALSE;
        }
    }

    List_LeaveCriticalSection (pSrv->hList);

    return TRUE;
}
