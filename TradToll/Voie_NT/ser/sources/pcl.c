
/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL SERIE
* FICHIER: SERIE.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE: 
* --------------------------------------------------------------------
* RESUME: Fonctions publiques du module liaisons series 
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log : $
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <serie.h>
#define LOC_DEF
#include <pcl_loc.h>

#include <stdio.h>
#include <reg.h>

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- DEFINES: ----------------*/

#define WRITETOTALTIMEOUTMULTIPLIER 10
#define WRITETOTALTIMEOUTCONSTANT 1000

#define SER_HANDLE_COM_HS (HANDLE) -2
#define SER_HANDLE_COM_FAKEOPEN (HANDLE) -3

/*--------------- FONCTIONS: ---------------*/

// pour tests sans debug du noyau...
// void WINAPI DBG_EcritFichierErreurs (int nom_trace, void *debug, char *fmt,...) {}
// void WINAPI DBG_EcritFichierTraces (int nom_trace, void *debug, char *fmt,...) {}

PRIVATE char * SerPrint( char * pcString, 
						 size_t spcStringSize,
                         DWORD * pdwSize, 
                         char * pcFormat, 
                         ... );

PRIVATE HANDLE SerTryReopen( short int numero_port );


/*--------------- CODE: ---------------*/

PRIVATE Ser_enum_cpt_erreur_t Donne_Port (short int numero_port, Ser_struct_port **port)
{
    
    // au cas o erreur et si qd meme utilis, levera une exception
    *port = NULL;
    
    // teste si index est dans la limite
    if (numero_port < 1 || numero_port > SER_NB_PORTS_MAX)
        return SER_CPT_NUM_PORT_INCORRECT;
    
    // un raccourci
    *port = &_SER_.port[numero_port];
    
    // teste si port est dclar HS
    if ((*port)->hComm == SER_HANDLE_COM_HS)
        return SER_CPT_PORT_INEXISTANT;
    
    // teste si port est ferm
    if ((*port)->hComm == INVALID_HANDLE_VALUE)
        return SER_CPT_PORT_NON_OUVERT;
    
    return SER_CPT_OK;
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerFermePort( short int numero_port)
{
    Ser_enum_cpt_erreur_t retour;
    Ser_struct_port *port;
    BOOL fBackup;
    
    retour = Donne_Port (numero_port, &port);
    if (retour != SER_CPT_OK)
    {
        // teste si port dclar absent
        if (retour == SER_CPT_PORT_INEXISTANT)
        {
            DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerFermePort(%d) : retour = HS", 
            numero_port);

            return SER_CPT_OK;
        }

        // on ne peut pas faire de traces d'erreurs sur un port qui n'est pas ouvert !
        return retour;
    }
    
    retour = SER_CPT_OK;
    
    // remise en place de l'ancien paramtrage
    // REM : la fonction BuildCommDCBAndTimeouts() lve une exception...
    if ( SerTryReopen( numero_port ) != SER_HANDLE_COM_FAKEOPEN )
    {
        if (!SetCommState(port->hComm, &port->dcb))
            retour = SER_CPT_PARAMETRE_INCORRECT;
        if (!SetCommTimeouts(port->hComm, &port->timeouts))
            retour = SER_CPT_PARAMETRE_INCORRECT;
    
        // fermeture des handles qq soit rsultat prcdent
        if (port->hComm != INVALID_HANDLE_VALUE)
            if (!CloseHandle (port->hComm))
                retour = SER_CPT_NOK;
    }

    DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerFermePort(%d) : retour = %ld", 
            numero_port, retour);

    // Arret des traces !
    if( DBG_Arret(&port->dbg) != DBG_OK )
        retour = SER_CPT_NOK;
        
    // RAZ du port qq soit le rsultat prcdent
    fBackup = port->fAllowOpenError;
    memset (port, 0, sizeof (Ser_struct_port));
    port->hComm = INVALID_HANDLE_VALUE;
    port->fAllowOpenError = fBackup;

    return retour;
}

