#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "lbltp.h"
#if SYSTEM == SOLARIS || SYSTEM == OS4
#include <sys/mtio.h>
#endif
#if SYSTEM == UNIXWARE
#include <sys/st01.h>
#include <sys/tape.h>
#include <sys/scsi.h>
#endif
#if SYSTEM == OS4 || !defined SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif 
extern char message_area[4096];
#define rd_ibg(function)                                                       \
  len_read = read(tape->file,tape->buffer,12);           /* read in the IBG */ \
  if (len_read !=12)                      /* Ibg must be 12 chars in length */ \
   return (gap_err(function,tape));                                            \
  tape->buffer[12]='\0';                                 /* end IBG string */  \
  if (strspn((char*)tape->buffer,"0123456789ABCDEF") != 12)                    \
   return (gap_err(function,tape));            /* invalid characters in IBG */ \
  length[4]='\0';                                                              \
  memcpy(length,tape->buffer,4);              /* get length previous record */ \
  len_prev = strtol(length,&stopchar,16);                /* numeric form */    \
  memcpy(length,tape->buffer+4,4);          /* get length of current record */ \
  len_this = strtol(length,&stopchar,16);                /* numeruc form */    \
  memcpy(length,tape->buffer+8,4);                       /* get check word */  \
  len_check = strtol(length,&stopchar,16);               /* numeric form */    \
  test = (unsigned)len_prev ^ (unsigned)len_this;   /* calculate check word */ \
  if (test != (unsigned)len_check)                                             \
   return(gap_err(function,tape));                    /* valdate check word */ \
  if (len_prev <0 || len_prev > 65535)                                         \
   return (gap_err(function,tape));                      /* valdate lengths */ \
  if (len_this <0 || len_this > 65535)                                         \
   return (gap_err(function,tape));                     /* validate lengths */ 
 
int tpsync(struct deviceinfo *tape)
 {
#if SYSTEM == SOLARIS || SYSTEM == OS4
  struct mtop tapec;
  struct mtget tapes;
#endif
  
  if (tape->realtape == NO) return;       /* if virtual tape already in sync */
#if SYSTEM == SOLARIS || SYSTEM == OS4
  ioctl(tape->file,MTIOCGET,&tapes);      /* get tape status */
  tapec.mt_count=1;                       /* backspace over over TM */
  tapec.mt_op=MTBSR; 
  ioctl(tape->file,MTIOCTOP,&tapec);
  if (tapes.mt_blkno==0) return;          /* return if at start of file */
  tapec.mt_count=1;                       /* else forward space over last rec */
  tapec.mt_op=MTFSR;
  ioctl(tape->file,MTIOCTOP,&tapec);      /* get tape status */
#endif
#if SYSTEM == UNIXWARE
  ioctl(tape->file,T_SFB,1);
#endif
 }
