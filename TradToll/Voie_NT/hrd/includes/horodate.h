/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL HORODATE
* FICHIER: HORODATE.H
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION: HORODATE est chargee de gerer les demandes de reveil et
*              les conversions de date et d'heure sous differents formats.
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Hrd/Includes/HORODATE.H_v  $
 * 
 *    Rev 1.2   Jan 20 1999 14:15:38   bph
 *  
* 
*    Rev 1.1   02 Oct 1998 11:08:52   bph
*  
* 
*    Rev 1.14   03 Jun 1998 10:38:42   DPI
* 
*
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef HRD_H
#define HRD_H

/*--------------- INCLUDES: ---------------*/


#ifdef HRD_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*--------------- CONSTANTES: ---------------*/

#define HRD_ANNEE_BASE            1980

/**/
/*--------------- TYPES: ---------------*/
/* formats de date et d'heure que HORODATE sait reconnaitre */
typedef enum
{
    PREMIER_FORMAT = 0,
        
        FORMAT_HEX = PREMIER_FORMAT,
        FORMAT_BCD,
        FORMAT_CHAINE_CARAC,
        
        BUTEE_FORMAT,
        DERNIER_FORMAT = BUTEE_FORMAT - 1,
        NB_FORMAT = BUTEE_FORMAT - PREMIER_FORMAT
}
enum_hrd_format_date_heure;

/*presentation de la chaine de caracteres date sous differentes formes*/
typedef enum
{
    PREMIERE_PRESENTATION = 0,
        
        JMA = PREMIERE_PRESENTATION,      /*jj/mm/aa*/
        JAM,                              /*jj/aa/mm*/
        AMJ,                              /*aa/mm/jj*/
        AJM,                              /*aa/jj/mm*/
        MAJ,                              /*mm/aa/jj*/
        MJA,                              /*mm/jj/aa*/
        
        BUTEE_PRESENTATION,
        DERNIERE_PRESENTATION = BUTEE_PRESENTATION - 1,
        NB_PRESENTATION = BUTEE_PRESENTATION - PREMIERE_PRESENTATION
}
enum_hrd_presentation_date;


/*date au format hexadecimal en entree de la fonction de conversion*/
typedef struct
{
    unsigned short annee;
    unsigned char mois;
    unsigned char jour;
}
struct_hrd_date_entree_hex;

/*date au format hexadecimal en sortie de la fonction de conversion*/
typedef struct
{
    unsigned short annee;
    unsigned char mois;
    unsigned char jour;
    unsigned short julien;
    unsigned char num_jour_semaine;
}
struct_hrd_date_sortie_hex;

/*date au format BCD en entree de la fonction de conversion*/
typedef struct
{
    unsigned char annee[2];
    unsigned char mois;
    unsigned char jour;
}
struct_hrd_date_entree_bcd;

/*date au format hexadecimal en sortie de la fonction de conversion*/
typedef struct
{
    unsigned char annee[2];
    unsigned char mois;
    unsigned char jour;
    unsigned char julien[2];
    unsigned char num_jour_semaine;
}
struct_hrd_date_sortie_bcd;

/*pour la fonction de conversion de dates, le format de la date est precise*/
/*en entree et le champ correspondant est rempli*/
typedef struct
{
    enum_hrd_format_date_heure format;
    struct_hrd_date_entree_hex hex;
    struct_hrd_date_entree_bcd bcd;
}
struct_hrd_date_entree;

/*la fonction de conversion de dates donne en sortie*/
typedef struct
{
    struct_hrd_date_sortie_hex hex;
    struct_hrd_date_sortie_bcd bcd;
    unsigned char chaine_carac[11];
}
struct_hrd_date_sortie;

/*heure au format hexadecimal en sortie de la fonction de conversion*/
typedef struct
{
    unsigned char heure;
    unsigned char minute;
    unsigned char seconde;
}
struct_hrd_heure_hex;

/*heure au format BCD en sortie de la fonction de conversion*/
typedef struct
{
    unsigned char heure;
    unsigned char minute;
    unsigned char seconde;
}
struct_hrd_heure_bcd;

