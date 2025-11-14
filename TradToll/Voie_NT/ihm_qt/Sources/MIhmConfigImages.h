#ifndef MIHM_CONFIG_IMAGES_H
#define MIHM_CONFIG_IMAGES_H

#include <QList>
#include <QString>

#define IHM_IMG_DEFAULT "DefaultListItem"

struct ConfigImage
{
	QString m_sImgName;
	QString m_sFileName;
	bool m_bAnimated;
};


class ImagesList
{
public:
	ImagesList();
	~ImagesList();
	
	inline void append(ConfigImage*p)
		{m_plstOrderedImages->append(p);};
	
	QList<ConfigImage*> *m_plstOrderedImages;

	static bool lessThan(const ConfigImage* x1, const ConfigImage* x2); 

	bool findImage(QString  &sImgName, ConfigImage ** pFound);

	void sort();
};


class MIhmConfigImages 
{
public:
	MIhmConfigImages();
	~MIhmConfigImages();

	bool initFromFile(QString sCfgFilePath);

	QString getImageFileName(QString sImgName, bool * pbIsAnimated);
	QString getImageFullPath(QString sImgName, bool * pbIsAnimated = NULL);
	bool isImageAnimated(QString sImgName);
	
	QString getLuhnCheckOKImgPath();
	QString getLuhnCheckNOKImgPath();
	QString getNoPictureImgPath();

private:

	QString m_sImagesRootPath;
	ImagesList m_lstImageList;
};






#endif


