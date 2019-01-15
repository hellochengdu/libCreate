#include "../include/jf_inc/JF_V_Class.h"
#include "../include/v_inc/HCNetSDK.h"
using namespace std;
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
/*int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset,from_charset);
    if (cd==0) return -1;
    memset(outbuf,0,outlen);
    if (iconv(cd,pin,(size_t*)&inlen,pout,(size_t*)&outlen)==-1)
    {
        return -1;
    }
    iconv_close(cd);
    return 0;
}*/
BOOL SADPdevice_v(PDEVICE_FIND_CALLBACK pDeviceFindCallBack, int bInstallNPF , void* pUserData )
{
    //SADP_SetLogToFile(3,"./sadplog",FALSE);
    if(SADP_Start_V30(pDeviceFindCallBack,bInstallNPF,pUserData))
    {
        return 1;
    }else
    {
        return 0;
    }
}
BOOL StopSADPdevice_v()
{
    if(SADP_Stop())
    {
        return 1;
    }
    return 0;
}
CLog* sdkLog_v=getLogInstance();
CConfig inisdk_v;
void sdkLogcout_v(char* ip=NULL)
{
    int error=NET_DVR_GetLastError();
    char errorchar[100];
    char errorgood[10];
    sprintf(errorgood,"%d",error);
    if(ip==NULL)
    {
        sdkLog_v->WriteLog(inisdk_v.GetValue("Config",errorgood),sdkLog_v->DEBUG_LEVEL);
    } else{
        sprintf(errorchar,"%s%s%s",ip,":",inisdk_v.GetValue("Config",errorgood).toLocal8Bit().data());
        sdkLog_v->WriteLog(errorchar,sdkLog_v->DEBUG_LEVEL);
    }

}
//-----------------|-----------------------------|------------------------------
//-----------------|          JF_SDK_V 类        |------------------------------
//-----------------|-----------------------------|------------------------------
JF_SDK_V::JF_SDK_V()
{
    flag = true;
    sdkLog_v->InitModule("DVR_SDK_V");
    char s_buff[1024] = { 0 };
    int len = readlink("/proc/self/exe", s_buff, 1024);

    std::string s_ret = s_buff;

    //去掉 ----> '/程序名'
    int n_reverse_index = s_ret.rfind("/");
    if (n_reverse_index == -1)//未查找成功
    {
        sdkLog_v->WriteLog("错误输出文件无");
        strcpy(s_buff,"./hik_error.ini");
    }
    //截断
    s_ret.replace(n_reverse_index, s_ret.size() - n_reverse_index, "/");
    memset(s_buff,0,1024);
    sprintf(s_buff,"%s%s",s_ret.data(),"hik_error.ini");
    inisdk_v.InitMoudle(s_buff);
    if(FALSE == NET_DVR_Init())//初始化SDK，成功返回true，FALSE失败
    {
        flag = false;
        //Error_v();
        sdkLogcout_v();
    }
    else
    {
        if(FALSE == NET_DVR_SetConnectTime(3000,1))//设置网络连接的超时时间和连接次数，成功返回true，FALSE失败
        {
            flag = false;
            //Error_v();
             sdkLogcout_v();
        }
        else
        {
            if(FALSE == NET_DVR_SetReconnect(5000,TRUE))//设置重连功能，成功返回true，FALSE失败
            {
                flag = false;
                //Error_v();
                 sdkLogcout_v();
            }
        }
    }
}