int tpread(struct deviceinfo *tape,unsigned char *buf,unsigned int len)
 {
  int len_prev, len_this, len_check,len_read;
  unsigned  int test;
  unsigned short int bdw;
  off_t seek_len;
  char length[5];
  char *stopchar;
 
  if (tape->realtape == YES)
   {
#if SYSTEM == SOLARIS || SYSTEM == OS4
    len_read=read(tape->file,buf,len);      /* read from tape */ 
#endif
#if SYSTEM == UNIXWARE
    len_read=read(tape->file,buf,61440);      /* read from tape */ 
#endif
    if (len_read <0)                        /*  read ok ? */
     {
      issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
      issue_ferror_message("Unrecovered read error occured\n");
      tape->fatal=1;
     }
#if SYSTEM == UNIXWARE
    if (len_read > 0) tape->vblkno++;
#endif
    return(len_read);                       /* real tape-real read */
   }
  if (tape->tape_type==FILESYSTEM)
   {
    test=tape->block_len+tape->block_eol;
    if (tape->format[0] != 'F') test+=+4;
    len_read=read(tape->file,buf,test);
    if (len_read>0)
     {
      if (tape->format[0]=='F') /* were done */;
      else if (tape->format[0]=='V')
       {
        memcpy(&bdw,buf,2);
        if (bdw < 8 || bdw>tape->block_len || bdw > len_read)
         {
          if (bdw<8) 
           issue_ferror_message("Block descriptor word length than 8 encountered\n");
          else if (bdw>tape->block_len)
           issue_ferror_message("Block descriptor word length greater than declarded block length encountered\n");
          else
           issue_ferror_message("Block descriptor word length greater than remaining length of file\n");
          tape->fatal=1;
          return(-1);
         }
        if (len_read-bdw == 0) seek_len=0; 
        else
         {
          int len;
          for (len=0;bdw+len<len_read-2; len++)
           if (memcmp(buf+bdw+len,"\0\0",2) == 0) break;
          len=len-2;
          if (len <0)
           {
            issue_ferror_message("Could not find start of next block\n");
            tape->fatal=1;
            return(-1);
           }
          bdw+= len;
          seek_len = bdw-len_read;
          len_read=bdw; 
         }
        lseek(tape->file,seek_len,SEEK_CUR);
       }
      else if (tape->format[0]=='D')
       {
        memcpy(length,buf,4);
        length[4]='\0';
        if (strspn((char*)length,"0123456789") != 4) 
         {
          issue_ferror_message("Could not find start of next block\n");
          tape->fatal=1;
          return (-1);
         }
        bdw = strtol(length,&stopchar,10);                /* numeric form */    
        if (bdw < 8 || bdw>tape->block_len || bdw > len_read ||
            ((tape->format[1]='S' || tape->format[2]=='S') && bdw <9))
         {
          if (bdw<8) 
           issue_ferror_message("Block descriptor word length than 8 encountered\n");
          else if (bdw>tape->block_len)
           issue_ferror_message("Block descriptor word length greater than declarded block length encountered\n");
          else if (bdw <9)
           issue_ferror_message("Block descriptor word length than 9 encountered\n");
          else
           issue_ferror_message("Block descriptor word length greater than remaining length of file\n");
          tape->fatal=1;
          return(-1);
         }
        if (len_read-bdw == 0) seek_len=0; 
        else
         {
          int chr, len, done;
          for (done=len=0;bdw+len<len_read && !done; len++)
           switch (chr=buf[bdw+len])
            {
             case '0': case '1': case '2': case '3': case '4': case '5':
             case '6': case '7': case '8': case '9':
              len--;
              done=1;
             default:
              ;
            }
          if (len == len_read)
           {
            issue_ferror_message("Could not find start of next block\n");
            tape->fatal=1;
            return(-1);
           }
          bdw+= len;
          seek_len = bdw-len_read;
          len_read=bdw; 
         }
        lseek(tape->file,seek_len,SEEK_CUR);
       }
      tape->vblkno++;         /* incr virtual block number */
      if (tape->block_eol > 0)
       {
        len_read-=tape->block_eol;
        if (strcmp((char *)buf+len_read,"\n") != 0)
         issue_warning_message("Expected EOL sequence not found after block\n");
       }
     }
    return(len_read);
   }
  rd_ibg("TPREAD");                         /* read IBG */
  if (len_this == 0)                        /* EOF */  
   {
    lseek(tape->file,-12L,SEEK_CUR);        /* back over TM */
    return(0);                              /* return no data read */
   }
  len_read=read(tape->file,buf,len_this);   /* read a record */
  if (len_read != len_this)                 /* better be this */
   {
    issue_ferror_message("IBG length and length read do not agree\n");
    tape->fatal=1;
    return(-1);
   }
  tape->vblkno++;                           /* incr virtual block number */
  return (len_read);                        /* return length read */
 }
