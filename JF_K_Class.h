#ifndef JF_K_CLASS
#define JF_K_CLASS


#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include "Config.h"
#include "CLog.h"
#include "JF_Define.h"

class JF_DownloadFile_K;
class JF_PlayBack_K;
class JF_VoiceCom_K;
class JF_RealPlay_K;

#define Error_k() do\
{\
        fprintf(stderr, "K-Error-MSG::<%s> <%s> <%d> -- Error_k Code: %d\n",\
                __FILE__,__FUNCTION__,__LINE__,(CLIENT_GetLastError()&(0x7fffffff)));\
}while(0)

#define error_k(com) do\
{\
        fprintf(stderr, "Error-MSG::<%s> <%s> <%d> -- Error_k of: %s\n",\
        __FILE__,__FUNCTION__,__LINE__,com);\
}while(0)
BOOL SADPdevice_k(DEVICE_IP_SEARCH_INFO* pIpSearchInfo, fSearchDevicesCB cbSearchDevices, LDWORD dwUserData, char* szLocalIp, DWORD dwWaitTime);
BOOL StopSADPdevice_k();
//-----------------|-----------------------------|------------------------------
//-----------------|          JF_SDK_K 类        |------------------------------
//-----------------|-----------------------------|------------------------------
class JF_SDK_K final
{
public:
	JF_SDK_K(fDisConnect_k cbDisConnect, LDWORD dwUser=0);
	virtual ~JF_SDK_K();
    //设置断线重连的回调函数
    virtual void SetReconnect(fHaveReConnect_k cbReconnect=NULL, LDWORD dwUser=0);
    //设备登录
    virtual LLONG  JF_Login_k(char *pchDVRIP,                  //设备IP
                              WORD wDVRPort,                   //设备端口
                              char *pchUserName,               //用户名
                              char *pchPasswd,                 //用户密码
                              EM_LOGIN_SPAC_CAP_TYPE emSpecCap,//设备支持的能力
                              void *pCapParm,                  //emSpecCap的补充参数
                              NET_DEVICEINFO_Ex* pstruDevInfo,//设备信息
                              int *err=NULL);                 //返回登录错误码
    //设备登出
    virtual DWORD  JF_Logout_k(LLONG loginID);
    //设备在线状态检测
    virtual bool   JF_RemoteControl_k(LLONG loginID);
    //获取错误码
    virtual int JF_GetLastError_k();
    //批量获取NVR上IPC通道
    virtual bool JF_GetNewDevConfig_k(LLONG lLoginID,char* dwCommand,int lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,int* lpBytesReturned,int waittime);
    //批量设置NVR上IPC通道
    virtual bool JF_SetNewDevConfig_k(LLONG lLoginID,char* dwCommand,int lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,int waittime);
    //批量更换修改NVR上IPC通道
    //virtual bool JF_MatrixGetCameras_k(LLONG lLoginID, const DH_IN_MATRIX_GET_CAMERAS* pInParam, DH_OUT_MATRIX_GET_CAMERAS* pOutParam, int nWaitTime );
    //批量删除NVR上IPC通道
    virtual bool JF_MatrixSetCameras_k(LLONG lLoginID, const DH_IN_MATRIX_SET_CAMERAS* pInParam, DH_OUT_MATRIX_SET_CAMERAS* pOutParam, int nWaitTime );
    //获取存储上所有的通道信息
    virtual bool JF_QueryDevInfo_k(LLONG lLoginID ,int nQueryType,void *pInBuf , void *pOutBuf,void *pReserved,int nWaitTime);
    //获取摄像机列表信息
    virtual bool JF_MatrixGetCameras_k(LLONG lLoginID,const DH_IN_MATRIX_GET_CAMERAS* pInParam,DH_OUT_MATRIX_GET_CAMERAS* pOutParam,int nWaitTime);
    //获取通道编码
    virtual bool JF_GetNewDevConfig_k(LLONG lLoginID, char* szCommand, int nChannelID, char* szOutBuffer, DWORD dwOutBufferSize, int *error, int waittime);
    //设置通道编码
    virtual bool JF_SetNewDevConfig_k(LLONG lLoginID, char* szCommand, int nChannelID, char* szInBuffer, DWORD dwInBufferSize, int *error, int *restart, int waittime);
    //设置前端IPC的账号密码（用户名密码）/IP信息（IP地址与端口）以及重要参数的读取设置（编码信息、OSD信息_通道时间标签以及位置）
    virtual bool JF_SetCameraInfo_k(JF_CameraInfo CameraInfo);
    //设置设备用户名、密码
    virtual bool JF_OperateUserInfoNew_k(LLONG lLoginID,int nOperateType,void *opParam,void *subParam,void* pReserved,int waittime);
    //设置设备IP、端口
    virtual bool JF_ModifyDevice_k(DEVICE_NET_INFO_EX *pDevNetInfo,DWORD dwWaitTime,int *iError,char *szLocalIp,void *reserved);
    //获取设备信息包括（系统信息（CPU、内存使用情况）、硬盘信息（硬盘总容量、剩余可用空间、锁定空间大小）、版本信息等）
    virtual bool JF_QuerySystemStatus_k(LLONG lLoginID,DH_SYSTEM_STATUS *pstuStatus,int nWaitTime);
    //NVR批量和单个通道的文件锁定信息设置
    virtual bool JF_SetMarkFileByTime_k(LLONG lLoginID,const NET_IN_SET_MARK_FILE_BY_TIME* pInParam,NET_OUT_SET_MARK_FILE_BY_TIME* pOutParam,int nWaitTime);
    //NVR获取文件锁定的信息
    virtual bool JF_GetMarkInfo_k(LLONG lLoginID,const NET_IN_GET_MARK_INFO* pInParam,NET_OUT_GET_MARK_INFO* pOutParam,int nWaitTime);
    //通过SDK控制存储抓取实时图片
    virtual bool JF_SnapPictureEx_k(LLONG lLoginID,SNAP_PARAMS *par,int* reserved);
    //控制前端设备恢复出厂设置以及重启关机操作
    virtual bool JF_ResetSystem_k(LLONG lLoginID,const NET_IN_RESET_SYSTEM* pstInParam,NET_OUT_RESET_SYSTEM* pstOutParam,int nWaitTime);
    //控制配置文件导出
    virtual bool JF_ExportConfigFile_k(LLONG lLoginID,DH_CONFIG_FILE_TYPE emConfigFileType,char *szSavedFilePath,fDownLoadPosCallBack cbDownLoadPos,long dwUserData);
    //控制配置文件导入
    virtual bool JF_ImportConfigFile_k(LLONG lLoginID,char *szFileName,fDownLoadPosCallBack cbUploadPos,long dwUserData,DWORD param);
    //获取OSD接口
    virtual bool JF_GetDVRConfig_OSD_k(LLONG   loginID,LONG lChannel,char* OSD_lpOutBuffer);
    //获取字符叠加的接口
    virtual bool JF_GetDVRConfig_CharOSD_k(LLONG   loginID,LONG lChannel,char* char_lpOutBuffer);
    //设置OSD接口
    virtual bool JF_SetDVRConfig_OSD_k(LLONG   loginID,LONG lChannel,char* OSD_lpInBuffer);
    //设置字符叠加的接口
    virtual bool JF_SetDVRConfig_CharOSD_k(LLONG   loginID,LONG lChannel,char* char_lpInBuffer);
    //获取软件版本
    virtual  bool JF_GetSoftwareVersion_k(LLONG lLoginID, const NET_IN_GET_SOFTWAREVERSION_INFO* pstInParam, NET_OUT_GET_SOFTWAREVERSION_INFO* pstOutParam, int nWaitTime);
    //云台控制
	virtual DWORD  JF_PTZControl_k(LLONG    loginID,
                                    int    nChannelID,       //通道号
                                    DWORD dwPTZCommand,     //球机控制命令，param2相当于参数dwStep
                                    LONG  lParam1,          //参数1
                                    LONG  lParam2,          //参数2
                                    LONG  lParam3,          //参数3
                                    BOOL  dwStop=FALSE,     //是否停止，TRUE表示停止
                                    void* param4=NULL);     //支持扩展控制命令参数
    //获取设备配置信息
	virtual DWORD  JF_GetDVRConfig_k(LLONG   loginID,
                                    DWORD    dwCommand,        //设备配置命令参见配置命令 
                                    LONG     lChannel,         //通道号，如果获取全部通道数据为0xFFFFFFFF，如果命令不需要通道号，该参数无效
                                    void*    lpOutBuffer,      //接受数据缓冲指针
                                    DWORD    dwOutBufferSize,  //接收数据缓冲长度(以字节为单位) 
                                    LPDWORD  lpBytesReturned,  //实际收到数据的长度
                                    int      waittime=500);    //等待超时时间 
	//配置设备属性
    virtual DWORD  JF_SetDVRConfig_k(LLONG   loginID,
                                    DWORD  dwCommand,          //设备配置命令参见配置命令
                                    LONG    lChannel,           //通道号
                                    LPVOID  lpInBuffer,         //数据输入缓冲指针
                                    DWORD   dwInBufferSize,     //数据缓冲长度(以字节为单位) 
                                    int     waittime=500);      //等待时间 
    //查找文件
	virtual DWORD  JF_FindFile_k(LLONG   loginID,
                                int      nChannelID,        //通道号
                                int      nRecordFileType,   //文件类型
                                JF_Timer  *stime,            //起始时间戳
                                JF_Timer  *etime,            //截止时间戳
                                fFindFileLog cbFindFile,    //超时时间
                                 long userData,
                                LLONG *plhFindFile=NULL,
                                sem_t *pSem=NULL);
	//停止文件查找                       文件查询句柄
    virtual DWORD  JF_StopFindFile_k(LLONG lFindHandle);
	//查找日志
	virtual DWORD  JF_FindLog_k(LLONG   loginID,
                                int     logType,        //日志类型
                                char    *pLogBuffer,    //日志缓冲区
                                int     maxlen,         //日志缓冲区的最大长度(单位字节) 
                                int     *nRetLogBuflen); //返回的日志长度(单位字节)
                                 
