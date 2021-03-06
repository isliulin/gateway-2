#ifndef EXTERNFUNC_H
#define EXTERNFUNC_H

typedef struct _st_dir {
	char 	path[128];		// 文件名称 包括路径
	void *	data;			// 文件
}ST_DIR;

#define SaveLog(fmt,arg...)   \
	do { \
		char RecDate[50]; \
		FILE *log_fd; \
		GetDate(RecDate,sizeof(RecDate)); \
		log_fd = fopen("log.txt","a+"); \
		fprintf(log_fd,"%s:[%s]",RecDate,__FUNCTION__); \
		fprintf(log_fd,fmt,##arg); \
		fflush(log_fd); \
		fclose(log_fd); \
	} while(0) \
//---------------------------------------------------------------------------
//外部函数
//---------------------------------------------------------------------------
int fileexists(const char *FileName);
char *strupper(char *pdst,const char *pstr,int Size);
void DelayMs(int ms);
char * excuteCmd(char *Cmd,...);
void ErrorLog(int ecode,const char *fmt,...);
char * GetDate(char *cBuf,int Size);
const char * GetSendIP(const char *pSrcIP,const char *pDestIP,const char *pMask);
int jugdeRecIP(const char *pSrcIP,const char *pDestIP,const char *pMask);
unsigned int my_inet_addr(const char *IP);

void SetNetwork(int flag,const char *cIp,const char *cMask,const char *cGateWay,const char *cMac);
void SetNetMac(unsigned char *pImei,char *MAC);
// unsigned long GetMs(void);
unsigned long long GetFlashFreeSpace(void);
unsigned long long GetFlashFreeSpaceKB(void);
unsigned long long GetFlashFreeSpaceMB(void);

int net_detect(char* net_name);
void print_data(char *data,int len);
int GetFilesNum(char *pPathDir,void (*func)(void *));

void WatchDogOpen(void);
void WatchDogFeed(void);
void WatchDogClose(void);

int recoverData(const char *file);
#endif

