/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL HORODATE
* FICHIER: HRD_CONV.C
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION: Ce module permet de convertir des dates et des heures
*              aux formats hexadecimal et BCD en y ajoutant les
*              informations suivantes :
*                                     - le jour julien
*                                     - le numero du jour dans la semaine
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Hrd/SOURCES/HRD_CONV.C_v  $
 * 
 *    Rev 1.1   02 Oct 1998 11:09:02   bph
 *  
 * 
 *    Rev 1.16   03 Jun 1998 15:45:26   DPI
*
* --------------------------------------------------------------------
* $F_HEAD


/* #include Borland C */
#include <stdio.h>
#include <string.h>

/* #include Modulotheque */
#include <noyau.h>
#include <convert.h>
#include <horodate.h>
//#include <stdcsee.h>
#include "err.h"

/* #include interne */
#include <hrd_serv.h>

#include "memclass.h"

/*--------------- PRIVATE: --------------*/
PROTECTED void ConvEntreeBcdHex(struct_hrd_date_entree_bcd date_entree,
                              struct_hrd_date_entree_hex *date_sortie);

/*--------------- VARIABLES: --------------*/
/* tableau permettant de calculer le jour julien  */
/* a partir d'une date correcte.                  */
/* ATTENTION, le premier indice est bissextile    */
static unsigned short int julian_table[2][13] =
{
   {   0,   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335 },
   {   0,   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334 },
};

static unsigned long int annee_seconde[5] =
{
     1L * ( 0),                 /* ( 0 ) jours * 24 heures * 3600 secondes */
     1L * ( 366) * 24 * 3600,         /* 1 ann‚e bissextile */
     1L * ( 366 + 365) * 24 * 3600,   /* 2 ann‚es dont 1 ann‚e bissextile */
     1L * ( 366 + 2*365) * 24 * 3600, /* 3 ann‚es dont 1 ann‚e bissextile */
     1L * ( 366 + 3*365) * 24 * 3600, /* 4 ann‚es dont 1 ann‚e bissextile */
};



/*--------------------Constante--------------------------*/
/*Nombre de jour par mois*/
/* ATTENTION, l'annee est bissextile              */
PROTECTED unsigned long int nbr_jourmois[] =
/*      Jan   Fev   Mar   Avr   Mai   Jun   Jui   Aou   Sep   Oct  Nov   Dec */
{ 0,    31,   29,   31,   30,   31,   30,   31,   31,   30,   31,  30,   31};

/*Nombre de secondes par mois*/
/* ATTENTION, l'annee est bissextile              */
PROTECTED unsigned long int secondes_mois[] =
/*       Jan         Fev          Mar       Avr       Mai        Jun       Jui */
{0x00l, 0x28DE80l, 0x263B80l, 0x28DE80l, 0x278D00l, 0x28DE80l, 0x278D00l, 0x28DE80l,
/* Aou        Sep        Oct         Nov       Dec */
 0x28DE80l, 0x278D00l, 0x28DE80l, 0x278D00l, 0x28DE80l };

/*------------------------ TRAITEMENT DE LA DATE ------------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED unsigned short NumJourSemaineHex
* PARAMETRES: La date au format hexa.
* RETOUR: Le numero du jour dans la semaine.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Donne le numero du jour dans la semaine (en hexa).
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_hrd_jour_semaine HRDNumJourSemaineHex
                      (struct_hrd_date_entree_hex *date_entree)
{
   unsigned long             nbr_jours = 0;
   enum_hrd_jour_semaine     result = HRD_LUN;
   unsigned short            index;

   if( date_entree->annee <= HRD_ANNEE_BASE)
      ERR_ErreurFatale();

   /* calcul du nombre de jour entre ANNEE REF et ANNEE SAISIE -1 */
   /* on  prend l'annee 1979 comme base car le 1 janvier 1979 est un lundi */
   for (index = 1979; index < date_entree->annee; index++)
   {
     if (bissextile(index))
          nbr_jours = nbr_jours + 366;
     else
          nbr_jours = nbr_jours + 365;
   }

   /* ajout du calcul du nombre de jour entre 1ier MOIS et MOIS SAISIE -1 */
   for (index = 1 ; index < date_entree->mois; index++)
   {
     nbr_jours = nbr_jours + nbr_jourmois[index];

     /* Prise en compte de l'annee bissextile */
     if( ! (bissextile(date_entree->annee)) && ( index == 2) )
       nbr_jours --;
   }

   /* ajout du nombre de jour */
   nbr_jours = nbr_jours + date_entree->jour;

   /* l'indice du jour est obtenu en prenant le reste de la division */
   /*   --> nbre de jour MODULO 7                                    */
   result = (nbr_jours % 7);

   /* il faut tester car le 0 signifie DIMANCHE */
   if( result == 0)
      result = 7;

   return( result);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED unsigned short  NumJourSemaineBcd
