Version 2.2.0 4/06/02 HMO : csr_lan.h / csr_pipe.dll / csr_sock.dll

    * Utilisation d'un buffer de 4096 octets


Version 2.1.0 13/04/01 CL+NHO+SBA : csr_sock.dll

   * Correction bug service mise-à-l'heure

   * Report évolutions service DOP->ESPION

   * Correction pb abonné unique au service de tranfert de fichiers



Version 2.0.0 08/09/00 BPH : csr_lan.h / csr_pipe.dll / csr_sock.dll

   * Adaptation de csr_lan.h aux specs de modules.h pour le dop

   * Gestion des BAL demandeurs invalides lors d'envois de messages de services
     (plus d'ExitBAD, mais fin du service pour ce demandeur)



Version 1.0.1 18/11/99 BPH : csr_pipe.dll/lib/reg

   * correction bug service horaire et ajout nouvelle clef de tolerance de difference de temps en
     en secondes _Max_Diff_Time



Version 1.0.0 29/10/99 BPH : csr_pipe.dll/lib

   * suppression des régions bloquantes sur déconnexion, remplacees par une section critique
     locale au module



Version 1.0.0 11/10/99 BPH :

   * modif de csr_pipe.dll/lib, service fichier illimité en jetons


