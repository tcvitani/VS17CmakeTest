Il faut avoir sur le disque c:
- un repertoire 'tft', qui n'est pas modifi‚ par l'applicatif

Il faut avoir quelque part:
- des fichiers de reference
- des fichiers de tft

Procedure de test:
1) - mettre l'executable en cold start, en modifiant RC_INIT.c
   - accepter la demande de service de fichier
   Le process est OK.
2) - indiquer un nouveau fichier de reference, en donnant le path exact
        de l'endroit ou il se trouve
   Le process demande a FTP un transfert de fichier
3) - Effectuer le transfert en respectant les path; le fichier est alors
     copi‚ de son emplacement origine vers celui mentionn‚
     Le process va determiner seul le travail … faire, et envoyer des messages
     demander des transferts FTP, ...
4) - effectuer les transferts FTP comme en 3
   Chaque fichier est verifi‚ et un message est emis
   Des que tous les fichiers ont ‚t‚ recus, les actions de r‚ception sont d‚clenches

5) Si une action attends un reveil, Horodate le fournit.
6) Si une action attends un message applicatif, vous devez le fournir avec 
   le simulateur.
7) Vous pouvez aussi simuler le changement de TFT par l'op‚rateur



