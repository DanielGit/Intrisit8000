#ifndef __UCFSDEF__
#define __UCFSDEF__

#include "jzfs_api.h"

#define FILE   JZFS_FILE
#define fopen  jzfs_Open
#define fclose jzfs_Close
#define fread  jzfs_Read
#define fwrite jzfs_Write
#define fseek  jzfs_Seek
#define ftell  jzfs_Tell
#define ferror  jzfs_Error

#undef fgets
//char *fgets(char *line,int size,FILE *fp);
#undef fprintf
#define fprintf(x,y,c...) ({printf("%s %d",__FILE__,__LINE__); printf(y,##c);})
#undef stderr
#define stderr 1
#undef stdout
#define stdout 2
#undef vfprintf
#define vfprintf fprintf
//int fprintf(FILE * stream, const char * format,...);
#define EOF 0 
#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0
#undef mkdir
#define mkdir(x,y) jzfs_Mkdir(x)
#undef rmdir
#define rmdir jzfs_RmDir 
#if 0

#undef open
#define open(x,y,d) ({printf("%s %d",__FILE__,__LINE__); printf("open no surppost\n");})
#undef close
#define close(x) ({printf("%s %d",__FILE__,__LINE__); printf("close no surppost\n");})

#undef read 
#define read(x,y,z) ({printf("%s %d",__FILE__,__LINE__); printf("read no surppost\n");})

#undef lseek
#define lseek(x,y,z) ({printf("%s %d",__FILE__,__LINE__); printf("lseek no surppost\n");})

#undef sync
#define sync(x) ({printf("%s %d",__FILE__,__LINE__); printf("sync no surppost\n");})

// open
FILE *  BUFF_Open ( const char *pFileName, const char *pMode );
// close
void   BUFF_Close( FILE * PORT  );


static inline int uc_fopen( const char * pathname, int flags,mode_t mode)
{
	//printf("uc_fopen\n");
	return   (int)BUFF_Open( pathname, "rb" );
	//return (int)jzfs_Open(pathname,"rb");
}
static inline ssize_t uc_fread(int fd,void * buf ,size_t count)
{
	return (ssize_t)BUFF_Read( buf, 1,count,(FILE *)fd );
	// return (ssize_t)jzfs_Read(buf,1,count,(FILE *)fd);
} 

static inline int uc_fsync(void)
{
	return 1;
} 

static inline ssize_t uc_fwrite (int fd,const void * buf,size_t count)
{
	return 0;
} 
static inline off_t uc_fseek (int fd, off_t offset, int whence)
{
	printf("errot");
}
static inline int uc_fclose (int fd)
{
  BUFF_Close( (FILE *)fd );
	return 1;
}
static inline int UCFS_FClose(FILE *fp)
{
	jzfs_Close(fp);
	return 1;
}
#endif
/*
int uc_fopen( const char * pathname, int flags,mode_t mode);
ssize_t uc_fread(int fd,void * buf ,size_t count); 
int uc_fsync(void); 

ssize_t uc_fwrite (int fd,const void * buf,size_t count); 
off_t uc_flseek (int fd, off_t offset, int whence);
int uc_fclose (int fd);
*/

#endif //__UCFSDEF__
