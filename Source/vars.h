/*
 *  vars.h
 *  lbltp
 *
 *  Created by Mike Alexander on 11/3/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef VARS_H
#define VARS_H

#ifdef INIT_GLOBALS
#define Public
#define INIT(x) x
#else
#define Public extern
#define INIT(x)
#endif

#ifdef INIT_GLOBALS
#include "translate.h"
#else
Public unsigned char ASCEBC[256];
Public unsigned char EBCASC[256];
Public unsigned char ASCMTS[256];
Public unsigned char MTSASC[256]; 
#endif

Public unsigned char magic_id[4] 
#ifdef INIT_GLOBALS
  = {0x0E,0xE5,0x2A,0xFE}
#endif
;
Public unsigned char magic_id2[4] 
#ifdef INIT_GLOBALS
  = {0x0F,0xE2,0xAE,0x5E}
#endif
;

Public int show_warnings INIT(= ON);
Public int infile_recording_mode INIT(= RECORD);
Public int outfile_recording_mode INIT(= STREAM);
Public char message_area[4096];

Public struct deviceinfo tapei, tapeo;


#endif /* VARS_H */
