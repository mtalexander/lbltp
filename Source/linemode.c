#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>  
#include <limits.h>
#include <time.h>

#include "lbltp.h"
#include "vars.h"
#include "linemode.h"
#include "functions.h"
#include "machine.h"

/* Internal methods */
static int cmdparse(unsigned char *,unsigned char *a[]);
static struct deviceinfo *onwhat(char *);
static void process_cmds(void);
static int closcmd(int ,unsigned char *parsv[]);
static int dispcmd(int ,unsigned char *parsv[]);
static int eovcmd(int ,unsigned char *parsv[]);
static int formcmd(int ,unsigned char *parsv[]);
static int listcmd(int ,unsigned char *parsv[]);
static int lpcmd(int ,unsigned char *parsv[]);
static int opencmd(int ,unsigned char *parsv[]);
static int trancmd(int ,unsigned char *parsv[]);
static int warncmd(int ,unsigned char *parsv[]);
static int copycmd(int parsc,unsigned char *parsv[]);
static int datecmd(int parsc,unsigned char *parsv[]);
static int ditocmd(int parsc,unsigned char *parsv[]);
static int duplcmd(int parsc,unsigned char *parsv[]);
static int exprcmd(int parsc,unsigned char *parsv[]);
static int filecmd(int parsc,unsigned char *parsv[]);
static int initcmd(int parsc,unsigned char *parsv[]);
static int prefcmd(int parsc,unsigned char *parsv[]);
static int posncmd(int parsc,unsigned char *parsv[]);
static int recmcmd(int parsc,unsigned char *parsv[]);
static int termcmd(int parsc,unsigned char *parsv[]);
static int rewcmd(int parsc,unsigned char *parsv[]);

static char UNUSED(copywrite[640])  = "Copyright (c) 1992-1997 Regents of the University of Michigan. All rights reserved. Redistribution and use in source and binary forms are permitted provided that this notice is preserved and that due credit is given to the University of Michigan at Ann Arbor. The name of the University may not be used to endorse or promote products derived from this software without specific prior written permission. This software is provided ``as is'' without express or implied warranty.";
static void unvattn(int);
static char var_name[256];
static char version[10]="1.2";



int main (int argc, char *argv[])
 {
   int  i,j;
   static char *drive_name="\0";
   
   tapeinit();
   for (i=1; i<argc; i++)
    {
     if (argv[i][0]== '-' && argv[i][1]== 'f')
      {
       j=i+1;
       if (j<argc)
        drive_name=argv[j];
       else
        {
         fprintf(stderr,"Expected tape name not found\n");
         exit (1);
        }
      }
     else {fprintf(stderr,"`%s' is not valid\n",argv[i]);exit (1);}
    }
   if (drive_name[0] != '\0')
    if (tpopen(INPUT,(unsigned char *)drive_name,0,0) != 1) exit (2);
   process_cmds();
   
   return 0;
  }


static void process_cmds(void)
 {
   int  i,cmd_len;
   char *rc;
   void (*old_handler)(int);

#if SYSTEM == OS4 || SYSTEM == DARWIN || SYSTEM == MSVC || SYSTEM == CYGWIN
   old_handler = signal(SIGINT,&unvattn);
#else
   old_handler = sigset(SIGINT,&unvattn);
#endif
   if (old_handler == SIG_ERR) exit(1);
   printf("ITD Tape Utility version %s\n",version);
   for (;;)
   {
    unsigned char command[258];
    unsigned char *parsv[40];
    int parsc;

    printf("\nPlease Enter Request\n");
    fflush(NULL);
    rc=fgets((char *)command,256,stdin);
    if (rc==NULL) break; 
    if (command[0] == '$')
     {
      if (system((char *)command+1) == -1)
       issue_error_message(strcat(strcpy(message_area,strerror(errno)),"\n"));
      continue;
     }
    parsc=cmdparse(command,parsv);
    cmd_len=strlen((char *)parsv[0]);
    if (cmd_len == 0) continue;
    for (i=0;i<cmd_len;i++) command[i]=toupper(command[i]);
    if (cmd_len>1 && strncmp((char*)parsv[0],"CLOSE",cmd_len) == 0)
     closcmd(parsc,parsv); 
    else if (strncmp((char *)parsv[0],"COPY",cmd_len) == 0) 
     copycmd(parsc,parsv); 
    else if (cmd_len>1 && strncmp((char *)parsv[0],"DATECHECK",cmd_len) == 0)
     datecmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"DISPLAY",cmd_len) == 0)
     dispcmd(parsc,parsv);
    else if (strcmp((char *)parsv[0],"DITTO") == 0)
     ditocmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"DUPLICATE",cmd_len) == 0)
     duplcmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"EOV",cmd_len) == 0)
     eovcmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"EXPIRE",cmd_len) == 0)
     exprcmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"FILENAME",cmd_len)==0)
     filecmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"FORMAT",cmd_len)== 0) 
     formcmd(parsc,parsv);
    else if (strcmp((char *)parsv[0],"INITIALIZE")== 0) 
     initcmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"LIST",cmd_len) == 0)
     listcmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"LP",cmd_len) == 0)
     lpcmd(parsc,parsv);
    else if (strncmp((char *)parsv[0],"OPEN",cmd_len) == 0) 
     opencmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"POSITION",cmd_len) == 0)
     posncmd(parsc,parsv);
    else if (cmd_len>1 && strncmp((char *)parsv[0],"PREFIX",cmd_len) ==0)
     prefcmd(parsc,parsv);
    else if (cmd_len>2 && strncmp((char *)parsv[0],"RECORDINGMODE",cmd_len) ==0)
     recmcmd(parsc,parsv);
    else if (cmd_len>2 && strncmp((char *)parsv[0],"REWIND",cmd_len) ==0)
     rewcmd(parsc,parsv);
    else if (cmd_len>2 && strncmp((char *)parsv[0],"STOP",cmd_len) == 0) 
     break;
    else if (cmd_len>2 && strncmp((char *)parsv[0],"STRUCTURE",cmd_len) ==0)
     recmcmd(parsc,parsv);
    else if (strcmp((char *)parsv[0],"TERMINATE") == 0)
     termcmd(parsc,parsv);
    else if (strncmp((char *)parsv[0],"TRANSLATE",cmd_len) == 0)
     trancmd(parsc,parsv);
    else if (strncmp((char *)parsv[0],"WARN",cmd_len)== 0) 
     warncmd(parsc,parsv);
    else fprintf(stderr,"Request not recognized - reenter.\n");
   }

#if SYSTEM == OS4 || SYSTEM == DARWIN || SYSTEM == MSVC || SYSTEM == CYGWIN
   signal(SIGINT,old_handler);
#else
   sigset(SIGINT,old_handler);
#endif
  tapeclose();
  exit (1);
 }


static int cmdparse(unsigned char *command,unsigned char *parsv[])
 {
  int i,offset=0;
  size_t str_posn,len;

  for (i=0;i<40;i++)
   {
    str_posn=strspn((char *)&command[offset]," ");
    parsv[i]=&command[str_posn+offset];
    if (parsv[i][0] == '\'' || parsv[i][0] == '"')
     {
      char * endquote = strchr(&((char *)parsv[i])[1], *((char *)parsv[i]));
      if (endquote)
       {
        len = endquote + 1 - (char *)parsv[i];
        *(endquote + 1) = '\0';
       }
      else
       {
        len=strcspn((char *)parsv[i],"\n");
        parsv[i][len]='\0';
       }
     }
    else
     {
      len=strcspn((char *)parsv[i]," \n");
      parsv[i][len]='\0';
     }
    if (len==0)
     {
      if (i>0) i--;
      return (i);
     }
    offset+=str_posn+len+1;
   }
  return (0);
 }


static void unvattn(int signum)
 {
  fprintf(stderr,"\n attention\n"); 
#if SYSTEM == OS4
  sigsetmask(!sigmask(SIGINT));
#endif
  process_cmds();
 }


void append_normal_message(char *message,int where)
 {
  fprintf(stdout,"%s",message);
 } 


