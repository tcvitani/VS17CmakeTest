*** Module EMI ***

Module qui gere les fichiers-messages :
Il assure :
- l'emission automatique des fichiers vers le PCS
- les demandes de backup (par date ou numero de fichier)
- les purges quotidiennes
Ce module utilise la DLL pcs_msg_files.dll de OUTILS_C.

REMARQUE :
Actuellement ce module gere le nommage des fichiers-messages de la facon suivante :
- en local : xxxxxxxx.yyy avec 0 < xxxxxxxx < 99999999 et 0 <= yyy (yyy = nb emission(s) reussie(s) du fichier vers le PCS)
- sur le serveur distant : MSG.PCSpppp.Lllll.Sxxxxxxxx ou BAK.PCSpppp.Lllll.Sxxxxxxxx avec pppp = numero de gare (plaza) et llll = numero de voie (lane).


****************************


Version 1.0.1 : AFX 02/12/1999
- correction backup par date.


Version 1.1.0 : ECSAT 12/04/2001
The module was linked to the csr_pipe.lib library.
It was impossible to reconfigure the module to use the csr_sock module for network communication.
To enable the configuration of the module the EmiEnvoiLan function has been added in the emi_mess.c file.
This function is used to send the message to the LAN module (csr_sock or csr_pipe) instead of EnvoiLan function which has been placed inside the LAN module so it was necessary to link the EMI_FIC module to one of the LAN libraries (csr_pipe.lib or csr_sock.lib).
After changes the link is no longer useful and the module can easily be reconfigured to use different network protocols by running the desired LAN module (csr_sock.dll or csr_pipe.dll).

