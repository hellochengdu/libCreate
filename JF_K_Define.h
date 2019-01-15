#ifndef JF_K_DEFINE
#define JF_K_DEFINE

#ifdef DHNETSDK_H
#define DHNETSDK_H_Val 0
#else
#define DHNETSDK_H_Val 1
#endif

#if DHNETSDK_H_Val
typedef enum EM_LOGIN_SPAC_CAP_TYPE{ 
    EM_LOGIN_SPEC_CAP_TCP               = 0,    // TCP登陆, 默认方式
    EM_LOGIN_SPEC_CAP_ANY               = 1,    // 无条件登陆
    EM_LOGIN_SPEC_CAP_SERVER_CONN       = 2,    // 主动注册的登入
    EM_LOGIN_SPEC_CAP_MULTICAST         = 3,    // 组播登陆
    EM_LOGIN_SPEC_CAP_UDP               = 4,    // UDP方式下的登入 
    EM_LOGIN_SPEC_CAP_MAIN_CONN_ONLY    = 6,    // 只建主连接下的登入
    EM_LOGIN_SPEC_CAP_SSL               = 7,    //  SSL加密方式登陆
    EM_LOGIN_SPEC_CAP_INTELLIGENT_BOX   = 9,    // 登录智能盒远程设备
    EM_LOGIN_SPEC_CAP_NO_CONFIG         = 10,   // 登录设备后不做取配置操作
    EM_LOGIN_SPEC_CAP_U_LOGIN           = 11,   // 用U盾设备的登入
    EM_LOGIN_SPEC_CAP_LDAP              = 12,   // LDAP方式登录
    EM_LOGIN_SPEC_CAP_AD                = 13,   // AD（ActiveDirectory）登录方式
    EM_LOGIN_SPEC_CAP_RADIUS            = 14,   // Radius 登录方式 
    EM_LOGIN_SPEC_CAP_SOCKET_5          = 15,    // Socks5登陆方式
    EM_LOGIN_SPEC_CAP_CLOUD             = 16,   // 云登陆方式
    EM_LOGIN_SPEC_CAP_AUTH_TWICE        = 17,   // 二次鉴权登陆方式
    EM_LOGIN_SPEC_CAP_TS                = 18,   // TS码流客户端登陆方式
    EM_LOGIN_SPEC_CAP_P2P               = 19,   // 为P2P登陆方式
    EM_LOGIN_SPEC_CAP_MOBILE            = 20,   // 手机客户端登陆
    EM_LOGIN_SPEC_CAP_INVALID                   // 无效的登陆方式
};

struct NET_DEVICEINFO_Ex{
    BYTE  sSerialNumber[DH_SERIALNO_LEN];       //序列号 
    int   nAlarmInPortNum;                      //DVR报警输入个数
    int   nAlarmOutPortNum;                     //DVR报警输出个数
    int   nDiskNum;                             //DVR硬盘个数
    int   nDVRType;                             //DVR类型
    int   nChanNum;                             //DVR通道个数
    BYTE  byLimitLoginTime;                     //在线超时时间，为0表示不限制登陆，非0表示限制的分钟数
    BYTE  byLeftLogTimes;                       //当登陆失败原因为密码错误时，通过此参数通知用户，剩余登陆次数，为0时表示此参数无效
    BYTE  bReserved[2];                         //保留字节 
    int   nLockLeftTime;                        //当登陆失败，用户解锁剩余时间（秒数）, -1表示设备未设置该参数 
    char  Reserved[32];                         //保留
};

struct NET_RECORDFILE_INFO{
    unsigned int  ch;               //通道号
    char          filename[128];    //文件名 
    unsigned int  framenum;         //文件总帧数 
    unsigned int  size;             //文件长度
    NET_TIME      starttime;        //开始时间 
    NET_TIME      endtime;          //结束时间
    unsigned int  driveno;          //磁盘号
    unsigned int  startcluster;     //起始簇号
    BYTE          nRecordFileType;  //录象文件类型
    BYTE          bImportantRecID;  //0 普通录象, 1 重要录象 
    BYTE          bHint;            //文件定位索引
    BYTE          bRecType;         //0-主码流录像 1-辅码1流录像 2-辅码流2 3-辅码流3录像
};

struct NET_TIME{
    DWORD  dwYear; 
    DWORD  dwMonth; 
    DWORD  dwDay; 
    DWORD  dwHour;
    DWORD  dwMinute; 
    DWORD  dwSecond;
};

typedef struct{
    DHDEVTIME       time;       //日期 
    unsigned short  type;       //类型
    unsigned char   reserved;   //保留
    unsigned char   data;       //数据
    unsigned char   context[8]; //内容 
}DH_LOG_ITEM;

#endif

#endif // !JF_K_DEFINE