void issue_error_message(char *message)
 {
  fprintf(stderr,"%s",message);
 } 


void issue_ferror_message(char *message)
 {
  fprintf(stderr,"%s",message);
  /* Callers expect us to return 
  exit(1);
  */
 } 


void issue_warning_message(char *message)
 {
  fprintf(stderr,"%s",message);
 } 


void postblocks(int num) 
 {
  if (num !=0) fprintf(stderr,"  %d blocks processed\n",num);
 }


void postbytes(double num) 
 {
  if (num !=0) fprintf(stderr,"  %0.0f bytes processed\n",num);
 }


char *getAnotherFile(char *filename)
 {
  char *rc, *ret_name;

  fprintf(stderr," Warning file %s already exists.\n",filename);
  fprintf(stderr," Enter replacement name, hit return to overwrite, or generate \n  an end of file to cancel\n");
  ret_name = (char *)malloc(258);
  rc = fgets(ret_name,256,stdin);
  if (rc != NULL) return(ret_name);
  free(ret_name);
  return (NULL); 
 }


#define val_io(function)                                                       \
  if (tape==NULL)                          /* user specify input or output? */ \
   {                                                                           \
    if (tapei.name != NULL && tapeo.name != NULL)  /* are both in use? */      \
     {                                                                         \
      tape = onwhat(function);            /* yep - get clarifcation */         \
      if (tape==NULL) return (-1);        /* user didn't clarify */            \
     }                                                                         \
    else if (tapei.name != NULL) tape=&tapei; /* input assued if allocated */  \
    else if (tapeo.name != NULL) tape=&tapeo; /* output assumed if allocated */\
    else                                                                       \
     {                                                                         \
      tape = onwhat(function);            /* yep - get clarifcation */         \
      if (tape==NULL) return (-1);        /* user didn't clarify*/             \
     }                                                                         \
   }                                                                           \
  if (tape->name == NULL)                /* specified device in use? */        \
   {                                                                           \
    char *dev;                                                                 \
    if (tape==&tapei) dev="Input";       /* no - tell user */                  \
    else dev="Output";                                                         \
    fprintf(stderr,"%s device is not defined\n",dev);                          \
    return (-1);                         /* and return */                      \
   }


#define get_number(number,value,min)                                           \
   {                                                                           \
    len=strlen(number);                        /* yep get length of number */  \
    if (len>0 && strspn(number,"0123456789") == len)      /* valid number?*/   \
     value=atoi(number);                       /*numeric value */              \
    if ((len==0) || (value<min))               /* legal number? */             \
     {                                                                         \
      fprintf(stderr," `%s' is illegal\n",parsv[i]); /* no tell user */        \
      return (-1);                                                             \
     }                                         /* return */                    \
   }

 
#define get_range(start,end);                                                  \
  {                                                                            \
   char *minus;                                                                \
   minus=strpbrk(&*(equal+1),"-");             /* find range marker */         \
   if (minus != NULL) *minus='\0';             /* break up range */            \
   len=strlen(&*(equal+1));                    /* length of first number */    \
   if (len>0 && strspn(&*(equal+1),"0123456789") == len) /* legal string */    \
    start=atoi(&*(equal+1));                   /* yep get numeric equiv */     \
   else if(strcmp(&*(equal+1),"EOT") == 0)     /* EOT used? - strange */       \
    start=LONG_MAX;                            /* set to large number */       \
   if ((len==0) || (start<1))                  /* valid number? */             \
    {                                                                          \
     fprintf(stderr," `%s' is illegal\n",parsv[i]); /* no - tell user */       \
     return (-1);                                                              \
    }                                                                          \
   if (minus!=NULL)                            /* range specified? */          \
    {                                                                          \
     len=strlen(&*(minus+1));                  /* yep - length second val*/    \
     if (len>0 && strspn(&*(minus+1),"0123456789") == len) /* legal? */        \
      end=atoi(&*(minus+1));                   /* yep ste number equiv */      \
     else if(strcmp(&*(minus+1),"EOT") == 0)   /* EOT used? */                 \
      end=LONG_MAX;                            /* set to large number */       \
     if ((len==0) || (end<1))                  /* valid number? */             \
      {                                                                        \
       fprintf(stderr," `%s' is illegal\n",parsv[i]); /* no tell user */       \
       return (-1);                                                            \
      }                                                                        \
    }                                                                          \
   else end=start;                            /* end and start the same */     \
   if (start>end) {j=end; end=start; start=j;} /* go from low to high */       \
  }


static int closcmd(int parsc,unsigned char *parsv[]) 
 {
  struct deviceinfo *tape;
  int i, j, len;

  tape=NULL;                                       /* no device defined */
  for (i=1; i <= parsc; i++)                       /* cycle though pars */
   {
    len=strlen((char *)parsv[i]);                  /* length of parameter */
    for (j=0; j <= len; j++)var_name[j]=toupper(parsv[i][j]); /* copy and tran*/
    var_name[len]='\0';                            /* terminate parameter */
    if (strcmp((char *)var_name,"INPUT") == 0)     /* close input */
     {tape=&tapei;} 
    else if (strcmp((char *)var_name,"OUTPUT") == 0) /* or output? */
     {tape=&tapeo;} 
    else 
     {
      fprintf(stderr," `%s' is an invalid parameter\n",parsv[i]); /* say what */
      return (-1);
     } 
   }
  val_io("Close");                              /* validate input/output spec */
  return (closetape(tape));                     /* do it */
 }


