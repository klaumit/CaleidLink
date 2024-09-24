/*
 * pvuploader.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "receiver_hex.h"
#include "serialwork.h"
#include "pvuploader.h"


/*
 * global variables
 */

PVUPLOADPARAMS m_params;


int LoadHexdata()
{
  int dwLen;
  
  if (m_params.pHexData != NULL)
  {
        /* data is already loaded, return success */
    return 1;
  }
  
      // copy data to m_pHexData
  dwLen = sizeof(receiver_hex);
  if (dwLen > 0)
  {
    m_params.pHexData = (char *)malloc(dwLen);
    if (m_params.pHexData != NULL)
    {
      memcpy(m_params.pHexData,receiver_hex,dwLen);
      m_params.iHexDataLen = dwLen;
      return 1;
    }
  }

      // out of memory or other failure
  return 0;
}

int PvUploaderInit()
{
  if (LoadHexdata() == 0)
  {
        /* hex data could not be loaded */
    return 0;
  }
  
  m_params.iHexDataOnly = PVUPLOADHEX_RECEIVER;
  
  m_params.iQueryResult = PVUPLOADERR_INITINPROGRESS;

  SerialWorker(&m_params);

  SerialCleanUp();
  
  return 1;
}

int PvUploaderHexData(char * pPort, int iBaudrate, int iWakeUp,
                      char * pPin, char * pHexData, int iDataLen,
                      int iType)
{
  if ((iType != PVUPLOADHEX_ADDIN)
      && (iType != PVUPLOADHEX_HEXDATA))
  {
    return 0;
  }
  
  if (m_params.pHexData != NULL)
  {
    free(m_params.pHexData);
  }
  
      // copy data to m_pHexData
  m_params.pHexData = (char *)malloc(iDataLen);
  if (m_params.pHexData != NULL)
  {
    memcpy(m_params.pHexData,pHexData,iDataLen);
    m_params.iHexDataLen = iDataLen;
  }
  else
  {
    return 0;
  }
  
  m_params.iWakeUp = iWakeUp;
  
  m_params.iBaudrate = iBaudrate;

  m_params.iAppend = 0;

  m_params.iHexDataOnly = iType;
  
  if (pPin == NULL)
  {
    m_params.pPin = NULL;
  }
  else
  {
    m_params.pPin = (char *)malloc(strlen(pPin)+2);
    if (m_params.pPin != NULL)
    {
      strcpy(m_params.pPin,pPin);
      strcat(m_params.pPin,"\r");
    }
  }
  
  if (pPort == NULL)
  {
    m_params.pPort = NULL;
  }
  else
  {
    m_params.pPort = (char *)malloc(strlen(pPort)+1);
    if (m_params.pPort != NULL)
    {
      strcpy(m_params.pPort,pPort);
    }
  }
  
  m_params.pFilename = NULL;

  m_params.iQueryResult = PVUPLOADERR_INITINPROGRESS;

  SerialWorker(&m_params);
  
  SerialCleanUp();
  
  return 1;
}

int PvUploaderQuery(int * pFile, int * pPercent)
{
  if (pFile != NULL)
  {
    *pFile = m_params.iFileNr;
  }
  
  if (pPercent != NULL)
  {
    *pPercent = m_params.iPercent;
  }
  
  return m_params.iQueryResult;
}

void PvUploaderGetLastWords(char * pBuffer, int iBufferLen)
{
  if ((pBuffer != NULL) && (iBufferLen))
  {
    GetLastWords(pBuffer,iBufferLen);
  }
}

int PvUploaderAbort()
{
  if (m_params.iQueryResult < PVUPLOADERR_RECEIVERFAIL)
  {
    m_params.iQueryResult = PVUPLOADERR_RECEIVERFAIL;
  }
  else if (m_params.iQueryResult < PVUPLOADERR_FILEFAIL)
  {
    m_params.iQueryResult = PVUPLOADERR_FILEFAIL;
  }
  
  return m_params.iQueryResult;
}

void usage()
{
  printf("\
pvuploader\n\
\n\
usage: pvuploader --port=PORT [--baud=BAUDRATE] [--wakeup] [--pin=PIN] [--append] file1 file2 ...\n\
\n\
       PORT name of serial port, e.g. /dev/ttyS1\n\
       BAUDRATE one of 2400, 4800, 9600 (default), 19200, 38400, 57600, 115200\n\
       --wakeup if given tries to wake up the PV\n\
       --append if given appends to archives of same name\n\
       PIN to unlock PvPIN (requires PvPIN 1.6 and above to work)\n\
       file1 file2 ... filenames of archives to upload\n\
\n\
");
}

int main(int argc, char * argv[])
{
  int i, n;
  char * filenames = malloc(1000);
  char * p;
  
  if (filenames == NULL)
  {
    printf("error: out of memory\n");
    return 1;
  }

  filenames[0] = 0;
  
  m_params.pFilename = NULL;
  m_params.pPin = NULL;
  m_params.iBaudrate = 9600;
  m_params.iQueryResult = 0;
  m_params.iFileNr = 0;
  m_params.iPercent = 0;
  m_params.pHexData = NULL;
  m_params.iHexDataLen = 0;
  m_params.iHexDataOnly = PVUPLOADHEX_RECEIVER;

      /* process arguments */
  if (argc < 2)
  {
    usage();
  }

  for (i=1;i<argc;++i)
  {
    p = argv[i];
    
    if (strstr(p,"--port=") == p)
    {
      p += 7;
      if (*p && ((n = strlen(p))))
      {
        m_params.pPort = malloc(n+1);
        if (m_params.pPort != NULL)
        {
          strcpy(m_params.pPort,p);
        }
      }
    }
    else if (strstr(p,"--baud=") == p)
    {
      p += 7;
      if (*p && ((sscanf(p,"%d",&n) == 1)))
      {
        m_params.iBaudrate = n;
      }
    }
    else if (strstr(p,"--pin=") == p)
    {
      p += 6;
      if (*p && ((n = strlen(p))))
      {
        m_params.pPin = malloc(n+2);
        if (m_params.pPin != NULL)
        {
          strcpy(m_params.pPin,p);
          strcat(m_params.pPin,"\r");
        }
      }
    }
    else if (strstr(p,"--wakeup") == p)
    {
      m_params.iWakeUp = 1;
    }
    else if (strstr(p,"--append") == p)
    {
      m_params.iAppend = 1;
    }
    else
    {
          /* everything unrecognised is a filename */
      if ((filenames[0]) && (strlen(filenames)+strlen(p)+2 < 1000))
      {
        strcat(filenames,"|");
      }

      if (strlen(filenames)+strlen(p)+1 < 1000)
      {
        strcat(filenames,p);
      }
    }
  }


  m_params.pFilename = malloc(strlen(filenames)+1);
  if (m_params.pFilename != NULL)
  {
    strcpy(m_params.pFilename,filenames);
  }

  m_params.fdSerial = -1;
  
      /* custom hex data upload is not implemented right now,
         so always start PvUploaderInit */
  PvUploaderInit();

  if (filenames != NULL)
  {
    free(filenames);
    filenames = NULL;
  }

  if (m_params.pFilename != NULL)
  {
    free(m_params.pFilename);
    m_params.pFilename = NULL;
  }
  
  if (m_params.pPin != NULL)
  {
    free(m_params.pPin);
    m_params.pPin = NULL;
  }
  
  if (m_params.pHexData != NULL)
  {
    free(m_params.pHexData);
    m_params.pHexData = NULL;
  }
  
  return 0;
}
