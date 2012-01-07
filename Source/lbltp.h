#ifndef _LBLTP_H_
#define _LBLTP_H_

#define TDATE 88053
#define NON_IND 0
#define IND 1
#define OFF 0
#define ON 1
#define NO 0 
#define YES 1
#define INPUT 0
#define OUTPUT 1
#define STREAM 0
#define RECORD 1
#define IBM_LABEL 1
#define ANSI_LABEL 2
#define TOS_LABEL 3
#define VLO_LABEL 4
#define UNLABELED 5
#define FS_UNLABELED 128
#define FS_IBMLABEL 129
#define FS_VLOLABEL 130
#define FILESYSTEM 256

/* Values for drive_type.  The first few are real tapes */
#define DEV_8MM 1
#define DEV_4MM 2
#define DEV_QIC 4
#define DEV_HALFINCH 8
/* The next two are fake tapes */
#define DEV_FAKETAPE 16
#define DEV_AWSTAPE 32

#define SOLARIS 1
#define UNIXWARE 2 
#define OS4 3
#define DARWIN 4
#define MSVC 5
#define CYGWIN 6
#if defined(_MSC_VER)
#define SYSTEM MSVC
#define BYTE_SWAPPED
#elif defined(__MWERKS__) || defined(__APPLE__)
#define SYSTEM DARWIN
#if __LITTLE_ENDIAN__
#define BYTE_SWAPPED
#endif
#elif defined(__MINGW32__) || defined(__CYGWIN__)
#define SYSTEM CYGWIN
#define BYTE_SWAPPED
#elif defined(LINUX)
#define SYSTEM LINUX
#define BYTE_SWAPPED
#else
#error "What system is it?"
#endif

/* Macros to swap shorts and ints if necessary and do nothing if not.
   Don't use the normal ntohs and ntohl names since they may already
   be defined on some paltforms (such as DARWIN). */
#ifdef BYTE_SWAPPED
#define etohs(s) swap_short(s)
#define etohl(l) swap_long(l)
#else
#define etohs(s) s
#define etohl(l) l
#endif

/* Define a macro that can be used to tell gcc that a variable, etc., is unused */
#ifdef UNUSED 
#elif defined(__GNUC__) 
# define UNUSED(x) x __attribute__((unused)) 
#elif defined(__LCLINT__) 
# define UNUSED(x) /*@unused@*/ x 
#else 
# define UNUSED(x) x 
#endif

#if SYSTEM != MSVC && SYSTEM != CYGWIN
#define O_BINARY 0
#endif

typedef unsigned short HWORD;
typedef unsigned char BYTE;

struct deviceinfo;
struct buf_ctl;

typedef unsigned char *(*READ_RTN)(struct deviceinfo *,struct buf_ctl *);
typedef int (*WRITE_RTN)(struct deviceinfo *device, const unsigned char *buf, unsigned int size);

struct buf_ctl
 {
  unsigned int  blocks;
  unsigned int  blocks_;
  unsigned int  records_;
  int  notify_;
  int  translate;
  char *path;
  char *formatSpec;
  int  iofrom, start_file, end_file;
  unsigned char *bufaddr;
  unsigned char *trtable;
  unsigned char *otrtable;
  int  linemode;
  int  eor;
  int  sor;
  int  seg_len;
  int  fs;
  int  warn;
  int  line_num;
  int  last_line_num;
  READ_RTN read_rtn;
  int  blk_len;
  int  blk_eol;
  int  max_blk;
  int  rec_len;
  int  blkpfx;
  int  blkpfxl;
  char format[8];
  unsigned char copy_type;
  char fmchar[1];
 };
 
struct tapestats
 {
  int mt_fileno;
  int mt_blkno;
 };

struct fs_header
 {
  int header_id;
  short filenum;
  short version;
  unsigned char file_name[32];
  unsigned char filetype;
  unsigned char devicetype;
  short maxreclen;
  short filesize;
  unsigned char date[15];
  unsigned char time[8];
  unsigned char dsn[17];
  char blockinfo[17];
  unsigned char docsw;
  unsigned char doc[4096];
 };

#if defined(__GNUC__)
#elif defined(_MSC_VER)
#pragma pack(push, 1)
#elif defined(__MWERKS__)
#pragma options align=packed
#else
#error "How do we align this"
#endif 

