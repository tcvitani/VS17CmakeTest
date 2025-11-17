
#ifndef WAITUSER_DLG_H
#define WAITUSER_DLG_H


class Ui_WaitUserDlg;

class MWaitUserDlg: public QDialog
{
    Q_OBJECT

public:
	MWaitUserDlg(QWidget *parent);

	
private slots:

protected:


private:
	Ui_WaitUserDlg *ui;
};

#endif

