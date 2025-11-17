#include <QApplication>

#include "maindlg.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	QString sBlName;
	bool bFirstParameterIsMbox = false;

	if (argc == 1)
		sBlName = QString("BL_IHM_PEAGER");
	else
	{
		sBlName = argv[1];
		if (sBlName.indexOf(".") >= 0)
		{
			sBlName = QString("BL_IHM_PEAGER");
		}
		else
		{
			bFirstParameterIsMbox = true;
		}
	}

	MainDlg widget(0, sBlName);
    widget.show();

	if (argc > 1)
	{
		int i;

		for (i = (bFirstParameterIsMbox?2:1); i < argc; i++)
			widget.executeScript(QString(argv[i]), false);
	}

    return app.exec();
}

