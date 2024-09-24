/*
 * serialwork.h
 */

#ifndef serialwork_h_included_12345
#define serialwork_h_included_12345

typedef struct tagPvUploadParams
{
      /* IN these are set before TreadFunc is called */
  char * pPort;
  char * pFilename;
  char * pPin;
  int iWakeUp;
  int iBaudrate;
  int iAppend;
  char * pHexData; /* HEX file to upload */
  int iHexDataLen; /* length of HEX data m_pHexData points to */
  int iHexDataOnly; /* flag whether process should stop
                       after upload of HEX file */
  
      /* OUT these are set during the process and can be queried
         by other threads */
  int iState;
  int fdSerial;
  int iQueryResult;
  int iFileNr;
  int iPercent;
} PVUPLOADPARAMS;

/*
 * SerialWorker
 *
 * entry point
 */
int SerialWorker(PVUPLOADPARAMS * pParameter);

/*
 * GetLastWords
 *
 * copy "last words" from receiver into buffer
 */
void GetLastWords(char * pBuffer, int iBufferLen);

/*
 * SerialCleanUp
 *
 * clean up
 */
void SerialCleanUp(void);

#endif
