#ifndef JF_DEFINE
#define JF_DEFINE

#include <time.h>
#include <semaphore.h>
#include <QtCore/QTextCodec>
#include <unistd.h>
#include "../k_inc/dhnetsdk.h"
#include "../k_inc/dhconfigsdk.h"
#include "../v_inc/HCNetSDK.h"
//#include "../k_inc/dhplay.h"
#include "../v_inc/Sadp.h"
#define HadDefine 0
typedef struct {
    char szIP[64];
    char szSubmask[64];
    char szGateway[64];
    char szMac[40];
    int  nPort;
    char szSerialNo[48];
    char szDeviceType[32];
    char szDevSoftVersion[128];
    char szDSPVersion[48];
    char DeviceStyle;
    long dwuser;
}JF_SadpDevice;//
typedef struct {
    char szIP[64];
    char szMac[256];
    char UserName[64];   //登录的用户名
    char Passwd[64];     //用户密码
    int  nPort;
    char osdContent[128];//通道标签
    char szSerialNo[48]; //序列号
    char deviceType[64];
    bool linkState;//连接状态
    bool vtState;//录像状态
}JF_DeviceInfo;
typedef struct  {
    CFG_VIDEO_COMPRESSION emCompression;//视频压缩格式.有如下格式 "MPEG4" "MS-MPEG4" "MPEG2" "MPEG1" "H.263" "MJPG" "FCC-MPEG4" "H.264"
    int nWidth;                          //视频宽度，利用atoi和itoa转换
    int nHeight;                         //视频高度
    CFG_BITRATE_CONTROL emBitRateControl;//码流控制模式.
                                                     /* 字符串值	码流控制模式
                                                        "CBR	固定码流
                                                        "VBR"	可变码流*/
    int nBitRate;                        //视频码流(kbps)
    int nFrameRate;                      //视频帧率
    int nIFrameInterval;                 //I帧间隔
    CFG_IMAGE_QUALITY emImageQuality;    //图像质量，只有在可变码流下才起作用。取值1-6，值越大，图像质量越好
    int nFrameType;                      //打包模式。
                                                   /*字符串值	打包模式
                                                    "DHAV"	"DHAV"头
                                                      "PS"	PS流*/

}Video_Format;
typedef struct {
    CFG_AUDIO_FORMAT    emCompression;//音频压缩模式，CFG_AUDIO_FORMAT具体定义参考下表
                                                        /*             枚举值	对应音频格式
                                                      AUDIO_FORMAT_G711A = 0	G711a
                                                        AUDIO_FORMAT_PCM = 1	PCM
                                                      AUDIO_FORMAT_G711U = 2	G711u
                                                        AUDIO_FORMAT_AMR = 3	AMR
                                                        AUDIO_FORMAT_AAC = 4	AAC*/
    AV_int32             nDepth;       //音频采样深度
    AV_int32             nFrequency;   //音频采样频率
    AV_int32             nMode;        //音频编码模式
    AV_int32             nFrameType;   //音频打包模式, 0-DHAV, 1-PS
    AV_int32             nPacketPeriod;//音频打包周期, ms
}Audio_Format;
typedef struct AV_Color
{
    AV_int32			nRed;							// 红
    AV_int32			nGreen;							// 绿
    AV_int32			nBlue;							// 蓝
    AV_int32			nAlpha;							// 透明
};
typedef struct AV_Rect
{
    AV_int32			nLeft;
    AV_int32			nTop;
    AV_int32			nRight;
    AV_int32			nBottom;
};
typedef struct {
    AV_BOOL         bEncodeBlend;          //叠加到主码流
    AV_BOOL         bEncodeBlendExtra1;    //叠加到辅码流1
    AV_BOOL         bEncodeBlendExtra2;    //叠加到辅码流2
    AV_BOOL         bEncodeBlendExtra3;    //叠加到辅码流3
    AV_BOOL         bEncodeBlendSnapshot;  //叠加到抓图
    AV_Color        stuFrontColor;         //前景色
    AV_Color        stuBackColor;          //背景色
    AV_Rect         stuRect;               //区域, 坐标取值0~8191, 仅使用left和top值, 点(left,top)应和(right,bottom)设置成同样的点
    AV_BOOL         bPreviewBlend;         //叠加到预览视频
}AV_VideoWidget;
typedef struct {
    AV_BOOL         bEncodeBlend;
    AV_BOOL         bEncodeBlendExtra1;
    AV_BOOL         bEncodeBlendExtra2;
    AV_BOOL         bEncodeBlendExtra3;
    AV_BOOL         bEncodeBlendSnapshot;
    AV_Color        stuFrontColor;
    AV_Color        stuBackColor;
    AV_Rect         stuRect;
    AV_BOOL         bPreviewBlend;
}AV_VideoTime;
typedef struct {
    char szIP[64];
    char UserName[64];   //登录的用户名
    char Passwd[64];     //用户密码
    int  nPort;
}Login_Format;
typedef struct JF_VideoInfo{
    Video_Format vF;      //视频编码
    Audio_Format aF;      //音频编码
    AV_VideoWidget avm;   //OSD信息
    AV_VideoTime avt;     //时间信息
    Login_Format lF;      //账号信息
}JF_VideoInfo;
typedef struct{
    int info;//类型0-读取 1-设置
    int DevInfo;// 设备设置/读取类型0-账号密码IP端口 1-编码信息 2-osd信息
    JF_VideoInfo*  DevCont;
}JF_CameraInfo;
typedef struct {
    int nUsage;//CPU利用率；
}JF_CPUInfo;
typedef struct {
    DWORD dwTotal;//总内存
    DWORD dwFree;//剩余内存
}JF_MemoryInfo;
typedef  struct {
    DWORD dwVolume;//硬盘容量
    DWORD dwFreeSpace;//硬盘的剩余空间
}JF_DiskState;
typedef struct {
    DWORD dwDiskNum;//硬盘或者分区个数
    JF_DiskState stDisks[32];
}JF_HardDisk;
typedef struct {
    BYTE DeviceType[64];
    int nCount;//CPU数量
    JF_CPUInfo stuCPUs[16];//CPU信息
    JF_MemoryInfo stuMemory;//内存信息
    JF_HardDisk diskInfo;//硬盘信息
    char  szSoftWareVersion[128];
}JF_Details;
typedef  struct {
    int                 nChannel; //通道号 0xff 表示全部通道
    NET_TIME_EX         stuStartTime;//开始时间
    NET_TIME_EX         stuEndTime;  //结束时间
    BOOL                bFlag;      //flase 查询   true 设置
}JF_MarkTImeInfo;
//                  文件查找回调函数    通道号            文件个数        文件信息缓冲区
typedef void (CALLBACK *fFindFileLog)(DWORD dwChannel, DWORD dwFileNum, char* lpFileInfo,LDWORD userData);
typedef void* (CALLBACK * fSadpDevice)(void* device);

