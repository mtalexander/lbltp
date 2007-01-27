/*
 *  machine.h
 *  lbltp
 *
 *  Created by Mike Alexander on 11/2/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MACHINE_H_
#define _MACHINE_H_

/* Function prototypes */
int read_ibg(struct deviceinfo *tape,
             unsigned int *p_len_prev,
             unsigned int *p_len_this);
void skip_ibg(struct deviceinfo *tape, int count);
int tpsync(struct deviceinfo *tape);
int tpread(struct deviceinfo *tape,unsigned char *buf,unsigned int len);
int tpwrite(struct deviceinfo *tape ,const unsigned char *buf,unsigned int len);
int tapefsf(struct deviceinfo *tape, int count);
int taperew(struct deviceinfo *tape);
int tapebsr(struct deviceinfo *tape,int count);
int tapestatus(struct deviceinfo *tape, struct tapestats *tapestats);
int tapenbsf(struct deviceinfo *tape, int count);
int tapebsf(struct deviceinfo *tape, int count);
int tapeweof(struct deviceinfo *tape, int count);
int gap_err(char * function,struct deviceinfo *tape);
int unknown_tape(char * function);
int getdrivetype(struct deviceinfo *tape);

#endif /* _MACHINE_H_ */
