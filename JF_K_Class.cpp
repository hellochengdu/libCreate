#include "../include/jf_inc/JF_K_Class.h"
#include <iostream>
#include <typeinfo>
using namespace std;
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
BOOL SADPdevice_k(DEVICE_IP_SEARCH_INFO* pIpSearchInfo, fSearchDevicesCB cbSearchDevices, LDWORD dwUserData, char* szLocalIp, DWORD dwWaitTime)
{
    if(CLIENT_SearchDevicesByIPs(pIpSearchInfo,cbSearchDevices,dwUserData,szLocalIp,dwWaitTime))
    {
        return 1;
    }else
    {
        return 0;
    }
}
BOOL StopSADPdevice_k()
{
    return 1;
}
CLog* sdkLog_k=getLogInstance();
CConfig inisdk_k;
void sdkLogcout(char* ip=NULL)
{
    int error=CLIENT_GetLastError()&(0x7fffffff);
    char errorchar[100];
    char errorgood[10];
    sprintf(errorgood,"%d",error);
    if(ip==NULL)
    {
        sdkLog_k->WriteLog(inisdk_k.GetValue("Config",errorgood),sdkLog_k->DEBUG_LEVEL);
    } else{
        sprintf(errorchar,"%s%s%s",ip,":",inisdk_k.GetValue("Config",errorgood).toLocal8Bit().data());
        sdkLog_k->WriteLog(errorchar,sdkLog_k->DEBUG_LEVEL);
    }

}
//-----------------|-----------------------------|------------------------------
//-----------------|          JF_SDK_K 类        |------------------------------
//-----------------|-----------------------------|------------------------------
JF_SDK_K::JF_SDK_K(fDisConnect_k cbDisConnect, LDWORD dwUser)
{
    flag = true;
    sdkLog_k->InitModule("DVR_SDK_K");
    char s_buff[1024] = { 0 };
    int len = readlink("/proc/self/exe", s_buff, 1024);

    std::string s_ret = s_buff;

    //去掉 ----> '/程序名'
    int n_reverse_index = s_ret.rfind("/");
    if (n_reverse_index == -1)//未查找成功
    {
        sdkLog_k->WriteLog("错误输出文件无");
        strcpy(s_buff,"./dh_error.ini");
    }
    //截断
    s_ret.replace(n_reverse_index, s_ret.size() - n_reverse_index, "/");
    memset(s_buff,0,1024);
    sprintf(s_buff,"%s%s",s_ret.data(),"dh_error.ini");
    inisdk_k.InitMoudle(s_buff);
    LOG_SET_PRINT_INFO sdklog;
    sdklog.dwSize=sizeof(LOG_SET_PRINT_INFO);
    sdklog.bSetFileNum=0;
    sdklog.bSetFilePath=0;
    sdklog.bSetFileSize=0;
    sdklog.bSetPrintStrategy=0;
    if(!CLIENT_LogOpen(&sdklog))
    {
        sdkLogcout();
    }
    if(FALSE == CLIENT_Init(cbDisConnect,dwUser))
    {
        flag = false;
        //Error_k();
        sdkLogcout();
    }
}

