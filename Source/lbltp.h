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
#define UNLABELED 4
#define FS_UNLABELED 128
#define FS_IBMLABEL 129
#define FS_VLOLABEL 130
#define FILESYSTEM 256
#define DEV_8MM 1
#define DEV_4MM 2
#define DEV_QIC 4
#define DEV_HALFINCH 8
#define SOLARIS 1
#define UNIXWARE 2 
#define OS4 3
#define SYSTEM SOLARIS
#define INV_LONG_INT(org_num)                                                  \
   if (machine_arch ==IND)                   /* need to invert? */             \
    {                                                                          \
     unsigned long int temp;                 /* yep */                         \
     union long_int                                                            \
      {                                                                        \
       unsigned long int num;                                                  \
       unsigned char num_c[4];                                                 \
      } n;                                                                     \
                                                                               \
     temp = 0;                                                                 \
     n.num = org_num;                                                          \
     temp = n.num_c[0];                                                        \
     temp = temp <<8;                                                          \
     temp += n.num_c[1];                                                       \
     temp = temp <<8;                                                          \
     temp += n.num_c[2];                                                       \
     temp = temp <<8;                                                          \
     temp += n.num_c[3];                                                       \
     org_num=temp;                                                             \
    } 
#define INV_SHORT_INT(org_num)                                                 \
   if (machine_arch ==IND)                   /* need to invert? */             \
    {                                                                          \
     unsigned short int temp;                /* yep */                         \
     union short_int                                                           \
      {                                                                        \
       unsigned short int num;                                                 \
       unsigned char num_c[2];                                                 \
      } n;                                                                     \
                                                                               \
     n.num = org_num;                                                          \
     temp = n.num_c[0];                                                        \
     temp = temp <<8;                                                          \
     temp += n.num_c[1];                                                       \
     org_num=temp;                                                             \
    } 

static unsigned char magic_id[4] = {0x0E,0xE5,0x2A,0xFE};
static unsigned char magic_id2[4] = {0x0F,0xE2,0xAE,0x5E};

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
  unsigned char *(*read_rtn)();
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
typedef int (*WRITE_rtn)();
struct deviceinfo
 {
  char *name;                      /* first entry - device or file name */
  int realtape;                    /* Virtual tape or a physical tape? */
  int file;                        /* As returned from open */
  FILE *unixfile;                  /* As returned from Fopen */
  int fatal;                       /* Fatal error Marker */
  int vfileno,vblkno,len_prev; /* Vitual file - file #, block #, len prev blk */
  int tape_type;                   /* tape structure indicator */
  int drive_type;                  /* type of physical drive */
  int need_trailer;
  void (*old_handler)();
  unsigned char volume[7];
  char owner[15];
  char fmchar[1];
  char op[1];
  char expiration[6];             /* Expiration date to be used for new files */
  int create_date;                /* Creation date of current file */
  int cur_expiration;             /* expiration date of current file */
  int label;                      /* if On, Label tape on Open */
  int  (*rd_hdr_rtn)();           /* Routine to read tape header files */
  int  (*rd_tlr_rtn)();           /* Routine to read tape trailer files */
  WRITE_rtn (*wrt_hdr_rtn)();     /* Routine to write tape Header files */
  int  (*wrt_tlr_rtn)();          /* Routine to write tape trailer files */
  unsigned char *(*read_rtn)();   /* Routine to read data */
  WRITE_rtn (*write_rtn)();       /* Routine to wrire data */
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
   } *fsheader;
 } tapei, tapeo;
typedef unsigned char *(*READ_RTN)(struct deviceinfo *,struct buf_ctl *);
