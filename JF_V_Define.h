#ifndef JF_V_DEFINE
#define JF_V_DEFINE

#ifdef _HC_NET_SDK_H_
#define _HC_NET_SDK_H_Val 0
#else
#define _HC_NET_SDK_H_Val 1
#endif

#if _HC_NET_SDK_H_Val
/*typedef unsigned long       DWORD;
typedef long                LONG ;
typedef unsigned char    BYTE;
typedef unsigned short      WORD;
//#include "../v_inc/HCNetSDK.h"*/
typedef struct{
    DWORD    dwYear;    //年
    DWORD    dwMonth;   //月
    DWORD    dwDay;     //日
    DWORD    dwHour;    //时
    DWORD    dwMinute;  //分
    DWORD    dwSecond;  //秒
}NET_DVR_TIME, *LPNET_DVR_TIME;

typedef struct{
    NET_DVR_DEVICEINFO_V30    struDeviceV30;        //设备参数
    BYTE                      bySupportLock;        //设备是否支持锁定功能
    BYTE                      byRetryLoginTime;     //剩余可尝试登陆的次数，用户名、密码错误时，此参数有效
    BYTE                      byPasswordLevel;      //密码安全等级：0- 无效，1- 默认密码，2- 有效密码，3- 风险较高的密码
    BYTE                      byRes1;               //保留，置为0
    DWORD                     dwSurplusLockTime;    //剩余时间，单位：秒，用户锁定时此参数有效
    BYTE                      byCharEncodeType;     //字符编码类型
    BYTE                      byRes2[255];          //保留，置为0
}NET_DVR_DEVICEINFO_V40,*LPNET_DVR_DEVICEINFO_V40;

typedef struct{
    LONG                              lChannel;                     //通道号
    DWORD                             dwFileType;                   //录象文件类型
    DWORD                             dwIsLocked;                   //是否锁定：0-未锁定文件，1-锁定文件，0xff表示所有文件
    DWORD                             dwUseCardNo;                  //是否带ATM信息进行查询
    BYTE                              sCardNumber[CARDNUM_LEN_OUT]; //dwUseCardNo为1、4时表示卡号，有效字符个数为20；dwUseCardNo为5时表示课程名称
    NET_DVR_TIME                      struStartTime;                //开始时间
    NET_DVR_TIME                      struStopTime;                 //结束时间
    BYTE                              byDrawFrame;                  //是否抽帧：0- 不抽帧，1- 抽帧 
    BYTE                              byFindType;                   //0- 查询普通卷，1- 查询存档卷(适用于CVR设备，普通卷用于通道录像，存档卷用于备份录像)，2- 查询N+1录像文件
    BYTE                              byQuickSearch;                //0- 普通查询，1- 快速（日历）查询
    BYTE                              bySpecialFindInfoType;        //专有查询类型：0-无效，1-带ATM信息的查询
    DWORD                             dwVolumeNum;                  //存档卷号，适用于CVR设备 
    BYTE                              byWorkingDeviceGUID[GUID_LEN];//工作机GUID，通过获取N+1设备信息（NET_DVR_WORKING_DEVICE_INFO）得到，byFindType为2时有效。
    NET_DVR_SPECIAL_FINDINFO_UNION    uSpecialFindInfo;             //专有查询条件联合体
    BYTE                              byStreamType;                 //码流类型：0- 主码流，1- 子码流，2- 三码流，0xff- 全部
    BYTE                              byAudioFile;                  //查找音频文件：0- 不搜索音频文件，1- 搜索音频文件，该功能需要设备支持，启动音频搜索后只搜索音频文件 
    BYTE                              byRes2[30];                   //保留
}NET_DVR_FILECOND_V40, *LPNET_DVR_FILECOND_V40;

typedef struct{
    char             sFileName[100];    //文件名，日历查询时无效
    NET_DVR_TIME     struStartTime;     //文件的开始时间
    NET_DVR_TIME     struStopTime;      //文件的结束时间 
    DWORD            dwFileSize;        //文件大小 
    char             sCardNum[32];      //卡号
    BYTE             byLocked;          //文件是否被锁定，1－文件已锁定；0－文件未锁定
    BYTE             byFileType;        //文件类型
    BYTE             byQuickSearch;     //0- 普通查询结果，1- 快速（日历）查询结果
    BYTE             byRes;             //保留 
    DWORD            dwFileIndex;       //文件索引号
    BYTE             byStreamType;      //码流类型：0- 主码流，1- 子码流，2- 码流三
    BYTE             byRes1[127];       //保留
}NET_DVR_FINDDATA_V40,*LPNET_DVR_FINDDATA_V40;

typedef struct{
    DWORD    dwSize;        //结构体大小
    DWORD    dwChannel;     //通道号
    BYTE     byRes[64];     //保留
}NET_DVR_REMOTECONTROL_COND, *LPNET_DVR_REMOTECONTROL_COND;

