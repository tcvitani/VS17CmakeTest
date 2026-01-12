/* --------------------------------------------------------------------
 * (C) 1998 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef RESINFO_VERSION_H
#define RESINFO_VERSION_H

#define RESINFO_VERSION          2,2,0,0
#define RESINFO_VERSION_STRING  "2,2,0,0"

#if DB_VER == 0
#define RESINFO_COMMENTS        "Generic interface mapper"
#define RESINFO_FILEDESC        "Database interface library (GENERIC)"

#elif DB_VER == 8
#define RESINFO_COMMENTS        "Built with OCI 8.0.5"
#define RESINFO_FILEDESC        "Database interface library (ORA8)"

#elif DB_VER == 9
#define RESINFO_COMMENTS        "Built with OCI 9.2.0.6"
#define RESINFO_FILEDESC        "Database interface library (ORA9)"

#elif DB_VER == 10
#define RESINFO_COMMENTS        "Built with OCI 10.2.0.3.0"
#define RESINFO_FILEDESC        "Database interface library (ORA10)"

#elif DB_VER == 11
#define RESINFO_COMMENTS        "Built with OCI 11.2.0.1.0"
#define RESINFO_FILEDESC        "Database interface library (ORA11)"

#elif DB_VER == 19
#define RESINFO_COMMENTS        "Built with OCI 19.3.0.0.0"
#define RESINFO_FILEDESC        "Database interface library (ORA19)"

#endif

#define RESINFO_COMPANY         "CSSI"
#define RESINFO_COPYRIGHT       "Copyright © 2008"
#define RESINFO_PRODUCT         "DBIF"

#endif // RESINFO_VERSION_H
