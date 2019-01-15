#ifndef JF_CLASS
#define JF_CLASS
#define DLL_PUBLIC __attribute__((visibility("default")))

#include <map>
#include <vector>
#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include "JF_K_Class.h"
#include "JF_V_Class.h"
#include "JF_Define.h"

#define DH  1
#define HK  2

class JF_Users;
class JF_DownloadFile;
class JF_PlayBack;
class JF_VoiceCom;
class JF_RealPlay;
int Get_Config();
typedef struct {
    fAlarmMSGCallBack   cbAlarmMsg;
    void*               pAlarmUserData;
}JF_ListenCallbackArgs;

extern "C" int SADPdevice(char* deviceType,char* stip,char* edip,void* sadpcallback,LDWORD dwuser,DWORD dWaitTime);
extern "C" int StopSADPdevice(char* device);
void* static_LoginThread(void* LoginStruct);
//-------------------------------------------------------
//|------------>      JF_SDK 类      <-------------------|
//-------------------------------------------------------
class JF_SDK final
{
    JF_SDK(int initType, fDisConnect_k cbDisConnect_k=NULL, LDWORD dwDiscUser_k=0);
public:

    //卸载SDK
    virtual ~JF_SDK();
    //加载SDKS
    static JF_SDK* JF_SDK_Init(int DvrType=0,fDisConnect_k cbDisConnect_k=NULL, LDWORD dwDiscUser_k=0);
    //登录
    LONG JF_Login(char *pchDVRIP, WORD wDVRPort, char *pchUserName, char *pchPasswd,LDWORD *dwUser,char*deviceType, JF_DVRInfo *pDVRInfo, DVRLoginCallBack pUsersLoginCB);
    //转发登录                 IP           端口              用户名               密码             设备类型              设备信息              操作类
    LONG JF_Login_SM(char *pchDVRIP, WORD wDVRPort, char *pchUserName, char *pchPasswd,char*deviceType, JF_DVRInfo *pDVRInfo,JF_Users* &pUsers);
    //登出
    bool JF_Logout(JF_Users* &pUsers);   
    bool JF_Logout(LLONG loginID);
    //获取错误码
    int  JF_GetLastError();
    void*  JF_LoginThread(void* LoginStruct);

