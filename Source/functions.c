#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <limits.h>

#include "lbltp.h"

#if SYSTEM != MSVC
#ifdef __MWERKS__
/* skip the redefinitioin of user_from_uid in pwd.h */
#define _XOPEN_SOURCE
#endif
#include <pwd.h>
#ifdef __MWERKS__
#undef _XOPEN_SOURCE
#endif
#include <unistd.h>
#else 
#include <io.h>
#endif

#include "vars.h"
#include "functions.h"
#include "fsrtns.h"
#include "linemode.h"
#include "machine.h"

#if SYSTEM == OS4 && ! defined SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
#define ANSI '\1'
#define IBM  '\0'
#define FILENAME 0x01 
#define DIRECTORY 0x02
#define TAPE  0x04
#define TERMINAL 0x80
#define UNDEFINED 0xFF
#define INSTALATION_NAME "ITD ANN ARBOR"
#define HEADER_RTN -1
#define vb_startblk(span)                                                      \
  memset(device->buffer,'\0',8);           /* zap initial  BDW & RDW */        \
  device->seg_start=&device->buffer[4];    /* point to first rec */            \
  dw=4;                                                                        \
  if(span != 1) device->arec_len=0;        /* reset actual rec len */          \
  memcpy(device->seg_start,&dw,2);         /* update RDW */                    \
  dw=device->offset = 8;                   /* update BDW and offset */         \
  memcpy(device->buffer,&dw,2);                                                \
  device->rem_len = device->block_len-8;   /* update reamaing length block */
#define db_startblk(span)                                                      \
  memset(device->buffer,'^',device->blkpfx);   /* zap BDW */                   \
  len=4+span;                                  /* length of RDW */             \
  device->offset=device->blkpfx+len;           /* offset to start of rec */    \
  if (device->blkpfxl==1)                      /* recording BDW */             \
    memcpy(device->buffer,&device->offset,4);  /* yep - update */              \
  if (span == 1)                               /* spanned format? */           \
   memset(&device->buffer[device->blkpfx],'0',1); /* yes - init span byte */   \
  memcpy(&device->buffer[device->blkpfx+span],&len,4); /* update RDW */        \
  device->rem_len=device->block_len-device->offset; /* reset remaning len blk*/\
  if (span !=1) device->arec_len=0;            /* reset actual rec len */      \
  device->seg_start=&device->buffer[device->blkpfx]; /* point to start of rec*/
#define NAMELISTLEN 5
#define NAMESUBLEN 5
struct stream_ctl
 {
  int rem_len,offset;
  unsigned char buffer[512];
 } ostream;

/* Internal methods */
static int rd_nohdr(struct deviceinfo *,int);
static int rd_ibmhdr(struct deviceinfo *,int);
static int rd_ansihdr(struct deviceinfo *,int);
static int rd_toshdr(struct deviceinfo *,int);
static WRITE_RTN wrt_nohdr(struct deviceinfo *tape);
static int wrt_notlr(struct deviceinfo *tape);
static WRITE_RTN  wrt_ibmhdr(struct deviceinfo *);
static int wrt_ibmtlr(struct deviceinfo *);
static WRITE_RTN  wrt_ansihdr(struct deviceinfo *);
static int wrt_ansitlr(struct deviceinfo *);
static WRITE_RTN  wrt_toshdr(struct deviceinfo *);
static int wrt_tostlr(struct deviceinfo *);
static char *validate_format(struct deviceinfo *);
static unsigned char *vbs_read(struct deviceinfo *,struct buf_ctl *);
static unsigned char *vb_read(struct deviceinfo *,struct buf_ctl *);
static unsigned char *fb_read(struct deviceinfo *,struct buf_ctl *);
static unsigned char *u_read(struct deviceinfo *,struct buf_ctl *);
static unsigned char *ru_read(struct deviceinfo *,struct buf_ctl *);
static unsigned char *dbs_read(struct deviceinfo *,struct buf_ctl *);
static unsigned char *db_read(struct deviceinfo *,struct buf_ctl *);
static unsigned char *unix_read(struct deviceinfo *, struct buf_ctl *);
static int vbs_write(struct deviceinfo *, const unsigned char *, unsigned int);
static int vb_write(struct deviceinfo *, const unsigned char *, unsigned int);
static int fb_write(struct deviceinfo *, const unsigned char *, unsigned int);
static int u_write(struct deviceinfo *, const unsigned char *, unsigned int);
static int dbs_write(struct deviceinfo *, const unsigned char *, unsigned int);
static int db_write(struct deviceinfo *,const unsigned char *, unsigned int);
static int unix_write(struct deviceinfo *, const unsigned char *, unsigned int);
static int tflush(struct deviceinfo *, int);
static char *getpath(char *);
static unsigned char *crefname(unsigned char *);
static unsigned char *verfname(unsigned char *,struct buf_ctl *,int);
static int getfname(struct deviceinfo *);

/* Static data */
static unsigned char path_name[4096];
static char fname[258];
static char julian_date[20];
static int cancel_command=OFF;

#if SYSTEM == OS4
typedef
struct 
 {
  int quot;
  int rem;
 } div_t;
div_t div(int num, int div)
 {
  div_t d;
  d.quot = num/div;
  d.rem = num%div;
  return d;
 } 
#endif
 
void myattn(int signum)
 {
  if (tapeo.need_trailer==ON)
   wrt_tlr(&tapeo);
  (tapeo.old_handler)(signum);
 }


int wrt_tlr(struct deviceinfo *odevice) 
 {
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
  void (*old_handler)(int);
#endif

  tflush(odevice,1);                      /* flush the current file */
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
  old_handler = signal(SIGINT,SIG_IGN);
#else
  sighold(SIGINT);
#endif
  tapeweof(odevice,1);                    /* write a tape mark */
  odevice->format[0]=toupper(odevice->format[0]);
  odevice->leot=ON;
  if (odevice->drive_type != DEV_QIC)     /* if not a QIC */
   {
    int i;

    (odevice->wrt_tlr_rtn)(odevice);      /* write out trailer label */
    for (i=0;i<3;i++)                     /* terminate tape - just incase */
     {
      if (odevice->realtape == YES)
       {   
        tpwrite( odevice,(unsigned char*)"                     ",20);
        tapebsr(odevice,1); 
       }
      tapeweof(odevice,1);
     }
    tapebsf(odevice,3);                  /* back up over extra TM's */
   }
  odevice->need_trailer=OFF;             /* trailer safely written */
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
  signal(SIGINT,old_handler);
#else
  sigrelse(SIGINT);
#endif
  return 0;
 }


int tpopen(int open_type, unsigned char * device_name, int is_VLO_tape, int not_fs)
  {
   char vol1[] = "VOL1";
   char *tpath_name;
   char *type;
   unsigned char *status;
   unsigned char buf[4];
   int len,taped = -1,i;
   struct buf_ctl tbuf;
   struct deviceinfo *tape;
  
   memset(&tbuf, 0, sizeof(tbuf));
   
   if (open_type == INPUT) tape=&tapei;      /* point to proper control block */
   else if (open_type == OUTPUT) 
    {
     tape=&tapeo;
     if (tape->label <= 0) open_type=INPUT;
    }
   else
    {
     fprintf(stderr,"Internal error TPOPEN\n"); exit(2);
     } /*should not occ*/
   tbuf.warn=OFF;                            /* don't sent warnings */      
   tpath_name=getpath((char *) device_name); /* get full path name */
   status=verfname((unsigned char *)tpath_name,&tbuf,open_type); /* get info */
   if (status==NULL)                      /* file/device exist? */
    {
     sprintf(message_area,
             "File `%s' does not exist\n",device_name); /* no - tell user */  
     issue_error_message(message_area);
     return (-1);                            /* return */
    }
   if (tbuf.copy_type!=TAPE && tbuf.copy_type!=FILENAME) /* type we handle? */
    {                                        /* no */
     sprintf(message_area,
             "`%s' must be a file or tape.\n",device_name); /*tell user*/
     issue_error_message(message_area);
     return (-1);                            /* return */
    } 
   if (tape->name != NULL)                   /* already opened? */
    {
     if (strcmp(tape->name,(char *)device_name)==0)/* yep - trying to reopen? */
      return(0);                            /* yep - nothing to do - just rtn */
     else                                    /* else -*/
      {
       if (tape==&tapei) type="INPUT";       /* tell user to close old device */
       else type="OUTPUT";
        {
         sprintf(message_area,
               "%s is the current %s device and must be closed first\n",
               tape->name,type);
         issue_error_message(message_area);
        }
       return (-1);                          /* and return */
      } 
    } 
   if (tape==&tapei)                         /* opening for input? */
    {
     if (tapeo.name != NULL &&
         strcmp(tapeo.name,(char *)device_name)==0) /*yep - already opened for output?*/
      {
       sprintf(message_area,
               "%s already opened as output\n",device_name); /* yep - */
       issue_error_message(message_area);
       return (-1);                          /* user and return */
      } 
     taped=open(tpath_name,O_RDONLY|O_BINARY); /* try to open for input */
    }
   else if (tape==&tapeo)                    /* opening for output? */
    {
     if (tapei.name != NULL &&
         strcmp(tapei.name,(char*)device_name)==0) /* yep - already opened for input?*/
      {
       sprintf(message_area,
               "%s already opened as input\n",device_name); /* yep - */
       issue_error_message(message_area);
       return(-1);                           /* tell user and return */
      } 
     if (tape->label>0 && tbuf.copy_type==FILENAME)
      /* Labeling a virtual tape, truncate it */
      taped = open(tpath_name, O_RDWR|O_TRUNC|O_BINARY);
     else
      taped=open(tpath_name,O_RDWR|O_BINARY); /* try to open for output */ 
    }
   if (taped < 0)                            /* open sucessfull? */
    {
     if (tape == &tapeo)                     /* no - was this an output? */  
      {
       taped=open(tpath_name,O_RDONLY|O_BINARY); /* yep - opens as input? */ 
       if (taped >= 0)                       /* yep - tell user can't open*/
        {
         issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
         sprintf(message_area,"Could not open '%s' for write\n",device_name);
         issue_error_message(message_area);
         close(taped);                       /* for write - close and */
         return(-1);                         /* return */
        }
      }
     if (tape == &tapeo && tape->label>0 && 
         tbuf.copy_type==FILENAME) /* if labeling and a file */
      taped=open(tpath_name,O_RDWR|O_CREAT|O_BINARY,
#if SYSTEM == MSVC
                  _S_IREAD | _S_IWRITE);
#else
                  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); /* try to create and open */
#endif
     if (taped <= 0)                         /* open still fails? */
      { 
       issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
       sprintf(message_area,
               "Could not open '%s'\n",device_name); /* yep - tell user*/
       issue_error_message(message_area);
       return(-1);                           /* return */
      }
    }
   tape->file=taped;                       /* put file/device name in ctl blk */
   tape->leot=OFF;
   len=strlen((char*)device_name);
   tape->name = (char *) malloc(len+1);
   strcpy(tape->name,(char *)device_name);
   if (tbuf.copy_type == TAPE)             /* opening a real tape? */
    {
     tape->realtape=YES;                   /* yep - mark as real and */
     tape->drive_type=getdrivetype(tape);  /* get some info on the device */
    }
   else     
    {
     tape->realtape=NO;                    /* not real - mark as virtual */
     /* If we're initializing this tape drive_type is already set.  If not
        it will be set below. */
    }
   taperew(tape);                          /* rewind the tape */
   if (tape==&tapeo && tape->label>0)      /* initializing a tape? */
    {
     /* If it's not a real tape we should have a format */
     if (tape->realtape == NO &&
         tape->drive_type != DEV_FAKETAPE &&
         tape->drive_type != DEV_AWSTAPE)
      {
       issue_error_message("Format not set when initializing a simulated tape\n");
       close(taped);                       /* close */
       free(tape->name);                   /* release control block */
       tape->name=NULL; 
       return -1;
      }
     memset(tape->vol1,' ',80);            /* yep - creat blank vol label */ 
     memcpy(&tape->vol1,"VOL1",4);         /* put in the VOL! indentifer */
     len=strlen((char *)tape->volume);     /* fill in the volume name */
     memcpy(&tape->vol1[4],tape->volume,len);
     len=strlen(tape->owner);              /* get length of owner name */
     if (tape->label == IBM_LABEL ||
         tape->label == VLO_LABEL ||
         tape->label == TOS_LABEL)         /* IBM or TOS labeled? */  
      {
       memset(&tape->vol1[10],'0',1);      /* yes - char 0 always here */
       memcpy(&tape->vol1[41],tape->owner,len); /* owner name goes here */
       for (i=0; i<80; i++) tape->vol1[i]=ASCEBC[tape->vol1[i]];/*tran to EBCD*/
      } 
     else if (tape->label == ANSI_LABEL)   /* ANSI labeled? */
      {
       memcpy(&tape->vol1[37],tape->owner,len); /* yep owner name goes here */
       memset(&tape->vol1[79],'1',1);       /* char 1 always goes here */
      }
     else if (tape->label != UNLABELED)   /* unlabeled tape? - do nada */
      {                                    /* don't support this type */
       sprintf(message_area,
               "Unknown value for label\n"); /* tell user  */
       issue_error_message(message_area);
       close(taped);                       /* close */
       free(tape->name);                   /* release control block */
       tape->name=NULL; 
       return (-1);                        /* and return */     
      }
     if (tape->label != UNLABELED)
      tpwrite(tape,tape->vol1,80);         /* write out volume label */
     tapeweof(tape,3);                     /* terminate the tape */
     taperew(tape);                        /* rewind the tape */
    }
   if (tape->realtape == NO)               /* this a virtual tape? */
    {
     lseek(tape->file,0L,SEEK_SET);        /* verify we start with a IBG */
     len=read(tape->file,tape->buffer,12); /* read IBG */
     lseek(tape->file,0L,SEEK_SET);        /* rewind */
     tape->buffer[12]='\0';                /* term IBG */
     if (len == 12 &&                      /* validate length, */
        (strspn((char *)tape->buffer,"0123456789ABCDEF") == 12) && /* char set*/
        (memcmp(tape->buffer,"0000",4) == 0) &&  /* prev rec len must = 0 */
        (memcmp(tape->buffer+4,tape->buffer+8,4) == 0)) /* current len and check must = */
      {
      /* It looks like a faketape, is it supposed to be? */
      if (tape->drive_type == 0 || tape->drive_type == DEV_FAKETAPE)
       tape->drive_type = DEV_FAKETAPE;
      else
       tape->drive_type = 0;
      }
     else 
      {
       /* See if it looks like an AWSTape file */
       AWSTAPE_BLKHDR hdr;
       lseek(tape->file,0L,SEEK_SET);        /* verify we start with a IBG */
       len=read(tape->file, &hdr, sizeof(hdr));  /* read IBG */
       lseek(tape->file,0L,SEEK_SET);        /* rewind */
       if (len == sizeof(hdr) &&
           hdr.prvblkl == 0 &&
           (hdr.flags1 == AWSTAPE_FLAG1_NEWREC + AWSTAPE_FLAG1_ENDREC ||
            hdr.flags1 == AWSTAPE_FLAG1_TAPEMARK) &&
           hdr.flags2 == 0)
        {
         /* Could be, it's a bit hard to say since the header isn't self 
            checking.  If it's supposed to be (or we don't know) assume
            it is. */
         if (tape->drive_type == 0 || tape->drive_type == DEV_AWSTAPE)
          tape->drive_type = DEV_AWSTAPE;
         else
          tape->drive_type = 0;
        }
       else
        /* Not a format we recognize */
        tape->drive_type = 0;
      }
     if (tape->drive_type != DEV_FAKETAPE && tape->drive_type != DEV_AWSTAPE)
      {
       sprintf(message_area,
               "'%s' is not a virtual tape\n",device_name); /*not valid*/
       issue_error_message(message_area);
       close(taped);                       /* close */
       free(tape->name);                   /* release control block */
       tape->name=NULL; 
       return (-1);                        /* return */
      }
    }
   if (tape->drive_type != DEV_QIC)       /* for all types but QIC open as */
    tape->data_mode=RECORD;               /* records */ 
   else                                   /* for QIC treat tape as a*/
    tape->data_mode=STREAM;               /* stream of characters */ 
   tape->fatal=OFF;                       /* No fatal errors - yet */
   tape->translate=ON;                    /* default EBCDIC translation */
   tape->eov=OFF;                         /* default - write EOF trailers */
   tape->trtable=EBCASC;                  /* default translate table */ 
   tape->blocking=ON;                     /* Perform Blocking or Deblocking */
   tape->op[0]='\0';
   if (tape==&tapeo) tape->otrtable=ASCEBC; /* if output set out tran table */
   else tape->otrtable=NULL;
                 /* disable block prefixing */
   tape->blkpfx=tape->blkpfxl=tape->blkpfxs=tape->span=0; 
   cancel_command=OFF;                    /* operation not canceled */
   tape->fsheader=NULL;                   /* no FS header name */  
   tape->rd_hdr_rtn=NULL;                 /* no header read routine defined */ 
   tape->rd_tlr_rtn=NULL;                 /* no trailer read routine defined */
   tape->wrt_hdr_rtn=NULL;                /* no header write routine defined */
   tape->wrt_tlr_rtn=NULL;                /* no trailer write routine defined */
   tape->vol1[0]=tape->hdr1[0]=tape->hdr2[0]=tape->file_name[0]=tape->format[0]='\0';
   tape->block_len=tape->rec_len=tape->newfile_name=0;
   tape->fmchar[0]='\0';                  /* tape format character */
   tape->datecheck=ON;                    /* date checking on */
   memcpy(tape->expiration," 00000",6);   /* setting for exiration dates */
   tape->owner[0]='\0';                   /* no owner yet */
   tape->format[0]='\0';                  /* no format yet */
   if (tape->drive_type != DEV_QIC)       /* if not a QIC have to check if FS */ 
    {
     len=tpread(tape,tape->buffer,32768*2); /* read first block */
     for (i=0;i<4;i++) buf[i]=EBCASC[tape->buffer[i]]; /*copy and tran 4 char*/
     if (len < 0)                         /* happy camper? */
      {
       issue_ferror_message("unrecoverd error occured while reading first block");
       tape->fatal=ON;
       return(len);
      }
     else if (len != 80)                  /* could the first record be a VOL */
      {
       tape->tape_type=UNLABELED;        /* mark tape as unlabeled */
       if (len==0)                        /* tape start wtih TM ? */
        {
         tpsync(tape);                    /* yep sync tape */
         tapefsf(tape,1);                 /* skip over TM */
         len=tpread(tape,tape->buffer,32768*2);    /* read a block */
         if (len <0)
          {
           issue_ferror_message("Null first file - unrecovered error reading first block second file\n");
           tape->fatal=ON;
           return (len);
          }
         if (len>80 && !not_fs)           /* check to see if FS header */
          if (memcmp((char*)tape->buffer,magic_id,4) ==0 ||           
              memcmp((char *)tape->buffer,magic_id2,4)==0) 
           {
            tape->tape_type=FS_UNLABELED; /* mark tape as an UNLABELED FS */
           }
        }
       else if (len < 0)
        {
         issue_error_message("Unrecovered error reading second block of tape\n");
         return (len);
        }
      }
     else if (strncmp((char*)buf,(char *)vol1,4)==0) /*start with EBCDIC VOL1?*/
      {
       if (tape->label==TOS_LABEL)           /* just labeled AS TOS */
        {
         tape->tape_type=TOS_LABEL;          /* set tape to TOS labeled */
         tape->rd_hdr_rtn=&rd_toshdr;        /* set  header read routine */
         tape->rd_tlr_rtn=NULL;              /* set trailer read routine */
         if (tape==&tapeo)                   /* if opening for output */
          {
           tape->wrt_hdr_rtn=&wrt_toshdr;    /* set header write routine */
           tape->wrt_tlr_rtn=&wrt_tostlr;    /* set trailer write routine */
          }
        }
       else if (tape->label==VLO_LABEL ||    /* Just labeled as a VLO tape */
                is_VLO_tape)                 /* Caller said it is VLO */
        {
         tape->tape_type = VLO_LABEL;
         tape->rd_hdr_rtn = &rd_nohdr;
         tape->rd_tlr_rtn = NULL;
         if (tape == &tapeo)
          {
           tape->wrt_hdr_rtn = &wrt_nohdr;
           tape->wrt_tlr_rtn = &wrt_notlr;
          }
        }
       else
        {
         tape->tape_type=IBM_LABEL;         /* set initially as IBM labeled */
         tape->rd_hdr_rtn=&rd_ibmhdr;       /* set header read routine */
         tape->rd_tlr_rtn=NULL;             /* set trailer read routine */
         if (tape==&tapeo)                  /* if opening for output */
          {
           tape->wrt_hdr_rtn=&wrt_ibmhdr;   /* set header write routine */
           tape->wrt_tlr_rtn=&wrt_ibmtlr;   /* set trailer write routine */
          }
        }
       memcpy(tape->vol1,tape->buffer,80);  /* copy the volume hdaer */ 
       for (i=0;i<6 && tape->vol1[i+4] != 0x40;i++) /* copy and tran vol label*/
        tape->volume[i] = EBCASC[tape->vol1[i+4]];
       tape->volume[i]='\0';                /* terminate volume label */
       for (i=0;i<10 && tape->vol1[i+41] != 0x40;i++)  /* copy and tran owner */
        tape->owner[i] = EBCASC[tape->vol1[i+41]];
       tape->owner[i]='\0';                 /* terminate owner id */
       len=tpread(tape,tape->buffer,32768*2); /* try to read HDR1 record */ 
       for (i=0;i<4;i++) buf[i]=EBCASC[tape->buffer[i]];/*copy and tran 4 char*/
       if (len<0)
        {
         issue_ferror_message("Unrecovered error occured when reading  HDR1 of first file\n");
         tape->fatal=ON;
         return(len);
        }
       if (len==0)                  
        {
         if (! not_fs)
          {
           tpsync(tape);                     /* there wasn't one */
           tapefsf(tape,1);                  /* skip the tape mark */
           len=tpread(tape,tape->buffer,32768*2); /* see if first file is null */ 
           if (len==0)                       /* was there one? */
            {
             tpsync(tape);                   /* first file is Null */
             tapefsf(tape,1);                /* skip the tape mark */
             len=tpread(tape,tape->buffer,32768*2); /* try to read FS header */ 
             if (len>80)                       /* was there one? */
              if (memcmp((char *)tape->buffer,magic_id,4) ==0 ||        
                  memcmp((char *)tape->buffer,magic_id2,4)==0)
               {
                tape->rd_hdr_rtn=&rd_nohdr;    /* yep -change header read rtn */
                tape->rd_tlr_rtn=NULL;         /* change trailer read rtn */
                if (tape==&tapeo)              /* if opening for output */
                 {
                  tape->wrt_hdr_rtn=&wrt_nohdr; /* change header read rtn */
                  tape->wrt_tlr_rtn=&wrt_notlr; /* change header write rtn */
                 }
                tape->tape_type=FS_VLOLABEL;    /* mark tape as FS VLO tape */
               }
            }
          }
        }
       else if (tape->tape_type == VLO_LABEL) /* Can't have HDR1 on VLO tape */
        {
         issue_ferror_message("VLO tape should have a tape mark following VOL1 label\n");
         return -1;
        }
       else if (len==80 &&
                strncmp((char*)&buf,"HDR1",4) == 0) /* This a HDR1 record? */
        {
         len=tpread(tape,tape->buffer,32768*2); /* yep try to read HDR2 */
         for (i=0;i<4;i++) 
          buf[i]=EBCASC[tape->buffer[i]];   /*copy and tran 4 char*/
         if (len<0)
          {
           issue_ferror_message("Unrecovered error occured when reading  HDR2 of first file\n");
           tape->fatal=ON;
           return(len);
          }
         if (len==0)
          {
           tpsync(tape);                    /* no HDR2 */
           tape->tape_type=TOS_LABEL;       /* change tape type to TOS label */
           tape->rd_hdr_rtn=&rd_toshdr;     /* change header read rtn */
           tape->rd_tlr_rtn=NULL;           /* change trailerr read rtn */
           if (tape==&tapeo)                /* if opening for output */
            {
             tape->wrt_hdr_rtn=&wrt_toshdr; /* change header write rtn */
             tape->wrt_tlr_rtn=&wrt_tostlr; /* change trailer write rtn */
            }
          }
         else if (len==80 && !not_fs)
          {
           tapefsf(tape,1);                 /* skip over rest of header */
           len=tpread(tape,tape->buffer,32768*2); /* is this a null file? */
           if (len<0)
            {
             issue_ferror_message("Unrecovered error occured when reading 1st data record of first file\n");
             tape->fatal=ON;
             return(len);
            }
           if (len==0)
            {
             tpsync(tape);                  /* yep sync tape */
             for (i=1;i<=3;i++)            /* skip over data, tlr, and header */
              tapefsf(tape,1); 
             len=tpread(tape,tape->buffer,32768*2); /*read possible FS header */
             if (len<0)
              {
               issue_ferror_message("Unrecovered error occured when reading 1st data record of second file\n");
               tape->fatal=ON;
               return(len);
              }
             if (len>80)                    /* is it */
              if (memcmp((char *)tape->buffer,magic_id,4) ==0 ||       
                  memcmp((char *)tape->buffer,magic_id2,4)==0) 
               tape->tape_type=FS_IBMLABEL; /* mark as Labelled FS tape */
            }
          } 
        }
      }
     else if (strncmp((char*)tape->buffer,vol1,4)==0) /* ansi volume? */
      { 
       tape->rd_hdr_rtn=&rd_ansihdr;        /* set header read rtn */
       tape->rd_tlr_rtn=NULL;               /* set trailer read rtn */
       if (tape==&tapeo)                    /* if open for output */
        {
         tape->wrt_hdr_rtn=&wrt_ansihdr;    /* set header read rtn */
         tape->wrt_tlr_rtn=&wrt_ansitlr;    /* set trailer read rtn */
        }
       memcpy(tape->vol1,tape->buffer,80);  /* copy VOL1 record */
       memcpy(tape->volume,&tape->vol1[4],6); /* copy volume name */
       for (i=0;i<6 && tape->volume[i] != ' ';i++) ; /* and terminate */
        tape->volume[i]='\0';
       memcpy(tape->owner,&tape->vol1[37],14); /* copy owner ID */
       for (i=0;i<14 && tape->owner[i] != ' ';i++) ; /* and terminate */
        tape->owner[i]='\0';                 
       tape->translate=OFF;                /* default to no translate */
       tape->tape_type=ANSI_LABEL;         /* set tape type to ansi label */
      }
     else
      tape->tape_type=UNLABELED;          /* mark tape as unlabeled */
    } 
   else
    tape->tape_type=UNLABELED;          /* mark tape as unlabeled */
   if (tape->tape_type == UNLABELED || tape->tape_type==FS_UNLABELED)
    {
     tape->rd_hdr_rtn=&rd_nohdr;         /* set header read routine */
     tape->rd_tlr_rtn=NULL;              /* set trailer read routine */
     if (tape==&tapeo)                   /* if opened for output */
      {
       tape->wrt_hdr_rtn=&wrt_nohdr;     /* set header write routine */
       tape->wrt_tlr_rtn=&wrt_notlr;     /* set trailer write routine */
      }
     tape->vol1[0]=tape->volume[0]=tape->owner[0]='\0'; /*no VOL1 owner or vol*/
     if (tape->realtape==YES)            /* real tape? */
      if (tape->drive_type == DEV_QIC || /* yep - non mainframe types? */
          tape->drive_type == DEV_4MM ||
          tape->drive_type == DEV_8MM)
       tape->translate=OFF;              /* yep - default translate to off */ 
    } 
   tape->lp=ON;                       /* mark label processing enabled */
   taperew(tape);                     /* rewind tape */
   posn(tape,1);                      /* position to first file */
   if (tape==&tapeo && tape->label>0)      /* initializing a tape? */
    tape->leot=ON;
   return (1);                        /* done */
  }


