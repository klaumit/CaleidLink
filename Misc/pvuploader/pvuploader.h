/*
 * pvuploader.h
 *
 * an example implementation of a BootStrap client
 *
 * Author: J. Steingraeber
 *
 * released to the PUBLIC DOMAIN
 */

#ifndef pvuploader_h_included_12345
#define pvuploader_h_included_12345

/*
 * Error resp. status return values
 *
 * These are returned from PvUploaderQuery() to signal the current state
 * of the upload process.
 *
 * PVUPLOADERR_INITINPROGRESS, PVUPLOADERR_WAITFORLINK, PVUPLOADERR_RECEIVER,
 * and PVUPLOADERR_FILE so signal that the serial port is opened,
 * all other values do signal that the serial port is closed.
 */
enum PVUPLOADERR
{
  PVUPLOADERR_NOTSTARTED = 0, /* call PvUploaderInit() first */
  PVUPLOADERR_INITINPROGRESS = 1, /* initialization has not finished yet */
  PVUPLOADERR_NOPORT = 2, /* serial port not available */
  PVUPLOADERR_WAITFORLINK = 3, /* PV did not answer yet */
  PVUPLOADERR_NOBOOTSTRAP = 4, /* BootStrap patch is not installed */
  PVUPLOADERR_RECEIVER = 5, /* upload of receiver is in progress */
  PVUPLOADERR_RECEIVERFAIL = 6, /* upload of receiver failed */
  PVUPLOADERR_FILE = 7, /* upload of file is in progress */
  PVUPLOADERR_FILEFAIL = 8, /* upload of (at least one) file failed */
  PVUPLOADERR_SUCCESSSKIP = 9, /* upload of some files succeeded */
  PVUPLOADERR_SUCCESS = 10 /* upload of all files succeeded */
};

enum PVUPLOADHEXDATA
{
  PVUPLOADHEX_RECEIVER = 0, /* upload build in receiver and
                               perform file upload */
  PVUPLOADHEX_ADDIN = 1, /* upload custom hex data and
                            set start record to 0x0800:0x0100 */
  PVUPLOADHEX_HEXDATA = 2 /* upload custom hex data,
                              assume start record included in hex data */
};

/*
 * PvUploaderInit
 *
 * intiate upload of (LZH) file(s) to the PV
 *
 * this automates the whole upload process including
 *
 * - open serial port
 * - wake up PV
 * - trigger BootStrap mechanism
 * - upload receiver hex file
 * - upload lzh file(s)
 * - close serial port
 *
 * Parameters
 *
 * pPort       name of serial device to open, e.g. "COM1:"
 *
 * iBaudrate   baudrate of transmission
 *
 * iWakeUp     set this to 0 to avoid waking up the PV; in this case
 *             you have to start communication by hand (e.g. Start
 *             button); the state returned by PvUploaderQuery() will remain
 *             PVUPLOADERR_WAITFORLINK until communication was started or
 *             aborted by a call to PvUploaderAbort();
 *             set this to 1 to start communication automatically
 *
 * iAppend     setting this to 0 means that existing files on the PV
 *             with the same name as the uploaded file will be deleted
 *             setting this to 1 means to append to existing files
 *
 * pPin        password for PvPIN; PvPIN from version 1.6 on support input
 *             of PIN via serial port
 *
 * pFiles      one or more files to upload to the PV; should contain the
 *             whole path to the file; uses '|' to separate filenames
 *             example:
 *
 *             "c:\my long path\my long filename.lzh"|c:\autexec.bat
 *
 *             will upload two files
 *
 * Return value
 *
 * 0   Some error occured, initialisation could not be started
 * 1   Initialisation was started; call PvUploaderQuery to get the current
 *     state of the upload process
 */
int PvUploaderInit();

/*
 * PvUploaderHexData
 *
 * send custom HEX file to PV
 *
 * this automates the process of uploading a custom HEX file including
 *
 * - open serial port
 * - wake up PV
 * - trigger BootStrap mechanism
 * - upload receiver hex file
 * - close serial port
 *
 * Parameters
 *
 * pPort       name of serial device to open, e.g. "COM1:"
 *
 * iBaudrate   baudrate of transmission
 *
 * iWakeUp     set this to 0 to avoid waking up the PV; in this case
 *             you have to start communication by hand (e.g. Start
 *             button); the state returned by PvUploaderQuery() will remain
 *             PVUPLOADERR_WAITFORLINK until communication was started or
 *             aborted by a call to PvUploaderAbort();
 *             set this to 1 to start communication automatically
 *
 * pPin        password for PvPIN; PvPIN from version 1.6 on support input
 *             of PIN via serial port
 *
 * pHexData    pointer to data which must be valid HEX data
 *
 * iDataLen    length of HEX data pHexData points to
 *
 * iType       determines if a start record should be inserted before
 *             the end record automatically, pointing to 0x0800:0x0100;
 *             PVUPLOADHEX_ADDIN will insert the extra record
 *             PVUPLOADHEX_HEXDATA will leave the data alone
 *
 * Return value
 *
 * 0   Some error occured, initialisation could not be started
 * 1   Initialisation was started; call PvUploaderQuery to get the current
 *     state of the upload process
 */
