#ifndef JF_V_CLASS
#define JF_V_CLASS


#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Config.h"
#include "CLog.h"
#include "JF_Define.h"

class JF_DownloadFile_V;
class JF_PlayBack_V;
class JF_VoiceCom_V;
class JF_RealPlay_V;

#define Error_v() do\
{\
        fprintf(stderr, "V-Error-MSG::<%s> <%s> <%d> -- Error_v Code: %d\n",\
        __FILE__,__FUNCTION__,__LINE__,NET_DVR_GetLastError());\
}while(0)

#define error_v(com) do\
{\
        fprintf(stderr, "Error-MSG::<%s> <%s> <%d> -- Error_v of: %s\n",\
        __FILE__,__FUNCTION__,__LINE__,com);\
}while(0)

BOOL SADPdevice_v(PDEVICE_FIND_CALLBACK pDeviceFindCallBack, int bInstallNPF = 0, void* pUserData = NULL);
BOOL StopSADPdevice_v();
//-----------------|-----------------------------|------------------------------
//-----------------|          JF_SDK_V 类        |------------------------------
//-----------------|-----------------------------|------------------------------
class JF_SDK_V final
{
public:
	JF_SDK_V();
	virtual ~JF_SDK_V();
	//设备登录
    virtual LONG  JF_Login_v(char*  pchDVRIP,		//设备IP
                    		  WORD   wDVRPort,		//设备端口
                     		  char*  pchUserName,	//登录用户名
                   			  char*  pchPasswd,		//登录密码
            	   			  void*  pUserData,		//用户数据
                              char*  deviceType,
							  NET_DVR_DEVICEINFO_V40* struDeviceInfo);//设备信息结构体
    //设备登出
	virtual DWORD  JF_Logout_v(LONG loginID);
    //设备在线状态检测
    virtual BOOL   JF_RemoteControl_v(LONG loginID);
	//获取错误码
	virtual  int JF_GetLastError_v();
	//批量获取NVR上IPC通道
	virtual bool JF_GetRemoteDecice_v(LONG lUserID,DWORD dwCommand,LONG lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned);
	//批量设置NVR上IPC通道
	virtual bool JF_SetRemoteDevice_v(LONG lUserID, DWORD dwCommand,LONG lChannel, LPVOID lpInBuffer, DWORD dwInBufferSize);
	//批量更换修改NVR上IPC通道
	virtual bool JF_ChangRemoteDevice_v(JF_UserIDInfo* deviceInfo);
	//批量删除NVR上IPC通道
	virtual bool JF_DeleteRemoteDevice_v(JF_UserIDInfo* deviceInfo);
	//获取存储上所有的通道信息
	virtual bool JF_QueryDevInfo_v(JF_DeviceInfo* deviceInfo);
	//设置前端IPC的账号密码（用户名密码）/IP信息（IP地址与端口）以及重要参数的读取设置（编码信息、OSD信息_通道时间标签以及位置）
	virtual bool JF_SetCameraInfo_v(JF_CameraInfo CameraInfo);
	//获取设备信息包括（设备型号、系统信息（CPU、内存使用情况）、硬盘信息（硬盘总容量、剩余可用空间、锁定空间大小）、版本信息等）
	virtual bool JF_GetDeviceInfo_v(JF_Details DetailsInfo);
	//NVR批量和单个通道的文件锁定信息读取（通道号、文件列表以及文件大小——支持文件和时间锁定及解锁功能），并支持批量和单个通道设置文件锁定计划设置
	virtual bool JF_MarkInfo_v(JF_MarkTImeInfo stInfo);
	//控制前端设备恢复出厂设置以及重启关机操作
	virtual bool JF_ResetSystem_v(int stType);
	//控制配置文件导出
	virtual bool JF_ExportConfigFile_v(char *szFileName,fMarkConfigPosCallBack cbUploadPos,LDWORD dwUserData);
	//控制配置文件导入
	virtual bool JF_ImportConfigFile_v(char *szFileName,fMarkConfigPosCallBack cbUploadPos,LDWORD dwUserData);
	//获取OSD接口
	virtual bool JF_GetDVRConfig_OSD_v(LONG loginID,LONG lChannel,char* OSD_lpOutBuffer);
	//获取字符叠加的接口
	virtual bool JF_GetDVRConfig_CharOSD_v(LONG loginID,LONG lChannel,char* char_lpOutBuffer);
	//设置OSD接口
	virtual bool JF_SetDVRConfig_OSD_v(LONG loginID,LONG lChannel,char* OSD_lpInBuffer);
	//设置字符叠加的接口
	virtual bool JF_SetDVRConfig_CharOSD_v(LONG loginID,LONG lChannel,char* char_lpInBuffer);
	//云台基本控制
	virtual DWORD  JF_PTZControl_v(LONG loginID, LONG lChannel, DWORD dwPTZCommand, BOOL bStop=FALSE);
    //云台控制带速度
    virtual bool   JF_PTZControlWithSpeed_v(LONG loginID,LONG lChannel,DWORD dwPTZCommand,DWORD bStop,DWORD dwSpeed);
	//云台预置点操作
	virtual DWORD  JF_PTZPreset_v(LONG loginID, LONG lChannel, DWORD dwPTZCommand, DWORD dwPresetIndex);
	//云台巡航操作
	virtual DWORD  JF_PTZCruise_v(LONG loginID, LONG lChannel, DWORD dwPTZCommand, BYTE byCruiseRoute, BYTE byCruisePoint, WORD wInput);
	//云台轨迹操作
	virtual DWORD  JF_PTZTrack_v(LONG loginID, LONG lChannel, DWORD dwPTZCommand);
    //云台守望设置边界
    virtual bool  JF_PTZRemoteControl_v(LONG lUserID,DWORD dwCommand,LPVOID lpInBuffer,DWORD dwInBufferSize);
	//获取设备配置信息
	virtual DWORD  JF_GetDVRConfig_v(LONG loginID, DWORD dwCommand, LONG lChannel, void* pOutBuf, DWORD dwOutBufSize, DWORD* pBytesRet);
	//配置设备
	virtual DWORD  JF_SetDVRConfig_v(LONG loginID, DWORD dwCommand, LONG lChannel, void* pInBuf, DWORD dwInBufSize);
	//查找文件
    virtual DWORD  JF_FindFile_v(LONG loginID, LONG lChannel, DWORD dwFileType, JF_Timer *stime, JF_Timer *etime, fFindFileLog cbFindFile, long userData, BYTE byStreamType=0xff,LLONG *plhFindFile=NULL,sem_t *pSem=NULL);
	//停止文件查找
	virtual DWORD  JF_StopFindFile_v(LONG lhFindFile);
    //
   // virtual int    JF_FindNextFile_v(LONG lhFindFile,LPNET_DVR_FINDDATA_V40* lpFindData);
	//查找日志
    virtual DWORD  JF_FindLog_v(LONG loginID, LONG lSelectMode, DWORD dwMajorType, DWORD dwMinorType, JF_Timer *stime, JF_Timer *etime, fFindFileLog cbFindLog,LDWORD userData);
	//停止日志查找
	virtual DWORD  JF_StopFindLog_v(LONG lhFindLog);
	//开启监听
    virtual DWORD  JF_StartListen_v(int hLoginID,MSGCallBack cbMSG, WORD wLocalPort, char *pLocalIP=NULL, long pUserData=NULL);
	//停止监听
	virtual DWORD  JF_StopListen_v(LONG lhListen);
	//重启设备
	virtual DWORD  JF_RebootDVR_v(LONG loginID);
	//关闭设备
	virtual DWORD  JF_ShutDownDVR_v(LONG loginID);
	//查询设备状态
	virtual DWORD  JF_QueryDVRState_v(LONG loginID, DWORD dwCommand, DWORD dwCount, void* pInBuf, DWORD dwInBufSize, void* pStatusList, void* pOutBuf, DWORD dwOutBufSize);
	//设置设备时间
	virtual DWORD  JF_SetDVRDateTime_v(LONG loginID, DWORD dwYear, DWORD dwMon, DWORD dwDay, DWORD dwHour, DWORD dwMin, DWORD dwSec);