int posn(struct deviceinfo *tape, int tposn) 
 {
  struct tapestats tapes;
  int mt_count;
  int truefile, rc, label;
  int curr_posn;

  label=OFF;                           /* assume not labeled */
  curr_posn=tape->position;
  if (tape->lp != HEADER_RTN) tape->position=tposn; /* not in header - set pos*/
  if (tape->lp == HEADER_RTN ||        /* called from header routine or */
      tape->lp == OFF ||               /* label processing is off or */
      tape->tape_type == UNLABELED)   /* tape is unlabeled */
   truefile = tposn-1;         /* set phsyical file # one less logical file # */
  else if (tape->tape_type == IBM_LABEL ||
           tape->tape_type == ANSI_LABEL ||
           tape->tape_type == TOS_LABEL || 
           tape->tape_type == FS_IBMLABEL) /* a labeled tape? */
   {
    label=ON;                          /* yep - mark as such */
    truefile = (3 * (tposn-1));    /* physical file = 3 * (logical file # -1) */
    if (tape->tape_type == FS_IBMLABEL) /* if a labeled FS tape */
     truefile+=3;                     /* physical file = 3 * (logical file #) */
   }
  else if (tape->tape_type == FS_UNLABELED || /* fs unlabeled tape? */
           tape->tape_type == VLO_LABEL)  /* VLO tape */
   truefile = tposn;                  /* yep - physical file = logical file # */
  else if (tape->tape_type == FS_VLOLABEL)  /* vlo FS tape ? */
   truefile = tposn+1;             /* yep - physical file = logical file # +1 */
  else 
   {
    fprintf(stderr,"POSN unknown tape type %d \n",tape->tape_type);
    exit (2);                          /* real - inform user and quit */
   }
  if (truefile==0) taperew(tape);   /* positioning to front of tape? - rewind */
  rc = tapestatus(tape, &tapes);       /* get info on tapes current position */
  if ( rc == -1 ) 
   {
    fprintf(stderr,"Failed to acquire tape status. \n"); /* this is bad */
    exit (2);                          /* quit */
   } 
  mt_count = truefile - tapes.mt_fileno; /* number of files to position */
  if (mt_count>=1)               /* forward spaceing ? */
   {
    int i, end, eot, len;
    end=mt_count;                /* yep - save count */
    eot=OFF;                           /* not at EOT */
    if (tape->lp == ON && tape->leot == ON) 
     {
      tape->position=curr_posn;
      return (1);
     }
    for (i=1;i<=end;i++)               /* try to forward space request amount */
     {
      tapefsf(tape,1);                 /* forward space one file */
      tapestatus(tape, &tapes);        /* get tape status */
      len=tpread(tape,tape->buffer,327678*2);
      if (len <0)
       {
        issue_ferror_message("Unrecovered error occured when reading data in an positioning operation\n");
        tape->fatal=ON;
        return(len);
       }
      if (len==0)                      /* was it a TM? */
       {
        tpsync(tape);                  /* yep - sync tape */
        if (eot==OFF && label==ON && tapes.mt_fileno % 3 == 0) /*labeled EOT? */
         {
          tape->position = (tapes.mt_fileno/3) + 1; /*yep - set logical tape #*/     
          if (tape->tape_type == FS_IBMLABEL) /* if FS tape */
           {
            tapebsf(tape,3);           /* backup over trailing dummy file  */
            tape->position-=2;         /* And return *fs file number */ 
           }
          tape->leot=ON;
          break;                       /* done */
         }
        if (eot==ON)                   /* have we hit a possible LEOT? */
         {
          tapebsf(tape,1);             /* backup to previous file */
          if (tape->lp == OFF || tape->tape_type == UNLABELED) /* unlabeled? */
           tape->position=tapes.mt_fileno; /* yep logical file = physical file +1 */ 
          else if (tape->tape_type == FS_UNLABELED || /* fs unlabeled? */
                   tape->tape_type == VLO_LABEL)  /* VLO tape? */
           tape->position=tapes.mt_fileno+1; /* yep logical file = physical file +2 */
          else if (tape->tape_type == FS_VLOLABEL) /* fs vlo tape? */ 
           tape->position=tapes.mt_fileno+2; /* yep logical file = physical file+3 */
          if (tapes.mt_blkno != 0)     /* should be at start of a file */
           {
            issue_ferror_message("Fatal error beyond EOT?\n");/*no give severe warning*/ 
            tape->fatal=ON;
           }
          tape->leot=ON;
          return(eot);                 /* return LEOT */
         }
        else eot=ON;                  /* mark as possible LEOT */
       }
      else eot=OFF;                 /* not really an LEOT  - just a null file */
     }
    if (eot==OFF) {tapebsr(tape,1);}/*if not at EOT-backspace over record read*/
   }
  else
   {
    tape->leot=OFF;
    tapefsf(tape,mt_count);         /* we can backspace with abandon */
   }
  tapestatus(tape, &tapes);               /* get tape status */ 
  if (tape->lp == HEADER_RTN) return (0);/*if call from header rtn just return*/
  return ((*tape->rd_hdr_rtn)(tape,truefile)); /*else read header and return*/
 }


static int rd_nohdr(struct deviceinfo *tape,int posn)
 { 
  int len;
  struct tapestats tapes;

  tape->blkpfx=tape->blkpfxl=tape->blkpfxs=0; /* disable block prefixing */
  tape->file_name[0]='\0';                  /* current file name  not defined */
  tape->cur_expiration=tape->create_date=0;
  if (tape->lp==ON && tape->tape_type >= FS_UNLABELED) /* this an FS tape? */
   { 
    tape->read_rtn=&mts_fsread;             /* yep set read routine */
    if (tape->format[0] == '\0')            /* format already set? */
     {
      strcpy((char *)tape->format,"U");     /* default to U */
      tape->block_len=32767;                /* default block len */
      tape->rec_len=0;                      /* and record length */
     }
   }
  else if (tape->format[0] == '\0' ||
           tape->read_rtn == NULL)         /*format and read rtn already set? */
   {                                        /* no - det defaults */
    strcpy((char *)tape->format,"U");        /* mark format U */
    tape->read_rtn=&u_read;                  /* yep -set read routine */
    tape->rec_len=0;                         /* set rec length */
    if (tape->realtape==NO ||                /* virtual tape or */
        tape->drive_type==DEV_HALFINCH)  /* a half inch device? */
     tape->block_len=32767;                  /* set block len */
    else if (tape->drive_type==DEV_QIC)      /* QIC tape? */ 
     tape->block_len=32768;                  /* set "block len" */
    else                                     /* must be a 4mm or 8mm */
     tape->block_len=65535;                  /* set block length */
   }
  if (tape->drive_type != DEV_QIC)         /* Non QIC devices check for EOF */ 
   {
    len=tpread(tape,tape->buffer,32768*2); /* read a block */ 
    if (len < 0)
     {    
      issue_ferror_message("Unrecovered read error while processing a label\n");
      tape->fatal=ON;
      return(len);
     }
    if (len==0)                            /* was it a tape mark? */
     {
      tpsync(tape);                        /* yep - sync tape */
      if (tape->tape_type==IBM_LABEL ||    /* was this a labeled tape? */
          tape->tape_type==ANSI_LABEL ||
          tape->tape_type==FS_IBMLABEL ||
          tape->tape_type==TOS_LABEL)
       {
        tapestatus(tape,&tapes);           /*should this have been a hdr file*/
        if (tapes.mt_fileno % 3 == 0)
         return (1);                       /* yep assume LEOT */     
       }
      tapefsf(tape,1);                     /* skip tm */
      len=tpread(tape,tape->buffer,32768*2); /* any data here? */
      if (len < 0)
       {    
        issue_ferror_message("Unrecovered read error while looking for second tape mark\n");
        tape->fatal=ON;
        return(len);
       }
      if (len == 0) tpsync(tape);          /* if another tape mark - sync and */
      tapebsf(tape,1);                    /* and backspace a file */
      if (len == 0) return (1);     /* if two null files in a row return LEOT */
     }
    else                                   /* file wasn't null */ 
     tapebsr(tape,1);                      /* backspace to beginning of file */
   }
  else
   {
    tapenbsf(tape,0);                    /* QIC device just position to start */
   }
  validate_format(tape);                  /* validate tape format */
  return (getfname(tape));                /* get file name if *FS and return */ 
 }


static int rd_ibmhdr(struct deviceinfo *tape,int posn)
 {
  int len,i;
  char leng[7];
  struct tapestats tapes;

  tape->file_name[0]='\0';                  /* current file name  not defined */
  tape->cur_expiration=tape->create_date=0; /* current expiration not defined */
  len=0;
  if (posn==0) len=tpread(tape,tape->buffer,32768*2); /* if at start skip VOL1 */
  if (len < 0)
   {    
    issue_ferror_message("Unrecovered read error skipping volume label\n");
    tape->fatal=ON;
    return(len);
   }
  tape->hdr1[0]=tape->hdr2[0]='\0';         /* no HDRS yet */
  len=tpread(tape,tape->buffer,32768*2);    /* read HDR1 */    
  if (len < 0)
   {    
    issue_ferror_message("Unrecovered read error trying to fetch HDR1 record\n");
    tape->fatal=ON;
    return(len);
   }
  if (len==0)
   {
    tpsync(tape);
    if (tape->format[0]=='\0')                   /* if no format set */
     {
      strcpy((char *)tape->format,"VBS");        /* use VBS(32670,327771) */
      tape->block_len=32760;
      tape->rec_len=32771;
     }
    return (1);
   }    /* if TM - sync and return EOT */
  tape->blkpfx=tape->blkpfxl=tape->blkpfxs=0; /*disable block prefixing */
  if (len !=80) goto lblerr;                /* HDR1 must be 80 bytes */
  memcpy(tape->hdr1,tape->buffer,80);       /* copy HDR1 */
  for (i=0;i<80;i++) tape->hdr1[i]=EBCASC[tape->hdr1[i]]; /* tran to ascii */
  if (strncmp((char *)tape->hdr1,"HDR1",4) != 0) goto lblerr; /*must have HDR1*/
  len=tpread(tape,tape->buffer,32768*2);    /* read HDR2 */
  if (len < 0)
   {    
    issue_ferror_message("Unrecovered read error trying to fetch HDR2 record\n");
    tape->fatal=ON;
    return(len);
   }
  if (len != 80) goto lblerr;               /* HDR2 must be 80 bytes */
  memcpy(tape->hdr2,tape->buffer,80);       /* copy */
  for (i=0;i<80;i++) tape->hdr2[i]=EBCASC[tape->hdr2[i]]; /* translate ascii */
  if (strncmp((char *)tape->hdr2,"HDR2",4) != 0) goto lblerr; /*must have HDR2*/
  memcpy(leng,tape->hdr2+5,5);              /* copy block length */
  leng[5]='\0';                             /* terminate block length string */
  tape->block_len=atoi(leng);               /* get numeric value */
  memcpy(leng,tape->hdr2+10,5);             /* copy record length */
  tape->rec_len=atoi(leng);                 /* get numeric value */
  memcpy(leng,tape->hdr1+41,6);             /* get creation date */
  tape->create_date=atoi(leng);             /* change to numeric value */
  leng[6]='\0';                             /* terminate create date */
  memcpy(leng,tape->hdr1+47,6);             /* get expiration date */
  tape->cur_expiration=atoi(leng);          /* change to numeric value */
  tape->format[0]='\0';                     /* initial tape format */
  tape->format[1]='\0';
  if (tape->hdr2[4]=='U') tape->format[0]='U';/* get first character of format*/
  else if (tape->hdr2[4]=='F') tape->format[0]='F';
  else if (tape->hdr2[4]=='V') tape->format[0]='V';
  else goto lblerr;                           /* say what - not legal */
  if (tape->hdr2[36]==' ') tape->fmchar[0]='\0'; /* pick if CC char */
  else tape->fmchar[0]=tape->hdr2[36];
  if (tape->hdr2[38]=='B') strcat((char *)tape->format,"B"); /* blocked? */
  else if (tape->hdr2[38]=='S') strcat((char *)tape->format,"S");/* spanned? */
  else if (tape->hdr2[38]=='R') strcat((char *)tape->format,"BS"); /* both? */
  else if (tape->hdr2[38]!=' ') goto lblerr; /* never heard of this */

  if (tape->lp == 1 && tape->tape_type >= FS_UNLABELED) /* get read routine */
   tape->read_rtn=&mts_fsread;               
  else if (strcmp((char *)tape->format,"VBS") == 0) tape->read_rtn=&vbs_read;
  else if (strcmp((char *)tape->format,"VS") == 0) tape->read_rtn=&vbs_read;
  else if (strcmp((char *)tape->format,"VB") == 0) tape->read_rtn=&vb_read;
  else if (strcmp((char *)tape->format,"V") == 0) tape->read_rtn=&vb_read;
  else if (strcmp((char *)tape->format,"FBS") ==0) tape->read_rtn=&fb_read;
  else if (strcmp((char *)tape->format,"FS") ==0) tape->read_rtn=&fb_read;
  else if (strcmp((char *)tape->format,"FB") == 0) tape->read_rtn=&fb_read;
  else if (strcmp((char *)tape->format,"F") == 0) tape->read_rtn=&fb_read;
  else if (strcmp((char *)tape->format,"U") ==0) tape->read_rtn=&u_read;
  else goto lblerr;             /* error -  don't know what to do with format */
  tapefsf(tape,1);                            /* skip over rest of HDR */
  return (getfname(tape));                    /* get file name and return */
  lblerr:
   tapestatus(tape,&tapes);                  /*get tape status */
   sprintf(message_area, " len=%d file=%d\n  hdr1=%s\n hdr2=%s\n"
           ,len,tapes.mt_fileno,tape->hdr1,tape->hdr2);
   issue_ferror_message(message_area);
   issue_ferror_message("Header label error\n");  /* tell user bad news */
   tape->fatal=ON;
   return(-1);
 }


static int rd_ansihdr(struct deviceinfo *tape,int posn)
 {
  int len;
  char leng[7];
  struct tapestats tapes;

  tape->file_name[0]='\0';                  /* current file name  not defined */
  tape->cur_expiration=tape->create_date=0; /* current expiration not defined */
  len=0;
  if (posn==0) len=tpread(tape,tape->buffer,32768*2); /* if at SOT skip vol label */
  if (len < 0)
   {    
    issue_ferror_message("Unrecovered read error skipping volume label\n");
    tape->fatal=ON;
    return(len);
   }
  len=tpread(tape,tape->buffer,32768*2);    /* read HDR1 rec */
  if (len < 0)
   {    
    issue_ferror_message("Unrecovered read error trying to fetch HDR1 record\n");
    tape->fatal=ON;
    return(len);
   }
  if (len==0) 
   {
    tpsync(tape);
    if (tape->format[0]=='\0')                    /* if no format */
     {
      strcpy((char *)tape->format,"DBS");         /* use DBS(2048,327872) */
      tape->block_len=2048;
      tape->rec_len=32772;
      tape->blkpfx=4;
      tape->blkpfxl=ON;
     }
    return (1);
   }    /* if EOF return */
  if (len !=80) goto lblerr;                /* better be 80 characters */
  memcpy(tape->hdr1,tape->buffer,80);       /* make a copy */
  if (strncmp((char *)tape->hdr1,"HDR1",4) != 0) goto lblerr; /*better be HDR1*/
  len=tpread(tape,tape->buffer,32768*2);    /* read HDR2 */ 
  if (len < 0)
   {    
    issue_ferror_message("Unrecovered read error trying to fetch HDR2 record\n");
    tape->fatal=ON;
    return(len);
   }
  tape->blkpfx=tape->blkpfxl=tape->blkpfxs=0; /* disable block prefixing */ 
  if (len==0)                               /* EOF? */
   {
    tpsync(tape);                           /* sync tape */
    strcpy((char *)tape->format,"U");       /* set default format */
    tape->block_len=32767;
    return (getfname(tape));                /* set file name and return */
   }
  if (len != 80) goto lblerr;               /* better be 80 characters */
  memcpy(tape->hdr2,tape->buffer,80);       /* make a copy */
  if (strncmp((char *)tape->hdr2,"HDR2",4) != 0) goto lblerr; /*better be HDR2*/
  memcpy(leng,tape->hdr2+5,5);              /* get block length (char) */ 
  leng[5]='\0';                             /* terminate */
  tape->block_len=atoi(leng);               /* block length numeric */
  memcpy(leng,tape->hdr2+10,5);             /* record length (char) */
  tape->rec_len=atoi(leng);                 /* record length numeric */
  memcpy(leng,tape->hdr1+41,6);             /* creation date (julian char) */
  tape->create_date=atoi(leng);             /* creation date numeric */
  leng[6]='\0';                             /* terminate */
  memcpy(leng,tape->hdr1+47,6);             /* expiration date (julian char) */
  leng[6]='\0';                             /* terminate */
  tape->cur_expiration=atoi(leng);          /* expiration date numeric */
  tape->blkpfx=atoi(strncpy((char *)tape->format,(char *)tape->hdr2+50,2));/*pfx length */
  memset(tape->format,'\0',3);              /* zap format */
  if (tape->hdr2[4]=='U') tape->format[0]='U'; /* pick of first char of format*/
  else if (tape->hdr2[4]=='F') tape->format[0]='F';
  else if (tape->hdr2[4]=='D') tape->format[0]='D';
  else if (tape->hdr2[4]=='V') tape->format[0]='V';
  else goto lblerr;                         /* never heard of this */
  if (tape->hdr2[36]==' ') tape->fmchar[0]='\0'; /* pck up CC char */
  else tape->fmchar[0]=tape->hdr2[36];
  if (tape->hdr2[38]=='B') strcat((char *)tape->format,"B"); /* fill in 2 & 3 */
  else if (tape->hdr2[38]=='S') strcat((char *)tape->format,"S"); /*characters*/
  else if (tape->hdr2[38]=='R') strcat((char *)tape->format,"BS"); /* of fmt */
  else if (tape->hdr2[38]!=' ') goto lblerr; /* never heard of this */
  /* set read appropiate read routine */
  if (strcmp((char *)tape->format,"DBS") == 0) tape->read_rtn=&dbs_read;
  else if (strcmp((char *)tape->format,"DS") == 0) tape->read_rtn=&dbs_read;
  else if (strcmp((char *)tape->format,"DB") == 0) tape->read_rtn=&db_read;
  else if (strcmp((char *)tape->format,"D") == 0) tape->read_rtn=&db_read;
  else if (strcmp((char *)tape->format,"VB") == 0) tape->read_rtn=&vb_read;
  else if (strcmp((char *)tape->format,"V") == 0) tape->read_rtn=&vb_read;
  else if (strcmp((char *)tape->format,"FS") ==0) tape->read_rtn=&fb_read;
  else if (strcmp((char *)tape->format,"FB") == 0) tape->read_rtn=&fb_read;
  else if (strcmp((char *)tape->format,"F") ==0) tape->read_rtn=&fb_read;
  else if (strcmp((char *)tape->format,"U") ==0) tape->read_rtn=&u_read;
  else goto lblerr;                        /* no read routine for this */
  tapefsf(tape,1);                         /* position to start of data area */
  return (getfname(tape));                 /* get name of datset and return */
  lblerr:                                  /* on label error print some info */
   tapestatus(tape,&tapes);
   sprintf(message_area, " len=%d file=%d\n",len,tapes.mt_fileno);
   issue_ferror_message(message_area); 
   issue_ferror_message("Header label error\n");
   tape->fatal=ON;
   return(-1);
 }


static int rd_toshdr(struct deviceinfo *tape,int posn)
 {
  int len,i;
  struct tapestats tapes;

       /* just in case - disable block prefixing */
  tape->blkpfx=tape->blkpfxl=tape->blkpfxs=0;
  tape->file_name[0]='\0';                  /* current file name  not defined */
  tape->cur_expiration=tape->create_date;   /* current expiration not defined */
  len=0;
  if (posn==0) len=tpread(tape,tape->buffer,32768*2); /* if SOT skip volume label */
  if (len < 0)
   {    
    issue_ferror_message("Unrecovered read error skipping volume label\n");
    tape->fatal=ON;
    return(len);
   }
  len=tpread(tape,tape->buffer,32768*2);    /* read HDR1 */
  if (len < 0)
   {    
    issue_ferror_message("Unrecovered read error trying to fetch HDR1 record\n");
    tape->fatal=ON;
    return(len);
   }
  if (len==0) {tpsync(tape);return (1);}    /* if EOF return */
  if (len !=80) goto toslblerr;             /* better be 80 characters */
  memcpy(tape->hdr1,tape->buffer,80);       /* make copy */
  for (i=0;i<80;i++) tape->hdr1[i]=EBCASC[tape->hdr1[i]]; /*translate to ASCII*/
  if (strncmp((char *)tape->hdr1,"HDR1",4) != 0) goto toslblerr; /* HDR1? */
  tape->block_len=32767;                    /* set default format */
  tape->rec_len=0;
  tape->format[0]='U';
  tape->format[1]='\0';
  tapefsf(tape,1);                          /* skip to data file */
  return (getfname(tape));                  /* get file name and return */
  toslblerr:                                /* on label error print some info */
   tapestatus(tape,&tapes);
   sprintf(message_area," len=%d file=%d \n",len,tapes.mt_fileno);
   issue_ferror_message(message_area);
   issue_ferror_message("Header label error\n");
   tape->fatal=ON;
   return(-1);
 }


