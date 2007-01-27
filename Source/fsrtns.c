#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lbltp.h"
#include "lblextrn.h"

unsigned char *mts_fsread(struct deviceinfo *taper, struct buf_ctl *buf_ctl) 
  { 
   int debug, n, len;
   int header_len, in_len, out_len;
   size_t this_line_len;
   char line_num_c[30];
   int line_index, data_index,line_number;
   typedef struct data_segment_header
    {
     unsigned int more_data:1;
     unsigned int first_segment:1;
     unsigned int bit_2:1;
     unsigned int bit_3:1;
     unsigned int line_len:12;
     char line_number[4];
     unsigned char data[4096];
    } segment_hdr;
   segment_hdr *data_segment, segment;
   union seg_descriptor
    {
     unsigned short int seg_len;
     unsigned char seg_len_char[2];
    } seglen;
   static unsigned char filename[33];
   char  *rc;
   int i, j, m;
   
   if (taper->rem_len==0)                    /* data to process?  */
    {
     taper->rem_len= tpread(taper,taper->buffer,32768*2); /* no read next blk */
     if (taper->rem_len== 0) {tpsync(taper); return (NULL);} /* eof? */
     if (taper->rem_len< 0)                  /* tape err? */
      return(tape_err(taper,"Read","MTS/FS",taper->blocks,errno));
     taper->blocks++;                        /* incr real block count */
     buf_ctl->blocks++;                      /* incr rel block count */
     taper->offset = 0;                      /* point to start of block */
     if (memcmp(&taper->fsheader->header_id, &magic_id, 4) == 0)
      {
       taper->offset = 12;                   /* unless we have check sum area */
       taper->rem_len-=12;                   /* adjust rem length */
      }
    }
   data_segment = (segment_hdr*)&taper->buffer[taper->offset]; /*point to seg */
   memcpy(&seglen.seg_len,data_segment,2);  /* length of segment */
   seglen.seg_len_char[0] &=  0x0f;         /* clear info bits from len */
   INV_SHORT_INT(seglen.seg_len);           /* invert length if necessary */
   buf_ctl->seg_len=seglen.seg_len;          /* length of this segment */
   memcpy(&seglen.seg_len,data_segment,1);   /* info on segment */
   seglen.seg_len_char[1] = seglen.seg_len_char[0] & 0x80; /* more data bit */
   seglen.seg_len_char[0] &=  0x40;         /* first segment bit */
   memcpy(&buf_ctl->line_num,&data_segment->line_number,4); /*get line number */
   INV_LONG_INT(buf_ctl->line_num);        /* invert line number if necessary */
   buf_ctl->bufaddr=data_segment->data;      /* point to segment */
   buf_ctl->eor=0;                           /* set EOR if last segment */
   if (seglen.seg_len_char[1]==0) buf_ctl->eor=1;
   buf_ctl->sor=0;
   if (seglen.seg_len_char[0] != 0) buf_ctl->sor=1; /*set SOR if first segment*/
   taper->offset+=buf_ctl->seg_len+6;        /* offset of next set */ 
   taper->rem_len-=(buf_ctl->seg_len+6);     /* update remaining len */
   if (taper->rem_len<0)
    {issue_error_message("ran off end of blk\n"); return NULL;}
   return (buf_ctl->bufaddr);                /* return segment */
  } 
int mts_fswrite(struct deviceinfo *device,char *buf,unsigned int len)
 {
  fprintf(stderr,"Writting of FS tapes is not supported.");
  exit (1);
 }
extern int getfsname(struct deviceinfo *tape)
 {
  struct tapestats tapes;
  int len,i,date;
  char date_c[7];
  static char month[12][5] = {{"JAN."},
                                    {"FEB."},
                                    {"MAR."},
                                    {"APR."},
                                    {"MAY "},
                                    {"JUN."},
                                    {"JUL."},
                                    {"AUG."},
                                    {"SEP."},
                                    {"OCT."},
                                    {"NOV."},
                                    {"DEC."}};
  static int month_n[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

  tape->offset=tape->rem_len=tape->blocks=0; /* init these fields */
  tapestatus(tape,&tapes);                   /* get info on tape */
  if (tapes.mt_blkno==0)                     /* at start of data? */
   { 
    len=tpread(tape,tape->buffer,32768*2);   /* yep read fs header */
    if (len==0)                              /* EOF? */
     {
      tpsync(tape);                          /* yep sync tape */
      if (tape->fsheader !=NULL) free(tape->fsheader); /* free old header */
      tape->fsheader=NULL;                   /* say no header */
      tape->file_name[0]='\0';               /* no file name */
      tape->trtable=EBCASC;                  /* default trtables */
      if (tape==&tapeo) tape->otrtable=ASCEBC;
      return (1);                            /* return */
     }
    if (tape->fsheader !=NULL) free(tape->fsheader); /* free old hdr if nec */
    tape->fsheader=malloc(len);              /* where new header goes */
    memcpy(tape->fsheader,tape->buffer,len); /* allocate new header */
    for (i=0;i<32;i++)                 /* copy and translate fs file name */
     tape->fsheader->file_name[i]=EBCASC[tape->fsheader->file_name[i]];
    for (i=0;i<39;i++)                 /* copy and translate fs create date */
     tape->fsheader->date[i]=EBCASC[tape->fsheader->date[i]]; 
    tape->blocks=1;                    /* count the fs block header read */
   }
  if (tape->fsheader == NULL)          /* do we have a FS file header? */
   {
    tape->file_name[0]='\0';           /* no - no file name */
    tape->doc_len=0;                   /* no doc */
    tape->version=0;                   /* no version */
    tape->trtable=EBCASC;              /*defaut trranslate tables */
    tape->create_date=0;
    if (tape==&tapeo) tape->otrtable=ASCEBC;
   }
  else 
   {
    unsigned short int version;

    memcpy(tape->file_name,tape->fsheader->file_name,32); /* copy filename */
    tape->file_name[32]='\0';
    date_c[4]='\0';                    /* get julidan version of date */
    date=atoi(strncpy((char *)date_c,(char *)tape->fsheader->date+9,4));
    if (date % 400 == 0 || (date % 4 == 0 && date % 400 != 0 )) month_n[1]=29;
    else month_n[1]=28;
    date = (date-1900)*1000;  
    for (i=0; i<12; i++)
     if (strncmp((char *)tape->fsheader->date,(char *)month[i],3)==0) break;
     else date+=month_n[i];
    date_c[2]='\0';
    date+=atoi(strncpy((char *)date_c,(char *)tape->fsheader->date+5,2));
    if (tape->tape_type==FS_UNLABELED || tape->tape_type==FS_VLOLABEL)
     tape->create_date=date;
    if (date<TDATE) 
     {
      tape->trtable=MTSASC;   /* set translate tape base on date */
      if (tape==&tapeo) tape->otrtable=ASCMTS;
     }
    else
     {
      tape->trtable=EBCASC;
      if (tape==&tapeo) tape->otrtable=ASCEBC;
     }
    memcpy(&version,&tape->fsheader->version,2); /* set FS version no */
    INV_SHORT_INT(version);                 /* invert if necessary */
    tape->version=version;
    i=tape->fsheader->docsw;        /* have a DOC field? */
    if (i == 255 ) 
     {
      tape->doc_len=(sizeof *tape->fsheader) - 4096; /* length of doc field */
     }
    else tape->doc_len=0;
   }
  return (0);
 }
