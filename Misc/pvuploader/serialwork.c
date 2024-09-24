/*
 * serialwork.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/time.h>
#include <unistd.h>

#include "pvuploader.h"
#include "serialwork.h"

PVUPLOADPARAMS * pParams = NULL;

/* buffer for received data */
char m_buffer[1024];
unsigned int m_dwDataCnt;

/* some helper variables for HEX file upload */
int m_TinyPtr;
int m_linecnt;
int m_ackcnt;
unsigned char m_cBlockNr;

/* how many files could not be send (file open error) */
int m_iSkipCnt;

FILE * m_fDebug = NULL;

/* file handle for active file to transfer */
FILE * m_fXFile = NULL;

unsigned int m_dwFileSize;
unsigned int m_dwFilePosition;

/* actual data send */
char m_xbuffer[1024+8];
int m_xbufferlen;

#define SOH 1
#define STX 2
#define EOT 4
#define ACK 6
#define NAK 21
#define CAN 24

/* name of lockfile */
char lfname[80];

/*
 * state of upload process
 */
enum
{
  IDLE,
  WAIT4LINK,
  WAIT4ACK,
  WAIT4CANDSENDBLOCK0,
  WAIT4ACKANDC,
  WAIT4ACKANDSENDBLOCKOREOT,
  WAIT4ACKAFTEREOT,
  STARTTRANSFER,
  WAIT4FINALACK,
  WAIT4LASTWORDS
};

int releaselockfile();

/* signal handler */
static void sig_abort (int iSignal)
{
  releaselockfile();
  printf("received signal %d, aborting\n",iSignal);
  exit(3);
}


void DebugLog(char * msg, int iDumpBuffer)
{
#ifdef WITH_DEBUG_FILE
  int i, k, l;
  char text[200];
  
  if (m_fDebug == NULL)
  {
    if ((m_fDebug = fopen("debug.log","w")) != NULL)
    {
      fwrite("PvUploder.dll debug information\r\n",1,33,m_fDebug);
    }
  }
  
  if (m_fDebug == NULL) return;

  if ((msg != NULL) && (msg[0] != 0))
  {
    fwrite(msg,1,strlen(msg),m_fDebug);
  }

  if (iDumpBuffer == 0) return;
  
  k = 0;

  while (k < m_dwDataCnt)
  {
    l = 0;
    for (i=k;i<m_dwDataCnt;++i)
    {
      sprintf(&text[l],"%02X ",((unsigned char)m_buffer[i]));
      l += 3;
      if ((i % 16) == 15)
      {
        ++i;
        break;
      }
    }

    while (i % 16)
    {
      strcat(text,"   ");
      l += 3;
      ++i;
    }
    
    for (i=k;i<m_dwDataCnt;++i)
    {
      if (((unsigned char)m_buffer[i]) < 0x20)
      {
        text[l] = '.';
      }
      else
      {
        text[l] = m_buffer[i];
      }
      ++l;
      text[l] = 0;

      if ((i % 16) == 15)
      {
        ++i;
        break;
      }
    }
  
    strcat(text,"\r\n");
    fwrite(text,1,strlen(text),m_fDebug);
    
    k += 16;
  }
#endif  
}

/*
 * releaselockfile
 *
 * release (delete) the lockfile
 *
 * returns 1 on success
 *         0 on failure
 */
int releaselockfile()
{
  if (lfname[0] == 0) return 1;
  if (unlink(lfname))
  {
        /* error */
    return 0;
  }

  return 1;
}

/*
 * createlockfile
 *
 * try to create a lockfile for device devicename
 *
 * returns 1 on success
 *         0 on failure
 *         may directly exit the program in case of a serious error
 */
