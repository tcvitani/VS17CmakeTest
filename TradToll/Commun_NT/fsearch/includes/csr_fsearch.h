/* --------------------------------------------------------------------
 * (C) 2003 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : FSEARCH
 * FILE       : CSR_FSEARCH.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Recherche de données dans une structure formatés
 * --------------------------------------------------------------------
 * DESCRIPTION: Cette librairie permet d'effectuer des recherches
 *              d'enregistrements dans des fichiers ou des blocs
 *              mémoire.
 *
 *              Le format attendu pour un fichier ou un bloc mémoire est :
 *                - texte ASCII : composés de lignes de caractères affichables,
 *                  séparées soit par un caractère " Retour Chariot " (ASCII 13),
 *                  soit par un caractère " Saut de ligne " (ASCII 10), soit par
 *                  les deux.
 *                - brut : à taille d'enregistrement fixe,
 *                - enregistrements préfixés par une taille. Le format attendu 
 *                  pour un fichier ou un bloc mémoire de ce type est le suivant:
 *
 *              +------------+------------+-------------------------------------+
 *              | Nom        | Taille     | Commentaires                        |
 *              |            | octets     |                                     |
 * +------------+------------+------------+-------------------------------------+
 * | Entête     | HeaderSize | 4          | Taille des données de l'entête      |
 * |(Facultatif)|            |            | Entier binaire poids faible en tête |
 * |            +------------+------------+-------------------------------------+
 * |            | HeaderData | HeaderSize | Données de l'entête                 |
 * +------------+------------+------------+-------------------------------------+
 * | Record 0   | RecordSize | 4          | Taille des données du record        |
 * |            |            |            | Entier binaire poids faible en tête |
 * |            +------------+------------+-------------------------------------+
 * |            | RecordData | RecordSize | Données du record                   |
 * +------------+------------+------------+-------------------------------------+
 * |                                                                            |
 * | ...........................                                                |
 * |                                                                            |
 * +------------+------------+------------+-------------------------------------+
 * | Record n   | RecordSize | 4          | Taille des données du record        |
 * |            |            |            | Entier binaire poids faible en tête |
 * |            +------------+------------+-------------------------------------+
 * |            | RecordData | RecordSize | Données du record                   |
 * +------------+------------+------------+-------------------------------------+
 * | EndMark    | EndMark    | 4          | Equivaut à un record de taille 0    |
 * |(Facultatif)|            |            | (les 4 octets valent 0)             |
 * +------------+------------+------------+-------------------------------------+
 *
 *              L'entête est facultatif, ainsi que le marqueur de fin.
 *              Lorsque les records sont de taille constante et triés par ordre 
 *              croissant sur la clé utilisée pour la recherche, la librairie est
 *              capable d'effectuer une recherche dichotomique, sinon, la recherche
 *              est linéraire.
 *              Dans tous les cas, la taille d'entête reste libre.
 *              La librairie gère trois type de données pour la clé :
 *                + ASCII : Il s'agit d'une comparaison texte (octet 0 = fin de
 *                        chaine). Il est possible (optionnellement) d'utiliser
 *                        un joker multiple ('*') et des jokers simples ('?') dans
 *                        les données du fichier et / ou la clé à rechercher.
 *                        La comparaison peut être "case sensitive" ou non.
 *                + BCD : Il s'agit d'une suite de digits codés à 2 par octets
 *                        (1 digit par quartet). Dans un octet, le premier digit
 *                        est codé dans le quartet de poids fort.
 *                        Il est possible (optionnellement) d'utiliser un joker
 *                        multiple (0x0A) et des jokers simples (0x0E) dans les
 *                        données du fichier et / ou la clé à rechercher.
 *                + BINAIRE : Il s'agit d'une suite brut d'octets. Il n'est pas
 *                        possible d'utiliser de jocker.
 *
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef FSEARCH_H
#define FSEARCH_H


#ifdef FSEARCH_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif


typedef enum 
{
    SEARCH_FILE_DWORD_SIZED_RECORD = 0x01000000,
    SEARCH_FILE_TEXT               = 0x02000000,
    SEARCH_FILE_RAW_ZERO           = 0x03000000,

    SEARCH_FILE_MASK               = 0xFF000000,
    SEARCH_FILE_NMASK              = 0x00FFFFFF,
}
    SEARCH_FILE_TYPE;

#define SEARCH_FILE_RAW(recordBytes)    ( SEARCH_FILE_RAW_ZERO | ( recordBytes ) )



   
//
// Structure de paramétrage de la recherche
//
typedef struct _SEARCH_PARAMS
{
    // La clé est de type ASCII :
    // Incompatible avec fBinarySearch et fBCDSearch
    unsigned int        fAsciiSearch        :1;

    // La clé est de type binaire brute :
    // Incompatible avec fAsciiSearch et fBCDSearch
    // Incompatible avec le type de fichier SEARCH_FILE_TEXT.
    unsigned int        fBinarySearch       :1;

    // La clé est de type BCD :
    // Incompatible avec fBinarySearch et fAsciiSearch
    // Incompatible avec le type de fichier SEARCH_FILE_TEXT.
    unsigned int        fBCDSearch          :1;

    // Les données sont dans un fichier.
    // La valeur szPath doit être renseignée.
    // Incompatible avec fFromMemory.
    unsigned int        fFromFile           :1;

    // Les données sont en mémoire.
    // Les valeurs pbBufferBytes et dwBufferBytes doivent être renseignées.
    // Incompatible avec fFromFile.
    unsigned int        fFromMemory         :1;

    // Les jockers de type '*' sont accepté en ASCII ou BCD
    // Imcompatible avec fBinarySearch.
    unsigned int        fMultiJokerAllowed  :1;

    // Les jockers de type '?' sont accepté en ASCII ou BCD
    // Imcompatible avec fBinarySearch.
    unsigned int        fSingleJokerAllowed :1;

    // Les enregistrements du fichiers sont triés par ordre croissant
    // selon la clé utilisée pour la recherche. Il ne doit pas y avoir
    // de doubon.
    // Lorsque cette option est combinée avec fFixedLength, la recherche
    // effectuée est dichotomique.
    // Obligatoire avec le type de fichier RAW.
    unsigned int        fSortedRecords      :1;

    // Le bloc possède un record d'entête qui doit être ignoré dans
    // la recherche.
    // Incompatible avec le type de fichier RAW.
    unsigned int        fHeaderPresent      :1;

    // Les comparaison tiennent compte de la casse des lettres
    unsigned int        fCaseSensitive      :1;

    // Permet d'optimiser la recherche dans le cas où la taille
    // des records est fixe.
    unsigned int        fFixedLength        :1;

    // Chemin du fichier dans lequel effectuer la recherche.
    // A définir uniquement si fFromFile est utilisé.
    char                szPath[MAX_PATH];

    // Pointeur sur le bloc en mémoire dans lequel effectuer
    // la recherche.
    // A définir uniquement si fFromMemory est utilisé.
    BYTE              * pbBufferBytes;

    // Taille du bloc en mémoire.
    // A définir uniquement si fFromMemory est utilisé.
    DWORD               dwBufferBytes;
}
    SEARCH_PARAMS;





/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI FSearchKeyEx( 
 *                      IN          DWORD               dwType,
 *                      IN          SEARCH_PARAMS     * psParams,
 *                      IN          BYTE              * pbSearchRecord,
 *                      IN          DWORD               dwKeyOffet,
 *                      IN          DWORD               dwKeyBytes,
 *                      IN OUT      DWORD             * pdwFound,
 *                      OUT         BYTE              * pbFound )
 * PARAMETERS: dwType         : Type du fichier, parmis les valeurs suivantes :
 *                                 SEARCH_FILE_DWORD_SIZED_RECORD : Fichier dont chaque
 *                                      record est préfixé par une taille des données codée
 *                                      sur un DWORD big endian.
 *                                 SEARCH_FILE_TEXT : Fichier texte dont chaque record est
 *                                      terminé par CR, LF ou CR+LF
 *                                 SEARCH_FILE_RAW(N) : Fichier brut dont chaque record est
 *                                      de taille unique N.
 *             psParams       : Pointe sur une structure contenant les paramètres de la recherche.
 *             pbSearchRecord : Pointe sur un buffer contenant le record à chercher
 *                              (la position de la clé de recherche doit être la même
 *                              dans ce buffer que dans les records à parcourir).
 *             dwKeyOffset    : Position de la clé dans le buffer pointé par pbSearchRecord et dans
 *                              les records à parcourir.
 *             dwKeyBytes     : Taille de la clé
 *             pdwFound       : En entrée, taille du buffer pointé par pbFound.
 *                              En sortie, en cas de succés de la recherche, taille du record trouvé.
 *                              En sortie, en cas d'échec de la recherche, 0.
 *             pbFound        : Pointe sur un buffer dans lequel sera stocké le record complet
 *                              si la recherche réuissi.
 * RETURN    : TRUE  : Aucune erreur n'est survenue pendant la recherche Cela ne veux
 *                     pas dire pour autant qu'un record a été trouve. Utiliser
 *                     (*pdwFound) pour savoir si un record a été trouvé ou non.
 *             FALSE : Une erreur a rendu la recherche impossible. GetLastError() permet
 *                     d'avoir un détail de cette erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectuer une recherche d'enregistrement dans un fichier ou
 *             dans un bloc mémoire.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI FSearchKeyEx( 
        IN          DWORD               dwType,
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI FSearchKeyPosition( 
 *                      IN          SEARCH_PARAMS     * psParams,
 *                      IN          BYTE              * pbSearchRecord,
 *                      IN          DWORD               dwKeyOffet,
 *                      IN          DWORD               dwKeyBytes,
 *                      IN OUT      DWORD             * pdwFound,
 *                      OUT         BYTE              * pbFound ,
 *                      IN OUT       DWORD             * pdwPosition)
 * PARAMETERS: psParams       : Pointe sur une structure contenant les paramètres de la recherche.
 *             pbSearchRecord : Pointe sur un buffer contenant le record à chercher
 *                              (la position de la clé de recherche doit être la même
 *                              dans ce buffer que dans les records à parcourir).
 *             dwKeyOffset    : Position de la clé dans le buffer pointé par pbSearchRecord et dans
 *                              les records à parcourir.
 *             dwKeyBytes     : Taille de la clé
 *             pdwFound       : En entrée, taille du buffer pointé par pbFound.
 *                              En sortie, en cas de succés de la recherche, taille du record trouvé.
 *                              En sortie, en cas d'échec de la recherche, 0.
 *             pbFound        : Pointe sur un buffer dans lequel sera stocké le record complet
 *                              si la recherche réuissi.
 *             pdwPosition    : en entrée position du curseur dans le buffer
 *                              en sortie, position réelle du curseur dans le buffer
 * RETURN    : TRUE  : Aucune erreur n'est survenue pendant la recherche Cela ne veux
 *                     pas dire pour autant qu'un record a été trouve. Utiliser
 *                     (*pdwFound) pour savoir si un record a été trouvé ou non.
 *             FALSE : Une erreur a rendu la recherche impossible. GetLastError() permet
 *                     d'avoir un détail de cette erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectuer une recherche d'enregistrement dans un fichier ou
 *             dans un bloc mémoire.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI FSearchKeyPosition( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffet,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound ,
		IN OUT      DWORD             * pdwPosition);

/*
 * --------------------------------------------------------------------
 * SYNTAX    : 
 * PARAMETERS: 
 * RETURN    : 
 * --------------------------------------------------------------------
 * ROLE      : Fonction conservée pour compatibilité ascendante.
 *             * Elle équivaut à un appel à  FSearchKeyEx dans lequel
 *             le paramètre dwType vaudrait SEARCH_FILE_DWORD_SIZED_RECORD.
 *             * Elle équivaut à un appel à  FSearchKeyPosition dans lequel
 *             le paramètre pdwPosition en entré vaut 0 et en sortie est 
 *             ignoré
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI FSearchKey( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffet,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound );



#endif  // FSEARCH_H