	//开启监听                                        监听端口         本机IP
	virtual DWORD  JF_StartListen_k(LLONG loginID, WORD wPort, char* strHostIP, fMSGCallBack_k cbMessage, long dwMsgUserData, fServiceCallBack_k cbService, long dwServiceUserData);
    //停止监听                                          服务器句柄
	virtual DWORD  JF_StopListen_k(LLONG loginID, LLONG lhService=-1);
    //重起设备
	virtual DWORD  JF_RebootDVR_k(LLONG loginID);
    //关闭设备
	virtual DWORD  JF_ShutDownDVR_k(LLONG loginID);
    //查询设备状态
	virtual DWORD  JF_QueryDVRState_k(LLONG   loginID,
                                      int    nType,         //查询信息类型
                                      char   *pBuf,         //接收查询返回的数据的缓存
                                      int    nBufLen,       //缓存长度，单位字节
                                      int    *pRetLen);      //返回的数据长度，单位字节
    //设置设备时间
	virtual DWORD  JF_SetDVRDateTime_k(LLONG  loginID,
                                       DWORD year,        //年
                                       DWORD mon,         //月
                                       DWORD day,         //日
                                       DWORD hour,        //时
                                       DWORD min,         //分
                                       DWORD sec);        //秒

    //  按文件名下载  
    virtual JF_DownloadFile_K* JF_DownloadFile_k(LLONG   loginID,
                      NET_RECORDFILE_INFO &struFileInfo, //文件信息
                      char* strSaveFileName,                    //保存文件名
                      int streamType=0,                         //码流类型
                      fDownLoadPosCallBack_k cbFileDownloadPos=NULL, //文件下载进度回调函数
                      long        dwPosUserData=0,
                      fDataCallBack_k cbFileDownloadData=NULL,    //数据回调函数
                      long dwDataUser=0);                  //用户自定义回调数据
    //  按时间下载
    virtual JF_DownloadFile_K* JF_DownloadFile_k(LLONG   loginID,EM_REAL_DATA_TYPE nSCtype,
                      int chan,                             //通道号
                      JF_Timer *stime,                          //起始时间
                      JF_Timer *etime,                          //截止时间
                      char* strSaveFileName,                //保存文件名
                      int nRecordFileType,                  //文件类型
                      int streamType=0,                     //码流类型 
                      fDownLoadPosCallBack_k cbFileDownloadPos=NULL, //文件下载进度回调函数
                      long        dwPosUserData=0,
                      fDataCallBack_k cbFileDownloadData=NULL,//数据回调函数
                      long dwDataUser=0);              //用户自定义回调数据