    friend class JF_Users;
    friend class JF_DownloadFile;
    friend class JF_PlayBack;
    friend class JF_VoiceCom;
    friend class JF_RealPlay;
    friend void* static_LoginThread(void* LoginStruct);
    static std::vector<JF_UserIDInfo*> m_vcUserInfo;   //用户管理链表

private:
    JF_SDK_K *m_pk;
    JF_SDK_V *m_pv;
    DVRLoginCallBack DLoginCB;
    static bool flag;
};
//-------------------------------------------------------
//|------------>     JF_Users 类     <-------------------|
//-------------------------------------------------------
class JF_Users final
{
    JF_Users(LLONG loginID, int companyType);
public:
    //释放用户类对象
    virtual ~JF_Users();
    //
    void Setm_lLoginID(int hLoginID);
    //设备在线状态检测
    bool JF_RemoteControl();
    //批量设置NVR上IPC通道
    bool JF_SetRemoteDevice(LONG lChannel,LONG elChannel,Login_Format* deviceInfo);
    //批量更换修改NVR上IPC通道
    bool JF_ChangRemoteDevice(LONG lChannel,LONG elChannel);
    //批量删除NVR上IPC通道
    bool JF_DeleteRemoteDevice(LONG lChannel,LONG elChannel);
    //获取存储上所有的通道信息
    bool JF_QueryDevInfo(JF_DeviceInfo* deviceInfo,int* theory,int* real);
    //设置前端IPC的账号密码（用户名密码）/IP信息（IP地址与端口）以及重要参数的读取设置（编码信息、OSD信息_通道时间标签以及位置）
    bool JF_SetCameraInfo(JF_CameraInfo* CameraInfo);
    //设置账号密码IP信息
  /*数值	操作类型	    opParam对应结构体类型	     subParam对应结构体类型
    0	增加用户组	 USER_GROUP_INFO_NEW
    1	删除用户组	 USER_GROUP_INFO_NEW
    2	修改用户组	 USER_GROUP_INFO_NEW	 USER_GROUP_INFO_NEW
    3	增加用户	     USER_INFO_NEW
    4	删除用户	     USER_INFO_NEW
    5	修改用户	     USER_INFO_NEW	         USER_INFO_NEW
    6	修改用户密码	 USER_INFO_NEW	         USER_INFO_NEW*/
    bool JF_OperateUserInfoNew(int nOperateType,Login_Format* CameraInfo,int waittime);
    //获取设备信息包括（设备型号、系统信息（CPU、内存使用情况）、硬盘信息（硬盘总容量、剩余可用空间、锁定空间大小）、版本信息等）
    bool JF_GetDeviceInfo(JF_Details* DetailsInfo);
    //NVR批量和单个通道的文件锁定信息读取（通道号、文件列表以及文件大小——支持文件和时间锁定及解锁功能），并支持批量和单个通道设置文件锁定计划设置
    bool JF_MarkInfo(JF_MarkTImeInfo* stInfo);
    //通过SDK控制存储抓取实时图片
    bool JF_SnapPictureEx(LONG lChannel);
    //通过SDK控制存储抓取实时数据流
    //bool JF_SetRealDataCallBackEx()
    //控制前端设备恢复出厂设置以及重启操作
    bool JF_ResetSystem(int stType);
    //控制配置文件导出
    bool JF_ExportConfigFile(char *szFileName,fMarkConfigPosCallBack cbUploadPos,long dwUserData);
    //控制配置文件导入
    bool JF_ImportConfigFile(char *szFileName,fMarkConfigPosCallBack cbUploadPos,long dwUserData);
    //获取OSD接口
    bool JF_GetDVRConfig_OSD(LONG lChannel,char* OSD_lpOutBuffer);
    //获取字符叠加的接口
    bool JF_GetDVRConfig_CharOSD(LONG lChannel,char* char_lpOutBuffer);
    //设置OSD接口
    bool JF_SetDVRConfig_OSD(LONG lChannel,char* OSD_lpInBuffer);
    //设置字符叠加的接口
    bool JF_SetDVRConfig_CharOSD(LONG lChannel,char* char_lpInBuffer);
    //云台所有操作
    bool JF_PTZAll(LONG lChannel,int dwPTZCommand, LONG param1, LONG param2, LONG param3,bool dwStop);
    //云台基本操作                                     //speed=1~8
    bool JF_PTZControl(LONG lChannel, int PTZCommand, int speed=6, BOOL bStop=FALSE);
    //云台预置点操作                                   //预置点号从1开始,最多300个
    bool JF_PTZPreset(LONG lChannel, int PTZCommand, DWORD dwPresetIndex);
    //云台巡航操作
    bool JF_PTZCruise(LONG lChannel, int PTZCommand, BYTE byCruiseRoute, BYTE byCruisePoint_v, WORD wInput);
    //云台轨迹操作
    bool JF_PTZTrack(LONG lChannel, int PTZCommand);
    //获取设备配置信息
    bool JF_GetDVRConfig(DWORD dwCommand, LONG lChannel, JF_DevConfig* pOutBuf, DWORD dwOutBufSize, DWORD* pBytesRet);
    //配置设备
    bool JF_SetDVRConfig(DWORD dwCommand, LONG lChannel, void* pInBuf, DWORD dwInBufSize);
    //查找文件
    LLONG JF_FindFile(LONG lChannel, DWORD dwFileType, JF_Timer *stime, JF_Timer *etime, fFindFileLog cbFindFile,long dwUser, BYTE byStreamType_v=0xff);
    //查找下一条录像文件记录
    //int JF_FindNextFile(LLONG lFindHandle,JF_RecodeFile_Info* lpFindData);
    //停止文件查找
    bool JF_StopFindFile(LLONG lhFindFile);
    //查找日志
    LLONG JF_FindLog(JF_FindLogArgs *findLog);
    //停止日志查找
    bool JF_StopFindLog(LLONG lhFindLog);
    //BOOL g_cbListen_k(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG lDVRPort, LDWORD dwUser);
    //void g_cbListen_v(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void *pUser);
    //开启报警监听
    LLONG JF_StartListen(int hLoginID,fAlarmMSGCallBack cbAlarmMsg, long pAlarmUserData, WORD wPort, char* strHostIP=NULL, fServiceCallBack_k cbService_k=NULL, long dwServiceUserData_k=0);
    //停止报警监听
    bool JF_StopListen(LLONG lhListen);
    //重起设备
    bool JF_RebootDVR();
    //关闭设备
    bool JF_ShutDownDVR();
    //查询设备状态
    bool JF_QueryDVRState(JF_DVRState *state, LONG lChannel_v,DWORD dwCommand);
    //设置时间
    bool JF_SetDVRDateTime(JF_Timer *tm);

