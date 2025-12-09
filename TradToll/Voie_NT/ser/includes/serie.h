/*------   (v) 1996 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SERIE
* FICHIER: SERIE.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Prototypage des fonctions publiques de la librairie SERIE
* et definition des objets publiques
* --------------------------------------------------------------------
* DESCRIPTION: Fichier d interface
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log : $
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef SER_H
#define SER_H

/*--------------- INCLUDES: ---------------*/
#include <windows.h>

#ifdef SER_DEF
#  include <public.h>
#else
#  include <export.h>
#endif

/*--------------- DEFINES: ----------------*/

#define SER_NB_PORTS_MAX 32
#define SER_REG_KEYn "SER"
#define SER_REG_KEYn_COM_HS "COM%d_HS" // booleen 0 ou 1

/* definition booleen propre au module serie */
typedef enum
{
   SER_FALSE,
   SER_TRUE
}
Ser_enum_booleen_t;

/* definition des types de tampons possibles pour chaque port de la carte */
typedef enum
{
   SER_TAMPON_RECEPTION,
   SER_TAMPON_EMISSION
}
Ser_enum_type_tampon_t;

/* definition des valeurs possibles du type de demande d information tampon */
typedef enum
{
   SER_INFO_PLACE_LIBRE,
   SER_INFO_PLACE_OCCUPEE
}
Ser_enum_type_info_tampon_t;

/* definition des comptes rendus renvoyes par les fonctions publiques */
typedef enum
{
   SER_CPT_OK = 1,
   SER_CPT_NOK,
   SER_CPT_PORT_DEJA_OUVERT,
   SER_CPT_NUM_PORT_INCORRECT,
   SER_CPT_PARAMETRE_INCORRECT,
   SER_CPT_PORT_INEXISTANT,
   SER_CPT_PORT_NON_OUVERT,
   SER_CPT_DONNEES_ABSENTES,
   SER_CPT_ECRIT_INCOMPLET,
   SER_CPT_DONNEES_INCOMPLETES,
   SER_CPT_FONCTION_NON_IMPLEMENTEE
}
Ser_enum_cpt_erreur_t;

/*--------------- FUNCTIONS: ---------------*/

EXPORT Ser_enum_cpt_erreur_t WINAPI SerOuvrePort (short int numero_port,
                                                  unsigned char *mode_command_line) ;
EXPORT Ser_enum_cpt_erreur_t WINAPI SerFermePort( short int numero_port);

EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitCaractere (short int numero_port,
                                                     char *car_lu,
                                                     unsigned long delai);
EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitCaractereMs (short int numero_port,
                                                     char *car_lu,
                                                     unsigned long delaiMs);
EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitMessage (short int numero_port,
                                                   char *message,
                                                   unsigned long longueur,
                                                   unsigned long *nbre_car_lus,
                                                   unsigned long delai);
EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitMessageMs (short int numero_port,
                                                   char *message,
                                                   unsigned long longueur,
                                                   unsigned long *nbre_car_lus,
                                                   unsigned long delaiMs);
EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitMessageComplet(short int numero_port,
                                                         char *message,
                                                         unsigned long longueur,
                                                         unsigned long *lus,
                                                         unsigned long delai);
EXPORT Ser_enum_cpt_erreur_t WINAPI SerLitMessageCompletMs(short int numero_port,
                                                         char *message,
                                                         unsigned long longueur,
                                                         unsigned long *lus,
                                                         unsigned long delaiMs);

EXPORT Ser_enum_cpt_erreur_t WINAPI SerEcritCaractere (short int numero_port,
                                                       char carac);
EXPORT Ser_enum_cpt_erreur_t WINAPI SerEcritMessage(short int numero_port,
                                                    char *message,
                                                    unsigned long longueur);

EXPORT Ser_enum_cpt_erreur_t WINAPI SerRazTampon (short int numero_port,  
                                                  Ser_enum_type_tampon_t type_tampon);

EXPORT Ser_enum_booleen_t WINAPI SerDonneDSR (short int numero_port);

EXPORT Ser_enum_booleen_t WINAPI SerDonneCTS (short int numero_port);

EXPORT Ser_enum_cpt_erreur_t WINAPI SerEmetBreak (short int numero_port, 
                                                  short int duree_break);

/* nouvelles fonctions pour compatibilite avec le module interface protocole MELODHY */
EXPORT Ser_enum_booleen_t WINAPI SerReset (void);
EXPORT Ser_enum_booleen_t WINAPI SerIsPresentChar (short int numero_port);
EXPORT Ser_enum_cpt_erreur_t WINAPI SerConfig (short int numero_port, short int device);

#undef PUBLIC
#undef I
#undef INIT
#endif




