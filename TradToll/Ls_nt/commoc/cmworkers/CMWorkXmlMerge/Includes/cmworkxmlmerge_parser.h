/******************* (v) 2018 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 																	 */
/* FILE: cmworkxmlmerge_parser.h                                             */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMWXMLMERGE_PARSER_H
#define CMWXMLMERGE_PARSER_H


#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------- INCLUDES:  -------------------------------*/

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

#define MAX_LOCAL_ORDER_LEN 500

/*-------------------------------- TYPEDEFS:  -------------------------------*/
	
typedef struct 
{
  CHAR szLogin[MAX_LOCAL_ORDER_LEN+1];
  CHAR szPassword[MAX_LOCAL_ORDER_LEN+1];
  CHAR szName[MAX_LOCAL_ORDER_LEN+1];
  CHAR szIdentifier[MAX_LOCAL_ORDER_LEN+1];
  CHAR szRole[MAX_LOCAL_ORDER_LEN+1];
  CHAR szNoPin[MAX_LOCAL_ORDER_LEN+1];
  CHAR szChangePinOnLogin[MAX_LOCAL_ORDER_LEN+1];
}
struct_FILE_DATA_XML;

typedef struct
{
	XML_TOOLS_Ctx *pXmlCtx;
	void *pLstNode;
	DWORD dwCurrent;
}
struct_PARSER_CONTEXT;
	
/*-------------------------------- VARIABLES: -------------------------------*/

struct_PARSER_CONTEXT * XML_PARSER_CONTEXT_New (void);
BOOL XML_PARSER_CONTEXT_Delete_All (struct_PARSER_CONTEXT **phMsg);
enum_xml_tools_result FILE_XML_Load(IN struct_PARSER_CONTEXT *pParserContext, IN CHAR *szFilePath);
BOOL FindElementByIdentifier(IN struct_PARSER_CONTEXT *pParserContext, IN char *pcLogin, OUT struct_FILE_DATA_XML *psXmlElement);
DWORD GetNumberOfElements(IN struct_PARSER_CONTEXT *pParserContext);
BOOL GetNextElement(IN struct_PARSER_CONTEXT *pParserContext, OUT struct_FILE_DATA_XML *psXmlElement);

#ifdef __cplusplus
}
#endif


#undef I
#undef INIT
#undef PUBLIC
#endif
/*-------------------------------- END OF FILE ------------------------------*/