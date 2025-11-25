
12/10/99 BPH : csr_msg.dll csr_msg.h

* ajout de traces par "printf" sur erreur en lecture (MSG_Read) ou écriture (MSG_Write)
  pas de changement de l'interface



13/10/99 BPH : csr_msg.dll

* modif du type RAW pour faire du codage en hexa ASCII sur 2 caractères par octet, ex. 216 -> "8E"
 REM : - tous les octets sont accolés SANS sépararteur dans le buffer
       - la conversion du type RAW en hexa codée ASCII multiplie par
         deux la taille finale du buffer ASCII par rapport au nombre
         d'octets du tableau initial dans la structure C
       - du fait, le DWORD contenant la taille du RAW est multipliée par deux



29/10/99 BPH : csr_msg.dll

* modif de la lecture de DWORD et LONG en utilisant sscanf pour autoriser les caractères "espaces".



Version 2.1.0 - BPH 19/12/00

* Ajout d'un type FixRaw qui reprend le principe du type Raw mais avec une taille figée et prédéfinie
* Ajout des fontions de Dump MSG_Dump_Start et Stop. Décortique un message de facon humaine lisible lors d'un MSG_WRITE
  Le choix du flux de sortie est laissé à l'utilisateur (console, fichier...)
* Ajout de la fonction MSG_Duplicate qui fait une copie "profonde" d'un message. Ceci inclu une réallocation des structures
  dynamiques liées aux types LIST, INCLUDE et éventuellement CUSTOM
* Ajout de la fonction MSG_Get_Name pour récupérer le nom ASCII d'un type de message



Version 2.2.0 - ECSAT 12/04/01

For the DUTRA project it was necessary to convert the data into the hexadecimal format.
The new MSG_FIELD_LIST_HEX and MSG_FIELD_HEXBE data types have been added to enable this conversion.
The MSG_FIELD_LIST_HEX data type is used to write the number of list items in the hexadecimal format.
The MSG_FIELD_HEXBE data type is used to convert data in the "big endian" format.
(Then we discovered that we can use the MSG_FIELD_FIXRAW data type for that conversion ... 
so that the MSG_FIELD_HEXBE type in no longer used in the DUTRA project).
To add these two data types it was necessary to change the library header file and all the functions 
that manage data conversion (MSG_Is_Fixed_Size, MSG_Duplicate_Recurse, MSG_Dump, MSG_Delete_Recurse, 
MSG_Read_Recurse and MSG_Write_Recurse).

Version 2.3.0 - BPH 29/11/01

* Ajout de traces dans le Dump des messages en cas d'erreur
* Ajout d'un nouveau define MSG_FIELD_SET en remplacement de MSG_OFFSET qui simplifie l'écriture des DLL de messages
   Voir les exemples de DLL de msg sous PVCS pour reporter les modifications.
  (MSG_OFFSET & MSG_DECLARE_REF & MSG_INIT_REF deviennent inutiles et obsoletes mais sont conservés pour la compatibilité)
* Utilisation de LIST 2.0.0

Version 2.4.0 - BPH 12/04/02
* Remplacement des malloc par des heapalloc pour amaeliorer les performances
* ajout d'une fonction Msg_Duplicate_List qui permet de dupliquer un message creer par Msg_New_List (message inserable dans une liste)