#include <noyau.h>
#include <conio.h>

noyau_event_id Event1, Event2;


DWORD WINAPI ThreadA (PVOID param)
{
    noyau_objet_retour retour;

    printf ("\nThread : Création des Events");

    AlloueEvent (&Event1, "Evt1");
    AlloueEvent (&Event2, "Evt2");

    printf ("\nThread : En attente...");
/*
    retour = AttendObjets(NOYAU_FAUX, 2, Event1, Event2);

    printf ("\nThread : retour = %ld", retour);

    EffaceEvt(Event1);
    EffaceEvt(Event2);
*/
    retour = AttendObjetsTantQue(10000/55, NOYAU_VRAI, 2, Event1, Event2);

    printf ("\nThread : retour = %ld", retour);

    while(1)
        DelaiTache(10);

    return 0;
}


void main (void)
{
    DWORD Id;


    
    DebutRegion();
    
    CreateThread (NULL, 0, ThreadA, NULL, 0, &Id);
    
    printf ("\nUne Touche pour signaler l'Event 1...");
	_getch();

    SignalEvt (Event1);

    FinRegion();

    printf ("\nUne Touche pour signaler l'Event 2...");
    _getch();

    SignalEvt(Event2);

    _getch();
}