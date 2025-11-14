

#ifndef MSTRING_DETECTION_H
#define MSTRING_DETECTION_H

#include <QString>
#include <QObject>
#include <QTimer>

#define IHM_REG_KEY_STRING_DETECTION "StringDetection"

class MIhmMsgStringDetection;

class MEnteredKey;
class MStrDetectionPair;
class MStringDetectionAtm;




class MStringDetection:public QObject
{
    Q_OBJECT
public:
	MStringDetection();
	~MStringDetection();

	void initialize(QString sConfigKey);
	bool checkIfStringDetection(QString sText, int iScanCode);

	bool isStringComplete();
	bool isTollKey();
	int  getTollKeyDetection();
	QString  getCompleteStringDetectionWithPrePostAmbules();
	bool getCompleteStringDetection(MIhmMsgStringDetection * pMsg, bool * pbPairActuallyActive);

	void resetPairs();

	void updateStrDetectionPairs(QList <MStrDetectionPair *> *lst);
	
	static bool loadDetectionPairs(QString sConfigKey, QList <MStrDetectionPair *> *pLst);
	bool loadDetectionTimeout(QString sConfigKey);

	void deactivateAllPairs();
	void activateAllPairs();

signals:
	void reemitKeyEvent();


private slots:
	void onDetectionTimerTimeout();
		
private:
	int getCountPreambleMatch(bool *pbCompleteMatch); //and record the selected pair match if it is single one
	bool isPreamblePairMatch(MStrDetectionPair * pDetPair, bool *pbFullMatch);
	bool isPostambleMatch(bool * pbInputBufferOK);

	void detectionComplete();
	QString getDetectedString(); //note it might be incomplete
	
	void startTimer();
	void stopTimer();
	
	MStrDetectionPair * m_pTollKeyDetectionPair;
	QList <MStrDetectionPair *> m_lstPairs;

	QTimer m_tmDetectionTimer;
	unsigned int m_iDetectionTimeOut;


	MStringDetectionAtm * m_pStrDetAtm;

	void addNewKeyToBuffer(QString sText, int iScanCode);
	void clearInputBuffer();

	//entered keys buffer 
	QList <MEnteredKey *> m_lstEnteredKeys;
	
	//the detection pair for which was detectected the preamble complete match
	MStrDetectionPair * m_pDetectionPairMatch;	 
	
	//string detection is completed with success
	bool m_bDetectionCompleteSucces;
	
	friend class MStringDetectionAtm;
};


#endif
