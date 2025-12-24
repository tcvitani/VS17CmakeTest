/******************* (v) 2006 CSSI - All rights reserved *********************/
/*				                                                             */
/* --------------------------------------------------------------------------*/
/* MODULE:																	 */
/* FILE:     emi_pic_import.h											 */
/* LANGUAGE: C																 */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:																	 */
/*****************************************************************************/
#ifndef EMI_PIC_IMPORT_H
#define EMI_PIC_IMPORT_H
/*--------------------------- INCLUDES:  ------------------------------------*/
#include <windows.h>
#include <io.h>
/*--------------------------- RESERVED:  ------------------------------------*/
#include <protect.h>
/*--------------------------- EXTERNALS: ------------------------------------*/

/*--------------------------- DEFINES:   ------------------------------------*/
#define MAX_DESTINATIONS	10
#define MAX_EXTENSION_LEN	6

#define FILE_EXISTANCE	0
#define FILE_NOT_FOUND	-1
/*--------------------------- TYPEDEFS:  ------------------------------------*/
typedef enum
{
	FIRST_FORMAT_ELEMENT			= 0,
		
		ELEMENT_PLAZA		= FIRST_FORMAT_ELEMENT,
		ELEMENT_LANE,
		ELEMENT_YEAR,
		ELEMENT_MONTH,
		ELEMENT_DAY,
		ELEMENT_HOUR,
		ELEMENT_MINUTE,
		ELEMENT_SECOND,
		ELEMENT_MILLISECOND,
		ELEMENT_RECORD_ID,
		ELEMENT_TRANSACTION,
		ELEMENT_GENERIC_1,
		ELEMENT_GENERIC_2,
		
		END_FORMAT_ELEMENT,
		
		LAST_FORMAT_ELEMENT	= END_FORMAT_ELEMENT - 1,
		NB_FORMAT_ELEMENTS	= END_FORMAT_ELEMENT - FIRST_FORMAT_ELEMENT, 
}enum_format_element;

typedef struct
{
	BOOL
		bIsUsed;		// Is element present in format
	INT
		iValue,			// Value of the element
		iFormatLen;		// Length (in characters) of the element
}struct_format_element;

typedef struct
{
	struct_format_element
		Element[NB_FORMAT_ELEMENTS];
	CHAR
		szFormatString[MAX_PATH];
	INT
		Order[NB_FORMAT_ELEMENTS];
}struct_format_data, *LP_FORMAT_DATA;

typedef struct
{
	LPSTR
		lpstrFilter;
	LPVOID
		lpNextFilter;
}struct_filter, *LP_FILTER;

typedef struct
{
	CHAR
		szDestinationDir[MAX_PATH],
		szTempExtension[MAX_EXTENSION_LEN];
	
	struct_format_data
		sFormatData;

	LP_FILTER
		lpFilterList;
}struct_config_data, *LP_CONFIG_DATA;

typedef struct
{
	LONG
		lValue[NB_FORMAT_ELEMENTS];
}struct_string_data, *LP_STRING_DATA;
/*--------------------------- FUNCTIONS: ------------------------------------*/
PROTECTED BOOL GetFormatConfiguration( LPSTR szFormat, LP_FORMAT_DATA lpFormatData );
PROTECTED BOOL CompareStringToFilterList( LPSTR szTestString, LP_FILTER lpFilterList );
PROTECTED VOID CreateDestinationPath( IN LPSTR szDestination, LP_CONFIG_DATA lpDestinationConfig, LP_STRING_DATA lpStringData );
PROTECTED BOOL GetDataFromString( LPSTR szDataString, LP_FORMAT_DATA lpFormatData, LP_STRING_DATA lpStringData );
PROTECTED VOID GetPartialDataFromString( LPSTR szDataString, LP_FORMAT_DATA lpFormatData, LP_STRING_DATA lpStringData );
PROTECTED BOOL CreateStringByFormat( OUT LPSTR szDataString, IN LP_FORMAT_DATA lpFormatData, IN LP_STRING_DATA lpStringData );

PROTECTED VOID FillStringDataWithDateTime( OUT LP_STRING_DATA lpStringData, IN LPSYSTEMTIME lpDateTime );
PROTECTED VOID FillDateTimeWithStringData( OUT LPSYSTEMTIME lpDateTime, IN LP_STRING_DATA lpStringData );
PROTECTED LONG GetNumberOfSubdirs( IN LPSTR szPath );

PROTECTED VOID AddCounterData( IN OUT LPSTR szFilePath );
PROTECTED VOID RemoveCounterData( IN OUT LPSTR szFilePath );
PROTECTED VOID IncreaseCounterData( IN OUT LPSTR szFilePath );
PROTECTED VOID ProcessNewFiles( short siInstId, LPVOID* lpFiles, CONST LPSTR szSourceDir );
/*--------------------------- VARIABLES: ------------------------------------*/

/*--------------------------- END OF FILE -----------------------------------*/
#endif // EMI_PIC_IMPORT_H