JF_SDK_V::~JF_SDK_V()
{
    if(FALSE == NET_DVR_Cleanup())//释放SDK资源，在程序结束之前调用，成功返回true，FALSE失败
    {
        //Error_v();
         sdkLogcout_v();
    }
}
//设备登录
LONG JF_SDK_V::JF_Login_v(char*  pchDVRIP,  //设备地址
                     WORD   wDVRPort,        //设备端口号
                     char*  pchUserName,     //用户名
                     char*  pchPasswd,       //用户密码
                     void*  pUserData,       //用户数据
                     char*  deviceType,
                     NET_DVR_DEVICEINFO_V40* struDeviceInfo)//设备信息 out
{
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    strcpy(struLoginInfo.sDeviceAddress, pchDVRIP);
    struLoginInfo.wPort = wDVRPort;
    strcpy(struLoginInfo.sUserName, pchUserName);
    strcpy(struLoginInfo.sPassword, pchPasswd);
    struLoginInfo.cbLoginResult = NULL;
    struLoginInfo.pUser = pUserData;
    struLoginInfo.bUseAsynLogin = 0;
    struLoginInfo.byProxyType=0;
    if(strcmp((const char*)deviceType,"PA")==0)
    {
        struLoginInfo.byJiaFaEncypt=2;
    }else if(strcmp((const char*)deviceType,"XC")==0)
    {
        struLoginInfo.byJiaFaEncypt = 1;
    }else if(strcmp((const char*)deviceType,"OTH")==0)
    {
        struLoginInfo.byJiaFaEncypt=0;
    }
    LONG loginID = NET_DVR_Login_V40(&struLoginInfo, struDeviceInfo);//用户注册设备，
    if(-1 == loginID)//用户ID
    {
        sdkLogcout_v(pchDVRIP);
    }
    return loginID;
}
//设备登出
DWORD JF_SDK_V::JF_Logout_v(LONG loginID)//用户ID
{
    if(FALSE == NET_DVR_Logout_V30(loginID))//用户注销，
    {
        //Error_v();
         sdkLogcout_v();
        return -1;
    }
    return 0;
}
BOOL  JF_SDK_V::JF_RemoteControl_v(LONG loginID)
{
    if(TRUE==NET_DVR_RemoteControl(loginID,NET_DVR_CHECK_USER_STATUS,NULL,0))
    {
        return TRUE;
    }else
    {
        return FALSE;
    }
}
//获取错误码
int JF_SDK_V::JF_GetLastError_v()
{
    return  NET_DVR_GetLastError();
}
//批量获取NVR上IPC通道
bool JF_SDK_V::JF_GetRemoteDecice_v(LONG lUserID,DWORD dwCommand,LONG lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned)
{
    if(NET_DVR_GetDVRConfig(lUserID,dwCommand,lChannel,lpOutBuffer,dwOutBufferSize,lpBytesReturned))
    {
        return 1;
    }
    sdkLogcout_v();
    return 0;
}
//批量设置NVR上IPC通道
bool JF_SDK_V::JF_SetRemoteDevice_v(LONG lUserID, DWORD dwCommand,LONG lChannel, LPVOID lpInBuffer, DWORD dwInBufferSize)
{
    if(NET_DVR_SetDVRConfig(lUserID,dwCommand,lChannel,lpInBuffer,dwInBufferSize))
    {
        return 1;
    }
    sdkLogcout_v();
    return 0;
}
//批量更换修改NVR上IPC通道
bool JF_SDK_V::JF_ChangRemoteDevice_v(JF_UserIDInfo* deviceInfo)
{
    return 0;
}
//批量删除NVR上IPC通道
bool JF_SDK_V::JF_DeleteRemoteDevice_v(JF_UserIDInfo* deviceInfo)
{
    return 0;
}
//获取存储上所有的通道信息
bool JF_SDK_V::JF_QueryDevInfo_v(JF_DeviceInfo* deviceInfo)
{
    return 0;
}
//设置前端IPC的账号密码（用户名密码）/IP信息（IP地址与端口）以及重要参数的读取设置（编码信息、OSD信息_通道时间标签以及位置）
bool JF_SDK_V::JF_SetCameraInfo_v(JF_CameraInfo CameraInfo)
{
    return 0;
}
//获取设备信息包括（设备型号、系统信息（CPU、内存使用情况）、硬盘信息（硬盘总容量、剩余可用空间、锁定空间大小）、版本信息等）
bool JF_SDK_V::JF_GetDeviceInfo_v(JF_Details DetailsInfo)
{
    return 0;
}
//NVR批量和单个通道的文件锁定信息读取（通道号、文件列表以及文件大小——支持文件和时间锁定及解锁功能），并支持批量和单个通道设置文件锁定计划设置
bool JF_SDK_V::JF_MarkInfo_v(JF_MarkTImeInfo stInfo)
{
    return 0;
}
//控制前端设备恢复出厂设置以及重启关机操作
bool JF_SDK_V::JF_ResetSystem_v(int stType)
{
    return 0;
}
//控制配置文件导出
bool JF_SDK_V::JF_ExportConfigFile_v(char *szFileName,fMarkConfigPosCallBack cbUploadPos,LDWORD dwUserData)
{
    return 0;
}
//控制配置文件导入
bool JF_SDK_V::JF_ImportConfigFile_v(char *szFileName,fMarkConfigPosCallBack cbUploadPos,LDWORD dwUserData)
{
    return 0;
}
//获取OSD接口
bool JF_SDK_V::JF_GetDVRConfig_OSD_v(LONG loginID,LONG lChannel,char* OSD_lpOutBuffer)
{
    NET_DVR_PICCFG_V40 OSD_Struct;
    char lposd[32];
    DWORD sizeout;
    if(NET_DVR_GetDVRConfig(loginID,NET_DVR_GET_PICCFG_V40,lChannel,(void*)&OSD_Struct,sizeof(OSD_Struct),&sizeout))
    {
        strncpy(OSD_lpOutBuffer,(const char*)OSD_Struct.sChanName,32);
        return 1;
    }
    sdkLogcout_v();
    return 0;
}
//获取字符叠加的接口
bool JF_SDK_V::JF_GetDVRConfig_CharOSD_v(LONG loginID,LONG lChannel,char* char_lpOutBuffer)
{
    NET_DVR_SHOWSTRING_V30 CharStruct;
    DWORD sizeout;
    if(NET_DVR_GetDVRConfig(loginID,NET_DVR_GET_SHOWSTRING_V30,lChannel,(void*)&CharStruct,sizeof(CharStruct),&sizeout))
    {
        strncpy(char_lpOutBuffer,(const char*)CharStruct.struStringInfo->sString,32);
        return 1;
    }
    sdkLogcout_v();
    return 0;
}
//设置OSD接口
bool JF_SDK_V::JF_SetDVRConfig_OSD_v(LONG loginID,LONG lChannel,char* OSD_lpInBuffer)
{
    NET_DVR_PICCFG_V40 OSD_Struct;
    DWORD sizeout;
    if(NET_DVR_GetDVRConfig(loginID,NET_DVR_GET_PICCFG_V40,lChannel,(void*)&OSD_Struct,sizeof(OSD_Struct),&sizeout))
    {
        //code_convert("UTF-8","gbk",utf,32,gbk,32);
        QString utf=QString::fromUtf8(OSD_lpInBuffer);
        QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");       //获取UTF-8编码对象
        QString  unicode = utf8->toUnicode(utf.toLocal8Bit().data(),32);               //通过UTF-8编码对象将啊哈转为utf-16
        QTextCodec *gbk = QTextCodec::codecForName("GBK");          //获取GBK编码对象
        QByteArray arr=gbk->fromUnicode(unicode);
        memset(OSD_Struct.sChanName,0x00,32);
        strncpy((char*)OSD_Struct.sChanName,arr.data(),32);
        OSD_Struct.wShowNameTopLeftX =0;
        OSD_Struct.wShowNameTopLeftY=500;
        if(NET_DVR_SetDVRConfig(loginID, NET_DVR_SET_PICCFG_V40, lChannel,&OSD_Struct,sizeof(NET_DVR_PICCFG_V30)))
        {
            return 1;
        }
    }
    sdkLogcout_v();
    return 0;
}
//设置字符叠加的接口
bool JF_SDK_V::JF_SetDVRConfig_CharOSD_v(LONG loginID,LONG lChannel,char* char_lpInBuffer)
{
    NET_DVR_SHOWSTRING_V30 CharStruct;
    DWORD sizeout;
    if(NET_DVR_GetDVRConfig(loginID,NET_DVR_GET_SHOWSTRING_V30,lChannel,(void*)&CharStruct,sizeof(CharStruct),&sizeout))
    {
        QString gbkstring=QString::fromUtf8(char_lpInBuffer);
        QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");       //获取UTF-8编码对象
        QString  unicode = utf8->toUnicode(gbkstring.toLocal8Bit().data(),32);               //通过UTF-8编码对象将啊哈转为utf-16
        QTextCodec *gbk = QTextCodec::codecForName("GBK");          //获取GBK编码对象
        QByteArray arr=gbk->fromUnicode(unicode);
        memset(CharStruct.struStringInfo->sString,0x00,44);
        strncpy(CharStruct.struStringInfo->sString,arr.data(),32);
        CharStruct.struStringInfo->wShowString=1;
        CharStruct.struStringInfo->wStringSize=32;
        CharStruct.struStringInfo->wShowStringTopLeftX=0;
        CharStruct.struStringInfo->wShowStringTopLeftY=550;
        if(NET_DVR_SetDVRConfig(loginID,NET_DVR_SET_SHOWSTRING_V30,lChannel,&CharStruct,sizeof(NET_DVR_SET_SHOWSTRING_V30)))
        {
            return 1;
        }
    }
    sdkLogcout_v();
    return 0;
}
//云台基本控制          NET_DVR_Login_v40返回值，通道号，             云台控制命令  ， 云台开始动作或者停止动作    0-开始，1-结束
DWORD JF_SDK_V::JF_PTZControl_v(LONG loginID, LONG lChannel, DWORD dwPTZCommand, BOOL bStop)
{ 
    if(FALSE == NET_DVR_PTZControl_Other(loginID,lChannel,dwPTZCommand,bStop))//云台控制操作函数
    {
        //Error_v();
         sdkLogcout_v();
        return -1;
    }
    return 0;
}
//云台控制带速度
bool  JF_SDK_V::JF_PTZControlWithSpeed_v(LONG loginID,LONG lChannel,DWORD dwPTZCommand,DWORD bStop,DWORD dwSpeed)
{
    if(FALSE==NET_DVR_PTZControlWithSpeed_Other(loginID,lChannel,dwPTZCommand,bStop,dwSpeed))
    {
        sdkLogcout_v();
        return  0;
    }
    return 1;
}
//云台预置点操作
DWORD JF_SDK_V::JF_PTZPreset_v(LONG loginID, LONG lChannel, DWORD dwPTZCommand, DWORD dwPresetIndex)//函数登陆返回值，通道号，操作云台预置点命令，预置点序号
{
    switch (dwPTZCommand)
    {
        case 8:
        case 9:
        case 39:
            break;
        default:
            return -1;
    }
    if(FALSE == NET_DVR_PTZPreset_Other(loginID,lChannel,dwPTZCommand,dwPresetIndex))//云台预置点操作函数
    {
         sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//云台巡航操作
DWORD JF_SDK_V::JF_PTZCruise_v(LONG loginID, LONG lChannel, DWORD dwPTZCommand, BYTE byCruiseRoute, BYTE byCruisePoint, WORD wInput)//登陆函数返回值，通道号，操作云台巡航的命令，巡航路线，巡航点，wInput：不同巡航命令时，值不同，预置点（最大300），速度（最大255），时间（最大40）
{
    NET_DVR_CRUISE_RET lpCruiseRet;
    int i=0;
    switch(dwPTZCommand)
    {
        case 30:
        case 31:
        case 32:
        case 33:
        case 37:
        case 38:
            break;
        case 39:
            if(FALSE==NET_DVR_GetPTZCruise(loginID,lChannel,byCruiseRoute,&lpCruiseRet))
            {
                 sdkLogcout_v();
                //Error_v();
                return -1;
            }
            for(;lpCruiseRet.struCruisePoint[i].PresetNum!=0;)
            {
                if(FALSE==NET_DVR_PTZCruise_Other(loginID,lChannel,33,byCruiseRoute,lpCruiseRet.struCruisePoint[i++].PresetNum,wInput))
                {
                     sdkLogcout_v();
                    //Error_v();
                    return -1;
                }

            }
            return 0;
            break;
        default:
            return -1;
    }
    if(FALSE == NET_DVR_PTZCruise_Other(loginID,lChannel,dwPTZCommand,byCruiseRoute,byCruisePoint,wInput))//云台巡航操作
    {
         sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//云台轨迹操作
DWORD JF_SDK_V::JF_PTZTrack_v(LONG loginID, LONG lChannel, DWORD dwPTZCommand)//登陆函数返回值，通道号，操作云台轨迹的命令
{
    switch(dwPTZCommand)
    {
        case 34:
        case 35:
        case 36:
        case 44:
            break;
        default:
            return -1;
    }
    if(FALSE == NET_DVR_PTZTrack_Other(loginID,lChannel,dwPTZCommand))//云台轨迹操作函数
    {
         sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//云台守望设置边界
bool  JF_SDK_V::JF_PTZRemoteControl_v(LONG lUserID,DWORD dwCommand,LPVOID lpInBuffer,DWORD dwInBufferSize)
{
    if(!NET_DVR_RemoteControl(lUserID,dwCommand,lpInBuffer,dwInBufferSize))
    {
        sdkLogcout_v();
        return 0;
    }
    return 1;
}
//获取设备配置信息                  登陆函数返回值，设备配置命令，      通道号，        接受数据的缓冲指针，接受数据的缓冲长度，   实际接受到的数据长度指针
DWORD JF_SDK_V::JF_GetDVRConfig_v(LONG loginID, DWORD dwCommand, LONG lChannel, void* pOutBuf, DWORD dwOutBufSize, DWORD* pBytesRet)
{
    if(NULL == pOutBuf)
    {
        return -1;
    }
    //                          登陆函数返回值，设备配置命令，通道号，接受数据的缓冲指针，接受数据的缓冲长度，实际接受到的数据长度指针
    if(FALSE == NET_DVR_GetDVRConfig(loginID,dwCommand,lChannel,pOutBuf,dwOutBufSize,pBytesRet))//获取设备的配置信息函数
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//配置设备                          登陆函数的返回值，配置命令，通道号，输入数据的缓冲指针，输入数据的缓冲长度
DWORD JF_SDK_V::JF_SetDVRConfig_v(LONG loginID, DWORD dwCommand, LONG lChannel, void* pInBuf, DWORD dwInBufSize)//用户ID，设备配置命令，通道号，输入数据的缓冲指针，输入数据的缓冲长度
{
    if(NULL == pInBuf)
    {
        return -1;
    }                       //   登陆函数的返回值，配置命令，通道号，输入数据的缓冲指针，输入数据的缓冲长度
    if(FALSE == NET_DVR_SetDVRConfig(loginID,dwCommand,lChannel,pInBuf,dwInBufSize))//设置设备的配置信息
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}

void g_cbFindFile(DWORD dwChannel, DWORD dwFileNum, char* lpFileInfo, fFindFileLog cbFindFile,LDWORD userData)
{              
    char *chpFileInfo = new char[10*sizeof(JF_FileInfo)]();
    for(int i=0; i<dwFileNum; ++i)
    {
        NET_DVR_FINDDATA_V40 fileInfo = {0};//文件查找结果信息结构体
        JF_FileInfo file = {0};//自定义的文件结构体
        char *pNowIn = lpFileInfo+i*sizeof(fileInfo);

        memcpy(&fileInfo, pNowIn, sizeof(fileInfo));
        strcpy(file.FileName, fileInfo.sFileName);//文件名
        file.StartTime.tm_year= fileInfo.struStartTime.dwYear;//文件开始的时间
        file.StartTime.tm_mon = fileInfo.struStartTime.dwMonth;
        file.StartTime.tm_mday= fileInfo.struStartTime.dwDay;
        file.StartTime.tm_hour= fileInfo.struStartTime.dwHour;
        file.StartTime.tm_min = fileInfo.struStartTime.dwMinute;
        file.StartTime.tm_sec = fileInfo.struStartTime.dwSecond;
        struct tm stm = {0};
        stm.tm_year= fileInfo.struStartTime.dwYear;
        stm.tm_mon = fileInfo.struStartTime.dwMonth;
        stm.tm_mday= fileInfo.struStartTime.dwDay;
        stm.tm_hour= fileInfo.struStartTime.dwHour;
        stm.tm_min = fileInfo.struStartTime.dwMinute;
        stm.tm_sec = fileInfo.struStartTime.dwSecond;
        time_t s = mktime(&stm);//time函数将时间转换成秒数从1970年1月1号到至今UTC所经过的秒数
        struct tm etm = {0};
        etm.tm_year= fileInfo.struStopTime.dwYear;//文件结束的时间
        etm.tm_mon = fileInfo.struStopTime.dwMonth;
        etm.tm_mday= fileInfo.struStopTime.dwDay;
        etm.tm_hour= fileInfo.struStopTime.dwHour;
        etm.tm_min = fileInfo.struStopTime.dwMinute;
        etm.tm_sec = fileInfo.struStopTime.dwSecond;
        time_t e = mktime(&etm);
        file.TimeLen = e - s;//文件回放时间
        file.FileLen = fileInfo.dwFileSize;//文件的大小

        char *pNowOut = chpFileInfo+i*sizeof(file);
        memcpy(pNowOut,&file,sizeof(file));
    }
    cbFindFile(dwChannel,dwFileNum,chpFileInfo,userData);
    delete[] chpFileInfo;
    chpFileInfo = NULL;
}
//查找文件                    登陆函数的返回值， 通道号，        文件类型            开始时间         结束时间           文件查找的回调函数           码流类型          文件查找的句柄        
DWORD JF_SDK_V::JF_FindFile_v(LONG loginID, LONG lChannel, DWORD dwFileType, JF_Timer *stime, JF_Timer *etime, fFindFileLog cbFindFile, long userData,BYTE byStreamType,LLONG *plhFindFile,sem_t *pSem)
{ 
    NET_DVR_FILECOND_V40 struFindCond = {0};//文件信息结构
    struFindCond.lChannel = lChannel;//通道号
    struFindCond.dwFileType = dwFileType;//文件类型
    struFindCond.dwIsLocked = 0xff; //文件是否被锁

    struFindCond.struStartTime.dwYear   = stime->tm_year;//文件开始时间
    struFindCond.struStartTime.dwMonth  = stime->tm_mon;
    struFindCond.struStartTime.dwDay    = stime->tm_mday;
    struFindCond.struStartTime.dwHour   = stime->tm_hour;
    struFindCond.struStartTime.dwMinute = stime->tm_min;
    struFindCond.struStartTime.dwSecond = stime->tm_sec;

    struFindCond.struStopTime.dwYear   = etime->tm_year;//文件结束时间
    struFindCond.struStopTime.dwMonth  = etime->tm_mon;
    struFindCond.struStopTime.dwDay    = etime->tm_mday;
    struFindCond.struStopTime.dwHour   = etime->tm_hour;
    struFindCond.struStopTime.dwMinute = etime->tm_min;
    struFindCond.struStopTime.dwSecond = etime->tm_sec;

    struFindCond.byDrawFrame = 0;//是否抽帧 0-不  1-是
    struFindCond.byStreamType = byStreamType;//码流类型 0-主码流 1-子码流 2-第三码流 0xff-全部
    LONG lhFindFile = NET_DVR_FindFile_V40(loginID, &struFindCond);//根据文件类型、时间查找设备录像文件
    if((NULL!=plhFindFile)&&(NULL!=pSem))//查找文件的返回值和查找文件的句柄为空
    {
        *plhFindFile = lhFindFile;  //线程时用
        sem_post(pSem);  //线程时用
    }
    if(-1 == lhFindFile)
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }

    char *outFileInfoBuf = new char[10*sizeof(NET_DVR_FINDDATA_V40)]();
    char *nowFileInfoBufp = outFileInfoBuf;
    unsigned int fileNum = 0;
    bool loop = true;
    while(loop)
    {
        NET_DVR_FINDDATA_V40 struFindData = {0};//文件查找结果信息结构体
        int ret = NET_DVR_FindNextFile_V40(lhFindFile, &struFindData);//逐个获取查找到的文件信息
        switch(ret)
        {
            case NET_DVR_FILE_SUCCESS:  //查找到一个文件
                if(fileNum >= 10)
                {
                    g_cbFindFile(lChannel, fileNum, outFileInfoBuf,cbFindFile,userData);
                    memset(outFileInfoBuf, 0, fileNum*sizeof(struFindData));//将某一块内存空间设置成某个数值
                    nowFileInfoBufp = outFileInfoBuf;
                    fileNum = 0;
                }
                memcpy(nowFileInfoBufp, &struFindData, sizeof(struFindData));//实现两块内存的拷贝，将第二块拷贝到第一块。
                nowFileInfoBufp += sizeof(struFindData);
                ++fileNum;
            case NET_DVR_ISFINDING:     //正在查找
                continue;
            case NET_DVR_FILE_NOFIND:   //未找到文件
                loop = false;
                break;
            case NET_DVR_NOMOREFILE:    //查找结束
                if(fileNum > 0)
                {
                    g_cbFindFile(lChannel, fileNum, outFileInfoBuf,cbFindFile,userData);
                }
                loop = false;
                break;
            case -1:
            case NET_DVR_FILE_EXCEPTION://查找异常
                //Error_v();
               sdkLogcout_v();
               if(outFileInfoBuf != NULL)
                {
                    delete[] outFileInfoBuf;
                    outFileInfoBuf = NULL;
                }
                NET_DVR_FindClose_V30(lhFindFile);
                return -1;
        }
    }
    if(outFileInfoBuf != NULL)
    {
        delete[] outFileInfoBuf;
        outFileInfoBuf = NULL;
    }

    return lhFindFile;
}
//停止文件查找
DWORD JF_SDK_V::JF_StopFindFile_v(LONG lhFindFile)
{
    if(FALSE == NET_DVR_FindClose_V30(lhFindFile))//关闭文件查找，释放资源，参数：文件查找句柄
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
void g_cbFindLog(DWORD dwChannel, DWORD dwFileNum, char* lpFileInfo, fFindFileLog cbFindLog,LDWORD userData)//查找日志
{
    char *chpFindLogInfo = new char[10*sizeof(JF_LogInfo)]();
    for(int i=0; i<dwFileNum; ++i)
    {
        NET_DVR_LOG_V30 logInfo = {0};//日志文件的信息体
        JF_LogInfo loginfo = {0};//自定义的日志信息体
        char *pNowIn = lpFileInfo+i*sizeof(logInfo);

        memcpy(&logInfo, pNowIn, sizeof(logInfo));
        loginfo.StartTime.tm_year= logInfo.strLogTime.dwYear;//开始时间
        loginfo.StartTime.tm_mon = logInfo.strLogTime.dwMonth;
        loginfo.StartTime.tm_mday= logInfo.strLogTime.dwDay;
        loginfo.StartTime.tm_hour= logInfo.strLogTime.dwHour;
        loginfo.StartTime.tm_min = logInfo.strLogTime.dwMinute;
        loginfo.StartTime.tm_sec = logInfo.strLogTime.dwSecond;
        loginfo.Type = logInfo.dwMajorType;
        strncpy(loginfo.LogContent, logInfo.sInfo, sizeof(loginfo.LogContent));
        loginfo.dwInfoLen = logInfo.dwInfoLen;

        char *pNowOut = chpFindLogInfo+i*sizeof(JF_LogInfo);
        memcpy(pNowOut, &loginfo, sizeof(JF_LogInfo));
    }
    
    cbFindLog(dwChannel, dwFileNum, chpFindLogInfo,userData);
    delete[] chpFindLogInfo;
    chpFindLogInfo = NULL;
}
//查找日志                     登陆函数返回值，查询方式，                   日志主类型，日志的次类型，           文件开始时间，文件结束时间，
DWORD JF_SDK_V::JF_FindLog_v(LONG loginID, LONG lSelectMode, DWORD dwMajorType, DWORD dwMinorType, JF_Timer *stime, JF_Timer *etime, fFindFileLog cbFindLog,LDWORD userData)
{
    NET_DVR_TIME stm = {0};
    stm.dwYear  = stime->tm_year;
    stm.dwMonth = stime->tm_mon;
    stm.dwDay   = stime->tm_mday;
    stm.dwHour  = stime->tm_hour;
    stm.dwMinute= stime->tm_min;
    stm.dwSecond= stime->tm_sec;
    NET_DVR_TIME etm = {0};
    etm.dwYear  = etime->tm_year;
    etm.dwMonth = etime->tm_mon;
    etm.dwDay   = etime->tm_mday;
    etm.dwHour  = etime->tm_hour;
    etm.dwMinute= etime->tm_min;
    etm.dwSecond= etime->tm_sec;
    //查找设备的日志信息                登陆函数返回值，查询方式，日志主类型，日志的次类型，文件开始时间，文件结束时间，是否只查询带有smart信息的日志
    LONG lhFindLog = NET_DVR_FindDVRLog_V30(loginID,lSelectMode,dwMajorType,dwMinorType,&stm,&etm,FALSE);
    if(-1 == lhFindLog)
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }

    char *outLogInfoBuf = NULL;
    try
    {
        outLogInfoBuf = new char[10*sizeof(NET_DVR_LOG_V30)](); //此处有时会出现未知异常       
    }
    catch (...)
    {
        sdkLog_v->WriteLog("new error");
        //error_v("new");
        return -1;
    }
    char *nowLogInfoBUfp = outLogInfoBuf;
    unsigned int logNum = 0;
    bool loop = true;
    while(loop)
    {
        NET_DVR_LOG_V30 struLogData = {0};
        int ret = NET_DVR_FindNextLog_V30(lhFindLog, &struLogData);//逐条获取差到的的日志信息
        switch(ret)
        {
            case NET_DVR_FILE_SUCCESS:  //查找到一个文件
                if(logNum >= 10)
                {
                    g_cbFindLog(-1, logNum, outLogInfoBuf,cbFindLog,userData);
                    memset(outLogInfoBuf, 0, 10*sizeof(struLogData));
                    nowLogInfoBUfp = outLogInfoBuf;
                    logNum = 0;
                }
                memcpy(nowLogInfoBUfp, &struLogData, sizeof(struLogData));
                nowLogInfoBUfp += sizeof(struLogData);
                ++logNum;
                continue;
            case NET_DVR_FILE_NOFIND:   //未查找到
                loop = false;
                break;
            case NET_DVR_ISFINDING:     //正在查找
                continue;
            case NET_DVR_NOMOREFILE:    //查找完毕
                if(logNum > 0)
                {
                    g_cbFindLog(-1, logNum, outLogInfoBuf,cbFindLog,userData);
                }
                loop = false;
                break;
            case -1:
            case NET_DVR_FILE_EXCEPTION://查找异常
                sdkLogcout_v();
                //Error_v();
                if(outLogInfoBuf != NULL)
                {
                    delete[] outLogInfoBuf;
                    outLogInfoBuf = NULL;
                }
                NET_DVR_FindLogClose_V30(lhFindLog);//释放查找日志的资源
                return -1;
        }
    }
    if(outLogInfoBuf != NULL)
    {
        delete[] outLogInfoBuf;
        outLogInfoBuf = NULL;
    }

    return lhFindLog;
}
//停止查找日志
DWORD JF_SDK_V::JF_StopFindLog_v(LONG lhFindLog)
{
    if(FALSE == NET_DVR_FindLogClose_V30(lhFindLog))//释放查找日志资源 参数：日志查找的句柄
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//开启监听                             回调函数，         端口，     IP，                       用户数据       
DWORD JF_SDK_V::JF_StartListen_v(int hLoginID,MSGCallBack cbMSG, WORD wLocalPort, char *pLocalIP, long pUserData)
{
    if(FALSE==NET_DVR_SetDVRMessageCallBack_V30(cbMSG,NULL))
    {
        sdkLogcout_v();
        //Error_v();
    }
    NET_DVR_ALARMCENTER_NETCFG lpInBuffer;
    lpInBuffer.dwSize=sizeof(NET_DVR_ALARMCENTER_NETCFG);
    strcpy((char*)&(lpInBuffer.byAuxiliaryAlarmAddr),pLocalIP);
    lpInBuffer.wAuxiliaryAlarmPort=wLocalPort;
    if(FALSE==NET_DVR_SetDVRConfig(hLoginID,NET_DVR_SET_ALARMCENTER_NETCFG,1,(void*)&lpInBuffer,sizeof(lpInBuffer)))
    {
        sdkLogcout_v();
        //Error_v();
    }
    LONG lhListen = NET_DVR_StartListen_V30(pLocalIP, wLocalPort, cbMSG, (void*)pUserData);//启动监听，接收设备主动上传的报警等信息（支持多线程）
    if(-1 == lhListen)
    {
        sdkLogcout_v();
        //Error_v();
    }
    LONG hAlarmChan=NET_DVR_SetupAlarmChan_V30(hLoginID);
    if(-1==hAlarmChan)
    {
        sdkLogcout_v();
        //Error_v();
    }
    return lhListen;
}
//停止监听
DWORD JF_SDK_V::JF_StopListen_v(LONG lhListen)
{
    if(FALSE == NET_DVR_StopListen_V30(lhListen))//停止监听（支持多线程）
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//重启设备
DWORD JF_SDK_V::JF_RebootDVR_v(LONG loginID)
{
    if(FALSE == NET_DVR_RebootDVR(loginID))//重启设备 参数：登陆设备的返回值
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//关闭设备
DWORD JF_SDK_V::JF_ShutDownDVR_v(LONG loginID)
{
    if(FALSE == NET_DVR_ShutDownDVR(loginID))//关闭设备
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//查询设备状态              登陆函数的返回值， 获取设备状态的命令值，要获取状态的个数，状态获取的条件缓冲区，    缓冲区的长度，         错误列表，   设备返回的状态内容， 输出缓冲区的大小
DWORD JF_SDK_V::JF_QueryDVRState_v(LONG loginID, DWORD dwCommand, DWORD dwCount, void* pInBuf, DWORD dwInBufSize, void* pStatusList, void* pOutBuf, DWORD dwOutBufSize)
{
    switch(dwCommand)
    {
        case 1:     //获取设备运行状态
            dwCommand = NET_DVR_GET_DEVICE_RUN_STATUS;
            dwCount = 1;
            pInBuf = NULL;
            break;
        case 2:     //获取无线布防状态
            dwCommand = NET_DVR_GET_REMOTECONTROL_STATUS;
            break;
        case 3:     //获取报警输入状态
            dwCommand = NET_DVR_GET_ALARMIN_STATUS;
            break;
        case 4:     //获取报警输出状态
            dwCommand = NET_DVR_GET_ALARMOUT_STATUS;
            break;
        case 5:     //获取语音对讲状态
            dwCommand = NET_DVR_GET_AUDIO_CHAN_STATUS;
            break;
        default:    //获取全部状态
            dwCount = 0xffffffff;
            pInBuf = NULL;
            dwInBufSize = 0;
            pStatusList = NULL;
            break;
    }
    //获取设备状态  参数：登陆函数的返回值， 获取设备状态的命令值，要获取状态的个数，状态获取的条件缓冲区，缓冲区的长度，错误列表，设备返回的状态内容，输出缓冲区的大小
    if(FALSE == NET_DVR_GetDeviceStatus(loginID,dwCommand,dwCount,pInBuf,dwInBufSize,pStatusList,pOutBuf,dwOutBufSize))
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//设置设备时间
DWORD JF_SDK_V::JF_SetDVRDateTime_v(LONG loginID, DWORD dwYear, DWORD dwMon, DWORD dwDay, DWORD dwHour, DWORD dwMin, DWORD dwSec)
{
    NET_DVR_TIME dvrTime = {0};
    dvrTime.dwYear  = dwYear;
    dvrTime.dwMonth = dwMon;
    dvrTime.dwDay   = dwDay;
    dvrTime.dwHour  = dwHour;
    dvrTime.dwMinute= dwMin;
    dvrTime.dwSecond= dwSec;
    //设置设备的配置信息 参数：登陆函数返回值，设备配置命令：设置时间参数，通道号，输入数据的缓冲指针，输入数据的缓冲长度
    if(FALSE == NET_DVR_SetDVRConfig(loginID,NET_DVR_SET_TIMECFG,-1,&dvrTime,sizeof(dvrTime)))
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}

	//按文件名下载
JF_DownloadFile_V* JF_SDK_V::JF_DownloadFile_v(LONG loginID, char* pSrcFileName, char* pSaveFileName)
{
    JF_DownloadFile_V *tmp = new JF_DownloadFile_V(loginID,pSrcFileName,pSaveFileName);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }
    return tmp;
}
	//按时间下载
JF_DownloadFile_V* JF_SDK_V::JF_DownloadFile_v(LONG loginID, char* pSaveFileName, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType)
{
    JF_DownloadFile_V* tmp = new JF_DownloadFile_V(loginID,pSaveFileName,dwChannel,stime,etime,byStreamType);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }
    return tmp;
}

	//按文件名回放
JF_PlayBack_V* JF_SDK_V::JF_PlayBack_v(LONG loginID, char* strFileName, HWND hWnd, fDataCallBack_v cbPlayData, long pUser)
{
    JF_PlayBack_V* tmp = new JF_PlayBack_V(loginID,strFileName,hWnd,cbPlayData,pUser);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }
    return tmp;
}
	//按时间段回放
JF_PlayBack_V* JF_SDK_V::JF_PlayBack_v(LONG loginID, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType, HWND hWnd, fDataCallBack_v cbPlayData, long pUser)
{
    JF_PlayBack_V* tmp = new JF_PlayBack_V(loginID,dwChannel,stime,etime,byStreamType,hWnd,cbPlayData,pUser);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }
    return tmp;
}

	//开启语音对讲
JF_VoiceCom_V* JF_SDK_V::JF_VoiceCom_v(LONG loginID, fVoiceDataCallBack cbVoiceData, long dwUser,BOOL ModeSize)
{
    JF_VoiceCom_V* tmp = new JF_VoiceCom_V(loginID,cbVoiceData,dwUser,ModeSize);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }
    return tmp;
}

	//开启实时预览
JF_RealPlay_V* JF_SDK_V::JF_RealPlay_v(LONG loginID, LONG lChannel, DWORD dwPlayBufNum, DWORD dwStreamType, DWORD dwLinkMode, 
				HWND hWnd, fDataCallBack_v cbRealData, long pUser)
{
    JF_RealPlay_V* tmp = new JF_RealPlay_V(loginID,lChannel,dwPlayBufNum,dwStreamType,dwLinkMode,hWnd,cbRealData,pUser);
    if(!tmp->getFlag())
    {
        tmp->Release();
        delete tmp;
        tmp = NULL;
    }
    return tmp;
}

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_DownloadFile_V 类   |------------------------------
//-----------------|-----------------------------|------------------------------
JF_DownloadFile_V::JF_DownloadFile_V(LONG loginID, char* pSrcFileName, char* pSaveFileName):m_lLoginID(loginID)
{
    flag = true;
    m_lhDownloadFile = -1;
    //按文件名下载录像文件 参数：登陆函数返回值，要下载的录像文件名，下载后保存到PC机的文件路径，需为绝对路径
    m_lhDownloadFile = NET_DVR_GetFileByName(m_lLoginID, pSrcFileName, pSaveFileName);
    if(-1 == m_lhDownloadFile)
    {
        flag = false;
        sdkLogcout_v();
        //Error_v();
    }
    else
    {
        //控制录像下载状态 参数：下载句柄，控制录像下载状态命令，指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
        if(FALSE == NET_DVR_PlayBackControl_V40(m_lhDownloadFile,NET_DVR_PLAYSTART,NULL,0,NULL,NULL))
        {
            flag = false;
            sdkLogcout_v();
            //Error_v();
        }
    }
}

JF_DownloadFile_V::JF_DownloadFile_V(LONG loginID, char* pSaveFileName, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType):m_lLoginID(loginID)
{
    flag = true;
    NET_DVR_PLAYCOND struDownloadCond = {0};
    struDownloadCond.dwChannel = dwChannel;

    struDownloadCond.struStartTime.dwYear   = stime->tm_year;
    struDownloadCond.struStartTime.dwMonth  = stime->tm_mon;
    struDownloadCond.struStartTime.dwDay    = stime->tm_mday;
    struDownloadCond.struStartTime.dwHour   = stime->tm_hour;
    struDownloadCond.struStartTime.dwMinute = stime->tm_min;
    struDownloadCond.struStartTime.dwSecond = stime->tm_sec;

    struDownloadCond.struStopTime.dwYear   = etime->tm_year;
    struDownloadCond.struStopTime.dwMonth  = etime->tm_mon;
    struDownloadCond.struStopTime.dwDay    = etime->tm_mday;
    struDownloadCond.struStopTime.dwHour   = etime->tm_hour;
    struDownloadCond.struStopTime.dwMinute = etime->tm_min;
    struDownloadCond.struStopTime.dwSecond = etime->tm_sec;
    struDownloadCond.byDrawFrame  = 0;//是否抽帧，
    struDownloadCond.byStreamType = byStreamType;//码流类型
    //按时间下载录像  参数：登陆函数返回值，下载后保存到pc机的路径 绝对路径，下载条件
    m_lhDownloadFile = NET_DVR_GetFileByTime_V40(m_lLoginID, pSaveFileName, &struDownloadCond);
    if(-1 == m_lhDownloadFile)
    {
        flag = false;
        sdkLogcout_v();
        //Error_v();
    }
    else
    {
        //控制回放状态  参数：播放句柄，控制回放状态命令：开始播放，指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
        if(FALSE == NET_DVR_PlayBackControl_V40(m_lhDownloadFile,NET_DVR_PLAYSTART,NULL,0,NULL,NULL))
        {
            flag = false;
            sdkLogcout_v();
            //Error_v();
        }
    }
}
//停止下载录像文件
bool JF_DownloadFile_V::Release()
{
    if(-1 != m_lhDownloadFile)
    {
        if(FALSE == NET_DVR_StopGetFile(m_lhDownloadFile))//停止下载录像文件 参数：下载句柄
        {
            sdkLogcout_v();
            //Error_v();
            return false;
        }
        else
        {
            m_lhDownloadFile = -1;
            return true;
        }
    }
    return false;
}

JF_DownloadFile_V::~JF_DownloadFile_V()
{}
 //获取下载进度
DWORD JF_DownloadFile_V::JF_GetDownloadPos_v()
{
    //获取当前下载录像文件的进度 参数：下载句柄
    int nPos = NET_DVR_GetDownloadPos(m_lhDownloadFile);
    if((-1==nPos)||(200==nPos))
    {
        sdkLogcout_v();
        //Error_v();
    }
    return nPos;
}

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_PlayBack_V 类       |------------------------------
//-----------------|-----------------------------|------------------------------
JF_PlayBack_V::JF_PlayBack_V(LONG loginID, char* strFileName, HWND hWnd, fDataCallBack_v cbPlayData, long pUser)
                            :m_lLoginID(loginID),m_bMethod(true),m_bPause(false)
{
    flag = true;
    m_lhPlayBack = -1;
    if(strlen(strFileName) > 100)
    {
        m_lhPlayBack = -1;
        flag = false;
        sdkLog_v->WriteLog("file name length too long");
        //error_v("file name length too long");
    }
    else
    {
        //按文件名回放录像文件 参数：登陆接口函数的返回值，回放的文件名，回放窗口句柄，
        m_lhPlayBack = NET_DVR_PlayBackByName(m_lLoginID, strFileName, hWnd);
        if(-1 == m_lhPlayBack)
        {
            flag = false;
            sdkLogcout_v();
            //Error_v();
        }
        else
        {
            //控制录像回放的状态 参数：播放句柄，控制窗口回放状态的控制命令：开始播放，指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
            if(FALSE == NET_DVR_PlayBackControl_V40(m_lhPlayBack,NET_DVR_PLAYSTART,NULL,0,NULL,NULL))
            {
                flag = false;
                sdkLogcout_v();
                // Error_v();
                if(NET_DVR_StopPlayBack(m_lhPlayBack))//停止回放录像文件
                {
                    m_lhPlayBack = -1;
                }
            }
            else
            {
                if(NULL != cbPlayData)
                {
                    //注册回调函数，捕获录像数据 参数：播放句柄，录像数据回调函数，用户数据
                    if(FALSE == NET_DVR_SetPlayDataCallBack_V40(m_lhPlayBack,cbPlayData,(void*)pUser))
                    {
                        flag = false;
                        sdkLogcout_v();
                        //Error_v();
                    }
                }
            }
        }
    }
}

JF_PlayBack_V::JF_PlayBack_V(LONG loginID, DWORD dwChannel, JF_Timer *stime, JF_Timer *etime, BYTE byStreamType, 
                            HWND hWnd, fDataCallBack_v cbPlayData, long pUser)
                            :m_lLoginID(loginID),
                            m_bMethod(false),
                            m_bPause(false)
{
    flag = true;
    
    NET_DVR_VOD_PARA vodPara = {0};//录像回放结构体
    vodPara.dwSize = sizeof(vodPara);//结构体大小
    vodPara.struIDInfo.dwSize = sizeof(vodPara.struIDInfo);//流ID信息结构体的结构体大小
    vodPara.struIDInfo.dwChannel = dwChannel;//关联的设备通道

    vodPara.struBeginTime.dwYear    = stime->tm_year;//开始时间参数结构体
    vodPara.struBeginTime.dwMonth   = stime->tm_mon;
    vodPara.struBeginTime.dwDay     = stime->tm_mday;
    vodPara.struBeginTime.dwHour    = stime->tm_hour;
    vodPara.struBeginTime.dwMinute  = stime->tm_min;
    vodPara.struBeginTime.dwSecond  = stime->tm_sec;

    vodPara.struEndTime.dwYear    = etime->tm_year;//结束时间参数结构体
    vodPara.struEndTime.dwMonth   = etime->tm_mon;
    vodPara.struEndTime.dwDay     = etime->tm_mday;
    vodPara.struEndTime.dwHour    = etime->tm_hour;
    vodPara.struEndTime.dwMinute  = etime->tm_min;
    vodPara.struEndTime.dwSecond  = etime->tm_sec;
    vodPara.hWnd = hWnd;//回放窗口句柄
    vodPara.byDrawFrame = 0;//是否抽帧：否
    vodPara.byVolumeType = 0;//0-普通录像卷，1-存档卷，适用于CVR设备，普通卷用于通道录像，存档卷用于备份录像 
    vodPara.byStreamType = byStreamType;//码流类型：0- 主码流，1- 子码流，2- 码流三
    vodPara.byAudioFile = 0;//回放音频文件：0- 不回放音频文件，1- 回放音频文件，该功能需要设备支持，启动音频回放后只回放音频文件 
    //按流ID和时间回放录像文件 参数：NET_DVR_Login_V40等登录接口的返回值，查找条件
    m_lhPlayBack = NET_DVR_PlayBackByTime_V40(m_lLoginID, &vodPara);
    if(-1 == m_lhPlayBack)
    {
        flag = false;
        sdkLogcout_v();
        //Error_v();
    }
    else
    {
        //控制录像回放的状态 参数：播放句柄，控制录像回放状态命令：开始播放，指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
        if(FALSE == NET_DVR_PlayBackControl_V40(m_lhPlayBack, NET_DVR_PLAYSTART, NULL, 0, NULL, NULL))
        {
            flag = false;
            sdkLogcout_v();
            //Error_v();
            //停止回放录像文件 参数：回放句柄
            if(NET_DVR_StopPlayBack(m_lhPlayBack))
            {
                m_lhPlayBack = -1;
            }
        }
        else
        {
            if(NULL != cbPlayData)
            {
                //注册回调函数，捕获录像数据             参数：播放句柄，录像数据回调函数，用户数据
                if(FALSE == NET_DVR_SetPlayDataCallBack_V40(m_lhPlayBack,cbPlayData,(void*)pUser))
                {
                    flag = false;
                    sdkLogcout_v();
                    //Error_v();
                }
            }
        }
    }
}
//停止回放录像文件
bool JF_PlayBack_V::Release()
{
    if(-1 != m_lhPlayBack)
    {
        if(FALSE == NET_DVR_StopPlayBack(m_lhPlayBack))//停止回放录像文件 参数：回放句柄
        {
            sdkLogcout_v();
            //Error_v();
            return false;
        }
        else
        {
            m_lhPlayBack = -1;
            return true;
        }
    }
    return false;
}

JF_PlayBack_V::~JF_PlayBack_V()
{}
 //获取/更改播放进度
BOOL JF_PlayBack_V::JF_SGPlayBackPos_v(unsigned int* nPos, BOOL SGetPos)
{
    if(SGetPos) //设置进度
    {
        if(0 > *nPos)
        {
            *nPos = 0;
        }
        else if(*nPos > 100)
        {
            *nPos = 100;
        }
        //控制录像回放的状态 参数：播放句柄，控制录像回放状态命令：改变文件回放的进度，指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
        if(FALSE == NET_DVR_PlayBackControl_V40(m_lhPlayBack,NET_DVR_PLAYSETPOS,nPos,sizeof(*nPos),NULL,NULL))
        {
            sdkLogcout_v();
            //Error_v();
            return FALSE;
        }
    }
    else        //获取进度
    {
        //控制录像回放的状态 参数：播放句柄，控制录像回放状态命令：获取按文件或者按时间回放的进度，指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
        if(FALSE == NET_DVR_PlayBackControl_V40(m_lhPlayBack,NET_DVR_PLAYGETPOS,NULL,0,nPos,NULL))
        {
            sdkLogcout_v();
            //Error_v();
            return FALSE;
        }
    }
    return TRUE;
}
//快放
BOOL JF_PlayBack_V::JF_FastPlayBack_v()
{
     //控制录像回放的状态 参数：播放句柄，控制录像回放状态命令：快放，指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
    if(FALSE == NET_DVR_PlayBackControl_V40(m_lhPlayBack,NET_DVR_PLAYFAST,NULL,0,NULL,NULL))
    {
        sdkLogcout_v();
        //Error_v();
        return FALSE;
    }
    return TRUE;
}
//慢放
BOOL JF_PlayBack_V::JF_SlowPlayBack_v()
{
    //控制录像回放的状态 参数：播放句柄，控制录像回放状态命令：慢放，指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
    if(FALSE == NET_DVR_PlayBackControl_V40(m_lhPlayBack,NET_DVR_PLAYSLOW,NULL,0,NULL,NULL))
    {
        sdkLogcout_v();
        //Error_v();
        return FALSE;
    }
    return TRUE;
}
//暂停/继续
BOOL JF_PlayBack_V::JF_PausePlayBack_v()
{
    if(!m_bPause)
    {
         //控制录像回放的状态 参数：播放句柄，控制录像回放状态命令：暂停播放，指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
        if(FALSE == NET_DVR_PlayBackControl_V40(m_lhPlayBack,NET_DVR_PLAYPAUSE,NULL,0,NULL,NULL))
        {
            sdkLogcout_v();
            //Error_v();
            return FALSE;
        }
        m_bPause = true;
    }
    else
    {
         //控制录像回放的状态 参数：播放句柄，控制录像回放状态命令：恢复播放（在暂停后调用将恢复暂停前的速度播放），指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
        if(FALSE == NET_DVR_PlayBackControl_V40(m_lhPlayBack,NET_DVR_PLAYRESTART,NULL,0,NULL,NULL))
        {
            sdkLogcout_v();
            //Error_v();
            return FALSE;
        }
        m_bPause = false;
    }
    
    return TRUE;
}
//恢复正常播放
BOOL JF_PlayBack_V::JF_NormalPlayBack_v()
{
    //控制录像回放的状态 参数：播放句柄，控制录像回放状态命令：正常速度播放（快放或者慢放时调用将恢复单倍速度播放），指向输入参数的指针，输入参数的长度，指向输出参数的指针，输出参数的长度
    if(FALSE == NET_DVR_PlayBackControl_V40(m_lhPlayBack,NET_DVR_PLAYNORMAL,NULL,0,NULL,NULL))
    {
        sdkLogcout_v();
        //Error_v();
        return FALSE;
    }
    return TRUE;
}

//-----------------|-----------------------------|------------------------------
//-----------------|          JF_VoiceCom_V 类       |------------------------------
//-----------------|-----------------------------|------------------------------
void callBackVoice(LONG lVoiceComHandle,char *pRecvDataBuffer,DWORD dwBufSize,BYTE byAudioFlag,void *pUser)
{
    if(FALSE==NET_DVR_VoiceComSendData(lVoiceComHandle,pRecvDataBuffer,dwBufSize))
    {
        sdkLogcout_v();
        //Error_v();
    }
}
JF_VoiceCom_V::JF_VoiceCom_V(LONG loginID, fVoiceDataCallBack cbVoiceData, long dwUser,BOOL ModeSize):m_lLoginID(loginID)
{
    flag = true;
    m_lhVoiceCom = -1;
    //启动语音对讲 参数：登陆函数的返回值，音频数据回调函数，用户数据
    if(TRUE==ModeSize)
    {
        if(-1==NET_DVR_StartVoiceCom_MR_V30(m_lLoginID,1,callBackVoice,(void*)dwUser))
        {
            flag=false;
            sdkLogcout_v();
            //Error_v();
        }
    }else
    {
        m_lhVoiceCom = NET_DVR_StartVoiceCom(m_lLoginID, cbVoiceData, dwUser);
        if(-1 == m_lhVoiceCom)
        {
            flag = false;
            //Error_v();
            sdkLogcout_v();
        }
    }
}
//停止语音对讲
bool JF_VoiceCom_V::Release()
{
    if(-1 != m_lhVoiceCom)
    {
        if(FALSE == NET_DVR_StopVoiceCom(m_lhVoiceCom))//停止语音对讲或者语音转发 参数：返回值
        {
            //Error_v();
            sdkLogcout_v();
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

JF_VoiceCom_V::~JF_VoiceCom_V()
{}
//设置音量
DWORD JF_VoiceCom_V::JF_SetVolume_v(WORD wVolume)
{
    if(0 > wVolume)
    {
        wVolume = 0;
    }
    else if(100 > wVolume)
    {
        wVolume = 100;
    }

    wVolume = wVolume * 0xffff / 100;
    //设置语音对讲客户端的音量 参数：登陆接口返回值，设置音量
    if(FALSE == NET_DVR_SetVoiceComClientVolume(m_lhVoiceCom,wVolume))
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}

//-----------------|-----------------------------|----------------------s--------
//-----------------|          JF_RealPlay_V 类       |------------------------------
//-----------------|-----------------------------|------------------------------
JF_RealPlay_V::JF_RealPlay_V(LONG loginID, LONG lChannel, DWORD dwPlayBufNum, DWORD dwStreamType, 
                            DWORD dwLinkMode, HWND hWnd, fDataCallBack_v cbRealData, long pUser)
                            :m_lLoginID(loginID)
{
    flag = true;
    m_lhRealPlay = -1;
#if 1
    NET_DVR_PREVIEWINFO struPreviewInfo = {0};//预览参数结构体
    struPreviewInfo.lChannel = lChannel;//通道号
    struPreviewInfo.dwStreamType = dwStreamType;//码流类型
    struPreviewInfo.dwLinkMode = dwLinkMode;//连接方式
    struPreviewInfo.hPlayWnd = hWnd;//播放窗口句柄
    struPreviewInfo.bBlocked = 1;//阻塞取流
    struPreviewInfo.bPassbackRecord = 0;//不启用录像回传
    struPreviewInfo.byPreviewMode = 0;//预览模式：正常预览
    struPreviewInfo.byProtoType = 0;//应用层取流协议：0- 私有协议，1- RTSP协议
    struPreviewInfo.byVideoCodingType = 0;//码流数据编解码类型：0- 通用编码数据，1- 热成像探测器产生的原始数据
    struPreviewInfo.dwDisplayBufNum = dwPlayBufNum;//播放库播放缓冲区最大缓冲帧数，取值范围：1~50，置0时默认为1
    //实时预览（支持多码流）参数：            登陆数据返回值，预览参数，码流数据的回调函数，用户数据
    m_lhRealPlay = NET_DVR_RealPlay_V40(m_lLoginID, &struPreviewInfo, cbRealData, (void*)pUser);
#else
    NET_DVR_CLIENTINFO struRealDataInfo = {0};
    struRealDataInfo.lChannel = lChannel;
    struRealDataInfo.lLinkMode = dwLinkMode;
    struRealDataInfo.hPlayWnd = hWnd;
    m_lhRealPlay = NET_DVR_RealPlay_V30(m_lLoginID,&struRealDataInfo,cbRealData,pUser,1);
#endif
    if(-1 == m_lhRealPlay)
    {
        flag = false;
        sdkLogcout_v();
        //Error_v();
    }
}

bool JF_RealPlay_V::Release()
{
    if(-1 != m_lhRealPlay)
    {
        if(FALSE == NET_DVR_StopRealPlay(m_lhRealPlay))//停止预览 参数：预览句柄
        {
            sdkLogcout_v();
            //Error_v();
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

JF_RealPlay_V::~JF_RealPlay_V()
{}
//设置丢帧数
DWORD JF_RealPlay_V::JF_ThrowBFrame_v(DWORD dwNum)
{
    switch(dwNum)
    {
        case 0:
        case 1:
        case 2:
            break;
        default:
            sdkLog_v->WriteLog("dwNum value out of bounds in 0,1,2");
            //error_v("dwNum value out of bounds in 0,1,2");
            return 1;
    }
    //设置解码时丢弃B帧的个数 参数：登陆返回值，丢弃B帧个数：0 1 2 
    if(FALSE == NET_DVR_ThrowBFrame(m_lhRealPlay,dwNum))
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//设置图像效果
DWORD JF_RealPlay_V::JF_SetVideoEffect_v(JF_VideoEffect* struVidEff, LONG lChannel)
{
    if(-1 == lChannel)
    {
        //设置预览视频显示参数 参数：预览返回值，亮度，对比度，饱和度，色度
        if(FALSE == NET_DVR_ClientSetVideoEffect(m_lhRealPlay,struVidEff->nBrightness, struVidEff->nContrast, struVidEff->nSaturation, struVidEff->nHue))
        {
            sdkLogcout_v();
            //Error_v();
            return -1;
        }
    }
    else
    {
        //设置预览视频显示参数 参数：预览返回值，通道号，亮度，对比度，饱和度，色度
        if(FALSE == NET_DVR_SetVideoEffect(m_lLoginID, lChannel, struVidEff->nBrightness, struVidEff->nContrast, struVidEff->nSaturation, struVidEff->nHue))
        {
            sdkLogcout_v();
            //Error_v();
            return -1;
        }
    }

    return 0;
}
//获取图像效果
DWORD JF_RealPlay_V::JF_GetVideoEffect_v(JF_VideoEffect* struVidEff, LONG lChannel)
{
    if(-1 == lChannel)
    {
        //获取预览视频显示参数 参数：预览返回值，亮度指针，对比度指针，饱和度指针，色度指针
        if(FALSE == NET_DVR_ClientGetVideoEffect(m_lhRealPlay,(unsigned int*)&struVidEff->nBrightness,(unsigned int*)&struVidEff->nContrast,(unsigned int*)&struVidEff->nSaturation,(unsigned int*)&struVidEff->nHue))
        {
            sdkLogcout_v();
            //Error_v();
            return -1;
        }
    }
    else
    {
        //获取预览视频显示参数 参数：预览返回值，通道号，亮度指针，对比度指针，饱和度指针，色度指针
        if(FALSE == NET_DVR_GetVideoEffect(m_lLoginID,lChannel,(unsigned int*)&struVidEff->nBrightness,(unsigned int*)&struVidEff->nContrast,(unsigned int*)&struVidEff->nSaturation,(unsigned int*)&struVidEff->nHue))
        {
            sdkLogcout_v();
            //Error_v();
            return -1;
        }
    }

    return 0;
}
 //打开声音
DWORD JF_RealPlay_V::JF_OpenSound_v()
{
    if(FALSE == NET_DVR_OpenSound(m_lhRealPlay))//独占声卡模式下开启声音 参数：返回值
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}

DWORD JF_RealPlay_V::JF_CloseSound_v()
{
    if(FALSE == NET_DVR_CloseSound())//独占声卡模式下关闭声音
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//调节音量
DWORD JF_RealPlay_V::JF_SetVolume_v(WORD wVolume)
{
    if(0 > wVolume)
    {
        wVolume = 0;
    }
    else if(wVolume > 100)
    {
        wVolume = 100;
    }

    wVolume = wVolume*0xffff/100;
    //调节播放音量 参数：返回值，音量
    if(FALSE == NET_DVR_Volume(m_lhRealPlay,wVolume))
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//保存实时数据
DWORD JF_RealPlay_V::JF_SaveRealData_v(char *sFileName)
{
    if(NULL == sFileName)
    {
        sdkLog_v->WriteLog("sFileName is NULL, can't save file");
        return 1;
    }
    //捕获数据并存放到指定的文件中 参数：返回值,文件路径指针，包括文件名，
    if(FALSE == NET_DVR_SaveRealData(m_lhRealPlay,sFileName))
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//停止实时数据保存
DWORD JF_RealPlay_V::JF_StopSaveRealData_v()
{
    //停止数据捕获
    if(FALSE == NET_DVR_StopSaveRealData(m_lhRealPlay))
    {
        sdkLogcout_v();
        //Error_v();
        return -1;
    }
    return 0;
}
//实时预览抓图
DWORD JF_RealPlay_V::JF_CapturePicture_v(char *pCapture, DWORD dwCaptureMode, DWORD dwPicSize)
{
    if(NULL == pCapture)
    {
        return -3;
    }
    if((0==dwCaptureMode))
    {
        //设置抓图模式 参数：抓图模式 BMP_MODE BMP模式 JPEG_MODE JPEG模式 
        if(FALSE == NET_DVR_SetCapturePictureMode(dwCaptureMode))
        {
            sdkLogcout_v();
            //Error_v();
            return -2;
        }
    }
    if((1==dwCaptureMode))
    {
        //dwCaptureMode = 1;
        //设置抓图模式 参数：抓图模式 BMP_MODE BMP模式 JPEG_MODE JPEG模式
        if(FALSE == NET_DVR_SetCapturePictureMode(dwCaptureMode))
        {
            sdkLogcout_v();
            //Error_v();
            return -2;
        }
    }
    if(strlen(pCapture) != 0)
    {
#if 0
         if(FALSE == NET_DVR_CapturePictureBlock(m_lhRealPlay,pCapture,1000))
#else   //预览时，单帧数据捕获并保存成图片 参数：返回值，保存图象的文件路径（包括文件名）
        if(FALSE == NET_DVR_CapturePicture(m_lhRealPlay,pCapture))
#endif
        {
            sdkLogcout_v();
            //Error_v();
            return -1;
        }
        return 0;
    }
    else
    {
        if(dwPicSize > 0)
        {
            DWORD retSize;
            //预览时抓图并保存在指定内存中 参数：预览句柄，缓冲图片数据的缓冲区，缓冲区大小，实际返回的大小
            /*if(FALSE == NET_DVR_CapturePictureBlock_New(m_lhRealPlay,pCapture,dwPicSize,&retSize))
            {
                Error_v();
                return -1;
            }*/
            return retSize;
        }
        else
        {
            return -4;
        }
    }

}
