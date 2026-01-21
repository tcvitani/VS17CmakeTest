#include <QApplication>
#include <QMessageBox>
#include "MTranslator.h"
#include "formmaintransltest.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MTranslator translator;
    int res = translator.initLanguagesFromFile("qt_trans_gui_Languages.xml", "croatian");
	
	if(QCoreApplication::instance() == nullptr)
	{
		QMessageBox::critical(nullptr, "Error", "QCoreApplication instance is null!");
		return -1;
	}

	if (!QCoreApplication::installTranslator(&translator))
	{
		QMessageBox::critical(nullptr, "Error", "Failed to install translator!");
	}

    FormMainTranslTest formMainTranslTest(&app, &translator);

    formMainTranslTest.show();
    return app.exec();
}
