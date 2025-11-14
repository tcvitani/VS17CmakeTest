#ifndef TEMPLATE_HANDLER
#define TEMPLATE_HANDLER

#include <QString>
#include <QHash>

typedef QHash<QString, QString> StringPairs;

class TemplateHandler
{
	private:
		static const QString		platformTemplate;

	public:
		static QString		BuildIHMWebPlatformConfig(StringPairs params);
		static StringPairs	BuildIHMWebDesktopConfig(StringPairs params);
};

#endif // #ifndef TEMPLATE_HANDLER