typedef struct{
    DWORD    dwSize;                //结构体大小
    DWORD    dwMemoryTotal;         //内存总量，单位：Kbyte
    DWORD    dwMemoryUsage;         //内存使用量，单位：Kbyte 
    BYTE     byCPUUsage;            //CPU使用率：0~100
    BYTE     byMainFrameTemp;       //机箱温度，单位：摄氏度
    BYTE     byBackPanelTemp;       //背板温度，单位：摄氏度 
    BYTE     byRes1;                //保留
    BYTE     byLeftDecResource[32]; /*各解码板剩余解码资源，以解D1分辨率资源为单位，
                                    byLeftDecResource[i]表示槽位号i解码板剩余资源，
                                    0xff表示无效（非解码板或没插板子）*/
    float    fNetworkFlow;          //网络流量，单位：KB/s，保留小数点后两位
    BYTE     byRes[78];             //保留
}NET_DVR_DEVICE_RUN_STATUS,*LPNET_DVR_DEVICE_RUN_STATUS;

typedef struct{
    DWORD    dwSize;            //结构体大小 
    BYTE     byAlarmStatus;     //布防状态：0- 保留，1- 撤防，2- 布防
    BYTE     byRes[3];          //保留
    WORD     wAlarmDealyTime;   //布防延迟时间，单位：s 
    WORD     wDisAlarmDealyTime;//撤防延迟时间，单位：s
    BYTE     byRes1[64];        //保留 
}NET_DVR_REMOTECONTROL_STATUS, *LPNET_DVR_REMOTECONTROL_STATUS;

typedef struct{
    char    sIpV4[16];  //设备IPv4地址 
    BYTE    sIpV6[128]; //设备IPv6地址
}NET_DVR_IPADDR, *LPNET_DVR_IPADDR;
NET_DVR_USER_LOGIN_INFO
typedef struct{
    NET_DVR_TIME     strLogTime;                //日志时间
    DWORD            dwMajorType;               //日志主类型
    DWORD            dwMinorType;               //日志次类型
    BYTE             sPanelUser[MAX_NAMELEN];   //操作面板的用户名
    BYTE             sNetUser[MAX_NAMELEN];     //网络操作的用户名
    NET_DVR_IPADDR   struRemoteHostAddr;        //远程主机地址 
    DWORD            dwParaType;                //
    DWORD            dwChannel;                 //通道号
    DWORD            dwDiskNumber;              //硬盘号 
    DWORD            dwAlarmInPort;             //报警输入端口
    DWORD            dwAlarmOutPort;            //报警输出端口
    DWORD            dwInfoLen;                 //日志附加信息长度 
    char             sInfo[LOG_INFO_LEN];       //日志附加信息
}NET_DVR_LOG_V30,*LPNET_DVR_LOG_V30;

typedef struct{
    BYTE     byUserIDValid;             //userid是否有效：0－无效；1－有效 
    BYTE     bySerialValid;             //序列号是否有效：0－无效；1－有效
    BYTE     byVersionValid;            //版本号是否有效：0－无效；1－有效
    BYTE     byDeviceNameValid;         //设备名字是否有效：0－无效；1－有效 
    BYTE     byMacAddrValid;            //MAC地址是否有效：0－无效；1－有效 
    BYTE     byLinkPortValid;           //Login端口是否有效：0－无效；1－有效
    BYTE     byDeviceIPValid;           //设备IP是否有效：0－无效；1－有效 
    BYTE     bySocketIPValid;           //Socket IP是否有效：0-无效；1-有效 
    LONG     lUserID;                   //NET_DVR_Login或NET_DVR_Login_V30返回值, 布防时有效
    BYTE     sSerialNumber[SERIALNO_LEN];//序列号
    DWORD    dwDeviceVersion;           //版本信息：V3.0以上版本支持的设备最高8位为主版本号，次高8位为次版本号，低16位为修复版本号；V3.0以下版本支持的设备高16位表示主版本，低16位表示次版本
    char     sDeviceName[NAME_LEN];     //设备名称
    BYTE     byMacAddr[MACADDR_LEN];    //MAC地址
    WORD     wLinkPort;                 //设备通讯端口
    char     sDeviceIP[128];            //设备IP地址
    char     sSocketIP[128];            //报警主动上传时的Socket IP地址
    BYTE     byIpProtocol;              //IP协议：0－IPV4；1－IPV6
    BYTE     byRes2[11];                //保留，置为0
}NET_DVR_ALARMER,*LPNET_DVR_ALARMER;

typedef struct{
    DWORD           dwChannel;              //通道号
    NET_DVR_TIME    struStartTime;          //开始时间
    NET_DVR_TIME    struStopTime;           //结束时间
    BYTE            byDrawFrame;            //是否抽帧：0- 不抽帧，1- 抽帧
    BYTE            byStreamType;           //码流类型：0- 主码流，1- 子码流，2- 码流三
    BYTE            byStreamID[STREAM_ID_LEN];// 流ID，使用流ID方式时dwChannel设为0xffffffff
    BYTE            byRes[30];              //保留
}NET_DVR_PLAYCOND, *LPNET_DVR_PLAYCOND;

#endif // !_HC_NET_SDK_H_

#endif // !JF_V_DEFINE