static WRITE_RTN wrt_nohdr(struct deviceinfo *tape) 
 {
  WRITE_RTN write_rtn;
  int hdr_posn;
  struct tapestats tapes;
 
  tapestatus(tape,&tapes);                /* get info on tape position */
  if (tape->lp==ON &&
      tape->tape_type==FS_VLOLABEL)
   hdr_posn = tapes.mt_fileno - 1;        /* Logical file = phys - 1 */
  else if (tape->lp== ON &&
       tape->tape_type==VLO_LABEL)
   hdr_posn = tapes.mt_fileno;              /* Logical file = phys file */
  else
   hdr_posn = tapes.mt_fileno+1;          /* Locical file = phys +1 */
  tape->position = hdr_posn;
  posn(tape,hdr_posn);                    /*make sure at start of data area*/
  if ((tape->lp==ON && tape->tape_type==FS_VLOLABEL) || /* If this is an FS tape */
    (tape->lp==ON && tape->tape_type==FS_UNLABELED))
   {
/*  strcpy((char *)tape->format,"U");    / set this a the defalut format /
    tape->block_len=32767;
    tape->rec_len=tape->blkpfxs=0;       / make sure these are reset */
    issue_error_message("Copying or duplictation of FS tape not supported\n");
    return (NULL);
   }
  else if (tape->format[0] == '\0')      /* user set format? */
   {
    strcpy((char *)tape->format,"U");    /* fmt = U */
    tape->rec_len=0;
    if (tape->drive_type==DEV_HALFINCH)  /* no - 9 track or cartridge? */
     {
      tape->block_len=32767;             /* fmt = u(32767) */
     }
    else if (tape->drive_type==DEV_QIC)  /* if QIC? */
     {
      tape->block_len=512;               /* fmt = U(512) */
     }
    else                                 /* must be 8mm or 4mm */
     {
      tapeo.block_len=65535;             /* fmt=u(512) */
     }
   }
  if (validate_format(tape) == NULL) return(NULL); /* see if format valid */
   /* set appropiate write routine */
  if (tape->lp == 1 && tape->tape_type >= FS_UNLABELED) write_rtn=&mts_fswrite;
  else if (tape->lp == 1 && tape->tape_type >= FS_VLOLABEL) write_rtn=&mts_fswrite;
  else if (strcmp((char *)tape->format,"DBS") == 0) write_rtn=&dbs_write;
  else if (strcmp((char *)tape->format,"DS") == 0) write_rtn=&dbs_write;
  else if (strcmp((char *)tape->format,"DB") == 0) write_rtn=&db_write;
  else if (strcmp((char *)tape->format,"D") == 0) write_rtn=&db_write;
  else if (strcmp((char *)tape->format,"VBS") == 0) write_rtn=&vbs_write;
  else if (strcmp((char *)tape->format,"VS") == 0) write_rtn=&vbs_write;
  else if (strcmp((char *)tape->format,"VB") == 0) write_rtn=&vb_write;
  else if (strcmp((char *)tape->format,"V") == 0) write_rtn=&vb_write;
  else if (strcmp((char *)tape->format,"FBS") ==0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"FS") ==0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"FB") == 0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"F") == 0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"U") ==0) write_rtn=&u_write;
  else exit(2);                          /* sno */ 
  tape->blocks=tape->arec_len=0;         /* initialize blocks read */
  return(write_rtn);                     /* return the write routine */
 }


static WRITE_RTN wrt_ibmhdr(struct deviceinfo *tape)
 {
  struct tapestats tapes;
  int len, i, hdr_posn, cur;
  time_t timeval;
  struct tm *tm;
  WRITE_RTN write_rtn;

  if (tape->lp == ON && tape->tape_type == FS_IBMLABEL)
   { 
    issue_error_message("Copying or duplictation of FS tape not supported\n");
    return (NULL);
   }
  time(&timeval);                             /* get current time */ 
  tm=localtime(&timeval);                     /* in exploded format */
  if (tape->datecheck==ON && tape->cur_expiration>0) /* need to ckeck expir? */
   {
    cur=(tm->tm_year*1000)+tm->tm_yday+1;     /* yep - currnet date julian */
    if (cur<=tape->cur_expiration)             /* ok to overwrite */
     {
      issue_error_message("Attempt to overwrite an unexpired data set.\n");
      return (NULL);                          /* no tell user - done */
     }
   }  
  tapestatus(tape,&tapes);                    /* get current position */
  tape->position=(tapes.mt_fileno/3)+1;       /* logical file =(phys/3+1) */
  hdr_posn=((tape->position-1)*3)+1;          /* label pos = (logical -1*3)+1 */
  tape->lp=HEADER_RTN;                        /* say were in a header routine */
  if (tape->newfile_name == OFF) tape->file_name[0]='\0'; /*if Fname not set reset*/
  posn(tape,hdr_posn);                        /* to to label HDR label area */
  tape->newfile_name=0;                       /* turn off file name set flag */
  tapestatus(tape,&tapes);                    /* get tape info */
  tape->lp=ON;                               /* turn back on label processing */
  memset(tape->hdr1,' ',80);                  /* Zap HDR1 info */
  memcpy(tape->hdr1,"HDR1",4);                /* Set HDR1 */
  len=strlen((char *)tape->file_name);        /* length of file name */
  if (len==0)                                 /* if no filename */ 
   {
    sprintf((char *)tape->file_name,"V%s ",tape->volume); /* set to */
    for (i=0;i<7;i++)                         /* Vvolume.Fposition */
     if (tape->file_name[i]==' ') break;
     else len++;
    sprintf((char *)&tape->file_name[len],".F%05d",tape->position);
   }
  sprintf((char *)tape->buffer,"%-17.17s",tape->file_name); /*fill in filename*/
  memcpy(&tape->hdr1[4],tape->buffer,17);
  sprintf((char *)tape->buffer,"%6.6s",tape->volume);  /* Fill in volume name */
  memcpy(&tape->hdr1[21],tape->buffer,6);
  memcpy(&tape->hdr1[27],"0001",4);            /* always 0001 */
  sprintf((char *)tape->buffer,"%04d",tape->position); /* fill in tape file # */
  memcpy(&tape->hdr1[31],tape->buffer,4);
  sprintf((char *)tape->buffer,"%03d%03d",tm->tm_year,tm->tm_yday+1); /* cre date */
  memcpy(&tape->hdr1[41],tape->buffer,6);
  if (tape->hdr1[41]=='0') tape->hdr1[41]=' '; /*if first char 0 change to ' '*/
  memcpy(&tape->hdr1[47],tape->expiration,6);  /* fill in expiration date */
  tape->hdr1[53]='0';                          /* always 0 */
  memcpy(&tape->hdr1[54],"000000",6);          /* Block count always 0 hdr */
  len=strlen(INSTALATION_NAME);                /* fill in Installation name */
  if (len>13) len=13;
  memcpy(&tape->hdr1[60],INSTALATION_NAME,len);
  memset(tape->hdr2,' ',80);                   /* Zap HDR2 */
  memcpy(tape->hdr2,"HDR2",4);                 /* fill in HDR2 */
  if (tape->format[0]=='\0')                   /* if no format set */
   {
    strcpy((char *)tape->format,"VBS");        /* use VBS(32670,327771) */
    tape->block_len=32760;
    tape->rec_len=32771;
   }
  if (tape->format[0]=='F' || tape->format[0]=='V' || tape->format[0]=='U')
   tape->hdr2[4]=tape->format[0];              /* fill and ad validate basic */
  else                                         /* format */
   {
    bad_type:
    sprintf(message_area,"'%s' is an illegal format type for IBM lbelled tapes.\n",tape->format); 
    issue_error_message(message_area);
    return (NULL);
   }
  if (validate_format(tape) == NULL) return(NULL); /* finsh format valiation */
  tape->hdr2[36]=tape->fmchar[0];                  /* put in CC infor */
  len=strlen((char *)tape->format);
  if (len==2) tape->hdr2[38]=tape->format[1];      /* fill in blocking info */
  else if (len==3) tape->hdr2[38]='R';
  /* fill in block length and record length */
  sprintf((char *)tape->buffer,"%05d%05d",tape->block_len,tape->rec_len);
  memcpy(&tape->hdr2[5],tape->buffer,10);
  tape->hdr2[16]='0';                              /* always zero */
  if (tapes.mt_fileno==0) tpwrite(tape,tape->vol1,80); /* write vol if nec */
  for (i=0;i<80;i++) tape->buffer[i]=ASCEBC[tape->hdr1[i]]; /* trans HDR1 */
  tpwrite(tape,tape->buffer,80);                   /* write HDR1 */
  for (i=0;i<80;i++) tape->buffer[i]=ASCEBC[tape->hdr2[i]]; /* translate HDR2 */
  tpwrite(tape,tape->buffer,80);                   /* write HDR2 */
  tapeweof(tape,1);                                /* terminate HDR file */
    /* set write routine */
  if (tape->lp == 1 && tape->tape_type >= FS_UNLABELED) write_rtn=&mts_fswrite;
  else if (strcmp((char *)tape->format,"VBS") == 0) write_rtn=&vbs_write;
  else if (strcmp((char *)tape->format,"VS") == 0) write_rtn=&vbs_write;
  else if (strcmp((char *)tape->format,"VB") == 0) write_rtn=&vb_write;
  else if (strcmp((char *)tape->format,"V") == 0) write_rtn=&vb_write;
  else if (strcmp((char *)tape->format,"FBS") ==0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"FS") ==0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"FB") == 0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"F") == 0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"U") ==0) write_rtn=&u_write;
  else goto bad_type;                              /* format not valid ANSI */
  tape->blocks=tape->arec_len=0;                   /* initialize block count */
  return (write_rtn);                              /* return write routine */
 }


static WRITE_RTN wrt_ansihdr(struct deviceinfo *tape)
 {
  struct tapestats tapes;
  int len, i, hdr_posn,cur;
  time_t timeval;
  struct tm *tm;
  WRITE_RTN write_rtn;
 
  time(&timeval);                               /* get current time */
  tm=localtime(&timeval);                       /* in exploded format */
  if (tape->datecheck==ON && tape->cur_expiration>0) /* check expiration date?*/
   {
    cur=(tm->tm_year*1000)+tm->tm_yday;         /* yep current date exploded */
    if (cur<=tape->cur_expiration)               /* expired? */
     {
      issue_error_message("Attempt to overwrite an unexpired data set.\n");
      return (NULL);                            /* no tell user done */
     }
   }  
  tapestatus(tape,&tapes);                      /* current position info */
  tape->position=(tapes.mt_fileno/3)+1;         /* logical = (phys/3)+1 */
  hdr_posn=((tape->position-1)*3)+1;            /* hdr = ((logical-1)*3)+1 */
  tape->lp=HEADER_RTN;                          /* say we are in hdr routine */
  if (tape->newfile_name == OFF) tape->file_name[0]='\0';/* If no Fname set reset */
  posn(tape,hdr_posn);                          /* position tape to label area*/
  tape->newfile_name=0;                         /* reset file name set flag */
  tapestatus(tape,&tapes);                      /* info current position */
  tape->lp=ON;                                  /* enable label processing */
  memset(tape->hdr1,' ',80);                    /* Zap HDR1 area */
  memcpy(tape->hdr1,"HDR1",4);                  /* fill in HDR1 */
  len=strlen((char *)tape->file_name);          /* length of filename */
  if (len==0)                                   /* if no filename */
   {
    sprintf((char *)tape->file_name,"V%s ",tape->volume); /* set to */
    for (i=0;i<7;i++)                         /* Vvolume.Fposition */
     if (tape->file_name[i]==' ') break;
     else len++;
    sprintf((char *)&tape->file_name[len],".F%05d",tape->position);
   }
  sprintf((char *)tape->buffer,"%-17.17s",tape->file_name); /* fill in Fname */
  memcpy(&tape->hdr1[4],tape->buffer,17);
  sprintf((char *)tape->buffer,"%6.6s",tape->volume); /* fill in volume name */
  memcpy(&tape->hdr1[21],tape->buffer,6);
  memcpy(&tape->hdr1[27],"0001",4);              /* always 0001 */
  sprintf((char *)tape->buffer,"%04d",tape->position); /* fill in file number */
  memcpy(&tape->hdr1[31],tape->buffer,4);
  sprintf((char *)tape->buffer,"%03d%03d",tm->tm_year,tm->tm_yday+1);/* fill  */
  memcpy(&tape->hdr1[41],tape->buffer,6);             /* creation date */ 
  if (tape->hdr1[41]=='0') tape->hdr1[41]=' ';  /* start 0 change to ' ' */
  memcpy(&tape->hdr1[47],tape->expiration,6);   /* fill in expiration date */
  tape->hdr1[53]='0';                           /* always 0 */
  memcpy(&tape->hdr1[54],"000000",6);          /* fill block count - zero hdr */
  len=strlen(INSTALATION_NAME);                 /* fill in installation name */
  if (len>13) len=13;
  memcpy(&tape->hdr1[60],INSTALATION_NAME,len);
  memset(tape->hdr2,' ',80);                    /* zap HDR2 area */
  memcpy(tape->hdr2,"HDR2",4);                  /* fill in HDR2 */
  if (tape->format[0]=='\0')                    /* if no format */
   {
    strcpy((char *)tape->format,"DBS");         /* use DBS(2048,327872) */
    tape->block_len=2048;
    tape->rec_len=32772;
    tape->blkpfx=4;
    tape->blkpfxl=ON;
   }
  if (tape->format[0]=='F' || tape->format[0]=='D' || tape->format[0]=='U')
   tape->hdr2[4]=tape->format[0];               /* validate and fill in fmt */
  else if (tape->format[0]=='V' && tape->op[0] =='D')
   tape->hdr2[4]=tape->format[0];               /* validate and fill in fmt */
  else
   {
    bad_type:
    sprintf(message_area,"'%s' is an illegal format type for ANSI tapes.\n",tape->format); 
    issue_error_message(message_area);
    return (NULL);
   }
   /* fill in block and record length */
  sprintf((char *)tape->buffer,"%05d%05d",tape->block_len,tape->rec_len);
  memcpy(&tape->hdr2[5],tape->buffer,10);
  tape->hdr2[16]='0';                           /* always 0 */
  tape->span=0;                                 /* spanning format? */
  if (tape->format[1]=='S' || tape->format[2] == 'S') tape->span=1;
  if (validate_format(tape) == NULL) return(NULL); /* validate format */
  tape->hdr2[36]=tape->fmchar[0];               /* fill in CC char */
  len=strlen((char *)tape->format);             /* fill in blocking info */
  if (len==2) tape->hdr2[38]=tape->format[1];
  else if (len==3) tape->hdr2[38]='R';
  sprintf((char *)tape->buffer,"%02d",tape->blkpfx); /* fill in blkpfx info */
  memcpy(&tape->hdr2[50],tape->buffer,2);
  if (tapes.mt_fileno==0) tpwrite(tape,tape->vol1,80); /* write vol if nec */
  tpwrite(tape,tape->hdr1,80);                  /* write HDR1  */
  tpwrite(tape,tape->hdr2,80);                  /* write HDR2 */
  tapeweof(tape,1);                             /* teminate hdr label */
    /* set write routine */
  if (strcmp((char *)tape->format,"DBS") == 0) write_rtn=&dbs_write;
  else if (strcmp((char *)tape->format,"DS") == 0) write_rtn=&dbs_write;
  else if (strcmp((char *)tape->format,"DB") == 0) write_rtn=&db_write;
  else if (strcmp((char *)tape->format,"D") == 0) write_rtn=&db_write;
  else if (strcmp((char *)tape->format,"FB") == 0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"F") == 0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"U") ==0) write_rtn=&u_write;
  else if (strcmp((char *)tape->format,"VB") == 0 && tape->op[0] == 'D')
   write_rtn=&vb_write;
  else if (strcmp((char *)tape->format,"V") == 0 && tape->op[0] == 'D')
   write_rtn=&vb_write;
  else goto bad_type;                          /* oh well */
  tape->blocks=tape->arec_len=0;               /* initialize block count */
  return (write_rtn);                          /* return write routine */
 }


static WRITE_RTN wrt_toshdr(struct deviceinfo *tape) 
 {
  WRITE_RTN write_rtn;
  int hdr_posn, i, len, cur;
  time_t timeval;
  struct tm *tm;
  struct tapestats tapes;
 
  time(&timeval);                          /* current date */
  tm=localtime(&timeval);                  /* in exploded format */
  if (tape->datecheck==ON && tape->cur_expiration>0) /* check expiration date*/
   {
    cur=(tm->tm_year*1000)+tm->tm_yday+1;  /* yep - current date julian */
    if (cur<=tape->cur_expiration)          /* expired? */
     {
      issue_error_message("Attempt to overwrite an unexpired data set.\n");
      return (NULL);                       /* no - tell user done */
     }
   }  
  tapestatus(tape,&tapes);                 /* get info current position */
  tape->position=(tapes.mt_fileno/3)+1;    /* logical = (phys/3)+1 */
  hdr_posn=((tape->position-1)*3)+1;       /* hdr = (( logical -1)*3)+1 */
  tape->lp=HEADER_RTN;                     /* say in header routine */
  if (tape->newfile_name == OFF) tape->file_name[0]='\0';/*if FNAME not set reset */
  posn(tape,hdr_posn);                     /*position to label area */
  tape->newfile_name=0;                    /* reset tapename set flag */
  tapestatus(tape,&tapes);                 /* info current position */
  tape->lp=ON;                             /* enable label processing */
  memset(tape->hdr1,' ',80);               /* Zap HDR1 area */
  memcpy(tape->hdr1,"HDR1",4);             /* fill in HDR1 */
  len=strlen((char *)tape->file_name);     /* length of filename */
  if (len==0)                              /* no file name? */
   {
    sprintf((char *)tape->file_name,"V%s",tape->volume); /* set to */
    for (i=0;i<7;i++)                      /* Vvolume.Fpositon */
     if (tape->file_name[i]==' ') break;
     else len++;
    sprintf((char *)&tape->file_name[len],".F%05d",tape->position);
   }
  sprintf((char *)tape->buffer,"%-17.17s",tape->file_name); /*fill in filename*/
  memcpy(&tape->hdr1[4],tape->buffer,17); 
  sprintf((char *)tape->buffer,"%6.6s",tape->volume); /* fill in volume name */
  memcpy(&tape->hdr1[21],tape->buffer,6);
  memcpy(&tape->hdr1[27],"0001",4);         /* always 0001 */
  sprintf((char *)tape->buffer,"%04d",tape->position); /* fill in file number */
  memcpy(&tape->hdr1[31],tape->buffer,4);
  sprintf((char *)tape->buffer,"%03d%03d",tm->tm_year,tm->tm_yday+1);/*fill in*/
  memcpy(&tape->hdr1[41],tape->buffer,6);             /* creation date */
  if (tape->hdr1[41]=='0') tape->hdr1[41]=' ';
  memcpy(&tape->hdr1[47],tape->expiration,6); /* fill in expiration date */
  tape->hdr1[53]='0';                       /* always 0 */
  memcpy(&tape->hdr1[54],"000000",6);       /* fill in block count - zero hdr */
  len=strlen(INSTALATION_NAME);             /* fill in instaltion name */
  if (len>13) len=13;
  memcpy(&tape->hdr1[60],INSTALATION_NAME,len);
  tape->hdr2[0]='\0';                       /* no hdr2 */
  if (tape->format[0]=='\0')                /* user set format? */
   {
    strcpy((char *)tape->format,"U");       /* no  use U(32767) */
    tape->block_len=32767;
    tape->rec_len=0;
   }
  if (validate_format(tape) == NULL) return(NULL); /* validate format */
  if (tapes.mt_fileno==0) tpwrite(tape,tape->vol1,80); /* write vol if nec */
  for (i=0;i<80;i++) tape->buffer[i]=ASCEBC[tape->hdr1[i]]; /* trans HDR1 */
  tpwrite(tape,tape->buffer,80);            /* write hdr1 */
  tapeweof(tape,1);                         /* terminate hdr file */
   /* set write routine */
  if (strcmp((char *)tape->format,"DBS") == 0) write_rtn=&dbs_write;
  else if (strcmp((char *)tape->format,"DS") == 0) write_rtn=&dbs_write;
  else if (strcmp((char *)tape->format,"DB") == 0) write_rtn=&db_write;
  else if (strcmp((char *)tape->format,"D") == 0) write_rtn=&db_write;
  else if (strcmp((char *)tape->format,"VBS") == 0) write_rtn=&vbs_write;
  else if (strcmp((char *)tape->format,"VS") == 0) write_rtn=&vbs_write;
  else if (strcmp((char *)tape->format,"VB") == 0) write_rtn=&vb_write;
  else if (strcmp((char *)tape->format,"V") == 0) write_rtn=&vb_write;
  else if (strcmp((char *)tape->format,"FBS") ==0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"FS") ==0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"FB") == 0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"F") == 0) write_rtn=&fb_write;
  else if (strcmp((char *)tape->format,"U") ==0) write_rtn=&u_write;
  else exit(2);                            /* sno */
  tape->blocks=tape->arec_len=0;           /* reset block count */
  return(write_rtn);                       /* return write routine */ 
 }


static char *validate_format(struct deviceinfo *tape)
 {
  int len;
  char *insert;
  
  if (strcmp((char *)tape->format,"DBS") != 0 && /*make sure we have a val for*/
      strcmp((char *)tape->format,"DS") != 0 &&
      strcmp((char *)tape->format,"DB") != 0 &&
      strcmp((char *)tape->format,"D") != 0 &&
      strcmp((char *)tape->format,"VBS") != 0 &&
      strcmp((char *)tape->format,"VS") != 0  &&
      strcmp((char *)tape->format,"VB") != 0 &&
      strcmp((char *)tape->format,"V") != 0  &&
      strcmp((char *)tape->format,"FBS") != 0 &&
      strcmp((char *)tape->format,"FB") != 0 &&
      strcmp((char *)tape->format,"F") != 0 &&
      strcmp((char *)tape->format,"U") !=0)
   {
    sprintf(message_area,"'%s' is an unknown format type.\n",tape->format);
    issue_error_message(message_area);
    return (NULL);                             /* error -tell user - return */
   }
  if (tape->tape_type != ANSI_LABEL && tape->tape_type!=UNLABELED)
   tape->blkpfxs = OFF;                 /*turn block prefixing  off FS and IBM*/
  if (tape->blkpfxs == OFF)             /* user set block prefixing? */
   {
    if (tape->format[0]=='D')          /* no  -  set the defaults */
     {                                  
      tape->blkpfx=4;                  /* default for D, DB, DS, and DBS */
      tape->blkpfxl=1;
     }          
    else {tape->blkpfx=tape->blkpfxl=0;} /* default all others */
   }
  len=tape->block_len - tape->blkpfx; /* space available in block */
  if (len < 1)                       /* can we fit any info in? */
   {    
    pfxerr:
    sprintf(message_area,"Block prefix too long - no room for records\n");
    issue_error_message(message_area);
    return (NULL);                  /* tell user and return */
   }
  else if (tape->format[0] == 'F')  /* validate F formats */
   {
    if (strcmp((char *)tape->format,"F") == 0 ||
        strcmp((char *)tape->format,"FS") == 0) 
     tape->rec_len=len;             /* format F and FS us this as rec len */
    if (tape->format[1] == 'B')     /* block format ? */        
     {
      if (len < tape->rec_len )     /* can we hold at lest on record */
        goto pfxerr;                /* no tell user and return */
       /* for FB format lrecl must be multiple of block len - prefix len */
      else if (tape->format[2] != 'S' && (len % tape->rec_len) !=0)        
       {
        if (tape->blkpfx != 0) insert=" - block prefix len"; /* it wasn't */
        else insert="";
        sprintf(message_area,
          "Block length%s length is not a multiple of record length.\n",insert);
        issue_error_message(message_area);
        return (NULL);
       }
     } 
   }
  else if (tape->format[0] == 'D')     /* validate format D's */
   {
    if (strcmp((char *)tape->format,"DS")==0 ||
        strcmp((char*)tape->format,"DBS")==0) 
     len-=5;                           /* remaining len in block for spanned  */
    else 
     {
      if (tape->rec_len > len)       /* room for at least on record */ 
       {
        sprintf(message_area,"(Block length - Block prefix length) < record length\n");
        issue_error_message(message_area);
        return (NULL);                 /* tell user and return */
       }
     }
    if (len <1) goto pfxerr;          
   } 
  return ((char *)tape->format);       /* looks fine */
 }


static int wrt_notlr(struct deviceinfo *tape)
 {                        /* can't get much easier than this */
  return 0;
 }


static int wrt_ibmtlr(struct deviceinfo *tape)
 {
  int  i;
  
  if (tape->eov == ON)                   /* write EOV trailer? */
   { 
    memcpy(tape->hdr1,"EOV1",4);         /* change HDR1 to EOV1 */
    memcpy(tape->hdr2,"EOV2",4);         /* change HDR2 to EOV2 */
   }
  else                                   /* write EOF trailer */
   { 
    memcpy(tape->hdr1,"EOF1",4);         /* change HDR1 to EOF1 */
    memcpy(tape->hdr2,"EOF2",4);         /* change HDR2 to EOF2 */
   }
  sprintf((char *)tape->buffer,"%06d",tape->blocks); /*char form # of blocks*/
  memcpy(&tape->hdr1[54],tape->buffer,6);  /* put in EOF1 record */
  for (i=0;i<80;i++) tape->buffer[i]=ASCEBC[tape->hdr1[i]]; /* EOF1 in EBCDIC */
  tpwrite(tape,tape->buffer,80);       /* write EOF1 */
  for (i=0;i<80;i++) tape->buffer[i]=ASCEBC[tape->hdr2[i]]; /* EOF2 in EBCDIC */
  tpwrite(tape,tape->buffer,80);       /* write EOF2 */
  tapeweof(tape,1);                    /* write Tape Mark */
  return 0;                              /* done */
 }


static int wrt_ansitlr(struct deviceinfo *tape)
 {
  if (tape->eov == ON)                   /* write EOV trailer? */
   { 
    memcpy(tape->hdr1,"EOV1",4);         /* change HDR1 to EOV1 */
    memcpy(tape->hdr2,"EOV2",4);         /* change HDR2 to EOV2 */
   }
  else                                   /* write EOF trailer */
   { 
    memcpy(tape->hdr1,"EOF1",4);         /* change HDR1 to EOF1 */
    memcpy(tape->hdr2,"EOF2",4);         /* change HDR2 to EOF2 */
   }
  sprintf((char *)tape->buffer,"%06d",tape->blocks); /*char form # of blocks */
  memcpy(&tape->hdr1[54],tape->buffer,6); /* put in EOF1 record */
  tpwrite(tape,tape->hdr1,80);         /* write EOF1 */
  tpwrite(tape,tape->hdr2,80);         /* write EOF2 */
  tapeweof(tape,1);                    /* write Tape Mark */
  return 0;                              /* done */
 }