PRIVATE Ser_enum_cpt_erreur_t SerLanceTraces (short int numero_port)
{
    DWORD dwLen;
    DWORD dwTailleMax;
    char pcKey[MAX_PATH];
    char pcFicName[MAX_PATH];
    dbg_struct_debug *debug;
    dbg_struct_trace *tab_traces;
    Ser_enum_cpt_erreur_t retour;
    Ser_struct_port *port;
    
    retour = Donne_Port (numero_port, &port);
    if (retour != SER_CPT_OK && retour != SER_CPT_PORT_INEXISTANT)
        return retour;
    
    debug = &port->dbg;
    tab_traces = &port->tab_traces[SER_TRC];
    
	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG);
    
    // chemin traces
    dwLen = sizeof( debug->rep_fichiers_traces );
    if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_TRACEPATH, debug->rep_fichiers_traces, &dwLen ) != ERROR_SUCCESS )
        return SER_CPT_NOK;
    
    // chemin erreurs
    dwLen = sizeof( debug->rep_fichier_erreurs );
    if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_ERRORPATH, debug->rep_fichier_erreurs, &dwLen ) != ERROR_SUCCESS )
        return SER_CPT_NOK;
    
    // Taille max des fichiers
    if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_FILEMAXSIZE, &dwTailleMax ) != ERROR_SUCCESS )
        return SER_CPT_NOK;
    debug->taille_limite = dwTailleMax;
    
	sprintf_s(pcFicName, sizeof(pcFicName), "SER_COM%d", numero_port);
    
    // pour initialiser les traces  l'cran
    // chaque port du module SER aura son propre
    // fichier donn par son numero
	strcpy_s(debug->nom_fichier_traces_ecran, sizeof(debug->nom_fichier_traces_ecran), pcFicName);
    
    // chaque port du module SER aura galement son propre
    // fichier d'erreurs donn par numro
	strcpy_s(debug->nom_fichier_erreurs, sizeof(debug->nom_fichier_erreurs), pcFicName);
    
    // chaque port du module SER aura son propre fichier de trace 
	strcpy_s(tab_traces->nom, sizeof(tab_traces->nom), pcFicName);
    
    debug->tab_traces = tab_traces;
    debug->nb_fichiers_traces = SER_NB_TRACES;
    
    if( DBG_Lance(debug) != DBG_OK )
        return SER_CPT_NOK;
    
    return SER_CPT_OK;
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerOuvrePort(short int numero_port,
                                                 unsigned char *mode_command_line)
{
    Ser_enum_cpt_erreur_t retour;
    DCB dcb;
    COMMTIMEOUTS timeouts;
    CHAR string[_MAX_PATH]; 
    Ser_struct_port *port;
    
    retour = Donne_Port (numero_port, &port);
    switch(retour)
    {
    case SER_CPT_OK:
        return SER_CPT_PORT_DEJA_OUVERT;
        
    case SER_CPT_PORT_INEXISTANT:
    case SER_CPT_PORT_NON_OUVERT:
        break;
        
    default:
        return retour;
    }
    
    __try 
    {
        // teste si port dclar absent
        if (retour == SER_CPT_PORT_INEXISTANT)
        {
            if (SerLanceTraces(numero_port) != SER_CPT_OK)
                return (retour = SER_CPT_PARAMETRE_INCORRECT);

            DBG_EcritFichierErreurs(SER_TRC, &port->dbg,"SerOuvrePort(%d,%s) : retour = HS", 
                numero_port, mode_command_line);

            return (retour = SER_CPT_OK);
        }

        if ( strlen( mode_command_line ) >= sizeof( port->mode_command_line ) )
            return (retour = SER_CPT_PARAMETRE_INCORRECT);
		strcpy_s(port->mode_command_line, sizeof(port->mode_command_line), mode_command_line);
        
        retour = SER_CPT_OK;
        
        // on construit le chemin d'accs au port
		sprintf_s(string, sizeof(string), "\\\\.\\COM%d", numero_port);
        
        port->dwLastAttemptTick = GetTickCount();

        // on l'ouvre en overlapped (asynchrone) -> Wrong comment
        port->hComm = CreateFile( 
            string,  
            GENERIC_READ | GENERIC_WRITE, 
            0, 
            0, 
            OPEN_EXISTING,
            0,
            0);
        
        if (port->hComm == INVALID_HANDLE_VALUE)
            if ( ! port->fAllowOpenError )
                return (retour = SER_CPT_PORT_INEXISTANT);
            else
                port->hComm = SER_HANDLE_COM_FAKEOPEN;
        
        if ( port->hComm != SER_HANDLE_COM_FAKEOPEN )
        {
            __try
            {
                // sauvegarde des paramtres courants du port
                if (!GetCommTimeouts(port->hComm, &port->timeouts))
                {
                    retour = SER_CPT_NOK;
                    __leave;
                }
    
                if (!GetCommState(port->hComm, &port->dcb))
                {
                    retour = SER_CPT_NOK;
                    __leave;
                }
    
                // suppression des timeouts du port (!= timeouts des fonctions win32)
                timeouts.ReadIntervalTimeout = MAXDWORD;
                timeouts.ReadTotalTimeoutMultiplier = 0;
                timeouts.ReadTotalTimeoutConstant = 500;
                timeouts.WriteTotalTimeoutMultiplier = WRITETOTALTIMEOUTMULTIPLIER;
                timeouts.WriteTotalTimeoutConstant = WRITETOTALTIMEOUTCONSTANT;
    
                if (!SetCommTimeouts(port->hComm, &timeouts))
                {
                    retour = SER_CPT_PARAMETRE_INCORRECT;
                    __leave;
                }
    
                // paramtrage du port format commande en ligne 'MODE' du DOS
                memset(&dcb, 0, sizeof(dcb));
                dcb.DCBlength = sizeof(dcb);
    
                // remplissage de la structure DCB  partir des paramtres de la ligne de commande
                if (!BuildCommDCB(mode_command_line, &dcb))
                {
                    retour = SER_CPT_PARAMETRE_INCORRECT;
                    __leave;
                }
    
                    /* pour debug ...     
        
                      printf ("DCB : %s\n%lu\n%lu\n%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu \n%u\n%u\n%u \n%u %u %u\n%d %d %d %d %d\n%u", 
                      mode_command_line,
                      dcb.DCBlength,           // sizeof(DCB) 
                      dcb.BaudRate,            // current baud rate 
                      dcb.fBinary,          // binary mode, no EOF check 
                      dcb.fParity,          // enable parity checking 
                      dcb.fOutxCtsFlow,      // CTS output flow control 
                      dcb.fOutxDsrFlow,      // DSR output flow control 
                      dcb.fDtrControl,       // DTR flow control type 
                      dcb.fDsrSensitivity,   // DSR sensitivity 
                      dcb.fTXContinueOnXoff, // XOFF continues Tx 
                      dcb.fOutX,            // XON/XOFF out flow control 
                      dcb.fInX,             // XON/XOFF in flow control 
                      dcb.fErrorChar,   // enable error replacement 
                      dcb.fNull,            // enable null stripping 
                      dcb.fRtsControl,       // RTS flow control 
                      dcb.fAbortOnError,     // abort reads/writes on error 
                      dcb.fDummy2,          // reserved 
                      dcb.wReserved,            // not currently used 
                      dcb.XonLim,               // transmit XON threshold 
                      dcb.XoffLim,              // transmit XOFF threshold 
                      dcb.ByteSize,         // number of bits/, 4-8 
                      dcb.Parity,               // 0-4=no,odd,even,mark,space 
                      dcb.StopBits,             // 0,1,2 = 1, 1.5, 2 
                      dcb.XonChar,          // Tx and Rx XON acter 
                      dcb.XoffChar,        // Tx and Rx XOFF acter 
                      dcb.ErrorChar,        // error replacement acter 
                      dcb.EofChar,          // end of input acter 
                      dcb.EvtChar,       // received event acter 
                      dcb.wReserved1);
                */
    
                // pb apres remplissage au niveau des char Xon/Xoff  0
                dcb.XonChar = 2;
                dcb.XoffChar = 3;
    
                // mise en place du nouveau paramtrage
                if (!SetCommState(port->hComm, &dcb))
                {
                    retour = SER_CPT_PARAMETRE_INCORRECT;
                    __leave;
                }

            }
            __finally
            {
                if ( ( retour != SER_CPT_OK ) && port->fAllowOpenError )
                {
                    CloseHandle( port->hComm );
                    port->hComm = SER_HANDLE_COM_FAKEOPEN;
                    retour = SER_CPT_OK;
                }
            }

            if ( retour != SER_CPT_OK )
                __leave;
        }
        
        // REM : la fonction BuildCommDCBAndTimeouts() lve une exception...
        
        if (SerLanceTraces(numero_port) != SER_CPT_OK)
            return (retour = SER_CPT_PARAMETRE_INCORRECT);
    }
    
    __finally
    {
        // si erreur, ferme port pour librer les handles
        if (retour != SER_CPT_OK)
        {
            DBG_EcritFichierErreurs(SER_TRC, &port->dbg,"SerOuvrePort(%d,%s) : Echec retour = %ld LastError = 0x%lx", 
                numero_port, mode_command_line, retour, GetLastError());
            SerFermePort (numero_port);
        }
    }
    
    DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerOuvrePort(%d,%s) : retour = %ld", 
        numero_port, mode_command_line, retour);

    return retour;
}


