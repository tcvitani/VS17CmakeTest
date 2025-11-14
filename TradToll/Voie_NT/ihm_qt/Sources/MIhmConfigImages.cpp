

#include "MIhmConfigImages.h"
#include "MHelpFuncs.h"
#include "MTracer.h"

#include <QDir>
#include <QDomDocument>
#include <QFile>


extern "C" {
	#include <ihm.H>
};


#define XML_ELEMENT_IHM_CONFIG_ROOT		"IHM_CONFIG"
#define XML_ELEMENT_IMAGES				"IMAGES"

#define XML_ATTR_IMAGES_ROOT_FOLDER			"ROOT_FOLDER"

#define XML_ELEMENT_IMAGE				"IMG"
#define XML_ATTR_IMG_ID					"ID"
#define XML_ATTR_IMG_FILE				"FILE"
#define XML_ATTR_IMG_IS_ANIMATED		"IS_ANIMATED"


#define IMAGE_ID_LUHN_KEY_CHECK_OK		"ImgLuhnKeyOK"
#define IMAGE_ID_LUHN_KEY_CHECK_NOK		"ImgLuhnKeyNOK"
#define IMAGE_ID_NO_PICTURE				"NoPicture"

 
MIhmConfigImages::MIhmConfigImages()
{
	
}


MIhmConfigImages::~MIhmConfigImages()
{

}

bool MIhmConfigImages::initFromFile(QString sCfgFilePath)
{
	bool bRetVal = false;
    TRACE_D(QString( "MIhmConfigImages::initFromFile:%1").arg(sCfgFilePath));

	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument xmlData;

	QFile file(sCfgFilePath);
	
	if(file.open(QFile::ReadOnly))
	{

		bool bRet = xmlData.setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);

		if(bRet)
		{
			QDomElement root = xmlData.documentElement();
			if(root.tagName() != XML_ELEMENT_IHM_CONFIG_ROOT)
			{
				TRACE_W("MIhmConfigImages::initFromFile: Invalid root element!");
				bRet = false;
			}
			else
			{
				QDomNodeList lstImagesItems = root.elementsByTagName(XML_ELEMENT_IMAGES);
				
				if(lstImagesItems.count()>0)
				{	
					QDomNode nodeImages = lstImagesItems.at(0);

					//root element for all lane types
					QDomElement elImages = nodeImages.toElement();
					
					m_sImagesRootPath = MHelpFuncs::getAttributeText(&nodeImages, XML_ATTR_IMAGES_ROOT_FOLDER);

					QDomNodeList lstImgItems = elImages.elementsByTagName(XML_ELEMENT_IMAGE);
					
					QDomNode currImg; 
					
					for(int i=0;i<lstImgItems.count();i++)
					{
						currImg = lstImgItems.at(i); 

						QString sID = MHelpFuncs::getAttributeText(&currImg, XML_ATTR_IMG_ID);
						QString sFile = MHelpFuncs::getAttributeText(&currImg, XML_ATTR_IMG_FILE);
						QString sIsAnimated = MHelpFuncs::getAttributeText(&currImg, XML_ATTR_IMG_IS_ANIMATED);

						ConfigImage *pImage = new ConfigImage();
						pImage->m_sImgName = sID;
						pImage->m_sFileName = sFile;
						pImage->m_bAnimated = (sIsAnimated=="1"||sIsAnimated=="true")?true:false;

						m_lstImageList.append(pImage);
						bRetVal = true;
					}

					m_lstImageList.sort();

					
				}
				else
				{
					TRACE_W(QString("MIhmConfigImages::initFromFile: Element %1 not found!").arg(XML_ELEMENT_IMAGES));
					return false;
				}

			}
		}
		else
		{
			QString sMsg = QString("MIhmConfigImages::initFromFile:Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			TRACE_W(sMsg);
			return false;
		}
	}
	else
	{
		TRACE_W(QString("MIhmConfigImages::initFromFile: Unable to open file %1").arg(sCfgFilePath) );
		return false;
	}
	
	
	return bRetVal;
}



//If the image with that name is not found in the registry definition it would return empty string
QString MIhmConfigImages::getImageFileName(QString sImgName, bool * pbIsAnimated)
{
	QString sVal;
	ConfigImage * pFound;

	if(pbIsAnimated!=NULL)
		*pbIsAnimated = false;


	if(m_lstImageList.findImage(sImgName, &pFound))
	{
		sVal = pFound->m_sFileName;
		
		if(pbIsAnimated!=NULL)
			*pbIsAnimated = pFound->m_bAnimated;
	}

	return sVal;
}


bool MIhmConfigImages::isImageAnimated(QString sImgName)
{
	bool bAnimated = false;
	
	ConfigImage * pFound;

	if(m_lstImageList.findImage(sImgName, &pFound))
	{
		bAnimated = pFound->m_bAnimated;
	}

	return bAnimated;
}

//If the image with that name is not found in the registry definition it would return empty string
QString MIhmConfigImages::getImageFullPath(QString sImgName, bool * pbIsAnimated)
{
	QString sVal;
	
	if(pbIsAnimated!=NULL)
		*pbIsAnimated = false;
	
	if(!sImgName.isEmpty())
	{
		sVal = getImageFileName(sImgName, pbIsAnimated);

		if(sVal != "")
			sVal = QDir::cleanPath(m_sImagesRootPath + QDir::separator() + sVal); 
	}
	
	return sVal;
}


QString MIhmConfigImages::getLuhnCheckOKImgPath()
{
	QString sVal;
	
	sVal = getImageFullPath(IMAGE_ID_LUHN_KEY_CHECK_OK);
	
	return sVal;
}

QString MIhmConfigImages::getLuhnCheckNOKImgPath()
{
	return getImageFullPath(IMAGE_ID_LUHN_KEY_CHECK_NOK);
}




QString MIhmConfigImages::getNoPictureImgPath()
{
	return getImageFullPath(IMAGE_ID_NO_PICTURE);
}



//----------------------------------------------------------
ImagesList::ImagesList()
{
	m_plstOrderedImages = new QList<ConfigImage*>();
}

ImagesList::~ImagesList()
{
	while(!m_plstOrderedImages->isEmpty())
		delete m_plstOrderedImages->takeLast();

	delete m_plstOrderedImages;

}



bool ImagesList::lessThan(const ConfigImage* x1, const ConfigImage* x2) 
{ 
	return x1->m_sImgName < x2->m_sImgName; 
}


void ImagesList::sort()
{
	//qSort(m_plstOrderedImages->begin(),m_plstOrderedImages->end(), ImagesList::lessThan);
	std::sort(m_plstOrderedImages->begin(), m_plstOrderedImages->end(), ImagesList::lessThan);
}


bool ImagesList::findImage(QString  &sImgName, ConfigImage ** pFound)
{
	ConfigImage oWanted;
	oWanted.m_sImgName = sImgName;
	
	QList<ConfigImage*>::iterator it;
	
	//it = qBinaryFind(m_plstOrderedImages->begin(), m_plstOrderedImages->end(), &oWanted, ImagesList::lessThan);
	it = std::lower_bound(m_plstOrderedImages->begin(), m_plstOrderedImages->end(), &oWanted, ImagesList::lessThan);

	

	if( it !=  m_plstOrderedImages->end())
	{
		*pFound = *it;
		return true;
	}

	return false; //NULL
}