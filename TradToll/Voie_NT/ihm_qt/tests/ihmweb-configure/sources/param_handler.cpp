
#include <QDir>

#include "param_handler.h"

QStringList ParamHandler::GetRequiredParams()
{
	QStringList allowedParams;

	allowedParams.append("--active-lane");
	allowedParams.append("--desktop-port");
	allowedParams.append("--webroot-dir");

	return allowedParams;
}

QStringList ParamHandler::GetOptionalParams()
{
	QStringList optionalParams;

	optionalParams.append("--yes");
	optionalParams.append("--verbose");

	return optionalParams;
}

StringPairs ParamHandler::GetDefaultParams()
{
	StringPairs params;

	params["--desktop-port"]	= "8080";
	params["--webroot-dir"]		= "c:\\csr\\exe\\others\\WebUI\\";

	return params;
}

StringPairs ParamHandler::ParseParams(QStringList arguments)
{
	QStringList requiredParams = ParamHandler::GetRequiredParams();
	QStringList optionalParams = ParamHandler::GetOptionalParams();
	StringPairs params = ParamHandler::GetDefaultParams();

	for (int i = 1; i < arguments.size(); i++)
	{
		int index = arguments[i].indexOf("=");

		QString paramName = arguments[i].left(index);
		QString paramValue = arguments[i].right(arguments[i].length() - index-1);

		if (!requiredParams.contains(paramName) && !optionalParams.contains(paramName))
		{
			std::cout << "Parameter " << paramName.toStdString() << " not recognized, aborting!" << std::endl;
			exit(1);
		}

		// bugfix: sometimes, QT parses cmdline arguments as though
		// they are escaping the quotes instead the backslash..
		if (paramName == "--webroot-dir")
		{
			paramValue = QDir::toNativeSeparators(paramValue);

			while (paramValue.endsWith("\\") || paramValue.endsWith("\""))
			{
				paramValue.chop(1);
			}
		}

		params[paramName] = paramValue;
	}

	return params;
}

bool ParamHandler::CheckParams(StringPairs params)
{
	QStringList requiredParams = ParamHandler::GetRequiredParams();
	QStringListIterator iter(requiredParams);
	bool bRetVal = true;

	while (iter.hasNext())
	{
		QString item = iter.next();

		if (!params.contains(item))
		{
			std::cerr << "ERROR: Parameter " << item.toStdString() << " missing!" << std::endl;
			bRetVal = false;
		}
	}

	QDir webRoot(params["--webroot-dir"]);

	if (!webRoot.exists())
	{
		std::cerr << "ERROR: " << QDir::toNativeSeparators(webRoot.absolutePath()).toStdString() << " does not exist!" << std::endl;
		bRetVal = false;
	}

	return bRetVal;
}

void ParamHandler::ShowHelp()
{
	std::cout << "Configures IHM Web interface and updates desktop interface to enable communication." << std::endl;
	std::cout << "\n\tSyntax: ihmweb-configure.exe [parameters]" << std::endl << std::endl;
	std::cout << "\tParameters:" << std::endl << std::endl;
	std::cout << "\t--active-lane" << std::endl;
	std::cout << "\t\tThe lane to which the web interface will connect." << std::endl;
	std::cout << "\t--desktop-port" << std::endl;
	std::cout << "\t\tPort number of the IHM web server. [default: 8080]" << std::endl;
	std::cout << "\t--webroot-dir" << std::endl;
	std::cout << "\t\tRoot folder of the IHM Web interface. [default: c:\\csr\\exe\\others\\WebUI\\]" << std::endl;

	std::cout << "\t--yes" << std::endl;
	std::cout << "\t\tDo not ask for confirmation." << std::endl;
	std::cout << "\t--verbose" << std::endl;
	std::cout << "\t\tMore console output." << std::endl;
	std::cout << "\t--help" << std::endl;
	std::cout << "\t\tShows this help." << std::endl;
}