static int copycmd(int parsc,unsigned char *parsv[])
 {
  struct deviceinfo *idevice, *odevice;
  struct buf_ctl  input_ctl, output_ctl;
  unsigned int j, len;
  int i;
  char *equal, *opar, *cpar; 
 
  memset(&input_ctl, 0, sizeof(input_ctl));
  memset(&output_ctl, 0, sizeof(output_ctl));
  
  idevice=odevice=NULL;  /* input/output devices not defined */
  input_ctl.iofrom=output_ctl.iofrom=-1;
  input_ctl.translate=-1; /* translate mode not set */
  output_ctl.translate=-1; /* translate mode not set */
  input_ctl.path=NULL;  /* no pathname for input given */
  output_ctl.path=NULL; /* no pathname for output given */
  output_ctl.warn=show_warnings;
  input_ctl.linemode=0;           /* don't write out line numbers */
  input_ctl.start_file=input_ctl.end_file=0;   /* copy from current position */
  input_ctl.trtable=NULL;         /* no overriding translate mode */
  input_ctl.format[0]=output_ctl.format[0]='\0';
  input_ctl.notify_=output_ctl.notify_=0;
  input_ctl.blocks_=output_ctl.blocks_=0;
  input_ctl.records_=output_ctl.records_=0;
  input_ctl.blkpfx=output_ctl.blkpfx=0;
  input_ctl.blk_eol=output_ctl.blk_eol=0;
  if (tapei.lp==1 && tapei.tape_type>=FS_UNLABELED) input_ctl.fs=1;
  
  for (i=1;i<=parsc;i++)  /* cycle though parameters */
   {
    len=strlen((char *)parsv[i]);             /* length of parameter */
    for (j=0; j<len; j++) var_name[j]=toupper(parsv[i][j]); /* copy and tran */
    var_name[len]='\0';                      /* terminate parameter */
    equal=strpbrk((char *)var_name,"="); /* find equal sign in par - if any */
    if (equal != NULL) *equal= '\0';     /* break into keyword value pair */ 
    len=strlen((char *)var_name);        /* length of keyword */
    if (equal == NULL)                   /* just a keyword? */
     {
      opar=strpbrk((char *)var_name,"("); /* find opening par - if any */ 
      cpar=strpbrk((char *)var_name,")"); /* find closing par - if any */
      if (len>3 && strncmp((char *)var_name,"TRANSLATE",len)==0)
       input_ctl.translate=ON;            /* mark if TRANSLATE specified */
      else if (len>5 && strncmp((char *)var_name,"NOTRANSLATE",len)==0) 
       input_ctl.translate=OFF;           /* mark if NOTRANSLATE specified */
      else if (strcmp((char *)var_name,"LINE")==0) input_ctl.linemode=1; /* linemode? */
      else if (strcmp((char *)var_name,"WARN")==0) output_ctl.warn=ON; /*Warn on?*/
      else if (strcmp((char *)var_name,"NOWARN")==0) output_ctl.warn=OFF;/*or off*/
      else if (len>3 && strncmp((char *)var_name,"TERMINAL",len)==0) 
        output_ctl.iofrom=2;                  /* copy to terminal */
      else if (len>3 && strncmp((char *)var_name,"SCREEN",len)==0) 
        output_ctl.iofrom=2;                  /* copy to terminal */
      else if (len>3 && strncmp((char *)var_name,"OUTPUT",len)==0)/*use default out tape?*/
       output_ctl.iofrom=1;                    /* set defaut output device */
      else if (opar != NULL && cpar != NULL) /* where parens used */
       {
        if (cpar < opar) goto badpar;    /* yep - parens in valid order? */
        *opar=*cpar='\0';                /* yes make parened value a string */
        len=strlen((char *)var_name);    /* len of keyword */  
        if (strcmp((char *)var_name,"LINE")==0) /* set a line mode? */
         {
          if (strcmp(&*(opar+1),"CHAR")==0) input_ctl.linemode=1; /* yep - char line #? */
          else if (strcmp(&*(opar+1),"CHAR12")==0) input_ctl.linemode=2; /* fixed char */
          else if (strcmp(&*(opar+1),"INT")==0) input_ctl.linemode=3; /* int line "? */
          else if (strcmp(&*(opar+1),"NONE")==0) input_ctl.linemode=0; /* no line #? */
          else goto badpar;                             /* don't recognize */
         } 
        else if (len>3 && strncmp((char *)var_name,"TRANSLATE",len)==0) /* trans? */
         {
          if (strcmp(&*(opar+1),"MTS")==0)
           {input_ctl.trtable=EBCASC; output_ctl.otrtable=ASCEBC;} /* set transtable new */
          else if(strcmp(&*(opar+1),"OLDMTS")==0) 
           {input_ctl.trtable=MTSASC; input_ctl.otrtable=ASCMTS;} /* set transtable pre tday */
          else if(strcmp(&*(opar+1),"IBM")==0) 
           {input_ctl.trtable=EBCASC; input_ctl.otrtable=ASCEBC;}/* set trans table new for now */
          else goto badpar;                    /* don't recognize too bad */
          input_ctl.translate=ON;             /* enable translation */
         } 
        else goto badpar;                      /* don't recognize this */
       } 
      else                                     /* don't recognize this */
       {
       badpar:
        fprintf(stderr," `%s' is an invalid parameter\n",parsv[i]);/*tell user*/
        return (-1);
       }
     }  
    else
     {
      if (strncmp((char *)var_name,"FILES",len)==0) /* file numbers spedified?*/
       {get_range(input_ctl.start_file,input_ctl.end_file)} /* resolve file range */
      else if (strncmp((char *)var_name,"INPUT",len)==0) /* INPUT specified? */
       {
        equal=strpbrk((char *)parsv[i],"="); 
        input_ctl.path=equal+1;                  /* yep pick up input name */
        infile_recording_mode=RECORD;            /* assume record mode */
       }
      else if (len>1 && strncmp((char *)var_name,"RINPUT",len)==0) /* RINPUT? */
       {
        equal=strpbrk((char *)parsv[i],"="); 
        input_ctl.path=equal+1;                     /* yep pick up input name */
        infile_recording_mode=RECORD;                 /* set record mode */
       }
      else if (len>1 && strncmp((char *)var_name,"SINPUT",len)==0) /* SINPUT? */
       {
        equal=strpbrk((char *)parsv[i],"="); 
        input_ctl.path=equal+1;         /* yep pick up input name */
        infile_recording_mode=STREAM;                  /* set stream mode */
       }
      else if (strncmp((char *)var_name,"OUTPUT",len)==0)  /* OUTPUT spec? */
       {
        equal=strpbrk((char *)parsv[i],"="); 
        output_ctl.path=equal+1;          /* yep pick up output name */
       }
      else if (strncmp((char *)var_name,"RECORDS",len)==0) /* copy N records? */
       {get_number(equal+1,input_ctl.records_,1)} /* yep get number of recs */ 
      else if (strncmp((char *)var_name,"BYTES",len)==0) /* copy N records? */
       {get_number(equal+1,input_ctl.records_,1)} /* yep get number of recs */ 
      else if (strncmp((char *)var_name,"BLOCKS",len)==0) /* copy N blocks? */
       {get_number(equal+1,input_ctl.blocks_,1)}           /* yep - get number of blks */
      else if (strncmp((char *)var_name,"NOTIFY",len)==0) /* notify N blocks? */
       {get_number(equal+1,input_ctl.notify_,1)}    /* yep - # blocks/notify */ 
      else if (strncmp((char *)var_name,"FORMAT",len)==0) /*Input format spec?*/
       {
        input_ctl.read_rtn=fmtstring((unsigned char*)&*(equal+1),&input_ctl); /*Yep - set */
        if (input_ctl.read_rtn == NULL)            /* valid format ? */
         {
          fprintf(stderr," `%s' is illegal\n",parsv[i]); /* no - tell user */
          return (-1);
         }
       }
      else if (strncmp((char *)var_name,"PREFIX",len)==0) /*block prefixing?*/
       {
        equal++;                            /* skip over = */
        len=strlen(equal);                     /* length of value */
        if (equal[len-1]=='L')              /* write BDW? */
         {   
          input_ctl.blkpfxl=ON;                          /* yes */
          equal[len-1]='\0';                /* erase L */
          len--; 
         }
        get_number(equal,input_ctl.blkpfx,0); /* get size of blockpfx */
       } 
      else if (strncmp((char *)var_name,"EOL",len)==0) /*End of line spec?*/
       {
        equal++;
        if (strncmp(equal,"YES",len)==0) input_ctl.blk_eol=1;
        else if (strncmp(equal,"NO",len)==0) input_ctl.blk_eol=0;
        else
         {
          fprintf(stderr,"Eol only takes a Yes or No\n");
          return(-1);
         }
       }
      else 
       {
        fprintf(stderr," `%s' is an invalid parameter\n",parsv[i]);
        return (-1);
       }
     }
   }
  return(copyfunction(&input_ctl,&output_ctl,OFF));
 }


static int datecmd(int parsc,unsigned char *parsv[])
 {
  struct deviceinfo *tape;
  int i, j, check, len;
 
  tape=NULL;                                 /* no device defined */
  check=-1;                                  /* date checking opt not defined */
  if (parsc>0)
   {
    for (i=1; i<= parsc; i++)                /* cycle though pars */
     {
      len=strlen((char *)parsv[i]);          /* len of parameter */
      for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /* copy and tran */
      var_name[len]='\0';                    /* terminate */ 
      if (strcmp((char *)var_name,"ON") == 0) check=ON;       /* set on or  */
      else if (strcmp((char *)var_name,"OFF") == 0) check=OFF;/* off */
      else if (strncmp((char *)var_name,"INPUT",len) == 0) tape=&tapei; /* in?*/
      else if (strncmp((char *)var_name,"OUTPUT",len) == 0) tape=&tapeo;/*out?*/
      else                                   /* say what? */
       {
        fprintf(stderr," %s is not a legal parameter",parsv[i]);
        return (-1);
       }
     }
   }
  val_io("Set date ckecking option for");      /* validate input/output spec */ 
  if (check != -1)                             /* option specified */
   tape->datecheck=check;                      /* set value */
  else                                         /* command is a no-op */
   {     
    fprintf(stderr,"This does nothing\n");
   }
  return (0);
 }


