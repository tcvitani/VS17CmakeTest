/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL CONVERT
* FICHIER: CONVERT.C
* LANGAGE: C
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Outils_C/Convert/Sources/CONVERT.C_v  $
 * 
 *    Rev 1.2   Aug 22 2000 11:32:22   bph
 *  
 * 
 *    Rev 1.1   02 Oct 1998 11:06:44   bph
 *  
* --------------------------------------------------------------------
* $F_HEAD
*/
 

/*--------------- INCLUDES: ---------------*/

#include "stdio.h"
#include "ctype.h"
#include "convert.h"

/*--------------- RESERVED: ---------------*/

/*--------------- EXTERNALS: ---------------*/

#include "memclass.h"

/*--------------- DEFINES: ---------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- FUNCTIONS: ---------------*

/*--------------- VARIABLES: ---------------*/

/*--------------- CODE: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void  ConversionExpanse (unsigned char *,
*                                          unsigned char *,
*                                          unsigned char,
*                                          unsigned char)
* PARAMETRES: pointeur sur la chaine a convertir,
*             pointeur sur la chaine convertie,
*             longueur de la chaine convertie (nombre d octets)
*             type de cadrage (RIGHT ou LEFT)
* REMARQUE    pointeur chaine a convertir doit etre different de 
*             pointeur chaine convertie
* RETOUR: Aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: conversion d une chaine en une chaine Expansee
*       en tenant compte du cadrage de la source
*       (ex. 30 60 17 = 03 00 06 00 01 07) source cadree a gauche (lg=6)
*       (ex. X8 12 34 = 08 01 02 03 04) source cadree a droite (lg=5)
*       (ex. 81 23 4X = 08 01 02 03 04) source cadree a gauche (lg=5)
*       (ex. X1 23 4X = 01 02 03 04 ) source cadree a droite (lg=4)
*
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void WINAPI ConversionExpanse (unsigned char *src,
                               unsigned char *dst,
                               unsigned char lg_dst,
                               unsigned char alignment)
{
   unsigned char isrc;
   unsigned char idst;

   /*** initialisation de variables de travail ***/
   isrc = 0; /* Indice source */
   idst = 0; /* Indice destination */

   /*** Suivant le type d'alignement choisi ***/
   if (alignment == SRV_RIGHT)
   {
      /*** Masque du quarte de poids fort ***/
      dst[idst] = src[isrc] & 0x0F;
      isrc++;
      idst++;
   }
 
   /*** Tant que la longueur de la chaine n'est pas atteinte ***/
   while (idst < lg_dst)
   {
      /* Expansion du quartet de poids fort de l octet source */
      /* et memorisation dans l octet destination */
      dst[idst] = (src[isrc] & 0xF0) >> 4;
      idst++;

      if (idst < lg_dst)
      {
         /* conversion du quartet de poids faible de l octet source */
         /* et memorisation dans l octet destination */
         dst[idst] = src[isrc] & 0x0F;
         idst++;
      }

      /* passage a l octet a convertir suivant */
      isrc++;
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void ConversionExpanseBcd (unsigned char *,
*                                            unsigned char *,
*                                            unsigned char,
*                                            unsigned char)
* PARAMETRES: pointeur sur la chaine a convertir,
*             pointeur sur la chaine convertie,
*             longueur de la chaine a convertir (nombre d octets)
*             type de cadrage (RIGHT ou LEFT)
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES: 
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: conversion d une valeur expansee en une valeur BCD avec cadrage
*       a droite ou a gauche (pour les longueurs impaires)
*       (ex. 03 00 06 00 01 = 03 06 01 avec cadrage a droite
*                           = 30 60 10 avec cadrage a gauche
*        et 03 00 06 00 01 07 = 30 60 17)
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void WINAPI ConversionExpanseEnBcd (unsigned char *source,
                                    unsigned char *destination,
                                    unsigned char length,
                                    unsigned char alignment)
{
   unsigned char temporary;
   
   /*** si la longueur passee n'est pas paire ***/
   if ((length % 2) != 0)
   {
      /*** la conversion porte sur une longueur impaire ... ***/
      if (alignment == SRV_RIGHT)
      {
         /*** conversion du premier caractere et cadrage a droite ***/
         *destination = (*source & 0x0F);

         /*** pointer sur l adresse destination suivante ***/
         destination++;

         /*** pointer sur l adresse source suivante ***/
         source++;

         /*** la longueur restant a convertir devient paire... ***/
         length--;

      }
      else
      {
         /*** conversion du dernier caractere et cadrage a gauche ***/
         *(destination + ((length+1) / 2) - 1) = (*(source + (length-1)) << 4);

         /*** la longueur restant a convertir devient paire... ***/
         length--;
      }
   }

   while (length > 0)
   {
      /*** la conversion n est pas terminee... ***/
      /*** determination du quartet de poids fort ***/
      temporary = (*source << 4);

      /*** pointer sur l adresse source suivante ***/
      source++;

      /*** valeur convertie = quartet de poids fort | quartet de poids faible ***/
      *destination = (temporary | (*source & 0x0F));

      /*** pointer sur l adresse destination suivante ***/
      destination++;

      /*** pointer sur l adresse source suivante ***/
      source++;

      /*** 2 caracteres "source" ont ete traites, la longueur restant a ***/
      /*** convertir est amputee d autant ***/
      length -= 2;

   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC unsigned long  ConversionBcdEnHexa (unsigned char *,
*                                                     unsigned char)
* PARAMETRES: pointeur sur la chaine a convertir,
*             longueur de la chaine a convertir
* RETOUR: valeur convertie
* --------------------------------------------------------------------
* VARIABLES: 
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: conversion d une valeur BCD en une valeur hexadecimale
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC unsigned long WINAPI ConversionBcdEnHexa (unsigned char *bcd_value,
                                          unsigned char nb_bytes)
{
   unsigned long hexa_value;
   unsigned char i;
   unsigned long j;
   unsigned char treated_byte;

   /* initialisation du code de retour de la fonction */
   hexa_value = (unsigned long)0;

   for (i = 0, j = 1; i < nb_bytes; i++, (j *= 100))
   {
      treated_byte = bcd_value[nb_bytes - 1 - i];
      hexa_value = ((treated_byte & 0x0F) * j) + hexa_value;
      hexa_value = (((treated_byte & 0xF0) >> 4) * (j * 10)) + hexa_value;
   }

   return (hexa_value);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void  ConversionHexaEnBcd (unsigned long,
*                                             unsigned char *,
*                                             unsigned char)
* PARAMETRES: valeur a convertir,
*             adresse du tableau de stockage de la valeur convertie
*             taille du tableau de stockage de la valeur convertie
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES: 
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: conversion d une valeur hexadecimale en une valeur BCD
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void WINAPI ConversionHexaEnBcd (unsigned long hex_value,
                                 unsigned char *p_bcd_array,
                                 unsigned char nb_bytes)
{
   unsigned long diviseur;
   unsigned long quotient;
   unsigned char i;
   unsigned long j;

   /*** Initialisation du diviseur ***/
   diviseur = (unsigned long)1;

   /*** Mise a jour du diviseur en fonction de la longueur de la chaine ***/
   for (j = 0; j < ((2UL * nb_bytes) - 1); j++)
   {
      diviseur *= 10;
   }
   
   /*** Conversion de la chaine ***/
   for (i = 0; i < nb_bytes; i++)
   {
      quotient = (hex_value / diviseur);
      p_bcd_array[i] = (unsigned char)(quotient << 4);
      hex_value = hex_value - (quotient * diviseur);
      diviseur /= 10;

      quotient = (hex_value / diviseur);
      p_bcd_array[i] |= (unsigned char)quotient;
      hex_value = hex_value - (quotient * diviseur);

      /*** mise a jour du diviseur ***/
      diviseur /= 10;
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void ConversionHexaEnBcdExpanse (unsigned long,
*                                                  unsigned char *,
*                                                  unsigned char)
* PARAMETRES: valeur a convertir,
*             adresse du tableau de stockage de la valeur convertie
*             taille (en octets) de la valeur convertie (>1)
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES: 
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: conversion d une valeur hexadecimale en une valeur BCD expanse
* REMARQUE : comme c'est une expansion le parametre <taille> doit etre
*            superieur a 1.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void WINAPI ConversionHexaEnBcdExpanse (unsigned long hex_value,
                                        unsigned char *p_expansed_array,
                                        unsigned char expansed_length)
{
   /* 5 est le nombre d'octets necessaires pour faire tenir */
   /* 4.294.967.295 (0xFFFFFFFF) en BCD */
   unsigned char bcd_array[5];

   /* Si nb octets expanses < 2 (normalement impossible) */
   if (expansed_length < 2)
   {
      /* Si nb octets expanses = 1 (normalement impossible) ET valeur < 10 */
      if ((expansed_length == 1) && (hex_value < 10))       
      {
         /* Recopie valeur hexa dans premier element du tableau */
         p_expansed_array[0] = (unsigned char) hex_value;
      }
   }
   else /* Nb octets expanses >= 2 */
   {
      if (expansed_length % 2) /* Si nombre d'octets expanses impair */
      {  
         /* Conversion Hexa-->Bcd */
         ConversionHexaEnBcd (hex_value, 
                              bcd_array, 
                              (unsigned char) ((expansed_length/2)+1) );

         /* Conversion Bcd-->Bcd expanse (cadrage a droite) */
         ConversionExpanse (bcd_array, 
                            p_expansed_array, 
                            expansed_length,
                            SRV_RIGHT);
      }
      else /* Nombre d'octets expanses pair */
      {
         /* Conversion Hexa-->Bcd */
         ConversionHexaEnBcd ( hex_value, 
                               bcd_array, 
                               (unsigned char) (expansed_length/2) );

         /* Conversion Bcd-->Bcd expanse (cadrage à gauche) */
         ConversionExpanse ( bcd_array, 
                             p_expansed_array, 
                             expansed_length,
			                 SRV_LEFT );
      }
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void ConversionBcdEnAscii (unsigned char *str_bcd,
*                                            unsigned char *str_ascii,
*                                            unsigned short int bytes_bcd)
* PARAMETRES: pointeur sur la chaine BCD (compressee) a convertir,
*             pointeur recevant la chaine ASCII (la longueur de cette chaine
*               doit etre au moins egale a (bytes_bcd*2+1))
*             nombre d'octets BCD a convertir (attention un octet BCD =
*               deux digits BCD !)
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES: 
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: conversion d une valeur BCD (sous forme de tableau) en une 
*       chaine ASCII.
*       Exemple : str_bcd = "\x12\xa3\x00\xB9"
*                 ConversionBcdEnAscii( str_bcd, str_ascii, 4);
*         ===>    str_ascii = "12A300B9"
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC void WINAPI ConversionBcdEnAscii (unsigned char *str_bcd,
                                  unsigned char *str_ascii,
                                  unsigned short int bytes_bcd)
{
   unsigned short i;

   if( bytes_bcd )              /* Si au moins un octet BCD a convertir */
   {
      for(i=0;i<bytes_bcd;i++)  /* Boucler sur tous les octets BCD */
      {
         /*******************************************************************/
         /* Transformation en ASCII (1 octet BCD = 2 digits = 2 car. ASCII) */
         /*******************************************************************/
#pragma warning(push)
#pragma warning(disable: 4996)
		  sprintf ((char *)&str_ascii[i*2], "%02X", str_bcd[i]);
#pragma warning(pop)
      }
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void far ConversionAsciiEnBcdExpanse (unsigned char *str_ascii,
                                                        unsigned char *str_bcd,
                                                        unsigned short int bytes_ascii)
* PARAMETRES: pointeur recevant la chaine ASCII a convertir,
*             pointeur recevant la chaine BCD,
*             nombre d'octets ASCII a convertir 
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES: 
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: conversion d une valeur ASCII en BCD Expanse
*       Exemple : str_ascii = "12A3"
*                 ConversionBcdEnAscii( str_bcd, str_ascii, 4);
*         ===>    str_bcd = "\x01\x02\x0A\x03"
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC void WINAPI ConversionAsciiEnBcdExpanse (unsigned char *str_ascii,
                                         unsigned char *str_bcd,
                                         unsigned short int bytes_ascii)
{
   unsigned short i;

   if( bytes_ascii )              /* Si au moins un octet BCD a convertir */
   {
      for(i=0;i<bytes_ascii;i++)  /* Boucler sur tous les octets BCD */
      {
         /***********************************************************************/
         /* Transformation en BCD expanse (1 car ASCII = 1 digit = 1 octet BCD) */
         /***********************************************************************/
			
		  if (str_ascii[i] >= 'a' && str_ascii[i] <= 'f') 
			  str_bcd[i] = str_ascii[i] - 87;
		  else if (str_ascii[i] >= 'A' && str_ascii[i] <= 'D')
			  str_bcd[i] = str_ascii[i] - 55;
		  else
	          str_bcd[i] = str_ascii[i] & 0x0F;
      }
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC long ConversionAsciiEnHexa (unsigned char *str_ascii,
*                                             unsigned short nb_bytes,
*                                             unsigned char str_base)
* PARAMETRES: - pointeur sur la chaine ASCII à convertir,
*               cette chaine doit contenir les caracteres :
*               - '0' a '9'  (base 10 et 16)
*               - 'a' a 'f'  (base 16 seulement)
*               - 'A' a 'F'  ( "    "     "    )
*               Tout autre caractere provoquera un arret de la conversion.
*             - nombre d'octets ASCII a convertir
*             - base de la chaine ASCII :
*               - BASE10 = la chaine represente du decimal
*               - BASE16 = la chaine represente de l hexadecimal
* RETOUR: valeur hexadecimale (entier long)
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: conversion d une chaine ASCII en une valeur hexadecimale.
*       Exemple 1 : str_ascii = "12a300B9"
*                   val_hexa = ConversionAsciiEnHex( str_ascii, 8, 0);
*         ===>      val_hexa = 0x12A300B9
*       Exemple 2 : str_ascii = "123950"
*                   val_hexa = ConversionAsciiEnHex( str_ascii, 6, 1);
*         ===>      val_hexa = 123950
*       Exemple 3 : str_ascii = "15"
*                   val_hexa = ConversionAsciiEnHex( str_ascii, 2, 0);
*         ===>      val_hexa = 0x15 (soit 21 en base 10)
*                   val_hexa = ConversionAsciiEnHex( str_ascii, 2, 1);
*         ===>      val_hexa = 0x0F (soit 15 en base 10)
*       Exemple 4 : str_ascii = "123Z50"
*                   val_hexa = ConversionAsciiEnHex( str_ascii, 6, 1);
*         ===>      val_hexa = 50  (le 'Z' provoque l'arret de la conversion).
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC long WINAPI ConversionAsciiEnHexa ( unsigned char  *str_ascii,
                                    unsigned short nb_bytes,
                                    unsigned char  str_base)
{
   long hexa_value;
   int  i;
   long j;

   hexa_value = 0L;
   j = 1L;

   /*** Boucle sur la longueur ***/
   for (i=0; i<nb_bytes; i++)
   {
      /* Si digit ('0' a '9') */
      if (isdigit (str_ascii[nb_bytes-i-1]))
      {
         hexa_value += ((str_ascii[nb_bytes-i-1] - '0') * j);
      }
      else
      {
         /* Si base hexa et digit hexa */
         if ((str_base == SRV_BASE16) && isxdigit (str_ascii[nb_bytes-i-1]))
         {
            /* Si majuscule */
            if (isupper (str_ascii[nb_bytes-i-1]))
            {
               hexa_value += ((str_ascii[nb_bytes-i-1] - 'A' + 10) * j);
            }
            else /* minuscule */
            {
               hexa_value += ((str_ascii[nb_bytes-i-1] - 'a' + 10) * j );
            }
         }
         else /* Caractere non autorise */
         {
            /* Provoquer une sortie de boucle */
            i = nb_bytes;
         }
      }
      if( str_base == SRV_BASE10 )
      {
         j *= 10;  /* Base 10 (decimal) */
      }
      else
      {
         j *= 16;  /* Base 16 (hexadecimal) */
      }
   }

   return (hexa_value);
}