static int wrt_tostlr(struct deviceinfo *tape)
 {
  int  i;
  
  if (tape->eov == ON)                   /* write EOV trailer? */
   { 
    memcpy(tape->hdr1,"EOV1",4);         /* change HDR1 to EOV1 */
   }
  else                                   /* write EOF trailer */
   { 
    memcpy(tape->hdr1,"EOF1",4);         /* change HDR1 to EOF1 */
   }
  sprintf((char *)tape->buffer,"%06d",tape->blocks); /* char form # of blocks */
  memcpy(&tape->hdr1[54],tape->buffer,6); /* put in HDR1 record */ 
  for (i=0;i<80;i++) tape->buffer[i]=ASCEBC[tape->hdr1[i]]; /* HDR1 to EBCDIC */
  tpwrite(tape,tape->buffer,80);      /* write EOF1 */
  tapeweof(tape,1);                   /* write tape mark */
  return 0;                             /* done */
 }


static unsigned char *vbs_read(struct deviceinfo *taper,struct buf_ctl *buf_ctl)
 {
#define IBM_FIRST 0x01
#define IBM_MID 0x03
#define IBM_LAST 0x02 
  unsigned short int dw;
  unsigned char span;
 
  span=0;
  if (taper->rem_len==0)                    /* read a block? */
   {
    taper->rem_len=tpread(taper,taper->buffer,32768*2); /* yep - get block */
    if (taper->rem_len==0) {tpsync(taper);return (NULL);} /* return on EOF */
    else if (taper->rem_len<0)                  /* read error? */
     return(tape_err(taper,"Read","VS/VBS",taper->blocks,errno));  /* yep - too bad */
    dw = etohs(*((unsigned short *)taper->buffer));             /* pick up BDW */
    if (taper->rem_len==18) taper->rem_len=dw;/* if len read=18 substitute BDW*/
    taper->offset=4;                       /* offset of first record */
    taper->rem_len -=4;                    /* calc remaining length in block */
    taper->blocks++;                       /* incr physical block count */
    buf_ctl->blocks++;                     /* incr relative block count */
   }
  memcpy(&dw,taper->buffer+taper->offset,2); /* pick up RDW */
  dw = etohs(dw);                            /* invert RDW if necessary */ 
  memcpy(&span,taper->buffer+taper->offset+2,1); /* pick up span byte */
  buf_ctl->seg_len=dw-4;                   /* length of segment = LRECL-4 */
  buf_ctl->bufaddr=&taper->buffer[taper->offset+4]; /* point to start of seg */
  taper->rem_len -= dw;                    /* calc reamaining length in block */
  taper->offset += dw;                     /* calc offset of next pos segment */
  if (span & IBM_FIRST)  buf_ctl->eor=0;   /* End of Record? */
  else buf_ctl->eor=1;                     /* yep */
  if (taper->rem_len<0)                    /* run off end of block */
   {
    sprintf(message_area,"Block %d error - ran off end of block\n",
            buf_ctl->blocks);             /* too bad - tell user */
    issue_error_message(message_area);
    return (NULL);                        /* stop processiong (indicate EOF) */
   }
  return (buf_ctl->bufaddr);              /* return address of segment */
 }


static unsigned char *vb_read(struct deviceinfo *taper,struct buf_ctl *buf_ctl)
 {
  unsigned short int dw;

  if (taper->rem_len==0)                 /* read new block? */
   {
    taper->rem_len=tpread(taper,taper->buffer,32768*2); /*yep - read new block*/
    if (taper->rem_len==0) {tpsync(taper);return (NULL);} /* return on EOF */
    else if (taper->rem_len<0)           /* read error? */
     return(tape_err(taper,"Read","V/VB",taper->blocks,errno)); /* yep - too bad */
    dw = etohs(*((unsigned short *)taper->buffer));           /* get  block descriptor word */ 
    if (taper->rem_len==18) taper->rem_len=dw; /* if len=18 use BDW instead */
    if (taper->tape_type==ANSI_LABEL)    /* should illegal but MTS allows */
     {
      taper->offset=taper->blkpfx;       /* point to start of record */
      taper->rem_len-=taper->blkpfx;     /* calc remaining length in block */
     }
    else
     {
      taper->offset=4;                   /* point to start of record */
      taper->rem_len-=4;                 /* calc remaining length in block */
     }
    taper->blocks++;                     /* incr physical block count */
    buf_ctl->blocks++;                   /* incr relative block count */
   }
  memcpy(&dw,taper->buffer+taper->offset,2); /* Get RDW */
  dw = etohs(dw);                        /* invert RDW if necessary */ 
  buf_ctl->seg_len=dw-4;                 /* record length = LRECL -4 */
  buf_ctl->bufaddr=&taper->buffer[taper->offset+4]; /* point to start of rec */
  buf_ctl->eor=1;                        /* mark end of record */
  taper->rem_len -= dw;                  /* remaining length in block */
  taper->offset += dw;                   /* next possible record */
  if (taper->rem_len<0)                  /* off end of block? */
   {
    sprintf(message_area,"Block %d error - ran off end of block\n",
            buf_ctl->blocks);            /* yep tell user */
    issue_error_message(message_area);
    return (NULL);                       /* stop processing ( return EOF) */
   }
  return (buf_ctl->bufaddr);             /* return start of record */
 }


static unsigned char *fb_read(struct deviceinfo *taper,struct buf_ctl *buf_ctl)
 {
  div_t d;
  int rec_len;

  rec_len=taper->rec_len;               /* get record length */
  if (taper->rem_len == 0)              /* read new block? */
   {
    taper->rem_len=tpread(taper,taper->buffer,32768*2); /* yep - get new block*/
    if (taper->rem_len==0) {tpsync(taper);return (NULL);} /* return on EOF */
    else if (taper->rem_len<0)          /* read error? */                
     return(tape_err(taper,"Read","F/FB/FS/FBS",taper->blocks,errno));/* yep too bad */
    taper->rem_len-= taper->blkpfx;     /* calc remaining length in block */
    if (rec_len==0) rec_len=taper->block_len-taper->blkpfx; /* rec len F or FS*/
    d=div(taper->rem_len,rec_len);      /* should be an integer if proper */
    if (d.rem != 0)                     /* is it? */
     {
      sprintf(message_area,"Block %d error - not multiple of LRECL.\n",
                     buf_ctl->blocks+1); /* no too bad */
      issue_error_message(message_area);
      return (NULL);                    /* stop processing - return EOF */
     }
    taper->offset = taper->blkpfx;      /* point to 1st record */
    buf_ctl->blocks++;                  /* incr relative block number */ 
    taper->blocks++;                    /* incr physical block number */
   }
   buf_ctl->bufaddr=&taper->buffer[taper->offset]; /* point to record */
   buf_ctl->seg_len=rec_len;            /* set length of record */
   buf_ctl->eor=1;                      /* mark end of record */
   taper->rem_len -= rec_len;           /* remaining length in block */
   taper->offset += rec_len;            /* point to next pos record */
   return (buf_ctl->bufaddr);           /* return pointer to record */
 }


static unsigned char *ru_read(struct deviceinfo *taper, struct buf_ctl *buf_ctl)
 {

  buf_ctl->seg_len=tpread(taper,taper->buffer,32768*2); /* read a block */
  if (buf_ctl->seg_len==0) {tpsync(taper);return (NULL);} /* return if EOF */
  else if (buf_ctl->seg_len<0)             /* read error? */             
   return(tape_err(taper,"Read","unformatted",taper->blocks,errno)); /* yep - too bad */
  buf_ctl->bufaddr=taper->buffer;          /* point to block read */
  buf_ctl->eor=1;                          /* treat as though a redcord */
  buf_ctl->blocks++;                       /* incr relative block number */
  taper->blocks++;                         /* incr physical block number */
  taper->offset=0;                     /* reset - not used for this read type */
  taper->rem_len=0;                    /* reset - not used for this read type */
  return (taper->buffer);                  /* return pointer to "record" */
 }


static unsigned char *u_read(struct deviceinfo *taper, struct buf_ctl *buf_ctl)
 {
  
  buf_ctl->seg_len=tpread(taper,taper->buffer,32768*2); /*read a block */
  if (buf_ctl->seg_len>buf_ctl->max_blk) buf_ctl->max_blk=buf_ctl->seg_len;
  if (buf_ctl->seg_len==0) {tpsync(taper);return (NULL);} /* return if EOF */
  else if (buf_ctl->seg_len<0)             /* read error? */              
   return(tape_err(taper,"Read","unformatted",taper->blocks,errno));  /* yep - too bad */
  buf_ctl->bufaddr=taper->buffer+taper->blkpfx; /* point to start of record */
  buf_ctl->seg_len-=taper->blkpfx;         /* adjust the record length */     
  buf_ctl->eor=1;                          /* mark as end of record */
  buf_ctl->blocks++;                       /* incr realative block number */
  taper->blocks++;                         /* incr physical block number */
  taper->offset=0;                     /* reset - not used for this read type */
  taper->rem_len=0;                    /* reset - not used for this read type */
  return (buf_ctl->bufaddr);               /* return pointer to record */
 }


static unsigned char *dbs_read(struct deviceinfo *taper,struct buf_ctl *buf_ctl)
 {
#define ANSI_FIRST '1'
#define ANSI_MID   '2'
#define ANSI_LAST  '3'
  char buf[5];
  int seg_len;
 
  if (taper->rem_len==0)                     /* read New block? */
   { 
    reread:
    taper->rem_len=tpread(taper,taper->buffer,32768*2); /* yep - get vlock */
    if (taper->rem_len==0) {tpsync(taper);return (NULL);} /* return on EOF */
    else if (taper->rem_len<0)               /* read error */
     return(tape_err(taper,"Read","DS/DBS",taper->blocks,errno)); /* yep - too bad */
    taper->offset=taper->blkpfx;             /* point to first record */
    taper->rem_len-=taper->blkpfx;          /* calc remaining length in block */
    taper->blocks++;                         /* incr physical block number */
    buf_ctl->blocks++;                       /* incr relative block number */
   }
  buf[4]='\0';
  strncpy(buf,(char *)&taper->buffer[taper->offset+1],4); /* copy RDW */
  if (isdigit(buf[0]) == 0) goto reread; /* not a digit-assume padded - reread*/
  seg_len=atoi(buf);                         /* transform to machine number */
  buf_ctl->seg_len=seg_len-5;                /* segment length = LRECL - 5 */
  buf_ctl->bufaddr=&taper->buffer[taper->offset+5]; /* point to start of rec */
  buf_ctl->eor=0;                            /* mark as not end of record */
  if (taper->buffer[taper->offset]==ANSI_LAST || /* if last seg or only seg */
      taper->buffer[taper->offset]=='0') buf_ctl->eor=1; /*mark as end of rec */
  taper->rem_len -= seg_len;              /* calc remaining length this block */
  taper->offset += seg_len;               /* point to next segment in block */
  if (taper->rem_len<0)                   /* this indicates soething wrong */
   {
    sprintf(message_area,"Block %d error - ran off end of block\n",
            buf_ctl->blocks);             /* tell user */
    issue_error_message(message_area);
    return (NULL);                        /* stop reading (indicate EOF) */
   }
  return (buf_ctl->bufaddr);              /* return address of segment read */
 }


static unsigned char *db_read(struct deviceinfo *taper,struct buf_ctl *buf_ctl)
 {
  char buf[5];
  int seg_len;

  if (taper->rem_len==0)                 /* processed all data in buffer? */
   {
    reread: 
    taper->rem_len=tpread(taper,taper->buffer,32768*2); /* read a block */
    if (taper->rem_len==0) {tpsync(taper);return (NULL);} /* EOF? */
    else if (taper->rem_len<0)                          /* Error? */
     return(tape_err(taper,"Read","D/DB",taper->blocks,errno));   /* too bad */
    taper->offset=taper->blkpfx;        /* point to start of first record */
    taper->rem_len-=taper->blkpfx;      /* set remaining length in block */
    taper->blocks++;                    /* incr phsycal block number */
    buf_ctl->blocks++;                  /* incr relative block number */
   }
  buf[4]='\0';                          /* insert string end char */
  strncpy(buf,(char *)&taper->buffer[taper->offset],4); /* copy the RDW */
  if (isdigit(buf[0]) == 0) goto reread; /*  not a digit - assume pad - reread*/
  seg_len=atoi(buf);                    /* transform to number */
  buf_ctl->seg_len=seg_len-4;           /* len of record = LRECL -4 */
  buf_ctl->bufaddr=&taper->buffer[taper->offset+4]; /* point to start of rec */
  buf_ctl->eor=1;                       /* set end of Record condition */
  taper->rem_len -= seg_len;            /* calc remaining length in block */
  taper->offset += seg_len;             /* offset to next poential recore */
  if (taper->rem_len<0)                 /* This would be a good indicatation */
   {
    sprintf(message_area,"Block %d error - ran off end of block\n",
            buf_ctl->blocks);           /* of a serious error */
    issue_error_message(message_area);
    return (NULL);                      /* stop processing  (signal EOF) */
   }
  return (buf_ctl->bufaddr);            /* return pointer to segment. */
 }


extern unsigned char *tape_err(struct deviceinfo *tape, char *type,char *type2,int block, int error)
 {
  sprintf(message_area,"%s error, during an %s operation, block %d, errno %d\n",
          type,type2,block,error);
  issue_error_message(message_area);
  issue_ferror_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
  tape->fatal=1;
  return (NULL);
 }


static unsigned char *unix_read(struct deviceinfo *filei,struct buf_ctl *buf_ctl) 
 {
  unsigned char *rc, *eol;
  
  if (filei->data_mode==RECORD)                  /* in record mode? */
   {
    buf_ctl->sor=buf_ctl->eor=1;     /* set Start 0f Record and End Of Record */
    memset(filei->buffer,'\n',32768*2);
    rc=(unsigned char *)fgets((char *)filei->buffer,32768*2,filei->unixfile);
    if (rc != NULL)
     {
      eol = (unsigned char *)memchr(&filei->buffer[0],'\n',32768*2);
      if (eol == NULL) buf_ctl->seg_len=(32768*2)-1;
      else if (eol == &filei->buffer[32768*2-1]) buf_ctl->seg_len=(32768*2)-2;
      else if (*(eol+1) == '\0')
       buf_ctl->seg_len=eol-rc; /* strip trail \0\n */
      else
       buf_ctl->seg_len=eol-rc-1; /* strip trailing \0\n */
     }
   } 
  else                                          /* must be stream mode */
   {
    buf_ctl->seg_len=fread(filei->buffer,1,32768,filei->unixfile); /* read */
    if (buf_ctl->seg_len==0)                    /* EOF? */
     {
      buf_ctl->eor=1;                           /* Yep - return EOR */
      rc=NULL;                                  /* no string read */
     }
    else
     {
      buf_ctl->eor=0;                           /* not EOR */
      rc=filei->buffer;                         /* point to seg */
     }
   }
  return (rc);                                  /* return buffer address */
 }


READ_RTN fmtstring(unsigned char *format,struct buf_ctl *buf_ctl)
{
  char *opar, *cpar, *comma;
  READ_RTN read_rtn;
  int i, len; 
  
  buf_ctl->format[0]='\0';                    /* no format yet */
  opar=strpbrk((char*)format,"(");            /* look for starting ( */
  if (opar == NULL)                           /* was there one */
   {
    if (strcmp((char *)format,"MTS*FS") == 0) /* set to *FS format? */
     {
      buf_ctl->fs=1;                          /* yep */
      return (&mts_fsread);                   /* return read routine */
     }
    else 
     {
      sprintf(message_area,
       "Format must be in form \"fmt(spec)\" - ( not found.\n");
      issue_error_message(message_area);
      return (NULL);                         /* invalid format */
     }
   } 
  comma=strpbrk(&*(opar+1),",");              /* find comma */ 
  if (comma == NULL) cpar=strpbrk(&*(opar+1),")"); /* look for ending ) */
  else cpar=strpbrk(&*(comma+1),")");         /* look for ending ) after comma*/
  if (cpar == NULL)                        /* error if no ) */
   {
    sprintf(message_area,
     "Format must be in form \"fmt(spec)\" - ) not found.\n");
    issue_error_message(message_area);
    return (NULL);                         /* invalid format */
   }
  if(*(cpar+1) != '\0')                    /* error if ) not last*/
   {
    sprintf(message_area,
     "Format must be in form \"fmt(spec)\" - characters found after ).\n");
    issue_error_message(message_area);
    return (NULL);                         /* invalid format */
   }
  *opar=*cpar='\0';                        /* turn format portion into string */
  if (comma != '\0') *comma='\0';          /* turn block size into string */
  if (strspn(&*(opar+1),"0123456789") == strlen(&*(opar+1))) /* val block size*/
   buf_ctl->blk_len=atoi(&*(opar+1));      /* turn block size into number */
  else                                     /* bad block size */
   {
    sprintf(message_area,
     "Non numberic characters found in block size spec.\n");
    issue_error_message(message_area);
    goto fmterr;                         /* invalid format */
   }
  buf_ctl->rec_len=0;                      /* initial lrecl */
  if (comma != NULL)                       /* was a LRECL specified? */
   {
    if (strspn(&*(comma+1),"0123456789") == strlen(&*(comma+1))) /* yep - val */
     buf_ctl->rec_len=atoi(&*(comma+1));   /* and turn into number */
    else                                   /* bad lrecl */
     {
      sprintf(message_area,
       "Non numberic characters found in logical record length spec.\n");
      issue_error_message(message_area);
      goto fmterr;                         /* invalid format */
     }
   }
  if (buf_ctl->blk_len<=0)                 /* validate block size range */
   {
    sprintf(message_area,"Block size must be greater than 0.\n");
    issue_error_message(message_area);
    goto fmterr;                           /* too small */
   }
  else if (buf_ctl->blk_len > 65535)       /* validate block size range */
   {
    sprintf(message_area,"Block sizes greater than 65535 are not supported.\n");
    issue_error_message(message_area);
    goto fmterr;                           /* too large */
   }
  else if (buf_ctl->warn== ON && buf_ctl->blk_len > 32760) /*issue warning if > MVS max*/
   {
    sprintf(message_area,"Block sizes greater than 32760 can cause severe handling prolbems on some platforms.\n");
    issue_warning_message(message_area);
   }
  len=strlen((char *)format);              /* get length of format type */
  for (i=0; i<= len; i++) format[i]=toupper(format[i]); /* Make sure in uppr */
  buf_ctl->fmchar[0]='\0';                 /* initial ougoing format */
  if (len>1 && (format[len-1]=='A' || format[len-1]=='M')) /* cc specified? */
   {   
    len--;                                 /* yes - decr length of format */
    buf_ctl->fmchar[0]=format[len];        /* save char her for future ref */
    format[len]='\0';                      /* change CC into \0 */
   }
       /* See if format DBS or format DB */
  if (strcmp((char *)format,"DBS") == 0 || strcmp((char *)format,"DS") == 0)
   {
    if (buf_ctl->rec_len < 6)
     {
      sprintf(message_area,"Record length must be larger than 6.\n");
      issue_error_message(message_area);
      goto fmterr;   
     }
    if (buf_ctl->blk_len < 6)       /*block length resonable?*/
     {
      sprintf(message_area,"Block length must be 6 or greater\n");
      issue_error_message(message_area);
      goto fmterr;   
     }
    read_rtn=&dbs_read;                    /* set appropriate read routine */
   }
       /* See if format DB or format D */
  else if (strcmp((char *)format,"DB") == 0 || strcmp((char *)format,"D") == 0)
   {
    if (buf_ctl->rec_len < 5)
     {
      sprintf(message_area,"Record length must be larger than 5.\n");
      issue_error_message(message_area);
      goto fmterr;   
     }
    if (buf_ctl->rec_len > buf_ctl->blk_len)
     {
      sprintf(message_area,"Block length must be larger than record length\n");
      issue_error_message(message_area);
      goto fmterr;             /* block size or record length not reasonable */
     }
    read_rtn=&db_read;                     /* set appropriate read routine */
   }
  else if (strcmp((char *)format,"VBS") == 0 || 
           strcmp((char *)format,"VS") == 0)   /* VBS or VS format */ 
   {
    if (buf_ctl->rec_len < 5)
     {
      sprintf(message_area,"Record length must be larger than 5.\n");
      issue_error_message(message_area);
      goto fmterr;   
     }
    if(buf_ctl->blk_len < 9)
     {
      sprintf(message_area,"Block length must be 9 or greater\n");
      issue_error_message(message_area);
      goto fmterr;            /* block size or record len not reasonable */
     }
    read_rtn=&vbs_read;                    /* set appropriate read routine */
   }
  else if (strcmp((char *)format,"VB") == 0 ||
           strcmp((char *)format,"V") == 0)  /* format VB or V */
   {
    if (buf_ctl->rec_len < 5)
     {
      sprintf(message_area,"Record length must be larger than 5.\n");
      issue_error_message(message_area);
      goto fmterr;   
     }
    if (buf_ctl->rec_len > buf_ctl->blk_len - 4)
     {
      sprintf(message_area,"Block length must be at least 4 greater than record length\n");
      issue_error_message(message_area);
      goto fmterr;           /* block size or record length not reasonable */
     }
    read_rtn=&vb_read;                    /* set appropriate read routine */
   }
  else if (strcmp((char *)format,"FB") == 0 ||
           strcmp((char *)format,"FBS") == 0) /* format FB or FBS */
   {
    if (buf_ctl->rec_len == 0)
     {
      sprintf(message_area,"Record length must be larger than 5.\n");
      issue_error_message(message_area);
      goto fmterr;   
     }
    if (buf_ctl->rec_len>buf_ctl->blk_len)
     {
      sprintf(message_area,"Block length must be greater than record length\n");
      issue_error_message(message_area);
      goto fmterr;                    /* block and record length reasonable? */
     }
    read_rtn=&fb_read;                   /* set appropriate read routine */ 
   }
  else if (strcmp((char *)format,"F") == 0 ||
           strcmp((char *)format,"FS")== 0)  /* F or FS format? */
   {
    if (comma != NULL)                   /* error if LRECL specified */
     {
      sprintf(message_area,"F and FS formats usually don't take record lengths.\n"); 
      issue_warning_message(message_area);
      goto fmterr;                    /* block and record length reasonable? */
     }
    buf_ctl->rec_len=0;                    /* set recl ot 0  - none spedifed */
    strcpy((char *)buf_ctl->format,(char *)format); /* make format available */
    read_rtn = &fb_read;                   /* return appropriate read routine */
   }
  else if (strcmp((char *)format,"U") == 0) /* format U */
   {
    if (buf_ctl->rec_len>buf_ctl->blk_len)  /* val block and rec */
     {
      sprintf(message_area,"Block length must be greater than record length\n");
      issue_error_message(message_area);
      goto fmterr;                    /* block and record length reasonable? */
     }
    read_rtn=&u_read;                    /* set appropiate read routine */   
   }
  else                                   /* unknown format entered */
   {
    sprintf(message_area,"\"%s\" is an unkown format coding.\n", format);
    issue_error_message(message_area);
    goto fmterr;
   }
  if ((format[0] == 'V' || format[0] == 'D') && buf_ctl->rec_len==0)
    goto fmterr;                        /*lrecl specified?*/
  strcpy((char *)buf_ctl->format,(char *)format); /* make format available */
  if (comma != NULL) *comma=',';        /* reset . */
  *opar='(';                            /* reset ( */
  *cpar=')';                            /* reset ) */
  return (read_rtn);                     /* return appropiate read routine */
 fmterr:
  if (comma != NULL) *comma=',';        /* reset . */
  *opar='(';                            /* reset ( */
  *cpar=')';                            /* reset ) */
  return (NULL);                        /* return NULL (error) */
 }