int tpwrite(struct deviceinfo *tape ,const unsigned char *buf,unsigned int len)
 {
  unsigned int len_write, len_check;
  char igb[14];

  if (tape->realtape==YES)
   {
    len_write = write(tape->file,buf,len);   /* real tape-real write */
    if (len_write <0)                        /*  read ok ? */
     {
      issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
      issue_ferror_message("Unrecovered write error occured\n");
      tape->fatal=1;
     }
#if SYSTEM == UNIXWARE
    if (len_write > 0) tape->vblkno++;
#endif
    return(len_write);
   }
  if (len != 0)                                   /* have a recore to write? */
   {
    sprintf(igb,"%4.4X",tape->len_prev);          /* put len prev rec in IBG */
    if (len >65535)                               /* validate current length */
     {
      fprintf(stderr,"Length must be <65535.\n");
      return (-1);
     }
    sprintf(igb+4,"%4.4X",len);                   /* put len curr rec in IBG */
    len_check= len ^ (unsigned)tape->len_prev;    /* create chech word */
    sprintf(igb+8,"%4.4X",len_check);             /* put in IBG */
    write(tape->file,igb,12);                     /* write IBG */
    write(tape->file,buf,len);                    /* write record */
    tape->vblkno++;                              /* increment virtual block # */
    tape->len_prev=len;                           /* this is now prev len */
   }
 }
int tapefsf(struct deviceinfo *tape, int count)
 {
#if SYSTEM == SOLARIS || SYSTEM == OS4
  struct mtget tapes; 
  struct mtop tapec;
#endif
  int len_prev, len_this, len_check,len_read;
  unsigned  int test;
  off_t seek_len;
  char length[5];
  char *stopchar;

  if (tape->realtape == YES)                      /* real tape? */
   {
#if SYSTEM == SOLARIS || SYSTEM == OS4
    tapec.mt_count=count;                         /* yep - set FSF count */
    tapec.mt_op=MTFSF;                            /* set for FSF */
     ioctl(tape->file,MTIOCTOP,&tapec);           /* perform  FSF */
#endif
#if SYSTEM == UNIXWARE
    if (count>0)
     {
      ioctl(tape->file,T_SFF,count);
      tape->vfileno+=count;
     }
    else 
     {
      if (ioctl(tape->file,T_SFB,-count+1) != -1) ioctl(tape->file,T_SFF,1); 
      tape->vfileno+=count;
     }
    tape->vblkno=0;
#endif  
   }
  else if (tape->realtape == NO)                  /* virtual tape? */
   {
    if (count > 0)                          /* forward space 1 or more files? */
     {
      while (count>0)                             /* cycle */
       {
        rd_ibg("TAPEFSF");                        /* read IBG */
        if (len_this==0)                          /* reached EOF? */
         { 
          count -= count;                         /* yep - one down */
          tape->vfileno++;                        /* incr virtual file number */
          tape->vblkno=0;                       /* reset virtial block number */
          continue;                               /* forward space next file */
         } 
        seek_len=len_this;                       /* seek past this block */
        lseek(tape->file,seek_len,SEEK_CUR);
       }
     }
    else if (count == 0)                       /* back space start to SOF? */
     {
      while (count == 0)
       {
        rd_ibg("TAPEFSF");                       /* read IBG */
        if (len_prev==0)                         /* start of file? */
         { 
          lseek(tape->file,-12L,SEEK_CUR);     /* yep backspace over IBG read */
          tape->vblkno=0;                      /* reset virtual block number */
          break;                               /* done */
         } 
        seek_len=-len_prev-24L;               /*backspace ove IBG read, prev */
        lseek(tape->file,seek_len,SEEK_CUR);  /* rec and its IBG */
       }
     }
    else
     {
      while (count <= 0)                        /* backspace 1 or more files? */
       {
        rd_ibg("TAPEFSF");                      /* yep - read IBG */
        if (len_prev==0)                        /* start of file? */
         {
          if (count == 0)                       /* yep - End of BSF'S */
           {
            lseek(tape->file,-12L,SEEK_CUR);    /* yep _ back over IBG */
            tape->vblkno=0;                     /* reset virtual block number */
           }
          else 
           {
            lseek(tape->file,-24L,SEEK_CUR);  /* backspace IBG prev rec + IBG */
            read(tape->file,tape->buffer,12);   /* read prev IBG */
            if (strncmp((char *)tape->buffer,"0000",4) == 0) tape->vblkno=0;
            else tape->vblkno=1000000000; /* 0 if SOF else high number */
            lseek(tape->file,-12L,SEEK_CUR);   /* back up over IBG */
            tape->vfileno--;                 /* decrement virtual file number */
           }
          count ++;                          /* incr count */
          if (tape->vfileno==0) break;       /* quit if SOT */
          continue;                          /* bakspace next file */ 
         }                      
        seek_len = -len_prev  -24L;          /* back space IBG read + prev */
        lseek(tape->file,seek_len,SEEK_CUR); /* rec and its IBG */
       }
     }
   }   
  else
   unknown_tape("TAPEFSF");                     /* unknown device */
 }