int createlockfile(char * devicename)
{
  char * p;
  char lfcontent[80];
  FILE * f;
  mode_t orgmask;
  
  if (lfname[0]) return 1; /* we have already a lockfile */
  
  if (devicename == NULL) return 0;

  p = strrchr(devicename,'/');
  if (p == NULL)
  {
    p = devicename;
  }
  else
  {
    ++p;
  }
  
  if (strlen(p) > 10) return 0;

  sprintf(lfname,"/var/lock/LCK..%s",p);

      /* check for existing lockfile */
  f = fopen(lfname,"r");

  if (f != NULL)
  {
    fclose(f);
    printf("\nDevice is locked.\nAborting.\n");
    exit(1);
  }
  
      /* create lockfile */
  orgmask = umask(022);

  f = fopen(lfname,"w");

  if (f == NULL)
  {
        /* error: cannot create lockfile */
    printf("\nError while creating lockfile.\nAborting.\n");
    exit(2);
  }
  
  sprintf(lfcontent,"%10ld\n",(long)getpid());
  fwrite(lfcontent,1,strlen(lfcontent),f);
  fclose(f);

  umask(orgmask);

  return 1;
}

/*
 * openport
 *
 * opens a serial line and sets it's parameters to baudrate, 8n1
 *
 * name      name of serial line e.g. "/dev/ttyS0"
 * baudrate  desired baudrate
 * old_fd    if not -1 use this and do not open a new file (change baudraute)
 *
 * returns file descriptor returned from open() call
 *         or old_fd if the baudrate was changed successfully
 *         or -1 in case of any failure
 */
int openport(char * name, int baudrate, int old_fd)
{
  int fd;
  struct termios tty;
  int result;

  if ((old_fd == -1) && (name != NULL))
  {
    fd = open(name, O_RDWR);
  }
  else
  {
    fd = old_fd;
  }
  
  if (fd == -1)
  {
    return -1;
  }

  if (tcgetattr(fd, &tty) != 0)
  {
        /* cannot get attributes */
    close(fd);
    return -1;
  }

  cfmakeraw(&tty);

  result = 0;

  switch (baudrate)
  {
      case 0:
        result = cfsetospeed(&tty,B0);
        break;
        
      case 2400:
        result = cfsetospeed(&tty,B2400);
        result |= cfsetispeed(&tty,B2400);
        break;
        
      case 4800:
        result = cfsetospeed(&tty,B4800);
        result |= cfsetispeed(&tty,B4800);
        break;
        
      case 19200:
        result = cfsetospeed(&tty,B19200);
        result |= cfsetispeed(&tty,B19200);
        break;
        
      case 38400:
        result = cfsetospeed(&tty,B38400);
        result |= cfsetispeed(&tty,B38400);
        break;

      case 57600:
        result = cfsetospeed(&tty,B57600);
        result |= cfsetispeed(&tty,B57600);
        break;

      case 115200:
        result = cfsetospeed(&tty,B115200);
        result |= cfsetispeed(&tty,B115200);
        break;

      case 9600:
      default:
        result = cfsetospeed(&tty,B9600);
        result |= cfsetispeed(&tty,B9600);
        break;
  }

  if (result != 0)
  {
    close(fd);
    return -1;
  }

  result = tcsetattr(fd,TCSANOW,&tty);
  
  if (result != 0)
  {
    close(fd);
    return -1;
  }

  return fd;
}

void WaitMSec(unsigned int dwWait)
{
  struct timeval tv;

  tv.tv_sec = dwWait / 1000;
  tv.tv_usec = (dwWait % 1000) * 1000;

  select(0,NULL,NULL,NULL,&tv);
}

/*
 * Wake up PV by toggling DTR
 */
int WakeUpPV(int fd, int speed)
{
  int i;
  long avail;

  for (i=0;i<11;++i)
  {
        /* clear DTR by setting speed to 0 */
    openport(NULL,0,fd);

        /* 30 ms delay */
    WaitMSec(30);

        /* set DTR by setting speed */
    openport(NULL,speed,fd);
    
        /* 30 ms delay */
    WaitMSec(30);

        /* check if any character is available */
    avail = 0;
    (void) ioctl(fd, FIONREAD, &avail);
    if (avail) break;
  }

  return (avail != 0);
}


