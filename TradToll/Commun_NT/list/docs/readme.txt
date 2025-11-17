04/05/01 BPH v2.0.1
* mise à zero de la mémoire allouée par List_ItemNew()

05/04/01 BPH v2.0.0

* Ajout d'une section critique pour protéger la liste en accès multitaches

* Ajout des fonctions List_EnterCriticalSection et List_LeaveCriticalSection 
  pour protéger des manipulations successives d'elements d'une liste (atomicité)

* Modif des fonctions de création d'items qui doivent recevoir en parametre la liste à laquelle il sont destinés
  afin d'utiliser sa section critique 
  (permet d'assurer qu'item ne sera pas detruit pendant qu'on l'utilise en prenant au prealable la section critique de la liste)

* Modif des fonctions de recherche pour parcourir la liste depuis la tete vers la queue et inversement. 
  La fonction List_FindFirst à été remplacer par ces deux fonctions : List_FindFirstHead et List_FindFirstTail 
  qui commence la recherche par la tete ou la queue (la version antérieure commencait toujours par la tete). 
  Du fait la fonction List_FindPrevious a été ajoutée pour accompagner List_FindFirstTail en gardant le sens 
  de parcours de la queue vers la tete.