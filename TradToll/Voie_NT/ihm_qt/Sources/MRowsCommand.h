
#ifndef ROWSCOMMAND_H
#define ROWSCOMMAND_H

#include <QString>
#include "MTableModelRow.h"

extern "C" {
	#include "ihm.h"
}

class MRowsCommand 
{
public:
	MRowsCommand();
	~MRowsCommand();
	MRowsCommand& operator=(const MRowsCommand& right);
	bool operator==(const MRowsCommand& right);	

	quint64 m_uiCommandTimestamp;

	QList <MTableRow *> m_lstRows;

	enum_aff_table_command m_eCmd;
};


#endif
