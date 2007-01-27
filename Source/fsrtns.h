/*
 *  fsrtns.h
 *  lbltp
 *
 *  Created by Mike Alexander on 11/2/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _FSRTNS_H_
#define _FSRTNS_H_

unsigned char *mts_fsread(struct deviceinfo *taper, struct buf_ctl *buf_ctl);
int mts_fswrite(struct deviceinfo *device,const unsigned char *buf,unsigned int len);
extern int getfsname(struct deviceinfo *tape);

#endif /* _FSRTNS_H_ */
