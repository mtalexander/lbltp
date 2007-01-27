/*
 *  functions.h
 *  lbltp
 *
 *  Created by Mike Alexander on 11/2/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

struct tm;

void myattn(int signum);
int tpopen(int open_type,unsigned char * device_name,int is_VLO_tape,int not_fs);
int tapeinit(void);
struct deviceinfo *get_tape_ptr(int type);
int closetape(struct deviceinfo *tape);
int tapeclose(void);
int setfilename(char *file_name,struct deviceinfo * tape);
int listfunction(struct deviceinfo *tape, int start_file, int end_file, 
                 int docsw ,int datesw, int notify);
long displayfunction(struct deviceinfo *tape,long blocks, long length,
                     long hex, long ebcd, long ascii);
int dittofunction(char *tapename,long notify);
int setformat(struct deviceinfo *tape, struct buf_ctl *buf_ctl);
int blkpfxfunction(struct deviceinfo *tape,struct buf_ctl *buf_ctl, int blkpfx, int blkpfxl);
int lpfunction(struct deviceinfo *tape,int lp);
int rewindfunction (struct deviceinfo *tape);
int terminatefunction (struct deviceinfo *tape,int fileNumber);
int juliantime(int time,struct tm *tm);
int getTranslateTable(struct deviceinfo *tape);
int setTranslateTable(struct deviceinfo *tape, int table);
char *getparameter(char *,int);
READ_RTN fmtstring(unsigned char *,struct buf_ctl *);
int posn(struct deviceinfo *, int);
int wrt_tlr(struct deviceinfo *);
extern unsigned char *tape_err(struct deviceinfo *tape, char *type,char *type2,
                               int block, int error);
int copyfunction(struct buf_ctl *in_buf_ctl,struct buf_ctl *out_buf_ctl,
                 int duplicate);
char *expirefunction(struct deviceinfo *tape, char *date);

unsigned short swap_short(unsigned short s);
unsigned int swap_long(unsigned int l);

#endif /* _FUNCTIONS_H_ */