static int vbs_write(struct deviceinfo *device, const unsigned char *buf,unsigned int size)
 {
  short unsigned int dw;
  int move_size, rem_len, offset, len;

  if (device->offset==0)                /* starting a new block? */
   {vb_startblk(1)}                     /* yep - initialize */
  else if (device->seg_start==&device->buffer[device->offset]) /* new rec? */
   {
    device->offset += 4;                /* point to start of rec */
    device->rem_len -= 4;               /* remaining length in block */
    memset(device->seg_start,'\0',4);   /* zap RDW */
    dw=4;                               /* set RDW */
    memcpy(device->seg_start,&dw,2);
    dw=device->offset;                  /* set BDW */
    memcpy(device->buffer,&dw,2);
   }
  len=device->arec_len+size;      /* len of actual record  - so far */
  if (device->data_mode== RECORD && device->rec_len<99999)
   {
    if (len > device->rec_len-4)    /* if larger than specified truncate */
     {
      issue_warning_message(
          "Warning record longer that record length - truncation will occur\n");
      size=device->rec_len-4-device->arec_len; /* move just this  amount */
     }
   }
  device->seg_len=rem_len=size;         /* amount of data to move */
  offset=0;                             /* offset into input buffer */
  do                                    /* cycle over data */
   {
    if (rem_len > device->rem_len)      /* can we move whole segment */
     {
      move_size=device->rem_len;        /* no - just this amount */
      rem_len-=device->rem_len;         /* ammount left unmoved  */
     }
    else
     {
      move_size=rem_len;                /* move rest of segment */ 
      rem_len=0;                        /* everything moved */ 
     }
    memcpy(&device->buffer[device->offset],buf+offset,move_size); /* move */
    memcpy(&dw,device->seg_start,2);    /* udpate RDW */  
    dw += move_size;
    memcpy(device->seg_start,&dw,2);
    device->arec_len += move_size;      /* update physical record length */
    device->offset += move_size;        /* udpate off set into block */
    device->rem_len -= move_size;       /* update remaing block length */
    offset += move_size;                /* update offset into input buffer */
    dw=device->offset;                  /* update BDW */
    memcpy(device->buffer,&dw,2);       
    if (rem_len > 0)                    /* last seg?  */
     { 
      if (device->data_mode==RECORD && device->seg_start[2] == '\0') /*1st seg?*/
       device->seg_start[2] = IBM_FIRST; /* yep - mark */
      tflush(device,0);                 /* flush out this part */
      vb_startblk(1);                   /* start new block */
      if (device->data_mode==RECORD)    /* if record mode mark as middle seg */
       device->seg_start[2] = IBM_MID;
     }
   } while (rem_len>0);
  if (device->data_mode==RECORD && device->eor == 1) /*eor and in record mode?*/
   {
      /* mark as last seg ment if not only segment */
    device->arec_len=0;
    if (device->seg_start[2] != '\0') device->seg_start[2] = IBM_LAST; 
    if (device->hdr2[38]=='S' || device->rem_len <5) 
     {     
      tflush(device,0);                  /* flush if unlocked or out of room */
      return (0);                        /* done */
     }
#ifdef BYTE_SWAPPED        /* need to invert RDW? */
       {
        unsigned char temp;

        temp=device->seg_start[0];   /* yep */
        device->seg_start[0]=device->seg_start[1];
        device->seg_start[1]=temp;
       }
#endif
    device->seg_start=&device->buffer[device->offset]; /* point to new record */
   }
  return (0);                        /* done */
 }


static int vb_write(struct deviceinfo *device, const unsigned char *buf,unsigned size)
 {
  short unsigned int dw;
  int offset,len,i,move_size,rem_len;

  if (device->offset == 0 || device->seg_start==&device->buffer[device->offset])
   device->arec_len=0;
  len=device->arec_len+size;      /* len of actual record  - so far */
  if (device->data_mode== RECORD)
   {
    if (len > device->rec_len-4)    /* if larger than specified truncate */
     {
      issue_warning_message( 
          "Warning record longer that record length - truncation will occur\n");
      move_size=size=device->rec_len-4-device->arec_len; /* move just this */
     }
    else
     move_size=size;              /* move  entire segment. */
    device->seg_len=move_size;
   }
  else
   {
    device->seg_len=size;
    move_size=size;              /* move entire buffer? */
    rem_len=device->rec_len-4-device->arec_len; /* space left in record */
    if (move_size>rem_len) move_size=rem_len;   /* move just this if < buff */
    if (move_size>device->rem_len) move_size=device->rem_len; /*most we can do*/
   }
  rem_len=size;                 /* amount of data to preocess */  
  offset=0;                     /* initialize offset into buffer */
  do
   {
    if (device->offset==0)      /* starting a new block */
     {vb_startblk(0)}
    else if (device->seg_start==&device->buffer[device->offset]) /* new rec? */
     {
      device->offset += 4;                /* where to start new rec */
      memset(device->seg_start,'\0',4);   /* zap RDW */
      dw=4;                               /* initial RDW lenght */
      device->arec_len=0;                 /* new record - no length yet */
      memcpy(device->seg_start,&dw,2);    /* init RDW */
      dw=device->offset;                  /* BDW length*/
      memcpy(device->buffer,&dw,2);       /* update BDW */
      device->rem_len-=4;                 /* update remaining length in block */
      if (device->data_mode != RECORD &&
          move_size>device->rem_len) move_size=device->rem_len; /*most we can do*/
     }
    if (device->data_mode == RECORD && rem_len > device->rem_len)
                  /* will it still fit in current block? */ 
     {   
      int offset;      /* No - Write out current block - start new block */ 
      memcpy(&dw,device->seg_start,2); /* write out finished portion of block */
      len=dw;         /* Must move front portion to new block - length */
      device->offset-=len; /* adjust offset/length of current block */
      offset=device->offset; /* save offset for latter move */
      dw=device->offset;     /* Block length current block */
      memcpy(device->buffer,&dw,2);      /* update */
      tflush(device,0);                  /* write current Block */
      vb_startblk(0);                    /* prepare new  block */
      dw=len;
      memcpy(device->seg_start,&dw,2);
      for (i=2;i<len-2;i++)                /* move in unwritten record part */ 
       device->seg_start[i]=device->buffer[offset+i];
      device->offset=4+len;              /* BDW length new block */
      dw=device->offset;
      memcpy(device->buffer,&dw,2);      /* update BDW */
      device->rem_len=device->block_len-dw; /* space remaining in block */
     }
             /* add current seg to block */
    memcpy(&device->buffer[device->offset],buf+offset,move_size); /* move seg */
    memcpy(&dw,device->seg_start,2);   /* Update RDW length*/
    dw += move_size;
    device->arec_len=dw-4;             /* Update actual record length */
    memcpy(device->seg_start,&dw,2);   /* Update RDW */
    device->offset += move_size;       /* Update offset current block */
    offset += move_size;               /* Update pointer input buffer */
    device->rem_len -= move_size;      /* Update rem length current block */
    dw=device->offset;                 /* Update BDW length */
    memcpy(device->buffer,&dw,2);      /* Update BDW */
    rem_len -= move_size;              /* Update amount of data to process */
    if (device->data_mode == RECORD && device->eor == 1)   /* End of Record? */ 
     {
      if (device->hdr2[38]==' ' || device->rem_len < 5) 
       {     
        tflush(device,0);             /* flush if Unblocked or out of room */
        return (0);                   /* done */
       }
#ifdef BYTE_SWAPPED        /* need to invert RDW? */
       {
        unsigned char temp;

        temp=device->seg_start[0];   /* yep */
        device->seg_start[0]=device->seg_start[1];
        device->seg_start[1]=temp;
       }
#endif
      device->seg_start=&device->buffer[device->offset]; /* start of next rec */
     }
    else if (device->data_mode == STREAM)
     {
      if (device->rem_len == 0 || device->arec_len == device->rec_len-4)
       {
        if (device->hdr2[38]==' ' || device->rem_len < 5) 
         tflush(device,0);           /* flush if block full */ 
        else
         {
#ifdef BYTE_SWAPPED        /* need to invert RDW? */
           {
            unsigned char temp;

            temp=device->seg_start[0];   /* yep */
            device->seg_start[0]=device->seg_start[1];
            device->seg_start[1]=temp;
           }
#endif
          device->seg_start=&device->buffer[device->offset]; /* start new record */
         }
       }
      move_size=rem_len;            /* amount of data left */
      if (move_size > device->rec_len -4)  
       move_size=device->rec_len-4;   /* move at most this amount at a time */
      if (move_size > device->rem_len - 4)
       move_size=device->rem_len -4;  /* woops - can only move this amount */
     }
   } while (rem_len>0);
  return(0);
 }


static int fb_write(struct deviceinfo *device, const unsigned char *buf,unsigned size)
 {
  int len;
  int rem_len, offset, move_size;
   
  if (device->offset==0)         /* starting a new block */
   device->arec_len=0;
  len=device->arec_len + size;   /* size of current record */
  if (device->data_mode == RECORD)
        /* if larger than specified record length truncate */
   {
    device->seg_len=0;
    if (len > device->rec_len)
     {
      issue_warning_message(
          "Warning record longer that record length - truncation will occur\n");
      size=device->rec_len - device->arec_len;  /* move just this amount. */
     }
   }
  else
   device->seg_len=size;
  rem_len=size;                /* amount of data in input buffer to process */
  offset=0;                    /* offset into input buffer */
  do 
   {
    if (device->offset==0)         /* starting a new block? */
     {
      device->arec_len=0;          /* reset actual record length */
      if (device->blkpfx >0 )      /* set block prefix if necessary */
       {
        memset(device->buffer,'^',device->blkpfx);
        device->offset=device->blkpfx;
        device->rem_len-=device->blkpfx;
       }
     }
    if (rem_len > device->rem_len) move_size = device->rem_len;
    else move_size=rem_len;       /* set size of move */
    memcpy(&device->buffer[device->offset],buf+offset,move_size); /* move */
    device->arec_len+=move_size;    /* update length of record */
    device->offset+=move_size;      /* update offset into out buf */
    offset+=move_size;              /* update offset into in buf */
    device->rem_len-=move_size;     /* update remaining length in  out buf */
    rem_len-=move_size;             /* update remaining length in in buf */ 
    if (device->data_mode == RECORD && device->eor==1) /* record mode and eor?*/
     {
      device->seg_len=device->rec_len;
      len=device->rec_len - device->arec_len;
      if (len > 0)   /* need to pad record? */
       {
        if (device->translate == OFF)
         memset(&device->buffer[device->offset],' ',len);
        else memset(&device->buffer[device->offset],0x40,len);
       }
      device->arec_len=0;                      /* reset */
      device->offset+=len;                     /* offset to next rec */
      device->rem_len-=len;                    /* remaining length this rec */
      rem_len=0;                               /* finished with input buf */
      if (device->blkpfxl==1)            /* set length in blkpfx if necessary */
       memcpy(device->buffer,&device->offset,4);
      if (device->rem_len < device->rec_len) tflush(device,0);/* flush on EOB */
     }
    else if (device->data_mode == STREAM)
     {
      if (device->blkpfxl==1)            /* set length in blkpfx if necessary */
       memcpy(device->buffer,&device->offset,4);
      if (device->rem_len == 0) tflush(device,0);  /* flush on EOB */
     }
   } while (rem_len>0);
  return (0);
 }


static int u_write(struct deviceinfo *device, const unsigned char *buf,unsigned size)
 {
  int move_size, offset, rem_len;

  if (device->data_mode == RECORD && (int) size > device->rem_len-device->blkpfx)
   {
    issue_warning_message("Warning record longer than blocksize - truncation will occur\n");
    size=device->rem_len-device->blkpfx;
   } 
  device->seg_len=rem_len=size;                     /* amount of data to move */
  offset=0;
  do
   {
    if (device->blkpfx >0 && device->offset == 0)
                  /* start of Block & block prefixing enabeled? */
     {
      memset(device->buffer,'^',device->blkpfx); /* set block prefix */
      device->offset=device->blkpfx;             /* offset into block */
      device->rem_len-=device->blkpfx;           /* remaining length block */
     }
    if (rem_len > device->rem_len) move_size=device->rem_len;            
    else move_size=rem_len;                     /* set size of move */
    memcpy(&device->buffer[device->offset],buf+offset,move_size); /* move */ 
    device->offset += move_size;             /* update pointer for out buf */
    offset += move_size;                     /* update pointer for in bur */
    device->rem_len -= move_size;            /* update remaining len out buf */
    rem_len -= move_size;                    /* update remaining len in buf */
    if (device->blkpfxl==1)                  /* Block prefixing need length? */
     memcpy(device->buffer,&device->offset,4); /* yep - move in total len */
    if (device->data_mode == RECORD && device->eor==1) tflush(device,0);
    else if (device->data_mode == STREAM && device->rem_len==0) tflush(device,0);
   } while (rem_len>0);
  return(0);
 }


static int dbs_write(struct deviceinfo *device, const unsigned char *buf,unsigned int size)
 {
  int dw;
  char dw_char[5];
  int move_size,rem_len,offset,len;

  if (device->offset==0)                /* starting a new block? */
   {db_startblk(1)}                     /* yep - initialize */
  else if (device->seg_start==&device->buffer[device->offset]) /* new rec? */
   {
    device->offset += 5;                /* yep - offset start new rec */
    device->rem_len -= 5;               /* remaining length in block */
    memset(device->seg_start,'0',5);    /* zap RDW */
    dw=5;
    memcpy(device->seg_start+1,&dw,4);  /* update RDW */
    if (device->blkpfxl==1)             /* recording BDW */ 
     memcpy(device->buffer,&device->offset,4);  /* yep - update */
   }
  len=device->arec_len+size;      /* len of actual record  - so far */
  if (device->data_mode== RECORD && device->rec_len<9999)
   {
    if (len > device->rec_len-5)    /* if larger than specified truncate */
     {
      issue_warning_message(
          "Warning record longer that record length - truncation will occur\n");
      size=device->rec_len-5-device->arec_len; /* move just this  amount */
     }
   }
  device->seg_len=size;
  rem_len=size;                         /* amount of data to move */
  offset=0;                             /* offset into input buffer */
  do 
   {
    if (rem_len > device->rem_len)      /* calc  move size  */ 
     {   
      move_size=device->rem_len;        /* can move just this amount */ 
      rem_len-=device->rem_len;         /* ammount left after move */
     }
    else
     {
      move_size=rem_len;                /* can move rest of segment */
      rem_len=0;                        /* no more to move */
     }
    memcpy(&device->buffer[device->offset],buf+offset,move_size); /*move data */
    memcpy(&dw,device->seg_start+1,4);  /* update date RDW */
    dw += move_size;
    memcpy(device->seg_start+1,&dw,4);
    device->arec_len +=move_size;       /* update physical length of record */ 
    device->offset += move_size;        /* update offset into out block */
    device->rem_len -= move_size;       /* update remaining length block */
    offset += move_size;                /* update offset into input buff */
    if (device->blkpfxl==1)             /* recording BDW? */
     memcpy(device->buffer,&device->offset,4); /* yep update */
    if (rem_len > 0)                    /* more data to process? */
     { 
      if (device->data_mode==RECORD && device->seg_start[0] == '0') /*1st seg?*/
       device->seg_start[0] = ANSI_FIRST; /* yes - mark */
      tflush(device,0);                   /* write out block */
      db_startblk(1);                     /*init new block */
      if (device->data_mode==RECORD)      /* mark this block as a middle seg */
       device->seg_start[0] = ANSI_MID;
     }
   } while (rem_len>0);
  if (device->data_mode == RECORD && device->eor == 1) /* end of record? */
   {  /* yep mark last segment if necessary */
    device->arec_len=0;
    if (device->seg_start[0] != '0') device->seg_start[0] = ANSI_LAST;
    if (device->hdr2[38]=='S' || device->rem_len <6) 
     {     
      tflush(device,0);         /* flush if unbuffered or no room for new rec */
      return(0);                           /* done */
     }
    memcpy(&dw,device->seg_start+1,4); /* change RDW to char format */
    sprintf(dw_char,"%04d",dw);
    memcpy(device->seg_start+1,dw_char,4);
    device->seg_start=&device->buffer[device->offset]; /* point to new rec */
   }
  return (0);
 }


static int db_write(struct deviceinfo *device, const unsigned char *buf,unsigned size)
 {
  int dw;
  char dw_char[5];
  int offset,len,i,rem_len, move_size;
  
  if (device->offset == 0 || device->seg_start==&device->buffer[device->offset])
   device->arec_len=0; 
  len=device->arec_len+size;          /* len of actual record  - so far */
  if (device->data_mode == RECORD)    /* check max rec len (RECORD MODE ONLY) */
   {
    if ( len > device->rec_len-4)    /* if larger than max specified truncate */  
     {
      issue_warning_message( 
          "Warning record longer that record length - truncation will occur\n");
      move_size=size=device->rec_len-4-device->arec_len; /* move just this */
     }
    else move_size=size;              /* move entire  segment  */
    device->seg_len=move_size;
   }
  else
   {
    device->seg_len=size;
    move_size=size;                   /* move entire buffer if < space in */  
    rem_len=device->rec_len-4-device->arec_len; /* record else move amt in  */
    if (move_size>rem_len) move_size=rem_len;   /* to fill record or if less */ 
    if (move_size>device->rem_len) move_size=device->rem_len; /* the block */
   }
  rem_len=size;                      /* amount of data to process */
  offset=0;                          /* offset into buffer */ 
  do 
   {
    if (device->offset==0)           /* starting a new block? */
     {db_startblk(0)}                /* yep initialize new block */
    else if (device->seg_start==&device->buffer[device->offset]) /* new rec? */
     {
      device->offset += 4;          /* yep - actual start of record */
      device->rem_len -= 4;         /* remaining length in block */
      if (device->data_mode != RECORD &&
          move_size>device->rem_len) move_size=device->rem_len; /* the block */
      dw=4;                         /* length RDB so far */
      device->arec_len=0;           /* reset actual record length */
      memcpy(device->seg_start,&dw,4); /* fill in RDW */
      if (device->blkpfxl==1)       /* record block length? */
       memcpy(device->buffer,&device->offset,4); /* yep - update */
     }
    if (device->data_mode == RECORD && rem_len > device->rem_len) /* write curr block? */    
     {
      int offset;

      memcpy(&dw,device->seg_start,4); /* write out finished portion of block */
      len=dw;                      /* this portion should be moved to next */ 
      device->offset-=len;         /* actual length to write */
      offset=device->offset;       /* where to start move to next block */
      if (device->blkpfxl==1)      /* update block length if recording */
       memcpy(device->buffer,&device->offset,4);
      tflush(device,0);            /* write out complered block */
      db_startblk(0);              /* prepare new  block */
      memcpy(device->seg_start,&dw,4); /* copy in DW length */
      for (i=4;i<len-4;i++)        /* move in old portion of prev block */
       device->seg_start[i]=device->buffer[offset+i];
      device->offset=device->blkpfx+len; /* current offset into new block */
      if (device->blkpfxl==1)      /* update block length if recording */
       memcpy(device->buffer,&device->offset,4);
      device->rem_len=device->block_len-device->offset; /* rem len new block */
     }
                      /* add current seg to block */
    memcpy(&device->buffer[device->offset],buf+offset,move_size);
    memcpy(&dw,device->seg_start,4);     /* update DW len */
    dw += move_size;
    device->arec_len=dw-4;               /* update actual record len */
    memcpy(device->seg_start,&dw,4);
    device->offset += move_size;         /* update offset into record */
    offset+= move_size;                  /* update offset into input record */
    device->rem_len -= move_size;        /* update rem len */
    rem_len -= move_size;                /* update rem len */
    dw=device->offset;
    if (device->blkpfxl==1)              /* update BDW if recording */
     memcpy(device->buffer,&dw,4);
    if (device->data_mode == RECORD && device->eor == 1)   /* End of Record? */ 
     {
      if (device->hdr2[38]==' ' || device->rem_len < 5) 
       {     
        tflush(device,0);     /* flush is unblocked or no room for new rec */
        return (0);               /* done */
       }
      memcpy(&dw,device->seg_start,4);   /* need to transform RDW to char */
      sprintf(dw_char,"%04d",dw);
      memcpy(device->seg_start,dw_char,4);
      device->seg_start=&device->buffer[device->offset]; /* where to put next rec */
     }
    else if (device->data_mode == STREAM)
     {
      if (device->rem_len == 0 || device->arec_len == device->rec_len-4) 
       {
        if (device->hdr2[38]==' ' || device->rem_len < 5) 
         tflush(device,0); /* flush if unblocked or no room for another rec */
        else
         {
          memcpy(&dw,device->seg_start,4); /* transform block length to char */
          sprintf(dw_char,"%04d",dw);
          memcpy(device->seg_start,dw_char,4);
          device->seg_start=&device->buffer[device->offset]; /* point to rec */
         }
       }
      if (device->arec_len < 0) exit(1);
      move_size=rem_len; /* calc next move amount */
      if (move_size>device->rec_len-4) move_size=device->rec_len-4;
      if (move_size>device->rem_len-4) move_size=device->rem_len-4;
     }
   } while (rem_len>0);
  return (0);
 }


static int unix_write(struct deviceinfo *device, const unsigned char *buf,unsigned size)
 {
  device->seg_len=size;
  return(write(device->file,(unsigned char*)buf,size));  /* this is easy - just write */
 }


static int term_write(struct deviceinfo *device,const unsigned char *buf,unsigned size)
 {
  unsigned char buffer[258];
  int i,print;
  unsigned int j,temp;
 
  device->seg_len=size;
  print=i=0; 
  for (j=0; j<size; j++)
   { 
    if (isprint(buf[j])!=0 || buf[j] == '\n')
     {
      buffer[i]=buf[j];
      i++;
     }
    else
     {
      temp = (unsigned char)buf[j]; 
      sprintf((char *)&buffer[i],"\\x%2.2X",temp);  
      i+=4;
     }
    if (i>=250 || j>=size-1) 
     {
      buffer[i]='\0';
      print=1;
     }
    if (print==1)
     {
      append_normal_message((char *)buffer,4);
      print=i=0;
     }
   }
  return 0;
 }


static int tflush(struct deviceinfo *device,int last_write)
 {
  int dw;
  char dw_char[5];
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
  void (*old_handler)(int);
#endif

  if (device->offset>0)                          /* is there a line to write? */
   {
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
    old_handler = signal(SIGINT,SIG_IGN);
#else
    sighold(SIGINT);
#endif
    if (device->data_mode == STREAM && device->format[0] == 'F') /* yep -pad? */
     {
      int len, rec_len;

      rec_len=device->rec_len;     /* yep - get record length fb, fbs formats */
      if (rec_len==0) 
       rec_len=device->block_len-device->blkpfx;  /* rec length f, fs formats */
      len=(device->offset-device->blkpfx) % rec_len; /* calc len last block */
      if (len != 0)                                  /* need to pad? */
       {
        len=rec_len-len;                             /* calc padding amount */
        if (device->translate==OFF)                  /* ascii? */ 
         memset(device->buffer+device->offset,'^',len); /* ansi fill char */
        else
         memset(device->buffer+device->offset,0x40,len); /* ebcdic fill char */
        device->offset+=len;                         /* update block len */
        if (device->blkpfxl==1)                    /* need to put in blk len? */
         memcpy(device->buffer,&device->offset,4);   /* yep */ 
       }  
     }
    if (device->blkpfxl==1)                      /* ansi block length neededi?*/
     {
      memcpy(&dw,device->buffer,4);         /* yep - transform to char format */
      sprintf(dw_char,"%04d",dw);
      memcpy(device->buffer,dw_char,4);
     }
    if (device->format[0] == 'D')               /* format  D, DB, DS DBS? */
     {
      memcpy(&dw,device->seg_start+device->span,4); /* yep need to transform */
      sprintf(dw_char,"%04d",dw);                   /* last RDW to char */     
      memcpy(device->seg_start+device->span,dw_char,4); /* format */
     }
#ifdef BYTE_SWAPPED
    if (device->format[0]=='V') /* need to inverts DWs? */
     {
      unsigned char temp;

      temp=device->buffer[0];                      /* yep - invert BDW */
      device->buffer[0]=device->buffer[1];
      device->buffer[1]=temp;
      temp=device->seg_start[0];                   /* invert last RDW */
      device->seg_start[0]=device->seg_start[1];
      device->seg_start[1]=temp;
     }
#endif
    if (device->drive_type != DEV_QIC)             /* not a QIC tape? */
     tpwrite(device,device->buffer,device->offset); /* just write out then */
    else 
     {
      int rem_len, move_len, offset;
      rem_len=device->offset;                      /* amount to write out */
      offset=0;                                    /* offset into out buf */
      do                                           /* write out in 512 chunks */
       {
        move_len=rem_len;                    /* move this amount if not > QIC */
        if (move_len > ostream.rem_len) move_len=ostream.rem_len; /* buffer */
        memcpy(ostream.buffer+ostream.offset,device->buffer+offset,move_len);
        ostream.rem_len -= move_len;       /* calc space remaining QIC buffer */
        rem_len -= move_len;               /* calc space remaining out buffer */
        ostream.offset+=move_len;          /* calc offset into QIC buffer */
        offset+=move_len;                  /* calc offset into OUT buffer */
        if (ostream.rem_len == 0)          /* have block toe write? */
         {
          write(device->file,ostream.buffer,512); /* yep */
          ostream.rem_len=512;             /* reset remaining length QIC buf */
          ostream.offset=0;                /* reset offset into QIC buf */ 
         }
       } while (rem_len >0);               /* cycle */
     }
    device->blocks++;                      /* incr block count */
    device->offset=0;                     /* resset offet into output buffer */
    device->rem_len=device->block_len;   /* reset remaining length out buffer */
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
    signal(SIGINT,old_handler);
#else
    sigrelse(SIGINT);
#endif
   }
  if (last_write == 1 && device->drive_type==DEV_QIC && ostream.offset > 0)
   {
    int len;
    
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
    old_handler = signal(SIGINT,SIG_IGN);
#else
    sighold(SIGINT);
#endif
    len=512-ostream.offset;            /* QIC only - pad last block if needed */
    memset(ostream.buffer+ostream.offset,'^',len);
    write(device->file,ostream.buffer,512);
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
    signal(SIGINT,old_handler);
#else
    sigrelse(SIGINT);
#endif
   }
  return (0);
 }


