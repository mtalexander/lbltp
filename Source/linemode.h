/*
 *  linemode.h
 *  lbltp
 *
 *  Created by Mike Alexander on 11/2/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _LINEMODE_H_
#define _LINEMODE_H_

void append_normal_message(char *message,int where);
void issue_error_message(char *message);
void issue_ferror_message(char *message);
void issue_warning_message(char *message);
void postblocks(int num);
void postbytes(double num);
char *getAnotherFile(char *filename);
void yield_(void);

#endif /* _LINEMODE_H_ */
