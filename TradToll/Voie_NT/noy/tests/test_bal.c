#include <noyau.h>
#include <conio.h>
#include <time.h>
//#include <noy_dbg.h>

#define LIMIT 10000L

DWORD WINAPI ThreadB (PVOID param)
{
    noyau_bal_id bal_emi;
    noyau_bal_id bal_rec;
    struct_neutre *pp_neutre;
    DWORD i;
    
    printf ("\nThread Lancé !");

    bal_rec = PublieBAL ("BL_REC", LIMIT); 
    
    bal_emi = AttendBAL ("BL_EMI");

    i = LIMIT;
 
    while(i-- > 0)
    {
        Recoit(bal_rec, (struct_neutre **) &pp_neutre, NOYAU_ATTENTE_INFINIE);

        if (Envoie (bal_emi, bal_rec, pp_neutre) == NOYAU_BAL_PLEINE)
            printf ("Bal pleine !");
    }

    return 0;
}


void main (void)
{
    DWORD Id;
    noyau_bal_id bal_rec;
    noyau_bal_id bal_emi;
    struct_neutre *pp_neutre;  
    time_t ltime;
    unsigned int i;

    CreateThread (NULL, 0, ThreadB, NULL, 0, &Id);
    
    bal_emi = PublieBAL ("BL_EMI", LIMIT); 
    bal_rec = AttendBALTantQue ("BL_REC", 10000/55);

    getch();

    time( &ltime );   
    printf ("Démarrage des %u envois %s\n", LIMIT, ctime( &ltime ));

    i = 0;

    while(i++ < LIMIT)
    {
        ExitAlloue (&pp_neutre, sizeof(struct_neutre), 0);
        
        if (Envoie (bal_rec, bal_emi, pp_neutre) == NOYAU_BAL_PLEINE)
            printf ("Bal pleine !");

        Recoit (bal_emi, &pp_neutre, NOYAU_ATTENTE_INFINIE);

        ExitLibere (&pp_neutre);
    }

    time( &ltime );   
    printf ("Fin des receptions %s\n", ctime( &ltime ));
}