static char *getpath(char * path)
 {
#if SYSTEM != MSVC
  if (path[0]=='~')                        /* is there something to expand? */
   {
    char *slash, *home;
    struct passwd *passwd;

    slash=strpbrk(path,"/");               /* yep - find file's first / */
    if (path[1]=='/' || path[1] == '\0')   /* just a tilde or start ~/? */
     {
      home=getenv("HOME");                 /* yep - Try to get home dir */
      if (home==NULL) return(path);        /* Could resolve - return org name */
      path++;                              /* point past the tilde */
     }
    else
     {
      if (slash != NULL) *slash='\0';      /* cre string ~uniqname */
      passwd = getpwnam(&path[1]);         /* try to get info on uniqname */
      if (slash != NULL) *slash='/';       /* restore the  /   */
      if (passwd == NULL) return(path);    /* No info - return unigue name */
      home=passwd->pw_dir;                 /* point to apporpiate directory */
      if (slash != NULL) path = slash;     /* point to low end of name */
      else path="";                        /* theres nothing to point at */
     }
    strcpy((char *)path_name,home);      /* put in full directory path */
    strcat((char *)path_name,path);      /* cancat with low end parth */
    return ((char *)&path_name);         /* return full path name */
   }
#endif
  return(path);                         /* return original name */
 }


int tapeinit(void)
 {
  unsigned short int test=32767;
#ifndef BYTE_SWAPPED
  unsigned char ind_test1[2]={0X7F,0XFF};
#else
  unsigned char ind_test2[2]={0XFF,0X7F};
#endif

#ifdef BYTE_SWAPPED
  if (memcmp(ind_test2,&test,2)!=0)
   {
    printf("Compiled for little-endian machine, but this is a big-endian machine\n");
    exit(2);
   }
#else
  if (memcmp(ind_test1,&test,2)!=0)
   {
    printf("Compiled for big-endian machine, but this is a little-endian machine\n");
    exit(2);    
   }
#endif
  tapeo.label=tapei.label=OFF;                   /* disable tape labeling */
  tapeo.name=tapei.name=NULL;                    /* no devices allocated */
  return (0);
 }


struct deviceinfo *get_tape_ptr(int type)
 {
  if (type == INPUT) return(&tapei);             /* point to Device */
  else if (type == OUTPUT) return(&tapeo);
  else {printf("Unknown output type %d",type); return(NULL);}
 }


int closetape(struct deviceinfo *tape)
 {
  int rc;    

  if (tape->name==NULL) return(0);                /* Already closed? */
  if (tape->fsheader != NULL) free(tape->fsheader); /* Free Fs header */
  free(tape->name);                               /* Free name Field */
  tape->name=NULL;
  tape->fsheader=NULL;
  rc = close(tape->file);                              /* close the tape */
  if (rc != 0)
   issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
  tape->file=0;
  return(1);
 }


int tapeclose(void)
 { 
  if (tapeo.name != NULL) closetape(&tapeo);
  if (tapei.name != NULL) closetape(&tapei);
  return (0);
 }


static int getfname(struct deviceinfo *tape)
 {
  int date;
  char date_c[7];

  tape->offset=tape->rem_len=tape->blocks=0;    /* reading reader set to zero */
  if (tape->lp==ON)                             /* label processing enablew? */
   if (tape->tape_type>=FS_UNLABELED)          /* fs unlabeled? */
    return (getfsname(tape));                  /* yep-get name from FS header */
   else if (tape->tape_type==UNLABELED ||      /* unlabeled? */
            tape->tape_type==VLO_LABEL)        /* VLO label */
    tape->file_name[0]='\0';                    /* there is no name */ 
   else 
    {
     memcpy(tape->file_name,&tape->hdr1[4],17); /* get name from header */
     date_c[6]='\0';                       
     date=atoi(strncpy((char *)date_c,(char *)&tape->hdr1[41],6)); /* cre date*/
     if (date< TDATE)
      {
       tape->trtable=MTSASC;                    /* set trtable using date */
       if (tape==&tapeo) tape->otrtable=ASCMTS;
      }
     else 
      {
       tape->trtable=EBCASC;
       if (tape==&tapeo) tape->otrtable=ASCEBC;
      }
     tape->file_name[17]='\0';                   /* terminate file name */
    }
  else  
   {
    tape->file_name[0]='\0';                     /* no file name */
    tape->trtable=EBCASC;                        /* default translate tables */
     if (tape==&tapeo) tape->otrtable=ASCEBC;
   }  
  return (0);
 }


static unsigned char *crefname(unsigned char * filename) 
 {
  char s[]="\0\0";
  int i, j,len;
  
  len=strlen((char *)filename);           /* length of filename */
  for (i=0,j=0; i<=len; i++)              /* check characters */
   {
    switch (filename[i])
     {
      case ('.'):                     /* Don't want to create invisible files */
        if ( j == 0 ) filename[j++] = '_';
        else filename[j++] = '.';
        break;
      case ('-'):                     /* Don't want start a file with a  - */
        if ( j == 0 ) filename[j++] = '_';
        else filename[j++] = '-';
        break;
      case (':'):                     /* Check for ID prefixing the filename */
        j = 0;                        /* skip CCID: */
        break;
      case ('*'):/* Following characters aren't really good to use in Unix */
      case ('<'):
      case ('>'):
      case ('$'):
      case ('#'):
      case ('@'):
      case ('!'):
      case ('&'):
      case ('/'):
      case ('\\'):
        filename[j++] = '_';           /* these characters go to _ */
        break;
      default:                  /* Change to lower case to make things easier */
        filename[j++] = tolower(filename[i]);
        break;
     }
   }
  len=strlen((char *)filename);       /* recalculate length */         
  for (i=len-1;i>=0;i--)      
   if (filename[i] == ' ') filename[i]=s[0]; /* change trailing blanks to \0 */
   else s[0]='_';                     /* non trailing blanks to _ */ 
  return (filename);
 }


static unsigned char *verfname(unsigned char *filename,struct buf_ctl *buf_ctl,int io)
 {
  struct stat file_status; 
  char buf[258];
  unsigned char *ret_name, *rc;
  int len, n;
  ret_name=filename;
 
  if (io==OUTPUT)                           /* data set to be used as ouput? */
   {
    getstatus:
    n=stat((char *)ret_name,&file_status);  /* see if data set exists */
    buf_ctl->copy_type=FILENAME;            /* Assume file */
    if (n==0)                               /* something exists? */
     {
      if (file_status.st_mode & S_IFREG)    /* yes - was it a file? */
       {
        if (buf_ctl->warn==ON)              /* issue a warning? */
         {
          rc=(unsigned char *)getAnotherFile((char*)ret_name);
          if (rc == NULL) return (NULL);
          strcpy(buf,(char *)rc);
          free(rc);
          n=strspn(buf," ");
          len=strcspn(&buf[n]," \n\r");
          if (len != 0)                       /* was there one? */
           { 
            strcpy(fname,&buf[n]);
            fname[len]='\0';                /* terminate name */
            ret_name=(unsigned char *)getpath(fname); /* get full name */
            goto getstatus;                 /* get info on new name */
           }
         }
       }
      else if (file_status.st_mode & S_IFDIR) /* directory? */
       buf_ctl->copy_type=DIRECTORY;        /* mark asdirectory */
      else if (file_status.st_mode & S_IFCHR) /* tape? */
       buf_ctl->copy_type=TAPE ;            /* character special (like tape) */
      else buf_ctl->copy_type=UNDEFINED;    /* don't Know */
     }
   }
  else               /* assume input data set */
   {
    n=stat((char *)ret_name,&file_status);  /* get info on file */
    if (n==0)                               /* does it exist? */
     {
      if (file_status.st_mode & S_IFREG)
       buf_ctl->copy_type=FILENAME;         /* Set file */
      else if (file_status.st_mode & S_IFDIR)
       buf_ctl->copy_type=DIRECTORY;        /* directory */
      else if (file_status.st_mode & S_IFCHR)
       buf_ctl->copy_type=TAPE ;           /* character special (like tape) */
      else buf_ctl->copy_type=UNDEFINED;   /* don't know */
     } 
    else
     ret_name=NULL;
   } 
  return (ret_name);                       /* return file name */
 }


int copyfunction(struct buf_ctl *in_buf_ctl,struct buf_ctl *out_buf_ctl,
                 int duplicate)
 {
  div_t d;
  struct deviceinfo *idevice, *odevice;
  unsigned char *status, *name, *tape_file_name=NULL;
  char *open_mode, *ctl, *hdr, *tlr;
  unsigned char input_type, copy_type;
  int start, end;
  unsigned int records;
  WRITE_RTN write_rtn;
  READ_RTN read_rtn;
  unsigned char *trtable;
  unsigned char *otrtable=NULL;
  unsigned char *file_prefix, *dir_prefix;
  unsigned char tape_file_name_c[4096];
  unsigned char tape_file_name_c2[4096];
  unsigned char ipath_name[4096];
  unsigned char opath_name[4096];
  int i, len, file_num;
  double in_bytes, out_bytes;
  void (*old_handler)(int)=NULL;
  int data_transfer_mode;
  unsigned char *ipath, *opath;
  unsigned int current_cut, bytes, test;
  
  ipath = (unsigned char *)in_buf_ctl->path;
  opath = (unsigned char *)out_buf_ctl->path;
  start = in_buf_ctl->start_file; 
  end = in_buf_ctl->end_file;
  records=0;
  read_rtn=NULL;                                  /* don't have one yet */
  write_rtn=&unix_write;                 /* assume will be writting to a file */
  idevice=odevice=NULL;                  /* have not aquired input or output */
  file_prefix=(unsigned char*)"";          /* set file prefix to empty string*/
  dir_prefix=(unsigned char*)"";      /* set directory prefix to empty string*/
  if (out_buf_ctl->iofrom==1) odevice=&tapeo;
  if (out_buf_ctl->iofrom==2)
   {
    len=sizeof *odevice;
    odevice = (struct deviceinfo *) malloc(len);               
/*  memset(&odevice->name,'\0',len);             /  initialize */
    odevice->data_mode=outfile_recording_mode;    /* mark a stream device */
    write_rtn=&term_write;
   }
  trtable=NULL;                          /* no over riding translate table */
  if (ipath != NULL)                            /* INPUT parameter used? */
   {
    char *path_name;

    path_name=getpath((char *)ipath);           /* get fully qualified name */
    strcpy((char *)ipath_name,path_name);       /* make copy for future ref */
    status=verfname(ipath_name,in_buf_ctl,INPUT); /* get type of dev/file */
    if (status==NULL)                           /* could get and info? */
     {
      sprintf(message_area,
       "File `%s' does not exist\n",ipath_name); /*no - must not exist */
      issue_error_message(message_area);
      return (-1);
     }
    if (in_buf_ctl->copy_type==TAPE)          /* read from tape? */
     {
      sprintf(message_area,"Tapes must be opened first.\n");
      issue_error_message(message_area); 
      return (-1);
     }
    else if (in_buf_ctl->copy_type==DIRECTORY) /* read from directory? */
     {
      sprintf(message_area,"Copy from a directory is not supported(yet)\n");
      issue_error_message(message_area); 
      return (-1);
     }
    else if (in_buf_ctl->copy_type==FILENAME) /* read from File? */
     {
      if (in_buf_ctl->format[0] != '\0')
       {
        if (in_buf_ctl->format[0] == 'U' ||
           strcmp(in_buf_ctl->format,"FS") == 0 ||
           strcmp(in_buf_ctl->format,"FBS") == 0)
         {
          issue_error_message("Reading of format U, FS. or FBS from a file is not supported\n"); 
          return(-1);
         }
        else if (in_buf_ctl->format[0] == 'V' && in_buf_ctl->blkpfx != 0)
         {
          issue_error_message("Format V, VS, VB, or VBS does not accept a block prefix\n");
          return(-1);
         }
        else if (in_buf_ctl->format[0] == 'D' && (in_buf_ctl->blkpfx <4 ||
                 in_buf_ctl->blkpfxl != ON)) 
         {
          issue_error_message("Format D, DS, DB, or DBS must have a block prefix of at least 4 L\n");
          return(-1);
         }
        else if (in_buf_ctl->format[0] != 'F' && 
                 in_buf_ctl->format[0] != 'V' &&
                 in_buf_ctl->format[0] != 'D') 
         {
          issue_error_message("Unsupported format entered\n");
          return(-1);
         }
       }
      len=sizeof *idevice;                  /* yes - get length control block */
      idevice = (struct deviceinfo *) malloc(len);    /* get space for control block */
      memset(idevice,'\0',len);                /* allocate control block */
      idevice->file=0; 
      idevice->unixfile=NULL;
      if (in_buf_ctl->format[0] != '\0')
       {
        idevice->file=open((char *)ipath_name,O_RDONLY|O_BINARY); /* yep - opens as input? */ 
         if (idevice->file == 0)              /* yep - tell user can't open*/
         {
          issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
          sprintf(message_area,"Could not open '%s'\n",ipath_name);
          issue_error_message(message_area);
          return(-1);
         }
       } 
      else
       {
        if (infile_recording_mode == STREAM)
         open_mode="rb"; /*If declared stream - open bin*/
        else open_mode="r";                      /* else open for text */
         idevice->unixfile=fopen((char *)ipath_name,open_mode); /* open file */
        if (idevice->unixfile==NULL)         /* could file be opened for read?*/
         {
          issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
          sprintf(message_area,
                "Could not open '%s'\n.",ipath_name); /* no -tell user */
          issue_error_message(message_area); 
          free(idevice);                         /* relead control block */
          return (-1);                           /* done */
         }
       }
      idevice->tape_type=FILESYSTEM;
      idevice->data_mode=infile_recording_mode; /* set data mode */
      idevice->trtable=EBCASC;              /* incase transation gets enabled */
      idevice->otrtable=ASCEBC;
      if (in_buf_ctl->format[0] == '\0') read_rtn=&unix_read;
      else
       {
        idevice->block_len=in_buf_ctl->blk_len;   /* yes */
        idevice->rec_len=in_buf_ctl->rec_len;
        idevice->blkpfx=in_buf_ctl->blkpfx;
        strncpy((char *)idevice->format,in_buf_ctl->format,8);
        if (in_buf_ctl->format[0]=='F') read_rtn=&fb_read;
        else if (strcmp(in_buf_ctl->format,"VB") == 0 ||
                 strcmp(in_buf_ctl->format,"V") == 0)
         read_rtn=&vb_read;
        else if (strcmp(in_buf_ctl->format,"VBS") == 0 ||
                 strcmp(in_buf_ctl->format,"VS") == 0)
         read_rtn=&vbs_read;
        else if (strcmp(in_buf_ctl->format,"DB") == 0 ||
                 strcmp(in_buf_ctl->format,"D") == 0)
         read_rtn=&db_read;
        else if (strcmp(in_buf_ctl->format,"DBS") == 0 ||
                 strcmp(in_buf_ctl->format,"DS") == 0)
         read_rtn=&dbs_read;
        idevice->block_eol=0;
        if (in_buf_ctl->blk_eol > 0 ) idevice->block_eol=strlen("\n");
       }
     }
    else                                      /* read from WHAT! */
     {
      sprintf(message_area,
              "copying from '%s' is not supported\n",ipath_name);
      issue_error_message(message_area); 
      return (-1);
     }
    input_type=in_buf_ctl->copy_type;      /* save input type for reference */
   } 
  else                                /* read from already opened input tape */
   {
    if (tapei.name==NULL)                    /* is it defined?  */
     {
      sprintf(message_area,"Input tape is not defined\n"); /* no - tell user */
      issue_error_message(message_area); 
      return (-1);
     }
    idevice=&tapei;                         /* use input tape */
    input_type=TAPE;                        /* input is from tape */
   }
  if (in_buf_ctl->translate==-1)                 /* user specify translation mode?*/
   in_buf_ctl->translate=idevice->translate; /* yep - set translate mode */
  file_prefix=(unsigned char*)"";           /* set file prefix to empty string*/
  out_buf_ctl->copy_type=FILENAME;          /* This is the default copy mode */
  if (out_buf_ctl->iofrom==2)
   out_buf_ctl->copy_type=TERMINAL;
  else if (opath != NULL)                   /* OUTPUT parameter used? */
   {
    char *path_name;

    path_name=getpath((char *)opath);       /* yes get fully qualifiled path */
    strcpy((char *)opath_name,path_name);   /* make copy for future ref */
    len=strlen((char *)opath_name);         /* length of qualified path */
    if (opath_name[len-1]=='~')             /* Append file number?  */ 
     {
      opath_name[len-1]='_';                /* yep - create names in this form*/
      file_prefix=opath_name;               /* set this to file prefix */
      opath=NULL;                           /* output path not defined yet */
      odevice = (struct deviceinfo *) malloc(len);               
      memset(odevice,'\0',len);             /* initialize */
      odevice->data_mode=outfile_recording_mode;    /* mark a stream device */
     }
    else                                    /* this is fully qualified name */
     {
      path_name=(char *)verfname(opath_name,out_buf_ctl,OUTPUT); /*ver path name*/
      if (path_name==NULL)
       {
        if (input_type != TAPE) free(idevice);
        return (-1);
       }
      strcpy((char *)opath_name,path_name); /* make a copy for future ref */ 
      opath=opath_name;
      if (out_buf_ctl->copy_type==TAPE)    /* was this a tape? */
       {
        if (tpopen(OUTPUT,opath_name,0,0) < 0) return(-1);/*yep - open if possible*/
        odevice=&tapeo;                  /* make this the default output tape */
       }
      else if (out_buf_ctl->copy_type==DIRECTORY) /* was this a directory? */
       {
        dir_prefix=opath_name;               /* yep - set directroy prefix */
        opath=NULL;                          /* output path not defined yet */
        len=sizeof *odevice;                 /* get control block for output */
        odevice = (struct deviceinfo *) malloc(len);               
        memset(odevice,'\0',len);            /* initialize */
        odevice->data_mode=outfile_recording_mode;    /* mark a stream device */
       }
      else if (out_buf_ctl->copy_type==FILENAME)  /* output a file? */
       {
        len=sizeof *odevice;            /* yep - get control block for output */
        odevice = (struct deviceinfo *) malloc(len);
        memset(odevice,'\0',len);           /* zap control block */
        odevice->file=open((char *)opath_name, /*open file for output */
                          O_RDWR|O_CREAT|O_TRUNC|O_BINARY,
#if SYSTEM == MSVC
                          _S_IREAD | _S_IWRITE);
#else
                          S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#endif
         if (odevice->file == -1)            /* could we open the file? */
         {
          sprintf(message_area,
                 "Couldn't open file or device '%s'\n",opath_name); /* no */
          issue_error_message(message_area);
          free(odevice);                     /* release control block */
          return (-1);
         }
        odevice->data_mode=outfile_recording_mode;  /* mark output as a stream device */
       }
      else                                   /* we don't copy to this type */
       {
        sprintf(message_area,"copying to '%s' is not supported\n",opath_name);
        issue_error_message(message_area);
        return (-1);
       }
     }
   }
  else                                      /* use default */
   {
    if (odevice==NULL)                      /* output device defined */
     {
      len=sizeof *odevice;                 /* no - ready to copy to a file */
      odevice = (struct deviceinfo *) malloc(len);                 /* to be created */
      memset(odevice,'\0',len);
      odevice->data_mode=outfile_recording_mode;
     }
    else if (odevice->name == NULL)        /* default output device defined */
     {
      sprintf(message_area,
              "Ouput tape not defined\n"); /* no - no where to copy */
      issue_error_message(message_area);
      return (-1);
     }
    else                                   /* use default output tape */
     {
      strcpy((char *)opath_name,odevice->name);/* save name for future ref */
      opath=opath_name;                     /* set default output tape */
      out_buf_ctl->copy_type=TAPE;          /* mark as coping to tape */
     }
   }
  copy_type=out_buf_ctl->copy_type;         /* get copy type */ 
  if (out_buf_ctl->translate==-1) 
   {
    if (copy_type==TAPE)                     /* copying to tape? */
     {
      out_buf_ctl->translate=odevice->translate; /* yep - set translation mode */
      otrtable=odevice->otrtable;
     } 
    else                                     /* else we don't translate */
     out_buf_ctl->translate=OFF;
   } 
  if (input_type!=TAPE) start=end=0;        /* if this is not a tape s=e=0 */
  data_transfer_mode=idevice->data_mode-odevice->data_mode;/* define transfer mode */
  odevice->old_handler=signal(SIGINT,&myattn);      /*activate copies signal handler*/
  for (file_num=start;file_num<=end;file_num++) /* cycle */
   {
    signal(SIGINT,odevice->old_handler);   /* normal signal process in use */
    if (input_type == TAPE)                /* Reading from a tape? */
     {
      if (file_num != 0)                   /* need to position tape? */
       {
        if (posn(idevice,file_num) == 1)   /* yes - do so */
         {
          if (file_num==start)          /* LEOT sensed - this the first file? */
           {
            sprintf(message_area,
                   "File %d occurs after end of Logical End of Tape\n",
                    file_num); /*yes */
            issue_error_message(message_area);
            break;                 /* tell user and done */
           }      
          else
           { 
            sprintf(message_area,
                    "Logical End of Tape reached\n"); /* LEOT - tell user*/
            issue_warning_message(message_area);
            break;
           }
         }
        if (in_buf_ctl->fs==1)
         getfsname(idevice);              /* if this is a FS tape read FS hdr */
       } 
     }
    if  (out_buf_ctl->iofrom !=2 && opath == NULL)  /* output file or device defined? */
     {
      if (idevice->file_name[0]=='\0' || file_prefix[0]!= '\0') /* no */
       {
        if (file_prefix[0]=='\0')         /* no file prefix? */
         file_prefix=(unsigned char*)"tape.file."; /* use this as default */
        sprintf((char *)tape_file_name_c,"%s%d",file_prefix,idevice->position);
        tape_file_name=tape_file_name_c;  /* use this as file name */
       } 
      else
       {   
        strcpy((char *)tape_file_name_c,(char *)idevice->file_name); /* make */ 
        tape_file_name=crefname(tape_file_name_c);  /* same a input file */
       }
      if (copy_type==DIRECTORY)          /* copying to directory? */
       {
        len=strlen((char *)dir_prefix); /* prefix directory name to file name */
        if (dir_prefix[len-1]=='/') dir_prefix[len-1]='\0';
        sprintf((char *)tape_file_name_c2,"%s/%s",dir_prefix,tape_file_name); 
        tape_file_name=tape_file_name_c2;
       }
        /* protect form over writting */ 
      tape_file_name=verfname(tape_file_name,out_buf_ctl,OUTPUT); 
      if (tape_file_name == NULL)
       {
        if (input_type != TAPE) free(idevice);
        return (-1);
       }
      odevice->file=open((char *)tape_file_name,   /* open file */
          O_RDWR|O_CREAT|O_TRUNC|O_BINARY,
#if SYSTEM == MSVC
                         _S_IREAD | _S_IWRITE);
#else
                         S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#endif
      if (odevice->file==-1)
       {
        sprintf(message_area,
               "Couldn't open file '%s'\n",tape_file_name); /* couldn't */ 
        issue_error_message(message_area);
        if (input_type != TAPE) free(idevice);   /* free input control block */
        free(odevice);                           /* free output control block */
        return (-1);                                  /* copy done */
       }
     }
    if (copy_type==TAPE)                        /* copying to a tape? */
     {
      odevice->need_trailer=OFF;                /* make sure this is off */
      signal(SIGINT,&myattn);                   /* active special interupt */   
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
      old_handler = signal(SIGINT,SIG_IGN);
#else
      sighold(SIGINT);
#endif
      if (duplicate==ON) 
       {
        in_buf_ctl->translate=out_buf_ctl->translate=OFF;
        strncpy((char *)odevice->file_name,(char *)idevice->file_name,33);
        odevice->newfile_name=ON;
        strncpy((char *)odevice->format,(char *)idevice->format,8);
        odevice->block_len=idevice->block_len;
        odevice->rec_len=idevice->rec_len;
        odevice->blkpfx=idevice->blkpfx;
        odevice->blkpfxl=0;
        odevice->blkpfxs=ON;
        odevice->fmchar[0]=idevice->fmchar[0];
        if (odevice->lp==ON) odevice->op[0]='D';
       }
      write_rtn=(odevice->wrt_hdr_rtn)(odevice);  /* yes write header label */
      odevice->op[0]='\0';
      if (write_rtn == NULL) return(-1);
      odevice->need_trailer=ON; 
#if SYSTEM == OS4 || SYSTEM == MSVC || SYSTEM == CYGWIN
      signal(SIGINT,old_handler);
#else
      sigrelse(SIGINT);
#endif
      if (odevice->blocking == OFF || duplicate == ON)
       {
        odevice->blkpfx=0;
        odevice->blkpfxl=0;
        odevice->format[0]=tolower(odevice->format[0]);
        write_rtn=&u_write;
       }
      if (trtable != NULL) idevice->trtable=trtable; /* set trtable if set */
      odevice->arec_len=odevice->offset=0;          /* initialize some fields */
      odevice->rem_len=odevice->block_len;
     }
    if (input_type==TAPE)                        /* copying from tape? */
     {
      if (in_buf_ctl->format[0] != '\0')         /*yes - override label format*/
       {
        idevice->block_len=in_buf_ctl->blk_len;   /* yes */
        idevice->rec_len=in_buf_ctl->rec_len;
        idevice->blkpfx=in_buf_ctl->blkpfx;
        strncpy((char *)idevice->format,in_buf_ctl->format,8);
        read_rtn=in_buf_ctl->read_rtn;
       }
      else read_rtn=idevice->read_rtn;           /* use default read routine */
      if (idevice->blocking == OFF || duplicate == ON)
       {
        idevice->blkpfx=0;
        idevice->blkpfxl=0;
        read_rtn=&u_read;
       }
     }
    in_buf_ctl->blocks=records=ostream.offset=0; /*initalize offsets and counts */
    ostream.rem_len=512;                       /* block size for QIC */
    in_bytes=out_bytes=0.0;
    name=opath;                              /* assume this is file name */
    if (name == NULL) name=tape_file_name;   /* else this is the file name */
    if (copy_type==TERMINAL) name=(unsigned char*)"Std out";
    if (file_num==0)                         /* copping to current file? */ 
     {
      file_num=idevice->position;            /* get current file number */
     }
    if (copy_type==TAPE)                         /* copy to a tape? */ 
     {
      char file_number_c[30];
      if (odevice->file_name==NULL)
       sprintf(file_number_c," file %d",odevice->position); /*get out file num*/
      else 
       sprintf(file_number_c," file %d %s",odevice->position,odevice->file_name); /*get out file num*/
      tlr=file_number_c;                     /* for message */
     } 
    else tlr="";                             /* else nothing to report */
    if (copy_type==TERMINAL) ctl="\n";
    else ctl="";
    /* print basic copy info */
    if (input_type==TAPE)                    /* copying from tape */
     { 
      if (idevice->lp == 1 && (idevice->tape_type == FS_UNLABELED ||
                               idevice->tape_type == FS_IBMLABEL || 
                               idevice->tape_type == FS_VLOLABEL))
       hdr="FS file";                          /* yep - FS tape */
      else hdr="tape file";                       /* or regular tape? */
      len=strlen((char *)idevice->file_name);
      for (len--;len>0 && idevice->file_name[len]==' ';len--) 
       idevice->file_name[len]='\0';
      sprintf(message_area, "%sCopying %s %d %s to %s%s.\n",
              ctl,hdr,file_num,idevice->file_name,name,tlr);
      append_normal_message(message_area,4);
     }
    else                                    /* copied from a real file */
     {
      /* print copy stats */
      sprintf(message_area,"%sCopying file %s to %s%s.\n",ctl,ipath,name,tlr);
      append_normal_message(message_area,4);
     }
    current_cut=0;
    postblocks(0);
    postbytes(0.0);
    in_buf_ctl->max_blk=0;
    while (1)                                  /* cycle though input file */
     {        
      unsigned char * rec;
      char line_num_c[20];
  
      rec=(read_rtn)(idevice,in_buf_ctl);     /* read a record */
      if (copy_type==TAPE) odevice->eor=0;     /* if coping to tape init EOR */
      if (rec==NULL)  
       {
        if (data_transfer_mode == -1 &&
            (write_rtn == &dbs_write || write_rtn == &vbs_write))
         {
          odevice->eor=1; 
          (write_rtn)(odevice,(unsigned char *)line_num_c,0);
         }
        break;                                /* EOF - copy done */
       }
      if (in_buf_ctl->translate==ON) for (i=0;i< in_buf_ctl->seg_len;i++) /*translate*/ 
       rec[i]=idevice->trtable[rec[i]];        /* record if needed */
      if (read_rtn==&mts_fsread && in_buf_ctl->sor==1) /* from FS tape and SOR */
       {
        if (in_buf_ctl->linemode==1)            /* need line number in chars? */
         {
          d=div(in_buf_ctl->line_num,1000);  /* yes-get interger and dec parts */
          sprintf(line_num_c,"%d.",d.quot);    /* integer part to char */
          len=strlen(line_num_c);              /* len integer part */
          d.rem=abs(d.rem);                    /* make sure dec part is + */ 
          if (d.rem != 0) sprintf(&line_num_c[len],"%d",d.rem); /* dec part */ 
          len=strlen(line_num_c);              /* len of line num */
          line_num_c[len]='\t';                /* put in tab character */
          len++;                               /* include tab in count */
          if (out_buf_ctl->translate==ON)       /* translate if necessary */ 
           for (i=0;i<len;i++)
            line_num_c[i]=otrtable[(int)line_num_c[i]];
          if (in_buf_ctl->records_>0 && idevice->data_mode==STREAM) 
           {
            test = (unsigned int)(out_bytes+len);
            if (test  > in_buf_ctl->records_) len = (int)(in_buf_ctl->records_ - out_bytes);
           }
          (write_rtn)(odevice,(unsigned char *)line_num_c,len); /* write out line number */
          out_bytes+=len;       /* update byte count */
         }
        else if (in_buf_ctl->linemode==2)      /* need line number in fixed chars? */
         {
          d=div(in_buf_ctl->line_num,1000);  /* yes-get interger and dec parts */
          sprintf(line_num_c,"%8d.",d.quot);    /* integer part to char */
          d.rem=abs(d.rem);                    /* make sure dec part is + */ 
          sprintf(&line_num_c[9],"%-3.0d",d.rem); /* dec part */ 
          len=12;                               /* include tab in count */
          if (out_buf_ctl->translate==ON)       /* translate if necessary */ 
           for (i=0;i<len;i++)
            line_num_c[i]=otrtable[(int)line_num_c[i]];
          if (in_buf_ctl->records_>0 && idevice->data_mode==STREAM) 
           {
            test = (unsigned int)(out_bytes+len);
            if (test > in_buf_ctl->records_) len = (int)(in_buf_ctl->records_ - out_bytes);
           }
          (write_rtn)(odevice,(unsigned char *)line_num_c,len); /* write out line number */
          out_bytes+=len;                       /* update byte count */
         }
        else if (in_buf_ctl->linemode==3)      /* write line as integer? */
         {
          len=4;
          if (in_buf_ctl->records_>0 && idevice->data_mode==STREAM) 
           {
            test = (unsigned int)(out_bytes+len);
            if (test > in_buf_ctl->records_) len = (int)(in_buf_ctl->records_ - out_bytes);
           }
          (write_rtn)(odevice, (const unsigned char *)(&in_buf_ctl->line_num), len); /* yes write line number */
          out_bytes+=len;                          /* update byte count */
         }
       }
      if (data_transfer_mode == 0) odevice->eor=in_buf_ctl->eor; /*set EOR if nec */
      if (out_buf_ctl->translate==ON)          /* translate output if necessary */
       for (i=0;i<in_buf_ctl->seg_len;i++)
        rec[i]=otrtable[rec[i]];
      if (in_buf_ctl->records_>0 && idevice->data_mode==STREAM) 
       {
        test = (unsigned int)(out_bytes+in_buf_ctl->seg_len);
        if (test > in_buf_ctl->records_) in_buf_ctl->seg_len = (int)(in_buf_ctl->records_ - out_bytes);
       }
      (write_rtn)(odevice,rec,in_buf_ctl->seg_len); /* write out segment */
      in_bytes+=in_buf_ctl->seg_len;               /* update byte count */
      out_bytes+=odevice->seg_len;                 /* update byte count */
      if (in_buf_ctl->eor==1)                       /* read an EOR segment */
       {
        records++;                              /* yep update record count */
        if (data_transfer_mode == 1)            /* record to stream? */
         {
          sprintf(line_num_c,"\n");             /* create newline seg */
          len=strlen(line_num_c);    
          if (out_buf_ctl->translate==ON)        /* translate newline if nec */
           for (i=0;i<len;i++)
            line_num_c[i]=otrtable[(int)line_num_c[i]];
          (write_rtn)(odevice,(unsigned char *)line_num_c,len);  /* write out newine seq */ 
          out_bytes+=len;                       /* update byte count */
         }
       }
      if (in_buf_ctl->records_>0 && idevice->data_mode==RECORD &&
          in_buf_ctl->records_==records) break; /* done if request recs read*/
      if (in_buf_ctl->records_>0 && idevice->data_mode==STREAM &&
          out_bytes>=in_buf_ctl->records_) break; /* done if request bytes wrote*/
      if (input_type == TAPE) 
       { 
        if (in_buf_ctl->notify_>0)          /* need to give user notification?*/
         {
          d=div(in_buf_ctl->blocks,in_buf_ctl->notify_); /* do so if appropiate number blks */
          if (d.rem==0 && in_buf_ctl->blocks != current_cut) 
           {
            postblocks(in_buf_ctl->blocks);
            current_cut=in_buf_ctl->blocks;
           }
         }
       }
      else 
       {
        if (in_buf_ctl->notify_>0)          /* need to give user notification?*/
         {
          bytes = (unsigned int)(in_bytes/in_buf_ctl->notify_);
          if (bytes != current_cut)
           {
            postbytes(in_bytes);
            current_cut = bytes;
           }
         }
       }
      if (in_buf_ctl->blocks_>0 && in_buf_ctl->blocks_==in_buf_ctl->blocks) break; /*done if reg blks read*/ 
      if (records % 25 ==0) yield_();
      if (cancel_command==ON) break;
     } 
    i=copy_type;
    if (copy_type==DIRECTORY || copy_type==FILENAME)
     {
      if (odevice->file != 0)
       {
        close(odevice->file);                  /* if copying to a file close */
        odevice->file=0;
       }
     }
    else if (copy_type==TAPE)                /*if copying to a tape */
     wrt_tlr(odevice);                       /* flush and write out trailer */
    if (copy_type==TERMINAL) append_normal_message("\n",4); 
    if (input_type == TAPE ||
        (idevice->tape_type=FILESYSTEM && idevice->format[0] != '\0')) /* print stats */
     {
      if (idevice->data_mode==RECORD)
       sprintf(message_area, "   %d blocks %d records %.0f bytes",
               in_buf_ctl->blocks,records,in_bytes); 
      else
       sprintf(message_area, "             %d blocks %.0f bytes",
               in_buf_ctl->blocks,in_bytes); 
      append_normal_message(message_area,4);
     }
    else
     {
      if (idevice->data_mode==RECORD)
       sprintf(message_area,"             %d records %.0f bytes",
               records,in_bytes); 
      else
       sprintf(message_area,"                        %.0f bytes",
               in_bytes); 
      append_normal_message(message_area,4);
     }
    if (copy_type==TAPE)                    /* copying to tape */
     {
      if (odevice->format[0] == 'F') 
       {
        if (odevice->data_mode==RECORD)
         {
          if (idevice->data_mode==RECORD) out_bytes=records*odevice->rec_len;
          else if (records>0) out_bytes=odevice->rec_len;
          else out_bytes=0;
         }
        else
         {
          records = (int)out_bytes;
          d = div(records,odevice->rec_len);
          if (d.rem >0) d.quot++;
          out_bytes=d.quot*odevice->rec_len;
         }
       }
      if (odevice->data_mode==RECORD)
       {
        if (idevice->data_mode==RECORD)
         {
          if (odevice->format[0]=='U') records=odevice->blocks;
          sprintf(message_area, "   %d blocks %d records %.0f bytes \n",
                  odevice->blocks,records,out_bytes); 
         }
        else
         sprintf(message_area, "              %d blocks %.0f bytes \n",
                 odevice->blocks,out_bytes); 
       }
      else
       sprintf(message_area, "             %d blocks %.0f bytes\n",
               odevice->blocks,out_bytes); 
      append_normal_message(message_area,4);
     }
    else
     {
      if (idevice->data_mode==RECORD)
       sprintf(message_area,"             %d records %.0f bytes\n",
               records,out_bytes); 
      else
       sprintf(message_area,"                        %.0f bytes\n",
               out_bytes); 
      append_normal_message(message_area,4);
     }
    yield_();
    if (cancel_command == ON)
     {
      issue_warning_message("Copy aborted\n");
      cancel_command=OFF;
      break;
     }
   }
  if (copy_type==DIRECTORY || copy_type == FILENAME) 
   if (odevice->file != 0) 
    close(odevice->file);                 /* close file */
  if (idevice != &tapei) free(idevice);   /* free control block */ 
  if (odevice != &tapeo) free(odevice);   /* free control block */ 
  signal(SIGINT,old_handler);       /* restore old signal handler */
  return (0);
 }


