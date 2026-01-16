
#ifndef CMHOST_H
#define CMHOST_H

//
// Incrément automatique (en nombre d'éléments) pour les extensions de listes allouées
//
#define HOST_LIST_INCREMENT 16


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT HANDLE WINAPI HostOpen( char * szName, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: szName     : Nom donnée à la connexion.
 *             hKeyConfig : Handle de clé de registre dans laquelle la
 *                          fonction va aller chercher les informations
 *                          dont elle à besoin pour configurer la connexion.
 * --------------------------------------------------------------------
 * RETURN    : Un handle de connexion en cas de succés d'initialisation.
 *             NULL en cas d'échec.
 * --------------------------------------------------------------------
 * ROLE      : Initialise une connexion RAS. Attention, un exécution
 *             réussie ne signifie pas une connexion établie, mais
 *             signifie uniquement que la connexion pourra avoir lieu
 *             en fonction du paramétrage. Un appel à la fonction
 *             HostQueryStatus permettra de connaitre l'état réel de la
 *             connexion RAS.
 *             Le paramétrage attendu par la fonction dans la clé du
 *             registre est le suivant (attention, la présence de
 *             toutes les valeurs est obligatoire) :
 *
 *        * TraceFile (REG_SZ) : Chemin d'accés du fichier de trace utilisé
 *         pour tracer le fonctionnement de la librairie. Si cette chaine
 *         est vide, aucune trace n'est effectuée. Sinon, les traces sont
 *         envoyées sur la console et dans le fichier (si et uniquement si
 *         celui-ci existe déjà).
 *
 *        * PhoneBook (REG_SZ) : Chemin d'accés au répertoire téléphonique
 *         (phonebook) contenant les informations sur les connexions RAS
 *         possibles. Si ce paramètre est vide, c'est le répertoir téléphonique
 *         par défaut qui sera utilisé (en principe, le fichier RASPHONE.PBK
 *         dans %SYSTEMROOT%\SYSTEM32\RAS).
 *
 *        * EntryName (REG_SZ) : Nom de l'entrée à utiliser dans le répertoire
 *         téléphonique spécifié avec le paramètre PhoneBook. Si ce paramètre
 *         est vide, la tentative de connexion aura lieu sur le premier modem
 *         disponible en utilisant le numéro de téléphone spécifié à l'aide du
 *         paramètre PhoneNumber.
 *
 *        * PhoneNumber (REG_SZ) : Numéro de téléphone à composer pour
 *         effectuer la connexion RAS en remplacement de celui de l'entrée du
 *         répertoire téléphonique défini par les paramètres PhoneBook et
 *         EntryName. Si cette chaine est vide, c'est le numéro de l'entrée
 *         du répertoire téléphonique qui sera utilisé.
 *
 *        * CallbackNumber (REG_SZ) : Numéro de téléphone utilisé en mode
 *         callback pour effectuer la connexion. Si ce paramètre n'est pas
 *         fourni ou s'il est vide, le mode callback n'est pas utilisé.
 *         Si ce paramètre vaut "*", c'est le paramètre défini dans le
 *         répertoire téléphonique qui sera utilisé.
 *
 *        * UserName / Password / Domain (REG_SZ) : Nom d'utilisateur, mot de
 *         passe et domaine transmis lors de la connexion RAS. Ces données seront
 *         utilisées par le serveur RAS pour autoriser ou non la connexion.
 *
 *        * DialTimes (REG_SZ) : Plages horaires pendant lesquelles les connexions
 *         RAS doivent avoir lieu. Il s'agit d'une liste d'intervalles exprimés
 *         sous la forme "HH:MM-HH:MM", séparés par un caractère "|". HH exprimant
 *         l'heure (de 0 à 23), MM exprimant les minutes (de 0 à 59). Lorsque la
 *         première heure est plus petite que la deuxième, il s'agit d'un intervale
 *         entièrement compris dans une journée. Lorsque la première heure est plus
 *         ou égale à la deuxième, il s'agit d'un intervale à cheval sur minuit.
 *
 *        * AbortTransfer (REG_DWORD) : Définit le comportement à suivre si la
 *         fin d'un créneau de connexion arrive alors qu'un transfert est en cours.
 *         Si la valeur est 0, la connexion RAS ne sera fermée que lorsqu'aucun
 *         transfert ne sera en cours. Sinon, la connexion RAS est fermée
 *         immédiatement.
 *
 *        * Simulated (REG_DWORD) : Définit si les appels RAS sont simulés ou non.
 *         Lorsque la valeur est différent de 0, les appels sont simulés et
 *         aucune action RAS n'a réellement lieu. Le système se comporte comme si
 *         les appels RAS se déroulaient toujours correctement.
 *
 *        * PollingPeriodNormal (REG_SZ) : Délai de scrutation du thread de gestion dans
 *         un context normal. Ce délai est forcé au 1000 ms s'il est inférieur.
 *
 *        * PollingPeriodShort (REG_SZ) : Délai de scrutation du thread de gestion dans
 *         un contexte ou l'exécution doit être rapide. Ce délai doit être inférieur ou
 *         égal à PollingPeriodNormal (peut être 0). Sa valeur est forcée à
 *         PollingPeriodNormal s'il est supérieur.
 *
 *        * PollingPeriodLong (REG_SZ) : Délai de scrutation du thread de gestion dans un
 *         contexte ou l'exécution doit être ralentie. Ce délai doit être à la fois
 *         supérieur à PollingPeriodNormal et à 3000 ms. Sa valeur est forcée au maximum
 *         des deux s'il est inférieur.
 *
 * --------------------------------------------------------------------
 */
EXPORT HANDLE WINAPI HostOpen( char * szName, HKEY hKeyConfig );

typedef HANDLE WINAPI CM_FCNX_OPEN  ( char * szName, HKEY hKeyConfig );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostQueryStatus( HANDLE hCnx )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx : Handle retourné par HostOpen
 * --------------------------------------------------------------------
 * RETURN    : TRUE lorsque la connexion RAS est établie. FALSE en
 *             cas de non connexion.
 * --------------------------------------------------------------------
 * ROLE      : Donne l'état courant de la connexion RAS.
 * --------------------------------------------------------------------
 */
EXPORT BOOL   WINAPI HostQueryStatus( HANDLE hCnx );

typedef BOOL   WINAPI CM_FCNX_STATUS( HANDLE hCnx );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void   WINAPI HostClose( HANDLE hCnx )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx : Handle retourné par HostOpen
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libère les resources alloués pour la gestion de
 *             la connexion RAS. En particulier, si une connexion
 *             est établie, celle-ci est rompue immédiatement.
 *             Au delà de cet appel, le handle hCnx n'est plus
 *             valide.
 * --------------------------------------------------------------------
 */
EXPORT void   WINAPI HostClose( HANDLE hCnx );

typedef void   WINAPI CM_FCNX_CLOSE ( HANDLE hCnx );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostPutFile( HANDLE hCnx, char * pcLocalPath , char * pcRemotePath )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle retourné par HostOpen
 *             pcLocalPath : Chemin local du fichier source.
 *             pcRemotePath: Chemin distant du fichier destination.
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de réussite. FALSE en cas d'échec.
 *             GetLastError() permettant d'avoir des détails sur
 *             l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectue un transfert ascendant (upload) d'un fichier.
 * --------------------------------------------------------------------
 */
EXPORT BOOL   WINAPI HostPutFile( HANDLE hCnx, char * pcLocalPath , char * pcRemotePath );

typedef BOOL   WINAPI CM_FCNX_PUT( HANDLE hCnx, char * pcLocalPath , char * pcRemotePath );





/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostGetFile( HANDLE hCnx, char * pcRemotePath, char * pcLocalPath )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle retourné par HostOpen
 *             pcRemotePath: Chemin distant du fichier source.
 *             pcLocalPath : Chemin local du fichier destination.
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de réussite. FALSE en cas d'échec.
 *             GetLastError() permettant d'avoir des détails sur
 *             l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectue un transfert descendant (download) d'un fichier.
 * --------------------------------------------------------------------
 */
EXPORT BOOL   WINAPI HostGetFile( HANDLE hCnx, char * pcRemotePath, char * pcLocalPath );

typedef BOOL   WINAPI CM_FCNX_GET( HANDLE hCnx, char * pcRemotePath, char * pcLocalPath );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostMoveFile( HANDLE hCnx, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle retourné par HostOpen
 *             pcRemoteSrc : Chemin distant du fichier source.
 *             pcRemoteDst : Chemin local du fichier destination. Si NULL, le
 *                           fichier source sera effacé.
 *             bCopy       : TRUE pour effectuer une copie, FALSE pour
 *                           effectuer un déplacement.
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de réussite. FALSE en cas d'échec.
 *             GetLastError() permettant d'avoir des détails sur
 *             l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectue un déplacement ou une copie d'une localisation distante
 *             vers une autre localisation distante.
 * --------------------------------------------------------------------
 */
EXPORT BOOL   WINAPI HostMoveFile( HANDLE hCnx, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy );

typedef BOOL   WINAPI CM_FCNX_MOVE( HANDLE hCnx, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostEnumFiles( HANDLE hCnx, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle retourné par HostOpen
 *             pcRemoteMask: Chemin distant du masque à utiliser pour l'énumération.
 *             ppsFound    : En cas de succés, retourne un pointeur sur un tableau de
 *                           structures de type WIN32_FIND_DATA contenant les informations
 *                           sur les fichiers énumérés.
 *             pdwCount    : En cas de succés, retourne le nombre de fichiers énumérés.
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de réussite. FALSE en cas d'échec.
 *             GetLastError() permettant d'avoir des détails sur
 *             l'erreur.
 *             En cas d'échec, ne pas tenir compte des valeurs retournés par
 *             ppsFound et pdwCount.
 * --------------------------------------------------------------------
 * ROLE      : Effectue une énumération des fichiers contenus dans un répetoire
 *             distant. Attention, cette fonction n'énumère pas les répertoires.
 *             Le tableau alloué et retourné par la fonction doit être désalloué
 *             à l'aide de la fonction HostFreeEnum.
 * --------------------------------------------------------------------
 */
EXPORT BOOL   WINAPI HostEnumFiles( HANDLE hCnx, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount );

typedef BOOL   WINAPI CM_FCNX_ENUM( HANDLE hCnx, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void   WINAPI HostFreeEnum( WIN32_FIND_DATA * psFind )
 * --------------------------------------------------------------------
 * PARAMETERS: psFind : Pointe sur un tableau d'énumération de fichiers
 *                      retourné par HostEnumFiles.
 * --------------------------------------------------------------------
 * RETURN    : Rien.
 * --------------------------------------------------------------------
 * ROLE      : Libère les resources alloués par un énumération de fichier.
 * --------------------------------------------------------------------
 */
EXPORT void   WINAPI HostFreeEnum( WIN32_FIND_DATA * psFind );

typedef void   WINAPI CM_FCNX_FREE( WIN32_FIND_DATA * psFind );




#endif