int taperew(struct deviceinfo *tape)
 {
#if SYSTEM == SOLARIS || SYSTEM == OS4
  struct mtget tapes; 
  struct mtop tapec;
#endif

  if (tape->realtape == YES)                 /* this a real tape? */ 
   {
#if SYSTEM == SOLARIS || SYSTEM == OS4
    tapec.mt_count=1;                        /* yep rewind tape */
    tapec.mt_op=MTREW;
    ioctl(tape->file,MTIOCTOP,&tapec);
#endif
#if SYSTEM == UNIXWARE
    ioctl(tape->file,T_RWD,1);
    tape->vfileno=tape->vblkno=0;
#endif
   }
  else if (tape->realtape == NO)             /* this a virtual tape? */ 
   {
    lseek(tape->file,0L,SEEK_SET);           /* seek to front of tape */
    tape->vfileno=tape->vblkno=tape->len_prev=0;/* reset blkno fileno len_prev*/
   }
  else
   unknown_tape("TAPEREW"); 
 }
int tapebsr(struct deviceinfo *tape,int count)
 {
#if SYSTEM == SOLARIS || SYSTEM == OS4
  struct mtget tapes; 
  struct mtop tapec;
#endif
  int len_prev, len_this, len_check,len_read;
  unsigned  int test;
  off_t seek_len;
  char length[5];
  char *stopchar;

  if (count > 1) 
   {fprintf(stderr,"BSR for more than 1 record not yet support"); exit(2);}
  if (tape->realtape == YES)             /* real tape? */
   {
#if SYSTEM == SOLARIS || SYSTEM == OS4
    tapec.mt_count=1;               /* yep - set count (currently always one) */
    tapec.mt_op=MTBSR;                   /* set to do BSR */
    ioctl(tape->file,MTIOCTOP,&tapec);   /* do  operation */
#endif
#if SYSTEM == UNIXWARE
    if ( count > tape->vblkno) count=tape->vblkno;
    ioctl(tape->file,T_SBB,count);
    tape->vblkno-=count;
#endif
   }
  else if (tape->realtape == NO)         /* virtual tape? */
   {
    rd_ibg("TAPEBSR");                   /* yep - read IBG */
    seek_len=-12;                        /* back up over IBG just read */
    if (len_prev != 0)                   /* is there a previous record? */
     {
      tape->vblkno--;                    /* yep - decrement virtual block num */
      seek_len = seek_len - len_prev -12; /* back up over prev rec  IBG */
     }
    lseek(tape->file,seek_len,SEEK_CUR);  /* back up */
   }
  else
   unknown_tape("TAPEBSR");                       /* unknown device */ 
 }