	//按文件名下载
    virtual JF_DownloadFile_V* JF_DownloadFile_v(LONG loginID, char* pSrcFileName, char* pSaveFileName);
	//按时间下载
    virtual JF_DownloadFile_V* JF_DownloadFile_v(LONG loginID, char* pSaveFileName, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType=0);

	//按文件名回放
	virtual JF_PlayBack_V* JF_PlayBack_v(LONG loginID, char* strFileName, HWND hWnd=NULL, fDataCallBack_v cbPlayData=NULL, long pUser=NULL);
	//按时间段回放
	virtual JF_PlayBack_V* JF_PlayBack_v(LONG loginID, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType=0, HWND hWnd=NULL, fDataCallBack_v cbPlayData=NULL, long pUser=NULL);

	//开启语音对讲
    virtual JF_VoiceCom_V* JF_VoiceCom_v(LONG loginID, fVoiceDataCallBack cbVoiceData=NULL, long dwUser=0,BOOL ModeSize=FALSE);

	//开启实时预览
	virtual JF_RealPlay_V* JF_RealPlay_v(LONG loginID, LONG lChannel, DWORD dwPlayBufNum=1, DWORD dwStreamType=0, DWORD dwLinkMode=0, 
				HWND hWnd=NULL, fDataCallBack_v cbRealData=NULL, long pUser=NULL);

