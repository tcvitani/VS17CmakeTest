

#include "MRowsCommand.h"

MRowsCommand::MRowsCommand()
{

}

MRowsCommand::~MRowsCommand()
{
	while (!m_lstRows.isEmpty())
		delete m_lstRows.takeFirst();
}



MRowsCommand& MRowsCommand::operator=(const MRowsCommand& right)
{
	m_uiCommandTimestamp = right.m_uiCommandTimestamp;
	m_eCmd = right.m_eCmd;

	MTableRow *pCopiedRow;
	
	while (!m_lstRows.isEmpty())
		delete m_lstRows.takeFirst();

	for (int i=0; i< right.m_lstRows.count(); i++)
	{
		pCopiedRow = new MTableRow();
		*pCopiedRow = *((MTableRow*)right.m_lstRows.at(i));
		m_lstRows.append(pCopiedRow);
	}
	


	return *this;
}


bool MRowsCommand::operator==(const MRowsCommand& right)
{
	bool bRetVal = false;

	if(	m_uiCommandTimestamp == right.m_uiCommandTimestamp 
			&& m_eCmd == right.m_eCmd 
			&& m_lstRows.count() == right.m_lstRows.count())
	{
		bRetVal = true;
		bool bEqual = true;
		MTableRow* pCurrentLeft;
		MTableRow* pCurrentRight;

		for (int i=0; i < right.m_lstRows.count(); i++)
		{
			pCurrentLeft = m_lstRows.at(i);
			pCurrentRight = right.m_lstRows.at(i);

			if(pCurrentLeft!=NULL && pCurrentRight!=NULL)
				bEqual = bEqual && (*pCurrentLeft == *pCurrentRight);
			else
				bEqual = false;

			if(!bEqual)
			{
				bRetVal = false;
				break;
			}
		}

	}
	
	return bRetVal;
}	
