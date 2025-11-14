#ifndef PARAM_HANDLER
#define PARAM_HANDLER

#include <QStringList>
#include <QHash>

#include <iostream>

typedef QHash<QString, QString> StringPairs;

class ParamHandler
{
	public:
		static QStringList GetRequiredParams();
		static QStringList GetOptionalParams();
		static StringPairs GetDefaultParams();
		static StringPairs ParseParams(QStringList arguments);
		static bool CheckParams(StringPairs params);
		static void ShowHelp();
};

#endif // #ifndef PARAM_HANDLER
