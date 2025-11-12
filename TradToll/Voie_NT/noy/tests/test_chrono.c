#include <noyau.h>
#include <conio.h>
#include <time.h>


void main (void)
{
    noyau_chrono_id chrono;
    noyau_bal_id bal;
    struct_chrono *pp_chrono;
    time_t ltime;
    noyau_event_id evt;

    printf ("\nDémarrage du chrono\n\n");

    AlloueChrono(&chrono, "CHRONO_TEST");

    bal = PublieBAL ("BL_TEST", 0);

    LanceChronoCyclique(chrono, 0, 18*10, bal);

    AlloueEvent(&evt, "EVENT");

    while(1)
    {
        if (Recoit(bal, (struct_neutre **) &pp_chrono, NOYAU_ATTENTE_INFINIE) == NOYAU_BAL_MESS)
        {
            time( &ltime );   
            
            printf ("Chrono %d à %s\n", pp_chrono->numero, ctime( &ltime ));

            ExitLibere ((struct_neutre **) &pp_chrono);
        }
    }
}