	bool getFlag()
	{
		return flag;
	}


private:
	bool flag;
};

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_DownloadFile_V 类   |------------------------------
//-----------------|-----------------------------|------------------------------
class JF_DownloadFile_V
{
	//按文件名下载
    JF_DownloadFile_V(LONG loginID, char* pSrcFileName, char* pSaveFileName);
	//按时间下载
    JF_DownloadFile_V(LONG loginID, char* pSaveFileName, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType=0);
public:
	//停止文件下载
	virtual bool Release();
    virtual ~JF_DownloadFile_V();
	//获取下载进度
	virtual DWORD   JF_GetDownloadPos_v();

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
    const LONG      m_lLoginID;         //用户登录ID 
	bool 	flag;

	//按文件名下载
    friend JF_DownloadFile_V* JF_SDK_V::JF_DownloadFile_v(LONG loginID, char* pSrcFileName, char* pSaveFileName);
	//按时间下载
    friend JF_DownloadFile_V* JF_SDK_V::JF_DownloadFile_v(LONG loginID, char* pSaveFileName, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType);
};

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_PlayBack_V 类       |------------------------------
//-----------------|-----------------------------|------------------------------ 
class JF_PlayBack_V
{
	//按文件名回放
	JF_PlayBack_V(LONG loginID, char* strFileName, HWND hWnd=NULL, fDataCallBack_v cbPlayData=NULL, long pUser=NULL);
	//按时间段回放
	JF_PlayBack_V(LONG loginID, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType=0, HWND hWnd=NULL, fDataCallBack_v cbPlayData=NULL, long pUser=NULL);
public:
	//停止录像回放
	virtual bool Release();
	virtual ~JF_PlayBack_V();
	//改变/获取回放进度
	virtual BOOL   JF_SGPlayBackPos_v(unsigned int* nPos=NULL, BOOL SGetPos=TRUE);
	//快放
	virtual BOOL   JF_FastPlayBack_v();	
	//慢放
	virtual BOOL   JF_SlowPlayBack_v();	
	//暂停或恢复播放
	virtual BOOL   JF_PausePlayBack_v();
	//恢复正常播放
	virtual BOOL   JF_NormalPlayBack_v();

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
	const bool 	   m_bMethod;				//回放方式 true-按文件名 false-按时间
	bool		   m_bPause;			//回放暂停标记
    const LONG     m_lLoginID;         //用户登录ID 
	bool 	flag;

