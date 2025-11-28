
Version 1.0.1 - AFX 19/11/99
Referenc.dll :
- correction bug



Version 2.0.0 - AFX 05/12/1999
Referenc.dll et DLLs modules applicatifs:
- ajout publication BAL d'un module applicatif uniquement lorsque ce module est enregistre aupres de RFR
(l'application doit attendre la publication de chaque module qu'elle utilise)



Version 2.0.1 - FR 31/01/2001
Referenc.dll et rfr_tft_modular.dll:
- ajout du filtrage de l'émission des messages d'état vers le module PCS.
Pour que le filtrage ait lieu,il faut aussi adapter les DLLs des modules applicatifs (rfr_tci,rfr_aut,...).
Ce filtrage est necessaire pour les projets qui générent des messages d'acquittement de tables au LS, sinon
il n'apporte rien de plus que la version 2.0.0.



Version 2.0.3 - FR 21/03/2001
Referenc.dll:
- saut de la version 2.0.1 à la version 2.0.3 pour avoir la même version que le module rfr_tft_modular.dll
qui était déjà en version 2.0.2
- correction de la génération des messages d'état liés au traitement d'un nouveau fichier référence.
Cette correction va permettre aux applications voies de générer le status Lists Downloading (3/1/x).
- correction du blocage du module dès lors que le téléchargement d'une liste échouait (erreur de transfert
ou fichier recu de taille nulle).

!!!!!!!!!!!!!!!!!!! TRES IMPORTANT !!!!!!!!!!!!!!!!!!!!!!
Pour intégrer ce module, vous devez impérativement corriger TOUS vos modules rfr applicatifs
(rfr_tci, rfr_aut, ...) afin que ceux-ci ferment TOUJOURS le handle sur le fichier recu dans leur
fonction Verifier(). Actuellement, ce n'est pas le cas lorsque le fichier recu est de taille nulle par exemple.

rfr_tft_modular.dll:
- Lors du controle effectué suite à la réception d'un nouveau fichier, correction du bug de non fermeture du handle
sur le fichier recu lorsque celui-ci était vide (taille du fichier = 0).



Version 2.0.4 - ECSAT 03/05/2001
Referenc.dll:
The module was linked to the csr_pipe.lib library. It was impossible to reconfigure the module to use 
the csr_sock module for network communication. To solve this problem, the RC_OuvrirServiceFichier function 
has been changed so as not to use the EnvoiLan function from the LAN module.
There was also a problem with the reference file structure, which can have the file full path written close 
together with the file name (Dutra project). In this case it is necessary to ignore the distant_path registry 
value (on the other hand, if the path is not written in the reference file - that is when the file name doesn't 
start with the '\' sign -, the distant_path registry value has then to be concatenated with the reference file 
name ... the same way it was processed previously). The RC_DemanderChargementFichier function has been adapted 
to manage the distant_path value.

Version 2.0.5 - FR 24/09/2001
Referenc.dll:
- Ajout en base de registre du paramètre "sleep_duration_after_init_sec" qui permet au module de s'endormir
  après son initialisation et avant de traiter les premiers messages présents dans sa BAL.
  Cela permet aux modules RFR applicatifs de se référencer avant que le module ne traite un message de référence
  éventuellement présent dans sa BAL.
  Auparavant, cet endormissement était "en dur" de 2 secondes.
  Si le paramètre est absent de la base de registre, la valeur d'endormissement de 2 secondes est utilisée.
  (Pour info, sur le projet CCIH il y a 18 fichiers référencés et il faut positionner ce paramètre à 15 secondes)
- Suppression de la mise en région du module. Seul l'appel de la fonction ReceptionFaire des modules RFR
  applicatifs est conservé en région.
  (Pour info, sur le projet CCIH où il y a 18 fichiers référencés, chaque traitement d'un fichier de référence
   bloquait la voie pendant une période de 6 à 9 secondes !!! (CPU = Pentium 166 MHz) )

Version 7.0.0 - TCV 04/07/2008
- The REFERENC module was not managing the file checking properly. It was checking the temporary directory to find
  if the parameter file on PCS server is already downloaded to the lane PC local directory. Since parameter files
  are deleted from the temporary directory after the file is properly downloaded the REFERENC module was detecting
  the new file every time after the network recovery or application start-up. Also, if any new file was generated
  the REFERENC module was detected the new file download for all files listed in the reference file. This bug has
  been fixed. Note that the REFERENC module will not properly detect that the file is already downloaded if new version
  of extension reference libraries is not used with the new version of the REFERENC module. The extension libraries
  should fill comment function parameter to signalize that the parameter file already exists in the local directory.
- After the file download the REFERENC module couldn’t move or delete the file from the local directory because the
  parameter file copied from the server was read only. The new version of the REFERENC module removes the read only
  attribute from the file copied into the local directory.
- If two parameter lists have similar ID and if one file ID is substring of another ID the REFERENC module would
  crash if the server generates the same version of these two lists. The ComparerId function in the fic_ref.c file
  has been updated to fix this problem. This problem is generated in the version 2.0.4 of the REFERENC module. It is
  probably because the module was updated to support UNIX file names. The previous comparison function has been returned
  to fix this problem but now it can make problem when the communication server is connected to UNIX database server.


