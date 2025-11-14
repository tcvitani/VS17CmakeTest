
#ifndef MPOLICE_VALUE_H
#define MPOLICE_VALUE_H

#include <QString>

#define UNDEFINED_POLICE_VALUE -1

// Used to handle police value format which is a comma separated string, with values: 
// 1.	Font family name, 
// 2.	point size, 
// 3.	font weight: <50 - normal, >50-Bold
// 4.	underline: 0,1
// 5.	italic:0,1
// 6.	strike out:0,1
// 7.	foreground color:
// 8.	background color:
//example: Arial,12,70,0,0,0,000000,FFFFFF

class MPoliceValue
{
public:
	MPoliceValue();
	~MPoliceValue();
	
	bool isEmpty();
	bool isFontValid();
	void updateWithString(QString sAdditionalValues);

	QString getFontFamily(){return m_sFontFamily;};
	int getPointSize(){return m_iPointSize;};
	int getFontWeight(){return m_iFontWeight;};

	int getUnderline(){return m_iUnderline;};
	int getItalic(){return m_iItalic;};
	int getStrikeOut(){return m_iStrikeOut;};
	
	MPoliceValue& operator=(const MPoliceValue& right);

	QString getColorStyle();	
private:
	QString m_sFontFamily;
	int m_iPointSize;
	int m_iFontWeight;
	int m_iUnderline;
	int m_iItalic;
	int m_iStrikeOut;
	QString m_sForeColor;
	QString m_sBckColor;

};



#endif