	//按文件名回放
	friend JF_PlayBack_V* JF_SDK_V::JF_PlayBack_v(LONG loginID, char* strFileName, HWND hWnd, fDataCallBack_v cbPlayData, long pUser);
	//按时间段回放
	friend JF_PlayBack_V* JF_SDK_V::JF_PlayBack_v(LONG loginID, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType, HWND hWnd, fDataCallBack_v cbPlayData, long pUser);
};

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_VoiceCom_V 类       |------------------------------
//-----------------|-----------------------------|------------------------------
void callBackVoice(LONG lVoiceComHandle,char *pRecvDataBuffer,DWORD dwBufSize,BYTE byAudioFlag,void *pUser);
class JF_VoiceCom_V
{
	//开启语音对讲
    JF_VoiceCom_V(LONG loginID, fVoiceDataCallBack cbVoiceData=NULL, long dwUser=0,BOOL ModeSize=FALSE);

public:
    //关闭语音对讲
	virtual bool Release();
	virtual ~JF_VoiceCom_V();
	//设置音量大小
   	virtual DWORD	JF_SetVolume_v(WORD wVolume=0);

	bool getFlag()
	{
		return flag;
	}

	LLONG getHandle()
    {
        return m_lhVoiceCom;
    }

private:
   	LONG 			m_lhVoiceCom;           	//语音对讲句柄
    const LONG     	m_lLoginID;         //用户登录ID 
	bool 	flag;

		//开启语音对讲
    friend JF_VoiceCom_V* JF_SDK_V::JF_VoiceCom_v(LONG loginID, fVoiceDataCallBack cbVoiceData, long dwUser,BOOL Modesize);
};

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_RealPlay_V 类       |------------------------------
//-----------------|-----------------------------|------------------------------
class JF_RealPlay_V
{
	//开启实时预览
	JF_RealPlay_V(LONG loginID, LONG lChannel, DWORD dwPlayBufNum=1, DWORD dwStreamType=0, DWORD dwLinkMode=0, 
				HWND hWnd=NULL, fDataCallBack_v cbRealData=NULL, long pUser=NULL);
public:
	//关闭实时预览
	virtual bool Release();
	virtual ~JF_RealPlay_V();
	//设置抛帧数目
	virtual DWORD  JF_ThrowBFrame_v(DWORD dwNum=0);
	//设置视频效果参数
    virtual DWORD  JF_SetVideoEffect_v(JF_VideoEffect* struVidEff, LONG lChannel=-1);
	//获取视频效果参数
    virtual DWORD  JF_GetVideoEffect_v(JF_VideoEffect* struVidEff, LONG lChannel=-1);
	//打开声音
	virtual DWORD  JF_OpenSound_v();
	//关闭声音
	virtual DWORD  JF_CloseSound_v();
	//调节音量
	virtual DWORD  JF_SetVolume_v(WORD wVolume=0);
	//实时数据保存
	virtual DWORD  JF_SaveRealData_v(char *sFileName);
	//停止实时数据保存
	virtual DWORD  JF_StopSaveRealData_v();
	//抓图
	virtual DWORD  JF_CapturePicture_v(char *pCapture, DWORD dwCaptureMode=0, DWORD dwPicSize=0);

	bool getFlag()
	{
		return flag;
	}

	LLONG getHandle()
    {
        return m_lhRealPlay;
    }

private:
	LONG 		   m_lhRealPlay;	       //实时预览句柄
    const LONG     m_lLoginID;         //用户登录ID 
	bool 	flag;

	//开启实时预览
	friend JF_RealPlay_V* JF_SDK_V::JF_RealPlay_v(LONG loginID, LONG lChannel, DWORD dwPlayBufNum, DWORD dwStreamType, DWORD dwLinkMode, 
				HWND hWnd, fDataCallBack_v cbRealData, long pUser);
};

#endif // !JF_V_CLASS