int OpenSerialPort(char * pPort, int iWakeUp)
{
  if (pParams == NULL)
  {
    return PVUPLOADERR_NOTSTARTED;
  }
  
  if (pPort != NULL)
  {
    if (pParams->fdSerial != -1)
    {
      close(pParams->fdSerial);
    }

    pParams->fdSerial = openport(pPort,9600,-1);
    
  }
  else
  {
    pParams->fdSerial = -1;
    return PVUPLOADERR_NOPORT;
  }
  
  if ((pParams->fdSerial != -1) && (iWakeUp))
  {
    WakeUpPV(pParams->fdSerial,9600);
  }

  if (pParams->fdSerial != -1)
  {
    return PVUPLOADERR_WAITFORLINK;
  }

  return PVUPLOADERR_NOPORT;
}

int ChangeBaudrate()
{
  if (pParams == NULL)
  {
    return 0;
  }
  
  if (pParams->fdSerial != -1)
  {
    pParams->fdSerial = openport(NULL,pParams->iBaudrate,pParams->fdSerial);
    
    if (pParams->fdSerial != -1)
    {
      return 1;
    }
  }
  
  return 0;
}

long GetFileSize(FILE * f)
{
  long lCurrentPos;
  long lSize;
  
  lCurrentPos = ftell(f);
  
  fseek(f,0,SEEK_END);
  
  lSize = ftell(f);

  fseek(f,lCurrentPos,SEEK_SET);

  return lSize;
}

/*
 * ReceiveSer
 *
 * get some bytes from serial line and handle them according to current state
 */