static int dispcmd(int parsc,unsigned char *parsv[])
 {
  struct deviceinfo *tape;
  char *equal;
  int rec, length;
  unsigned int j,len;
  int i;
  char *path;
  struct deviceinfo file;
   
  path=NULL;
  rec=0;    
  length=32; 
  tape=NULL;                               /* no tape specified */
  if (parsc>0)                             /* cycle through parameters */
   {
    for (i=1; i<= parsc; i++)
     {
      len=strlen((char *)parsv[i]);        /* len of parameter */
      for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /* copy and trans */      var_name[len]='\0';                  /* terminate */
      equal=strpbrk((char *)var_name,"="); /* find keyword/value */
      if (equal==NULL)                     /* keyword only? */ 
       {
        if (strncmp((char *)var_name,"INPUT",len) == 0) /* if input?*/
         tape=&tapei;                      /* set input tape */ 
        else if (strncmp((char *)var_name,"OUTPUT",len) == 0) /*else if output*/
         tape=&tapeo;                      /* set output tape */
        else goto badpar;                  /* else too bad */
       }
      else
       {
        *equal='\0';                       /* make into keword/value string */
        len=strlen((char *)var_name);      /* length of keyword */
        if (strncmp((char *)var_name,"BLOCKS",len) == 0) /*display any blocks?*/
         {get_number(equal+1,rec,1)}            /* get # of blocks to display */
        else if (strncmp((char *)var_name,"LENGTH",len) == 0) /* amt of blk? */
         {get_number(equal+1,length,1)}         /* get # of bytes to display */
        else if (strncmp((char *)var_name,"INPUT",len)==0) /* INPUT specified? */
         {
          equal=strpbrk((char *)parsv[i],"="); 
          path=equal+1;                  /* yep pick up input name */
          infile_recording_mode=RECORD;            /* assume record mode */
         }
        else if (len>1 && strncmp((char *)var_name,"RINPUT",len)==0) /* RINPUT? */
         {
          equal=strpbrk((char *)parsv[i],"="); 
          path=equal+1;                     /* yep pick up input name */
          infile_recording_mode=RECORD;                 /* set record mode */
         }
        else if (len>1 && strncmp((char *)var_name,"SINPUT",len)==0) /* SINPUT? */
         {
          equal=strpbrk((char *)parsv[i],"="); 
          path=equal+1;         /* yep pick up input name */
          infile_recording_mode=STREAM;                  /* set stream mode */
         }
        else
         {
          badpar:                          /* don't rcogmize this parameter */
          fprintf(stderr," %s is not a legal parameter",parsv[i]);
          return (-1);
         }
       }
     }
   } 
  if (path == NULL) {val_io("Display");}    /* validate input or output spec */
  else
   {
    file.name=path;
    file.tape_type=FILESYSTEM;
    tape=&file;
   }
  return(displayfunction(tape,rec,length,1,1,1));
 }


static int ditocmd(int parsc,unsigned char *parsv[])
 {
  char *tapeowner, *tapevol, *equal;
  unsigned char *tapename;
  int i,j,len;
  int tape_type;
 
  tapeowner="";                        /* default tape owner */
  tapevol=NULL;                        /* no volume given */
  tapename=NULL;                       /* no file/device name given */
  tape_type = DEV_AWSTAPE;             /* Assume AWStape format, not FakeTape */
  for (i=1; i<= parsc; i++)            /* cycle through pars */
   {
    len=strlen((char *)parsv[i]);      /* length of parameter */
    for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /* copy and trans */
    var_name[len]='\0';                /* terminate */
    equal=strpbrk((char *)var_name,"="); /* split into keyword/value */
    if (equal==NULL) 
     {
       /* No equal sign in parameter */
      if (len > 3 && strncmp((char *)var_name, "FAKETAPE", len) == 0)
       {
        tape_type = DEV_FAKETAPE;
       }
      else if (len > 2 && strncmp((char *)var_name, "AWSTAPE", len) == 0)
       {
        tape_type = DEV_AWSTAPE;
       }
      else if (tapename == NULL)
       {
        tapename = parsv[i];
       }
      else
       goto parerr;
      continue;
     }
       
    *equal='\0';                        /* terminate keyword */ 
    equal=strpbrk((char *)parsv[i],"="); /* point to prestart of value */
      parerr:
      fprintf(stderr,"`%s' is illegal\n",parsv[i]);
      return (-1); 
   }
  if (tapename==NULL)                   /* user must specify the file/device */
   {
    fprintf(stderr,"File/Device name must be specified.\n");
    return(-1); 
   }
    /* Specified device must not already be open */
  if (tapeo.name != NULL && strcmp((char *)tapeo.name,(char *)tapename) == 0)
   {
    fprintf(stderr,"Tape is already open for output - close first.\n");
    return (-1); 
   }
  if (tapei.name != NULL && strcmp((char *)tapei.name,(char *)tapename) == 0)
   {
    fprintf(stderr,"Tape is already open for input - close first.\n");
    return (-1); 
   }
  if (tapei.name == NULL)
   {
    fprintf(stderr,"Input tape must be defined.\n");
    return (-1);
   }
  return(dittofunction((char *)tapename, 0, tape_type));
 }


static int duplcmd(int parsc,unsigned char *parsv[])
 {
  struct deviceinfo *idevice, *odevice;
  struct buf_ctl  input_ctl, output_ctl;
  unsigned int j, len;
  int i;
  char *equal; 
 
  memset(&input_ctl, 0, sizeof(input_ctl));
  memset(&output_ctl, 0, sizeof(output_ctl));
  
  idevice=odevice=NULL;  /* input/output devices not defined */
  input_ctl.iofrom=output_ctl.iofrom=-1;
  input_ctl.translate=-1; /* translate mode not set */
  output_ctl.translate=-1; /* translate mode not set */
  input_ctl.path=NULL;  /* no pathname for input given */
  output_ctl.path=NULL; /* no pathname for output given */
  output_ctl.warn=show_warnings;
  input_ctl.linemode=0;           /* don't write out line numbers */
  input_ctl.start_file=input_ctl.end_file=0;   /* copy from current position */
  input_ctl.trtable=NULL;         /* no overriding translate mode */
  input_ctl.format[0]=output_ctl.format[0]='\0';
  input_ctl.notify_=output_ctl.notify_=0;
  input_ctl.blocks_=output_ctl.blocks_=0;
  input_ctl.records_=output_ctl.records_=0;
  if (tapei.lp==1 && tapei.tape_type>=FS_UNLABELED) input_ctl.fs=1;
  
  for (i=1;i<=parsc;i++)  /* cycle though parameters */
   {
    len=strlen((char *)parsv[i]);             /* length of parameter */
    for (j=0; j<len; j++) var_name[j]=toupper(parsv[i][j]); /* copy and tran */
    var_name[len]='\0';                      /* terminate parameter */
    equal=strpbrk((char *)var_name,"="); /* find equal sign in par - if any */
    if (equal != NULL) *equal= '\0';     /* break into keyword value pair */ 
    len=strlen((char *)var_name);        /* length of keyword */
    if (equal == NULL)                   /* just a keyword? */
     {
      if (strcmp((char *)var_name,"WARN")==0) output_ctl.warn=ON; /*Warn on?*/
      else if (strcmp((char *)var_name,"NOWARN")==0) output_ctl.warn=OFF;/*or off*/
      else                                     /* don't recognize this */
       {
        fprintf(stderr," `%s' is an invalid parameter\n",parsv[i]);/*tell user*/
        return (-1);
       }
     }  
    else
     {
      if (strncmp((char *)var_name,"FILES",len)==0) /* file numbers specified?*/
       {get_range(input_ctl.start_file,input_ctl.end_file)} /* resolve file range */
      else if (strncmp((char *)var_name,"NOTIFY",len)==0) /* notify N blocks? */
       {get_number(equal+1,input_ctl.notify_,1)}    /* yep - # blocks/notify */ 
      else 
       {
        fprintf(stderr," `%s' is an invalid parameter\n",parsv[i]);
        return (-1);
       }
       output_ctl.iofrom=1;                    /* set defaut output device */
     }
   }
  if (tapei.name==NULL)
   {
    fprintf(stderr,"Input tape must be defined to use the duplicate function");
    return(-1);
   }
  if (tapeo.name==NULL)
   {
    fprintf(stderr,"Output tape must be defined to use the duplicate function");
    return(-1);
   }
  if (tapei.lp == OFF)
   {
    if (tapeo.lp == ON)
     if (tapeo.tape_type != UNLABELED && tapeo.tape_type != VLO_LABEL)
      {
       fprintf(stderr,"Tape with LP = OFF can only be duplicated to an unlabeled or VLO tape or an lp = OFF tape\n");
       return -1;
      }
   }
  else if (tapei.tape_type == IBM_LABEL)  
   {
    if (tapeo.tape_type == IBM_LABEL || tapeo.tape_type == UNLABELED) ;
    else
     {
      fprintf(stderr,"IBM labeled tape can only be duplicated to an unlabeled tape or an IBM labeled tape\n");
      return -1;
     }
   }
  else if (tapei.tape_type == ANSI_LABEL)
   {
    if (tapeo.tape_type == ANSI_LABEL || tapeo.tape_type == UNLABELED) ;
    else
     {
      fprintf(stderr,"ANSI labeled tape can only be duplicated to an unlabeled tape or an ANSI labeled tape\n");
      return -1;
     }
   }
  else if (tapei.tape_type == TOS_LABEL)
   {
    if (tapeo.tape_type == TOS_LABEL || tapeo.tape_type == UNLABELED) ;
    else
     {
      fprintf(stderr,"TOS labeled tape can only be duplicated to an unlabeled tape or a TOS labeled tape\n");
      return -1;
     }
   }    
  else if (tapei.tape_type == UNLABELED || tapei.tape_type == VLO_LABEL)
   {
    if (tapeo.tape_type != UNLABELED && tapeo.tape_type != VLO_LABEL) 
     {
      fprintf(stderr,"Unlabeled tape can only be duplicated to an unlabeled or VLO tape\n");
      return -1;
     }
   }
  else 
   {
    fprintf(stderr,"Duplication of FS tapes not supported\n");
    return -1;
   }
  if (input_ctl.start_file==0) input_ctl.start_file=tapei.position; /* c curr file */
  output_ctl.iofrom=1;                    /* set defaut output device */
  return(copyfunction(&input_ctl,&output_ctl,ON));
 }


