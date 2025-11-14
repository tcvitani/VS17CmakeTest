
#ifndef MHTML_CODEC_H
#define MHTML_CODEC_H

#include <QString>




class MHtmlCodec
{
public:
	MHtmlCodec();
	~MHtmlCodec();
	
	void init(QString sMapFile);
	QString htmlEncodeStr(QString str);
	
	struct HtmlChar 
	{
		QChar chValue;
		QString sHtml;
		QString sAlterHtml;
	};

private:

	bool m_bInitialized;

	QList <HtmlChar*> m_lstHtmlEncMap; 
};



#endif