typedef struct {
	char*	pchDVRIP;
	LONG 	lDVRPort;
    char    deviceType[10];
}AlarmDVRInfo;

//                  通用报警消息回调              通道号             报警设备信息         报警消息缓冲区     消息缓冲区大小  用户自定义数据
typedef void (CALLBACK *fAlarmMSGCallBack)(LONG lCommand, AlarmDVRInfo *alarmDVRInfo, void* pAlarmBuf, DWORD dwBufLen, void* pUser);
typedef void(CALLBACK *fMarkConfigPosCallBack)(LLONG  lPlayHandle, DWORD  dwTotalSize, DWORD  dwDownLoadSize, LDWORD dwUser);

//                  通用数据回调              句柄          数据类型         数据缓冲区        缓冲区大小       用户自定义数据
typedef void (CALLBACK *fDataCallBacks)(LLONG lhPlay, DWORD dwDataType, char* pDataBuf, DWORD dwBufSize, void* pUserData);

//					通用语音数据回调			语音对讲句柄				音频数据缓冲区			语音数据大小	音频数据类型（0本地 1外来）  用户数据
typedef void(CALLBACK *fVoiceDataCallBack)(LONG lVoiceComHandle, char *pRecvDataBuffer, DWORD dwBufSize, BYTE byAudioFlag, DWORD dwUser);