static int eovcmd(int parsc,unsigned char *parsv[])
 {
  struct deviceinfo *tape;
  int i, j, len, eov;
  
  tape=NULL;                                   /* no tape specified yet */
  eov=-1;
  if (parsc>0)
   {
    for (i=1; i<= parsc; i++)                 /* cycle through parameters */
     {
      len=strlen((char *)parsv[i]);           /* length of parameter */
      for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /* copy and trans */
      var_name[len]='\0';                     /* terminate */ 
      if (strcmp((char *)var_name,"INPUT") == 0) tape=&tapei; /* input? */
      else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo; /*output?*/
      else if (strcmp((char *)var_name,"ON") == 0) eov=1; /* EOV trailer */
      else if (strcmp((char *)var_name,"OFF") == 0) eov=0; /* EOF trailer */
      else                                    /* can't deal with this */
       {
        fprintf(stderr,"`%s' is illegal in a time string\n",parsv[i]);
        return (-1);
       } 
     }
   }
  val_io("EOV labels on");                   /* validate input/output spec */
  if (eov==1)                                /* specified to do anything? */ 
   {
    fprintf(stderr,"This command does nothing\n"); /* tell user * - return */
    return (0);
   }
  tape->eov=eov;                             /* set and return */
  return (0);
 }


static int exprcmd(int parsc,unsigned char *parsv[])
 {
  struct deviceinfo *tape;
  unsigned char *month, *day, *year;
  char *rc;
  int len, i, j, reset;

  tape=NULL;                                  /* no tape specified yet */
  reset=OFF;                                  /* don't disable */
  month=day=year=(unsigned char *)"\0";       /* init month day and year */
  if (parsc>0)
   {
    for (i=1; i<= parsc; i++)
     {
      len=strlen((char *)parsv[i]);
      for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]);
      var_name[len]='\0';
      if (strcmp((char *)var_name,"INPUT") == 0) tape=&tapei; /* input? */
      else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo; /*output?*/
      else if (strcmp((char *)var_name,"RESET") == 0) reset=ON;/*reset defaut*/
      else if (month[0] == '\0') month=parsv[i]; /* assume month specification*/
      else if (day[0] == '\0') day=parsv[i];  /* assume day specification */
      else if (year[0] == '\0') year=parsv[i]; /* assume year */
      else                                    /* can't deal with this */
       {
        fprintf(stderr,"`%s' is illegal in a time string\n",parsv[i]);
        return (-1);
       } 
     }
   }
  val_io("Set expiration date on");          /* validate input /output dev */
  if (reset == ON)                           /* reset to default value? */
   {
    memcpy(tape->expiration," 00000",6);     /* yep */
   }
  if (month[0] == '\0')                      /* if no pars command a no-op */
   {
    if (reset == OFF)
     fprintf(stderr,"This does nothing\n");
    return (0);
   }
  sprintf((char *)var_name,"%s %s %s",month,day,year); /*month day year string*/
  rc=expirefunction(tape,var_name);
  if (rc == NULL) return -1;
  return(1);
 }