    //按文件名下载录像
    JF_DownloadFile* JF_DownloadFileByName(JF_RecordFileInfo &struFileInfo, char* strSaveFileName, int streamType_k=0, JF_K_DownloadFileArgs *args_k=NULL);
    //按时间段下载录像
    JF_DownloadFile* JF_DownloadFileByTime(LONG lChannel, char* strSaveFileName, JF_Timer *stime, JF_Timer *etime, int streamType=0, int fileType_k=0, JF_K_DownloadFileArgs *args_k=NULL);
    //关闭录像下载
    bool JF_StopDownloadFile(JF_DownloadFile* &pdl);
    
    //按文件名回放录像
    JF_PlayBack* JF_PlaybackByName(JF_RecordFileInfo &struFileInfo, HWND hWnd=NULL, fDataCallBacks cbPlaybackData=NULL, long pUserData=NULL, fPlayBackPosCallBack_k cbPlaybackPos_k=NULL, long dwPosUserData_k=0);
    //按时间段回放录像
    JF_PlayBack* JF_PlaybackByTime(LONG lChannel, JF_Timer *stime, JF_Timer *etime, HWND hWnd=NULL, BYTE byStreamType_v=0, fDataCallBacks cbPlayData=NULL, long pUserData=NULL, fPlayBackPosCallBack_k cbPlaybackPos_k=NULL, long dwPosUserData_k=0);
    //关闭录像回放
    bool JF_StopPlayback(JF_PlayBack* &ppb);

    //开启语音对讲                                                            // 对讲模式 TRUE-服务器模式 FALSE-客户端模式
    JF_VoiceCom* JF_Voicecom(fVoiceDataCallBack cbVoiceData=NULL, long dwUser=0, BOOL CSMode_k=TRUE);
    //关闭语音对讲
    bool JF_StopVoicecom(JF_VoiceCom* &pvc);

    //开启实时预览
    JF_RealPlay* JF_Realplay(LONG lChannel,long pUserData,unsigned int playType_k=1, JF_V_RealPlayArgs *args_v=NULL, HWND hWnd=NULL, fRealDataCallBackEx cbRealPlayData=NULL, fRealDataCallBack fcallback=NULL,JF_Users *User=NULL);
    //关闭实时预览
    bool JF_StopRealplay(JF_RealPlay* &prl);

    friend LONG JF_SDK::JF_Login(char *pchDVRIP, WORD wDVRPort, char *pchUserName, char *pchPasswd,LDWORD *dwUser,char*deviceType, JF_DVRInfo *pDVRInfo, DVRLoginCallBack pUsersLoginCB);
    friend bool JF_SDK::JF_Logout(JF_Users* &pUsers);
    friend bool JF_SDK::JF_Logout(LLONG loginID);
    friend void* JF_SDK::JF_LoginThread(void* LoginStruct);
    friend LONG JF_SDK::JF_Login_SM(char *pchDVRIP, WORD wDVRPort, char *pchUserName, char *pchPasswd,char*deviceType, JF_DVRInfo *pDVRInfo,JF_Users* &pUsers);
private:
    const int m_CompanyType;
    const LLONG m_lLoginID;
    char IP[32];
    EM_REAL_DATA_TYPE nSCtype;
    std::map<LLONG,pthread_t> m_IDtoThread;
    std::map<LLONG,void*> m_IDtoPoint;