/*pour la fonction de conversion de d'heure, le format de l'heure est precise*/
/*en entree et le champ correspondant est rempli*/
typedef struct
{
    enum_hrd_format_date_heure format;
    struct_hrd_heure_hex hex;
    struct_hrd_heure_bcd bcd;
}
struct_hrd_heure_entree;

/*la fonction de conversion de l'heure donne en sortie*/
typedef struct
{
    struct_hrd_heure_hex hex;
    struct_hrd_heure_bcd bcd;
    unsigned char chaine_carac[9];
}
struct_hrd_heure_sortie;

/**/
/*----------Compte rendu des differentes fonctions de HORODATE----------*/
typedef enum
{
    PREMIER_COMPTE_RENDU = 0,
        
        HRD_LANCE_OK = PREMIER_COMPTE_RENDU,
        HRD_LANCE_NOK,
        HRD_CONFIGURE_OK,
        HRD_CONFIGURE_NOK,
        HRD_ARRET_OK,
        HRD_ARRET_NOK,
        
        BUTEE_COMPTE_RENDU,
        DERNIER_COMPTE_RENDU = BUTEE_COMPTE_RENDU - 1,
        NB_COMPTE_RENDU = PREMIER_COMPTE_RENDU - DERNIER_COMPTE_RENDU
}
enum_hrd_compte_rendu;

/*Type de trace*/
typedef enum
{
    PREMIER_TYPE_TRACE = 0,
        
        HRD_TRACE = PREMIER_TYPE_TRACE,
        HRD_NO_TRACE,
        
        BUTEE_TYPE_TRACE,
        DERNIER_TYPE_TRACE = BUTEE_TYPE_TRACE - 1,
        NB_TYPE_TRACE = PREMIER_TYPE_TRACE - DERNIER_TYPE_TRACE
}
enum_hrd_type_trace;

/**/
/*------------------- DEFINITION DES MESSAGES ------------------------------*/

/*identificateurs de services*/
typedef enum
{
    PREMIER_SERVICE = 0,
        
        M_HRD_ARRET = PREMIER_SERVICE,
        M_HRD_GESTION_ALARME,
        M_HRD_EVOL_HEURE,
        M_HRD_GESTION_DATE_HEURE,
        
        BUTEE_SERVICE,
        DERNIER_SERVICE = BUTEE_SERVICE - 1,
        NB_HRD_SERVICE = BUTEE_SERVICE - PREMIER_SERVICE
}
enum_hrd_service;

typedef enum
{
    
    PREMIER_HRD_TYPE = 0,
        
        /*types de message du service M_HRD_ARRET*/
        HRD_DEMANDE = PREMIER_HRD_TYPE,
        HRD_ARRET_ACQ,
        
        /*types de message du service M_HRD_GESTION_ALARME*/
        HRD_ARME_PONCT,
        HRD_ARME_PONCT_ACQ,
        HRD_ARME_PONCT_NACQ,
        
        HRD_ARME_HOR,
        HRD_ARME_HOR_ACQ,
        HRD_ARME_HOR_NACQ,
        
        HRD_ARME_JOUR,
        HRD_ARME_JOUR_ACQ,
        HRD_ARME_JOUR_NACQ,
        
        HRD_ARME_REGUL_ACQ,
        HRD_ARME_REGUL_NACQ,
        HRD_ARME_REGUL,
        
        HRD_ALARME,
        
        HRD_REVEIL_FIGEANT,
        
        HRD_DESARME_PONCT,
        HRD_DESARME_PONCT_ACQ,
        HRD_DESARME_PONCT_NACQ,
        
        HRD_DESARME_HOR,
        HRD_DESARME_HOR_ACQ,
        HRD_DESARME_HOR_NACQ,
        
        HRD_DESARME_JOUR,
        HRD_DESARME_JOUR_ACQ,
        HRD_DESARME_JOUR_NACQ,
        
        HRD_DESARME_REGUL,
        HRD_DESARME_REGUL_ACQ,
        HRD_DESARME_REGUL_NACQ,
        
        
        /*types de message du service M_HRD_EVOL_HEURE*/
        HRD_EVOL_ARRET,
        HRD_EVOL_ARRET_ACQ,
        HRD_EVOL_ARRET_NACQ,
        
        HRD_EVOL_DEPART,
        HRD_EVOL_DEPART_ACQ,
        HRD_EVOL_DEPART_NACQ,
        
        /*types de message du service M_HRD_GESTION_HEURE*/
        HRD_NOUVELLE_DATE_HEURE,
        HRD_NOUVELLE_DATE_HEURE_ACQ,
        HRD_NOUVELLE_DATE_HEURE_NACQ,
        HRD_NOUVELLE_DATE_HEURE_PB,
        
        BUTEE_HRD_TYPE,
        DERNIER_HRD_TYPE = BUTEE_HRD_TYPE - 1,
        NB_HRD_TYPE = BUTEE_HRD_TYPE - PREMIER_HRD_TYPE
}
enum_hrd_type;

