
#ifndef TEST_HELPER_H
#define TEST_HELPER_H


#ifdef __cplusplus
extern "C" {
#endif

	void FillRandomData(void * pData, unsigned int dwSize);
	void SetRandomData(void * pData, unsigned int dwSize, unsigned char *pSource);


#ifdef __cplusplus
}
#endif

#endif