int PvUploaderHexData(char * pPort, int iBaudrate, int iWakeUp,
                      char * pPin, char * pHexData, int iDataLen,
                      int iType);

/*
 * PvUploaderQuery
 *
 * get the actual state of the uploading process
 *
 * Parameters
 *
 * pFile     pointer to an int variable to receive the number of the
 *           current file in transfer; is 0 for transfer of receiver,
 *           1 for the first file etc.
 *
 * pPercent  pointer to an int varaible to receive the percentage of
 *           current upload; the variable will get a value between 0 and
 *           10000, divide this by 100 to get the actual percent value
 *
 * Return value
 *
 * PVUPLOADERR_NOTSTARTED
 *     PvUploaderInit was not called
 * PVUPLOADERR_INITINPROGRESS
 *     initialisation has not finished yet
 * PVUPLOADERR_NOPORT
 *     opening the serial port failed
 * PVUPLOADERR_WAITFORLINK
 *     serial port was opened and PvUploader is still waiting for some
 *     response from the PV; this may indicate that wake up of PV was skipped
 *     or PvPIN is in the way or the PV is not placed in the cradle
 * PVUPLOADERR_NOBOOTSTRAP
 *     BootStrap was not installed on PV; PvUploader requires the BootStrap
 *     patch to be installed
 * PVUPLOADERR_RECEIVER
 *     upload of receiver was started (and is still in progress)
 * PVUPLOADERR_RECEIVERFAIL
 *     upload of receiver ended with an error; no files were uploaded
 * PVUPLOADERR_FILE
 *     upload of receiver is finished, upload of file(s) is in progress
 * PVUPLOADERR_FILEFAIL
 *     upload of file(s) ended with an error; in a multi file upload some
 *     of the files may be uploaded successfully but there is no clean way
 *     to determine which ones :-(
 * PVUPLOADERR_SUCCESSSKIP
 *     upload of file(s) ended with success but at least one file
 *     could not be opened and was not sent; PvUploaderGetLastWords()
 *     shall indicate which files were transfered
 * PVUPLOADERR_SUCCESS
 *     upload of file(s) ended with success, port is closed
 */
int PvUploaderQuery(int * pFile, int * pPercent);

/*
 * PvUploaderGetLastWords
 *
 * receive the "last words" from the receiver
 *
 * the receiver sends some textual info at the end of each upload
 *
 * Parameters
 *
 * pBuffer      pointer to the buffer that will receive the text
 * iBufferLen   size of the buffer; it is guaranteed that no bytes
 *              will be written behind the end of the buffer and that
 *              the buffer will be terminated by a 0
 */
void PvUploaderGetLastWords(char * pBuffer, int iBufferLen);

/*
 * PvUploaderAbort
 *
 * abort upload of file
 *
 * this will close the serial port immediately
 *
 * Return value
 *
 * PVUPLOADERR_NOTSTARTED
 *     PvUploaderInit was not called (so there is nothing to abort)
 * PVUPLOADERR_WAITFORLINK
 *     serial port was opened and PvUploader is still waiting for some
 *     response from the PV
 * PVUPLOADERR_RECEIVERFAIL
 *     upload of receiver ended with an error (abort takes place during
 *     receiver upload)
 * PVUPLOADERR_FILEFAIL
 *     upload of file ended with an error (abort takes place during file
 *     upload)
 */
int PvUploaderAbort(void);


/*
 * typedefs for dynamic (runtime) dll loading
 *
 * use these at runtime like so
 *
 
  HINSTANCE hPvUploader;
  PVUPLOADERINIT PvUploaderInitCall;

  hPvUploader = LoadLibrary("PvUploader.dll");

  if (hPvUploader != NULL)
  {
    PvUploaderInitCall = (PVUPLOADERINIT) GetProcAddress(hPvUploader,"PvUploaderInit");
  }
  else
  {
    PvUploaderInitCall = NULL;
  }

  if (PvUploaderInitCall != NULL)
  {
    PvUploaderInitCall("COM1:",57600,1,1,NULL,"test.lzh");
  }

 *
 */

#endif