    JF_SDK_K *m_pk;
    JF_SDK_V *m_pv;
};

//-------------------------------------------------------
//|------------> JF_DownloadFile 类 <-------------------|
//-------------------------------------------------------
class JF_DownloadFile final
{
    JF_DownloadFile(LONG loginID, int companyType, JF_RecordFileInfo &struFileInfo, char* strSaveFileName, int streamType_k=0, JF_K_DownloadFileArgs *args_k=NULL);
    JF_DownloadFile(LONG loginID, int companyType, EM_REAL_DATA_TYPE nSCtype,  LONG lChannel,char* strSaveFileName, JF_Timer *stime, JF_Timer *etime, int streamType=0, int fileType_k=0, JF_K_DownloadFileArgs *args_k=NULL);
public:
    //停止录像下载
    virtual ~JF_DownloadFile();
    bool Release();
    //获取下载进度
    int JF_GetDownloadPos();

    bool getFlag()
    {
        if(HK == m_CompanyType)
        {
            if(NULL == m_pDlv)
                return false;
        }
        else if (DH == m_CompanyType)
        {
            if(NULL == m_pDlk)
                return false;
        }
        return true;
    }

    friend JF_DownloadFile* JF_Users::JF_DownloadFileByName(JF_RecordFileInfo &struFileInfo, char* strSaveFileName, int streamType_k, JF_K_DownloadFileArgs *args_k);
    friend JF_DownloadFile* JF_Users::JF_DownloadFileByTime(LONG lChannel, char* strSaveFileName, JF_Timer *stime, JF_Timer *etime, int streamType, int fileType_k, JF_K_DownloadFileArgs *args_k);
private:
    const   LONG    m_lLoginID;
    const   int     m_CompanyType;
    LONG    m_lhDownloadFile;
    JF_DownloadFile_K *m_pDlk;
    JF_DownloadFile_V *m_pDlv;

    JF_SDK_K *m_pk;
    JF_SDK_V *m_pv;
};

//-------------------------------------------------------
//|------------>   JF_PlayBack 类   <-------------------|
//-------------------------------------------------------
class JF_PlayBack final
{
    JF_PlayBack(LONG loginID, int companyType, JF_RecordFileInfo &struFileInfo, HWND hWnd=NULL, fDataCallBacks cbPlaybackData=NULL, long pUserData=NULL, fPlayBackPosCallBack_k cbPlaybackPos_k=NULL, long dwPosUserData_k=0);
    JF_PlayBack(LONG loginID, int companyType, LONG lChannel,EM_REAL_DATA_TYPE nSCtype, JF_Timer *stime, JF_Timer *etime, HWND hWnd=NULL, BYTE byStreamType_v=0, fDataCallBacks cbPlayData=NULL, long pUserData=NULL, fPlayBackPosCallBack_k cbPlaybackPos_k=NULL, long dwPosUserData_k=0);
public:
    //停止录像回放
    bool Release(); 
    virtual ~JF_PlayBack();
    //获取/更改播放进度
    bool JF_SGPlayBackPos(unsigned int* nPos, BOOL SGetPos_v=TRUE);
    //快放
    bool JF_FastPlayBack();
    //慢放
    bool JF_SlowPlayBack();
    //暂停/继续
    bool JF_PausePlayBack();
    //恢复正常播放
    bool JF_NormalPlayBack();

    bool getFlag()
    {
        if(HK == m_CompanyType)
        {
            if(NULL == m_pPbv)
                return false;
        }
        else if (DH == m_CompanyType)
        {
            if(NULL == m_pPbk)
                return false;
        }
        return true;
    }

    friend JF_PlayBack* JF_Users::JF_PlaybackByName(JF_RecordFileInfo &struFileInfo, HWND hWnd, fDataCallBacks cbPlaybackData, long pUserData, fPlayBackPosCallBack_k cbPlaybackPos_k, long dwPosUserData_k);
    friend JF_PlayBack* JF_Users::JF_PlaybackByTime(LONG lChannel, JF_Timer *stime, JF_Timer *etime, HWND hWnd, BYTE byStreamType_v, fDataCallBacks cbPlayData, long pUserData, fPlayBackPosCallBack_k cbPlaybackPos_k, long dwPosUserData_k);
private:
    const   LONG    m_lLoginID;
    const   int     m_CompanyType;
    LONG    m_lhPlayBack;
    JF_PlayBack_K *m_pPbk;
    JF_PlayBack_V *m_pPbv;