    //  按文件名回放录像
	virtual JF_PlayBack_K* JF_PlayBack_k(LLONG   loginID,
                  NET_RECORDFILE_INFO &struFileInfo,  //文件信息
                  HWND                       hWnd=NULL,      //窗口句柄
                  fPlayBackPosCallBack_k       cbDownloadPos=NULL,  //进度回调函数
                  long                     dwPosUserData=0,
                  fDataCallBack_k              cbPlayBack=NULL,//回放数据回调
                  long                     dwDataUser=0);//用户自定义数据
	//  按时间回放录像
    virtual JF_PlayBack_K* JF_PlayBack_k(LLONG   loginID,EM_REAL_DATA_TYPE nSCtype,
                int         nChannelID,         //通道号
                JF_Timer *   stime,              //起始时间
                JF_Timer *   etime,              //截止时间
                HWND          hWnd=NULL,          //窗口句柄
                fPlayBackPosCallBack_k cbDownloadPos=NULL,    //进度回调函数
                long        dwPosUserData=0,
                fDataCallBack_k cbPlayBack=NULL,    //数据回调函数
                long        dwDataUser=0);   //用户自定义回调数据

    //  开启语音对讲
    virtual JF_VoiceCom_K* JF_VoiceCom_k(LLONG loginID, BOOL Record=FALSE, BOOL CSMode=TRUE, fVoiceDataCallBack cbAudioData=NULL, long dwUser=0);