static int filecmd(int parsc,unsigned char *parsv[])
 {
  struct deviceinfo *tape;
  unsigned char *file_name;
  int i,j,len;
 
  tape=NULL;                              /* no tape specified */
  file_name=NULL;                         /* no file name specified */
  for (i=1; i<= parsc; i++)               /* cycle through pars */
   {
    len=strlen((char *)parsv[i]);         /* length of par */
    for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /* copy and trans */
    var_name[len]='\0';
    if (tape==NULL && strcmp((char *)var_name,"INPUT") == 0) 
     tape=&tapei;                         /* set input tape */
    else if (tape==NULL && strcmp((char *)var_name,"OUTPUT") == 0)
     tape=&tapeo;                         /* set output tape */
    else if (file_name==NULL) file_name=parsv[i]; /* set file name */
    else                                  /* woops - don't know what to do */
     {
      fprintf(stderr," `%s' is illegal\n",parsv[i]);
      return (-1);
     }
   }
  val_io("Set file name on");                  /* validate input/output spec */
  if (file_name==NULL)                         /* any file name given? */
   {
    fprintf(stderr," No file name given default will be used.\n");
    return (-1);
   }
  len=strlen((char *)file_name);               /* length of file name */
  if (file_name[0]=='\'')                      /* name start with a '? */
   {
    if (file_name[len-1]=='\'') file_name[len-1]='\0'; /*then must end with ' */
    else
     {
      fprintf(stderr,"File name starts with a ' and no closing ' found.\n");
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
    fprintf(stderr,"Warning -  file name being truncated to 17 characters");
   }
  file_name[len]='\0';                      /* terminate file name */
  strcpy((char *)tape->file_name,(char *)file_name); /* put in control block */ 
  tape->newfile_name=ON;                    /* mark filename has been set */
  return (0);
 }


static int formcmd(int parsc,unsigned char *parsv[])
 {
  struct deviceinfo *tape;
  struct buf_ctl buf_ctl;
  int i,j,len;
 
  memset(&buf_ctl, 0, sizeof(buf_ctl));
  
  tape=NULL;                               /* no input/output tape specified */
  buf_ctl.read_rtn=NULL;                  /* no format string specified */
  for (i=1; i<= parsc; i++)                /* cycle though pars */
   {
    len=strlen((char *)parsv[i]);          /* length of parameter */
    for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /* copy and trans */
    var_name[len]='\0';                    /* terminate */
    if (strcmp((char *)var_name,"INPUT") == 0) tape=&tapei; /* input spec */
    else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo; /* output */
    else if (buf_ctl.read_rtn != NULL) goto parerr; /* if we have format this illegal */
    else
     { 
      buf_ctl.read_rtn=fmtstring(parsv[i],&buf_ctl); /* process format */
      if (buf_ctl.read_rtn == NULL)                 /* was it legal? */
       {         
        parerr:                             /* no */
        fprintf(stderr,"`%s' is illegal\n",parsv[i]);
        return (-1);
       }
     }
   }
  val_io("Set format for");              /*validate input/ouput specification */
  if (buf_ctl.read_rtn == NULL)        /* if no format  command a no-op */
   {
    fprintf(stderr,"This command does nothing\n");
    return (0);
   }
  return (setformat(tape, &buf_ctl));
 }


static int initcmd(int parsc,unsigned char *parsv[])
 {
  char *tapeowner, *tapevol, *equal;
  unsigned char *tapename;
  int type=0,i,j,len;
  int tape_type;
 
  tapeowner="";                        /* default tape owner */
  tapevol=NULL;                        /* no volume given */
  tapename=NULL;                       /* no file/device name given */
  tape_type = DEV_AWSTAPE;             /* Assume AWStape format, not FakeTape */ 
  for (i=1; i<= parsc; i++)            /* cycle through pars */
   {
    len=strlen((char *)parsv[i]);      /* length of parameter */
    for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /* copy and trans */
    var_name[len]='\0';                /* terminate */
    equal=strpbrk((char *)var_name,"="); /* split into keyword/value */
    if (equal == NULL)
     {
      /* No equal sign in parameter */
      if (len > 3 && strncmp((char *)var_name, "FAKETAPE", len) == 0)
       {
        tape_type = DEV_FAKETAPE;
       }
      else if (len > 2 && strncmp((char *)var_name, "AWSTAPE", len) == 0)
       {
        tape_type = DEV_AWSTAPE;
       }
      else if (tapename == NULL)
       {
        tapename = parsv[i];
       }
      else
       goto parerr;
      continue;
     }
    
    *equal='\0';                        /* terminate keyword */ 
    equal=strpbrk((char *)parsv[i],"="); /* point to prestart of value */
    len=strlen((char *)var_name);
    if (len>2 &&strncmp((char *)var_name,"VOLUME",len) == 0) 
     {
      tapevol=equal+1;                  /* this is the volume specification */
      type=IBM_LABEL;                   /* mark label type */
     }
    else if (len>5 && strncmp((char *)var_name,"IBMVOLUME",len) == 0) 
     {
      tapevol=equal+1;                  /* set volume name specification */
      type=IBM_LABEL;                   /* mark label type */
     }
    else if (len>5 && strncmp((char *)var_name,"VLOVOLUME",len) == 0)
     {
      tapevol=equal+1;
      type=VLO_LABEL;
     }
    else if (len>6 && strncmp((char *)var_name,"ANSIVOLUME",len) == 0) 
     {
      tapevol=equal+1;                  /* set volume name specification */
      type=ANSI_LABEL;                  /* mark label type */
     }
    else if (len>5 && strncmp((char *)var_name,"TOSVOLUME",len) == 0) 
     {
      tapevol=equal+1;                  /* set volume name specification */
      type=TOS_LABEL;                   /* mark label type */
     }
    else if(strcmp((char *)var_name,"OWNER")==0) 
     tapeowner=equal+1;                 /* set owner name */
    else                                /* don't know this parameter */
     {
      parerr:
      fprintf(stderr,"`%s' is illegal\n",parsv[i]);
      return (-1); 
     }
   }
  if (tapename==NULL)                   /* user must specify the file/device */
   {
    fprintf(stderr,"File/Device name must be specified.\n");
    return(-1); 
   }
    /* Specified device must not already be open */
  if (tapeo.name != NULL && strcmp((char *)tapeo.name,(char *)tapename) == 0)
   {
    fprintf(stderr,"Tape is already open for output - close first.\n");
    return (-1); 
   }
  if (tapei.name != NULL && strcmp((char *)tapei.name,(char *)tapename) == 0)
   {
    fprintf(stderr,"Tape is already open for input - close first.\n");
    return (-1); 
   }
  if (tapevol==NULL)                    /* volume parameter must be specified */
   {
    fprintf(stderr,"Tape volume must be specified.\n");
    return (-1); 
   }
  len=strlen(tapevol);                  /* length of tape volume */
  if (tapevol[0]=='\'')                 /* if it start with ' */
   {
    if (tapevol[len-1]=='\'') tapevol[len-1]='\0'; /* it must end with ' */
    else
     {
      fprintf(stderr,"Ending ' in tape volume specification messing\n");
      return (-1); 
     }
    tapevol++;                         /* skip over ' */
    len=strlen(tapevol);               /* recalculate the length */
   }
  else                                 /* if not primed translate volume */
   for (i=0; i<len; i++) tapevol[i]=toupper(tapevol[i]); /* to upper case */
  if (strcmp(tapevol,"UNLABELED")==0 || /* asking for an unlabeled tape? */
      strcmp(tapevol,"UNLABELLED")==0)
   type=tapeo.label=UNLABELED;        /*mark as unlabeled tape */
  else if (len == 0 || len>6)       /* validate the length of the volume name */
   {
    fprintf(stderr,"Tape volume must be 1-6 characters in length.\n");
    return (-1); 
   }
  else  
   { 
    strcpy((char *)tapeo.volume,(char *)tapevol); /* save volume name */
    len=strlen(tapeowner);          /* set length of owner name */
    if (tapeowner[0]=='\'')         /* if it starts with a ' */
     {
      if (tapeowner[len-1]=='\'') tapeowner[len-1]='\0'; /* must end with ' */
      else
       {
        fprintf(stderr,"Ending ' in tape volume specification messing\n");
        return (-1); 
       }
      tapeowner++;                  /* skip leading ' */
      len=strlen(tapeowner);        /* recalculate the length */
     }
    else                            /* if owner name didn't start with ' */
     for (i=0; i<len; i++) tapeowner[i]=toupper(tapeowner[i]);/*trans to upper*/
    if (len>14) tapeowner[14]='\0'; /* max length of owner name ansi label */
    strcpy(tapeo.owner,tapeowner);
    if (type==IBM_LABEL || type==TOS_LABEL || type==VLO_LABEL) /* IBM or TOS labeled? */
     tapeo.owner[10]='\0';          /* max length ibm label */
    tapeo.label=type;               /* set label type */
   }
  /* If this is a real tape this value will be reset in tpopen, but if it's
     a simulated tape then we need to tell tpopen what type of simulated
     tape to create. */
  tapeo.drive_type = tape_type;
  tpopen(OUTPUT,tapename,0,0);      /* open and init tape */
  tapeo.label=OFF;                  /* disarm labelling */
  return (0);
 }


static int listcmd(int parsc,unsigned char *parsv[])
 {
  struct deviceinfo *tape;   
  unsigned int len, j;
  int i;
  int document_sw;
  int start_file, end_file;
  unsigned char docsw;
  int date_sw;
  int notify;
  
  tape=NULL;                               /* no tape specified yet */
  docsw='\0';                              /* don't list fs documentation */
  notify=date_sw=0;
  start_file=1;                            /* default starting file */
  end_file=LONG_MAX;                       /* default ending file */
  if (parsc>0)   
   {
    for (i=1; i<= parsc; i++)              /* cycle through command */
     {
      char *equal;

      len=strlen((char *)parsv[i]);        /* len of parameter */ 
      for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /*translate to UC */
      var_name[len]='\0';                  /* terminate */
      equal=strpbrk((char *)var_name,"=");
      if (len>2 && strncmp((char *)var_name,"DOCUMENTATION",len) == 0)
       docsw='\1';                         /* list FS documentation */
      else if (len>2 && strncmp((char *)var_name,"DATES",len) == 0)
       date_sw=1;
      else if (len>0 && strncmp((char *)var_name,"INPUT",len) == 0) 
       tape=&tapei;                        /* list input tape */
      else if (len>0 && strncmp((char *)var_name,"OUTPUT",len) == 0)
       tape=&tapeo;                        /* list output tape */
      else if(equal!=NULL)
       {
        *equal='\0';
        len=strlen((char *)var_name);
        if (len>0 && strncmp((char *)var_name,"FILES",len)==0) /*file range?*/
         {get_range(start_file,end_file)}                 /* get file range */
        else if (strncmp((char *)var_name,"NOTIFY",len)==0) /* notify N blocks? */
         {get_number(equal+1,notify,1)}    /* yep - # blocks/notify */ 
        else 
         {fprintf(stderr,"`%s' is illegal\n",parsv[i]); return(-1);} /* say what? */
       } 
      else 
       {fprintf(stderr,"`%s' is illegal\n",parsv[i]); return(-1);} /* say what? */
     }
   }
  val_io("List");                        /* validate tape specification */
  document_sw=docsw;
  return (listfunction(tape,start_file,end_file,document_sw,date_sw,notify));
 }


static int lpcmd(int parsc,unsigned char *parsv[])
 {
  int i, j, lp, len;
  struct deviceinfo *tape;

  tape=NULL;                                /* no tape specified */
  lp=-1;                                    /* no label processing to set */
  for (i=1; i<= parsc; i++)                 /* cycle though pars */
   {
    len=strlen((char *)parsv[i]);           /* length of par */
    for (j=0; j <= len; j++) var_name[j]=toupper(parsv[i][j]); /*tran to upper*/
    var_name[len]='\0';                     /* terminate */
    if (strcmp((char *)var_name,"ON") == 0) lp=1; /* enable label processing */
    else if (strcmp((char *)var_name,"OFF") == 0) lp=0; /* disable label proc */
    else if (strcmp((char *)var_name,"INPUT") == 0) tape=&tapei; /* set input */
    else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo; /* output */
    else {fprintf(stderr,"`%s' is illegal\n",parsv[i]); return(-1);} /* oops */
   }
  val_io("Set label processing for");      /*validate input/output spec */
  if (lp==-1)
   {
    fprintf(stderr,"This command does nothing");
    return (0);
   }
  if (tape->lp == lp) return (0);          /* already set right */
  return (lpfunction(tape,lp));
 }

    
static int opencmd(int parsc,unsigned char *parsv[]) 
 {
  struct deviceinfo *tape;
  int i, j, len,rc, open_type,is_vlo,not_fs;
  unsigned char *device_name;

  tape=NULL;                                    /* no tape spedified */
  device_name=NULL;                             /* no tape/file specified */
  is_vlo=0;                                     /* Not a VLO tape */
  not_fs=0;
  for (i=1; i <= parsc; i++)                    /* cycle through pars */
   {
    len=strlen((char *)parsv[i]);               /* length of parameter */
    for (j=0; j <= len; j++) var_name[j]=toupper(parsv[i][j]); /*copy and tran*/
    var_name[len]='\0';                         /* terminate */
    if (strcmp((char *)var_name,"VLO") == 0)    /* VLO tape? */
     {
      is_vlo = 1;
     }
    else if (len > 4 && strncmp((char *)var_name,"NOTFSTAPE",len) == 0)
     {
      not_fs = 1;
     }
    else 
    {
      if (tape==NULL)                        /* input or output not spec? */
      {
        if (strcmp((char *)var_name,"INPUT") == 0) tape=&tapei; /* set input */
        else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo;/*set output*/ 
        if (tape != NULL) continue;               /* don't use as device name */
      }
      if (device_name==NULL) 
       {
        device_name=parsv[i];                     /* set file/device name */
        if ((device_name[0] == '\'' || device_name[0] == '"') &&
            device_name[strlen((char *)device_name)-1] == device_name[0])
         {
          device_name[strlen((char *)device_name)-1] = '\0';
          device_name += 1;
         }
       }
      else                                        /* oops */
      {
        fprintf(stderr,"open `%s' or `%s'?\n",device_name,parsv[i]);
        return (-1);
      } 
    }
   }
  if (device_name==NULL)             /* file or device name must be specified */
   {
    fprintf(stderr,"File/Device name must be specified.\n");
    return (-1);
   }
  if (tape==NULL)                    /* input or output must be specified */ 
   tape=onwhat("Open");
  if (tape==NULL) return (-1);       /* oops still dont have a spec */ 
  open_type=INPUT;
  if (tape==&tapeo) open_type=OUTPUT;
  tape->drive_type = 0;              /* Don't know the type if simulated tape */
  rc = tpopen(open_type,device_name,is_vlo,not_fs); /* open device */
  if (rc < 1)                        /* already open? */
   {
    if (rc == 0) fprintf(stderr,"`%s' is already open\n",device_name);
    return (-1);
   }
  return (0);
 }


static int prefcmd(int parsc,unsigned char *parsv[]) 
 {
  struct deviceinfo *tape;
  struct buf_ctl buf_ctl;
  unsigned int j, len;
  int i;
  char *equal;
  int blkpfx,blkpfxl,blkpfxs;

  memset(&buf_ctl, 0, sizeof(buf_ctl));
  
  tape=NULL;                                   /* no tape specified */
  blkpfx=blkpfxl=blkpfxs=-1;                   /* no pars */
  for (i=1; i <= parsc; i++)                   /* cycle though pars */
   {
    len=strlen((char *)parsv[i]);              /* length of par */
    for (j=0; j <= len; j++) var_name[j]=toupper(parsv[i][j]);/*copy and trans*/
    var_name[len]='\0';                        /* terminate */
    equal=strpbrk((char *)var_name,"=");       /* break into keyword/value */
    if (equal == NULL)                         /* kwyword only? */
     {
      if (strcmp((char *)var_name,"INPUT") == 0) tape=&tapei; /* set input */
      else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo;/* output */
      else if (strcmp((char *)var_name,"RESET") == 0) blkpfxs=OFF;/* reset */
      else goto parerr;                        /* oops */
     }
    else
     {
      *equal='\0';                             /* terminate keyword */
      len=strlen((char *)var_name);            /* length of keyword */
      equal++;                                 /* point to start of value */
      if (len>2 && strncmp((char *)var_name,"LENGTH",len) == 0)
       {
        len=strlen(equal);                     /* length of value */
        if (equal[len-1]=='L')              /* write BDW? */
         {   
          blkpfxl=ON;                          /* yes */
          equal[len-1]='\0';                /* erase L */
          len--; 
         }
        get_number(equal,blkpfx,0);            /* get size of blockpfx */
       }
      else                                     /* don't recognise */
       {
        parerr:
        fprintf(stderr,"`%s' is illegal\n",parsv[i]);
        return (-1);
       } 
     }
   }
  val_io("Set block prefix on");
  if (blkpfx == -1 && blkpfxs == -1) /* if no par this command is no-op */
   {
    fprintf(stderr,"This command does nothing.\n");
    return (0);
   }
  if (blkpfxs==OFF) tape->blkpfxs=OFF; /* no blockpfx set */
  if (blkpfx != -1)                    /* have block prefix? */
   {
    return(blkpfxfunction(tape,&buf_ctl,blkpfx,blkpfxl));
   }
  return 0;
 }


static int posncmd(int parsc,unsigned char *parsv[]) 
 {
  struct tapestats tapes;
  struct deviceinfo *tape;
  unsigned int j, len;
  int i;
  int position=0;

  tape=NULL;                                 /* no control block yet */
  if (parsc == 0) {fprintf(stderr,"This does nothing\n"); return(0);} /*easy eh?*/
  for (i=1; i <= parsc; i++) 
   {
    len=strlen((char *)parsv[i]);            /* length of variable */
    for (j=0; j <= len; j++) var_name[j]=toupper(parsv[i][j]); /* copy to UC */
    var_name[len]='\0';                      /* and terminate */
    if (strcmp((char *)var_name,"INPUT") == 0)
     tape=&tapei;                          /* if input tape set control block */
    else if (strcmp((char *)var_name,"OUTPUT") == 0)
     tape=&tapeo;                         /* if output tape set control block */
    else if (strcmp((char *)var_name,"EOT") ==0)
     position=LONG_MAX;                /* treat end of tape as a large number */
    else if (len>0 && strspn((char *)var_name,"0123456789") == len)
     {get_number((char *)var_name,position,1)} /*get requested logical file  #*/
    else
     {
      fprintf(stderr," `%s' is illegal\n",parsv[i]); /* this */
      return(-1);                              /* done */
     }
   }
  val_io("Position");                   /* get input or output specification */
  if (posn(tape,position) ==1)        /* position the tape and see if at LEOT */
   {                                         
    fprintf(stderr,"Warning Logical End of Tape reached while positioning.\n"); 
    tapestatus(tape,&tapes);         /* tell use if at LEOT - get status*/
    if (tapes.mt_blkno != 0)         /* at start of a file? */
     {
      tapebsf(tape,1);               /* no - sync the tape */
      tapefsf(tape,1);
     }
   }  
  return(0);
 }


static int recmcmd(int parsc,unsigned char *parsv[])
 {
  int len, i, j;
  struct deviceinfo *tape;
  int data_mode;

  tape=NULL;                               /* no tape specification */
  data_mode=-1;                            /* no data mode */
  for (i=1; i<= parsc; i++)                /* cycle though pars */
   {
    len=strlen((char *)parsv[i]);          /* length of parameter */
    for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /* copy and trans */
    var_name[len]='\0';                    /* terminate */
    if (strcmp((char *)var_name,"INPUT") == 0) tape=&tapei; /* set input */
    else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo; /*set output*/
    else if (strcmp((char *)var_name,"RECORD")==0) data_mode=RECORD; /*record */
    else if (strcmp((char *)var_name,"STREAM")==0) data_mode=STREAM; /*stream*/
    else                                   /* dont recognize this */
     {
      fprintf(stderr,"`%s' is illegal\n",parsv[i]);
      return (-1);
     }
   }
  val_io("Set structure of");  /* validate input/output specification */
  if (data_mode !=  -1)
   tape->data_mode=data_mode;             /* Set recording mode */
  else
   fprintf(stderr,"This command does nothing\n");
  return (0);
 }


static int rewcmd(int parsc,unsigned char *parsv[])
 {
  int i, j, lp, len;
  struct deviceinfo *tape;

  tape=NULL;                                /* no tape specified */
  lp=-1;                                    /* no label processing to set */
  for (i=1; i<= parsc; i++)                 /* cycle though pars */
   {
    len=strlen((char *)parsv[i]);           /* length of par */
    for (j=0; j <= len; j++) var_name[j]=toupper(parsv[i][j]); /*tran to upper*/
    var_name[len]='\0';                     /* terminate */
    if (strcmp((char *)var_name,"INPUT") == 0) tape=&tapei; /* set input */
    else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo; /* output */
    else {fprintf(stderr,"`%s' is illegal\n",parsv[i]); return(-1);} /* oops */
   }
  val_io("Rewind");      /*validate input/output spec */
  return (rewindfunction(tape));
 }


static int termcmd(int parsc,unsigned char *parsv[])
 {
  unsigned int j, len;
  int i, position, lp;
  struct deviceinfo *tape;

  tape=NULL;                                /* no tape specified */
  lp=-1;                                    /* no label processing to set */
  position=-1;
  for (i=1; i<= parsc; i++)                 /* cycle though pars */
   {
    len=strlen((char *)parsv[i]);           /* length of par */
    for (j=0; j <= len; j++) var_name[j]=toupper(parsv[i][j]); /*tran to upper*/
    var_name[len]='\0';                     /* terminate */
    if (strcmp((char *)var_name,"INPUT") == 0)
     {
      fprintf(stderr,"Terminate only works on the output tape\n");
      return (-1);
     }
    else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo; /* output */
    else if (strcmp((char *)var_name,"EOT") ==0)
     position=LONG_MAX;                /* treat end of tape as a large number */
    else if (len>0 && strspn((char *)var_name,"0123456789") == len)
     {get_number((char *)var_name,position,1)} /*get requested logical file  #*/
    else {fprintf(stderr,"`%s' is illegal\n",parsv[i]); return(-1);} /* oops */
   }
  val_io("Terminate");      /*validate input/output spec */
  if (tape== &tapei)
   {
    fprintf(stderr,"Terminate only works on the output tape\n");
    return (-1);
   }
  if (position==-1)
   {
    fprintf(stderr,"File number must be specified\n");
    return (-1);
   }
  return (terminatefunction(tape,position));
 }


static int trancmd(int parsc,unsigned char *parsv[])
 {
  int len, i, j;
  struct deviceinfo *tape;
  int translate;
  unsigned char *itrtable,*otrtable;

  tape=NULL;                            /* no tape specified */
  itrtable=otrtable=NULL;               /* no translate tables */
  translate=-1;                         /* nothing enabled or disabled */
  for (i=1; i<= parsc; i++)             /* cycle though pars */
   {
    len=strlen((char *)parsv[i]);       /* length of parameter */
    for (j=0;j<len;j++)var_name[j]=toupper(parsv[i][j]); /* copy and trans */
    var_name[len]='\0';                 /* terminate */
    if (strcmp((char *)var_name,"INPUT") == 0) tape=&tapei; /* set input */
    else if (strcmp((char *)var_name,"OUTPUT") == 0) tape=&tapeo; /*set output*/
    else if (strcmp((char *)var_name,"ON")==0) translate=ON; /* enable trans */
    else if (strcmp((char *)var_name,"OFF")==0) translate=OFF; /* disable */
    else if (strcmp((char *)var_name,"MTS")==0) 
     {
      itrtable=EBCASC;                  /* set tranlate tables */
      otrtable=ASCEBC;
     }
    else if (strcmp((char *)var_name,"OLDMTS")==0) 
     {
      itrtable=MTSASC;                  /* set translate tables */
      otrtable=ASCMTS;
     }
    else if (strcmp((char *)var_name,"IBM")==0) 
     {
      itrtable=EBCASC;                  /* set translate tables */
      otrtable=ASCEBC;
     } 
    else                                     /* don't recognize */
     {
      fprintf(stderr,"`%s' is illegal\n",parsv[i]);
      return (-1);
     }
   }
  val_io("Set translate options for");     /* validate input/output specs */
  if (itrtable != NULL)                            /* Set translate tables */ 
   {
    tape->translate=ON;                    /* enable translation */
    tape->trtable=itrtable;
    if (tape == &tapeo) tape->otrtable=otrtable;
    else tape->otrtable=NULL;
   }
  else if (tape->trtable==NULL && translate == ON) /* need to set default */
   {
    tape->trtable=EBCASC;                  /* yep */
    if (tape == &tapeo) tape->otrtable=ASCEBC;
    else tape->otrtable=NULL;
   }
  if (translate !=  -1) tape->translate=translate; /* Set translate to REQ */
  else fprintf(stderr,"This command does nothing\n");
  return (0);
 }


static int warncmd(int parsc,unsigned char *parsv[])
 {
  int len, i, j;

  if (parsc == 0)                            /* command a no-op */
   {
    fprintf(stderr,"this command does nothing\n"); /* yep */
    return(0);
   }
  for (i=1; i<= parsc; i++)                  /* cycle though pars */
   {
    len=strlen((char *)parsv[i]);            /* length of parameter */
    for (j=0;j<len;j++) var_name[j]=toupper(parsv[i][j]); /* copy and trans */
    var_name[len]='\0';                     /* terminate */
    if (strcmp((char *)var_name,"OFF") == 0) show_warnings=OFF; /* disable warnings */
    else if (strcmp((char *)var_name,"ON") == 0) show_warnings=ON; /* enable warnings */
    else {fprintf(stderr,"`%s' is illegal\n",parsv[i]);return(-1);} /* oops */
   }
  return(0);
 }


static struct deviceinfo *onwhat(char *start_of_msg)
 {
  char * rc;
  int j,len;
  for (;;)                                      /* cycle */
   {
    fprintf(stderr,"%s input or output tape?\n",start_of_msg); /* ask user */
    rc=fgets((char *)var_name,256,stdin);       /* get response */
    if (rc==NULL) break;                        /* no response */
    len=strcspn((char *)var_name," \n");       /* translate response to upper */
    for (j=0;j<len;j++) var_name[j]=toupper(var_name[j]);
    var_name[j]='\0';
    if (strcmp((char *)var_name,"INPUT") == 0) return(&tapei);/*analyze response*/
    else if (strcmp((char *)var_name,"OUTPUT") == 0) return(&tapeo);
   }
  fprintf(stderr,"Command ignored.\n");       /* no response - Ignore command */
  return(NULL);                                 /* return no response */
 }


void yield_(void)
 {
 }