int setfilename(char *file_name,struct deviceinfo * tape)
 {
  int i,len;
 
  if (file_name==NULL)                         /* any file name given? */
   {
    issue_warning_message("No file name given default will be used.\n");
    return (0);
   }
  file_name=getparameter(file_name,OFF);
  if (strcmp(file_name,"") == 0)               /* any file name given? */
   {
    issue_warning_message("No file name given default will be used.\n");
    return (0);
   }
  len=strlen((char *)file_name);               /* length of file name */
  if (file_name[0]=='\'')                      /* name start with a '? */
   {
    if (file_name[len-1]=='\'') file_name[len-1]='\0'; /*then must end with ' */
    else
     {
      sprintf(message_area,
              "File name starts with a ' and no closing ' found.\n");
      issue_error_message(message_area);
      return (-1);
     }
    file_name++;                              /* skip over first ' */
    len=strlen((char *)file_name);            /* length of file name */
   }
  else                                        /* if it didn't start with ' */
   for (i=0;i<len;i++) file_name[i]=toupper(file_name[i]); /* trans to upper */
  if (len>17)                           /* long namew will be truncated */
   {
    len=17;
    sprintf(message_area,"File name being truncated to 17 characters,\n");
    issue_warning_message(message_area);
   }
  if (strcmp(file_name,"") == 0)               /* any file name given? */
   {
    sprintf(message_area,"No file name given default will be used.\n");
    issue_warning_message(message_area);
    return (0);
   }
  file_name[len]='\0';                      /* terminate file name */
  strcpy((char *)tape->file_name,(char *)file_name); /* put in control block */ 
  tape->newfile_name=ON;                    /* mark filename has been set */
  return (1);
 }


char *getparameter(char *parameter,int to_upper)
 {
  char *return_value;
  int i;

  return_value=&parameter[strspn(parameter," ")];
  for ( i = strlen(return_value)-1; i >= 0; i--)
   if (return_value[i] == ' ') return_value[i]='\0';
   else break;
  if (to_upper == ON)
   for ( i = strlen(return_value)-1; i >= 0; i--)
    return_value[i]=toupper(return_value[i]);
  return(return_value);
 }


int listfunction(struct deviceinfo *tape, int start_file, int end_file, 
                 int docsw ,int datesw, int notify)
 {
  struct buf_ctl buf_ctl;
  char type [20];
  int file_num, records, len, i,offset,doclen;
  double bytes;
  char *doc;
  unsigned char *file_name;
  unsigned char buffer[45];
  unsigned char seg_len;
  int output;
  struct tm tm;
  char date[30];
  div_t d;
  unsigned int current_cut, avg;
   
  memset(&buf_ctl, 0, sizeof(buf_ctl));
  
  cancel_command=OFF;
  buf_ctl.fs=0;                          /* don't activate fs processing yet */
         /* fiqure labeling  mode and active fs processing if necessary */
  if (tape->lp ==0) strcpy(type,"no label processing");
  else if (tape->tape_type==UNLABELED) strcpy(type,"unlabeled");
  else if (tape->tape_type==IBM_LABEL) strcpy(type,"IBM labeled");
  else if (tape->tape_type==ANSI_LABEL) strcpy(type,"ANSI labeled");
  else if (tape->tape_type==TOS_LABEL) strcpy(type,"TOS labeled");
  else if (tape->tape_type==VLO_LABEL) strcpy(type,"VLO labeled");
  else if (tape->tape_type==FS_UNLABELED)
   {strcpy(type,"unlabeled *FS");buf_ctl.fs=1;}
  else if (tape->tape_type==FS_IBMLABEL)
   {strcpy(type,"IBM labeled *FS");buf_ctl.fs=1;}
  else if (tape->tape_type==FS_VLOLABEL)
   {strcpy(type,"VLO labeled *FS");buf_ctl.fs=1;}
  else strcpy(type,"Undefined");
        /*  tell user */
  if (tape==&tapeo) output=5;
  else output=6;
  sprintf(message_area,"         Listing for %s tape %s \n",type,tape->volume);
  append_normal_message(message_area,output);
  for (file_num=start_file;file_num<=end_file;file_num++) /* cycle */
   {
    if (posn(tape,file_num) == 1)                   /* position - if  LEOT? */
     {
      tape->leot=1;
      if (file_num==start_file)                     /* while get to first? */
       {
        sprintf(message_area,"Logical End of Tape occurs before starting file.\n");
        issue_error_message(message_area);
        return(-1);
       }
      sprintf(message_area,"Logical End of Tape reached.\n"); /* tell user and return */
      issue_warning_message(message_area);
      return(0);
     }
    current_cut=records=buf_ctl.blocks=0;  /* initialize record & block count */
    buf_ctl.max_blk=0;
    bytes=0.0;                             /* initialize byte count */
    while (1)                              /* read through file */ 
     {
      unsigned char *rec;
      rec=(tape->read_rtn)(tape,&buf_ctl); /* read a record */ 
      if (rec == NULL && tape->fatal==ON) return (-1);
      if (rec==NULL) break;                /* if EOF - don this file */
      if (buf_ctl.eor==1) records++;       /* EOR? - incr record count */
      bytes+=buf_ctl.seg_len;              /* update byte count */
      if (records % 25 ==0) yield_();
      if (cancel_command == ON) goto list_aborted;
      if (notify>0)          /* need to give user notification?*/
       {
        d=div(buf_ctl.blocks,notify); /* do so if appropiate number blks */
        if (d.rem==0 && buf_ctl.blocks != current_cut) 
         {
          postblocks(buf_ctl.blocks);
          current_cut=buf_ctl.blocks;
         }
       }
     }
    date[0]='\0';
    if (datesw != 0 && tape->create_date>0)
     {
      juliantime(tape->create_date,&tm);
      strftime(date,20,"%b. %d, %Y ",&tm);
      strcat(strcpy(message_area," cre="),date);
      strcpy(date,message_area);
     } 
    if (buf_ctl.fs == 0)                   /* if not an FS tape listing */
     {
      sprintf(type,"%s(%d",tape->format,tape->block_len);/* get format */
      len=strlen(type);
      if (strcmp((char *)tape->format,"F") == 0 || tape->rec_len == 0)
       sprintf(&type[len],")");
      else sprintf(&type[len],",%d)",tape->rec_len);
       /* print info on the file */
      if (strcmp((char *)tape->format,"U") == 0)
       {
        if (buf_ctl.blocks==0) avg=0;
        else avg = (unsigned int)(bytes/buf_ctl.blocks);
        sprintf(message_area,"File %5d %s format=%-16s %smax=%d avg=%d blks=%d bytes=%.0f\n",
             file_num,tape->file_name,type,date,buf_ctl.max_blk,avg,
             buf_ctl.blocks,bytes);
        
       }
      else
       sprintf(message_area,"File %5d %s format=%-16s %sblks=%d recs=%d bytes=%.0f\n",
             file_num,tape->file_name,type,date,buf_ctl.blocks,records,
             bytes);
      append_normal_message(message_area,output);
      yield_();
      if (cancel_command == ON) goto list_aborted;
     }
    else                                   /* FS files are a little different */
     {
      if (tape->doc_len != 0) doc="=";     /* documetion associated with file */
      else doc=" ";             
      file_name=tape->file_name;           /* set file name */
      if (tape->version >1)              /* if this no the first version set */
       {
        for (i=31;i>0;i--) if (tape->file_name[i] != ' ') break; /* version # */
        i=i+1;
        sprintf((char *)buffer,"%.*s[%d]",i,tape->file_name,tape->version);
        file_name=buffer;
       }
        /* print info about file */ 
      sprintf(message_area,"FS file %5d %1s%-32s %sblks=%d recs=%d bytes=%.0f\n",
             file_num,doc,file_name,date,buf_ctl.blocks,records,
             bytes);
      append_normal_message(message_area,output);
      yield_();
      if (cancel_command == ON) goto list_aborted;
      records = 0;
      if (docsw == 1 && tape->doc_len>0) /* need to print documentation */
       {
        offset=0;                           /* offset to start of doc */
        if (memcmp(&tape->fsheader->header_id,magic_id,4)==0) offset=12;
        doclen=tape->doc_len-offset;        /* max ammount of info */
        do 
         {
          memcpy(&seg_len,tape->fsheader->doc+offset,1); /* get line len */
          if (seg_len == 0) break;          /* if none done? */
          for (i=0;i < (int)seg_len;i++)    /* translate to ASCII */ 
           tape->buffer[i] = EBCASC[tape->fsheader->doc[offset+1+i]];
          tape->buffer[i]='\0';             /* terminate line */
          sprintf(message_area," %s\n",tape->buffer); /* print line */
          append_normal_message(message_area,output);
          records++;
          offset+=1+seg_len;                /* point to next line */
          doclen-=(1+seg_len);              /* length still to process */
          if (doclen<0)                     /* oops */
           {
            sprintf(message_area,"Ran off end of documentation\n");
            issue_error_message(message_area);
            break;
           }
           if (records % 5 ==0) yield_();
           if (cancel_command == ON) goto list_aborted;
         } while(doclen>0);
       }
     } 
   }
  return (0);
  list_aborted:
   issue_warning_message("List Aborted");
  return (-1);
 }