* PARAMETRES: La date au format bcd.
* RETOUR: Le numero du jour dans la semaine.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Donne le numero du jour dans la semaine (en bcd).
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC unsigned short HRDNumJourSemaineBcd
                       (struct_hrd_date_entree_bcd *date_entree)
{
   unsigned short             num_jour_semaine;
   struct_hrd_date_entree_hex date_entree_hex;
   unsigned char              result[1];

   ConvEntreeBcdHex( *date_entree, &date_entree_hex);
   num_jour_semaine = HRDNumJourSemaineHex( &date_entree_hex);
   ConversionHexaEnBcd((long)num_jour_semaine, result ,1);

   return( result[0]);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvEntreeBcdHex
* PARAMETRES: La date au format BCD.
* RETOUR: La date au format hex .
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Conversion d'une date BCD au format hexa.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvEntreeBcdHex(struct_hrd_date_entree_bcd date_entree,
                              struct_hrd_date_entree_hex *date_sortie)

{
   date_sortie->annee = (unsigned short)
                       ( (ConversionBcdEnHexa(&date_entree.annee[0],1)*0x64)
                        + ConversionBcdEnHexa(&date_entree.annee[1],1)) ;
   date_sortie->mois = (unsigned char)ConversionBcdEnHexa(&date_entree.mois,1);
   date_sortie->jour = (unsigned char)ConversionBcdEnHexa(&date_entree.jour,1);

}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: HRDConversionDateJulien
* PARAMETRES: La date au format hexa.
* RETOUR: Le jour julien au format hexa.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Donne le jour julien (en hexa).
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC unsigned short HRDConversionDateJulien(struct_hrd_date_entree_hex *date)
{
   unsigned short       julien;
   int                  indice;

   if( bissextile( date->annee))
      indice = 0;
   else
      indice = 1;


   julien = date->jour + julian_table[ indice][date->mois];
   return( julien);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PUBLIC unsigned long int HRDSecondesDepuis80
*                               ( struct_hrd_date_entree_hex date,
*                                 struct_hrd_heure_hex temps)
*
* PARAMETRES: annee,mois,jour, heure minute et seconde.
* RETOUR: Le nombre de secondes ecoulees entre la date passee en
*         parametre et le 01/01/HRD_BASE_ANNEE … 0h00:00
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Calcul du nombre de secondes ecoulees de puis le 1/1/HRD_BASE_ANNEE
*        0h00:00
*       Cette fonction ne tient pas compte de l'heure GMT ni du
*       changement heure d'ete / heure d'hiver.
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC unsigned long int HRDSecondesDepuis80 (struct_hrd_date_entree_hex date,
                                               struct_hrd_heure_hex temps)

{
   unsigned short int julien;
   unsigned short int nb_paquets_4_annees;
   unsigned short int jours_ecoulees_dans_dernier_paquets;
   unsigned short int jours_ecoules_completement;

   julien = HRDConversionDateJulien( &date);
   nb_paquets_4_annees = (date.annee - HRD_ANNEE_BASE) / 4;
   jours_ecoulees_dans_dernier_paquets = (date.annee - HRD_ANNEE_BASE) % 4;
   jours_ecoules_completement = julien - 1;

   return ( annee_seconde[4] * nb_paquets_4_annees +
            annee_seconde[jours_ecoulees_dans_dernier_paquets] +
           (unsigned long int)jours_ecoules_completement * 24 * 3600 +
           (unsigned long int)temps.heure * 3600 +
           (unsigned long int)temps.minute * 60 +
           (unsigned long int)temps.seconde );
  /* ATTENTION, chaque expression '*' doit ˆtre cast‚ en UL, sinon
   * le calcul est fait en INT
   */
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED unsigned char * JourJulienBcd
* PARAMETRES: La date au format bcd.
* RETOUR: Le jour julien au format BCD.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Donne le jour julien (en BCD).
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED unsigned char *JourJulienBcd(struct_hrd_date_entree_bcd date_entree)
{
   static unsigned char p_julien[ 3];

   struct_hrd_date_entree_hex date_entree_hex;

   ConvEntreeBcdHex(date_entree,&date_entree_hex);
   ConversionHexaEnBcd( HRDConversionDateJulien( &date_entree_hex),p_julien,2);

   return p_julien;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED int bissextile(annee)
* PARAMETRES: annee
* RETOUR:     TRUE si annee est bissextile
* --------------------------------------------------------------------
* ROLE:      ANNEE BISSEXTILE : divisible par 4 et dans les annees
*            divisibles par 100 ne sont bissextile que celle qui sont
*            divisibles par 400.
* --------------------------------------------------------------------
*/
PROTECTED int bissextile(short int annee)
{
  if ( (annee%4 == 0) && ( (annee%0x64 != 0) || (annee%0x190 == 0) ) )
       return(1);
  else return(0);
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
* PUBLIC void HRDConvSecondesEnDateHeure(unsigned long int nb_secondes,
*                                        struct_hrd_date_entree_hex *date,
*                                        struct_hrd_heure_hex *temps)
*
* PARAMETRES: nombre de secondes,pointeurs sur date et heure.
* RETOUR: void
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Donne la date et l'heure correspondant au nombre de secondes
*       ecoulees depuis 01/01/80.
* --------------------------------------------------------------------
* $F_FCTN
*/


PUBLIC void HRDConvSecondesEnDateHeure(unsigned long int nb_secondes,
                                       struct_hrd_date_entree_hex *date,
                                       struct_hrd_heure_hex *temps)
{
   short int               nb_annee[4];
   short int               nb_jour_fevrier;
   short int               i;
   unsigned long int       reste;
   unsigned long int       nb_jour;
   unsigned long int       aux;
   boolean                 boucle;
   unsigned long int       nb_secondes_mois;


   for (i=0; i <= 3; i++)
   {
      nb_annee[i] = 0;
   }
   reste = nb_secondes;

   /*Calcul de l'annee*/
   i = 3;
   while(i >= 0)
   {
      nb_annee[i] = (short) (reste / annee_seconde[i+1]);
      reste = reste % annee_seconde[i+1];
      i -=1;
   }
   date->annee = HRD_ANNEE_BASE
                + 4 * nb_annee[3]
                + 3 * nb_annee[2]
                + 2 * nb_annee[1]
                + nb_annee[0];

   /*L'annee est-elle bissextile*/
   if (bissextile(date->annee) == 1)
   {
      nb_jour_fevrier = 29;
   }
   else
   {
      nb_jour_fevrier = 28;
   }

   /*Calcul du mois*/
   i = 1;
   boucle = TRUE;
   while ( boucle)
   {
      /* calcul du nombre de secondes dans le mois */
      nb_secondes_mois = secondes_mois[i];
      if ( (i == 2) && (nb_jour_fevrier == 28) )
      {
         /* L'annee n'est pas bissextile */
         nb_secondes_mois -= 86400l;
      }

      /* est-on dans un mois d'indice plus elev‚ ? */
      boucle = ( reste >= nb_secondes_mois );
      if( boucle)
      {
         reste -= nb_secondes_mois;
         i++;
      }
   }

   date->mois = (unsigned char) i;

   /*Calcul du jour*/

   nb_jour = (reste / 86400L) + 1;
   date->jour = (unsigned char) nb_jour;
   aux = 0;
   nb_jour -= 1;
   aux = nb_jour * 24L * 3600L;
   reste -= aux;

   /*Calcul de l'heure*/
   temps->heure = (unsigned char) (reste / 3600L);

   aux = 3600L * (unsigned long)temps->heure;

   if (reste > 0)
   {
      reste -= aux;

      /*Calcul des minutes*/
      temps->minute = (unsigned char) (reste / 60);

     if (reste > 0)
     {
        /*Calcul des secondes*/
         temps->seconde = (unsigned char) (reste - (60 * temps->minute));
      }
      else
      {
         temps->seconde = 0;
      }
   }
   else
   {
      temps->minute = 0;
      temps->seconde = 0;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvDateHexHex
* PARAMETRES: La date au format hexa.
* RETOUR: La date au format hex avec le jour julien et le numero du
*        jour dans la semaine.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Ajoute a la structure date le jour julien et le numero du
*      jour dans la semaine (en hexa).
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvDateHexHex(struct_hrd_date_entree_hex  date_entree,
                            struct_hrd_date_sortie_hex *date_sortie)

{

   date_sortie->annee = date_entree.annee;
   date_sortie->mois = date_entree.mois;
   date_sortie->jour = date_entree.jour;
   date_sortie->julien = HRDConversionDateJulien( &date_entree);
   date_sortie->num_jour_semaine = HRDNumJourSemaineHex( &date_entree);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvDateHexBcd
* PARAMETRES: La date au format hexa.
* RETOUR: La date au format bcd
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Conversion d'une date hexa en BCD .
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvDateHexBcd(struct_hrd_date_sortie_hex  date_entree,
                            struct_hrd_date_sortie_bcd *date_sortie)

{

   ConversionHexaEnBcd(date_entree.annee,date_sortie->annee,2);
   ConversionHexaEnBcd(date_entree.mois,&date_sortie->mois,1);
   ConversionHexaEnBcd(date_entree.jour,&date_sortie->jour,1);
   ConversionHexaEnBcd(date_entree.julien,date_sortie->julien,2);
   ConversionHexaEnBcd(date_entree.num_jour_semaine,
                       &date_sortie->num_jour_semaine,1);

}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvDateHexChaineCarac
* PARAMETRES: La date au format hexa, et la presentation desiree en sortie.
* RETOUR: La date en chaine de caracteres.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Transformer la structure date hexa en chaine de caracteres.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvDateHexChaineCarac(struct_hrd_date_entree_hex  date_entree,
                                     enum_hrd_presentation_date presentation,
                                     unsigned char *destination,
									 size_t destinationSize)

{

   char * separateur="/";

   switch (presentation)
      {
         case JMA :
            /*La date est au format JJ/MM/AAAA*/
			 sprintf_s((char *)destination, destinationSize, "%02u%s%02u%s%04u",
                    date_entree.jour,
                    separateur,
                    date_entree.mois,
                    separateur,
                    date_entree.annee);
            break;

         case JAM :
            /*La date est au format JJ/AAAA/MM*/
			 sprintf_s((char *)destination, destinationSize, "%02u%s%04u%s%02u",
                    date_entree.jour,
                    separateur,
                    date_entree.annee,
                    separateur,
                    date_entree.mois);
            break;

         case AMJ :
            /*La date est au format AAAA/MM/JJ*/
			 sprintf_s((char *)destination, destinationSize, "%04u%s%02u%s%02u",
                    date_entree.annee,
                    separateur,
                    date_entree.mois,
                    separateur,
                    date_entree.jour);
            break;

         case AJM :
            /*La date est au format AAAA/JJ/MM*/
			 sprintf_s((char *)destination, destinationSize, "%04u%s%02u%s%02u",
                    date_entree.annee,
                    separateur,
                    date_entree.jour,
                    separateur,
                    date_entree.mois);
            break;

         case MAJ :
            /*La date est au format MM/AAAA/JJ*/
			 sprintf_s((char *)destination, destinationSize, "%02u%s%04u%s%02u",
                    date_entree.mois,
                    separateur,
                    date_entree.annee,
                    separateur,
                    date_entree.jour);
            break;

         case MJA :
            /*La date est au format MM/JJ/AAAA*/
			 sprintf_s((char *)destination, destinationSize, "%02u%s%02u%s%04u",
                    date_entree.mois,
                    separateur,
                    date_entree.jour,
                    separateur,
                    date_entree.annee);
            break;

         default :
            /*Le format n'est pas connu*/
            break;
      }

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvDateBcdHex
* PARAMETRES: La date au format BCD.
* RETOUR: La date au format hex avec le jour julien et le numero du
*        jour dans la semaine.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Conversion de la date BCD au format hexa.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvDateBcdHex(struct_hrd_date_sortie_bcd  date_entree,
                            struct_hrd_date_sortie_hex *date_sortie)

{
   date_sortie->annee = (short) ( (unsigned short)(ConversionBcdEnHexa(&date_entree.annee[0],1)*0x64)
                          + ConversionBcdEnHexa(&date_entree.annee[1],1)) ;
   date_sortie->mois = (unsigned char) ConversionBcdEnHexa(&date_entree.mois,1);
   date_sortie->jour = (unsigned char) ConversionBcdEnHexa(&date_entree.jour,1);
   date_sortie->julien = ( (unsigned short) (ConversionBcdEnHexa(&date_entree.julien[0],1)*0x64)
                         + (short) ConversionBcdEnHexa(&date_entree.julien[1],1)) ;
   date_sortie->num_jour_semaine =
                        (unsigned char) ConversionBcdEnHexa(&date_entree.num_jour_semaine,1);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvDateBcdBcd
* PARAMETRES: La date au format bcd.
* RETOUR: La date au format bcd
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Ajout a la structure date le jour julien et le numero du jour
*       dans la semaine (en BCD).
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvDateBcdBcd(struct_hrd_date_entree_bcd  date_entree,
                            struct_hrd_date_sortie_bcd *date_sortie)

{
   memcpy(date_sortie->annee,&date_entree.annee,sizeof(date_entree.annee));
   date_sortie->mois = date_entree.mois;
   date_sortie->jour = date_entree.jour;
   memcpy(date_sortie->julien,
          JourJulienBcd(date_entree),
          sizeof(date_sortie->julien));
   date_sortie->num_jour_semaine = (unsigned char) HRDNumJourSemaineBcd( &date_entree);

}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvDateBcdChaineCarac
* PARAMETRES: La date au format bcd, et la presentation desiree en sortie.
* RETOUR: La date en chaine de caracteres.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Transformer la structure date BCD en chaine de caracteres.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void  ConvDateBcdChaineCarac(struct_hrd_date_entree_bcd  date_entree,
                                     enum_hrd_presentation_date presentation,
                                     unsigned char * date_sortie,
									 size_t date_sortieSize)

{
   struct_hrd_date_entree_hex date_hex;

   ConvEntreeBcdHex(date_entree,&date_hex);
   ConvDateHexChaineCarac(date_hex, presentation, date_sortie, date_sortieSize);

}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvDateHex
* PARAMETRES: La presentation de la date (desiree en sortie) et la date.
* RETOUR: La date sous differents formats.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE:Conversion d'une date hexadecimale aux formats definis
*      dans enum_format_date_heure.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvDateHex(struct_hrd_date_entree_hex date_entree,
                         enum_hrd_presentation_date presentation,
                         struct_hrd_date_sortie *date_sortie)
{
   ConvDateHexHex(date_entree,&date_sortie->hex);
   ConvDateHexBcd(date_sortie->hex,&date_sortie->bcd);
   ConvDateHexChaineCarac(date_entree, presentation, date_sortie->chaine_carac, sizeof(date_sortie->chaine_carac));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvDateBcd
* PARAMETRES: La presentation de la date (desiree en sortie) et la date.
* RETOUR: La date sous differents formats.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE:Conversion d'une date BCD aux formats definis
*      dans enum_format_date_heure.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvDateBcd(struct_hrd_date_entree_bcd date_entree,
                         enum_hrd_presentation_date presentation,
                         struct_hrd_date_sortie *date_sortie)
{
   ConvDateBcdBcd(date_entree,&date_sortie->bcd);
   ConvDateBcdHex(date_sortie->bcd,&date_sortie->hex);
   ConvDateBcdChaineCarac(date_entree,presentation,
                          date_sortie->chaine_carac,
						  sizeof(date_sortie->chaine_carac));

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void HRDConversionDate
* PARAMETRES: La presentation de la date (desiree en sortie) et la date.
* RETOUR: Compte-rendu d'arret.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE:Conversion d'une date dans un format donne aux formats definis
*      dans enum_format_date_heure.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void HRDConversionDate   (struct_hrd_date_entree date_entree,
                                 enum_hrd_presentation_date presentation,
                                 struct_hrd_date_sortie *date_sortie)
{

   switch (date_entree.format)
      {
         case FORMAT_HEX :
             /*La date en entree est au format hexadecimal*/
             ConvDateHex(date_entree.hex,presentation,date_sortie);
             break;

         case FORMAT_BCD :
            /*La date en entree est au format BCD*/
            ConvDateBcd(date_entree.bcd,presentation,date_sortie);
            break;

         default :

            break;
      }
}

/**/
/*-------------------------Traitement de l'heure ------------------------*/


/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvHeureHexHex
* PARAMETRES: L'heure au format hexa.
* RETOUR: L'heure au format hexa.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Simple recopie de l'heure.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvHeureHexHex(struct_hrd_heure_hex  heure_entree,
                             struct_hrd_heure_hex *heure_sortie)

{

   heure_sortie->heure = heure_entree.heure;
   heure_sortie->minute = heure_entree.minute;
   heure_sortie->seconde = heure_entree.seconde;

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvHeureHexBcd
* PARAMETRES: L'heure au format hexa.
* RETOUR: L'heure au format bcd
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Conversion de l'heure hexa en BCD.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvHeureHexBcd(struct_hrd_heure_hex  heure_entree,
                             struct_hrd_heure_bcd *heure_sortie)

{
   ConversionHexaEnBcd(heure_entree.heure,&heure_sortie->heure,1);
   ConversionHexaEnBcd(heure_entree.minute,&heure_sortie->minute,1);
   ConversionHexaEnBcd(heure_entree.seconde,&heure_sortie->seconde,1);

}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvHeureHexChaineCarac
* PARAMETRES: L'heure au format hexa.
* RETOUR: L'heure en chaine de caracteres.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Transformer la structure heure hexa en chaine de caracteres.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvHeureHexChaineCarac(struct_hrd_heure_hex heure_entree,
                                      unsigned char *destination,
									  size_t destinationSize)

{
   char * separateur=":";

   sprintf_s((char *)destination, destinationSize, "%02u%s%02u%s%02u",
           heure_entree.heure,
           separateur,
           heure_entree.minute,
           separateur,
           heure_entree.seconde);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvHeureBcdHex
* PARAMETRES: L'heure au format BCD.
* RETOUR: L'heure au format hexa.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Conversion de l'heure BCD en hexa.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvHeureBcdHex(struct_hrd_heure_bcd  heure_entree,
                             struct_hrd_heure_hex *heure_sortie)

{

   heure_sortie->heure =
              (unsigned char)ConversionBcdEnHexa(&(heure_entree.heure),1);
   heure_sortie->minute =
             (unsigned char)ConversionBcdEnHexa(&(heure_entree.minute),1);
   heure_sortie->seconde =
            (unsigned char)ConversionBcdEnHexa(&(heure_entree.seconde),1);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvHeureBcdBcd
* PARAMETRES: L'heure au format bcd.
* RETOUR: L'heure au format bcd
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Simple recopie.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvHeureBcdBcd(struct_hrd_heure_bcd  heure_entree,
                             struct_hrd_heure_bcd *heure_sortie)

{
   heure_sortie->heure = heure_entree.heure;
   heure_sortie->minute = heure_entree.minute;
   heure_sortie->seconde = heure_entree.seconde;
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvHeureBcdChaineCarac
* PARAMETRES: L'heure au format BCD.
* RETOUR: L'heure en chaine de caracteres.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Transformer la structure heure BCD en chaine de caracteres.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvHeureBcdChaineCarac(struct_hrd_heure_bcd  heure_entree,
                                     unsigned char *heure_sortie,
									 size_t heure_sortieSize)

{
   struct_hrd_heure_hex heure_hex;

   ConvHeureBcdHex(heure_entree,&heure_hex);
   ConvHeureHexChaineCarac(heure_hex, heure_sortie, heure_sortieSize);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvHeureBCD
* PARAMETRES: L'heure au format BCD.
* RETOUR: L'heure sous differents formats.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE:Conversion d'une heure BCD aux formats definis
*      dans enum_format_date_heure.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvHeureBCD(struct_hrd_heure_bcd heure_entree,
                         struct_hrd_heure_sortie *heure_sortie)
{
   ConvHeureBcdBcd(heure_entree,&(heure_sortie->bcd));
   ConvHeureBcdHex(heure_entree,&(heure_sortie->hex));
   ConvHeureBcdChaineCarac(heure_entree, heure_sortie->chaine_carac, sizeof(heure_sortie->chaine_carac));

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ConvHeureHex
* PARAMETRES: L'heure en hexa.
* RETOUR: L'heure sous differents formats.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE:Conversion d'une heure hexadecimale aux formats definis
*      dans enum_format_date_heure.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ConvHeureHex(struct_hrd_heure_hex heure_entree,
                         struct_hrd_heure_sortie *heure_sortie)
{

   ConvHeureHexHex(heure_entree,&(heure_sortie->hex));
   ConvHeureHexBcd(heure_entree,&(heure_sortie->bcd));
   ConvHeureHexChaineCarac(heure_entree, heure_sortie->chaine_carac, sizeof(heure_sortie->chaine_carac));

}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void HRDConversionHeure
* PARAMETRES: L'heure.
* RETOUR: L'heure sous differents formats.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE:Conversion d'une heure dans un format donne aux formats definis
*      dans enum_format_date_heure.
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC void HRDConversionHeure(struct_hrd_heure_entree heure_entree,
                                 struct_hrd_heure_sortie *heure_sortie)

{

   switch (heure_entree.format)
      {
         case FORMAT_HEX :
            /*L'heure en entree est au format hexadecimal*/
            ConvHeureHex(heure_entree.hex,heure_sortie);
            break;

         case FORMAT_BCD :
            /*L'heure en entree est au format BCD*/
            ConvHeureBCD(heure_entree.bcd,heure_sortie);
            break;

         default :

            break;
      }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: HRDConversionJulienDate
* PARAMETRES: - le jour julien IN
*             - la date d‚termin‚ OUT
*             - l'ann‚e pour laquelle on veut la date
* RETOUR: - TRUE si la date a ‚t‚ calcul‚e, FALSE sinon
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Transforme le jour julien en date dans l'ann‚e demand‚e
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean HRDConversionJulienDate( unsigned short julien,
                                        struct_hrd_date_entree_hex *date,
                                        int annee)
{
   int                           max_julien;
   int                           indice;

   date->annee = annee;
   if( bissextile( date->annee))
   {
       indice = 0;
       max_julien = 365;
   }
   else
   {
       indice = 1;
       max_julien = 366;
   }

   /* verifier que la valeur en entree est correcte */
   if( ( julien <= 0) && ( max_julien < julien))
      return( FALSE);

   /* calculer la date */
   date->mois = 1;
   while( ( julien > julian_table[ indice][ date->mois + 1]) && ( date->mois < 12))
      (date->mois) ++;

   date->jour = julien - julian_table[ indice][ date->mois];

   return( TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: HRDConversionJulienDateCourante
* PARAMETRES: - le jour julien IN
*             - l'heure du jour julien IN
*             - la date d‚termin‚ OUT
*             - un boolean indiquant si on refuse une date dans le futur
* RETOUR: - TRUE si la date a ‚t‚ calcul‚e, FALSE sinon
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Transforme le jour julien en date courante, relative … l'ann‚e
*       en cour
* NOTA: Si le futur est interdit, on a besoin de l'heure
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean HRDConversionJulienDateCourante( unsigned short julien,
                                                struct_hrd_heure_hex *heure_julien,
                                                struct_hrd_date_entree_hex *date,
                                                boolean futur_interdit)
{
   struct_hrd_date_entree_hex    date_courante;
   struct_hrd_heure_hex          heure_courante;
   char                          compare;
   boolean                       ok;

   date_courante = HRDDonneDate();
   heure_courante = HRDDonneHeure();

   ok = HRDConversionJulienDate( julien, date, date_courante.annee);
   if( ! ok)
   {
      /* cela peut signifier qu'on a regard‚ l'ann‚e pr‚c‚dente qui
       * ‚tait bissextile
       */
       if( futur_interdit)
       {
          ok = HRDConversionJulienDate( julien, date, date_courante.annee - 1);
          return( ok);
       }
       else
          return( FALSE);
   }

   /* on a une date correcte, si le futur est permis, on a fini */
   if( ! futur_interdit)
      return( TRUE);

   /* le futur est interdit, donc la date calcul‚e est-elle dans
    * le futur ?
    */
   if( heure_julien == NULL)
      return( FALSE);

   compare = HRDCompareDatesHeures ( *date, *heure_julien,
                                     date_courante, heure_courante);
   if( compare == 1)
   {
      /* on est dans le futur, donc il faut prendre l'annee pr‚c‚dente */
      ok = HRDConversionJulienDate( julien, date, date_courante.annee);
      return( ok);
   }

   return( TRUE);
}