typedef enum
{
    HRD_NOK,
        HRD_OK
}
enum_hrd_retour;

typedef enum
{
    // requested hour is elapsed
    HRD_DATE_HEURE_DEPASSEE,

        // no available timer
        HRD_PAS_DE_REVEIL_DISPONIBLE,
        
        // operation was successful
        HRD_REVEIL_OK,

        // hrd service is unavailable
        HRD_UNAVAILABLE,

        // Other error
        HRD_UNITITIALIZED
}
enum_hrd_reveil;

typedef enum
{
    HRD_LUN = 1,
        HRD_MAR,
        HRD_MER,
        HRD_JEU,
        HRD_VEN,
        HRD_SAM,
        HRD_DIM,
}
enum_hrd_jour_semaine;

typedef enum
{
    HRD_TEMPS_REPRISE_NOK,
        HRD_TEMPS_SUSPENDU,
        HRD_TEMPS_DEJA_SUSPENDU,
        HRD_TEMPS_EN_EVOLUTION,
        HRD_TEMPS_DEJA_EN_EVOLUTION
}
enum_hrd_evolution_temps;

/*------------------- FIN DEFINITION DES MESSAGES -------------------------*/

/**/
/*---------------------- Structure des messages ---------------------------*/
/*Structure des messages echanges entre l'application et HRD*/
/*Entete des messages*/
typedef struct
{
    struct_neutre neutre;
    enum_hrd_service  service;
    enum_hrd_type type_message;
}
struct_hrd_entete;

typedef short int t_hrd_num_reveil;

typedef struct
{
    struct_hrd_date_entree_hex date;
    struct_hrd_heure_hex  temps;
    long int periode;    /*La periode est exprimee en secondes*/
    t_hrd_num_reveil num_alarme;
}
struct_hrd_gestion_alarme;

typedef struct
{
    struct_hrd_date_entree_hex date;
    struct_hrd_heure_hex  temps;
}
struct_hrd_gestion_date_heure;

typedef struct
{
    struct_hrd_entete entete;
    union
    {
        /*Pour le service M_HRD_GESTION_ALARME*/
        struct_hrd_gestion_alarme msg_gestion_alarme;
        /*Pour le service M_HRD_GESTION_DATE_HEURE*/
        struct_hrd_gestion_date_heure msg_gestion_date_heure;
    }u;
    
}
struct_hrd_message;

/*structure regroupant les parametres necessaires au lancement des taches*/
typedef struct
{
    noyau_priorite_tache priorite_init_max;
    noyau_priorite_tache priorite_max;
    noyau_pool_id num_pool;
    short int nb_alarme_max; /*nombre maximum d'alarmes que HRD peut gerer*/
}
struct_hrd_config;

/**/
/*--------------- FUNCTIONS: ---------------*/

/* Envoi une fois un message d'alarme dans bal_reponse selon date/heure*/
EXPORT enum_hrd_reveil WINAPI HRDArmeReveilPonctuel (struct_hrd_date_entree_hex date,
                                                     struct_hrd_heure_hex heure,
                                                     noyau_bal_id bal_reponse,
                                                     t_hrd_num_reveil * num_reveil);