    JF_SDK_K *m_pk;
    JF_SDK_V *m_pv;
};

//-------------------------------------------------------
//|------------>   JF_VoiceCom 类   <-------------------|
//-------------------------------------------------------
class JF_VoiceCom final
{
    JF_VoiceCom(LONG loginID, int companyType, fVoiceDataCallBack cbVoiceData=NULL, long dwUser=0, BOOL CSMode_k=TRUE);
public:
    //停止语音对讲
    bool Release(); 
    virtual ~JF_VoiceCom();
    //调节音量
    bool JF_SetVolume(unsigned int nVolume=0);

    bool getFlag()
    {
        if(HK == m_CompanyType)
        {
            if(NULL == m_pVcv)
                return false;
        }
        else if (DH == m_CompanyType)
        {
            if(NULL == m_pVck)
                return false;
        }
        return true;
    }

    friend JF_VoiceCom* JF_Users::JF_Voicecom(fVoiceDataCallBack cbVoiceData, long dwUser, BOOL CSMode_k);
private:
    const   LONG    m_lLoginID;
    const   int     m_CompanyType;
    LONG    m_lhVoiceCom;
    JF_VoiceCom_K *m_pVck;
    JF_VoiceCom_V *m_pVcv;

    JF_SDK_K *m_pk;
    JF_SDK_V *m_pv;
};

//-------------------------------------------------------
//|------------>   JF_RealPlay 类   <-------------------|
//-------------------------------------------------------
class JF_RealPlay final
{//playType_k:大华设备参数多画面参数
    JF_RealPlay(LLONG loginID, int companyType, LONG lChannel, unsigned int playType_k=1, JF_V_RealPlayArgs *args_v=NULL, HWND hWnd=NULL, fRealDataCallBackEx cbRealPlayData=NULL, long pUserData=NULL,fRealDataCallBack fcallback=NULL);
public:
    //停止实时预览
    bool Release(); 
    virtual ~JF_RealPlay();
    //设置丢帧数
    bool JF_ThrowBFrame(DWORD dwNum_v=0);
    //设置图像效果
    bool JF_SetVideoEffect(JF_VideoEffect* struVidEff, LONG lChannel_v=-1);
    //获取图像效果
    bool JF_GetVideoEffect(JF_VideoEffect* struVidEff, LONG lChannel_v=-1);
    //打开声音
    bool JF_OpenSound();
    //关闭声音
    bool JF_CloseSound();
    //调节音量
    bool JF_SetVolume(unsigned int nVolume=0);
    //保存实时数据
    bool JF_SaveRealData(char *strFileName);
    //停止实时数据保存
    bool JF_StopSaveRealData();
    //实时预览抓图
    bool JF_CapturePicture(char *strPictureName, unsigned int picFormat=0, DWORD dwBufSize_v=0,fSnapRev OnSnapRevMessage=NULL);

    bool getFlag()
    {
        if(HK == m_CompanyType)
        {
            if(NULL == m_pRpv)
                return false;
        }
        else if (DH == m_CompanyType)
        {
            if(NULL == m_pRpk)
                return false;
        }
        return true;
    }
    static   fRealDataCallBackEx   InClassBcak;
    friend JF_RealPlay* JF_Users::JF_Realplay(LONG lChannel,long pUserData, unsigned int playType_k, JF_V_RealPlayArgs *args_v, HWND hWnd, fRealDataCallBackEx cbRealPlayData, fRealDataCallBack fcallback,JF_Users *user);
private:
    const   LLONG    m_lLoginID;
    const   int     m_CompanyType;
    LONG    m_lhRealPlay;
    JF_RealPlay_K *m_pRpk;
    JF_RealPlay_V *m_pRpv;

    JF_SDK_K *m_pk;
    JF_SDK_V *m_pv;
};
#endif // !JF_CLASS