    //开启实时预览
    virtual JF_RealPlay_K* JF_RealPlay_k(LLONG loginID, int nChannelID, HWND hWnd=NULL, unsigned int playType=1, unsigned  int  replayType=0,fRealDataCallBackEx cbRealPlayData=NULL, long dwUser=0,fRealDataCallBack fcallback=NULL);

    bool getFlag()
    {
        return flag;
    }

private:
    bool flag;
};

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_DownloadFile_K 类   |------------------------------
//-----------------|-----------------------------|------------------------------
class JF_DownloadFile_K
{   
    //  按文件名下载  
    JF_DownloadFile_K(LLONG loginID,
                      NET_RECORDFILE_INFO &struFileInfo, //文件信息
                      char* strSaveFileName,                    //保存文件名
                      int streamType=0,                         //码流类型
                      fDownLoadPosCallBack_k cbFileDownloadPos=NULL, //文件下载进度回调函数
                      long        dwPosUserData=0,
                      fDataCallBack_k cbFileDownloadData=NULL,    //数据回调函数
                      long dwDataUser=0);                  //用户自定义回调数据
    //  按时间下载
    JF_DownloadFile_K(LLONG loginID,EM_REAL_DATA_TYPE nSCtype,
                      int chan,                             //通道号
                      JF_Timer *stime,                          //起始时间
                      JF_Timer *etime,                          //截止时间
                      char* strSaveFileName,                //保存文件名
                      int nRecordFileType,                  //文件类型
                      int streamType=0,                     //码流类型 
                      fDownLoadPosCallBack_k cbFileDownloadPos=NULL, //文件下载进度回调函数
                      long        dwPosUserData=0,
                      fDataCallBack_k cbFileDownloadData=NULL,//数据回调函数
                      long dwDataUser=0);              //用户自定义回调数据
public:
    //  停止文件下载
    virtual bool Release();
    virtual ~JF_DownloadFile_K();
    //  获取下载进度                     已下载的长度，单位:KB  下载的总长度，单位:KB
	virtual DWORD   JF_GetDownloadPos_k(int *nDownloadSize, int *nTotalSize=NULL);

    bool getFlag()
    {
        return flag;
    }

    LLONG getHandle()
    {
        return m_lhDownloadFile;
    }

private:
	LONG            m_lhDownloadFile;  		//文件下载句柄
    const LONG      m_lLoginID;     //用户登录ID
    bool    flag;