/* Envoi regulierement un message d'alarme dans bal_reponse … partir de date/heure */
EXPORT enum_hrd_reveil WINAPI HRDArmeReveilRegulier (struct_hrd_date_entree_hex date,
                                                     struct_hrd_heure_hex heure,
                                                     long int periode_en_secondes,
                                                     noyau_bal_id bal_reponse,
                                                     t_hrd_num_reveil * num_reveil);

/* Envoi … chaque changement d'heure un message d'alarme dans bal_reponse */
EXPORT enum_hrd_reveil WINAPI HRDArmeReveilHoraire (noyau_bal_id bal_reponse, t_hrd_num_reveil * num_reveil);

/* Envoi … chaque changement de jour un message d'alarme dans bal_reponse */
EXPORT enum_hrd_reveil WINAPI HRDArmeReveilJournalier (noyau_bal_id bal_reponse, t_hrd_num_reveil * num_reveil);

/* Envoi une fois un message d'alarme dans bal_reponse selon date/heure*/
EXPORT enum_hrd_reveil WINAPI HRDArmeReveilPonctuelFigeant (struct_hrd_date_entree_hex date,
                                                            struct_hrd_heure_hex heure,
                                                            noyau_bal_id bal_reponse,
                                                            t_hrd_num_reveil * num_reveil);

/* Envoi regulierement un message d'alarme dans bal_reponse … partir de date/heure */
EXPORT enum_hrd_reveil WINAPI HRDArmeReveilRegulierFigeant (struct_hrd_date_entree_hex date,
                                                            struct_hrd_heure_hex heure,
                                                            long int periode_en_secondes,
                                                            noyau_bal_id bal_reponse,
                                                            t_hrd_num_reveil * num_reveil);

/* Envoi … chaque changement d'heure un message d'alarme dans bal_reponse */
EXPORT enum_hrd_reveil WINAPI HRDArmeReveilHoraireFigeant (noyau_bal_id bal_reponse, t_hrd_num_reveil * num_reveil);

/* Envoi … chaque changement de jour un message d'alarme dans bal_reponse */
EXPORT enum_hrd_reveil WINAPI HRDArmeReveilJournalierFigeant (noyau_bal_id bal_reponse, t_hrd_num_reveil * num_reveil);

/* Annule un reveil (de tous types) */
EXPORT enum_hrd_retour WINAPI HRDAnnuleReveil (t_hrd_num_reveil num_reveil);

/* Suspend le cours du temps (incr‚mente compteur) */
EXPORT enum_hrd_evolution_temps WINAPI HRDSuspendEvolutionDuTemps (noyau_bal_id bal_demandeur);

/* Reprend le cours du temps (d‚cr‚mente compteur),
doit ˆtre le mˆme demandeur que celui qui l'a suspendu */
EXPORT enum_hrd_evolution_temps WINAPI HRDReprendEvolutionDuTemps (noyau_bal_id bal_demandeur);

/* Etat d'evolution du temps (test … z‚ro du compteur) */
EXPORT enum_hrd_evolution_temps WINAPI HRDDonneEtatDuTemps (void);

/* Change uniquement la date du PC (mˆme heure) */
EXPORT enum_hrd_retour WINAPI HRDChangeDate (struct_hrd_date_entree_hex nouvelle_date);

/* Change uniquement l'heure du PC (mˆme date) */
EXPORT enum_hrd_retour WINAPI HRDChangeHeure (struct_hrd_heure_hex nouvelle_heure);

/*Retourne la date courante au format hexadecimal*/
EXPORT struct_hrd_date_entree_hex WINAPI HRDDonneDate(void);

/*Retourne la date courante au format jour julien*/
EXPORT unsigned short WINAPI HRDDonneJulien(void);

/*Retourne l'heure courante au format hexadecimal*/
EXPORT struct_hrd_heure_hex WINAPI HRDDonneHeure(void);

/*Conversion d'une date dans un format donne aux formats definis*/
/* dans enum_format_date_heure.La chaine de caracteres date peut etre*/
/*sous differentes formes definies dans enum_hrd_presentation_date*/
EXPORT void WINAPI HRDConversionDate   (struct_hrd_date_entree /*date_entree*/,
                                        enum_hrd_presentation_date /*presentation*/,
                                        struct_hrd_date_sortie * /*date_sortie*/);
