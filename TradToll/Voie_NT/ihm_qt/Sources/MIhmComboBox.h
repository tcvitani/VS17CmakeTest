
#ifndef MIHM_COMBO_BOX_H
#define MIHM_COMBO_BOX_H

#include <QComboBox>


class MIhmComboBox: public QComboBox
{
    Q_OBJECT

public:
    MIhmComboBox(QWidget *parent);
	~MIhmComboBox();
	
	void setAutoOpen(bool b){m_bAutoOpen = b;};
protected:
	virtual void focusInEvent(QFocusEvent * event);
	virtual void keyPressEvent(QKeyEvent * event);

signals:
	void gotFocus();

private:

	bool m_bAutoOpen;
};

#endif