int tapestatus(struct deviceinfo *tape, struct tapestats *tapestats)
 {
#if SYSTEM == SOLARIS || SYSTEM == OS4
  struct mtget tapes;
#endif
  int rc;

  if (tape->realtape == YES)                       /* real tape? */
   {
#if SYSTEM == SOLARIS || SYSTEM == OS4
    rc=ioctl(tape->file,MTIOCGET,&tapes);          /* yep get hardware info */
    tapestats->mt_fileno=tapes.mt_fileno;          /* move */      
    tapestats->mt_blkno=tapes.mt_blkno;            /* move */
    return (rc);                                        /* return */
#endif
#if SYSTEM == UNIXWARE
    tapestats->mt_fileno=tape->vfileno;          /* yep - fill in file number */
    tapestats->mt_blkno=tape->vblkno;              /* and block number */
    return (0);                                    /* done */
#endif
   }
  else if (tape->realtape == NO)                   /* virtual tape ? */
   {
    tapestats->mt_fileno=tape->vfileno;          /* yep - fill in file number */
    tapestats->mt_blkno=tape->vblkno;              /* and block number */
    return (0);                                    /* done */
   }
  else
   unknown_tape("TAPESTATUS");                     /* Unknown Device */
  return (0);
 }
int tapenbsf(struct deviceinfo *tape, int count)
 {
  tapefsf(tape,-count);                      /*  this is inverse of FSF */
 }
int tapebsf(struct deviceinfo *tape, int count)
 {
#if SYSTEM == SOLARIS || SYSTEM == OS4
  struct mtget tapes; 
  struct mtop tapec;
#endif
  int len_prev, len_this, len_check,len_read;
  unsigned  int test;
  off_t seek_len;
  char length[5];
  char *stopchar;

  if (tape->realtape == YES)              /* Is this a real tape */
   {
#if SYSTEM == SOLARIS || SYSTEM == OS4
    tapec.mt_count=count;                 /* yep - number of files back space */
    tapec.mt_op=MTBSF;                    /* set to do Back Space file */
    ioctl(tape->file,MTIOCTOP,&tapec);    /* perform operation */
#endif
#if SYSTEM == UNIXWARE
    if (count>= tape->vfileno)
     {
      ioctl(tape->file,T_RWD,1);
      tape->vfileno=tape->vblkno=0;
     }
    else
     {
      ioctl(tape->file,T_SFB,count);
      tape->vfileno-=count;
      tape->vblkno=2000000000;
     }
#endif
   }
  else if (tape->realtape == NO)          /* this a Virtual tape? */
   {
    if (count > 0)                        /* at least 1 to write? */
     {
      while (count>0)                     /* cycle */
       {
        rd_ibg("TAPEBSF");                /* read IBG */
        if (len_prev==0)                  /* at start of File? */
         { 
          count --;                       /* one less to back space */
          tape->vfileno--;                /* decrement virtual file number */
          lseek(tape->file,-24L,SEEK_CUR); /*back up over this IBG and TM */
          read(tape->file,tape->buffer,12);/*get info previous file from IBG */ 
          if (strncmp((char *)tape->buffer,"0000",4) == 0) tape->vblkno=0;
          else tape->vblkno=1000000000; /* virtual block O if SOF else large */
          lseek(tape->file,-12L,SEEK_CUR); /* back up over IBG we just read */
          continue;                      /* do next BSF */
         } 
        seek_len=-len_prev-24L;          /* back up over this IBG prev rec  */
        lseek(tape->file,seek_len,SEEK_CUR); /* the previous rec's IBG */
       }
     }
    else
     {
      fprintf(stderr,"backspacing to non positive integers not supported\n");
      exit(1);                          /* BSF currently must be > 0 */
     }
   }
  else
   unknown_tape("TAPEBSF");              /* Unknown device */
 }