//----------K
//                  断线回调函数          登录句柄        设备IP            设备端口        用户数据
typedef void (CALLBACK *fDisConnect_k)(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
//                  断线重连回调函数        登录句柄        当前设备ip     当前设备端口号   用户自定义数据
typedef void(CALLBACK *fHaveReConnect_k)(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
//                  消息回调函数          回调类型            登录句柄    接受警报缓冲区  缓冲区长度           设备IP        设备端口        用户数据
typedef BOOL (CALLBACK *fMSGCallBack_k)(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG lDVRPort, LDWORD dwUser);
//                  服务器消息回调函数   服务器消息回调句柄   设备IP    设备端口        报警类型    报警信息缓冲区     缓冲区长度       用户回调自定义数据
typedef int (CALLBACK *fServiceCallBack_k)(LLONG lHandle, char *pIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, LDWORD dwUserData);
//                  数据回调                句柄            数据类型，为0     数据缓冲区         缓冲区长度           用户数据
typedef int(CALLBACK *fDataCallBack_k)(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufferSize, LDWORD dwUser);
//                  进度回调                       句柄                总大小              当前大小         用户自定义数据
typedef void(CALLBACK *fDownLoadPosCallBack_k)(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, LDWORD dwUser);
typedef fDownLoadPosCallBack_k fPlayBackPosCallBack_k;
//-------

//----------V
//					消息回调			消息类型			报警设备信息				报警信息	   报警信息缓存大小		用户数据
typedef void(CALLBACK *fMSGCallBack_v)(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void *pUser);
//					数据回调					句柄				数据类型		数据缓冲区		缓冲区大小		用户数据
typedef void(CALLBACK *fDataCallBack_v)(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
//-------

//=======================================
typedef struct {
    //DH
    DHDEV_CHANNEL_CFG struChannelCfg_k;
    //HK
    NET_DVR_PICCFG_V40 struPictureCfg_v;
}JF_ParamVideo;//录像视频参数

typedef struct {
    //DH 无
    //HK
    NET_DVR_COMPRESSIONCFG_V30 struCompressCfg_v;
}JF_Compress_MAIN;//压缩参数

typedef struct {
    //DH
    DHDEV_RECORD_CFG struRecordCfg_k;
    //HK
    NET_DVR_RECORD_V40 struRecordCfg_v;
}JF_ParamRecord;//定时录像参数

typedef struct {
    //DH
    DH_MOTION_DETECT_CFG_EX struMotionCfg_k;
    //HK
    NET_DVR_HOLIDAY_HANDLE  struMotionCfg_v;
}JF_ParamMotion;//运动检测

typedef struct {
    //DH
    DH_ALARMIN_CFG_EX struAlarmCfg_k;
    //HK
    NET_IPC_AUX_ALARMCFG struAlarmCfg_v;
}JF_ParamAlarm; //报警设置

typedef struct {
    //DH
    DH_VIDEO_LOST_CFG_EX struVideoLostCfg_k;
    //HK
    NET_DVR_HOLIDAY_HANDLE struVideoLostCfg_v;
}JF_VideoLost;//视频丢失

typedef struct {
    //DH
    DH_BLIND_CFG_EX struBlindCfg_k;
    //HK
    NET_DVR_HOLIDAY_HANDLE struBlindCfg_v;
}JF_BlindAlarm;//遮挡报警


#if 0
//========================================
//视频参数--0
typedef struct   						//Schedle time
{
	unsigned short		iStartHour;
	unsigned short		iStartMin;
	unsigned short		iStopHour;
	unsigned short		iStopMin;
	unsigned short		iRecordMode;    //iEnable;
}NVS_SCHEDTIME,*PNVS_SCHEDTIME;

typedef struct
{
	unsigned short 		m_u16Brightness;
	unsigned short 		m_u16Hue;
	unsigned short 		m_u16Contrast;
	unsigned short 		m_u16Saturation;
	NVS_SCHEDTIME       strctTempletTime;
}STR_VideoParam;

typedef struct OSDTYPE
{
	int   PosX;     	//当_iPositionY >= 0时，为X坐标；当_iPositionY 为-1时，为位置编号 0~15 
	int   PosY;     	//Y坐标，当为-1时，_iPositionX是位置编号
	int   Enabled;   	//获得字符叠加状态
}OSDTYPE;
//录像文本信息坐标
typedef struct TEXT_ON_VIDEO
{
	int   PosX;     	//显示文本信息的起始X坐标
	int   PosY;     	//显示文本信息的起始Y坐标
	char  Text[256];    //要显示的文本信息，必须包含结束符\0,\n表示换行
}TEXT_ON_VIDEO;

#if HadDefine
typedef struct {
    long  left;
    long  top;
    long  right;
    long  bottom;
}RECT;
#endif
//录像视频参数
typedef struct ParamVideo
{
	STR_VideoParam  struSTR_VideoParam;	//NVS_SCHEDTIME_User  lpNVS_SCHEDTIME_User;//时间模板
	char  			OSDText[64];  		//字符叠加内容
	int   			TextColor;    		//字符叠加颜色
	int   			OsdType;  			//字符叠加类型 0-预览叠加 1-编码叠加
					/*
						OSDTYPE_TIME	0x01 叠加时间 
						OSDTYPE_TITLE	0x02 叠加字符串 
						OSDTYPE_LOGO	0x04 叠加标志
					*/
	OSDTYPE   		_OSDTYPE_TIME;    	//叠加时间
	OSDTYPE   		_OSDTYPE_TITLE;   	//叠加字符串
	OSDTYPE   		_OSDTYPE_LOGO;    	//叠加标志

	/*int  Format;/*0：yyyy/mm/dd    	2007/12/26
				1：mm/dd/yyyy      		12/26/2007
				2：dd/mm/yyyy      		26/12/2007
				3：M.day yyyy       		Dec 26 2007
				4：day M.yyyy       		26 Dec 2007
	*/
	//char  Separate;	分割符 ('/' 或 '-' 或 '.')
	int   			Diaphaneity;  		//透明度0-100
	RECT  			_rect[4]; 			//视频遮挡区域
	TEXT_ON_VIDEO 	_TEXT_ON_VIDEO;   	//通道上显示文本信息,最多255个
}ParamVideo;
//------------------------------

//压缩参数--1
typedef struct Compress_MAIN
{
	int    RecordQualityLevel;	////4~14  数值越大视频质量越差
	int    FrameRate;           //视频压缩帧率：1~25    PAL制；1~30    NTSC制
	int    StreamType;			//流类型,1 视频流 2 音频流 3 音视频流 
	int	   Mode;				//0(画质优先)/1(帧率优先)
	int    BitRate;				//码流K字节率
	int    VideoSize;		   	//分辨率 QCIF 0 Quarter Cif 172*144 
								/*HCIF1 Half Cif 352*144 
									FCIF
										2 Full Cif 352*288 
									HD1
										3 Half D1 720*288 
									FD1
										4 Full D1 720*576 
									QVGA
										6 QVGA 320x240 
									VGA
										7 VGA 640*480 
									HVGA
										8 HVGA 640*240 
									HD_720P
										9 720p 1280*720	
								*/	

	int    VBR;					//码流类型				//1 VBR，0 CBR
	//int    Vmode;				//制式视频制式 0   PAL，  1  NTSC， 2  自动识别
	int    aCoder;				//音频编码方式G711_A 0x01 64kbps G.711 A, see RFC3551.txt 4.5.14 PCMA 
								/*  G711_U 			 0x02 64kbps G.711 U, see RFC3551.txt 4.5.14 PCMU 
									ADPCM_DVI4 		 0x03 32kbps ADPCM(DVI4) 
									G726_24KBPS 	 0x05 24kbps G.726, see RFC3551.txt 4.5.4 G726-24 
									G726_32KBPS 	0x06  32kbps G.726, see RFC3551.txt 4.5.4 G726-32 
									G726_40KBPS 	0x07 40kbps G.726, see RFC3551.txt 4.5.4 G726-40 
									MPEG_LAYER2 	0x0E Mpeg layer 2 
									AMR_NB 		    0x15 AMR_narrowband 窄带AMR 
									ADPCM_IMA 		0x23 32kbps ADPCM(IMA) 
									MEDIA_G726_16KBPS 	0x24 G726 16kbps for ASF ... 
									MEDIA_G726_24KBPS 	0x25 G726 24kbps for ASF ... 
									MEDIA_G726_32KBPS 	0x26 G726 32kbps for ASF ... 
									MEDIA_G726_40KBPS 	0x27 G726 40kbps for ASF ...
								*/

	int    IFrameRate;			//I帧帧率

}Compress_MAIN;
//------------------------------

//录像计划--2
#define  MAX_TIMESEGMENT 4
typedef struct ParamRecord
{
	int 			Alarm_State;    	//报警录像状态，1启动报警录像，0停止报警录像
	int 			Time_State;     	//定时录像状态，1启动定时录像，0停止定时录像
	int 			Now_State;      	//前端录像状态，REC_STOP=0，REC_MANUAL=1，REC_TIME=2， REC_ALARM=3
	NVS_SCHEDTIME   _Schedule_0[7][MAX_TIMESEGMENT];
}ParamRecord;
//---------------------------------

//运动检测--3
typedef struct ParamMotion
{
	int 			Threshold;			//视频移动报警的灵敏度 0~255
	int				Enabled;			//获得视频移动检测状态使能有效或无效。
	char 			_posxy[22][18];  	//22*18块区域的有效值0x01-有效0x00-无效 
	int 			RecEnabled;			//获得视频移动报警联动录像使能按位操作,从右至左依次表示1-32个通道
	int 			SnapEnabled;		//获得视频移动报警联动抓拍	按位操作,从右至左依次表示1-32个通道
	int 			SoundEnable;		//声音开启：0：停止；1：启用
	int 			DisplayEnable;		//显示开启：0：停止；1：启用
	int 			OutportArray;		//获得视频移动报警联动输出端口按位操作，从右至左每一位代表一个输出端口
	NVS_SCHEDTIME   _Schedule_0[7][MAX_TIMESEGMENT];
										//时间模板，支持最大4个时间段
}ParamMotion;
//---------------------------------

//报警设置--4
typedef struct ParamAlarm
{
	int 			iAlarmInput;				//报警输入端口(查询要填)
	unsigned int	iAlarmOutput;				//报警输出联动设置,其中每一位对应一个报警输出端口：为1表示该通道联动此端口，为0表示该通道不联动此端口
	int 			iEnabled;					//0：不可用，1：可用
	int  			iPortNo;					//输入端口号(查询要填)
	int 			iEnabled_PortNo_Rec;		//获得输入端口报警联动录像。按位操作,从右至左依次表示1-32个通道
	int 			iEnabled_PortNo_Snap;		//获得输入端口报警联动抓拍。按位操作,从右至左依次表示1-32个通道
	int  			iLinkChannelNum;			//联动的通道(查询要填)
	int 			iLinkType;					//联动的类型：0，不联动；1，联动预置位；2，联动轨迹；3，联动路径
	int 			iActNum;					//联动的编号
	int 			_iSoundEnable;				//获得输入端口报警联动声音提示。[OUT]声音开启：0：停止；1：启用
	int 			_iDisplayEnable;			//和屏幕显示[OUT]显示开启：0：停止；1：启用
	int 			_iOutportArra;				//获得端口输入报警联动输出端口。[OUT]按位操作，从右至左每一位代表一个输出端口
	int				_iEnable_Sch_In;			//[OUT]1：启用；0：停用获得报警端口输入布防模板使能
	int 			_iEnable_Sch_Out;			//[OUT]1：启用；0：停用获得报警输出端口布防模板使能
	NVS_SCHEDTIME   _Schedule_0[7][MAX_TIMESEGMENT];	//获得端口报警布防模板。时间模板，支持最大4个时间段 
	NVS_SCHEDTIME   _Schedule_1[7][MAX_TIMESEGMENT];	//获得报警输出布防模板。时间模板，支持最大4个时间段
}ParamAlarm;
//---------------------------------

//远程压缩参数--5
typedef struct Compress_SUB
{
	int    RecordQualityLevel;	//4~14  数值越大视频质量越差
	int    FrameRate;           //视频压缩帧率：1~25    PAL制；1~30    NTSC制
	int    StreamType;			//流类型,1 视频流 2 音频流 3 音视频流 
	int	   Mode;				//0(画质优先)/1(帧率优先)
	int    BitRate;				//码流K字节率
	int    VideoSize;		   	//分辨率 QCIF 0 Quarter Cif 172*144 
								/*
									HCIF1 Half Cif 352*144 
									FCIF
										2 Full Cif 352*288 
									HD1
										3 Half D1 720*288 
									FD1
										4 Full D1 720*576 
									QVGA
										6 QVGA 320x240 
									VGA
										7 VGA 640*480 
									HVGA
										8 HVGA 640*240 
									HD_720P
										9 720p 1280*720	
								*/

	int    	VBR;				//码流类型	< 1 VBR， 0 CBR >
	int   	IFrameRate;			//I帧帧率
}Compress_SUB;
//---------------------------------

//视频丢失和遮挡报警-6
typedef struct Video_Alarm
{
	int   _iThreshold;			//	[OUT]视频遮挡报警灵敏度0~255
	int   _iEnabled;     		//	[OUT]1 该通道进行视频遮挡报警；0 该通道不进行视频遮挡报警
								//	获得视频丢失报警
	int   _iEnabled_lost;    	//	[OUT]开始/停止视频丢失报警检测
	int   _iLinkChannelNum;		//	[查询要填]ptz联动的通道
	int   _iLinkType;   		//	[OUT]联动的类型：0，不联动该通道PTZ；1，预置位；2，轨迹；3，路径
	int   _iActNum;     		//	[OUT]联动的编号
	int   _iSoundEnable;    	//	[OUT]声音开启：0：停止；1：启用
	int   _iDisplayEnable;  	//	[OUT]显示开启：0：停止；1：启用
	int   _iOutportArray;   	//	[OUT]输出端口按位操作，从右至左每一位代表一个输出端口
	int   _iEnable_Rec;     	//	[OUT]录像按位操作,从右至左依次表示1-32个通道
	int   _iEnable_Snap;    	//	[OUT]抓图按位操作,从右至左依次表示1-32个通道
	NVS_SCHEDTIME	_Schedule_0[7][MAX_TIMESEGMENT];
								//获得视频丢失报警检测模板。时间模板，支持最大4个时间段 
}Video_Alarm;
#endif
//---------------------------------

#define LoginType 0
#if LoginType
typedef enum EM_LOGIN_TYPE { 
    EM_LOGIN_SPEC_CAP_TCP               = 0,     // TCP登陆, 默认方式
    EM_LOGIN_SPEC_CAP_ANY               = 1,     // 无条件登陆
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
#endif

typedef struct {
    DWORD   tm_year;
    DWORD   tm_mon;
    DWORD   tm_mday;
    DWORD   tm_hour;
    DWORD   tm_min;
    DWORD   tm_sec;
}JF_Timer;

class JF_Users;
typedef struct
{
    LLONG       lLoginID;       //登录ID
    char        DVRIP[128];     //设备IP地址
    int         DVRPort;        //设备端口
    int         DVRType;        //设备类型
    char        UserName[64];   //登录的用户名
    char        Passwd[64];     //用户密码
    int         companyType;    //厂商类型
    BYTE        szDevType[32];  //设备型号
    JF_Users*   pUsers;         //用户操作类对象
    char        szDefGateway[256];//网关
    char        szMacAddress[256];//mac
    char        sSerialNumber[48];//序列号
    char        szSoftWareVersion[128];//软件版本号

}JF_UserIDInfo;

typedef struct {
    char  sSerialNumber[48];   //序列号
    int   nAlarmInPortNum;     //报警输入个数
    int   nAlarmOutPortNum;    //报警输出个数
    int   nDiskNum;            //硬盘个数
    int   nDVRType;            //DVR类型
    int   nChanNum;            //通道个数
}JF_DVRInfo;

typedef struct{
    char 	FileName[128];		//文件名
    JF_Timer 	StartTime;		//文件开始时间
    DWORD 	TimeLen;			//文件可以回放多少秒
    DWORD 	FileLen;			//文件字节数
}JF_FileInfo;

typedef struct {
    JF_Timer 	StartTime;		//日志记录的时间
    DWORD 	Type;				//日志的类型
    char  	LogContent[128]; 	//日志的内容
    DWORD   dwInfoLen;          //日志内容长度
}JF_LogInfo;

typedef struct {
    unsigned int nBrightness;      //亮度
    unsigned int nContrast;        //对比度
    unsigned int nHue;             //色度
    unsigned int nSaturation;      //饱和度
    unsigned char kBrightness;
    unsigned char kContrast;
    unsigned char kHue;
    unsigned char kSaturation;
}JF_VideoEffect;
class JF_SDK;
class JF_SDK_K;
class JF_SDK_V;
typedef struct {
    LLONG loginID;
    int   companyType;
    LONG lChannel;
    DWORD dwFileType;
    JF_Timer *stime;
    JF_Timer *etime;
    fFindFileLog cbFindFile;
    BYTE byStreamType_v;
    LLONG *lhFindFile;
    sem_t *sem;
    long UserData;
    JF_Users *pUser;
    JF_SDK_K *pk;
    JF_SDK_V *pv;
}JF_FindFileArgs;

typedef struct {
    //DH
    int     logType;            //日志类型
    /*
        0		所有日志 
        1	 	系统日志 
        2		配置日志 
        3	 	存储相关 
        4	 	报警日志 
        5	 	录象相关 
        6	 	帐号相关 
        7	 	清除日志 
        8	 	回放相关 
        9	 	前端管理运行相关
    */
    char    *pLogBuffer;        //获取日志内容缓冲区，其大小为 nLogNum*sizeof(JF_LogInfo)
    int     nLogNum;            //要查询的日志条数，其值为 pLogBuffer的大小除以 sizeof(JF_LogInfo)

    //HK
    LONG    lSelectMode;    //查询方式  0-全部 1-按类型 2-按时间 3-按时间和类型
    DWORD   dwMajorType;    //日志主类型
    /*
        1 		报警 
        2 		异常 
        3 		操作 
        4 		日志附加信息 
        5 		事件
    */
    DWORD   dwMinorType;    //日志次类型
    JF_Timer *stime;   //开始时间
    JF_Timer *etime;   //结束时间
    fFindFileLog cbFindLog_v;
    LDWORD userData;
    LONG    lhFindLog;
}JF_FindLogArgs;
typedef void(CALLBACK * DVRLoginCallBack)(void*pDeviceInfo,JF_Users *User,LDWORD* dwUserData,long dwError);
typedef struct {
    char pchDVRIP[32];
	WORD wDVRPort;
    char pchUserName[64];
    char pchPasswd[64];
    LDWORD* dwUser;
    JF_DVRInfo* pDVRInfo;
	JF_SDK* pUsers;
    DVRLoginCallBack callBack;
    char Dtype[3];
}JF_Login_Struct;
//==============================
#if HadDefine
typedef struct {
    int channelcount;                   //通道数
    int alarminputcount;                //报警输入口数
    unsigned char  alarm[32];           //外部报警
    unsigned char  motiondection[32];   //动态检测
    unsigned char  videolost[32];       //视频丢失
    BYTE           bReserved[32];       //扩展字段
}NET_CLIENT_STATE_EX;

typedef struct {
    DWORD  dwVolume;        //硬盘的容量
    DWORD  dwFreeSpace;     //硬盘的剩余空间
    BYTE   dwStatus;        //高四位的值表示硬盘类型,低四位的值表示硬盘的状态，0-休眠,1-活动,2-故障等
    /*
        DH_DISK_READ_WRITE = 0  读写驱动器
        DH_DISK_READ_ONLY = 1   只读驱动器
        DH_DISK_BACKUP = 2      备份驱动器或媒体驱动器
        DH_DISK_REDUNDANT = 3   冗余驱动器
        DH_DISK_SNAPSHOT = 4    快照驱动器
    */
    BYTE   bDiskNum;        //硬盘号
    BYTE   bSubareaNum;     //分区号
    BYTE   bSignal;         //标识，0为本地 1为远程
}NET_DEV_DISKSTATE;

typedef struct {
    DWORD             dwDiskNum;    //硬盘或者分区个数个数
    NET_DEV_DISKSTATE stDisks[DH_MAX_DISKNUM];//硬盘或分区信息
}DH_HARDDISK_STATE;
#endif
//-----------
/*
typedef struct {
    NET_CLIENT_STATE_EX     client_state;   //普通报警状态信息
    DH_HARDDISK_STATE       disk_state;     //磁盘状态信息
    unsigned char           shelterAlarm[16];//遮挡报警状态
    unsigned char           recording[16];  //录象状态
    DWORD                   resource[3];    //查询系统资源状态，3个DWORD，第一个dword 0 表示小于CPU超过50% 1 表示大于50%，第二个表示TV，第三个表示VGA
}JF_V_DVRState;
*/
//----------------------
#if HadDefine
typedef struct{
    DWORD    dwSize;                //结构体大小
    DWORD    dwMemoryTotal;         //内存总量，单位：Kbyte
    DWORD    dwMemoryUsage;         //内存使用量，单位：Kbyte
    BYTE     byCPUUsage;            //CPU使用率：0~100
    BYTE     byMainFrameTemp;       //机箱温度，单位：摄氏
    BYTE     byBackPanelTemp;       //背板温度，单位：摄氏度
    BYTE     byRes1;                //保留
    BYTE     byLeftDecResource[32]; //各解码板剩余解码资源，以解D1分辨率资源为单位，byLeftDecResource[i]表示槽位号i解码板剩余资源，0xff表示无效（非解码板或没插板子
    float    fNetworkFlow;          //网络流量，单位：KB/s，保留小数点后两位
    BYTE     byRes[78];             //保留
}NET_DVR_DEVICE_RUN_STATUS,*LPNET_DVR_DEVICE_RUN_STATUS;
#endif
//-----------
/*
typedef struct {
    NET_DVR_DEVICE_RUN_STATUS   device_state;   //获取设备运行状态
    unsigned int                audio_channel_state;//获取语音对讲状态，1个4字节状态(0-未开启，1-开启)
}JF_K_DVRState;

//----------------------
typedef struct {
    JF_V_DVRState   V_State;
    JF_K_DVRState   K_State;
}JF_DVRState;
*/
typedef struct {
    //DH
    NET_CLIENT_STATE_EX     client_state;   //普通报警状态信息
    DH_HARDDISK_STATE       disk_state;     //磁盘状态信息
    unsigned char           shelterAlarm[16];//遮挡报警状态
    unsigned char           recording[16];  //录象状态
    DWORD                   resource[3];    //查询系统资源状态，3个DWORD，第一个dword 0 表示小于CPU超过50% 1 表示大于50%，第二个表示TV，第三个表示VGA

    //HK
    NET_DVR_DEVICE_RUN_STATUS   device_state;   //获取设备运行状态
    unsigned int                audio_channel_state;//获取语音对讲状态，1个4字节状态(0-未开启，1-开启)
}JF_DVRState;
//===============================

typedef struct {
    unsigned int  ch;               //通道号
    char          filename[128];    //文件名
    unsigned int  framenum;         //帧数
    unsigned int  size;             //文件长度
    NET_TIME      starttime;        //开始时间
    NET_TIME      endtime;          //结束时间
    unsigned int  driveno;          //磁盘号
    unsigned int  startcluster;     //起始簇
    BYTE          nRecordFileType;  //文件类型 0 普通录象 1 报警录象 2 移动检测 3 卡号录象 4 图片 
    BYTE          bImportantRecID;  //重要程度 0 普通录象 1 重要录象 
    BYTE          bHint;            //文件定位索引
    BYTE          bRecType;         //码流类型 0-主码流录像 1-辅码1流录像 2-辅码流2 3-辅码流3录像
}JF_RecordFileInfo; //与DH查询的文件信息结构体兼容

typedef struct {
    fDownLoadPosCallBack_k  cbFileDownloadPos;  //文件下载进度回调函数
    LDWORD                  dwPosUserData;      //用户自定义进度回调数据
    fDataCallBack_k         cbFileDownloadData; //数据回调函数
    LDWORD                  dwDataUser;         //用户自定义数据回调数据
}JF_K_DownloadFileArgs;

typedef struct {
	DWORD dwPlayBufNum; //播放缓冲区个数,至少1个
	DWORD dwStreamType; //流类型 0-主码流，3-子码流，4-码流3，5-虚拟码流
	DWORD dwLinkMode;   //连接方式 0- TCP方式，1- UDP方式，2- 多播方式
}JF_V_RealPlayArgs;
typedef struct{
    //HK
    LPNET_DVR_FINDDATA_V40 lpFindData_HK;
    //DH
    NET_RECORDFILE_INFO  lpFindData_DH;
}JF_RecodeFile_Info;
typedef union tagJF_DevConfig{
    NET_DVR_PICCFG_V40         StructPiccfg_v;//HK        图像参数结构体
    NET_DVR_COMPRESSIONCFG_V30 StructCompressioncfg_v;//  压缩参数
    NET_DVR_RECORD_V40         StructRecord_v;        //  录像计划参数
    NET_DVR_JPEG_CAPTURE_CFG   StructJpegCapture_v;   //  设备抓图配置
    NET_DVR_SCHED_CAPTURECFG   StructSched_v;         //  抓图计划
    NET_DVR_SHOWSTRING_V30     StructShowString_v;    //  叠加字符参数
    NET_DVR_CAMERAPARAMCFG_EX  StructCameraparamcfg_v;//  前端参数(扩展)
    NET_DVR_ISP_CAMERAPARAMCFG StructIspCameraparamcfg_v; //ISP前端参数配置
    NET_IPC_AUX_ALARMCFG       StructAuxAlarmcfg_v;   //  辅助(PIR/无线)报警参数
    NET_DVR_VIDEO_INPUT_EFFECT StructVideoInput_v;    //  通道视频输入图像参数
    NET_DVR_HOLIDAY_HANDLE     StructHoliDay_v;       //  移动侦测假日报警处理方式
    NET_DVR_HOLIDAY_RECORD     StructHoliDayRecord_v; //  假日录像参数
    NET_DVR_LINK_STATUS        StructLink_v;          //  通道的工作状态
    NET_DVR_CHAN_GROUP_RECORD_STATUS StructChan_v;    //  通道录像状态信息
    NET_DVR_WD1_CFG            StructWd1_v;           //  WD1使能开关状态
    NET_DVR_STREAM_CABAC       StructStream_v;        //  码流压缩性能选项
    NET_DVR_ACCESS_CAMERA_INFO StructAccess_v;        //  通道对应的前端相机信息
    NET_DVR_VIDEO_AUDIOIN_CFG  StructVideo_v;         //  视频的音频输入参数
    NET_DVR_AUDIO_INPUT_PARAM  StructAudio_v;         //  音频输入参数
    NET_DVR_AUDIOOUT_VOLUME    StructAudioOut_v;      //  输出音频大小
    NET_DVR_CAMERA_DEHAZE_CFG  StructCameraDehaze_v;  //  去雾参数
    NET_DVR_LOW_LIGHT_CFG      StructLowLigh_v;       //  快球低照度信息
    NET_DVR_FOCUSMODE_CFG      StructFocusmode_v;     //  快球聚焦模式信息
    NET_DVR_INFRARE_CFG        StructInfrare_v;       //  快球红外信息
    NET_DVR_AEMODECFG          StructAemode_v;        //  快球其他参数信息
    NET_DVR_CORRIDOR_MODE      StructCorridor_v;      //  旋转功能配置
    NET_DVR_SIGNAL_SYNCCFG     StructSignal_v;        //  信号灯同步配置参数
    DHDEV_SYSTEM_ATTR_CFG      StructSystem_k;//DH        设备参数
    DHDEV_NET_CFG              StructNet_k;           //  网络参数
    DHDEV_CHANNEL_CFG          StructChannel_k;       //  图象通道参数
    DHDEV_RECORD_CFG           StructRecodeCfg_k;     //  定时录像参数
    DHDEV_COMM_CFG             StructComm_k;          //  串口参数
    DHDEV_ALARM_SCHEDULE       StructAlarm_k;         //  报警布防参数
    NET_TIME                   StructTime_k;          //  DVR时间
    DHDEV_PREVIEW_CFG          StructPreview_k;       //  预览参数
    DHDEV_AUTOMT_CFG           StructAutomt_k;        //  自动维护配置
    DHDEV_VIDEO_MATRIX_CFG     StructVideoMatrix_k;   //  本机矩阵策略配置
    DHDEV_MULTI_DDNS_CFG       StructMulti_k;         //  多ddns服务器配置
    DHDEV_SNAP_CFG             StructSnap_k;          //  抓图相关配置
    DHDEV_URL_CFG              StructUrl_k;           //  http路径配置
    DHDEV_FTP_PROTO_CFG        StructFtp_k;           //  ftp上传配置
    DHDEV_VIDEOCOVER_CFG       StructVideocover_k;    //  区域遮挡配置
    DHDEV_TRANSFER_STRATEGY_CFG StructTransfer_k;     //  传输策略配置
    DHDEV_DOWNLOAD_STRATEGY_CFG StructDownload_k;     //  录象下载策略配置
    DHDEV_WATERMAKE_CFG        StructWatermake_k;     //  图象水印配置
    DHDEV_WLAN_INFO            StructWlan_k;          //  配置无线网络信息
    DHDEV_WLAN_DEVICE_LIST     StructWlanDevice_k;    //  搜索无线设备
    DHDEV_REGISTER_SERVER      StructRegister_k;      //  主动注册参数配置
    DHDEV_CAMERA_CFG           StructCameracfg_k;     //  摄像头属性配置
    DH_INFRARED_CFG            StructInfrared_k;      //  红外报警参数
    DHDEV_MAIL_CFG             StructMail_k;          //  邮件配置
    DHDEV_DNS_CFG              StructDns_k;           //  DNS服务器配置
    DHDEV_NTP_CFG              StructNtp_k;           //  NTP配置
    DHDEV_SNIFFER_CFG          StructSniffer_k;       //  Sniffer抓包配置
    DH_AUDIO_DETECT_CFG        StructAudioDetect_k;   //  音频检测报警参数
    DH_STORAGE_STATION_CFG     StructStorage_k;       //  存储位置配置
    DHDEV_DST_CFG              StructDst_k;           //  夏令时配置
    DH_DVR_VIDEOOSD_CFG        StructVideoosd_k;   //  视频OSD叠加配置
    ALARMCENTER_UP_CFG         StructAlarmCenter_k;   //  报警中心配置
    DHDEV_CDMAGPRS_CFG         StructCdmagprs_k;      //  CDMA\GPRS网络配置
    DHDEV_IPIFILTER_CFG        StructIpifilter_k;     //  IP过滤配置
    DHDEV_TALK_ENCODE_CFG      StructTalk_k;          //  语音对讲编码配置
    DHDEV_RECORD_PACKET_CFG    StructRecordPacket_k;  //  录像打包长度配置
    DHDEV_MMS_CFG              StructMms_k;           //  短信MMS配置
    DHDEV_SMSACTIVATION_CFG    StructSmsacTivation_k; //  短信激活无线连接配置
    DHDEV_DIALINACTIVATION_CFG StructDialinactivation_k;//拨号激活无线连接配置
    DH_ATM_SNIFFER_CFG_EX      StructSnifferCfg_k;    //  网络抓包配置
    int                        StructInt_k;           //  下载速度限制
    ALARM_PANORAMA_SWITCH_CFG  StructPanotama_k;      //  全景切换报警配置
    ALARM_LOST_FOCUS_CFG       StructLost_k;          //  失去焦点报警配置
    DH_ALARMDEC_CFG            StructAlarmdec_k;      //  报警解码器配置
    DEV_VIDEOOUT_INFO          StructVideoOut_k;      //  视频输出参数配置
    DHDEV_POINT_CFG            StructPoint_k;         //  预制点使能配置
    ALARM_IP_COLLISION_CFG     StructIp_k;            //  Ip冲突检测报警配置
    DHDEV_OSD_ENABLE_CFG       StructOsd_k;           //  OSD叠加使能配置
    DH_ALARMIN_CFG_EX          StructAlarmCfg_k;      //  本地报警配置
    DH_MOTION_DETECT_CFG_EX    StructMotion_k;        //  动检报警配置
    DH_VIDEO_LOST_CFG_EX       StructVideLost_k;      //  视频丢失报警配置
    DH_BLIND_CFG_EX            StructBlind_k;         //  视频遮挡报警配置
    DH_DISK_ALARM_CFG_EX       StructDiskAlarm_k;     //  硬盘报警配置
    DH_NETBROKEN_ALARM_CFG_EX  StructNetBroken_k;     //  网络中断报警配置
    DEV_ENCODER_CFG            StructEncoder_k;       //  数字通道的前端编码器信息
    DHDEV_TVADJUST_CFG         StructTvadjust_k;      //  TV调节配置
    DHDEV_ABOUT_VEHICLE_CFG    StructAbout_k;         //  车载相关配置，北京公交使用
    DHDEV_ATM_OVERLAY_GENERAL  StructAtmover_k;       //  获取atm叠加支持能力信息
    DHDEV_ATM_OVERLAY_CONFIG   StructATMoverlay_k;    //  atm叠加配置，新atm特有
    DHDEV_DECODER_TOUR_CFG     StructTour_k;          //  解码器解码轮巡配置
    DHDEV_SIP_CFG              StructSip_k;           //  SIP配置
    DHDEV_VEHICLE_SINGLE_WIFI_AP_CFG StructVehicle_k; //  WIFI AP配置
    DH_STATIC_DETECT_CFG_EX    StructStatic_k;        //  静态报警配置
    DHDEV_DECODEPOLICY_CFG     StructDecodePolicy_k;  //  设备的解码策略配置
    DHDEV_MACHINE_CFG          StructMachine_k;       //  机器相关的配置
    ALARM_MAC_COLLISION_CFG    StructMacColision_k;   //  MAC冲突检测配置
    DHDEV_RTSP_CFG             StructRtsp_k;          //  RTSP配置
    COM_CARD_SIGNAL_LINK_CFG   StructLinkCfg_k;       //  232/485串口卡号信号事件配置
    DHDEV_FTP_PROTO_CFG_EX     StructFtpProto_k;      //  FTP上传扩展配置
    DHDEV_SYSLOG_REMOTE_SERVER StructSysLog_k;        //  SYSLOG 远程服务器配置
    DHDEV_COMM_CFG_EX          StructCommCfg_k;       //  扩展串口属性配置
    DHDEV_NETCARD_CFG          StructNetcard_k;       //  卡口信息配置
    DHDEV_BACKUP_VIDEO_FORMAT  StructBackUp_k;        //  视频备份格式配置
    DHEDV_STREAM_ENCRYPT       StructStreamEncrypt_k; //  码流加密配置
    DHDEV_IPIFILTER_CFG_EX     StructIpifilterCfg_k;  //  IP过滤配置扩展
    DHDEV_CUSTOM_CFG           StructCustom_k;        //  用户自定义配置
    DHDEV_WLAN_DEVICE_LIST_EX  StructWlanDeviceList_k;//  搜索无线设备扩展配置
    DHDEV_ACC_POWEROFF_CFG     StructAcc_k;           //  ACC断线事件配置
    DHDEV_EXPLOSION_PROOF_CFG  StructExpLosion_k;     //  防爆盒报警事件配置
    DHDEV_NET_CFG_EX           StructNetCfg_k;        //  网络扩展配置
    DHDEV_LIGHTCONTROL_CFG     StructLight_k;         //  灯光控制配置
    DHDEV_3GFLOW_INFO_CFG      Struct3gflow_k;        //  3G流量信息配置
    DHDEV_IPV6_CFG             StructIpv6_k;          //  IPv6配置
    DHDEV_NET_SNMP_CFG         StructNetSnmp_k;       //  Snmp配置
    DHDEV_SNAP_CONTROL_CFG     StructSnapControl_k;   //  抓图开关配置
    DHDEV_GPS_MODE_CFG         StructGps_k;           //  GPS定位模式配置
    DHDEV_SNAP_UPLOAD_CFG      StructSnapUpload_k;    //  图片上传配置信息
    DHDEV_SPEED_LIMIT_CFG      StructSpeed_k;         //  限速配置信息
    DHDEV_ISCSI_CFG            StructIscsi_k;         //  iSCSI配置
    DHDEV_WIRELESS_ROUTING_CFG StructWireless_k;      //  无线路由配置
    DHDEV_ENCLOSURE_CFG        StructEnclosure_k;     //  电子围栏配置
    DHDEV_ENCLOSURE_VERSION_CFG StructVersion_k;      //  电子围栏版本号配置
    DHDEV_RAID_EVENT_CFG       StructRaid_k;          //  Raid事件配置
    DHDEV_FIRE_ALARM_CFG       StructFire_k;          //  火警报警配置
    char*                      StructChar_k;          //  本地名称报警配置
    DHDEV_URGENCY_RECORD_CFG   StructUrgency_k;       //  紧急存储配置
    DHDEV_ELEVATOR_ATTRI_CFG   StructElevator_k;      //  电梯运行参数配置
    DHDEV_ATM_OVERLAY_CONFIG_EX StructAtmConfig_k;    //  atm叠加配置
    DHDEV_MACFILTER_CFG        StructMacFilter_k;     //  MAC过滤配置
    DHDEV_MACIPFILTER_CFG      StructMacipfilter_k;   //  MAC,IP过滤
    DHEDV_STREAM_ENCRYPT       StructStrea_k;         //  码流加密(加密计划)配置
    DHDEV_LIMIT_BIT_RATE       StructLimit_k;         //  限码流配置
    DHDEV_SNAP_CFG_EX          StructSnapCfg_k;       //  抓图相关配置扩展
    DHDEV_DECODER_URL_CFG      StructDecoder_k;       //  解码器url配置
    DHDEV_TOUR_ENABLE_CFG      StructTourEnable_k;    //  轮巡使能配置
    DHDEV_VEHICLE_WIFI_AP_CFG_EX StructVehicleWifi_k; //  wifi ap配置扩展
    DEV_ENCODER_CFG_EX         StructEncoderCfg_k;    //  数字通道的前端编码器信息扩展
}JF_DevConfig;
#endif // !JF_DEFINE