    friend JF_DownloadFile_K* JF_SDK_K::JF_DownloadFile_k(LLONG loginID,
                      NET_RECORDFILE_INFO &struFileInfo, //文件信息
                      char* strSaveFileName,                    //保存文件名
                      int streamType,                         //码流类型
                      fDownLoadPosCallBack_k cbFileDownloadPos, //文件下载进度回调函数
                      long        dwPosUserData,
                      fDataCallBack_k cbFileDownloadData,    //数据回调函数
                      long dwDataUser);                  //用户自定义回调数据
    //  按时间下载
    friend JF_DownloadFile_K* JF_SDK_K::JF_DownloadFile_k(LLONG loginID,EM_REAL_DATA_TYPE nSCtype,
                      int chan,                             //通道号
                      JF_Timer *stime,                          //起始时间
                      JF_Timer *etime,                          //截止时间
                      char* strSaveFileName,                //保存文件名
                      int nRecordFileType,                  //文件类型
                      int streamType,                     //码流类型 
                      fDownLoadPosCallBack_k cbFileDownloadPos, //文件下载进度回调函数
                      long        dwPosUserData,
                      fDataCallBack_k cbFileDownloadData,//数据回调函数
                      long dwDataUser);              //用户自定义回调数据
};

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_PlayBack_K 类       |------------------------------
//-----------------|-----------------------------|------------------------------
class JF_PlayBack_K
{
    //  按文件名回放录像
	JF_PlayBack_K(LLONG                       loginID,
                  NET_RECORDFILE_INFO &struFileInfo,  //文件信息
                  HWND                       hWnd=NULL,      //窗口句柄
                  fPlayBackPosCallBack_k       cbDownloadPos=NULL,  //进度回调函数
                  long                     dwPosUserData=0,
                  fDataCallBack_k              cbPlayBack=NULL,//回放数据回调
                  long                     dwDataUser=0);//用户自定义数据
	//  按时间回放录像
    JF_PlayBack_K(LLONG        loginID,EM_REAL_DATA_TYPE nSCtype,
                int           nChannelID,         //通道号
                JF_Timer *   stime,              //起始时间
                JF_Timer *   etime,              //截止时间
                HWND          hWnd=NULL,          //窗口句柄
                fPlayBackPosCallBack_k cbDownloadPos=NULL,    //进度回调函数
                long        dwPosUserData=0,
                fDataCallBack_k cbPlayBack=NULL,    //数据回调函数
                long        dwDataUser=0);   //用户自定义回调数据
public:
    //  停止录像回放
    virtual bool Release();
    virtual ~JF_PlayBack_K();
    //  改变回放进度
	virtual BOOL   JF_SeekPlayBack_k(unsigned int offset=0);
    //  快放    
	virtual BOOL   JF_FastPlayBack_k();	
    //  慢放	
	virtual BOOL   JF_SlowPlayBack_k();	
    //  暂停或恢复播放	
	virtual BOOL   JF_PausePlayBack_k();
    //  恢复正常播放	
	virtual BOOL   JF_NormalPlayBack_k();
    
    bool getFlag()
    {
        return flag;
    }

    LLONG getHandle()
    {
        return m_lhPlayBack;
    }

private:
	LONG           m_lhPlayBack;	        //回放句柄
    BOOL           m_bPause;                //是否暂停 TRUE-暂停 FALSE-播放

    const unsigned int   m_nFileSize;       //回放文件大小
    unsigned int   m_nFileTime;       //回放文件的时间长度
    const bool           m_bMethod;         //播放方式  true-按文件名 false-按时间
    const LONG           m_lLoginID;     //用户登录ID
    bool  flag;