JF_SDK_K::~JF_SDK_K()
{
    CLIENT_Cleanup();
}
//设置断线重连的回调函数
void JF_SDK_K::SetReconnect(fHaveReConnect_k cbReconnect, LDWORD dwUser)
{
   CLIENT_SetAutoReconnect(cbReconnect,dwUser);//设置断线重连的回调函数
}
//登录设备
LLONG JF_SDK_K::JF_Login_k(char *pchDVRIP,                  //设备IP
                           WORD wDVRPort,                   //设备端口
                           char *pchUserName,               //用户名
                           char *pchPasswd,                 //用户密码
                           EM_LOGIN_SPAC_CAP_TYPE emSpecCap,//设备支持的能力
                           void *pCapParm,                  //emSpecCap的补充参数
                           NET_DEVICEINFO_Ex* pstruDevInfo,//设备信息
                           int *err)                        //返回登录错误码
{
    LLONG loginID = -1;
    loginID = CLIENT_LoginEx2(pchDVRIP,wDVRPort,pchUserName,pchPasswd,emSpecCap,pCapParm,pstruDevInfo,err);
   // cout<<loginID<<endl;
    if(0 == loginID)
    {
        //rror_k();
        sdkLogcout(pchDVRIP);
        return -1;
    }
    return loginID;
}
//登出设备
DWORD JF_SDK_K::JF_Logout_k(LLONG loginID)
{
    if(FALSE == CLIENT_Logout(loginID))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//设备在线状态检测
bool  JF_SDK_K::JF_RemoteControl_k(LLONG loginID)
{
    DWORD Status;
    int nRelen;
    if(FALSE==CLIENT_QueryDevState(loginID,DH_DEVSTATE_ONLINE,(char*)&Status,sizeof(Status),&nRelen))
    {
        //Error_k();
        sdkLogcout();
    }else
    {
        if(1==Status)
        {
            return true;
        }else
        {
            return false;
        }
    }
}
//获取错误码
int JF_SDK_K::JF_GetLastError_k()
{
    return (CLIENT_GetLastError()&(0x7fffffff));
}
//批量获取NVR上IPC通道
bool JF_SDK_K::JF_GetNewDevConfig_k(LLONG lLoginID,char* dwCommand,int lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,int* lpBytesReturned,int waittime)
{
    char *szOutBuffer = new char[1024*10];
    memset(szOutBuffer, 0, 1024*10);
    int nerror = 0;
    int nrestart = 0;
    if(CLIENT_GetNewDevConfig(lLoginID,dwCommand,lChannel,szOutBuffer,1024*10,&nerror,waittime))
    {
        if(CLIENT_ParseData(dwCommand,szOutBuffer,lpOutBuffer,dwOutBufferSize,NULL))
        {
            delete[]  szOutBuffer;
            return 1;
        }
    }
    delete[] szOutBuffer;
    sdkLogcout();
    return 0;
}
//批量设置NVR上IPC通道
bool JF_SDK_K::JF_SetNewDevConfig_k(LLONG lLoginID,char* dwCommand,int lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,int waittime)
{
    char *szOutBuffer = new char[1024*10];
    memset(szOutBuffer, 0, 1024*10);
    int nerror = 0;
    int nrestart = 0;
    if(CLIENT_PacketData(dwCommand,lpOutBuffer,dwOutBufferSize,szOutBuffer,1024*10))
    {
        if(CLIENT_SetNewDevConfig(lLoginID,dwCommand,lChannel,szOutBuffer,1024*10,&nerror,&nrestart,waittime))
        {
            delete[] szOutBuffer;
            return 1;
        }
    }
    delete[] szOutBuffer;
    sdkLogcout();
    return 0;
}
//批量删除NVR上IPC通道
bool JF_SDK_K::JF_MatrixSetCameras_k(LLONG lLoginID, const DH_IN_MATRIX_SET_CAMERAS* pInParam, DH_OUT_MATRIX_SET_CAMERAS* pOutParam, int nWaitTime )
{
    if(CLIENT_MatrixSetCameras(lLoginID,pInParam,pOutParam,nWaitTime))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//获取存储上所有的通道信息
bool JF_SDK_K::JF_QueryDevInfo_k(LLONG lLoginID ,int nQueryType,void *pInBuf , void *pOutBuf,void *pReserved,int nWaitTime)
{
    if(CLIENT_QueryDevInfo(lLoginID,nQueryType,pInBuf,pOutBuf,pReserved,nWaitTime))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//获取摄像机列表信息
bool JF_SDK_K::JF_MatrixGetCameras_k(LLONG lLoginID, const DH_IN_MATRIX_GET_CAMERAS* pInParam, DH_OUT_MATRIX_GET_CAMERAS* pOutParam, int nWaitTime)
{
    if(CLIENT_MatrixGetCameras(lLoginID,pInParam,pOutParam,nWaitTime))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//获取通道编码
bool JF_SDK_K::JF_GetNewDevConfig_k(LLONG lLoginID, char* szCommand, int nChannelID, char* szOutBuffer, DWORD dwOutBufferSize, int *error, int waittime)
{
    char* stInfo[1024*10];
    if(CLIENT_GetNewDevConfig(lLoginID,szCommand,nChannelID,(char*)stInfo,1024*10,error,waittime))
    {
        if(CLIENT_ParseData(szCommand,(char*)stInfo,szOutBuffer,dwOutBufferSize,NULL))
        {
            return 1;
        }
    }
    sdkLogcout();
    return 0;
}
//设置通道编码
bool JF_SDK_K::JF_SetNewDevConfig_k(LLONG lLoginID, char* szCommand, int nChannelID, char* szInBuffer, DWORD dwInBufferSize, int *error, int *restart, int waittime)
{
    char* stInfo[1024*10];
    if(CLIENT_PacketData(szCommand,szInBuffer,dwInBufferSize,(char*)stInfo,1024*10))
    {
        if (CLIENT_SetNewDevConfig(lLoginID, szCommand, nChannelID, (char*)stInfo, 1024*10, error, restart, waittime))
        {
            return 1;
        }
    }
    sdkLogcout();
    return 0;
}
//设置前端IPC的账号密码（用户名密码）/IP信息（IP地址与端口）以及重要参数的读取设置（编码信息、OSD信息_通道时间标签以及位置）
bool JF_SDK_K::JF_SetCameraInfo_k(JF_CameraInfo CameraInfo)
{
    return 0;
}
//设置设备用户名、密码
bool JF_SDK_K::JF_OperateUserInfoNew_k(LLONG lLoginID,int nOperateType,void *opParam,void *subParam,void* pReserved,int waittime)
{
    if(CLIENT_OperateUserInfoNew(lLoginID,nOperateType,opParam,subParam,pReserved,waittime))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//设置设备IP、端口
bool JF_SDK_K::JF_ModifyDevice_k(DEVICE_NET_INFO_EX *pDevNetInfo,DWORD dwWaitTime,int *iError,char *szLocalIp,void *reserved)
{
    if(CLIENT_ModifyDevice(pDevNetInfo,dwWaitTime,iError,szLocalIp,reserved))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//获取设备信息包括（系统信息（CPU、内存使用情况）、硬盘信息（硬盘总容量、剩余可用空间、锁定空间大小）、版本信息等）
bool JF_SDK_K::JF_QuerySystemStatus_k(LLONG lLoginID,DH_SYSTEM_STATUS *pstuStatus,int nWaitTime)
{
    if(CLIENT_QuerySystemStatus(lLoginID,pstuStatus,nWaitTime))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//NVR批量和单个通道的文件锁定信息设置
bool JF_SDK_K::JF_SetMarkFileByTime_k(LLONG lLoginID,const NET_IN_SET_MARK_FILE_BY_TIME* pInParam,NET_OUT_SET_MARK_FILE_BY_TIME* pOutParam,int nWaitTime)
{
    if(CLIENT_SetMarkFileByTime(lLoginID,pInParam,pOutParam,nWaitTime))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//NVR获取文件锁定的信息
bool JF_SDK_K::JF_GetMarkInfo_k(LLONG lLoginID,const NET_IN_GET_MARK_INFO* pInParam,NET_OUT_GET_MARK_INFO* pOutParam,int nWaitTime)
{
    if(CLIENT_GetMarkInfo(lLoginID,pInParam,pOutParam,nWaitTime))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//通过SDK控制存储抓取实时图片
bool JF_SDK_K::JF_SnapPictureEx_k(LLONG lLoginID,SNAP_PARAMS *par,int* reserved)
{
    if(CLIENT_SnapPictureEx(lLoginID,par,reserved))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//控制前端设备恢复出厂设置以及重启关机操作
bool JF_SDK_K::JF_ResetSystem_k(LLONG lLoginID,const NET_IN_RESET_SYSTEM* pstInParam, NET_OUT_RESET_SYSTEM* pstOutParam, int nWaitTime)
{
    if(CLIENT_ResetSystem(lLoginID,pstInParam,pstOutParam,nWaitTime))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//控制配置文件导出
bool JF_SDK_K::JF_ExportConfigFile_k(LLONG lLoginID,DH_CONFIG_FILE_TYPE emConfigFileType,char *szSavedFilePath,fDownLoadPosCallBack cbDownLoadPos,long dwUserData)
{
    if(CLIENT_ExportConfigFile(lLoginID,emConfigFileType,szSavedFilePath,cbDownLoadPos,dwUserData))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
//控制配置文件导入
bool JF_SDK_K::JF_ImportConfigFile_k(LLONG lLoginID,char *szFileName,fDownLoadPosCallBack cbUploadPos,long dwUserData,DWORD param)
{
    if(CLIENT_ImportConfigFile(lLoginID,szFileName,cbUploadPos,dwUserData,param))
    {
        return 1;
    }
    sdkLogcout();
    return 0;
}
bool JF_SDK_K::JF_GetDVRConfig_OSD_k(LLONG loginID,LONG lChannel,char* OSD_lpOutBuffer)
{
    AV_CFG_ChannelName stInfo;
    stInfo.nStructSize= sizeof(AV_CFG_ChannelName);
    char ChannelName[1024*20];
    int NumOfError;
    bool ret=CLIENT_GetNewDevConfig(loginID,CFG_CMD_CHANNELTITLE,lChannel,ChannelName,1024*20,&NumOfError,2000);
    if(ret)
    {
        if(CLIENT_ParseData(CFG_CMD_CHANNELTITLE,ChannelName,&stInfo,sizeof(AV_CFG_ChannelName),&NumOfError))
        {
            strncpy(OSD_lpOutBuffer,(const char*)stInfo.szName,128);
            return 1;
        }
    }
    sdkLogcout();
    return 0;
}
//获取字符叠加的接口
bool JF_SDK_K::JF_GetDVRConfig_CharOSD_k(LLONG loginID,LONG lChannel,char* char_lpOutBuffer)
{
    char OnScreen[1024*20];
    int NumOfError;
    AV_CFG_VideoWidget stVideo;
    stVideo.nStructSize=sizeof(AV_CFG_VideoWidget);
    bool ret =CLIENT_GetNewDevConfig(loginID,CFG_CMD_VIDEOWIDGET,lChannel,OnScreen,1024*20,&NumOfError,2000);
    if(ret)
    {
        if(CLIENT_ParseData(CFG_CMD_VIDEOWIDGET,OnScreen,&stVideo,sizeof(AV_CFG_VideoWidget),&NumOfError))
        {
            strncpy(char_lpOutBuffer,(const char*)stVideo.stuCustomTitle[0].szText,128);
            return 1;
        }
    }
    sdkLogcout();
    return 0;
}
//设置OSD接口
bool JF_SDK_K::JF_SetDVRConfig_OSD_k(LLONG loginID,LONG lChannel,char* OSD_lpInBuffer)
{
    AV_CFG_ChannelName stInfo;
    stInfo.nStructSize=sizeof(AV_CFG_ChannelName);
    char ChannelName[1024];
    int NumOfError;
    bool ret=CLIENT_GetNewDevConfig(loginID,CFG_CMD_CHANNELTITLE,lChannel,ChannelName,1024,&NumOfError,2000);
    if(ret)
    {
        if(CLIENT_ParseData(CFG_CMD_CHANNELTITLE,ChannelName,&stInfo,sizeof(AV_CFG_ChannelName),&NumOfError))
        {
            memset(stInfo.szName,0x00,256);
            memset(ChannelName,0x00,1024);
            /*QString utf=QString::fromUtf8(OSD_lpInBuffer);
            QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");       //获取UTF-8编码对象
            QString  unicode = utf8->toUnicode(utf.toLocal8Bit().data(),128);               //通过UTF-8编码对象将啊哈转为utf-16
            QTextCodec *gbk = QTextCodec::codecForName("GBK");          //获取GBK编码对象
            QByteArray arr=gbk->fromUnicode(unicode);*/
            strncpy(stInfo.szName,OSD_lpInBuffer,128);
            if(CLIENT_PacketData(CFG_CMD_CHANNELTITLE,&stInfo,sizeof(AV_CFG_ChannelName),ChannelName,1024))
            {
                if(CLIENT_SetNewDevConfig(loginID,CFG_CMD_CHANNELTITLE,lChannel,ChannelName,1024,&NumOfError,0,2000))
                {
                    return 1;
                }
            }
        }
    }
    sdkLogcout();
    return 0;
}
//设置字符叠加的接口
bool JF_SDK_K::JF_SetDVRConfig_CharOSD_k(LLONG loginID,LONG lChannel,char* char_lpInBuffer)
{

    char OnScreen[1024*20];
    int NumOfError;
    AV_CFG_VideoWidget stVideo;
    stVideo.nStructSize=sizeof(AV_CFG_VideoWidget);
    bool ret =CLIENT_GetNewDevConfig(loginID,CFG_CMD_VIDEOWIDGET,lChannel,OnScreen,1024*20,&NumOfError,5000);
    if(ret)
    {
        if(CLIENT_ParseData(CFG_CMD_VIDEOWIDGET,OnScreen,&stVideo,sizeof(AV_CFG_VideoWidget),&NumOfError))
        {
            memset(stVideo.stuCustomTitle[0].szText,0x00,128);
            /*QString utf=QString::fromUtf8(char_lpInBuffer);
            QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");       //获取UTF-8编码对象
            QString  unicode = utf8->toUnicode(utf.toLocal8Bit().data(),128);               //通过UTF-8编码对象将啊哈转为utf-16
            QTextCodec *gbk = QTextCodec::codecForName("GBK");          //获取GBK编码对象
            QByteArray arr=gbk->fromUnicode(unicode);*/
            strncpy(stVideo.stuCustomTitle[0].szText,char_lpInBuffer,128);
            stVideo.stuCustomTitle[0].bEncodeBlend=1;
            stVideo.stuCustomTitle[0].bPreviewBlend=1;
            stVideo.stuChannelTitle.stuRect.nLeft=0;
            stVideo.stuChannelTitle.stuRect.nRight=0;
            stVideo.stuChannelTitle.stuRect.nTop=7700;
            stVideo.stuChannelTitle.stuRect.nBottom=7700;

            stVideo.stuCustomTitle[0].stuRect.nLeft=3100;
            stVideo.stuCustomTitle[0].stuRect.nRight =3100;
            stVideo.stuCustomTitle[0].stuRect.nTop=7700;
            stVideo.stuCustomTitle[0].stuRect.nBottom=7700;
            if(CLIENT_PacketData(CFG_CMD_VIDEOWIDGET,&stVideo,sizeof(AV_CFG_VideoWidget),OnScreen,1024*20))
            {
                if(CLIENT_SetNewDevConfig(loginID,CFG_CMD_VIDEOWIDGET,lChannel,OnScreen,1024*20,&NumOfError,0,2000))
                {
                    return 1;
                }
            }
        }
    }
    sdkLogcout();
    return 0;
}
//获取软件版本
bool JF_SDK_K::JF_GetSoftwareVersion_k(LLONG lLoginID, const NET_IN_GET_SOFTWAREVERSION_INFO* pstInParam, NET_OUT_GET_SOFTWAREVERSION_INFO* pstOutParam, int nWaitTime)
{
    if(CLIENT_GetSoftwareVersion(lLoginID,pstInParam,pstOutParam,nWaitTime))
    {
        return true;
    }
    sdkLogcout();
    return 0;
}
//云台基本操作
DWORD JF_SDK_K::JF_PTZControl_k(LLONG   loginID,        //函数登陆返回值
                                int   nChannelID,       //通道号
                                DWORD dwPTZCommand,     //球机控制命令，param2相当于参数dwStep
                                LONG  lParam1,          //参数1
                                LONG  lParam2,          //参数2
                                LONG  lParam3,          //参数3
                                BOOL  dwStop,           //是否停止，TRUE表示停止
                                void* param4)           //支持扩展控制命令参数
{
    if(FALSE == CLIENT_DHPTZControlEx2(loginID,nChannelID,dwPTZCommand,lParam1,lParam2,lParam3,dwStop,param4))//扩展云台控制命令
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//获取设备配置
DWORD JF_SDK_K::JF_GetDVRConfig_k(LLONG    loginID,         //登陆函数返回值
                                  DWORD    dwCommand,       //设备配置命令参见配置命令 
                                  LONG     lChannel,        //通道号，如果获取全部通道数据为0xFFFFFFFF，如果命令不需要通道号，该参数无效
                                  void*    lpOutBuffer,     //接受数据缓冲指针
                                  DWORD    dwOutBufferSize, //接收数据缓冲长度(以字节为单位) 
                                  LPDWORD  lpBytesReturned, //实际收到数据的长度
                                  int      waittime)        //等待超时时间 
{
    if(FALSE == CLIENT_GetDevConfig(loginID,dwCommand,lChannel,lpOutBuffer,dwOutBufferSize,lpBytesReturned,waittime))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//配置设备
DWORD JF_SDK_K::JF_SetDVRConfig_k(LLONG   loginID,
                                 DWORD  dwCommand,          //设备配置命令参见配置命令
                                 LONG    lChannel,           //通道号
                                 LPVOID  lpInBuffer,         //数据输入缓冲指针
                                 DWORD   dwInBufferSize,     //数据缓冲长度(以字节为单位) 
                                 int     waittime)           //等待时间
{
    if(FALSE == CLIENT_SetDevConfig(loginID,dwCommand,lChannel,lpInBuffer,dwInBufferSize,waittime))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//设置查找文件时间
void g_cbFindFile(DWORD dwChannel, DWORD dwFileNum, char* lpFileInfo, fFindFileLog cbFindFile,LDWORD userData)
{
    char *chpFileInfo = new char[10*sizeof(JF_FileInfo)]();
    for(int i=0; i<dwFileNum; ++i)
    {
        NET_RECORDFILE_INFO fileInfo = {0};
        JF_FileInfo file = {0};
        char *pNowIn = lpFileInfo+i*sizeof(fileInfo);

        memcpy(&fileInfo,pNowIn,sizeof(fileInfo));
        strcpy(file.FileName, fileInfo.filename);
        file.StartTime.tm_year= fileInfo.starttime.dwYear;
        file.StartTime.tm_mon = fileInfo.starttime.dwMonth;
        file.StartTime.tm_mday= fileInfo.starttime.dwDay;
        file.StartTime.tm_hour= fileInfo.starttime.dwHour;
        file.StartTime.tm_min = fileInfo.starttime.dwMinute;
        file.StartTime.tm_sec = fileInfo.starttime.dwSecond;
        struct tm stm = {0};
        stm.tm_year= fileInfo.starttime.dwYear;
        stm.tm_mon = fileInfo.starttime.dwMonth;
        stm.tm_mday= fileInfo.starttime.dwDay;
        stm.tm_hour= fileInfo.starttime.dwHour;
        stm.tm_min = fileInfo.starttime.dwMinute;
        stm.tm_sec = fileInfo.starttime.dwSecond;
        time_t s = mktime(&stm);
        struct tm etm = {0};
        etm.tm_year= fileInfo.endtime.dwYear;
        etm.tm_mon = fileInfo.endtime.dwMonth;
        etm.tm_mday= fileInfo.endtime.dwDay;
        etm.tm_hour= fileInfo.endtime.dwHour;
        etm.tm_min = fileInfo.endtime.dwMinute;
        etm.tm_sec = fileInfo.endtime.dwSecond;
        time_t e = mktime(&etm);
        file.TimeLen = e - s;
        file.FileLen = fileInfo.size;

        char *pNowOut = chpFileInfo+i*sizeof(file);
        memcpy(pNowOut,&file,sizeof(file));
    }
    cbFindFile(dwChannel,dwFileNum,chpFileInfo,userData);
    delete[] chpFileInfo;
    chpFileInfo = NULL;
}
//查找文件
DWORD JF_SDK_K::JF_FindFile_k(LLONG   loginID,
                              int    nChannelID,         //通道号
                              int    nRecordFileType,    //文件类型
                              JF_Timer *stime,          //起始时间戳
                              JF_Timer *etime,          //截止时间戳
                              fFindFileLog cbFindFile,   //超时时间
                              long userData,
                              LLONG *plhFindFile,
                              sem_t *pSem)
{
    NET_TIME stm = {0};
    stm.dwYear    = stime->tm_year;
    stm.dwMonth   = stime->tm_mon;
    stm.dwDay     = stime->tm_mday;
    stm.dwHour    = stime->tm_hour;
    stm.dwMinute  = stime->tm_min;
    stm.dwSecond  = stime->tm_sec;

    NET_TIME etm = {0};
    etm.dwYear    = etime->tm_year;
    etm.dwMonth   = etime->tm_mon;
    etm.dwDay     = etime->tm_mday;
    etm.dwHour    = etime->tm_hour;
    etm.dwMinute  = etime->tm_min;
    etm.dwSecond  = etime->tm_sec;
    //打开录像查询句柄             登陆函数的返回值，通道号，       录像文件的类型，卡号，录像开始的时间，录像结束的时间，是否按时间查询，查询超时时间
    LLONG lhFindFile = CLIENT_FindFile(loginID,nChannelID,nRecordFileType,NULL,&stm,&etm,FALSE,5000);
    if(0 == lhFindFile)
    {
        lhFindFile = -1;
    }
    if((NULL!=plhFindFile)&&(NULL!=pSem))
    { 
        *plhFindFile = lhFindFile;  //线程时用
        sem_post(pSem); //线程时用
    }
    if(-1 == lhFindFile)
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }

    NET_RECORDFILE_INFO findData;
    char* chFileInfoBuf = new char[10*sizeof(findData)]();
    char* nowFileInfoBufp = chFileInfoBuf;
    unsigned int fileNum = 0;
    bool loop = true;
    while(loop)
    {
        memset(&findData, 0, sizeof(findData));
        int ret = CLIENT_FindNextFile(lhFindFile,&findData);//查询下一条录像文件记录
        switch (ret)
        {
        case 1: //查询到一个文件
            if(fileNum >= 10)   //判断是否会溢出
            {   //通过回调函数将查询到的文件信息输出给调用者
                g_cbFindFile(nChannelID,fileNum,chFileInfoBuf,cbFindFile,userData);
                memset(chFileInfoBuf, 0, fileNum*sizeof(findData));
                nowFileInfoBufp = chFileInfoBuf;
                fileNum = 0;
            }
            memcpy(nowFileInfoBufp,&findData,sizeof(findData));
            nowFileInfoBufp += sizeof(findData);
            ++fileNum;
            continue;
        case 0: //查询完毕
            if(fileNum > 0)
            {   //通过回调函数将查询到的文件信息输出给调用者
                g_cbFindFile(nChannelID,fileNum,chFileInfoBuf,cbFindFile,userData);
            }
            loop = false;
            break;
        case -1://出错
            //Error_k();
            sdkLogcout();
            if(chFileInfoBuf != NULL)
            {
                delete[] chFileInfoBuf;
                chFileInfoBuf = NULL;
            }
            CLIENT_FindClose(lhFindFile);//关闭录像查询的句柄
            return -1;
        }
    }
    if(chFileInfoBuf != NULL)
    {
        delete[] chFileInfoBuf;
        chFileInfoBuf = NULL;
    }
    return lhFindFile;
}
//停止文件查找
DWORD JF_SDK_K::JF_StopFindFile_k(LLONG lFindHandle)
{
    if(0 >= lFindHandle)
    {
        sdkLog_k->WriteLog("findFile handle error");
        //error_k("findFile handle error");
        return -1;
    }
    if(FALSE == CLIENT_FindClose(lFindHandle))//关闭文件查找句柄
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//查找日志
DWORD JF_SDK_K::JF_FindLog_k(LLONG  loginID,
                             int    logType,        //日志类型
                             char   *pLogBuffer,    //日志缓冲区
                             int    maxlen,         //日志缓冲区的最大长度(单位字节) 
                             int    *nRetLogBuflen) //返回的日志长度(单位字节)  //等待时间
{
    DH_LOG_QUERY_TYPE emLogType = DHLOG_ALL;
    switch(logType)
    {
        case 0:
            emLogType = DHLOG_ALL;
            break;
        case 1:
            emLogType = DHLOG_SYSTEM;
            break;
        case 2:
            emLogType = DHLOG_CONFIG;
            break;
        case 3:
            emLogType = DHLOG_STORAGE;
            break;
        case 4:
            emLogType = DHLOG_ALARM;
            break;
        case 5:
            emLogType = DHLOG_RECORD;
            break;
        case 6:
            emLogType = DHLOG_ACCOUNT;
            break;
        case 7:
            emLogType = DHLOG_CLEAR;
            break;
        case 8:
            emLogType = DHLOG_PLAYBACK;
            break;
        case 9:
            emLogType = DHLOG_MANAGER;
            break;
    }
    //查询日志的扩展接口        登陆函数返回值，日志类型， 日志缓冲区，    日志缓冲区的最大长度，返回的日志长度，
    if(FALSE == CLIENT_QueryLogEx(loginID,emLogType,(char*)pLogBuffer,maxlen,nRetLogBuflen,NULL))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//开启监听                         登陆函数返回值，端口，IP，                     消息回调函数，                用户自定义数据，         服务器的消息回调函数，服务器维护连接的超时时间，用户自定义数据             
DWORD JF_SDK_K::JF_StartListen_k(LLONG loginID, WORD wPort, char* strHostIP, fMSGCallBack_k cbMessage, long dwMsgUserData, fServiceCallBack_k cbService, long dwServiceUserData)
{
    CLIENT_SetDVRMessCallBack(cbMessage, dwMsgUserData);//设置设备消息回调函数, 用来得到设备当前状态信息，与调用顺序无关，SDK默认不回调，此回调函数必须先调用报警消息订阅接口 CLIENT_StartListen 或 CLIENT_StartListenEx 才有效
    if(FALSE == CLIENT_StartListenEx(loginID))//订阅设备消息，得到的消息从CLIENT_SetDVRMessCallBack的设置值回调出来
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    if(NULL != cbService)
    {
        LLONG lhService = -1;
        //报警上传功能，启动服务，目前只实现了报警监听功能 参数：监听的端口，IP，服务器的消息回调函数，服务器维护连接的超时时间，用户自定义数据
        lhService = CLIENT_StartService(wPort,strHostIP,cbService,0,dwServiceUserData);
        if(0 == lhService)
        {
            //Error_k();
            sdkLogcout();
            return -1;
        }
        return lhService;
    }
    
    return 0;
}
//停止报警监听
DWORD JF_SDK_K::JF_StopListen_k(LLONG loginID, LLONG lhService)
{
    if(FALSE == CLIENT_StopListen(loginID))//停止对某个设备订阅报警
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    if(0 == lhService)
    {
        return 0;
    }
    if(FALSE == CLIENT_StopService(lhService))//停止端口监听服务 参数：要关闭的服务器的句柄，是CLIENT_StartService的返回值 
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//重启设备
DWORD JF_SDK_K::JF_RebootDVR_k(LLONG loginID)
{
    if(FALSE == CLIENT_RebootDev(loginID))//重启前端设备
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//关闭设备
DWORD JF_SDK_K::JF_ShutDownDVR_k(LLONG loginID)
{
    if(FALSE == CLIENT_ShutDownDev(loginID))//关闭前端设备
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//查询设备状态
DWORD JF_SDK_K::JF_QueryDVRState_k(LLONG   loginID,
                                   int    nType,       //查询信息类型
                                   char   *pBuf,       //接收查询返回的数据的缓存
                                   int    nBufLen,     //缓存长度，单位字节
                                   int    *pRetLen)    //返回的数据长度，单位字节
{
    //获取前端设备的当前工作状态，参数：登陆函数返回值，查询信息类型，输出参数，缓冲长度，输出参数
    if(FALSE == CLIENT_QueryDevState(loginID,nType,pBuf,nBufLen,pRetLen))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//设置设备时间
DWORD JF_SDK_K::JF_SetDVRDateTime_k(LLONG  loginID,
                                    DWORD year,     //年
                                    DWORD mon,      //月
                                    DWORD day,      //日
                                    DWORD hour,     //时
                                    DWORD min,      //分
                                    DWORD sec)      //秒
{
    NET_TIME time = {0};
    time.dwYear     = year;
    time.dwMonth    = mon;
    time.dwDay      = day;
    time.dwHour     = hour;
    time.dwMinute   = min;
    time.dwSecond   = sec;
    //设置设备的配置时间 参数：登陆函数返回值，设备配置命令：设置DVR时间，通道号，输入缓冲指针，输入缓冲长度
    if(FALSE == CLIENT_SetDevConfig(loginID,DH_DEV_TIMECFG,-1,&time,sizeof(time)))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}

    //  按文件名下载  
JF_DownloadFile_K* JF_SDK_K::JF_DownloadFile_k(LLONG   loginID,
                                                NET_RECORDFILE_INFO &struFileInfo, //文件信息
                                                char* strSaveFileName,                    //保存文件名
                                                int streamType,                         //码流类型
                                                fDownLoadPosCallBack_k cbFileDownloadPos, //文件下载进度回调函数
                                                LDWORD dwPosUserData,
                                                fDataCallBack_k cbFileDownloadData,    //数据回调函数
                                                LDWORD dwUserData)                  //用户自定义回调数据
{
    JF_DownloadFile_K* tmp = new JF_DownloadFile_K(loginID,struFileInfo,strSaveFileName,
                            streamType,cbFileDownloadPos,dwPosUserData,cbFileDownloadData,dwUserData);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }
    return tmp;
}
    //  按时间下载
JF_DownloadFile_K* JF_SDK_K::JF_DownloadFile_k(LLONG   loginID,EM_REAL_DATA_TYPE nSCtype,
                      int chan,                             //通道号
                      JF_Timer *stime,                          //起始时间
                      JF_Timer *etime,                          //截止时间
                      char* strSaveFileName,                //保存文件名
                      int nRecordFileType,                  //文件类型
                      int streamType,                     //码流类型 
                      fDownLoadPosCallBack_k cbFileDownloadPos, //文件下载进度回调函数
                      LDWORD dwPosUserData,
                      fDataCallBack_k cbFileDownloadData,//数据回调函数
                      LDWORD dwUserData)              //用户自定义回调数据
{
    JF_DownloadFile_K* tmp = new JF_DownloadFile_K(loginID,nSCtype,chan,stime,etime,strSaveFileName,
                            nRecordFileType,streamType,cbFileDownloadPos,dwPosUserData,cbFileDownloadData,dwUserData);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }              
    return tmp;              
}                      

    //  按文件名回放录像
JF_PlayBack_K* JF_SDK_K::JF_PlayBack_k(LLONG   loginID,
                  NET_RECORDFILE_INFO &struFileInfo,  //文件信息
                  HWND                       hWnd,      //窗口句柄
                  fDownLoadPosCallBack_k       cbDownloadPos,  //进度回调函数
                  long                     dwPosUserData,
                  fDataCallBack_k              cbPlayBack,//回放数据回调
                  long                     dwUserData)//用户自定义数据
{
    JF_PlayBack_K* tmp = new JF_PlayBack_K(loginID,struFileInfo,hWnd,
                        cbDownloadPos,dwPosUserData,cbPlayBack,dwUserData);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }              
    return tmp;          
}                  
	//  按时间回放录像
JF_PlayBack_K* JF_SDK_K::JF_PlayBack_k(LLONG   loginID,EM_REAL_DATA_TYPE nSCtype,
                int         nChannelID,         //通道号
                JF_Timer *   stime,              //起始时间
                JF_Timer *   etime,              //截止时间
                HWND          hWnd,          //窗口句柄
                fDownLoadPosCallBack_k cbDownloadPos,    //进度回调函数
                long        dwPosUserData,
                fDataCallBack_k cbPlayBack,    //数据回调函数
                long        dwUserData)   //用户自定义回调数据
{
    JF_PlayBack_K* tmp = new JF_PlayBack_K(loginID,nSCtype,nChannelID,stime,etime,hWnd,
                                        cbDownloadPos,dwPosUserData,cbPlayBack,dwUserData);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }              
    return tmp;                              
}                

    //  开启语音对讲
JF_VoiceCom_K* JF_SDK_K::JF_VoiceCom_k(LLONG loginID, BOOL Record, BOOL CSMode, fVoiceDataCallBack cbAudioData, long dwUser)
{
    JF_VoiceCom_K* tmp = new JF_VoiceCom_K(loginID,Record,CSMode,cbAudioData,dwUser);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }
    return tmp;
}

    //开启实时预览
JF_RealPlay_K* JF_SDK_K::JF_RealPlay_k(LLONG loginID, int nChannelID, HWND hWnd, unsigned int playType,unsigned  int  replayType, fRealDataCallBackEx cbRealPlayData,long dwUser,fRealDataCallBack fcallback)
{
    //cout<<loginID<<endl;
    JF_RealPlay_K* tmp = new JF_RealPlay_K(loginID,nChannelID,hWnd,playType,replayType,cbRealPlayData,dwUser,fcallback);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }
    return tmp;
}

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_DownloadFile_K 类   |------------------------------
//-----------------|-----------------------------|------------------------------
JF_DownloadFile_K::JF_DownloadFile_K(LLONG loginID,
                                    NET_RECORDFILE_INFO &struFileInfo, //文件信息
                                    char* strSaveFileName,                    //保存文件名
                                    int streamType,                         //码流类型 
                                    fDownLoadPosCallBack_k cbFileDownloadPos, //文件下载进度回调函数
                                    long dwPosUserData,
                                    fDataCallBack_k cbFileDownloadData,    //数据回调函数
                                    long dwUserData)                  //用户自定义回调数据
                                    :m_lLoginID(loginID)
{
    flag = true;
    m_lhDownloadFile = -1;
    //设置设备语音对讲、回放、权限等工作模式 参数：登陆函数返回值，枚举值：设置待查询及按时间回放的录像码流类型(0-主辅码流,1-主码流,2-辅码流)，对应结构体值
    if(FALSE == CLIENT_SetDeviceMode(m_lLoginID,DH_RECORD_STREAM_TYPE,&streamType))
    {
        flag = false;
        //Error_k();
        sdkLogcout();
    }
    //下载录像文件--扩展 // sSavedFileName不为空, 录像数据写入到该路径对应的文件; fDownLoadDataCallBack不为空, 录像数据通过回调函数返回
    //参数：                            登陆函数的返回值，录像文件信息指针，要保存的录像文件名 全路径，下载进度回调函数，下载进度回调用户自定义数据，数据回调函数，下载数据回调用户自定义数据，
    m_lhDownloadFile = CLIENT_DownloadByRecordFileEx(m_lLoginID,&struFileInfo,strSaveFileName,cbFileDownloadPos,dwPosUserData,cbFileDownloadData,dwUserData);
    if(0 == m_lhDownloadFile)
    {
        flag = false;
        m_lhDownloadFile = -1;
        //Error_k();
        sdkLogcout();
    }
}

JF_DownloadFile_K::JF_DownloadFile_K(LLONG loginID,EM_REAL_DATA_TYPE nSCtype,
                                    int chan,                        //通道号
                                    JF_Timer *stime,                      //起始时间
                                    JF_Timer *etime,                      //截止时间
                                    char* strSaveFileName,            //保存文件名
                                    int nRecordFileType,              //文件类型
                                    int streamType,                  //码流类型
                                    fDownLoadPosCallBack_k cbFileDownloadPos, //文件下载进度回调函数
                                    long dwPosUserData,
                                    fDataCallBack_k cbFileDownloadData,//数据回调函数
                                    long dwUserData)              //用户自定义回调数据
                                    :m_lLoginID(loginID)
{
    flag = true;
    //设置设备语音对讲、回放、权限等工作模式 参数：登入句柄，枚举值：设置待查询及按时间回放的录像码流类型(0-主辅码流,1-主码流,2-辅码流)
    if(FALSE == CLIENT_SetDeviceMode(m_lLoginID,DH_RECORD_STREAM_TYPE,&streamType))
    {
        flag = false;
        //Error_k();
        sdkLogcout();
    }
    else
    {
        NET_TIME stm = {0};
        stm.dwYear  = stime->tm_year;
        stm.dwMonth = stime->tm_mon;
        stm.dwDay   = stime->tm_mday;
        stm.dwHour  = stime->tm_hour;
        stm.dwMinute= stime->tm_min;
        stm.dwSecond= stime->tm_sec;

        NET_TIME etm = {0};
        etm.dwYear  = etime->tm_year;
        etm.dwMonth = etime->tm_mon;
        etm.dwDay   = etime->tm_mday;
        etm.dwHour  = etime->tm_hour;
        etm.dwMinute= etime->tm_min;
        etm.dwSecond= etime->tm_sec;
        NET_IN_DOWNLOAD_BY_DATA_TYPE stIn={sizeof(stIn)};
        NET_OUT_DOWNLOAD_BY_DATA_TYPE stOut={sizeof(stOut)};
        stIn.dwSize=sizeof(NET_IN_DOWNLOAD_BY_DATA_TYPE);
        stOut.dwSize=sizeof(NET_OUT_DOWNLOAD_BY_DATA_TYPE);
        stIn.fDownLoadDataCallBack=cbFileDownloadData;
        stIn.emDataType=nSCtype;
        stIn.nChannelID=chan;
        stIn.dwDataUser=(DWORD)dwUserData;
        stIn.stStartTime=stm;
        stIn.stStopTime=etm;
	  stIn.szSavedFileName=strSaveFileName;
        m_lhDownloadFile=CLIENT_DownloadByDataType(loginID,&stIn,&stOut,5000);
        //按时间下载，直接输入指定通道起始时间和结束时间下载放录像 
        //参数：登入函数的返回值，通道号，录像文件类型，开始时间，结束时间，要保存的录像文件名 全路径，载进度回调函数，下载进度回调用户自定义数据，数据回调函数，下载数据回调用户自定义数据，
        //m_lhDownloadFile = CLIENT_DownloadByTimeEx(m_lLoginID,chan,nRecordFileType,&stm,&etm,strSaveFileName,(fTimeDownLoadPosCallBack)cbFileDownloadPos,dwPosUserData,cbFileDownloadData,dwUserData);
        if(0 == m_lhDownloadFile)
        {
            flag = false;
            m_lhDownloadFile = -1;
            //Error_k();
            sdkLogcout();
        }
    }
}
//停止下载录像文件
bool JF_DownloadFile_K::Release()
{
    if(-1 != m_lhDownloadFile)
    {
        if(FALSE == CLIENT_StopDownload(m_lhDownloadFile))//停止下载录像文件 参数：CLIENT_DownloadByRecordFile的返回值 
        {
           //Error_k();
            sdkLogcout();
            return false;
        }
        return true;
    }
    return false;
}

JF_DownloadFile_K::~JF_DownloadFile_K()
{}
//获取下载进度
DWORD JF_DownloadFile_K::JF_GetDownloadPos_k(int *nDownloadSize, int *nTotalSize)
{
    //获得下载录像的当前位置，可以用于不需要实时显示下载进度的接口，与下载回调函数的功能类似
    //参数：CLIENT_DownloadByRecordFile的返回值 ，下载的总长度，已下载的长度
    if(FALSE == CLIENT_GetDownloadPos(m_lhDownloadFile,nTotalSize,nDownloadSize))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return *nDownloadSize;
}

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_PlayBack_K 类       |------------------------------
//-----------------|-----------------------------|------------------------------
JF_PlayBack_K::JF_PlayBack_K(LLONG loginID,
                            NET_RECORDFILE_INFO &struFileInfo,  //文件信息
                            HWND                       hWnd,      //窗口句柄
                            fDownLoadPosCallBack_k       cbDownloadPos,    //进度回调函数
                            long                     dwPosUserData,
                            fDataCallBack_k              cbPlayBack,//回放数据回调
                            long                     dwUserData)//用户自定义数据
                            :m_lLoginID(loginID),
                            m_bMethod(true),
                            m_nFileSize(struFileInfo.size),
                            m_nFileTime(-1)
{
    flag = true;
    m_bPause     = FALSE;
    m_lhPlayBack = -1;
    //带数据回调的按录像文件回放扩展接口，每通道同一时间只能播放一则录像，不能同时播放同一通道的多条记录
    //参数：                           CLIENT_LoginEx2的返回值，录像文件信息，回放窗口，进度回调函数，进度回调函数用户参数，数据回调函数，数据回调用户
    m_lhPlayBack = CLIENT_PlayBackByRecordFileEx(m_lLoginID,&struFileInfo,hWnd,cbDownloadPos,dwPosUserData,cbPlayBack,dwUserData);
    if(0 == m_lhPlayBack)
    {
        flag = false;
        m_lhPlayBack = -1;
        //Error_k();
        sdkLogcout();
    }
}

JF_PlayBack_K::JF_PlayBack_K(LLONG loginID,EM_REAL_DATA_TYPE nSCtype,
                            int          nChannelID,    //通道号
                            JF_Timer *  stime,         //起始时间
                            JF_Timer *  etime,         //截止时间
                            HWND          hWnd,          //窗口句柄
                            fDownLoadPosCallBack_k cbDownloadPos,    //进度回调函数
                            long        dwPosUserData,
                            fDataCallBack_k cbPlayBack,    //数据回调函数
                            long        dwUserData)    //用户自定义回调数据
                            :m_lLoginID(loginID),
                            m_bMethod(false),
                            m_nFileTime(etime-stime),
                            m_nFileSize(-1)
{
    tm startTime={0};
    startTime.tm_year=etime->tm_year-1900;
    startTime.tm_mon=etime->tm_mon-1;
    startTime.tm_mday=etime->tm_mday;
    startTime.tm_hour=etime->tm_hour;
    startTime.tm_min=etime->tm_min;
    startTime.tm_sec=etime->tm_sec;
    tm stopTime={0};
    stopTime.tm_year=stime->tm_year-1900;
    stopTime.tm_mon=stime->tm_mon-1;
    stopTime.tm_mday=stime->tm_mday;
    stopTime.tm_hour=stime->tm_hour;
    stopTime.tm_min=stime->tm_min;
    stopTime.tm_sec=stime->tm_sec;
    int SecStop=mktime(&startTime);
    int SecStart=mktime(&stopTime);
    m_nFileTime=SecStop-SecStart;
    flag = true;
    m_bPause     = FALSE;

    NET_TIME stm = {0};
    stm.dwYear  = stime->tm_year;
    stm.dwMonth = stime->tm_mon;
    stm.dwDay   = stime->tm_mday;
    stm.dwHour  = stime->tm_hour;
    stm.dwMinute= stime->tm_min;
    stm.dwSecond= stime->tm_sec;
    NET_TIME etm = {0};
    etm.dwYear  = etime->tm_year;
    etm.dwMonth = etime->tm_mon;
    etm.dwDay   = etime->tm_mday;
    etm.dwHour  = etime->tm_hour;
    etm.dwMinute= etime->tm_min;
    etm.dwSecond= etime->tm_sec;
    NET_IN_PLAYBACK_BY_DATA_TYPE stIn={sizeof(stIn)};
    NET_OUT_PLAYBACK_BY_DATA_TYPE stOut={sizeof(stOut)};
    stIn.dwSize=sizeof(NET_IN_PLAYBACK_BY_DATA_TYPE);
    stOut.dwSize=sizeof(NET_OUT_PLAYBACK_BY_DATA_TYPE);
    stIn.fDownLoadDataCallBack=cbPlayBack;
    stIn.emDataType=nSCtype;
    stIn.nChannelID=nChannelID;
    stIn.dwDataUser=dwUserData;
    stIn.stStartTime=stm;
    stIn.stStopTime=etm;
    m_lhPlayBack=CLIENT_PlayBackByDataType(loginID,&stIn,&stOut,5000);
    //m_lhPlayBack = CLIENT_PlayBackByTimeEx(m_lLoginID,nChannelID,&stm,&etm,hWnd,cbDownloadPos,dwPosUserData,cbPlayBack,dwUserData);
    if(0 == m_lhPlayBack)
    {
        flag = false;
        m_lhPlayBack = -1;
        //Error_k();
        sdkLogcout();
    }
}
//停止回放
bool JF_PlayBack_K::Release()
{
    if(-1 != m_lhPlayBack)
    {
        if(FALSE == CLIENT_StopPlayBack(m_lhPlayBack))//停止回放 参数：回放句柄
        {
            //Error_k();
            sdkLogcout();
            return false;
        }
        else
        {
            m_lhPlayBack = -1;
            return true;
        }
    }
}

JF_PlayBack_K::~JF_PlayBack_K()
{}
//  改变回放进度
BOOL JF_PlayBack_K::JF_SeekPlayBack_k(unsigned int offset)
{
    if(0 > offset)
    {
        offset = 0;
    }
    else if(offset > 100)
    {
        offset = 100;
    }

    if(m_bMethod)
    {
        //改变位置播放,即拖动播放 参数：回放句柄，相对开始出偏移时间，向对文件开始出偏移时间，回放位置*回放文件的大小/100
        if(FALSE == CLIENT_SeekPlayBack(m_lhPlayBack,0xffffffff,offset*m_nFileSize/100))
        {
            //Error_k();
            sdkLogcout();
            return FALSE;
        }
    }
    else
    {
        ////改变位置播放,即拖动播放 参数：回放句柄，相对开始出偏移时间：回放位置*回放文件的时间长度/100，向对文件开始出偏移时间，
        if(FALSE == CLIENT_SeekPlayBack(m_lhPlayBack,offset*m_nFileTime/100,-1))
        {
            //Error_k();
            sdkLogcout();
            return FALSE;
        }
    }
    return TRUE;
}
//快放
BOOL JF_PlayBack_K::JF_FastPlayBack_k()
{
    //快放，将当前帧率提高一倍，但是不能无限制的快放，目前最大200帧，大于时返回FALSE， 有音频的话不可以快放，慢放没有问题，只有打开图像的函数参数hWnd有效时才有此限制，否则没有限制，但是需要设备支持回放速度控制
    if(FALSE == CLIENT_FastPlayBack(m_lhPlayBack))
    {
        //Error_k();
        sdkLogcout();
        return FALSE;
    }
    return TRUE;
}
//慢放
BOOL JF_PlayBack_K::JF_SlowPlayBack_k()
{
    //慢放，将当前帧率降低一倍，最慢为每秒一帧，小于1则返回FALSE，在打开图像的函数参数hWnd为0时，设备支持回放速度控制情况下，此函数才会起作用
    if(FALSE == CLIENT_SlowPlayBack(m_lhPlayBack))
    {
        //Error_k();
        sdkLogcout();
        return FALSE;
    }
    return TRUE;
}
//暂停/继续
BOOL JF_PlayBack_K::JF_PausePlayBack_k()
{
    (m_bPause)?(m_bPause=FALSE):(m_bPause=TRUE);
    if(FALSE == CLIENT_PausePlayBack(m_lhPlayBack,m_bPause))//网络回放暂停与恢复播放
    {
        //Error_k();
        sdkLogcout();
        return FALSE;
    }
    return TRUE;
}
//恢复正常播放
BOOL JF_PlayBack_K::JF_NormalPlayBack_k()
{
    if(FALSE == CLIENT_NormalPlayBack(m_lhPlayBack))//恢复正常播放速度
    {
        //Error_k();
        sdkLogcout();
        return FALSE;
    }
    m_bPause = FALSE;

    return TRUE;
}

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_VoiceCom_K 类       |------------------------------
//-----------------|-----------------------------|------------------------------
JF_VoiceCom_K::JF_VoiceCom_K(LLONG loginID, BOOL Record, BOOL CSMode, fVoiceDataCallBack cbAudioData, 
                            long dwUser):m_lLoginID(loginID),m_bRecord(Record)
{
    flag = true;
    m_lhVoiceCom = -1;
    EM_USEDEV_MODE mode;
    if(CSMode)
    {
        mode = DH_TALK_SERVER_MODE;
    }
    else
    {
        mode = DH_TALK_CLIENT_MODE;
    }
    //设置设备语音对讲、回放、权限等工作模式 参数：登入句柄，枚举值，
    if(FALSE == CLIENT_SetDeviceMode(m_lLoginID,mode,NULL))
    {
        flag = false;
        //Error_k();
        sdkLogcout();
    }
    DHDEV_TALKDECODE_INFO g_curTalkMode;
    g_curTalkMode.encodeType = DH_TALK_PCM;
    if(FALSE == CLIENT_SetDeviceMode(m_lLoginID,DH_TALK_ENCODE_TYPE,&g_curTalkMode))
    {
        flag = false;
        //Error_k();
        sdkLogcout();
    }
    NET_SPEAK_PARAM stuSpeak = {sizeof(stuSpeak)};
    stuSpeak.nMode=0;
    stuSpeak.nSpeakerChannel=0;
    if(FALSE == CLIENT_SetDeviceMode(m_lLoginID,DH_TALK_SPEAK_PARAM,&stuSpeak))
    {
        flag = false;
        //Error_k();
        sdkLogcout();
    }
    NET_TALK_TRANSFER_PARAM stuTransfer = {sizeof(stuTransfer)};
    stuTransfer.bTransfer = FALSE;
    if(FALSE == CLIENT_SetDeviceMode(m_lLoginID,DH_TALK_TRANSFER_MODE,&stuTransfer))
    {
        flag = false;
        //Error_k();
        sdkLogcout();
    }
    if(m_bRecord)
    {
        //启动本地录音功能，录音采集出来的音频数据通过CLIENT_StartTalkEx的回调函数回调给用户，对应操作是CLIENT_RecordStopEx
        if(FALSE == CLIENT_RecordStartEx(m_lLoginID))//CLIENT_LoginEx2的返回值 
        {
            flag = false;
            //Error_k();
            sdkLogcout();
        }
    }
    //向设备发起语音对讲请求 参数：登陆返回的设备句柄，用户自定义的数据回调接口，用户自定义信息，通过回调函数返回给用户
    m_lhVoiceCom  = CLIENT_StartTalkEx(m_lLoginID,(pfAudioDataCallBack)cbAudioData,dwUser);
    if(0 == m_lhVoiceCom)
    {
        flag = false;
        m_lhVoiceCom = -1;
        //Error_k();
        sdkLogcout();
    }
}
//停止语音对讲
bool JF_VoiceCom_K::Release()
{
    if(m_bRecord)
    {
        if(FALSE == CLIENT_RecordStopEx(m_lLoginID))//停止本地录音，对应操作是CLIENT_RecordStartEx 参数：CLIENT_LoginEx2的返回值 
        {
            //Error_k();
            sdkLogcout();
            return false;
        }
    }

    if(-1 != m_lhVoiceCom)
    {
        if(FALSE == CLIENT_StopTalkEx(m_lhVoiceCom))//停止语音对讲 参数：语音对讲句柄，CLIENT_StartTalkEx返回
        {
            //Error_k();
            sdkLogcout();
            return false;
        }
        else
        {
            m_lhVoiceCom = -1;
            return true;
        }
    }
    return false;
}

JF_VoiceCom_K::~JF_VoiceCom_K()
{}
//  开启/关闭录音
DWORD JF_VoiceCom_K::JF_RecordControl_k()
{
    if(m_bRecord)
    {
        //停止本地录音，对应操作是CLIENT_RecordStartEx
        if(FALSE == CLIENT_RecordStopEx(m_lLoginID))
        {
            //Error_k();
            sdkLogcout();
            return -1;
        }
        m_bRecord = FALSE;
    }
    else
    {
        //启动本地录音功能，录音采集出来的音频数据通过CLIENT_StartTalkEx的回调函数回调给用户，对应操作是CLIENT_RecordStopEx
        if(FALSE == CLIENT_RecordStartEx(m_lLoginID))
        {
            //Error_k();
            sdkLogcout();
            return -1;
        }
        m_bRecord = TRUE;
    }

    return 0;
}
//  发送音频数据
DWORD JF_VoiceCom_K::JF_SendTalkData_k(char *pSendBuf, DWORD dwBufSize)
{
    if(NULL == pSendBuf)
    {
        error_k("send buf is NULL");
        return -1;
    }
    //向设备发送用户的音频数据，这里的数据可以是从CLIENT_StartTalkEx的回调接口中回调出来的数据
    //参数：CLIENT_StartTalkEx的返回值，要放送的音频数据，要发送的音频数据的长度
    LONG ret = CLIENT_TalkSendData(m_lhVoiceCom,pSendBuf,dwBufSize);
    if(-1 == ret)
    {
        //Error_k();
        sdkLogcout();
    }
    return ret;
}
//调节音量
DWORD JF_VoiceCom_K::JF_SetVolume_k(unsigned int nVolume)
{
    if(0 > nVolume)
    {
        nVolume = 0;
    }
    else if(nVolume > 100)
    {
        nVolume = 100;
    }
    //设置音量,函数只有在打开hPlayHandle的函数的参数hWnd有效时才生效 
    //参数：接口返回值，音量值
    if(FALSE == CLIENT_SetVolume(m_lhVoiceCom,nVolume))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_RealPlay_K 类       |------------------------------
//-----------------|-----------------------------|------------------------------
JF_RealPlay_K::JF_RealPlay_K(LLONG loginID, int nChannelID, HWND hWnd,
                            unsigned int playType,unsigned  int  replayType, fRealDataCallBackEx cbRealPlayData,
                            long   dwUser,fRealDataCallBack fcallback)
                            :m_lLoginID(loginID)
{
    flag = true;
    m_lhRealPlay = -1;
    DH_RealPlayType rType;
    EM_REAL_DATA_TYPE nSCtype;
    switch(replayType)
    {
        case 0:
            rType=DH_RType_Realplay;
            break;
        case 1:
            rType=DH_RType_Realplay_1;
            break;
        case 2:
            rType=DH_RType_Realplay_2;
            break;
        case 3:
            rType=DH_RType_Realplay_3;
        case 4:
            rType=DH_RType_Multiplay;
            break;
        case 5:
            rType=DH_RType_Multiplay_1;
            break;
        case 6:
            rType=DH_RType_Multiplay_4;
            break;
        case 7:
            rType=DH_RType_Multiplay_8;
            break;
        case 8:
            rType=DH_RType_Multiplay_9;
            break;
        case 9:
            rType=DH_RType_Multiplay_16;
            break;
        case 10:
            rType=DH_RType_Multiplay_6;
            break;
        case 11:
            rType=DH_RType_Multiplay_12;
            break;
        case 12:
            rType=DH_RType_Multiplay_25;
            break;
        case 13:
            rType=DH_RType_Multiplay_36;
            break;
        case 14:
            rType=DH_RType_Multiplay_64;
            break;
        case 225:
            rType=DH_RType_Realplay_Test;
            break;
        default:
            break;
    }
    switch(playType)
    {
          case 0:
                nSCtype=EM_REAL_DATA_TYPE_PRIVATE;
		break;
	  case 1:
		nSCtype=EM_REAL_DATA_TYPE_GBPS;
		break;
	  case 2:
		nSCtype=EM_REAL_DATA_TYPE_TS;
		break;
        case 3:
            nSCtype=EM_REAL_DATA_TYPE_MP4;
            break;
        case 4:
            nSCtype=EM_REAL_DATA_TYPE_H264;
            break;
        case 5:
            nSCtype=EM_REAL_DATA_TYPE_FLV_STREAM;
            break;
	  default:
		break;
    }
    //启动实时监视或多画面预览 参数：CLIENT_LoginEx2的返回值，实时监视通道号，窗口句柄，枚举值
    rType=DH_RType_Realplay;
    NET_IN_REALPLAY_BY_DATA_TYPE stIn={sizeof(stIn)};
    NET_OUT_REALPLAY_BY_DATA_TYPE stOut={sizeof(stOut)};
    memset(&stIn,0,sizeof(NET_IN_REALPLAY_BY_DATA_TYPE));
    memset(&stOut,0,sizeof(NET_OUT_REALPLAY_BY_DATA_TYPE));
    stIn.dwSize=sizeof(NET_IN_REALPLAY_BY_DATA_TYPE);
    stOut.dwSize=sizeof(NET_OUT_REALPLAY_BY_DATA_TYPE);
    stIn.cbRealData=cbRealPlayData;
    stIn.emDataType=nSCtype;
    stIn.nChannelID=nChannelID;
    stIn.hWnd=(void*)hWnd;
    stIn.dwUser=dwUser;
    stIn.rType=rType;
    m_lhRealPlay=CLIENT_RealPlayByDataType(m_lLoginID,&stIn,&stOut,3000);//    CLIENT_RealPlayByDataType(m_lLoginID,&stIn,&stOut,50000);
    //m_lhRealPlay = CLIENT_RealPlayEx(m_lLoginID,nChannelID,hWnd,rType);
    //m_lhRealPlay=CLIENT_StartRealPlay(m_lLoginID,nChannelID,hWnd,rType,cbRealPlayData,NULL,dwUser);
    //cout<<(CLIENT_GetLastError()&(0x7fffffff))<<endl;
    //delete stIn;
    //delete stOut;
    if(!m_lhRealPlay)
    {
        flag = false;
        m_lhRealPlay = -1;
        //Error_k();
        sdkLogcout();
    }
    else 
    {
        if(NULL!=cbRealPlayData)
        {
            //设置实时监视数据回调，给用户提供设备流出的数据，当cbRealData为NULL时结束回调数据
            //参数：CLIENT_RealPlay 的返回值 ，回调函数，用于传出设备流出的实时数据 ，用户数据
            /*if(FALSE == CLIENT_SetRealDataCallBackEx(m_lhRealPlay,cbRealPlayData,dwUser,REALDATA_FLAG_RAW_DATA))
            {
                flag = false;
                //Error_k();
                sdkLogcout();
            }*/
        }
    }
}

bool JF_RealPlay_K::Release()
{
    if(-1 != m_lhRealPlay)
    {
        //PLAY_Stop(0);
        //PLAY_CloseStream(0);
        if(FALSE == CLIENT_StopRealPlayEx(m_lhRealPlay))//停止实时监视或多画面预览
        {
            //Error_k();
            sdkLogcout();
            return false;
        }
        else
        {
            m_lhRealPlay = -1;
            return true;
        }
    }
    return false;
}

JF_RealPlay_K::~JF_RealPlay_K()
{}
//设置图像效果
DWORD JF_RealPlay_K::JF_SetVideoEffect_k(JF_VideoEffect* struVidEff)
{
    //设置解码库视频参数，只有打开图像的函数参数hWnd有效时该函数获取的参数才有效，否则无意义
    //参数：预览返回值，亮度，对比度，色度，饱和度，
    if(FALSE == CLIENT_ClientSetVideoEffect(m_lhRealPlay,struVidEff->kBrightness,struVidEff->kContrast,struVidEff->kHue,struVidEff->kSaturation))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//获取图像效果
DWORD JF_RealPlay_K::JF_GetVideoEffect_k(JF_VideoEffect* struVidEff)
{
    //获取解码库视频参数，只有打开图像的函数参数hWnd有效时该函数获取的参数才有效，否则无意义
    //参数：预览返回值，亮度指针，对比度指针，饱和度指针，色度指针
    if(FALSE == CLIENT_ClientGetVideoEffect(m_lhRealPlay,&(struVidEff->kBrightness),&(struVidEff->kContrast),&(struVidEff->kHue),&(struVidEff->kSaturation)))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//打开声音
DWORD JF_RealPlay_K::JF_OpenSound_k()
{
    //打开音频，目前只支持独占方式打开音频，如果正在调用此函数之前音频已经打开并且hPlayHandle是不一样，则关闭原来的音频打开新的音频；函数只有在打开hPlayHandle的函数的参数hWnd有效时才生效
    if(FALSE == CLIENT_OpenSound(m_lhRealPlay))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//关闭声音
DWORD JF_RealPlay_K::JF_CloseSound_k()
{
    if(FALSE == CLIENT_CloseSound())//关闭音频
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//调节音量
DWORD JF_RealPlay_K::JF_SetVolume_k(int nVolume)
{
    if(0 > nVolume)
    {
        nVolume = 0;
    }
    else if(nVolume > 100)
    {
        nVolume = 100;
    }
    //设置音量,函数只有在打开hPlayHandle的函数的参数hWnd有效时才生效
    if(FALSE == CLIENT_SetVolume(m_lhRealPlay,nVolume))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//保存实时数据
DWORD JF_RealPlay_K::JF_SaveRealData_k(char *strFileName)
{
    //开始保存实时监视数据,对前端设备监视的图像进行数据保存,形成录像文件,此数据是设备端传送过来的原始视频数据
    //参数：返回值，实时监视保存文件名
    if(FALSE == CLIENT_SaveRealData(m_lhRealPlay,strFileName))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//停止实时数据保存
DWORD JF_RealPlay_K::JF_StopSaveRealData_k()
{
    //停止保存实时监视数据，关闭保存的文件
    if(FALSE == CLIENT_StopSaveRealData(m_lhRealPlay))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
//实时预览抓图
DWORD JF_RealPlay_K::JF_CapturePicture_k(char *strPictureName, unsigned int picFormat,fSnapRev OnSnapRevMessage)
{
    NET_CAPTURE_FORMATS format;
    switch(picFormat)
    {
        case 100:
            format = NET_CAPTURE_JPEG;
            break;
        case 170:
            format = NET_CAPTURE_JPEG_70;
            break;
        case 150:
            format = NET_CAPTURE_JPEG_50;
            break;
        case 130:
            format = NET_CAPTURE_JPEG_30;
            break;
        default:
            format = NET_CAPTURE_BMP;
            break;
    }
    CLIENT_SetSnapRevCallBack(OnSnapRevMessage,NULL);
    //保存图片，对显示图像进行瞬间抓图，只有打开图像的函数参数hWnd有效时该函数获取的参数才有效，否则无意义
    //参数：预览句柄，图片文件名，图片类型
    //if(FALSE == CLIENT_CapturePictureEx(m_lhRealPlay,strPictureName,format))
    static short g_nCmdSerial = 0;
    SNAP_PARAMS stuSnapParams;
    stuSnapParams.Channel=0;
    stuSnapParams.mode=0;
    stuSnapParams.CmdSerial=++g_nCmdSerial;
    if(FALSE==CLIENT_SnapPictureEx(m_lLoginID,&stuSnapParams))
    {
        //Error_k();
        sdkLogcout();
        return -1;
    }
    return 0;
}
