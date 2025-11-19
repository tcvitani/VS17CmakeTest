Startup n'est plus dependant du noyau.

Version 1.0.2 AFX le 17/11/99
- Le fichier de traces et d'erreurs s'appelle desormais "_lwd.trc"
- La cle "Dynamic" est cree si elle n'existe pas.

Version 1.0.3 GFE le 20/01/2000
- Remplacement du TerminateProcess() qui pose le probleme lors de l'arret des process
  de ne pas en informer les DLL attachées  par le SafeTerminateProcess() 
  de A.TUCKER/http:\\wwww.wdj.com\archive\1007\feature.html qui fait tout bien.
- Correction concernant la liberation du handle du process qui provoque l'arret
  de tous les autres (WatchApplication()).

Version 1.0.4 NBL 04/07/2002
Ajout de la possibilité de définir la classe de priorité des processus lancés par startup.
Cette priorité se définie en ajoutant une valeur REG_DWORD nommée "priority_class" dans la
clé [HKLM\SOFTWARE\CSROUTE\LaneController\Config\Startup\NomDuProcess]. Cette valeur est
optionnelle. Elle peut valoir :
   32   (20h) pour une priorité "NORMAL", (valeur utilisée par défaut).
   64   (40h) pour une priorité "IDLE",
   128  (80h) pour une priorité "HIGH",
   256 (100h) pour une priorité "REALTIME".