EXPORT unsigned short WINAPI HRDConversionDateJulien( struct_hrd_date_entree_hex *date);


/*Conversion d'une heure dans un format donne aux formats definis*/
/* dans enum_format_date_heure*/
EXPORT void WINAPI HRDConversionHeure  (struct_hrd_heure_entree /*heure_entree*/,
                                        struct_hrd_heure_sortie * /*heure_sortie*/);

/*Operation sur les dates*/

/*Donne le nombre de secondes ecoulees deuis 01/01/1980 … 00:00*/
EXPORT unsigned long int WINAPI HRDSecondesDepuis80 (struct_hrd_date_entree_hex date,
                                                     struct_hrd_heure_hex temps);

/*Conversion de secondes(depuis 01/01/1980 a 00:00:00) en date et heure*/
EXPORT void WINAPI HRDConvSecondesEnDateHeure(unsigned long int nb_secondes,
                                              struct_hrd_date_entree_hex *date,
                                              struct_hrd_heure_hex *temps);

/*Ajoute le nombre de secondes*/
EXPORT void WINAPI HRDAjouteDateHeure (struct_hrd_date_entree_hex /*date_entree*/,
                                       struct_hrd_heure_hex /*heure_entree*/,
                                       unsigned long int /*nb_secondes*/,
                                       struct_hrd_date_entree_hex * /*date_sortie*/,
                                       struct_hrd_heure_hex * /*heure_sortie*/);

/*Retranche le nombre de secondes*/
EXPORT void WINAPI HRDRetrancheDateHeure (struct_hrd_date_entree_hex /*date_entree*/,
                                          struct_hrd_heure_hex /*heure_entree*/,
                                          unsigned long int /*nb_secondes*/,
                                          struct_hrd_date_entree_hex * /*date_sortie*/,
                                          struct_hrd_heure_hex * /*heure_sortie*/);

                                          /*Compare deux dates.Retourne -1 si date_heure1 < date_heure2
                                          0 si date_heure1 = date_heure2
1 si date_heure1 > date_heure2*/
EXPORT char WINAPI HRDCompareDatesHeures
(struct_hrd_date_entree_hex /*date1*/,
 struct_hrd_heure_hex /*heure1*/,
 struct_hrd_date_entree_hex /*date2*/,
 struct_hrd_heure_hex /*heure2*/);

 /*Test la coherence d'une date en tenant compte des annees bissextile
 Retourne 1 si date coherente
0 si incoherente*/
EXPORT enum_hrd_retour WINAPI HRDDateCoherence (struct_hrd_date_entree_hex /*date_entree*/);

/*Test la coherence d'une heure.Retourne 1 si heure coherente
0 si incoherente*/

EXPORT enum_hrd_retour WINAPI HRDHeureCoherence (struct_hrd_heure_hex /*heure_entree*/);

/*Initialisation et arret de HORODATE*/
EXPORT enum_hrd_compte_rendu WINAPI HRDLance(struct_hrd_config config, noyau_bal_id *bal_hrd);
EXPORT enum_hrd_compte_rendu WINAPI HRDArret(void);

/*Fonctions de debut et de fin de trace*/
EXPORT enum_hrd_type_trace WINAPI HRDDebutTrace (FILE *fic);
EXPORT enum_hrd_type_trace WINAPI HRDFinTrace (void);

EXPORT enum_hrd_jour_semaine WINAPI HRDNumJourSemaineHex
(struct_hrd_date_entree_hex *date_entree);
EXPORT unsigned short WINAPI HRDNumJourSemaineBcd
(struct_hrd_date_entree_bcd *date_entree);

EXPORT boolean WINAPI HRDConversionJulienDate( unsigned short julien,
                                              struct_hrd_date_entree_hex *date,
                                              int annee);
/* l'heure julien doit ˆtre renseign‚e si futur est interdit */
EXPORT boolean WINAPI HRDConversionJulienDateCourante( unsigned short julien,
                                                      struct_hrd_heure_hex *heure_julien,
                                                      struct_hrd_date_entree_hex *date,
                                                      boolean futur_interdit);

#undef PUBLIC
#undef I
#undef INIT
#endif