void ReceiveSer()
{
  char * p = NULL;
  char * q = NULL;
  char linkanswer[] = "BOOTSTRAP0A4E\00600\030000100";
  char ack[] = "ACK\r\n";
  char endrecord[] = ":00000001FF";
  char addrecord[] = ":0400000308000100F0";
  unsigned char cCheckSum;
  int i;
  char c1;
  char * r;
    
  unsigned int count;
  int i1;

  long avail;
  
      /*
       * receive available bytes and add them to m_buffer
       */

  avail = 0;
  (void) ioctl(pParams->fdSerial, FIONREAD, &avail);

  if (avail)
  {
    unsigned int readcount = avail;

    if (readcount > sizeof(m_buffer) - m_dwDataCnt - 1)
    {
      readcount = sizeof(m_buffer) - m_dwDataCnt - 1;
    }

    {
      char msg[80];
      sprintf(msg,"read additional %08X bytes\r\n",readcount);
      DebugLog(msg,0);
    }


    count = read(pParams->fdSerial,&m_buffer[m_dwDataCnt],readcount);

    if (count == -1)
    {
      return;
    }
      
    m_dwDataCnt += count;
    
    m_buffer[m_dwDataCnt] = 0;
  }

  DebugLog("ReceiveSer\r\n",1);
  
      /*
       * handle incoming data according to current state
       */

  switch (pParams->iState)
  {
      case IDLE:
        DebugLog("IDLE\r\n",0);
        break;
          
      case WAIT4LINK:
        DebugLog("WAIT4LINK\r\n",0);
            /* search for PVPIN signature */
        p = strstr(m_buffer,"PVPIN:");
        if ((p != NULL) && (pParams->pPin != NULL) && (pParams->pPin[0] != 0))
        {
              /* send PIN if available */
          write(pParams->fdSerial,pParams->pPin,
                strlen(pParams->pPin));
              /* remove PVPIN signature */
          *p = 'X';
          break;
        }

            /* serach for link packet of Casio protocol */
        p = strchr(m_buffer,0x18);

        if (p) {
          printf("link packet %s\n",p+1);
        }
        
        if ((p != NULL) && (strcmp(p,"\030000000") == 0))
        {
              /* link packet 00 found */
          switch (pParams->iBaudrate)
          {
              case 300:
                linkanswer[10] = '2';
                break;
                  
              case 600:
                linkanswer[10] = '3';
                break;
                  
              case 1200:
                linkanswer[10] = '4';
                break;
                  
              case 2400:
                linkanswer[10] = '5';
                break;
                  
              case 4800:
                linkanswer[10] = '6';
                break;
                  
              case 9600:
                linkanswer[10] = '7';
                break;
                  
              case 19200:
                linkanswer[10] = '8';
                break;
                  
              case 38400:
                linkanswer[10] = '9';
                break;
                  
              case 57600:
                linkanswer[10] = 'A';
                break;
                  
              case 115200:
                linkanswer[10] = 'B';
                break;

              default:
                linkanswer[10] = '7';
                DebugLog("Error: illegal baudrate\r\n",0);
          }
            
          count = write(pParams->fdSerial,linkanswer,strlen(linkanswer));

          if (count == -1)
          {
            return;
          }
          
          m_buffer[0] = 0;
          m_dwDataCnt = 0;
          m_linecnt = 0;
          m_ackcnt = 0;
          m_TinyPtr = 0;

          WaitMSec(500);
          ChangeBaudrate();
          
          if ((pParams->pHexData != NULL) && (pParams->iHexDataLen != 0))
          {
            pParams->iState = WAIT4ACK;
            pParams->iPercent = 0;

                /* empty rx buffer */
            tcflush(pParams->fdSerial,TCIFLUSH);

                /* wait for incoming data */
            for (i=0;i<25;++i)
            {
              avail = 0;
              (void) ioctl(pParams->fdSerial, FIONREAD, &avail);

              if (avail) break;
              WaitMSec(100);
            }

            if (i < 25)
            {
                  /* success: BootStrap has answered */
              pParams->iQueryResult = PVUPLOADERR_RECEIVER;
            }
            else
            {
                  /* error: bootstrap no installed */
              pParams->iQueryResult = PVUPLOADERR_NOBOOTSTRAP;
              pParams->iState = IDLE;
              close(pParams->fdSerial);
              pParams->fdSerial = -1;
            }
            
          }
          else
          {
            pParams->iState = IDLE;
            close(pParams->fdSerial);
            pParams->fdSerial = -1;
          }
        }
        break;
          
      case WAIT4ACK:
        if (m_TinyPtr < 0)
        {
          break;
        }

        p = strstr(m_buffer,ack);

        if ((p == NULL) && (m_ackcnt == 0)) 
        {
              // wait for initial ACK
          DebugLog("WAIT4ACK wait for initial ACK\r\n",0);
          break;
        }
          
        if (m_ackcnt == 0)
        {
          DebugLog("WAIT4ACK start of receiver upload\r\n",0);
          printf("\nreceiver\n");
        }
          
        if (pParams->pHexData[m_TinyPtr] != 0)
        {
          if ((p) || ((m_linecnt - m_ackcnt) < 3))
          {
            if (p)
            {
              ++m_ackcnt;
              
                  // remove ack from inbuffer
              q = &p[5];
              p = m_buffer;
              m_dwDataCnt = 0;
              while (*q)
              {
                *p++ = *q++;
                ++m_dwDataCnt;
              }
              *p = 0;
            }
              
                /* ACK seen, send next line */
            i1 = m_TinyPtr;
            if ((strncmp(&pParams->pHexData[i1],endrecord,strlen(endrecord)) == 0)
                && (pParams->iHexDataOnly < PVUPLOADHEX_HEXDATA))
            {
                  /* send extra record: startaddress of Add-In */
              write(pParams->fdSerial,addrecord,strlen(addrecord));
            }

                /* search for beginning of next hex data line */
            while (pParams->pHexData[++m_TinyPtr] && (pParams->pHexData[m_TinyPtr] != ':'));
            
            write(pParams->fdSerial,&pParams->pHexData[i1],m_TinyPtr-i1);
                
                /* reflect new position */
            pParams->iPercent = m_TinyPtr * 10000.0 / ((double)(pParams->iHexDataLen));
            printf("\r%d.%02d%% ",pParams->iPercent / 100,pParams->iPercent % 100);
            fflush(stdout);
                
            ++m_linecnt;

            if (pParams->pHexData[m_TinyPtr] == 0)
            {
              if (pParams->iHexDataOnly == PVUPLOADHEX_RECEIVER)
              {
                pParams->iState = WAIT4CANDSENDBLOCK0;
              }
              else
              {
                    /* wait for end of transmission */
                avail = 1;

                while (avail != 0)
                {
                  WaitMSec(100);
        
                  avail = 0;
                  (void) ioctl(pParams->fdSerial, FIONREAD, &avail);
                }

                m_buffer[0] = 0;
                m_dwDataCnt = 0;

                pParams->iQueryResult = PVUPLOADERR_SUCCESSSKIP;
                pParams->iState = IDLE;
                close(pParams->fdSerial);
                pParams->fdSerial = -1;
                break;
              }
            }
          }
          else
          {
                /* wait for more incoming data */
            break;
          }
        }
        
        {
          char msg[80];
          sprintf(msg,"m_TinyPtr %d\r\n",m_TinyPtr);
          DebugLog(msg,0);
        }
        
        if (pParams->iState == WAIT4CANDSENDBLOCK0)
        {
          pParams->iQueryResult = PVUPLOADERR_FILE;
        }
        
        break;

      case WAIT4CANDSENDBLOCK0:
        DebugLog("WAIT4CANDSENDBLOCK0\r\n",0);
            // wait for initial 'C' or NAK
        if ((!m_dwDataCnt) || ((m_buffer[m_dwDataCnt-1] != 'C')
                               && (m_buffer[m_dwDataCnt-1] != NAK)
                               && (m_buffer[m_dwDataCnt-1] != CAN)))
        {
          m_buffer[0] = 0;
          m_dwDataCnt = 0;
          break;
        }
          
        if (m_buffer[m_dwDataCnt-1] == CAN)
        {
          DebugLog("CAN received\r\n",0);
          pParams->iQueryResult = PVUPLOADERR_FILEFAIL;
          pParams->iState = IDLE;
          close(pParams->fdSerial);
          pParams->fdSerial = -1;
        }

        if (m_fXFile != NULL)
        {
          fclose(m_fXFile);
          m_fXFile = NULL;;
        }

        while (m_fXFile == NULL)
        {
          
              /* try next file in filelist */
          if ((pParams->pFilename == NULL)
              || (pParams->pFilename[0] == 0))
          {
            DebugLog("no file to send\r\n",0);
            q = NULL;
            break;
          }
          
          i = pParams->iFileNr;
          q = pParams->pFilename;
          p = strchr(q,'|');

          DebugLog("Filename(s): ",0);
          DebugLog(q,0);
          DebugLog("\r\n",0);
          if (p != NULL)
          {
            DebugLog("Separator found: ",0);
            DebugLog(p,0);
            DebugLog("\r\n",0);
          }
          
          while ((i != 0) && (p != NULL))
          {
            q = ++p;
            p = strchr(p,'|');
            --i;
          }

          if (p == NULL)
          {
            p = q;
            p += strlen(q);
          }

          DebugLog("After search Filename(s): ",0);
          DebugLog(q,0);
          DebugLog("\r\n",0);
          if (p != NULL)
          {
            DebugLog("Separator found: ",0);
            DebugLog(p,0);
            DebugLog("\r\n",0);
          }
          
          if (i != 0)
          {
                /* no more files */
            DebugLog("no more files to send\r\n",0);
            q = NULL;
            break;
          }

              /* test for doublequote at beginning of filename */
          if (*q == '"')
          {
            ++q;
          }

              /* test for doublequote at end of filename */
          if (p != q)
          {
            --p;
            if (*p != '"') ++p;
          }

          c1 = *p;
          *p = 0;

          DebugLog("Try to open ",0);
          DebugLog(q,0);
          DebugLog("\r\n",0);

              /* try to open file */
          m_fXFile = fopen(q,"r");

          if (m_fXFile == NULL)
          {
                /* count files that could not be opened */
            ++m_iSkipCnt;
          }
          
          *p = c1;
          
              /* increase number of current processed file */
          ++(pParams->iFileNr);
        }

        if (q) printf("\r100.00%%\n%s\n",q);
        
        m_cBlockNr = 0;
        cCheckSum = 0;
        memset(m_xbuffer,0,sizeof(m_xbuffer));
        pParams->iPercent = 0;

        m_xbuffer[0] = SOH;
        m_xbuffer[1] = m_cBlockNr;
        m_xbuffer[2] = -m_cBlockNr - 1;
        ++m_cBlockNr;
        
        if ((m_fXFile == NULL)
            || (q == NULL))
        {
          printf("\r100.00%%");
          
              // send last record with empty filename
          DebugLog("send last record with empty filename\r\n",0);
          for (i=3;i<131;++i)
          {
            m_xbuffer[i] = 0;
          }

          m_xbuffer[i++] = cCheckSum;
          
          pParams->iState = WAIT4FINALACK;
        }
        else
        {
              /* separate filename from path */
          c1 = *p;
          *p = 0;
          
          r = strrchr(q,'/');

          if (r != NULL)
          {
            q = ++r;
          }
          
          if (strlen(q) > 15)
          {
                // reduce filename
            *p = c1;
            p = q + 15;
            c1 = *p;
            *p = 0;
          }
          
          
          DebugLog("send record with filename\r\n",0);
          strcat((char*)&m_xbuffer[3],q);
          
          *p = c1;
          
          sprintf((char*)&m_xbuffer[strlen((char*)&m_xbuffer[3])+4],"%ld",GetFileSize(m_fXFile));

          if (pParams->iAppend == 0)
          {
            strcat((char*)&m_xbuffer[strlen((char*)&m_xbuffer[3])+4],
                   " 0 0 0 1"); // mark as file to delete
          }
            
          for (i=3;i<131;++i)
          {
            cCheckSum += m_xbuffer[i];
          }

          m_xbuffer[i++] = cCheckSum;
            
          pParams->iState = WAIT4ACKANDC;

          m_dwFileSize = GetFileSize(m_fXFile);
          m_dwFilePosition = 0;
        }
            
        m_xbufferlen = i;

        count = write(pParams->fdSerial,m_xbuffer,m_xbufferlen);
          
        m_buffer[0] = 0;
        m_dwDataCnt = 0;

        break;
          
      case WAIT4ACKANDC:
        DebugLog("WAIT4ACKANDC\r\n",0);
        while (m_dwDataCnt)
        {
          if ((m_buffer[0] != 'C')
              && (m_buffer[0] != ACK)
              && (m_buffer[0] != NAK)
              && (m_buffer[0] != CAN))
          {
                // remove first char from inbuffer
            q = &m_buffer[1];
            p = m_buffer;
            m_dwDataCnt = 0;
            while (*q)
            {
              *p++ = *q++;
              ++m_dwDataCnt;
            }
            *p = 0;
          }
          else
          {
            break;
          }
        }

        if (m_dwDataCnt == 0) break;
        
        if (m_buffer[0] == NAK)
        {
              // resend last block
          count = write(pParams->fdSerial,m_xbuffer,m_xbufferlen);

          m_buffer[0] = 0;
          m_dwDataCnt = 0;
          break;
        }
        else if (m_buffer[0] == CAN)
        {
          pParams->iQueryResult = PVUPLOADERR_FILEFAIL;
          pParams->iState = IDLE;
          close(pParams->fdSerial);
          pParams->fdSerial = -1;
          break;
        }
        else
        {
          m_buffer[0] = ACK;
          m_buffer[1] = 0;
          m_dwDataCnt = 1;
          pParams->iState = WAIT4ACKANDSENDBLOCKOREOT;
        }

            /* fall through to WAIT4ACKANDSENDBLOCKOREOT */
          
      case WAIT4ACKANDSENDBLOCKOREOT:
        DebugLog("WAIT4ACKANDSENDBLOCKOREOT\r\n",0);
            // wait for ACK (ignores 'C' before packet #1)
        while (m_dwDataCnt)
        {
          if ((m_buffer[0] != ACK)
              && (m_buffer[0] != NAK)
              && (m_buffer[0] != CAN))
          {
                // remove first char from inbuffer
            q = &m_buffer[1];
            p = m_buffer;
            m_dwDataCnt = 0;
            while (*q)
            {
              *p++ = *q++;
              ++m_dwDataCnt;
            }
            *p = 0;
          }
          else
          {
            break;
          }
        }

        if (m_dwDataCnt == 0) break;
        
        if (m_buffer[m_dwDataCnt-1] == NAK)
        {
              // resend last block
          count = write(pParams->fdSerial,m_xbuffer,m_xbufferlen);
          m_buffer[0] = 0;
          m_dwDataCnt = 0;
          break;
        }
        else if (m_buffer[m_dwDataCnt-1] == CAN)
        {
          pParams->iState = IDLE;
          close(pParams->fdSerial);
          pParams->fdSerial = -1;
          break;
        }

        m_xbuffer[0] = STX;
        m_xbuffer[1] = m_cBlockNr;
        m_xbuffer[2] = -m_cBlockNr - 1;
        ++m_cBlockNr;
          
        count = fread(&m_xbuffer[3],1,1024,m_fXFile);
            /* reflect new position */
        m_dwFilePosition += count;
        pParams->iPercent = (10000.0 * (double)m_dwFilePosition / (double)m_dwFileSize);

        printf("\r%d.%02d%% ",pParams->iPercent / 100,pParams->iPercent % 100);
        fflush(stdout);
        
        if (count == 0)
        {
              // send EOT
          count = write(pParams->fdSerial,"\004",1);

          pParams->iState = WAIT4ACKAFTEREOT;
          break;
        }
          
        for (i=3+count;i<1024+3;++i)
        {
          m_xbuffer[i] = '\032'; // ^Z
        }
          
        cCheckSum = 0;
          
        for (i=3;i<1024+3;++i)
        {
          cCheckSum += m_xbuffer[i];
        }

        m_xbuffer[i++] = cCheckSum;
                  
        m_xbufferlen = i;
        count = write(pParams->fdSerial,m_xbuffer,i);

        m_buffer[0] = 0;
        m_dwDataCnt = 0;
        break;

          
      case WAIT4FINALACK:
        DebugLog("WAIT4FINALACK\r\n",0);
        p = strstr(m_buffer,"\006"); // ACK
            // wait for ACK of last record
        if ((m_dwDataCnt) && (p == NULL)
            && (m_buffer[m_dwDataCnt-1] != NAK)
            && (m_buffer[m_dwDataCnt-1] != CAN))
        {
          m_buffer[0] = 0;
          m_dwDataCnt = 0;
          break;
        }

        if (p == NULL) break;
        
        pParams->iState = WAIT4LASTWORDS;

            /* fall through to WAIT4LASTWORDS */

      case WAIT4LASTWORDS:
            /* give PV time to send more characters */
        WaitMSec(200);

        avail = 0;
        (void) ioctl(pParams->fdSerial, FIONREAD, &avail);
        
        if (avail != 0) break; /* re-loop */

            /* no more characters -> end of transmission */
        if (m_iSkipCnt == 0)
        {
          pParams->iQueryResult = PVUPLOADERR_SUCCESS;
        }
        else
        {
          pParams->iQueryResult = PVUPLOADERR_SUCCESSSKIP;
        }
        
        if (m_dwDataCnt)
        {
          p = strrchr(m_buffer,'\n');
          if (p != NULL)
          {
            *++p = 0;
          }
        }

        DebugLog("LastWords\r\n",1);
        
/*          m_buffer[0] = 0; */
/*          m_dwDataCnt = 0; */
        pParams->iState = IDLE;
        close(pParams->fdSerial);
        pParams->fdSerial = -1;
        break;
          
      case WAIT4ACKAFTEREOT:
        DebugLog("WAIT4ACKAFTEREOT\r\n",0);
            // wait for ACK or NAK
        if ((!m_dwDataCnt) || ((m_buffer[m_dwDataCnt-1] != ACK)
                               && (m_buffer[m_dwDataCnt-1] != NAK)
                               && (m_buffer[m_dwDataCnt-1] != CAN)))
        {
          m_buffer[0] = 0;
          m_dwDataCnt = 0;
          break;
        }

        if (m_buffer[m_dwDataCnt-1] == NAK)
        {
              // re-send EOT
          count = write(pParams->fdSerial,"\004",1);
          break;
        }
        else if (m_buffer[m_dwDataCnt-1] == CAN)
        {
          pParams->iState = IDLE;
          close(pParams->fdSerial);
          pParams->fdSerial = -1;
          break;
        }
          
        m_buffer[0] = 0;
        m_dwDataCnt = 0;
          
        pParams->iState = WAIT4CANDSENDBLOCK0;
        break;

  }
}