long displayfunction(struct deviceinfo *tape,long blocks, long length,
                     long hex, long ebcd, long ascii)
 {
  struct tapestats tapes;
  struct buf_ctl buf_ctl;
  unsigned char *buf;
  unsigned char buffer[1060];
  unsigned char segment[90];
  READ_RTN read_rtn;
  div_t d;
  char *type, *dev, *lp, *next, *drtype;
  int i, j, len, ints;
  int offset, seg_length, quanity, max_seglen;
  unsigned char *status;
  char * open_mode;
  char *path_name;
  int records;
   
  memset(&buf_ctl, 0, sizeof(buf_ctl));
  
  cancel_command=OFF;
  if (tape->tape_type == FILESYSTEM)
   {
    path_name=getpath((char *)tape->name);       /* get fully qualified name */
    status=verfname((unsigned char *)path_name,&buf_ctl,INPUT); /* get type of dev/file */
    if (status==NULL)                           /* could get and info? */
     {
      sprintf(message_area,
       "File `%s' does not exist\n",tape->name); /*no - must not exist */
      issue_error_message(message_area);
      return (-1);
     }
    if (buf_ctl.copy_type==TAPE)          /* read from tape? */
     {
      sprintf(message_area,"Expecting a filename not a Tape.\n");
      issue_error_message(message_area); 
      return (-1);
     }
    else if (buf_ctl.copy_type==DIRECTORY) /* read from directory? */
     {
      sprintf(message_area,"Dumping a directory is not supported(yet)\n");
      issue_error_message(message_area); 
      return (-1);
     }
    else if (buf_ctl.copy_type==FILENAME) /* read from File? */
     {
      if (infile_recording_mode == STREAM) open_mode="rb"; /*If declared stream - open bin*/
      else open_mode="r";                      /* else open for text */
      tape->unixfile=fopen((char *)path_name,open_mode); /* open file */
      if (tape->unixfile==NULL)           /* could file be opened for read?*/
       {
        issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
        sprintf(message_area,
                "Could not open '%s'\n.",tape->name); /* no -tell user */
        issue_error_message(message_area); 
        return (-1);                           /* done */
       }
      tape->translate=OFF;                  /* don't translate */
      tape->data_mode=infile_recording_mode; /* set data mode */
      tape->trtable=EBCASC;              /* incase transation gets enabled */
      tape->otrtable=ASCEBC;
     }
    else                                      /* read from WHAT! */
     {
      sprintf(message_area,
              "Displaying from '%s' is not supported\n",tape->name);
      issue_error_message(message_area); 
      return (-1);
     }
    sprintf(message_area," File %s\n",tape->name);
    append_normal_message(message_area,4);
    read_rtn=&unix_read;
   } 
  else
   {
    if (tape->tape_type == UNLABELED) type="Unlabeled"; /* tape_type -> strng */
    else if (tape->tape_type == IBM_LABEL) type = "IBM labeled";
    else if (tape->tape_type == ANSI_LABEL) type = "ANSI labeled"; 
    else if (tape->tape_type == TOS_LABEL) type = "TOS labeled";
    else if (tape->tape_type == VLO_LABEL) type = "VLO labeled";
    else if (tape->tape_type == FS_UNLABELED) type = "Unlabeled *FS";
    else if (tape->tape_type == FS_IBMLABEL) type = "IBM labeled *FS";
    else if (tape->tape_type == FS_VLOLABEL) type = "VLO labeled *FS";
    else type="Undefined";
    if (tape==&tapei) dev="Input";            /* set input/ouput type */
    else if (tape==&tapeo) dev="Output";
    else dev="Undefined";
    if (tape->drive_type == DEV_AWSTAPE) drtype = "AWS";
    else if (tape->drive_type == DEV_FAKETAPE) drtype = "Fake";
    else drtype = "real";
      /* give info on device and volume label */
    sprintf(message_area,"%s %s Tape,%s Device=%s, volume=%s, owner=%s\n",
            type,drtype,dev,tape->name,tape->volume,tape->owner);
    append_normal_message(message_area,4);
    yield_();
    if (cancel_command == ON) goto display_abort;
    if (tape->lp == 0) lp="OFF";             /* label processing on or off */
    else if (tape->lp ==1) lp="ON";
    else lp = "Undefined";
    tapestatus(tape, &tapes);                /* get positiion information */
     /* give information on currenmt position */
    sprintf(message_area," LP=%s, File=%d, Block=%d, DSN=%s,",
                    lp,tape->position,tapes.mt_blkno,tape->file_name);
    if (tape->format[0] == '\0')
     sprintf(&message_area[strlen(message_area)]," format=undefined\n");
    else                                     /* print format information */
     {
      sprintf(&message_area[strlen(message_area)]," format=%s%.1s(%d",
                       tape->format,tape->fmchar,tape->block_len);
      if (strcmp((char *)tape->format,"F") !=0 && tape->rec_len != 0)
       sprintf(&message_area[strlen(message_area)],",%d",tape->rec_len);
      sprintf(&message_area[strlen(message_area)],")\n");
     } 
    append_normal_message(message_area,4);
    yield_();
    if (cancel_command == ON) goto display_abort;
    if (tape->translate == ON)              /* print translation information */
     {
      sprintf(message_area," Translate=on,");
      next=&message_area[strlen(message_area)];
      if (tape->trtable == EBCASC) sprintf(next," Trtable=EBCD");
      else if (tape->trtable == MTSASC) sprintf(next," Trtable=OLDMTS");
#if SYSTEM == MSVC
      else sprintf(next, "Trtable=undefined");
#else
      else sprintf(next," Trtable=undefined %d", (int)(tape->trtable));
#endif
     }
    else if (tape->translate ==OFF) sprintf(message_area," Translate=off,");
    else sprintf(message_area," Translate=undefined");
    next=&message_area[strlen(message_area)];
    if (tape->data_mode==STREAM)
     sprintf(next,", Recording_mode=Stream"); /* print recording mode info */
    else if(tape->data_mode==RECORD) sprintf(next,", Recording_mode=Record");
    else sprintf(next,", Recording_mode=undefined");
    next=&message_area[strlen(message_area)];
    if (show_warnings == OFF) sprintf(next,", Warn=off"); /* print warning mode */
    else if (show_warnings == ON) sprintf(next,", Warn=on");
    else sprintf(next,", Warn=undefined");
    next=&message_area[strlen(message_area)];
    if (tape->datecheck == OFF) 
     sprintf(next,", Date checking=off");       /* print datechecking info */
    else if (tape->datecheck == ON) sprintf(next,", Date checking=on");
    else sprintf(next,", Date checking=undefined");
    next=&message_area[strlen(message_area)];
    if (tape->tape_type==ANSI_LABEL)             /* print block prefix info */
     sprintf(next,", blkpfx=%d",tape->blkpfx);
    else if (tape->tape_type == UNLABELED && tape->blkpfxs==ON)
     sprintf(next,", blkpfx=%d",tape->blkpfx);
    next=&message_area[strlen(message_area)];
    sprintf(next,"\n"); 
    append_normal_message(message_area,4);
    if (tape->create_date>0)
     {
      struct tm tm;
      char date[20];
      juliantime(tape->create_date,&tm);
      strftime(date,20,"%b. %d, %Y",&tm);
      strcat(strcpy(message_area," Creation date="),date);
      if (tape->cur_expiration>0)
       {
        juliantime(tape->cur_expiration,&tm);
        strftime(date,20,"%b %d, %Y",&tm);
        strcat(strcpy(message_area,", Expiration date="),date);
       }
      strcat(message_area,"\n");
      append_normal_message(message_area,4);
     }
    read_rtn=&ru_read;
   }
  if (tape->data_mode == RECORD) quanity=blocks;
  else quanity=length;
  yield_();
  if (cancel_command == ON) goto display_abort;
  records=offset=0;
  if (hex) max_seglen=32;
  else max_seglen=80;
  if (quanity>0)                                   /* display and blocks? */
   for(i=0;i<quanity;)
    {
     int k;
     buf=(read_rtn)(tape,&buf_ctl); /* read a block */
     if (buf_ctl.seg_len<0)
      issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
     if (tape->data_mode == RECORD)
      {
       tapestatus(tape, &tapes);                /* get positiion information */
       if (tape->tape_type != FILESYSTEM) 
        sprintf((char *)buffer,"\nBlock %d\n",tapes.mt_blkno);
       else
        {
         sprintf((char*)buffer,"\nRecord %d\n",records);
         records++;
        }
       append_normal_message((char *)buffer, 4);   /* yep - skip line */      
      }
     if (buf==NULL) 
      {
       append_normal_message("\n End of file reached\n",4);
       break;                     /* if no block done */
      }
     if (buf_ctl.seg_len<length) len=buf_ctl.seg_len; /* ammount of blk to pr */
     else len=length; 
     if(tape->data_mode==STREAM) len=length; 
     offset=0;
     while (len>0)
      {
       k=0;
       if (tape->data_mode == RECORD)
        {
         if (len>max_seglen) seg_length=max_seglen;
         else seg_length=len;
         memcpy(segment,&buf[offset],seg_length);
         len -= seg_length;
        }
       else 
        {
         if (len>max_seglen) seg_length= max_seglen;
         else seg_length=len;
         if (buf_ctl.seg_len>=seg_length)
          {
           memcpy(segment,&buf[offset],seg_length);
           buf_ctl.seg_len-= seg_length;
           len -= seg_length;
          }
         else  
          {
           int old_len;
           old_len=0;
           do
            {  
             memcpy(&segment[old_len],&buf[offset],buf_ctl.seg_len);
             old_len+=buf_ctl.seg_len;
             len-=buf_ctl.seg_len;
             offset=0;
             if (buf==NULL) 
              {
               append_normal_message("\n End of file reached\n",4);
               return 0;                     /* if no block done */
              }
             buf=(read_rtn)(tape,&buf_ctl); /* read a block */
             if (buf!=NULL)
              {
               if (buf_ctl.seg_len<seg_length-old_len) continue;
               offset=seg_length-old_len;
               memcpy(&segment[old_len],buf,offset); 
               len-=offset;                    
               buf_ctl.seg_len-=offset;
               offset-=seg_length;
               break;
              } 
             buf_ctl.seg_len=0;
             break;
            } while(1);
          }
        }  
       if (hex)                                    /* print out in Hex? */ 
        {
         if (tape->data_mode==RECORD) sprintf((char *)&buffer[k],"%8.8X",offset);
         else sprintf((char *)&buffer[k],"%8.8X",i);
         k+=8;
         d=div(seg_length,4);
         for (j=0;j <seg_length;j++)                   /* print out in hex rep */
          {
           if (j%4 == 0) {buffer[k] = ' '; k++;}
           ints=segment[j];                     /* invert if necessary */
           sprintf((char *)&buffer[k],"%2.2X",ints);    /* print next four bytes */
           k+=2;
           if (k >= 1024) {buffer[k]='\0'; append_normal_message((char *)buffer,4); k=0;}
           if (cancel_command == ON) break;
          }
         buffer[k]='\n';
         k++;
         yield_();
         if (cancel_command == ON) goto display_abort;
        }      
       if (ascii)                            /* print ascii rep */
        {
         int i;
         if (hex)
          {
           memset(&buffer[k],' ',9);
           k+=9;
          }
         for (i=0;i<seg_length;i++)                     /* print each char */
          {
           if (hex) {buffer[k]=' ';k++;}
           if (hex && i>0 && i%4 == 0) {buffer[k]=' ';k++;}
           if (isprint(segment[i])==0) buffer[k]='?'; /* not legal char? */
           else buffer[k]=segment[i];                /* else print legal char */
           k++;
           if (k >= 1024) {buffer[k]='\0'; append_normal_message((char *)buffer,4); k=0;}
           if (cancel_command == ON) break;
          }
         buffer[k]='\n';
         k++;
         yield_();
         if (cancel_command == ON) goto display_abort;
        }
       if (ebcd)                            /* print ascii rep */
        {
         int i;
         if (hex)
          {
           memset(&buffer[k],' ',9);
           k+=9;
          }
         for (i=0;i<seg_length;i++) segment[i]=EBCASC[segment[i]]; /* trans block to EBCD */
         for (i=0;i<seg_length;i++)                     /* print each char */
          {
           if (hex) {buffer[k]=' ';k++;}
           if (hex && i>0 && i%4 == 0) {buffer[k]=' ';k++;}
           if (isprint(segment[i])==0) buffer[k]='?'; /* not legal char? */
           else buffer[k]=segment[i];                /* else print legal char */
           k++;
           if (k >= 1024) {buffer[k]='\0'; append_normal_message((char *)buffer,4); k=0;}
           if (cancel_command == ON) break;
          }
         buffer[k]='\n';
         k++;
         yield_();
         if (cancel_command == ON) goto display_abort;
        }
       buffer[k]='\0';
       append_normal_message((char *)buffer,4); 
       offset+=seg_length;
       if (tape->data_mode == STREAM) {length-= seg_length; i+=seg_length;}
       yield_();
       if (cancel_command == ON) goto display_abort;
      }
     if (tape->data_mode == RECORD) i++;
    }
  if (tape->tape_type == FILESYSTEM && tape->unixfile != NULL) 
   fclose(tape->unixfile);
  append_normal_message("\n",4);                        /* skip line */
  return (0);
  display_abort:
   issue_warning_message("Display Aborted");
  return (-1);
 }


int dittofunction(char *tapename,long notify,int tape_type) 
 {
  struct buf_ctl input_ctl, output_ctl;
  int rc,blocking,translate,lp;
 
  memset(&input_ctl, 0, sizeof(input_ctl));
  memset(&output_ctl, 0, sizeof(output_ctl));
  
  input_ctl.iofrom=output_ctl.iofrom=-1;
  input_ctl.translate=-1; /* translate mode not set */
  output_ctl.translate=-1; /* translate mode not set */
  input_ctl.path=NULL;  /* no pathname for input given */
  output_ctl.path=NULL; /* no pathname for output given */
  output_ctl.warn=show_warnings;
  input_ctl.linemode=0;           /* don't write out line numbers */
  input_ctl.trtable=NULL;         /* no overriding translate mode */
  input_ctl.format[0]=output_ctl.format[0]='\0';
  input_ctl.notify_=notify;
  output_ctl.notify_=0;
  input_ctl.blocks_=output_ctl.blocks_=0;
  input_ctl.records_=output_ctl.records_=0;
  tapeo.label=UNLABELED;           /* set label type  to UNLABLELLED */
  tapeo.drive_type = tape_type;  /* In case it's a simulated tape */            
  rc= tpopen(OUTPUT,(unsigned char *)tapename,0,0);      /* open and init tape */
  if (rc<0) return(-1);
  tapeo.label=OFF;                  /* disarm labeling */
  blocking=tapei.blocking;
  translate=tapei.translate;
  lp=tapei.lp;
  tapeo.blocking=tapei.blocking=tapeo.translate=tapei.translate=OFF;
  lpfunction(&tapei,OFF); 
  output_ctl.iofrom=1;   
  input_ctl.start_file=1;
  input_ctl.end_file=LONG_MAX;
  copyfunction(&input_ctl,&output_ctl,OFF);
  tapei.blocking=blocking;
  tapei.translate=translate;
  lpfunction(&tapei,lp); 
  closetape(&tapeo); 
  tpopen(OUTPUT,(unsigned char *)tapename,0,0);
  return (1);
 }


int setformat(struct deviceinfo *tape, struct buf_ctl *buf_ctl) 
 { 
  if (buf_ctl->fmchar[0]=='M' && tape->tape_type==ANSI_LABEL) /* legal FMCHAR? */
   {
    sprintf(message_area,"access method of M illegal for ANSI tapes\n");/* no */
    issue_error_message(message_area);
    return (-1);
   }  
  tape->read_rtn=buf_ctl->read_rtn;              /* set read routine */
  strcpy((char *)tape->format,(char *)buf_ctl->format); /*fill in control block*/
  tape->block_len=buf_ctl->blk_len; 
  tape->fmchar[0]=buf_ctl->fmchar[0];
  tape->rec_len=buf_ctl->rec_len;
  if (buf_ctl->fs==1) getfsname(tape);  /* if FS tape - get filesave name */ 
  return (1);
 }


char *expirefunction(struct deviceinfo *tape, char *date)
 {
  char *rc;
  struct tm tm;

#if SYSTEM == MSVC
  /* This stupid thing doesn't work anyway so just punt.  It's only
     used for the "expire" command */
  rc = NULL;
#else
  time_t now;
  struct tm *tm2;
   
  rc=strptime(date,"%b%d%Y",&tm);  /* check for mmddyyyy */
  if (rc == NULL)
   rc=strptime(date,"%b%d,%Y",&tm); /* check for mmdd,yyyy */
  if (rc == NULL)
   rc=strptime(date,"%b.%d%Y",&tm); /* check for mm.ddyyyyy */
  if (rc == NULL)
   rc=strptime(date,"%b.%d,%Y",&tm); /* check for mm.dd,yyyy */
  if (rc == NULL)
   rc=strptime(date,"%b%d%y",&tm); /* ckeck for mmddyy */
  if (rc == NULL)
   rc=strptime(date,"%b%d,%y",&tm); /* check for mmdd,yy */
  if (rc == NULL)
   rc=strptime(date,"%b.%d%y",&tm); /* check for mm.ddyy */
  if (rc == NULL)
   rc=strptime(date,"%b.%d,%y",&tm); /* check for mm.dd,yy */
  if (rc == NULL)
   rc=strptime(date,"%m/%d/%Y",&tm); /* check for mm/dd/yyyy */
  if (rc == NULL)
   rc=strptime(date,"%m/%d/%y",&tm); /* check for mm/dd/yy */
  if (rc == NULL)
   rc=strptime(date,"%m-%d-%Y",&tm); /* check for mm-dd-yyyy */
  if (rc == NULL) 
   rc=strptime(date,"%m-%d-%y",&tm); /* check for mm-dd-yy */
  if (rc == NULL)                      /* need to try forms with out a year? */
   {
    if (rc == NULL)
     rc=strptime(date,"%b%d",&tm); /* yep - check for mmdd */
    if (rc == NULL) 
     rc=strptime(date,"%b.%d",&tm); /* check for mm.dd */
    if (rc == NULL) 
     rc=strptime(date,"%m/%d",&tm); /* check for mm/dd */
    if (rc == NULL)
     rc=strptime(date,"%m-%d",&tm); /* check for mm-dd */
    if (rc != NULL)                   /* need to supply the year */
     {
      time(&now);                     /* yep - get todays date */
      tm2=localtime(&now);            /* get in exploded format */
      if (tm.tm_mon<tm2->tm_mon) tm.tm_year=tm2->tm_year+1; /* month < curr? */
      else if (tm.tm_mon==tm2->tm_mon && tm.tm_mday >= tm2->tm_mday) 
       tm.tm_year=tm2->tm_year+1;     /* month == curr and  day <= curr */
      else tm.tm_year=tm2->tm_year;   /* all other cases */ 
     }
   }
#endif
  if (rc == NULL)                    /*did  we recognize the date */
   {
    sprintf(message_area," '%s' is an unrecognized date form\n",date);
    issue_error_message(message_area);
    return(rc);
   }
  mktime(&tm);                     /* transform to julian form */
  sprintf(julian_date,"%03d%03d",tm.tm_year,tm.tm_yday+1); /* in char */
  if (julian_date[0]=='0') julian_date[0]=' '; /*change leading zero to ' ' */
  memcpy(tape->expiration,julian_date,6); /* save for header */
  strftime(julian_date,20,"%b. %d, %Y",&tm);
  return(julian_date);
 }


int blkpfxfunction(struct deviceinfo *tape,struct buf_ctl *buf_ctl, int blkpfx, int blkpfxl)
 {
  if (blkpfx <0 || blkpfx>99)        /* yep - legal value? */
   {
    issue_error_message("Block prefix must be between 0 and 99\n");/*no*/
    return (-1);
   }
  if (blkpfxl== ON)                  /* write BDW? */
   {
    if (blkpfx < 4)                  /* yep have room? */
      {
      issue_error_message("Block prefix must be >=4 characters to hold block length.\n"); 
      return (-1);
     }
    if (tape!=NULL) tape->blkpfxl=ON;               /* set write BDW */
    else buf_ctl->blkpfxl=ON;
   }
  buf_ctl->blkpfx=blkpfx;              /* set block prefix length */
  if (tape !=NULL)
   {
    tape->blkpfx=blkpfx;               /* set block prefix length */
    tape->blkpfxs=ON;                 /* mark block prefix set */
   }   
  return (1);
 }


int lpfunction(struct deviceinfo *tape,int lp)
 {
  struct tapestats tapes;

  tape->lp=lp;                             /* enable/disable */
  if (lp==0)                               /* disable? */
   {
    tape->file_name[0]='\0';               /* yep - no filename then */
    strcpy((char *)tape->format,"U");      /* set default format */
    tape->block_len=32767;
    tape->rec_len=0;
    tapestatus(tape,&tapes);               /* get where we are */
    tape->position=tapes.mt_fileno+1;      /* set logical file number */
    tape->read_rtn=&u_read;              /* set read/write rtn for unlabeled */
    tape->rd_hdr_rtn=&rd_nohdr;
    tape->rd_tlr_rtn=NULL;
    if (tape==&tapeo)
     {
      tape->wrt_hdr_rtn=&wrt_nohdr;
      tape->wrt_tlr_rtn=&wrt_notlr;
     }
   }
  else                                     /* enable label processing */
   {
    tapestatus(tape,&tapes);               /* get info on  current position */
    if (tape->tape_type == UNLABELED)     /* going to unlablled? */
     {
      tape->position=tapes.mt_fileno+1;    /* set Logical number = physical+1 */
      tape->rd_hdr_rtn=&rd_nohdr;          /* reset in/out header put rtns */
      tape->rd_tlr_rtn=NULL;
      if (tape==&tapeo)
       {
        tape->wrt_hdr_rtn=&wrt_nohdr;
        tape->wrt_tlr_rtn=&wrt_notlr;
       }
     }
    else if (tape->tape_type == VLO_LABEL) /* going to VLO? */
     {
      tape->position=tapes.mt_fileno+2;    /* set Logical number = physical+2 */
      tape->rd_hdr_rtn=&rd_nohdr;          /* reset in/out header put rtns */
      tape->rd_tlr_rtn=NULL;
      if (tape==&tapeo)
       {
        tape->wrt_hdr_rtn=&wrt_nohdr;
        tape->wrt_tlr_rtn=&wrt_notlr;
       }
     }
    else if (tape->tape_type == IBM_LABEL) /* going to IBM labeled? */
     {
      tape->position=(tapes.mt_fileno/3)+1; /* logical number = (phys/3)+1 */ 
      tape->rd_hdr_rtn=&rd_ibmhdr;         /* reset in/out header routines */
      tape->rd_tlr_rtn=NULL;
      if (tape==&tapeo) 
       {
        tape->wrt_hdr_rtn=&wrt_ibmhdr;
        tape->wrt_tlr_rtn=&wrt_ibmtlr;
       }
     }
    else if (tape->tape_type == ANSI_LABEL) /* going to ansi labeled? */
     {
      tape->position=(tapes.mt_fileno/3)+1; /* logical number=(phys/3)+1 */
      tape->rd_hdr_rtn=&rd_ansihdr;         /* reset in/out header routines */
      tape->rd_tlr_rtn=NULL;
      if (tape==&tapeo) 
       {
        tape->wrt_hdr_rtn=&wrt_ansihdr;
        tape->wrt_tlr_rtn=&wrt_ansitlr;
       }
     }   
    else if (tape->tape_type == TOS_LABEL) /* going to tos labeled? */
     {
      tape->position=(tapes.mt_fileno/3)+1; /* logical number=(phys/3)+1 */
      tape->rd_hdr_rtn=&rd_toshdr;          /* reset in/out header routines */
      tape->rd_tlr_rtn=NULL;
      if (tape==&tapeo) 
       {
        tape->wrt_hdr_rtn=&wrt_toshdr;
        tape->wrt_tlr_rtn=&wrt_tostlr;
       }
     }   
    else if (tape->tape_type == FS_UNLABELED) /* going to FS unlabeled? */
     {
      tape->position=tapes.mt_fileno;       /* logical number = phys */
      if (tape->position<1) tape->position=1; /* skip null file if necessary */
      tape->rd_hdr_rtn=&rd_nohdr;           /* reset in/out header routines */
      tape->rd_tlr_rtn=NULL;
      if (tape==&tapeo)
       {
        tape->wrt_hdr_rtn=&wrt_nohdr;
        tape->wrt_tlr_rtn=&wrt_notlr;
       }
     }   
    else if (tape->tape_type == FS_IBMLABEL) /* going to FS labeled? */
     {
      tape->position=(tapes.mt_fileno/3);    /* logical = (physical/3) */
      if (tape->position<1) tape->position=1; /* skip null file if necessary */
      tape->rd_hdr_rtn=&rd_ibmhdr;           /* set in/out header rtns */
      tape->rd_tlr_rtn=NULL;
      if (tape==&tapeo) 
       {
        tape->wrt_hdr_rtn=&wrt_ibmhdr;
        tape->wrt_tlr_rtn=&wrt_ibmtlr;
       }
     }
    else if (tape->tape_type == FS_VLOLABEL) /* going to VLO FS tape? */
     {
      tape->position=tapes.mt_fileno-1;     /* logical =physical -1 */
      if (tape->position<1) tape->position=1; /* skip hdr null file if necess */
      tape->rd_hdr_rtn=&rd_nohdr;            /* set in/out header rtns */
      tape->rd_tlr_rtn=NULL;
      if (tape==&tapeo)
       {
        tape->wrt_hdr_rtn=&wrt_nohdr;
        tape->wrt_tlr_rtn=&wrt_notlr;
       }
     }   
    else {fprintf(stderr,"Unknown tape type\n"); exit (2);} /* oops */
    if (posn(tape,tape->position) == 1)         /* position to start of file */
     issue_warning_message("Warning Logical End of Tape indicated.\n");/* oops-LEOT */
   }
  return 0;
 }


int rewindfunction (struct deviceinfo *tape)
 {
  taperew(tape);
  tape->fatal=0;
  tape->leot=OFF;
  return (posn(tape,1));
 }


int terminatefunction (struct deviceinfo *tape,int fileNumber)
 {
  int rc, lp, i;
  struct tapestats tapes;

  if (fileNumber < 1)
   {
    issue_error_message("Position value must be positive\n");
    return(-1);
   }
  else if (fileNumber == LONG_MAX)
   {
    issue_error_message("EOT is not an acceptable value for terminate\n");
    return(-1);
   }
  lp=tape->lp;
  tape->lp=HEADER_RTN;                 /* pretend we are a header routine */
  if (lp == ON)
   {
    if (tape->tape_type == IBM_LABEL || 
        tape->tape_type == TOS_LABEL || 
        tape->tape_type == ANSI_LABEL)
     fileNumber=(fileNumber-1)*3+1;
   }
  rc = posn(tape,fileNumber); 
  tape->lp = lp;                       /* restore label mode */
  if (rc < 0) return -1;              /* position failed */
  rc = tapestatus(tape, &tapes);       /* get info on tapes current position */
  if (rc == 0 && fileNumber != tapes.mt_fileno+1)
   {
    issue_error_message("Specified file does not exist\n");
    return(-1);
   }
  if (fileNumber == 1)
   {
    if (tape->tape_type == IBM_LABEL || tape->tape_type == TOS_LABEL)
     {
     }
    else if (tape->tape_type == ANSI_LABEL)
     {
     }
   } 
  for (i=0;i<3;i++)                     /* terminate tape - just incase */
   {
    if (tape->realtape == YES)
     {   
      tpwrite(tape,(unsigned char*)"                     ",20);
      tapebsr(tape,1); 
     }
    tapeweof(tape,1);
   }
  tapebsf(tape,3);
  return 1;
 }


int juliantime(int time,struct tm *tm)
 {
  static int table[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  int  i;
  div_t d;
     
  d=div(time,1000);
  tm->tm_year=d.quot;
  d.quot+=1900;
  tm->tm_mday=d.rem;
  tm->tm_yday=d.rem-1;
  tm->tm_wday=0;
  tm->tm_isdst=0;
  tm->tm_sec=0;
  tm->tm_min=0;
  tm->tm_hour=0;
  if (d.quot % 400 == 0 ||
      (d.quot %4 ==0 && d.quot %400 !=0)) table[1]=29;     
  else table[1]=28; 
  for (i=0; i<12; i++) 
   {
    if (tm->tm_mday <= table[i]) break;
    tm->tm_mday -= table[i];
   }
  tm->tm_mon=i;
  return 0;
 }


int getTranslateTable(struct deviceinfo *tape)
 {
  if (tape->trtable == EBCASC) return(0);
  if (tape->trtable == MTSASC) return(1);
  return(-1);
 }


int setTranslateTable(struct deviceinfo *tape, int table)
 {
  unsigned char *itrtable, *otrtable;

  itrtable=otrtable=NULL;
  if (table == 0) 
   {
    itrtable=EBCASC;
    otrtable=ASCEBC;
   }
  else if (table==1)
   {
    itrtable=MTSASC;
    otrtable=ASCMTS;
   }
  tape->trtable=itrtable;
  if (tape == &tapeo) tape->otrtable=otrtable;
  else tape->otrtable=NULL;
  return (0);
 }

unsigned short swap_short(unsigned short s)
 {
  return ((s << 8) & 0xff00) + ((s >> 8) & 0x00ff);
 }
 
unsigned int swap_long(unsigned int x)
 {
  return ((x << 24) & 0xff000000) + ((x >> 24) & 0xff) +
         ((x << 8) & 0xff0000) + ((x >> 8) & 0xff00);
 }
