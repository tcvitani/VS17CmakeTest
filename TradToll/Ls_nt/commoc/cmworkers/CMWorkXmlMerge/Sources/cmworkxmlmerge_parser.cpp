/******************* (v) 2018 EMOVIS - All rights reserved *******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   CMWORKXMLMERGE                                                  */
/* FILE:     cmworkgeaimg.c                                                  */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains wrapped functions which enables using of   */
/*             xml_tools and XERCES libraries in the ComMOC plugin.          */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
#include <xml_tools.h>
#include <xml_tools_dom.hpp>
#include <cmworkxmlmerge_parser.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

/**/
/*****************************************************************************/
/*SYNTAX: PUBLIC struct_PARSER_CONTEXT * XML_PARSER_CONTEXT_New(void)        */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Creates context used by XML tools.                             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*                                                                           */
/*===========================================================================*/
/*  Return                  Description                                      */
/*---------------------------------------------------------------------------*/
/* struct_PARSER_CONTEXT * Poiner to context. Returns NULL is the context    */
/*                         cannot be created                                 */
/*****************************************************************************/
PUBLIC struct_PARSER_CONTEXT * XML_PARSER_CONTEXT_New(void)
{
	struct_PARSER_CONTEXT * pNew = (struct_PARSER_CONTEXT *)XML_New(sizeof(struct_PARSER_CONTEXT));

	if (enuXML_TOOLS_OK != XML_CreateCtx(&(pNew->pXmlCtx)))
	{
		XML_Delete((PVOID*)(&pNew));
		pNew = NULL;
	}

	return pNew;
}

/**/
/*****************************************************************************/
/*SYNTAX: BOOL XML_PARSER_CONTEXT_Delete_All(struct_PARSER_CONTEXT **phMsg)  */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Deletes context used by XML tools.                             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*        struct_PARSER_CONTEXT **phMsg - Context previously open by         */
/*                                        XML_PARSER_CONTEXT_New             */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  Context is deleted                                 */
/*  FALSE                 Error deleting context                             */
/*****************************************************************************/
PUBLIC BOOL XML_PARSER_CONTEXT_Delete_All(struct_PARSER_CONTEXT **phMsg)
{
	struct_PARSER_CONTEXT *hMsg = *phMsg;

	if (phMsg == NULL || *phMsg == NULL)
		return TRUE;

	//!!! do not forget to delete the tools contexts (parser and structures)
	if (hMsg->pXmlCtx != NULL)
	{
		XML_FreeCtx(hMsg->pXmlCtx);
		hMsg->pXmlCtx = NULL;
	}

	if (!XML_Delete((PVOID*)phMsg))
		return FALSE;

	*phMsg = NULL;

	return TRUE;
}

/**/
/*****************************************************************************/
/*SYNTAX: BOOL FindElementByIdentifier(                                      */
/*                       IN struct_PARSER_CONTEXT *pParserContext,           */
/*                       IN char *pcIdentifier,                              */
/*                       OUT struct_FILE_DATA_XML *psXmlElement)             */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Finds XML element based on the identifier attribute value.     */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*     IN struct_PARSER_CONTEXT *pParserContext - Context previously open by */
/*                                                XML_PARSER_CONTEXT_New     */
/*     IN char *pcLogin                         - Identifier value to search */
/*     OUT struct_FILE_DATA_XML *psXmlElement   - Decoded XML element        */
/*                                                attributes                 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  Element found                                      */
/*  FALSE                 Element not found                                  */
/*****************************************************************************/
BOOL FindElementByIdentifier(IN struct_PARSER_CONTEXT *pParserContext,
							 IN char *pcLogin,
							 OUT struct_FILE_DATA_XML *psXmlElement)
{
	DOMNodeList *pLstNode = NULL;
	int i = 0;

	pLstNode = (DOMNodeList *)pParserContext->pLstNode;
	memset(psXmlElement, 0, sizeof(struct_FILE_DATA_XML));

	for (int i = 0; i< pLstNode->getLength(); i++)
	{
		DOMNode *pElementUser = pLstNode->item(i);
		getAttributeValue(static_cast<DOMElement*>(pElementUser), "login", psXmlElement->szLogin, sizeof(psXmlElement->szLogin));
		if (strcmp(psXmlElement->szLogin, pcLogin) == 0)
		{
			getAttributeValue(static_cast<DOMElement*>(pElementUser), "identifier", psXmlElement->szIdentifier, sizeof(psXmlElement->szIdentifier));
			getAttributeValue(static_cast<DOMElement*>(pElementUser), "password", psXmlElement->szPassword, sizeof(psXmlElement->szPassword));
			getAttributeValue(static_cast<DOMElement*>(pElementUser), "name", psXmlElement->szName, sizeof(psXmlElement->szName));
			getAttributeValue(static_cast<DOMElement*>(pElementUser), "role", psXmlElement->szRole, sizeof(psXmlElement->szRole));
			getAttributeValue(static_cast<DOMElement*>(pElementUser), "noPin", psXmlElement->szNoPin, sizeof(psXmlElement->szNoPin));
			getAttributeValue(static_cast<DOMElement*>(pElementUser), "changePinOnLogin", psXmlElement->szChangePinOnLogin, sizeof(psXmlElement->szChangePinOnLogin));

			return TRUE;
		}
	}

	return FALSE;
}

