#include "lbltp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "vars.h"
#include "fsrtns.h"
#include "linemode.h"
#include "machine.h"
#include "functions.h"

static int checksum(struct deviceinfo *tape, int len, int header);

unsigned char *mts_fsread(struct deviceinfo *taper, struct buf_ctl *buf_ctl) 
  { 
   segment_hdr data_segment;
   static unsigned char blanks[18] = { 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
   0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40};
   
   /* Look for padding added to short records by the MTS tape routines.
      Records less than 18 bytes long were padded to 18 with EBCDIC blanks. */
   if (taper->rem_len > 0 && 
       taper->offset + taper->rem_len <= 18 &&
       memcmp(&taper->buffer[taper->offset], blanks, taper->rem_len) == 0)
    /* It's padding, skip it */
    taper->rem_len = 0;
     
   if (taper->rem_len<=0)                    /* data to process?  */
    {
     taper->rem_len= tpread(taper,taper->buffer,32768*2); /* no read next blk */
     if (taper->rem_len== 0) {tpsync(taper); return (NULL);} /* eof? */
     if (taper->rem_len< 0)                  /* tape err? */
      return(tape_err(taper,"Read","MTS/FS",taper->blocks,errno));
     /* If this is the first block in the file, initialize some things */
     if (buf_ctl->blocks == 0)
      {
        buf_ctl->last_line_num = 0x80000000;
      }
     taper->offset = 0;                      /* point to start of block */
     if (memcmp(&taper->fsheader->header_id, &magic_id, 4) == 0)
      {
       if (!checksum(taper, taper->rem_len, 0))
        {
          issue_error_message("checksum failure for data block\n");
          return NULL;
        }
       taper->offset = 12;                   /* unless we have check sum area */
       taper->rem_len-=12;                   /* adjust rem length */
      }
     taper->blocks++;                        /* incr real block count */
     buf_ctl->blocks++;                      /* incr rel block count */
    }
   memcpy(&data_segment, &taper->buffer[taper->offset], sizeof(data_segment));
   data_segment.len_and_flags = etohs(data_segment.len_and_flags);
   buf_ctl->seg_len = data_segment.line_len;
   buf_ctl->line_num = etohl(data_segment.line_number);
   buf_ctl->bufaddr=taper->buffer + taper->offset + sizeof(data_segment);      /* point to segment */
   buf_ctl->eor = data_segment.more_data == 0; /* set EOR if last segment */
   buf_ctl->sor = data_segment.first_segment;
   taper->offset+=buf_ctl->seg_len+sizeof(data_segment); /* offset of next set */ 
   taper->rem_len-=(buf_ctl->seg_len+sizeof(data_segment)); /* update remaining len */
   if (taper->rem_len<0)
    {issue_error_message("ran off end of blk\n"); return NULL;}
   if (buf_ctl->sor ? buf_ctl->line_num <= buf_ctl->last_line_num :
         buf_ctl->line_num != buf_ctl->last_line_num)
    {
      issue_error_message("line numbers out of order\n"); 
      return NULL;
    }
   buf_ctl->last_line_num = buf_ctl->line_num;
   return (buf_ctl->bufaddr);                /* return segment */
  } 
  
int mts_fswrite(struct deviceinfo *device, const unsigned char *buf,unsigned int len)
 {
  fprintf(stderr,"Writting of FS tapes is not supported.");
  exit (1);
 }
 
extern int getfsname(struct deviceinfo *tape)
 {
  struct tapestats tapes;
  int len=0,i,date;
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

  tape->offset=tape->rem_len=0;              /* init these fields */
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
    tape->fsheader=(struct fs_header *)malloc(len);              /* where new header goes */
    memcpy(tape->fsheader,tape->buffer,len); /* allocate new header */
    tape->blocks=0;                    /* Header is block zero in the checksum */
    if (memcmp(&tape->fsheader->header_id, &magic_id, 4) == 0 &&
        !checksum(tape, len, 1))
      {
        issue_error_message("checksum failure for header\n");
      }
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
    version = etohs(tape->fsheader->version);  /* set FS version no */
    tape->version=version;
    i=tape->fsheader->docsw;        /* have a DOC field? */
    if (i == 255 ) 
     {
      tape->doc_len = len - ((sizeof *tape->fsheader) - 4096); /* length of doc field */
     }
    else tape->doc_len=0;
   }
  return (0);
 }

int checksum(struct deviceinfo *tape, int len, int header)
 {
  fs_checksum *cksum_info;
  unsigned int save_checksum, checksum;
  int padlen, i;
  
  if (header)
   {
    struct fs_header *hdr;
    hdr = (struct fs_header *) tape->buffer;
    cksum_info = (fs_checksum *) hdr->doc;
   }
  else
   {
    cksum_info = (fs_checksum *) tape->buffer;
   }
  
  if (etohs(cksum_info->file_number) != tape->position)
   return 0;
   
  if (etohs(cksum_info->block_number) != tape->blocks)
   return 0;
   
  if (etohs(cksum_info->block_length) != len)
   return 0;
  
  /* Save the checksum and clear it */
  save_checksum = cksum_info->checksum;
  cksum_info->checksum = 0;
  /* Pad the record to a multiple of 4 bytes in length.  This will add 4
     zeros if the length is a multiple of 4, but it's not worth worrying
     about that. */
  padlen = 4 - len % 4;
  while (padlen > 0)
   tape->buffer[len - 1 + padlen--] = '\0';
  checksum = 0;
  for (i = 0; i < (len + 3) / 4; i++)
   checksum += etohl(((unsigned int *)tape->buffer)[i]);
  /* Put the checksum back and check the result */
  cksum_info->checksum = save_checksum;
  if (checksum != etohl(save_checksum))
   return 0;
   
  /* All ok. */
  return 1;
 }
 