EXPORT Ser_enum_cpt_erreur_t WINAPI SerEcritMessage(short int numero_port,
                                                    char *message,
                                                    unsigned long longueur)
{
    DWORD ecrits;
    Ser_enum_cpt_erreur_t retour;
    Ser_struct_port *port;
    char BytesSent[MAX_PATH];
    char * pBytesSent;
    DWORD i, l;
    
    retour = Donne_Port (numero_port, &port);
    if (retour != SER_CPT_OK)
    {
        // teste si port dclar absent
        if (retour == SER_CPT_PORT_INEXISTANT)
        {
            l = sizeof(BytesSent);
            pBytesSent = BytesSent;
            for (i=0;i<longueur;i++)
            {
				pBytesSent = SerPrint(pBytesSent, sizeof(BytesSent), &l, "%02X ", (BYTE)message[i]);
                if (pBytesSent == NULL)
                    break;
            }
            
            DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerEcrit(%d,'%s',%lu) : ecrits = HS", 
                numero_port, BytesSent, longueur);
            
            return SER_CPT_OK;
        }

        l = sizeof(BytesSent);
        pBytesSent = BytesSent;
        for (i=0;i<longueur;i++)
        {
			pBytesSent = SerPrint(pBytesSent, sizeof(BytesSent), &l, "%02X ", (BYTE)message[i]);
            if (pBytesSent == NULL)
                break;
        }

        // sinon erreur !
        DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerEcrit(%d,'%s',%lu) : retour = %ld ecrits = 0", 
            numero_port, BytesSent, longueur, retour);
        
        return retour;
    }

    retour = SER_CPT_OK;
    
    // ecriture asynchrone
    if ( SerTryReopen( numero_port ) != SER_HANDLE_COM_FAKEOPEN )
        WriteFile(port->hComm, message, longueur, &ecrits, NULL);
    else
        ecrits = longueur;
    
    if (ecrits == 0)
        retour = SER_CPT_ECRIT_INCOMPLET;
    else
        // teste si tous les octets ont t crits
        if (ecrits != longueur)
            retour = SER_CPT_ECRIT_INCOMPLET;

    if (DBG_FichierTracesPresent (SER_TRC, &port->dbg) == TRUE)
    {
        l = sizeof(BytesSent);
        pBytesSent = BytesSent;
        for (i=0;i<longueur;i++)
        {
			pBytesSent = SerPrint(pBytesSent, sizeof(BytesSent), &l, "%02X ", (BYTE)message[i]);
            if (pBytesSent == NULL)
                break;
        }

        DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerEcrit(%d,'%s',%lu) : retour = %ld ecrits = %lu", 
            numero_port, BytesSent, longueur, retour, ecrits);
    }
        
    return retour;
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerEcritCaractere (short int numero_port,
                                                       char carac)
{
    return SerEcritMessage (numero_port, &carac, 1UL);
}


EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitMessage(short int numero_port,
	char *message,
	unsigned long longueur,
	unsigned long *lus,
	unsigned long delai)
{
	return  SerLitMessageMs(numero_port, message, longueur, lus, delai*55);
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitMessageMs(short int numero_port,
                                                  char *message,
                                                  unsigned long longueur,
                                                  unsigned long *lus,
                                                  unsigned long delaiMs)
{
    Ser_struct_port *port;
    Ser_enum_cpt_erreur_t retour;
    COMMTIMEOUTS timeouts;
    char BytesSent[MAX_PATH];
    char * pBytesSent;
    DWORD i, l;
    
    retour = Donne_Port (numero_port, &port);
    if (retour != SER_CPT_OK)
    {
        // teste si port dclar absent
        if (retour == SER_CPT_PORT_INEXISTANT)
        {
            // simule l'attente jusqu'au time out
            Sleep(delaiMs);
            
            // ne lit rien
            *lus = 0;
            
            DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerLit(%d,message,%lu, %lu, %lu ms) : retour = HS", 
                numero_port, longueur, *lus, delaiMs);
            
            return SER_CPT_DONNEES_ABSENTES;
        }

        // sinon erreur !
        DBG_EcritFichierErreurs(SER_TRC, &port->dbg,"SerLit(%d,message,%lu, *lus, %lu ms) : retour = %ld", 
            numero_port, longueur, delaiMs, retour);
        
        return retour;
    }

    if ( SerTryReopen( numero_port ) != SER_HANDLE_COM_FAKEOPEN )
    {
        // suppression des timeouts du port (!= timeouts des fonctions win32)
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = delaiMs;
        timeouts.WriteTotalTimeoutMultiplier = WRITETOTALTIMEOUTMULTIPLIER;
        timeouts.WriteTotalTimeoutConstant = WRITETOTALTIMEOUTCONSTANT;
    
        SetCommTimeouts(port->hComm, &timeouts);
    
        retour = SER_CPT_OK;
    
        // Lecture asynchrone
        ReadFile(port->hComm, message, longueur, lus, NULL);
    }
    else
    {
        Sleep( delaiMs );
        *lus = 0;
    }
    if (*lus == 0)
        retour = SER_CPT_DONNEES_ABSENTES;
    else
        // teste si tous les octets ont t lus
        if (*lus != longueur)
            retour = SER_CPT_DONNEES_INCOMPLETES;
        
    if (DBG_FichierTracesPresent (SER_TRC, &port->dbg) == TRUE)
    {
        l = sizeof(BytesSent);
        pBytesSent = BytesSent;
        for (i=0;i<longueur;i++)
        {
			pBytesSent = SerPrint(pBytesSent, sizeof(BytesSent), &l, "%02X ", (BYTE)message[i]);
            if (pBytesSent == NULL)
                break;
        }

        DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerLit(%d,'%s',%lu, %lu, %lu ms) : retour = %ld", 
            numero_port, BytesSent, longueur, *lus, delaiMs, retour);
    }
        
    return retour;
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitCaractereMs(short int numero_port,
	char * car_lu,
	unsigned long delaiMs)
{
	unsigned long lus;

	return SerLitMessageMs(numero_port, car_lu, 1UL, &lus, delaiMs);
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitCaractere (short int numero_port,
                                                     char * car_lu,
                                                     unsigned long delai)
{
    unsigned long lus;
    
    return SerLitMessageMs (numero_port, car_lu, 1UL, &lus, delai * 55);
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitMessageComplet(short int numero_port,
	char *message,
	unsigned long longueur,
	unsigned long *lus,
	unsigned long delai)
{
	return SerLitMessageCompletMs(numero_port, message, longueur, lus, delai * 55);
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitMessageCompletMs(short int numero_port,
                                                         char *message,
                                                         unsigned long longueur,
                                                         unsigned long *lus,
                                                         unsigned long delaiMs)
{
    unsigned long offset;
    unsigned long reste_a_lire;
    Ser_enum_cpt_erreur_t retour;
    
    retour = SER_CPT_OK;
    
    offset = 0;
    reste_a_lire = longueur;
    while (reste_a_lire > 0)
    {
        // lecture d'un bloc
        retour = SerLitMessageMs (numero_port, &message[offset], reste_a_lire, lus, delaiMs);
        
        // selon le nombre d'octets effectivement lus pour ce bloc
        // on dtermine s'il en reste encore  lire et combien
        if (retour == SER_CPT_OK || retour == SER_CPT_DONNEES_INCOMPLETES)
        {
            offset += *lus;
            reste_a_lire -= *lus;
        }
        else
        {
            // erreur en reception (time out)
            break;
        }
    }
    
    // calcul nbre de caracteres lus
    *lus = longueur - reste_a_lire; 
    
    return retour;
}

EXPORT Ser_enum_booleen_t WINAPI SerDonneDSR (short int numero_port)
{
    DWORD dwStatus;
    Ser_struct_port *port;
    
    if (Donne_Port (numero_port, &port) != SER_CPT_OK)
    {
        // erreur !
        DBG_EcritFichierErreurs(SER_TRC, &port->dbg,"SerDonneDSR(%d) : retour = %ld", 
            numero_port, GetLastError());

        return SER_FALSE;
    }
    
    if ( SerTryReopen( numero_port ) != SER_HANDLE_COM_FAKEOPEN )
    {
        if (GetCommModemStatus (port->hComm, &dwStatus) == FALSE)
        {
            // erreur !
            DBG_EcritFichierErreurs(SER_TRC, &port->dbg,"SerDonneDSR(%d) : retour = %ld", 
                numero_port, GetLastError());
        
            return SER_FALSE;
        }
    }
    else
    {
        dwStatus = 0;
    }

    return (dwStatus & MS_DSR_ON) > 0;
}

EXPORT Ser_enum_booleen_t WINAPI SerDonneCTS (short int numero_port)
{
    DWORD dwStatus;
    Ser_struct_port *port;
    
    if (Donne_Port (numero_port, &port) != SER_CPT_OK)
    {
        // erreur !
        DBG_EcritFichierErreurs(SER_TRC, &port->dbg,"SerDonneCTS(%d) : retour = %ld", 
            numero_port, GetLastError());

        return SER_FALSE;
    }
    
    if ( SerTryReopen( numero_port )  != SER_HANDLE_COM_FAKEOPEN )
    {
        if (GetCommModemStatus (port->hComm, &dwStatus) == FALSE)
        {
            // erreur !
            DBG_EcritFichierErreurs(SER_TRC, &port->dbg,"SerDonneCTS(%d) : retour = %ld", 
                numero_port, GetLastError());
        
            return SER_FALSE;
        }
    }
    else
        dwStatus = 0;

    return (dwStatus & MS_CTS_ON) > 0;
}

EXPORT Ser_enum_booleen_t WINAPI SerIsPresentChar(short int numero_port)
{
    Ser_enum_cpt_erreur_t retour;
    DWORD dwCommEvent;
    Ser_struct_port *port;
    
    retour = Donne_Port (numero_port, &port);
    if (retour != SER_CPT_OK)
        return SER_FALSE;
    
    if ( SerTryReopen( numero_port ) != SER_HANDLE_COM_FAKEOPEN )
    {
        // positionne le masque de "prsence caractre en lecture"
        if (!SetCommMask(port->hComm, EV_RXCHAR))
            return SER_FALSE;
    
        // attente asynchrone de l'event correspondant au masque -> Wrong comment, this is not overlapped, so blocking call
        if (!WaitCommEvent(port->hComm, &dwCommEvent, NULL)) 
            return SER_FALSE;
    }
    else
    {
        // Prevent from active loops sticking processes
        Sleep( 55 );
        return SER_FALSE;
    }

    return SER_TRUE;
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerRazTampon (short int numero_port,  
                                                  Ser_enum_type_tampon_t type_tampon)
{
    Ser_enum_cpt_erreur_t retour;
    Ser_struct_port *port;
    
    retour = Donne_Port (numero_port, &port);
    if (retour != SER_CPT_OK)
    {
        // teste si port dclar absent
        if (retour == SER_CPT_PORT_INEXISTANT)
        {
            DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerRazTampon(%d,%lu) : retour = HS", 
                numero_port, type_tampon);

            return SER_CPT_OK;
        }

        // sinon erreur
        DBG_EcritFichierErreurs(SER_TRC, &port->dbg,"SerRazTampon(%d,%lu) : retour = %ld", 
            numero_port, type_tampon, retour);

        return retour;
    }
    
    retour = SER_CPT_OK;
    
    switch (type_tampon)
    {
    case SER_TAMPON_RECEPTION:
        // vide le tampon de rception (caractres seront perdus)
        if ( SerTryReopen( numero_port ) != SER_HANDLE_COM_FAKEOPEN )
        {
            if (!PurgeComm (port->hComm, PURGE_RXABORT|PURGE_RXCLEAR))
                retour = SER_CPT_NOK;
        }
        break;
        
    case SER_TAMPON_EMISSION:
        // vide le tampon d'mission (car. ne seront pas mis != de FlushFileBuffers() !)
        if ( SerTryReopen( numero_port ) != SER_HANDLE_COM_FAKEOPEN )
        {
            if (!PurgeComm (port->hComm, PURGE_TXABORT|PURGE_TXCLEAR))
                retour = SER_CPT_NOK;
        }
        break;
        
    default:
        retour = SER_CPT_PARAMETRE_INCORRECT;
        break;
    }
    
    DBG_EcritFichierTraces(SER_TRC, &port->dbg,"SerRazTampon(%d,%lu) : retour = %ld", 
        numero_port, type_tampon, retour);
    
    return retour;
}

EXPORT  Ser_enum_cpt_erreur_t WINAPI SerEmetBreak (short int numero_port, 
                                                   short int duree_break)
{
    Ser_enum_cpt_erreur_t retour;
    Ser_struct_port *port;
    
    retour = Donne_Port (numero_port, &port);
    if (retour != SER_CPT_OK)
    {
        // teste si port dclar absent
        if (retour == SER_CPT_PORT_INEXISTANT)
        {
            // simule l'attente
            Sleep (duree_break * 55);
            
            return SER_CPT_OK;
        }
        
        // sinon erreur !
        return retour;
    }

    // suspend la transmission de caractres sur le port
    if ( SerTryReopen( numero_port ) != SER_HANDLE_COM_FAKEOPEN )
    {
        if (!SetCommBreak (port->hComm))
            return SER_CPT_NOK;
    }

    // bloquant le temps du break (ne sait pas comment le faire simplement sans bloquer)
    Sleep (duree_break * 55); // conserve tick PC de 55 ms sur MOXA
    
    // reprend la transmission de caractres sur le port
    if ( SerTryReopen( numero_port ) != SER_HANDLE_COM_FAKEOPEN )
    {
        if (!ClearCommBreak (port->hComm))
            return SER_CPT_NOK;
    }
    
    return SER_CPT_OK;
}

EXPORT Ser_enum_booleen_t WINAPI SerReset (void)
{
    WORD i;
    Ser_enum_booleen_t retour;
    
    // ferme brutalement tous les ports 
    retour = SER_TRUE;
    for (i = 1 ; i < SER_NB_PORTS_MAX ; i++)
        if (SerFermePort (i) != SER_CPT_OK)
            retour = SER_FALSE;
        
        return retour;
}

EXPORT Ser_enum_cpt_erreur_t WINAPI SerConfig (short int numero_port, short int device)
{
    // Pour eviter les Warnings ! 
    numero_port = numero_port;
    device = device;
    
    return SER_CPT_FONCTION_NON_IMPLEMENTEE;
}


PRIVATE void SerInit (void)
{
    int i;
    char sCom[_MAX_PATH];
    DWORD Bool;    

    /* efface toute la structure */
    memset (&_SER_, 0, sizeof(Ser_struct_locale));
    
    /* valeur par defaut des handles */
    for (i=0; i<=SER_NB_PORTS_MAX;i++)
    {
        _SER_.port[i].hComm = INVALID_HANDLE_VALUE;

		sprintf_s(sCom, sizeof(sCom), SER_REG_KEYn_COM_HS, i);
       
        // Teste si COM est en version HS (sans hard)
        if (REG_Lire_Entier (
            CSR_REG_KEYi_ROOT, 
            CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG MOD_REG_KEYn_MODULES 
            SER_REG_KEYn,
            sCom,
            &Bool) == ERROR_SUCCESS)
        {
            // port dclar HS ?
            if (Bool == 1)
                _SER_.port[i].hComm = SER_HANDLE_COM_HS;
            else if (Bool == 2)
                _SER_.port[i].fAllowOpenError = TRUE;
        }
    }

    if (REG_Lire_Entier (
        CSR_REG_KEYi_ROOT, 
        CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_CONFIG MOD_REG_KEYn_MODULES 
        SER_REG_KEYn,
        "RetryOpenDelay",
        &_RetryOpenDelay_ ) != ERROR_SUCCESS )
        _RetryOpenDelay_ = 30000;
}
    
extern int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) 
    {  
        // The DLL is loading due to process  
        // initialization or a call to LoadLibrary.  
    case DLL_PROCESS_ATTACH:
        // if (SER_NB_INSTANCES++ == 0)
            SerInit();
        
        // The attached process creates a new thread.  
    case DLL_THREAD_ATTACH:  
        break; 
        
        // The thread of the attached process terminates. 
    case DLL_THREAD_DETACH:  
        break;  
        
        // The DLL unloading due to process termination or call to FreeLibrary. 
    case DLL_PROCESS_DETACH:  
        //if (SER_NB_INSTANCES > 0)
        //    SER_NB_INSTANCES--;
        break;          
        
    default:             
        break;     
    }      
    
    return TRUE; 
    
    UNREFERENCED_PARAMETER(hInstance);     
    UNREFERENCED_PARAMETER(lpReserved); 
} 

PRIVATE char * SerPrint( char * pcString,
						 size_t spcStringSize,
                         DWORD * pdwSize, 
                         char * pcFormat, 
                         ... )
{
    DWORD dwSize = (*pdwSize);
    int iCount;
    va_list sArgs;

    if ( ( pcString != NULL ) && ( dwSize > 0 ) )
    {
        va_start( sArgs, pcFormat );
        //iCount = _vsnprintf_s( pcString,spcStringSize, dwSize, pcFormat, sArgs );
#pragma warning(push)
#pragma warning(disable: 4996) //Disable security warning for this block of code
		iCount = _vsnprintf(pcString, dwSize, pcFormat, sArgs);
#pragma warning(pop)
		//va_end(sArgs);
        if ( ( iCount < 0 ) || ( (DWORD)iCount == dwSize ) )
        {
            pcString[dwSize-1] = '\0';
            (*pdwSize) = 0;
            return NULL;
        }

        (*pdwSize) = dwSize - (DWORD)iCount;
        pcString += iCount;
        return pcString;
    }
    else
        return NULL;
}


PRIVATE HANDLE SerTryReopen( short int numero_port )
{
    Ser_struct_port * port;
    DCB dcb;
    COMMTIMEOUTS timeouts;
    CHAR string[_MAX_PATH];

    port = &_SER_.port[numero_port];

    // Make sure port configuration allows trying to reopen
    if ( ! port->fAllowOpenError )
        return port->hComm;

    // Make sure that previous attempt failed
    if ( port->hComm != SER_HANDLE_COM_FAKEOPEN )
        return port->hComm;

    // Make sure that enough time as ellapsed since last attempts
    // (avoid to overload the system by permanently trying to
    // open the ports)
    if ( ( GetTickCount() - port->dwLastAttemptTick ) < _RetryOpenDelay_ )
        return port->hComm;
    port->dwLastAttemptTick = GetTickCount();

    // on construit le chemin d'accs au port
	sprintf_s(string, sizeof(string), "\\\\.\\COM%d", numero_port);
        
    // on l'ouvre en overlapped (asynchrone) -> Wrong comment
    port->hComm = CreateFile( 
        string,  
        GENERIC_READ | GENERIC_WRITE, 
        0, 
        0, 
        OPEN_EXISTING,
        0,
        0);
        
    // Opening has failed, will be for a next time
    if ( port->hComm == INVALID_HANDLE_VALUE )
    {
        port->hComm = SER_HANDLE_COM_FAKEOPEN;
        return port->hComm;
    }
    
    // sauvegarde des paramtres courants du port
    if (!GetCommTimeouts(port->hComm, &port->timeouts))
    {
        CloseHandle( port->hComm );
        port->hComm = SER_HANDLE_COM_FAKEOPEN;
        return port->hComm;
    }
    
    if (!GetCommState(port->hComm, &port->dcb))
    {
        CloseHandle( port->hComm );
        port->hComm = SER_HANDLE_COM_FAKEOPEN;
        return port->hComm;
    }

    // suppression des timeouts du port (!= timeouts des fonctions win32)
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 500;
    timeouts.WriteTotalTimeoutMultiplier = WRITETOTALTIMEOUTMULTIPLIER;
    timeouts.WriteTotalTimeoutConstant = WRITETOTALTIMEOUTCONSTANT;

    if (!SetCommTimeouts(port->hComm, &timeouts))
    {
        CloseHandle( port->hComm );
        port->hComm = SER_HANDLE_COM_FAKEOPEN;
        return port->hComm;
    }

    // paramtrage du port format commande en ligne 'MODE' du DOS
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);

    // remplissage de la structure DCB  partir des paramtres de la ligne de commande
    if (!BuildCommDCB(port->mode_command_line, &dcb))
    {
        CloseHandle( port->hComm );
        port->hComm = SER_HANDLE_COM_FAKEOPEN;
        return port->hComm;
    }

        /* pour debug ...     
    
          printf ("DCB : %s\n%lu\n%lu\n%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu \n%u\n%u\n%u \n%u %u %u\n%d %d %d %d %d\n%u", 
          mode_command_line,
          dcb.DCBlength,           // sizeof(DCB) 
          dcb.BaudRate,            // current baud rate 
          dcb.fBinary,          // binary mode, no EOF check 
          dcb.fParity,          // enable parity checking 
          dcb.fOutxCtsFlow,      // CTS output flow control 
          dcb.fOutxDsrFlow,      // DSR output flow control 
          dcb.fDtrControl,       // DTR flow control type 
          dcb.fDsrSensitivity,   // DSR sensitivity 
          dcb.fTXContinueOnXoff, // XOFF continues Tx 
          dcb.fOutX,            // XON/XOFF out flow control 
          dcb.fInX,             // XON/XOFF in flow control 
          dcb.fErrorChar,   // enable error replacement 
          dcb.fNull,            // enable null stripping 
          dcb.fRtsControl,       // RTS flow control 
          dcb.fAbortOnError,     // abort reads/writes on error 
          dcb.fDummy2,          // reserved 
          dcb.wReserved,            // not currently used 
          dcb.XonLim,               // transmit XON threshold 
          dcb.XoffLim,              // transmit XOFF threshold 
          dcb.ByteSize,         // number of bits/, 4-8 
          dcb.Parity,               // 0-4=no,odd,even,mark,space 
          dcb.StopBits,             // 0,1,2 = 1, 1.5, 2 
          dcb.XonChar,          // Tx and Rx XON acter 
          dcb.XoffChar,        // Tx and Rx XOFF acter 
          dcb.ErrorChar,        // error replacement acter 
          dcb.EofChar,          // end of input acter 
          dcb.EvtChar,       // received event acter 
          dcb.wReserved1);
    */

    // pb apres remplissage au niveau des char Xon/Xoff  0
    dcb.XonChar = 2;
    dcb.XoffChar = 3;

    // mise en place du nouveau paramtrage
    if (!SetCommState(port->hComm, &dcb))
    {
        CloseHandle( port->hComm );
        port->hComm = SER_HANDLE_COM_FAKEOPEN;
        return port->hComm;
    }

    return port->hComm;
}

 