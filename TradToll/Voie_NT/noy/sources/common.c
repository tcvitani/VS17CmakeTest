//#include <string.h>

#include "noyau.h"
//#include "tcp_ip.h"
#include "noy_dbg.h"
 
#define LOC_DEF 
   #include "noy_cro.h"
#undef LOC_DEF
  
#define LOC_DEF
   #include "noy_loc.h"
#undef LOC_DEF

#include <memclass.h>


PROTECTED noyau_enum_booleen NoyauEstInitialise (void)
{
   return NOYAU_NB_INSTANCES > 0;
}

PROTECTED void EnterRegion (void)
{
   // pas de test car TIME_OUT infini et WAIT_ABANDONNED est OK
   WaitForSingleObject (_noyau_.hRegionNoyau, INFINITE);
}

PROTECTED void LeaveRegion (void)
{
   if (!ReleaseMutex (_noyau_.hRegionNoyau)) 
      ExitBad();
}

EXPORT noyau_objet_retour AttendObjets (noyau_enum_booleen attendre_tous, noyau_nb_objets nb_objets, noyau_objet objet_0, ...)
{
    noyau_objet_retour retour;

    retour = NOYAU_OBJET_ERREUR;
    if (nb_objets == 1)
        retour = WaitForSingleObject (objet_0, INFINITE);
    else
        retour = WaitForMultipleObjects (nb_objets, &objet_0, attendre_tous, INFINITE);

    return retour;
}

EXPORT noyau_objet_retour AttendObjetsTantQue (noyau_delai ticks, noyau_enum_booleen attendre_tous, noyau_nb_objets nb_objets, noyau_objet objet_0, ...)
{
    noyau_objet_retour retour;
   
    RESIZE_TIMEOUT(ticks);
    
    /*** Attente d'un evenement ***/
    retour = NOYAU_OBJET_ERREUR;
    if (nb_objets == 1)
        retour = WaitForSingleObject (objet_0, ticks);
    else
        retour = WaitForMultipleObjects (nb_objets, &objet_0, attendre_tous, ticks);

    return retour;
}