typedef struct data_segment_header
 {
  union {
   short len_and_flags;
   struct {
#ifdef BYTE_SWAPPED
    unsigned short line_len:12;
    unsigned short bit_3:1;
    unsigned short bit_2:1;
    unsigned short first_segment:1;
    unsigned short more_data:1;   
#else
    unsigned short more_data:1;
    unsigned short first_segment:1;
    unsigned short bit_2:1;
    unsigned short bit_3:1;
    unsigned short line_len:12;
#endif
   };
  };
   int line_number
#if defined(__GNUC__)
  __attribute__ ((packed))
#endif 
  ;
 } segment_hdr;
   
#if defined(__GNUC__)
#elif defined(_MSC_VER)
#pragma pack(pop)
#elif defined(__MWERKS__)
#pragma options align=reset
#endif 

/* *FS checksum info */
typedef struct fs_checksum_struct
 {
  unsigned short file_number;     /* First FS file on tape is file 1 */
  unsigned short block_number;    /* Header is block 0 */
  unsigned int checksum;         /* Logical sum of entire block */
  unsigned short block_length;    /* Length including checksum area */
  unsigned short reserved;
 } fs_checksum;
 
struct deviceinfo
 {
  char *name;                      /* first entry - device or file name */
  int realtape;                    /* Virtual tape or a physical tape? */
  int file;                        /* As returned from open */
  FILE *unixfile;                  /* As returned from Fopen */
  int fatal;                       /* Fatal error Marker */
  int vfileno,vblkno,len_prev; /* Virtual file - file #, block #, len prev blk */
  int tape_type;                   /* tape structure indicator */
  int drive_type;                  /* type of virtual or physical drive */
  int need_trailer;
  void (*old_handler)(int);
  unsigned char volume[7];
  char owner[15];
  char fmchar[1];
  char op[1];
  char expiration[6];             /* Expiration date to be used for new files */
  int create_date;                /* Creation date of current file */
  int cur_expiration;             /* expiration date of current file */
  int label;                      /* if On, Label tape on Open */
  int  (*rd_hdr_rtn)(struct deviceinfo *tape,int posn);  /* Routine to read tape header files */
  int  (*rd_tlr_rtn)(struct deviceinfo *tape,int posn);  /* Routine to read tape trailer files */
  WRITE_RTN (*wrt_hdr_rtn)(struct deviceinfo *tape);  /* Routine to write tape Header files */
  int  (*wrt_tlr_rtn)(struct deviceinfo *tape);          /* Routine to write tape trailer files */
  unsigned char *(*read_rtn)(struct deviceinfo *taper, struct buf_ctl *buf_ctl); /* Routine to read data */
  WRITE_RTN write_rtn;            /* Routine to wrire data */
  unsigned char file_name[33];    /* Name of current file */
  unsigned char format[8];        /* file format spelled out */
  int blocking;                   /* blocking enabled or disabled  flag */
  int block_len,rec_len,arec_len,seg_len;
  int block_eol;
  int blkpfx;
  int blkpfxl; 
  int blkpfxs;
  int span;
  int translate;
  int data_mode;
  int blocks;
  int offset;
  int eor;
  int eov;
  int rem_len;
  int position;
  int lp;
  int datecheck;
  int leot;
  int cancel;
  int newfile_name;
  unsigned char *trtable;
  unsigned char *otrtable;
  int doc_len; 
  short int version;
  char eob_chk;
  unsigned char *seg_start;
  unsigned char buffer[32768*2];
  unsigned char vol1[80];
  unsigned char hdr1[80];
  unsigned char hdr2[80];
  struct fs_header *fsheader;
 };

/*-------------------------------------------------------------------*/
/* Structure definition for AWSTAPE block header                     */
/*-------------------------------------------------------------------*/
typedef struct _AWSTAPE_BLKHDR {
        HWORD   curblkl;            /* Length of this block          */
        HWORD   prvblkl;            /* Length of previous block      */
        BYTE    flags1;             /* Flags byte 1                  */
        BYTE    flags2;             /* Flags byte 2                  */
    } AWSTAPE_BLKHDR;

/* Definitions for flags1 */
#define AWSTAPE_FLAG1_NEWREC   0x80 /* Start of record               */
#define AWSTAPE_FLAG1_TAPEMARK 0x40 /* Tape mark                     */
#define AWSTAPE_FLAG1_ENDREC   0x20 /* End of record                 */

/* Definitions for flags2 */
#define AWSTAPE_FLAG2_COMPR    0x80 /* Data compressed with zlib     */ 
                                    /* version 1.1.14 (Bus-Tech      */
                                    /* extension to aws format).     */
                                    /* In this case, curblkl contains*/
                                    /* post-compression size;        */
                                    /* original blksize can only be  */
                                    /* determined by decompressing.  */
/*-------------------------------------------------------------------*/


#endif /* _LBLTP_H_ */
