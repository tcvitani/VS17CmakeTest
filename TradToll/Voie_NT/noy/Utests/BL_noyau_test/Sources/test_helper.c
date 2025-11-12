
#include "test_helper.h"
#include <memory.h>

void FillRandomData(void * pData, unsigned int dwSize)
{
	unsigned int i;

	for(i=0;i<dwSize;i++)
	{
		*((unsigned char*)pData + i) = (unsigned char)i%256;
	}
}

void SetRandomData(unsigned char * pData, unsigned int dwSize, unsigned char *pSource)
{
	memcpy(pData,pSource, dwSize);
}