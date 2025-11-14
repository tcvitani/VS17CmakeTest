#include <QCoreApplication>
#include <QStringList>
#include <QHash>
#include <QTextStream>
#include <QDir>
#include <QFile>
#include <QSettings>

#include <iostream>

#include "param_handler.h"
#include "template_handler.h"

const QString WEB_PROXY_CONFIG_PATH		= "\\proxy\\config\\platform.config.php";
const QString DESKTOP_REGISTRY_ROOT_KEY	= "HKEY_LOCAL_MACHINE\\SOFTWARE\\CsRoute\\LaneController\\Config\\Modules\\IHM\\BL_IHM_PEAGER";

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

	QStringList arguments = app.arguments();

	if (arguments.contains("--help"))
	{
		ParamHandler::ShowHelp();
		exit(0);
	}

	QStringList requiredParams = ParamHandler::GetRequiredParams();
	QStringList optionalParams = ParamHandler::GetOptionalParams();
	StringPairs params = ParamHandler::ParseParams(arguments);

	if (!ParamHandler::CheckParams(params))
	{
		ParamHandler::ShowHelp();
		exit(1);
	}

	if (!params.contains("--yes"))
	{
		std::cout << "IHM Web interface will be updated with the following data:" << std::endl;

		QHashIterator<QString, QString> paramIter(params);

		while (paramIter.hasNext())
		{
			paramIter.next();
			std::cout << "\t" << paramIter.key().toStdString() << " => " << paramIter.value().toStdString() << std::endl;
		}

		std::cout << "Does this look okay? [Y/n]: ";

		QTextStream qtin(stdin);
		QString confirm = qtin.readLine();

		if (confirm.toUpper() != "Y" && confirm != "")
		{
			exit(0);
		}
	}

	QString platformConfig = TemplateHandler::BuildIHMWebPlatformConfig(params);
	StringPairs desktopConfig = TemplateHandler::BuildIHMWebDesktopConfig(params);

	if (params.contains("--verbose"))
	{
		std::cout << "\nUpdating web config with:\n";
		std::cout << std::string(80, '_') << std::endl;
		std::cout << platformConfig.toStdString();
		std::cout << std::string(80, '_') << std::endl;

		std::cout << "\nUpdating registry config with:\n";
		std::cout << std::string(80, '_') << std::endl;
		
		QHashIterator<QString, QString> i(desktopConfig);

		while (i.hasNext())
		{
			i.next();
			std::cout << i.key().toStdString() << " => " << i.value().toStdString() << std::endl;
		}

		std::cout << std::string(80, '_') << std::endl;
	}

	QString platformConfigFileName = params["--webroot-dir"];
	platformConfigFileName.append(WEB_PROXY_CONFIG_PATH);
	QFile platformConfigFile(platformConfigFileName);

	if (!platformConfigFile.open(QFile::WriteOnly | QFile::Truncate))
	{
		std::cerr << "Could not open " << platformConfigFileName.toStdString() << std::endl;
		exit(50);
	}

	QTextStream platformConfigFileStream(&platformConfigFile);
	platformConfigFileStream << platformConfig;
	platformConfigFile.close();

	QSettings regKey(DESKTOP_REGISTRY_ROOT_KEY, QSettings::NativeFormat);

	if (!regKey.isWritable())
	{
		std::cerr << "Could not open " << DESKTOP_REGISTRY_ROOT_KEY.toStdString() << std::endl;
		exit(52);
	}

	QHashIterator<QString, QString> desktopConfigIter(desktopConfig);
	
	while (desktopConfigIter.hasNext())
	{
		desktopConfigIter.next();

		// bugfix: some registry values should be written as DWORD
		// QT does this if you send an integer value instead of a string
		if (desktopConfigIter.key() == "WebServerPort")
		{
			regKey.setValue(desktopConfigIter.key(), desktopConfigIter.value().toInt());
		}
		else
		{
			regKey.setValue(desktopConfigIter.key(), desktopConfigIter.value());
		}
	}

	if (params.contains("--verbose"))
	{
		std::cout << std::endl << "Update completed successfully." << std::endl;
	}

    return 0;
}