    //  按文件名回放录像
	friend JF_PlayBack_K* JF_SDK_K::JF_PlayBack_k(LLONG loginID,
                  NET_RECORDFILE_INFO &struFileInfo,  //文件信息
                  HWND                       hWnd,      //窗口句柄
                  fPlayBackPosCallBack_k       cbDownloadPos,  //进度回调函数
                  long                     dwPosUserData,
                  fDataCallBack_k              cbPlayBack,//回放数据回调
                  long                     dwDataUser);//用户自定义数据
	//  按时间回放录像
    friend JF_PlayBack_K* JF_SDK_K::JF_PlayBack_k(LLONG loginID,EM_REAL_DATA_TYPE nSCtype,
                int         nChannelID,         //通道号
                JF_Timer *   stime,              //起始时间
                JF_Timer *   etime,              //截止时间
                HWND          hWnd,          //窗口句柄
                fPlayBackPosCallBack_k cbDownloadPos,    //进度回调函数
                long        dwPosUserData,
                fDataCallBack_k cbPlayBack,    //数据回调函数
                long        dwDataUser);   //用户自定义回调数据
};

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_VoiceCom_K 类       |------------------------------
//-----------------|-----------------------------|------------------------------
class JF_VoiceCom_K
{
    //  开启语音对讲
    JF_VoiceCom_K(LLONG loginID, BOOL Record=FALSE, BOOL CSMode=TRUE, fVoiceDataCallBack cbAudioData=NULL, long dwUser=0);
public:
    //  关闭语音对讲
    virtual bool Release();
    virtual ~JF_VoiceCom_K();
    //  开启/关闭录音
    virtual DWORD   JF_RecordControl_k();
    //  发送音频数据
    virtual DWORD   JF_SendTalkData_k(char *pSendBuf, DWORD dwBufSize);
    //  设置音量大小
   	virtual DWORD	JF_SetVolume_k(unsigned int nVolume=0);

    bool getFlag()
    {
        return flag;
    }

    LLONG getHandle()
    {
        return m_lhVoiceCom;
    }

private:
   	LONG 			m_lhVoiceCom;               //语音对讲句柄
    BOOL            m_bRecord;                  //是否开启本地录音
    const LONG      m_lLoginID;     //用户登录ID
    bool    flag;

    //  开启语音对讲
    friend JF_VoiceCom_K* JF_SDK_K::JF_VoiceCom_k(LLONG loginID, BOOL Record, BOOL CSMode, fVoiceDataCallBack cbAudioData, LDWORD dwUser);
};

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_RealPlay_K 类       |------------------------------
//-----------------|-----------------------------|------------------------------
class JF_RealPlay_K
{
    //开启实时预览
        JF_RealPlay_K(LLONG loginID, int nChannelID, HWND hWnd=NULL, unsigned int playType=1,unsigned  int  replayType=0, fRealDataCallBackEx cbRealPlayData=NULL, long dwUser=0,fRealDataCallBack fcallback=NULL);
public:
    //关闭实时预览
    virtual bool Release();
    virtual ~JF_RealPlay_K();
    //设置视频效果参数
    virtual DWORD  JF_SetVideoEffect_k(JF_VideoEffect* struVidEff);
    //获取视频效果参数
    virtual DWORD  JF_GetVideoEffect_k(JF_VideoEffect* struVidEff);
    //打开声音
	virtual DWORD  JF_OpenSound_k();
    //关闭声音
	virtual DWORD  JF_CloseSound_k();
    //设置音量
	virtual DWORD  JF_SetVolume_k(int nVolume=0);
    //保存实时图像数据
	virtual DWORD  JF_SaveRealData_k(char *strFileName);
    //停止实时图像数据保存
	virtual DWORD  JF_StopSaveRealData_k();
    //抓图
    virtual DWORD  JF_CapturePicture_k(char *strPictureName, unsigned int picFormat=0,fSnapRev OnSnapRevMessage=NULL);

    bool getFlag()
    {
        return flag;
    }

    LLONG getHandle()
    {
        return m_lhRealPlay;
    }

private:
	LLONG 		   m_lhRealPlay;	       //实时预览句柄
    const LLONG     m_lLoginID;     //用户登录ID
    bool    flag;

    //开启实时预览
        friend JF_RealPlay_K* JF_SDK_K::JF_RealPlay_k(LLONG loginID, int nChannelID, HWND hWnd, unsigned int playType,unsigned  int  replayType, fRealDataCallBackEx cbRealPlayData, long dwUser,fRealDataCallBack fcallback);
};

#endif // !JF_K_CLASS