void SerialCleanUp(void)
{
  if (m_fDebug != NULL)
  {
    fclose(m_fDebug);
    m_fDebug = NULL;
  }

  if (m_fXFile != NULL)
  {
    fclose(m_fXFile);
    m_fXFile = NULL;
  }

  if ((pParams != NULL) && (pParams->fdSerial != -1))
  {
    close(pParams->fdSerial);
    pParams->fdSerial = -1;
  }

  if ((pParams != NULL) && (pParams->pHexData != NULL))
  {
        /* release hex data */
    free(pParams->pHexData);
    pParams->pHexData = NULL;
    pParams->iHexDataLen = 0;
  }

}

int SerialWorker(PVUPLOADPARAMS * pParameter)
{
  int dwWaitResult;
  int dwReceived;
  long avail;
  
  if (pParameter == NULL)
  {
    return 1;
  }
  
  m_fDebug = NULL;
  
  pParams = pParameter;

  if (pParams == NULL)
  {
    return 1;
  }

  lfname[0] = 0;

  createlockfile(pParams->pPort);

  pParams->iQueryResult = OpenSerialPort(pParams->pPort,pParams->iWakeUp);

  if (pParams->fdSerial == -1)
  {
    releaselockfile();
    return 2;
  }
  
  pParams->iState = WAIT4LINK;

  m_fXFile = NULL;

  m_TinyPtr = 0;
  m_dwDataCnt = 0;
  m_buffer[0] = 0;
  
  m_iSkipCnt = 0;

      /* install signal handlers */
  (void) signal(SIGINT, sig_abort);
  (void) signal(SIGQUIT, sig_abort);
  (void) signal(SIGTERM, sig_abort);
  
  while (1)
  {
    dwReceived = 0;
    dwWaitResult = 0;
    
    avail = 0;
    (void) ioctl(pParams->fdSerial, FIONREAD, &avail);

    
    if ((avail == 0) /* wait if no incoming data available yet */
        && ((pParams->iState != WAIT4ACK) /* but don't wait in this state */
            || ((m_linecnt - m_ackcnt) < 3))) /* if we are not ahead */
    {
      char msg[80];
/*        DebugLog("WaitCommEvent ",0); */
          /* wait for incoming bytes */
      {
        fd_set fds;
        struct timeval tv;
        int selres;
        
        FD_ZERO(&fds);
        FD_SET(pParams->fdSerial,&fds);

        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        selres = select(pParams->fdSerial+1,&fds,NULL,NULL,&tv);

        if (selres == 0)
        {
              /* timeout */
          dwWaitResult = 0;
        }

        if (selres == -1)
        {
          dwWaitResult = 0;
        }
      }
      
      sprintf(msg,"%08X\r\n",dwWaitResult);
    }
    
    ReceiveSer();

    if (pParams->fdSerial == -1)
    {
      DebugLog("Serial port closed, quit thread\r\n",0);
          /* handle was closed (possibly by main thread), we can exit */
      break;
    }
  }

  printf("\n");
  
  releaselockfile();
  
  return 0;
}

void GetLastWords(char * pBuffer, int iBufferLen)
{
  unsigned int nCopyCnt;
  
  if ((pBuffer != NULL) && (iBufferLen > 0))
  {
    if (m_dwDataCnt < 2)
    {
      pBuffer[0] = 0;
      return;
    }
    
    nCopyCnt = iBufferLen;

    if (nCopyCnt > m_dwDataCnt - 1)
    {
      nCopyCnt = m_dwDataCnt - 1;
    }

    memcpy(pBuffer,&m_buffer[1],nCopyCnt);
    pBuffer[nCopyCnt-1] = 0;
  }
}