int tapeweof(struct deviceinfo *tape, int count)
 {
#if SYSTEM == SOLARIS || SYSTEM == OS4
  struct mtop tapec;
#endif
  char ibg[14];

  if (tape->realtape == YES)              /* this a real tape? */
   {
#if SYSTEM == SOLARIS || SYSTEM == OS4
    tapec.mt_count=count;              /* yep - number of tape marks to write */
    tapec.mt_op=MTWEOF;                   /* set to write tape marks */
    ioctl(tape->file,MTIOCTOP,&tapec);    /* do actual write */
#endif
#if SYSTEM == UNIXWARE
    ioctl(tape->file,T_WRFILEM,count);
    tape->vfileno+=count;
    tape->vblkno=0;
#endif
   }
  else if (tape->realtape == NO)          /* this a virtual tape? */
   {
    if (count > 0)                        /* more that 0 to write */
     {
      while (count>0)                     /* cycle through */ 
       {
        sprintf(ibg,"%4.4X%4.4X%4.4X",tape->len_prev,0,tape->len_prev);
        write(tape->file,ibg,12);         /* write tape mark */
        tape->len_prev=tape->vblkno=0;    /* prev length block pos now zero */
        count--;                          /* one less tape mark to write */
        tape->vfileno++;                  /* increment virtual tape number */
       }
      sprintf(ibg,"%4.4X%4.4X%4.4X",0,0,0); /* write one more so we can do */
      write(tape->file,ibg,12);           /* position functions */
      lseek(tape->file,-12L,SEEK_CUR);    /* back up over this last one */
     }
    else
     {
      fprintf(stderr,"Writing non positve number of Tape Marks not suppport\n");
      exit(2);   /* we dont support writting less that one tape marks */
     }
   }
  else
   unknown_tape("TAPEWEOF");              /* Unknown device */
 }
int gap_err(char * function,struct deviceinfo *tape)
 {
  sprintf(message_area,"%s error while reading IBG\n",function);
  issue_ferror_message(message_area);
  tape->fatal=1;
  return (-1);
 }
int unknown_tape(char * function)
 {
  fprintf(stderr,"%s unknown tape type encountered",function);
  exit(2);
 }
int getdrivetype(struct deviceinfo *tape)
 {
#if SYSTEM == SOLARIS || SYSTEM == OS4
  struct mtget tapes;
  int n;
 
  n=ioctl(tape->file,MTIOCGET,&tapes);    /* get tape status */
  if (n < 0) 
   {
    fprintf(stderr,"Couldn't get tape status\n");
    exit (2);                            /* operation failed - too bad */
   }
#if SYSTEM == SOLARIS
  if (tapes.mt_type == MT_ISREEL ||      /* determine type of drive */
      tapes.mt_type == MT_ISHP) return(DEV_HALFINCH);
  else if (tapes.mt_type == MT_ISDAT) return(DEV_4MM);
  else if (tapes.mt_type == MT_IS8MM) return(DEV_8MM);
  else if (tapes.mt_type == MT_ISQIC) return(DEV_QIC);
#endif
#if SYSTEM == OS4
  if (tapes.mt_type == MT_ISCPC ||
      tapes.mt_type == MT_ISXY ||
      tapes.mt_type == MT_ISKENNEDY ||
      tapes.mt_type == MT_ISHP) return(DEV_HALFINCH);
  else if (tapes.mt_type == MT_ISAR ||
           tapes.mt_type == MT_ISSYSGEN11 ||
           tapes.mt_type == MT_ISSYSGEN ||
           tapes.mt_type == MT_ISMT02 ||
           tapes.mt_type == MT_ISVIPER1 ||
           tapes.mt_type == MT_ISWANGTEK1) return(DEV_QIC);
  else if (tapes.mt_type == MT_ISEXABYTE ||
           tapes.mt_type == MT_ISEXB8500) return(DEV_8MM);
  else if (tapes.mt_type == MT_ISPYTHON) return(DEV_4MM);
#endif
  else
   {      
    fprintf(stderr,"GETDRIVETYPE Unknown tape drive device %x\n",tapes.mt_type);
    exit(2);                           /* not one we are handling yet */ 
   } 
#endif
#if SYSTEM == UNIXWARE
  struct blklen blklen;

  ioctl(tape->file,T_RDBLKLEN,&blklen);
  ioctl(tape->file,T_WRBLKLEN,&blklen);
  return(DEV_HALFINCH);
#endif
 }