/**/
/*****************************************************************************/
/*SYNTAX: GetNumberOfElements(IN struct_PARSER_CONTEXT *pParserContext)      */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Returns numnber of XML elements loaded tothe list.             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*     IN struct_PARSER_CONTEXT *pParserContext - Context previously open by */
/*                                                XML_PARSER_CONTEXT_New     */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  DWORD                 Number of XML elements                             */
/*****************************************************************************/
DWORD GetNumberOfElements(IN struct_PARSER_CONTEXT *pParserContext)
{
	DOMNodeList *pLstNode = (DOMNodeList *)pParserContext->pLstNode;

	return (DWORD)pLstNode->getLength();
}

/**/
/*****************************************************************************/
/*SYNTAX: GetNextElement(IN struct_PARSER_CONTEXT *pParserContext,           */
/*                       OUT struct_FILE_DATA_XML *psXmlElement)             */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Gets element by element and returns decoded element attributes.*/
/*            Every function call moves the pointer to the next element in   */
/*            the list.                                                      */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*     IN struct_PARSER_CONTEXT *pParserContext - Context previously open by */
/*                                                XML_PARSER_CONTEXT_New     */
/*     OUT struct_FILE_DATA_XML *psXmlElement   - Pointer to structure that  */
/*                                                conatins decoded attribute */
/*                                                elements                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  Element found                                      */
/*  FALSE                 No more elements - end of list                     */
/*****************************************************************************/
BOOL GetNextElement(IN struct_PARSER_CONTEXT *pParserContext, OUT struct_FILE_DATA_XML *psXmlElement)
{
	DOMNodeList *pLstNode = NULL;

	pLstNode = (DOMNodeList *)pParserContext->pLstNode;
	memset(psXmlElement, 0, sizeof(struct_FILE_DATA_XML));

	if (pLstNode->getLength() > 0 && pParserContext->dwCurrent < pLstNode->getLength())
	{
		DOMNode *pElementUser = pLstNode->item(pParserContext->dwCurrent);

		getAttributeValue(static_cast<DOMElement*>(pElementUser), "login", psXmlElement->szLogin, sizeof(psXmlElement->szLogin));
		getAttributeValue(static_cast<DOMElement*>(pElementUser), "password", psXmlElement->szPassword, sizeof(psXmlElement->szPassword));
		getAttributeValue(static_cast<DOMElement*>(pElementUser), "name", psXmlElement->szName, sizeof(psXmlElement->szName));
		getAttributeValue(static_cast<DOMElement*>(pElementUser), "identifier", psXmlElement->szIdentifier, sizeof(psXmlElement->szIdentifier));
		getAttributeValue(static_cast<DOMElement*>(pElementUser), "role", psXmlElement->szRole, sizeof(psXmlElement->szRole));
		getAttributeValue(static_cast<DOMElement*>(pElementUser), "noPin", psXmlElement->szNoPin, sizeof(psXmlElement->szNoPin));
		getAttributeValue(static_cast<DOMElement*>(pElementUser), "changePinOnLogin", psXmlElement->szChangePinOnLogin, sizeof(psXmlElement->szChangePinOnLogin));

		pParserContext->dwCurrent++;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/**/
/*****************************************************************************/
/*SYNTAX: enum_xml_tools_result generateDataUsingDOMParser(                  */
/*                           IN struct_PARSER_CONTEXT *pParserContext,       */
/*                           IN XercesDOMParser *pParser)                    */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Uses the DOM parser to load list of XML elements. The pointer  */
/*            to loaded list is stored in the context.                       */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*     IN struct_PARSER_CONTEXT *pParserContext - Context previously open by */
/*                                                XML_PARSER_CONTEXT_New     */
/*     IN XercesDOMParser *pParser              - Pointer to DOM parser      */
/*                                                object                     */
/*===========================================================================*/
/*  Return                         Description                               */
/*---------------------------------------------------------------------------*/
/*  enuXML_TOOLS_OK                List of elements loaded                   */
/*  enuXML_TOOLS_ERR_INVALID_XML   Error loading elements                    */
/*****************************************************************************/
enum_xml_tools_result generateDataUsingDOMParser(IN struct_PARSER_CONTEXT *pParserContext, IN XercesDOMParser *pParser)
{
	XERCES_CPP_NAMESPACE::DOMDocument* doc = pParser->getDocument();
	enum_xml_tools_result	eReturnStatus = enuXML_TOOLS_OK;

	if (doc != NULL)
	{
		DOMElement* root = doc->getDocumentElement();
		pParserContext->pLstNode = (void *)getElementsByTagName(static_cast<DOMElement*>(root), "user");
		pParserContext->dwCurrent = 0;
	}
	else
		return enuXML_TOOLS_ERR_INVALID_XML;

	return eReturnStatus;
}

/**/
/*****************************************************************************/
/*SYNTAX: enum_xml_tools_result ExtractXMLFileData(                          */
/*                                 IN struct_PARSER_CONTEXT *pParserContext) */
/*===========================================================================*/
/*TYPE:   Private function.                                                  */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Initializes the DOM parser and loads list of XML elements by   */
/*            calling the generateDataUsingDOMParser function.               */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*     IN struct_PARSER_CONTEXT *pParserContext - Context previously open by */
/*                                                XML_PARSER_CONTEXT_New     */
/*===========================================================================*/
/*  Return                         Description                               */
/*---------------------------------------------------------------------------*/
/*  enuXML_TOOLS_OK                List of elements loaded                   */
/*  enuXML_TOOLS_ERR_INVALID_XML   Error loading elements                    */
/*  enuXML_TOOLS_ERR_INVALID_CTX   Invalid context                           */
/*****************************************************************************/
enum_xml_tools_result ExtractXMLFileData(IN struct_PARSER_CONTEXT *pParserContext)
{
	enum_xml_tools_result	eReturnStatus = enuXML_TOOLS_OK;

	XercesDOMParser * pDomParser = XML_GetDomParser(pParserContext->pXmlCtx);

	if (pDomParser != NULL)
	{
		eReturnStatus = generateDataUsingDOMParser(pParserContext, pDomParser);
	}
	else
	{
		eReturnStatus = enuXML_TOOLS_ERR_INVALID_CTX;
	}

	return eReturnStatus;
}

/**/
/*****************************************************************************/
/*SYNTAX: enum_xml_tools_result FILE_XML_Load(                               */
/*                                  IN struct_PARSER_CONTEXT *pParserContext,*/
/*                                  IN CHAR *szFilePath)                     */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            Loads XML file elements to the list.                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*     IN struct_PARSER_CONTEXT *pParserContext - Context previously open by */
/*                                                XML_PARSER_CONTEXT_New     */
/*     IN CHAR *szFilePath                      - File name                  */
/*===========================================================================*/
/*  Return                         Description                               */
/*---------------------------------------------------------------------------*/
/*  enuXML_TOOLS_OK                List of elements loaded                   */
/*  enuXML_TOOLS_ERR_INVALID_XML   Error loading elements                    */
/*  enuXML_TOOLS_ERR_INVALID_CTX   Invalid context                           */
/*                                 Other codes could be found in xml_tolls.h */
/*****************************************************************************/
PUBLIC enum_xml_tools_result FILE_XML_Load(IN struct_PARSER_CONTEXT *pParserContext, IN CHAR *szFilePath)
{
	UINT			uiDone = 0;
	static DWORD	dwCntR = 1;
	CHAR			szXsdValidPath[MAX_PATH] = { 0 };
	DWORD			dwErr = 0;
	CHAR			szXmlFilesPath[MAX_PATH] = { 0 };
	CHAR			szValidPath[MAX_PATH] = { 0 };

	enum_xml_tools_result	eStatus = enuXML_TOOLS_OK;

	eStatus = XML_InitXmlParser(pParserContext->pXmlCtx, TRUE);

	if (enuXML_TOOLS_OK != eStatus &&
		enuXML_TOOLS_ERR_PARSER_ALREADY_INITIALIZED != eStatus)
	{

	}
	else
	{
		eStatus = XML_ParseDOMXmlFile(pParserContext->pXmlCtx, szFilePath, FALSE);
		if (enuXML_TOOLS_OK == eStatus)
		{
			eStatus = ExtractXMLFileData(pParserContext);
		}
	}

	return eStatus;
}
/*-------------------------------- END OF FILE ------------------------------*/