

#ifndef MSTR_DETECTION_PAIR_H
#define MSTR_DETECTION_PAIR_H

#include <QString>
#include <QList>


class MEnteredKey
{
public:
	MEnteredKey(){m_cAscii = '\0';m_iScanCode=0;};
	bool operator==(const MEnteredKey& right)
	{
		if(m_cAscii == right.m_cAscii  
			|| (m_iScanCode == right.m_iScanCode && m_iScanCode != 0))
			return true;
		else
			return false;
		
	};

	QString m_sText;
	char m_cAscii;
	int m_iScanCode;
};


class MStrDetectionPair
{
public:
	MStrDetectionPair();
	~MStrDetectionPair();
	MStrDetectionPair(QString sPairName);

	bool initialize(QString sConfRegistryPath);
	
	bool isPreambleKeyMatch(MEnteredKey *pKey, int iIndex);
	bool isPostambleMatch(QList <MEnteredKey *> * pLstEnteredKeys);
	int getNumPreambleKeys();
	int getNumPostambleKeys();
	bool isActive(){return m_bActive;};


	QString getName(){return m_sPairName;};
	QString getEndingAscii(){return m_sEndingAscii;};
	
	bool m_bEnvoiePrePostambule;
	bool m_bActive;
	
	virtual MStrDetectionPair& operator=(const MStrDetectionPair& right);


private:
	MEnteredKey * getEnteredKeyFromDesc(QString sCharDesc);
	void extractPreamble();
	void extractPostamble();

	QString m_sPairName;
	QString m_sPreamble;
	QString m_sPostamble;
	QString m_sEndingAscii;

	QList <MEnteredKey *> m_lstPreambleKeys;
	QList <MEnteredKey *> m_lstPostambleKeys;
};



#endif
