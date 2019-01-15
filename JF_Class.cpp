extern "C"{
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
}
#include <iostream>
#include <jf_inc/JF_Define.h>
#include <v_inc/HCNetSDK.h>
//#include <v_inc/HCNetSDK.h>
#include "../include/jf_inc/JF_Class.h"
#include "../include/jf_inc/JF_Define.h"
//#include "../include/v_inc/HCNetSDK.h"
#include <time.h>
using namespace std;
#define errmsg(msg)    do\
{\
    std::cerr << msg << std::endl;\
}while(0)
HWND ghWnd=NULL;
HWND pbhWnd=NULL;
bool JF_SDK::flag = true;
pthread_t loginThread;
pthread_mutex_t mutOne;
pthread_mutex_t mutTwo;
pthread_mutex_t mutThree;
pthread_t p1,p2;
std::vector<JF_UserIDInfo*> JF_SDK::m_vcUserInfo;
//JF_SDK_K* JF_SDK::m_pk = NULL;
//JF_SDK_V* JF_SDK::m_pv = NULL;
//DVRLoginCallBack JF_SDK::DLoginCB=NULL;
fRealDataCallBackEx JF_RealPlay::InClassBcak=NULL;
JF_ListenCallbackArgs *pListenArgs;
CLog* sdkLog=getLogInstance();
void cbReconnect_K(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    sdkLog->WriteLog("one user has disconnected :");
    sdkLog->WriteLog("on device info :");
	sdkLog->WriteLog("ip: ");
    sdkLog->WriteLog(pchDVRIP);
}
unsigned int sip=0;
unsigned int eip=0;
fSadpDevice devicea;
unsigned int CtoINT(char* ip)
{
    int IPV1 = 0;
    int IPV2 = 0;
    int IPV3 = 0;
    int IPV4 = 0;
    sscanf(ip, "%3d.%3d.%3d.%3d", &IPV1, &IPV2, &IPV3, &IPV4);
    unsigned int ipLong = 0;
    ipLong = IPV1 << 24;
    ipLong += IPV2 << 16;
    ipLong += IPV3 << 8;
    ipLong += IPV4 ;
    return ipLong;
}
void INTtoC(char*ip,unsigned int int_ip)
{
    int IPV1 = 0;
    int IPV2 = 0;
    int IPV3 = 0;
    int IPV4 = 0;
    IPV1 = int_ip >> 24;
    IPV2 = (int_ip >> 16) & 255;
    IPV3 = (int_ip >> 8) & 255;
    IPV4 = int_ip & 255;
    sprintf(ip, "%d.%d.%d.%d", IPV1, IPV2, IPV3, IPV4);
}
 void dhSearchDevicesCB (DEVICE_NET_INFO_EX *pDevNetInfo, void* pUserData)
 {
     JF_SadpDevice dh_sadp={0};
     if(pDevNetInfo->iIPVersion==6 )
     {
         return ;
     }
     pthread_mutex_lock(&mutThree);
     memcpy(dh_sadp.szDeviceType,pDevNetInfo->szDetailType,32);
     memcpy(dh_sadp.szDevSoftVersion,pDevNetInfo->szDevSoftVersion,128);
     memcpy(dh_sadp.szDSPVersion,pDevNetInfo->szDeviceType,32);
     memcpy(dh_sadp.szGateway,pDevNetInfo->szGateway,64);
     memcpy(dh_sadp.szIP,pDevNetInfo->szIP,64);
     memcpy(dh_sadp.szMac,pDevNetInfo->szMac,64);
     memcpy(dh_sadp.szSerialNo,pDevNetInfo->szSerialNo,48);
     dh_sadp.DeviceStyle='K';
     dh_sadp.nPort=pDevNetInfo->nPort;
     dh_sadp.dwuser=(long)pUserData;
     //devicea(&dh_sadp);
     pthread_create(&p2,NULL,devicea,(void*)&dh_sadp);
     pthread_join(p2,NULL);
     pthread_mutex_unlock(&mutThree);
 }
 void HKPDEVICE_FIND_CALLBACK (const SADP_DEVICE_INFO *lpDeviceInfo, void *pUserData)
 {
     JF_SadpDevice dh_sadp={0};
     if(CtoINT((char*)lpDeviceInfo->szIPv4Address)>eip||CtoINT((char*)lpDeviceInfo->szIPv4Address)<sip)
     {
         return;
     }
     pthread_mutex_lock(&mutThree);
     dh_sadp.nPort=lpDeviceInfo->dwPort;
     memcpy(dh_sadp.szDeviceType,lpDeviceInfo->szDevDesc,24);
     memcpy(dh_sadp.szDevSoftVersion,lpDeviceInfo->szDeviceSoftwareVersion,48);
     memcpy(dh_sadp.szDSPVersion,lpDeviceInfo->szDSPVersion,48);
     memcpy(dh_sadp.szGateway,lpDeviceInfo->szIPv4Gateway,16);
     memcpy(dh_sadp.szIP,lpDeviceInfo->szIPv4Address,16);
     memcpy(dh_sadp.szMac,lpDeviceInfo->szMAC,20);
     memcpy(dh_sadp.szSerialNo,lpDeviceInfo->szSerialNO,48);
     dh_sadp.DeviceStyle='V';
     dh_sadp.dwuser=(long)pUserData;
     pthread_create(&p1,NULL,devicea,(void*)&dh_sadp);
     pthread_join(p1,NULL);
     pthread_mutex_unlock(&mutThree);
 }
extern "C"{
int SADPdevice(char* deviceType,char* stip,char* edip,void* sadpcallback,LDWORD dwuser,DWORD dWaitTime)
{
    DEVICE_IP_SEARCH_INFO DHSearch={0};
    devicea=(fSadpDevice)sadpcallback;
    DHSearch.dwSize=sizeof(DEVICE_IP_SEARCH_INFO);
    string ipDump=stip;
    in_addr add;
    unsigned int int_ip;
    DHSearch.nIpNum=0;
    int i=0;
    sip=CtoINT((char*)ipDump.c_str());
    while(1)
    {
        DHSearch.nIpNum++;
        strcpy(DHSearch.szIP[i],ipDump.c_str());
        int_ip=CtoINT((char*)ipDump.c_str());
        int_ip++;
        ipDump.empty();
        memset(&add,0,sizeof(in_addr));
        char ipadd[64];
        INTtoC((char*)ipadd,int_ip);
        ipDump=ipadd;
        if(strcmp(ipDump.c_str(),edip)==0)
        {
            eip=int_ip;
            DHSearch.nIpNum++;
            strcpy(DHSearch.szIP[i],ipDump.c_str());
            break;
        }
        i++;
    }
    //long puser=0;
    if(strncmp("V",deviceType,1)==0)
    {
        if(SADPdevice_v(HKPDEVICE_FIND_CALLBACK,0,(void*)dwuser))
        {
            return 1;
        }
    }else if(strncmp("K",deviceType,1)==0)
    {
        if(SADPdevice_k(&DHSearch,(fSearchDevicesCB)dhSearchDevicesCB,dwuser,NULL,dWaitTime))
        {
            return 1;
        }
    }
    return 0;
}
int StopSADPdevice(char* device)
{
    if(strncmp("V",device,1)==0)
    {
        if(StopSADPdevice_v())
        {
            return 1;
        }
    }else if(strncmp("K",device,1)==0)
    {
        if(StopSADPdevice_k())
        {
            return 1;
        }
    }
    return 0;
}
}
//-------------------------------------------------------
//|------------>      JF_SDK 类      <-------------------|
//-------------------------------------------------------
//Description:获取加载类型
int Get_Config()    //获取加载类型
{
    /*char path[64] = {0};
    if(NULL == realpath("./", path))//将目标路径转换为绝对路径
    {
        return -1;
    }

    strcat(path, "/data_base/DataBase.xml");//将目标路径添加到path的结尾处

    FILE *pf = fopen(path, "r");//以只读的方式打开文件
    if(NULL == pf)
    {
        return -1;
    }

    int initType = -1;//加载类型：大华 1 or 海康 2
    char buf[256] = {0};
    do {
        memset(buf, 0, sizeof(buf));//将buf中当前位置的前sizeof（buf）的部分用0替换，并返回buf
        char *ret = fgets(buf,sizeof(buf),pf);//将buf的内容用pf替换，长度sizeof（buf）
        if(NULL == ret)
        {
            break;
        }
        buf[strlen(buf)-1] = '\0';
        if(NULL != strstr(buf,"LoadDllType"))//判断“LoadDllType”是否是buf的子串，是返回地址，否返回NULL
        {
            char *pstart = strchr(buf,'>');//在字符串中查找字符'>'
            char *pstop  = strchr(pstart,'<');//在字符串中查找字符'<'
            
            while(('0'>*pstart)||('9'<*pstart))//pstart指向的值大于0，或者小于9
            {
                if(++pstart > pstop)
                {
                    break;
                }
            }
            while(('0'>*pstop)||('9'<*pstop))
            {
                if(pstart > --pstop)
                {
                    break;
                }
            }
            if(pstart > pstop)
            {
                break;
            }
            *(pstop+1) = '\0';
            initType = atoi(pstart);//字符串转整形数
            break;
        }
    }while (EOF != feof(pf));//检测流上的文件结束符，文件结束返回非0，否则返回0  EOF 文件结束符的标志

    return initType;*/
}
//Description：JF_SDK初始化函数
//Input：initType：厂家类型  cbDisConnect_k:
JF_SDK::JF_SDK(int initType, fDisConnect_k cbDisConnect_k, LDWORD dwDiscUser_k)
{
    m_pk = NULL;
    m_pv = NULL;
    pthread_mutex_init(&mutOne,NULL);
    pthread_mutex_init(&mutTwo,NULL);
    pthread_mutex_init(&mutThree,NULL);
    sdkLog->InitModule("DVR_SDK_A");
    if(0 == initType)   //全部加载
    {
        m_pk = new JF_SDK_K(cbDisConnect_k, dwDiscUser_k);
        m_pv = new JF_SDK_V;
        if((NULL==m_pk)||(NULL==m_pv))//初始化失败
        {
            flag = false;
        }
        if((!m_pk->getFlag())||(!m_pv->getFlag()))
        {
            flag = false;
        }
    }
    else if(1 == initType)//加载大华
    {
        m_pk = new JF_SDK_K(cbDisConnect_k, dwDiscUser_k);
        if(NULL==m_pk)
        {
            flag = false;
        }
        if(!m_pk->getFlag())
        {
            flag = false;
        }
    }
    else if(2 == initType)//加载海康
    {
        m_pv = new JF_SDK_V;
        if(NULL==m_pv)
        {
            flag = false;
        }
        if(!m_pv->getFlag())
        {
            flag = false;
        }
    }
    else if(12 == initType)//加载大华和海康
    {
        m_pk = new JF_SDK_K(cbDisConnect_k, dwDiscUser_k);
        m_pv = new JF_SDK_V;
        if((NULL==m_pk)||(NULL==m_pv))
        {
            flag = false;
        }
        if((!m_pk->getFlag())||(!m_pv->getFlag()))
        {
            flag = false;
        }
    }
}

JF_SDK::~JF_SDK()
{
    while(!m_vcUserInfo.empty())    //判断vector是否为空 m_vcUserInfo用户管理链表
    {
        JF_UserIDInfo* &tmp = m_vcUserInfo.back();//得到数组的最后一个单元的引用
        delete tmp->pUsers;//删除用户类操作对象
        tmp->pUsers = NULL;
        delete tmp;//删除用户信息
        tmp = NULL;
        m_vcUserInfo.pop_back();//去掉数组的最后一个数据
    }

    if(NULL != m_pk)//m_pk 类JF_K_Class的指针
    {
        delete m_pk;
        m_pk = NULL;
    }

    if(NULL != m_pv)//m_pv JF_K_Class的类指针
    {
        delete m_pv;
        m_pv = NULL;
    }
}
////初始化函数                 /*断线回调函数*/               /*断线回调函数 用户自定义数据*/
JF_SDK* JF_SDK::JF_SDK_Init(int DvrType,fDisConnect_k cbDisConnect_k, LDWORD dwDiscUser_k)
{
    static bool initted = false;
    /*if(initted) //SDK只能初始化一次
    {
        errmsg("SDK had initted");
        return NULL;
    }
    else
    {
        initted = true;
    }*/

    //int initType =Get_Config();//获取加载类型
    if(-1 == DvrType)
    {
        sdkLog->WriteLog("unknow init type");
        //errmsg("unknow init type");
        return NULL;
    }                    

    JF_SDK *sdk = new JF_SDK(DvrType, cbDisConnect_k, dwDiscUser_k);//开辟堆内存，调用构造函数
    if(!flag)//falg是什么？定义在JF_Class的头文件中，私有变量，静态布尔值
    {
        sdkLog->WriteLog("sdk init error");
        //errmsg("sdk init error");
        delete sdk;
        sdk = NULL;
        flag = true;
    }

    return sdk;
}

/*int Get_CompanyType(int dvrType)
{
    switch(dvrType)
	{
        case 226://dh
		case 227:
		case 228:
		case 201:
		case 251:
		case 501:
		case 502:
		case 503:
		case 504:
            return DH;
        break;
		case 220://hk
		case 221:
		case 224:
		case 252:
		case 521:
		case 522:
		case 523:
		case 524:
            return HK;
		break;
        default:
            return -1;
    }
}*/
//    设备IP或域名,设备端口号,用户名,密码,断线重连的回调函数,登陆方式,emLoginType_k补充函数,用户数据,设备信息,登陆成功后对应的用户类对象
LONG JF_SDK::JF_Login(char *pchDVRIP, WORD wDVRPort, char *pchUserName, char *pchPasswd, LDWORD *dwUser,char*deviceType, JF_DVRInfo *pDVRInfo, DVRLoginCallBack pUsersLoginCB)
{
    LONG lhLogin = -1;
    //DLoginCB=pUsersLoginCB;
    if((NULL==m_pk)&&(NULL==m_pv))  //判断SDK是否初始化成功
    {
        //pUsers = NULL;//用户操作类对象
        return lhLogin;
    }
    JF_Login_Struct* LoginStruct=(JF_Login_Struct*)new JF_Login_Struct;
    strncpy(LoginStruct->pchDVRIP,pchDVRIP,32);
    strncpy(LoginStruct->Dtype,deviceType,3);
    LoginStruct->wDVRPort = wDVRPort;
    strncpy(LoginStruct->pchUserName,pchUserName,64);
    strncpy(LoginStruct->pchPasswd,pchPasswd,64);
    LoginStruct->dwUser=dwUser;
    LoginStruct->pDVRInfo =pDVRInfo;
    LoginStruct->pUsers = this;
    LoginStruct->callBack=pUsersLoginCB;
    int loginret;
    loginret=pthread_create(&loginThread,NULL,static_LoginThread,(void*)LoginStruct);
    if(-1==loginret)
    {
        return -1;
    }else
    {
        return 0;
    }
}
//转发登录
LONG JF_SDK::JF_Login_SM(char *pchDVRIP, WORD wDVRPort, char *pchUserName, char *pchPasswd,char*deviceType, JF_DVRInfo *pDVRInfo,JF_Users* &pUsers)
{
    LLONG lhLogin = -1;
    int temp;
    int err;
    NET_DEVICEINFO_Ex *struDevInfo=NULL;
    JF_UserIDInfo *userInfo=NULL;
    if (NULL!=m_pk)    //尝试登录大华设备
    {
        err = 0;
        m_pk->SetReconnect(cbReconnect_K, 0);//设置断线重连的回调函数
        struDevInfo = new NET_DEVICEINFO_Ex;//设备信息结构体
        if (strcmp((const char*)deviceType,"PA")==0||strcmp((const char*)deviceType,"XC")==0)   //登录根据设备类型
        {
            temp=90106;
            lhLogin=m_pk->JF_Login_k(pchDVRIP,wDVRPort,pchUserName,pchPasswd,(EM_LOGIN_SPAC_CAP_TYPE)28,&temp,struDevInfo,&err);
            if(-1==lhLogin)
            {
                if ((3 != err) && (2 != err))  //粗略判断欲登录的设备是大华的设备但未成功
                {
                    delete struDevInfo;
                    struDevInfo = NULL;
                }
            }
        }else if(strcmp((const char*)deviceType,"OTH")==0)
        {
            lhLogin=m_pk->JF_Login_k(pchDVRIP,wDVRPort,pchUserName,pchPasswd,(EM_LOGIN_SPAC_CAP_TYPE)0,NULL,struDevInfo,&err);
            if(-1==lhLogin)
            {
                if ((3 != err) && (2 != err))  //粗略判断欲登录的设备是大华的设备但未成功
                {
                    delete struDevInfo;
                    struDevInfo = NULL;
                }
            }
        }
        if(-1!=lhLogin)    //登录成功
        {
            pUsers = new JF_Users(lhLogin, DH);//用户操作类对象
            strncpy(pUsers->IP,pchDVRIP,32);
            if (NULL !=struDevInfo->sSerialNumber[0])//DVR设备信息
            {
                //memset(pDVRInfo, 0, sizeof(pDVRInfo));
                if(pDVRInfo!=NULL)
                {
                    memcpy(pDVRInfo->sSerialNumber, struDevInfo->sSerialNumber, 48);//序列号
                    pDVRInfo->nAlarmInPortNum = struDevInfo->nAlarmInPortNum;//报警输入个数
                    pDVRInfo->nAlarmOutPortNum = struDevInfo->nAlarmOutPortNum;//报警输出个数
                    pDVRInfo->nDiskNum = struDevInfo->nDiskNum;//硬盘个数
                    pDVRInfo->nDVRType = struDevInfo->nDVRType;//DVR类型
                    pDVRInfo->nChanNum = struDevInfo->nChanNum;//通道个数
                }
            }
            userInfo = new JF_UserIDInfo;    //用户信息
            memset(userInfo, 0, sizeof(JF_UserIDInfo));
            userInfo->lLoginID = lhLogin;//登陆ID
            strncpy(userInfo->DVRIP, pchDVRIP,32);//设备IP地址
            userInfo->DVRPort = wDVRPort;//设备端口
            userInfo->DVRType = struDevInfo->nDVRType;//设备类型
            strncpy(userInfo->UserName, pchUserName,64);//用户名
            strncpy(userInfo->Passwd, pchPasswd,64);//密码
            userInfo->companyType = DH;//厂商类型
            userInfo->pUsers = pUsers;//用户操作类对象
            DHDEV_VERSION_INFO stDeviceInfo={0};
            int rLong;
            strncpy((char*)userInfo->sSerialNumber,(const char*)struDevInfo->sSerialNumber,48);
            if(pUsers->m_pk->JF_QueryDVRState_k(lhLogin,DH_DEVSTATE_SOFTWARE,(char*)&stDeviceInfo,sizeof(DHDEV_VERSION_INFO),&rLong)==0)
            {
                strncpy((char*)userInfo->szDevType,(const char*)stDeviceInfo.szDevType,32);
                strncpy((char*)userInfo->szSoftWareVersion,(const char*)stDeviceInfo.szSoftWareVersion,128);
            }
            CFG_NETWORK_INFO network={0};
            if(pUsers->m_pk->JF_GetNewDevConfig_k(lhLogin,CFG_CMD_NETWORK,0xFFFFFFFF,(void*)&network,sizeof(CFG_NETWORK_INFO),&rLong,2000))
            {
                for(int i=0;i<network.nInterfaceNum;i++)
                {
                    if(strcmp(userInfo->DVRIP,network.stuInterfaces[i].szIP)==0)
                    {
                        strncpy((char*)userInfo->szDefGateway,(const char*)network.stuInterfaces[i].szDefGateway,256);
                        strncpy((char*)userInfo->szMacAddress,(const char*)network.stuInterfaces[i].szMacAddress,256);
                    }
                }
            }
            JF_SDK::m_vcUserInfo.push_back(userInfo);//在数组的最后添加一个数据
            //return NULL;//设备登陆返回值
        }
        return lhLogin;
    }
    NET_DVR_DEVICEINFO_V40 *DstruDevInfo=NULL;
    if (NULL != m_pv)    //尝试登录海康设备
    {
        DstruDevInfo = new NET_DVR_DEVICEINFO_V40;//设备参数结构体
        lhLogin = m_pv->JF_Login_v(pchDVRIP, wDVRPort, pchUserName, pchPasswd, (void*)(0),deviceType, DstruDevInfo);//设备登陆
        if (-1 == lhLogin)   //登录失败
        {
            //Error_v();
            delete DstruDevInfo;
            DstruDevInfo = NULL;
            //Login.pUsers = NULL;
            //return FALSE;
        }
        else    //登录成功
        {
            pUsers = new JF_Users(lhLogin, HK);//用户操作类对象
            strncpy(pUsers->IP,pchDVRIP,32);
            //设备信息
            if (NULL != DstruDevInfo->struDeviceV30.sSerialNumber[0])//设备DVR信息
            {
                //memset(Login.pDVRInfo, 0, sizeof(Login.pDVRInfo));
                if(pDVRInfo!=NULL)
                {
                    memcpy(pDVRInfo->sSerialNumber, DstruDevInfo->struDeviceV30.sSerialNumber, 48);
                    pDVRInfo->nAlarmInPortNum = DstruDevInfo->struDeviceV30.byAlarmInPortNum;
                    pDVRInfo->nAlarmOutPortNum = DstruDevInfo->struDeviceV30.byAlarmOutPortNum;
                    pDVRInfo->nDiskNum = DstruDevInfo->struDeviceV30.byDiskNum;
                    pDVRInfo->nDVRType = DstruDevInfo->struDeviceV30.wDevType;
                    pDVRInfo->nChanNum = DstruDevInfo->struDeviceV30.byChanNum;
                }
            }
            userInfo = new JF_UserIDInfo;    //用户登录信息
            memset(userInfo, 0, sizeof(JF_UserIDInfo));
            userInfo->lLoginID = lhLogin;//ID
            strncpy(userInfo->DVRIP, pchDVRIP,32);//IP
            userInfo->DVRPort = wDVRPort;//;//设备类型
            strncpy(userInfo->UserName,pchUserName,64);//用户名
            strncpy(userInfo->Passwd, pchPasswd,64);//密码
            userInfo->companyType = HK;//厂商类型
            userInfo->pUsers = pUsers;//用户操作类对象端口
            userInfo->DVRType = DstruDevInfo->struDeviceV30.wDevType;
            NET_DVR_DEVICECFG_V40 szDevice={0};
            DWORD rTime;
            if(pUsers->m_pv->JF_GetDVRConfig_v(lhLogin,NET_DVR_GET_DEVICECFG_V40,0,&szDevice,sizeof(NET_DVR_DEVICECFG_V40),&rTime)==0)
            {
                char* schar=strchr((char*)szDevice.sSerialNumber,'-');
                char* echar=strchr((char*)schar+1,'-');
                if(schar!=NULL&&echar!=NULL)
                {
                    strncpy((char*)userInfo->szDevType,(const char*)schar-2,echar-schar+7);
                }
                if((szDevice.dwSoftwareVersion>>24)&0xFF>0)
                {
                    sprintf(userInfo->szSoftWareVersion,"V%d.%d.%d build%02d%02d%02d",(szDevice.dwSoftwareVersion>>24)&0xFF,(szDevice.dwSoftwareVersion>>16)&0xFF,\
                            (szDevice.dwSoftwareVersion)&0xFFFF,(szDevice.dwSoftwareBuildDate>>16)&0xFFFF,(szDevice.dwSoftwareBuildDate>>8)&0xFF,(szDevice.dwSoftwareBuildDate)&0xFF);
                }
                strncpy((char*)userInfo->sSerialNumber,(const char*)szDevice.sSerialNumber,48);
            }
            NET_DVR_NETCFG_V50 netcfg={0};
            if(pUsers->m_pv->JF_GetRemoteDecice_v(lhLogin,NET_DVR_GET_NETCFG_V50,0xFFFFFFFF,(void*)&netcfg,sizeof(NET_DVR_NETCFG_V50),&rTime)==1)
            {
                if(strcmp(pchDVRIP,netcfg.struEtherNet[1].struDVRIP.sIpV4)==0)
                {
                    strcpy((char*)userInfo->szMacAddress,(const char*)netcfg.struEtherNet[1].byMACAddr);
                }
                if(strcmp(pchDVRIP,netcfg.struEtherNet[2].struDVRIP.sIpV4)==0)
                {
                    strcpy((char*)userInfo->szMacAddress,(const char*)netcfg.struEtherNet[2].byMACAddr);
                }
                strncpy((char*)userInfo->szDefGateway,(const char*)netcfg.struGatewayIpAddr.sIpV4,16);
            }
            JF_SDK::m_vcUserInfo.push_back(userInfo);//在数组最后添加一个数据
        }
        return lhLogin;
    }
}
void* static_LoginThread(void* LoginStruct)
{
    JF_Login_Struct Login = *(JF_Login_Struct*)LoginStruct;
    Login.pUsers->JF_LoginThread(LoginStruct);
}
bool JF_SDK::JF_Logout(JF_Users* &pUsers)//用户类的引用
{
    if(m_vcUserInfo.empty())//用户管理链表是否为空
    {
        sdkLog->WriteLog("vector is empty");
        //errmsg("vector is empty");
        return false;
    }

    int i;
    std::vector<JF_UserIDInfo*>::iterator it=m_vcUserInfo.begin();//用户信息  定义了it变量，数据类型是vector<JF_UserIDInfo>定义的iterator类型
    for(i=0; i<m_vcUserInfo.size(); ++i,++it)    //找出指定的用户信息
    {
        if (pUsers == m_vcUserInfo[i]->pUsers) {
            break;
        }
    }
    if (i == m_vcUserInfo.size()) {
        sdkLog->WriteLog("no this user");
        //errmsg("no this user");
        return false;
    } else {
        if (HK == (*it)->companyType)      //登出设备
        {
            if (-1 == m_pv->JF_Logout_v(pUsers->m_lLoginID)) {
                sdkLog->WriteLog("logout fault");
                //errmsg("logout fault");
                return false;
            }
        } else if (DH == (*it)->companyType) {
            if (-1 == m_pk->JF_Logout_k(pUsers->m_lLoginID)) {
                sdkLog->WriteLog("logout fault");
                //errmsg("logout fault");
                return false;
            }
        }
        delete pUsers;
        pUsers = NULL;
        //(*it)->pUsers = NULL;
        delete *it;
        *it = NULL;
        m_vcUserInfo.erase(it);//删除it位置的数据
    }
    return true;
}

bool JF_SDK::JF_Logout(LLONG loginID)
{
    if(m_vcUserInfo.empty())//判断用户管理链表是否为空
    {
        sdkLog->WriteLog("vector is empty");
        //errmsg("vector is empty");
        return false;
    }

    int i;
    std::vector<JF_UserIDInfo*>::iterator it = m_vcUserInfo.begin();//用户信息  定义了it变量，数据类型是vector<JF_UserIDInfo>定义的iterator类型
    for(i=0; i<m_vcUserInfo.size(); ++i,++it)    //找出指定的用户信息
    {
        if(loginID == m_vcUserInfo[i]->lLoginID)
        {
            break;
        }
    }

    if(i == m_vcUserInfo.size())
    {
        sdkLog->WriteLog("no this ID");
        //errmsg("no this ID");
        return false;
    }
    else
    {
        if(HK == (*it)->companyType)      //登出设备
        {
            if(-1 == m_pv->JF_Logout_v(loginID))
            {
                sdkLog->WriteLog("logout fault");
                //errmsg("logout fault");
                return false;
            }
        }
        else if (DH == (*it)->companyType)
        {
            if(-1 == m_pk->JF_Logout_k(loginID))
            {
                sdkLog->WriteLog("logout fault");
                //errmsg("logout fault");
                return false;
            }
        }
        delete (*it)->pUsers;
        (*it)->pUsers = NULL;
        delete *it;
        *it = NULL;
        m_vcUserInfo.erase(it);//删除it位置的数据
    }
    return true;
}
int JF_SDK::JF_GetLastError() {
    if(m_pk!=NULL)
    {
        return m_pk->JF_GetLastError_k();
    }else if(m_pv!=NULL)
    {
        return m_pv->JF_GetLastError_v();
    }
}
void* JF_SDK::JF_LoginThread(void* LoginStruct)
{
    pthread_detach(pthread_self());
    LLONG lhLogin = -1;
    JF_Login_Struct Login = *(JF_Login_Struct*)LoginStruct;
    int temp;
    JF_Users* pUsers;
    int err;
    NET_DEVICEINFO_Ex *struDevInfo=NULL;
    JF_UserIDInfo *userInfo=NULL;
    if (NULL!=Login.pUsers->m_pk)    //尝试登录大华设备
    {
        //Login.pUsers->m_pk->SetReconnect(cbReconnect_K, *(Login.dwUser));//设置断线重连的回调函数
        err = 0;
        struDevInfo = new NET_DEVICEINFO_Ex;//设备信息结构体
        /*temp = 1;
        lhLogin = Login.pUsers->m_pk->JF_Login_k(Login.pchDVRIP, Login.wDVRPort, Login.pchUserName, Login.pchPasswd, (EM_LOGIN_SPAC_CAP_TYPE)8, &temp, struDevInfo, &err);//设备登陆
        if (-1 == lhLogin)   //登录失败
        {
            temp=90106;
            lhLogin=Login.pUsers->m_pk->JF_Login_k(Login.pchDVRIP,Login.wDVRPort,Login.pchUserName,Login.pchPasswd,(EM_LOGIN_SPAC_CAP_TYPE)28,&temp,struDevInfo,&err);
            if(-1==lhLogin)
            {
                lhLogin=Login.pUsers->m_pk->JF_Login_k(Login.pchDVRIP,Login.wDVRPort,Login.pchUserName,Login.pchPasswd,(EM_LOGIN_SPAC_CAP_TYPE)0,NULL,struDevInfo,&err);
                if(-1==lhLogin)
                {
                    if ((3 != err) && (2 != err))  //粗略判断欲登录的设备是大华的设备但未成功
                    {
                        delete struDevInfo;
                        struDevInfo = NULL;
                        //Login.pUsers = NULL;
                        //return NULL;
                    }
                    //Error_k();
                }
            }
        }*/
        //temp = 1;
        //lhLogin = Login.pUsers->m_pk->JF_Login_k(Login.pchDVRIP, Login.wDVRPort, Login.pchUserName, Login.pchPasswd, (EM_LOGIN_SPAC_CAP_TYPE)8, &temp, struDevInfo, &err);//设备登陆
        if (strcmp((const char*)Login.Dtype,"PA")==0||strcmp((const char*)Login.Dtype,"XC")==0)   //登录根据设备类型
        {
            temp=90106;
            lhLogin=Login.pUsers->m_pk->JF_Login_k(Login.pchDVRIP,Login.wDVRPort,Login.pchUserName,Login.pchPasswd,(EM_LOGIN_SPAC_CAP_TYPE)28,&temp,struDevInfo,&err);
            if(-1==lhLogin)
            {
                if ((3 != err) && (2 != err))  //粗略判断欲登录的设备是大华的设备但未成功
                {
                    delete struDevInfo;
                    struDevInfo = NULL;
                }
            }
        }else if(strcmp((const char*)Login.Dtype,"OTH")==0)
        {
            lhLogin=Login.pUsers->m_pk->JF_Login_k(Login.pchDVRIP,Login.wDVRPort,Login.pchUserName,Login.pchPasswd,(EM_LOGIN_SPAC_CAP_TYPE)0,NULL,struDevInfo,&err);
            if(-1==lhLogin)
            {
                if ((3 != err) && (2 != err))  //粗略判断欲登录的设备是大华的设备但未成功
                {
                    delete struDevInfo;
                    struDevInfo = NULL;
                }
            }
        }
        if(-1!=lhLogin)    //登录成功
        {
            pUsers = new JF_Users(lhLogin, DH);//用户操作类对象
            strncpy(pUsers->IP,Login.pchDVRIP,32);
            if (NULL !=struDevInfo->sSerialNumber[0])//DVR设备信息
            {
                //memset(pDVRInfo, 0, sizeof(pDVRInfo));
                if(Login.pDVRInfo!=NULL) {
                    memcpy(Login.pDVRInfo->sSerialNumber, struDevInfo->sSerialNumber, 48);//序列号
                    Login.pDVRInfo->nAlarmInPortNum = struDevInfo->nAlarmInPortNum;//报警输入个数
                    Login.pDVRInfo->nAlarmOutPortNum = struDevInfo->nAlarmOutPortNum;//报警输出个数
                    Login.pDVRInfo->nDiskNum = struDevInfo->nDiskNum;//硬盘个数
                    Login.pDVRInfo->nDVRType = struDevInfo->nDVRType;//DVR类型
                    Login.pDVRInfo->nChanNum = struDevInfo->nChanNum;//通道个数
                }
            }
            userInfo = new JF_UserIDInfo;    //用户信息
            memset(userInfo, 0, sizeof(JF_UserIDInfo));
            userInfo->lLoginID = lhLogin;//登陆ID
            strncpy(userInfo->DVRIP, Login.pchDVRIP,32);//设备IP地址
            userInfo->DVRPort = Login.wDVRPort;//设备端口
            userInfo->DVRType = struDevInfo->nDVRType;//设备类型
            strncpy(userInfo->UserName, Login.pchUserName,64);//用户名
            strncpy(userInfo->Passwd, Login.pchPasswd,64);//密码
            userInfo->companyType = DH;//厂商类型
            userInfo->pUsers = pUsers;//用户操作类对象
            DHDEV_VERSION_INFO stDeviceInfo={0};
            int rLong;
            strncpy((char*)userInfo->sSerialNumber,(const char*)struDevInfo->sSerialNumber,48);
            if(pUsers->m_pk->JF_QueryDVRState_k(lhLogin,DH_DEVSTATE_SOFTWARE,(char*)&stDeviceInfo,sizeof(DHDEV_VERSION_INFO),&rLong)==0)
            {
                strncpy((char*)userInfo->szDevType,(const char*)stDeviceInfo.szDevType,32);
                //strncpy((char*)userInfo->szSoftWareVersion,(const char*)stDeviceInfo.szSoftWareVersion,128);
            }
            NET_IN_GET_SOFTWAREVERSION_INFO soft_info;
            soft_info.dwSize=sizeof(NET_IN_GET_SOFTWAREVERSION_INFO);
            NET_OUT_GET_SOFTWAREVERSION_INFO soft_out;
            soft_out.dwSize=sizeof(NET_OUT_GET_SOFTWAREVERSION_INFO);
            if(pUsers->m_pk->JF_GetSoftwareVersion_k(lhLogin,&soft_info,&soft_out,5000))
            {
                sprintf(userInfo->szSoftWareVersion,"%s %s%d.%02d.%02d",soft_out.szVersion,"build",soft_out.stuBuildDate.dwYear,soft_out.stuBuildDate.dwMonth,soft_out.stuBuildDate.dwDay);
            }
            CFG_NETWORK_INFO network={0};
            if(pUsers->m_pk->JF_GetNewDevConfig_k(lhLogin,CFG_CMD_NETWORK,0xFFFFFFFF,(void*)&network,sizeof(CFG_NETWORK_INFO),&rLong,2000))
            {
                for(int i=0;i<network.nInterfaceNum;i++)
                {
                    if(strcmp(userInfo->DVRIP,network.stuInterfaces[i].szIP)==0)
                    {
                        strncpy((char*)userInfo->szDefGateway,(const char*)network.stuInterfaces[i].szDefGateway,256);
                        strncpy((char*)userInfo->szMacAddress,(const char*)network.stuInterfaces[i].szMacAddress,256);
                    }
                }
            }
            JF_SDK::m_vcUserInfo.push_back(userInfo);//在数组的最后添加一个数据
            delete(LoginStruct);
            LoginStruct=NULL;
            //return NULL;//设备登陆返回值
        }
        pthread_mutex_lock(&mutOne);
        if(userInfo)
        {
            //sdkLog.WriteLog(Login.pchDVRIP,sdkLog.DEBUG_LEVEL);
            Login.callBack((void*)userInfo,userInfo->pUsers,Login.dwUser,err);
        }else
        {
            //sdkLog.WriteLog(Login.pchDVRIP,sdkLog.DEBUG_LEVEL);
            Login.callBack((void*)NULL,NULL,Login.dwUser,err);
        }
        pthread_mutex_unlock(&mutOne);

    }
    NET_DVR_DEVICEINFO_V40 *DstruDevInfo=NULL;
    if (NULL != Login.pUsers->m_pv)    //尝试登录海康设备
    {
        DstruDevInfo = new NET_DVR_DEVICEINFO_V40;//设备参数结构体
        lhLogin = Login.pUsers->m_pv->JF_Login_v(Login.pchDVRIP, Login.wDVRPort, Login.pchUserName, Login.pchPasswd, (void*)(Login.dwUser),Login.Dtype, DstruDevInfo);//设备登陆
        if (-1 == lhLogin)   //登录失败
        {
            //Error_v();
            delete DstruDevInfo;
            DstruDevInfo = NULL;
            //Login.pUsers = NULL;
            //return FALSE;
        }
        else    //登录成功
        {
            pUsers = new JF_Users(lhLogin, HK);//用户操作类对象
            strncpy(pUsers->IP,Login.pchDVRIP,32);
            //设备信息
            if (NULL != DstruDevInfo->struDeviceV30.sSerialNumber[0])//设备DVR信息
            {
                //memset(Login.pDVRInfo, 0, sizeof(Login.pDVRInfo));
                if(Login.pDVRInfo!=NULL)
                {
                    memcpy(Login.pDVRInfo->sSerialNumber, DstruDevInfo->struDeviceV30.sSerialNumber, 48);
                    Login.pDVRInfo->nAlarmInPortNum = DstruDevInfo->struDeviceV30.byAlarmInPortNum;
                    Login.pDVRInfo->nAlarmOutPortNum = DstruDevInfo->struDeviceV30.byAlarmOutPortNum;
                    Login.pDVRInfo->nDiskNum = DstruDevInfo->struDeviceV30.byDiskNum;
                    Login.pDVRInfo->nDVRType = DstruDevInfo->struDeviceV30.wDevType;
                    Login.pDVRInfo->nChanNum = DstruDevInfo->struDeviceV30.byChanNum;
                }
            }
            userInfo = new JF_UserIDInfo;    //用户登录信息
            memset(userInfo, 0, sizeof(JF_UserIDInfo));
            userInfo->lLoginID = lhLogin;//ID
            strncpy(userInfo->DVRIP, Login.pchDVRIP,32);//IP
            userInfo->DVRPort = Login.wDVRPort;//;//设备类型
            strncpy(userInfo->UserName,Login.pchUserName,64);//用户名
            strncpy(userInfo->Passwd, Login.pchPasswd,64);//密码
            userInfo->companyType = HK;//厂商类型
            userInfo->pUsers = pUsers;//用户操作类对象端口
            userInfo->DVRType = DstruDevInfo->struDeviceV30.wDevType;
            NET_DVR_DEVICECFG_V40 szDevice={0};
            DWORD rTime;
            if(pUsers->m_pv->JF_GetDVRConfig_v(lhLogin,NET_DVR_GET_DEVICECFG_V40,0,&szDevice,sizeof(NET_DVR_DEVICECFG_V40),&rTime)==0)
            {
                char* schar=strchr((char*)szDevice.sSerialNumber,'-');
                char* echar=strchr((char*)schar+1,'-');
                if(schar!=NULL&&echar!=NULL)
                {
                    strncpy((char*)userInfo->szDevType,(const char*)schar-2,echar-schar+7);
                }
                if((szDevice.dwSoftwareVersion>>24)&0xFF>0)
                {
                    sprintf(userInfo->szSoftWareVersion,"V%d.%d.%d build%02d%02d%02d",(szDevice.dwSoftwareVersion>>24)&0xFF,(szDevice.dwSoftwareVersion>>16)&0xFF,\
                            (szDevice.dwSoftwareVersion)&0xFFFF,(szDevice.dwSoftwareBuildDate>>16)&0xFFFF,(szDevice.dwSoftwareBuildDate>>8)&0xFF,(szDevice.dwSoftwareBuildDate)&0xFF);
                }
                strncpy((char*)userInfo->sSerialNumber,(const char*)szDevice.sSerialNumber,48);
            }
            NET_DVR_NETCFG_V50 netcfg={0};
            if(pUsers->m_pv->JF_GetRemoteDecice_v(lhLogin,NET_DVR_GET_NETCFG_V50,0xFFFFFFFF,(void*)&netcfg,sizeof(NET_DVR_NETCFG_V50),&rTime)==1)
            {
                if(strcmp(Login.pchDVRIP,netcfg.struEtherNet[1].struDVRIP.sIpV4)==0)
                {
                    strcpy((char*)userInfo->szMacAddress,(const char*)netcfg.struEtherNet[1].byMACAddr);
                }
                if(strcmp(Login.pchDVRIP,netcfg.struEtherNet[2].struDVRIP.sIpV4)==0)
                {
                    strcpy((char*)userInfo->szMacAddress,(const char*)netcfg.struEtherNet[2].byMACAddr);
                }
                strncpy((char*)userInfo->szDefGateway,(const char*)netcfg.struGatewayIpAddr.sIpV4,16);
            }
            JF_SDK::m_vcUserInfo.push_back(userInfo);//在数组最后添加一个数据
            if(LoginStruct)
            {
                  delete(LoginStruct);
            }
            //return TRUE;
        }
        pthread_mutex_lock(&mutOne);
        if(userInfo)
        {
             //sdkLog.WriteLog(Login.pchDVRIP,sdkLog.DEBUG_LEVEL);
             Login.callBack((void*)userInfo,pUsers,Login.dwUser,err);
        }else
        {
            //sdkLog.WriteLog(Login.pchDVRIP,sdkLog.DEBUG_LEVEL);
            Login.callBack((void*)NULL,NULL,Login.dwUser,err);
        }
        pthread_mutex_unlock(&mutOne);
    }

    return NULL;
}

//-------------------------------------------------------
//|------------>     JF_Users 类     <-------------------|
//-------------------------------------------------------
JF_Users::JF_Users(LLONG loginID, int companyType)
        :m_lLoginID(loginID),m_CompanyType(companyType),
        m_pk(/*JF_SDK::*/m_pk),m_pv(/*JF_SDK::*/m_pv)//ID 厂商类型
{}

JF_Users::~JF_Users()
{}
void JF_Users::Setm_lLoginID(int hLoginID)
{
    //m_lLoginID=hLoginID;
}
bool JF_Users::JF_RemoteControl()
{
    if(HK==m_CompanyType)
    {
        if(TRUE==m_pv->JF_RemoteControl_v(m_lLoginID))
        {
            return TRUE;
        }else
        {
            return FALSE;
        }
    }else if(DH==m_CompanyType)
    {
        if(TRUE==m_pk->JF_RemoteControl_k(m_lLoginID))
        {
            return TRUE;
        }else
        {
            return FALSE;
        }
    }
}
//批量设置NVR上IPC通道
bool JF_Users::JF_SetRemoteDevice(LONG lChannel,LONG elChannel,Login_Format *deviceInfo)
{
    if(DH==m_CompanyType)
    {
        DH_IN_MATRIX_GET_CAMERAS dhInMatrix={0};
        dhInMatrix.dwSize=sizeof(DH_IN_MATRIX_GET_CAMERAS);
        DH_OUT_MATRIX_GET_CAMERAS dhOutMatrix={0};
        dhOutMatrix.dwSize=sizeof(DH_OUT_MATRIX_GET_CAMERAS);
        dhOutMatrix.nMaxCameraCount=128;
        dhOutMatrix.pstuCameras=new DH_MATRIX_CAMERA_INFO[dhOutMatrix.nMaxCameraCount];
        memset(dhOutMatrix.pstuCameras,0,sizeof(DH_MATRIX_CAMERA_INFO)*(dhOutMatrix.nMaxCameraCount));
        for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
        {
            dhOutMatrix.pstuCameras[i].dwSize=sizeof(DH_MATRIX_CAMERA_INFO);
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.dwSize=sizeof(DH_REMOTE_DEVICE);
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount=128;
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs=new DH_VIDEO_INPUTS[dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount];
            memset(dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs,0,sizeof(DH_VIDEO_INPUTS)*(dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount));
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs->dwSize=sizeof(DH_VIDEO_INPUTS);
        }
       if(!m_pk->JF_MatrixGetCameras_k(m_lLoginID,&dhInMatrix,&dhOutMatrix,5000))//获取前端摄像机列表信息
       {
           for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
           {
               delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;

           }
           delete[] dhOutMatrix.pstuCameras;
           return 0;
       }
        /*DH_MATRIX_CAMERA_INFO ipcDevice={0};
        for(int i=0;i<20;i++)
        {
            ipcDevice=dhOutMatrix.pstuCameras[i];
            memset(&ipcDevice,0,sizeof(DH_MATRIX_CAMERA_INFO));
        }*/
        DH_IN_MATRIX_SET_CAMERAS stInSet = {sizeof(stInSet)};
        DH_OUT_MATRIX_SET_CAMERAS stOutSet = {sizeof(stOutSet)};

        stInSet.nCameraCount = dhOutMatrix.nRetCameraCount;
        stInSet.pstuCameras=dhOutMatrix.pstuCameras;
        if(elChannel==0)
        {
            stInSet.pstuCameras[lChannel].nUniqueChannel=lChannel;
            stInSet.pstuCameras[lChannel].stuRemoteDevice.dwSize = sizeof(DH_REMOTE_DEVICE);
            stInSet.pstuCameras[lChannel].stuRemoteDevice.bEnable = TRUE;
            stInSet.pstuCameras[lChannel].stuRemoteDevice.nPort = deviceInfo[0].nPort;
            strncpy(stInSet.pstuCameras[lChannel].stuRemoteDevice.szIp,deviceInfo[0].szIP,16);
            strncpy(stInSet.pstuCameras[lChannel].stuRemoteDevice.szUserEx,deviceInfo[0].UserName,32);
            strncpy(stInSet.pstuCameras[lChannel].stuRemoteDevice.szPwdEx,deviceInfo[0].Passwd,32);
        }else if(elChannel!=0)
        {
            for(int i=lChannel;i<elChannel+1;i++)
            {
                stInSet.pstuCameras[i].nUniqueChannel=i;
                stInSet.pstuCameras[i].stuRemoteDevice.dwSize = sizeof(DH_REMOTE_DEVICE);
                stInSet.pstuCameras[i].stuRemoteDevice.bEnable = TRUE;
                stInSet.pstuCameras[i].stuRemoteDevice.nPort = deviceInfo[i-lChannel].nPort;
                strncpy(stInSet.pstuCameras[i].stuRemoteDevice.szIp,deviceInfo[i-lChannel].szIP,16);
                strncpy(stInSet.pstuCameras[i].stuRemoteDevice.szUserEx,deviceInfo[i-lChannel].UserName,32);
                strncpy(stInSet.pstuCameras[i].stuRemoteDevice.szPwdEx,deviceInfo[i-lChannel].Passwd,32);
            }
        }
        if(m_pk->JF_MatrixSetCameras_k(m_lLoginID,&stInSet,&stOutSet,3000))
        {
            for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
            {
                delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;

            }
            delete[] dhOutMatrix.pstuCameras;
            return 1;
        }
        for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
        {
             delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;

        }
        delete[] dhOutMatrix.pstuCameras;
    }else if(HK==m_CompanyType)
    {
        NET_DVR_IPPARACFG_V40 hkstdDevice={0};
        DWORD hkrsize;
        if(!m_pv->JF_GetRemoteDecice_v(m_lLoginID,NET_DVR_GET_IPPARACFG_V40,0,&hkstdDevice,sizeof(NET_DVR_IPPARACFG_V40),&hkrsize))
        {
            return 0;
        }
        if(elChannel!=0)
        {
            for(int i=lChannel;i<elChannel+1;i++)
            {
                hkstdDevice.struIPDevInfo[i].byEnable=true;
                hkstdDevice.struIPDevInfo[i].byEnableQuickAdd=TRUE;
                hkstdDevice.struIPDevInfo[i].byProType=0;
                strncpy((char*)hkstdDevice.struIPDevInfo[i].sPassword,deviceInfo[i-lChannel].Passwd,16);
                strncpy((char*)hkstdDevice.struIPDevInfo[i].struIP.sIpV4,deviceInfo[i-lChannel].szIP,16);
                //strncpy((char*)hkstdDevice.struIPDevInfo[i].struIP.byIPv6,"::",2);
                strncpy((char*)hkstdDevice.struIPDevInfo[i].sUserName,deviceInfo[i-lChannel].UserName,32);
                hkstdDevice.struIPDevInfo[i].wDVRPort=deviceInfo[i-lChannel].nPort;
                hkstdDevice.struStreamMode[i].byGetStreamType= NET_SDK_IP_DEVICE;
                hkstdDevice.struStreamMode[i].uGetStream.struChanInfo.byEnable=true;
                //hkstdDevice.struStreamMode[i].uGetStream.struChanInfo.byTransProtocol=0;
                hkstdDevice.struStreamMode[i].uGetStream.struChanInfo.byIPID=i+1;
                hkstdDevice.struStreamMode[i].uGetStream.struChanInfo.byChannel=1;
            }
        } else if(elChannel==0)
        {
            hkstdDevice.struIPDevInfo[lChannel].byEnable=true;
            hkstdDevice.struIPDevInfo[lChannel].byEnableQuickAdd=TRUE;
            hkstdDevice.struIPDevInfo[lChannel].byProType=0;
            strncpy((char*)hkstdDevice.struIPDevInfo[lChannel].sPassword,deviceInfo[0].Passwd,16);
            strncpy((char*)hkstdDevice.struIPDevInfo[lChannel].struIP.sIpV4,deviceInfo[0].szIP,16);
            //strncpy((char*)hkstdDevice.struIPDevInfo[lChannel].struIP.byIPv6,"::",2);
            strncpy((char*)hkstdDevice.struIPDevInfo[lChannel].sUserName,deviceInfo[0].UserName,32);
            hkstdDevice.struIPDevInfo[lChannel].wDVRPort=deviceInfo[0].nPort;
            hkstdDevice.struStreamMode[lChannel].byGetStreamType= NET_SDK_IP_DEVICE;
            hkstdDevice.struStreamMode[lChannel].uGetStream.struChanInfo.byEnable=true;
            //hkstdDevice.struStreamMode[lChannel].uGetStream.struChanInfo.byTransProtocol=0;
            hkstdDevice.struStreamMode[lChannel].uGetStream.struChanInfo.byIPID=lChannel+1;
            hkstdDevice.struStreamMode[lChannel].uGetStream.struChanInfo.byChannel=1;
        }
        if(m_pv->JF_SetRemoteDevice_v(m_lLoginID,NET_DVR_SET_IPPARACFG_V40,0,&hkstdDevice,sizeof(NET_DVR_IPPARACFG_V40)))
        {
            return 1;
        }
    }
    return 0;
}
//批量更换修改NVR上IPC通道
bool JF_Users::JF_ChangRemoteDevice(LONG lChannel,LONG elChannel)
{
    AV_CFG_RemoteDevice* stdDevice={0};
    int rSize;
    NET_DVR_IPPARACFG_V40 hkstdDevice={0};
    DWORD hkrsize;
    if(HK==m_CompanyType)
    {
        if(!m_pv->JF_GetRemoteDecice_v(m_lLoginID,NET_DVR_GET_IPPARACFG_V40,0,&hkstdDevice,sizeof(NET_DVR_IPPARACFG_V40),&hkrsize))
        {
            return 0;
        }
        NET_DVR_IPDEVINFO_V31 ipcInfo={0};
        NET_DVR_STREAM_MODE streamInfo={0};
        ipcInfo=hkstdDevice.struIPDevInfo[lChannel];
        hkstdDevice.struIPDevInfo[lChannel]=hkstdDevice.struIPDevInfo[elChannel];
        hkstdDevice.struIPDevInfo[elChannel]=ipcInfo;
        streamInfo=hkstdDevice.struStreamMode[lChannel];
        hkstdDevice.struStreamMode[lChannel]=hkstdDevice.struStreamMode[elChannel];
        hkstdDevice.struStreamMode[elChannel]=streamInfo;
        hkstdDevice.struStreamMode[lChannel].uGetStream.struChanInfo.byIPID=lChannel+1;
        hkstdDevice.struStreamMode[elChannel].uGetStream.struChanInfo.byIPID=elChannel+1;
        if(m_pv->JF_SetRemoteDevice_v(m_lLoginID,NET_DVR_SET_IPPARACFG_V40,0,&hkstdDevice,sizeof(NET_DVR_IPPARACFG_V40)))
        {
            return 1;
        }

    } else if (DH==m_CompanyType)
    {
        DH_IN_MATRIX_GET_CAMERAS dhInMatrix={0};
        dhInMatrix.dwSize=sizeof(DH_IN_MATRIX_GET_CAMERAS);
        DH_OUT_MATRIX_GET_CAMERAS dhOutMatrix={0};
        dhOutMatrix.dwSize=sizeof(DH_OUT_MATRIX_GET_CAMERAS);
        dhOutMatrix.nMaxCameraCount=128;
        dhOutMatrix.pstuCameras=new DH_MATRIX_CAMERA_INFO[dhOutMatrix.nMaxCameraCount];
        memset(dhOutMatrix.pstuCameras,0,sizeof(DH_MATRIX_CAMERA_INFO)*(dhOutMatrix.nMaxCameraCount));
        for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
        {
            dhOutMatrix.pstuCameras[i].dwSize=sizeof(DH_MATRIX_CAMERA_INFO);
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.dwSize=sizeof(DH_REMOTE_DEVICE);
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount=128;
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs=new DH_VIDEO_INPUTS[dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount];
            memset(dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs,0,sizeof(DH_VIDEO_INPUTS)*(dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount));
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs->dwSize=sizeof(DH_VIDEO_INPUTS);
        }
        if(!m_pk->JF_MatrixGetCameras_k(m_lLoginID,&dhInMatrix,&dhOutMatrix,5000))//获取前端摄像机列表信息
        {
            for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
            {
                delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;
            }
            delete[] dhOutMatrix.pstuCameras;
            return 0;
        }
        DH_IN_MATRIX_SET_CAMERAS stInSet = {sizeof(stInSet)};
        DH_OUT_MATRIX_SET_CAMERAS stOutSet = {sizeof(stOutSet)};

        stInSet.nCameraCount = dhOutMatrix.nRetCameraCount;
        stInSet.pstuCameras=dhOutMatrix.pstuCameras;
        DH_MATRIX_CAMERA_INFO deviceInfo={0};
        deviceInfo=stInSet.pstuCameras[lChannel];
        stInSet.pstuCameras[lChannel]=stInSet.pstuCameras[elChannel];
        stInSet.pstuCameras[elChannel]=deviceInfo;
        stInSet.pstuCameras[lChannel].nUniqueChannel=lChannel;
        stInSet.pstuCameras[elChannel].nUniqueChannel=elChannel;
        if(m_pk->JF_MatrixSetCameras_k(m_lLoginID,&stInSet,&stOutSet,3000))
        {
            for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
            {
                delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;
            }
            delete[] dhOutMatrix.pstuCameras;
            return 1;
        }
        for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
        {
            delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;
        }
        delete[] dhOutMatrix.pstuCameras;
    }
}
//批量删除NVR上IPC通道
bool JF_Users::JF_DeleteRemoteDevice(LONG lChannel,LONG elChannel)
{
    AV_CFG_RemoteDevice* stdDevice={0};
    int rSize;
    NET_DVR_IPPARACFG_V40 hkstdDevice={0};
    DWORD hkrsize;
    if(HK==m_CompanyType)
    {
        if(!m_pv->JF_GetRemoteDecice_v(m_lLoginID,NET_DVR_GET_IPPARACFG_V40,0,&hkstdDevice,sizeof(NET_DVR_IPPARACFG_V40),&hkrsize))
        {
            return 0;
        }
        if(elChannel!=0)
        {
            for(int i=lChannel;i<elChannel+1;i++)
            {
                //hkstdDevice.struIPDevInfo[i].byEnable=0;
                hkstdDevice.struStreamMode[i].byGetStreamType=0;
                memset(&(hkstdDevice.struStreamMode[i].uGetStream.struChanInfo), 0, sizeof(NET_DVR_IPCHANINFO));
                memset(&(hkstdDevice.struIPDevInfo[i]), 0, sizeof(NET_DVR_IPDEVINFO_V31));
                /*strncpy((char*)hkstdDevice.struIPDevInfo[i].sPassword,"",16);
                strncpy((char*)hkstdDevice.struIPDevInfo[i].struIP.sIpV4,"",16);
                strncpy((char*)hkstdDevice.struIPDevInfo[i].sUserName,"",32);
                hkstdDevice.struIPDevInfo[i].wDVRPort=NULL;*/
            }
        }else
        {
            //hkstdDevice.struIPDevInfo[lChannel].byEnable=0;
            hkstdDevice.struStreamMode[lChannel].byGetStreamType=0;
            memset(&(hkstdDevice.struStreamMode[lChannel].uGetStream.struChanInfo), 0, sizeof(NET_DVR_IPCHANINFO));
            memset(&(hkstdDevice.struIPDevInfo[lChannel]), 0, sizeof(NET_DVR_IPDEVINFO_V31));
            /*strncpy((char*)hkstdDevice.struIPDevInfo[lChannel+1].sPassword,"",16);
            strncpy((char*)hkstdDevice.struIPDevInfo[lChannel+1].struIP.sIpV4,"",16);
            strncpy((char*)hkstdDevice.struIPDevInfo[lChannel+1].sUserName,"",32);
            hkstdDevice.struIPDevInfo[lChannel+1].wDVRPort=NULL;*/
        }
        if(m_pv->JF_SetRemoteDevice_v(m_lLoginID,NET_DVR_SET_IPPARACFG_V40,0,&hkstdDevice, sizeof(NET_DVR_IPPARACFG_V40)))
        {
           return 1;
        }
    } else if (DH==m_CompanyType)
    {
        DH_IN_MATRIX_GET_CAMERAS dhInMatrix={0};
        dhInMatrix.dwSize=sizeof(DH_IN_MATRIX_GET_CAMERAS);
        DH_OUT_MATRIX_GET_CAMERAS dhOutMatrix={0};
        dhOutMatrix.dwSize=sizeof(DH_OUT_MATRIX_GET_CAMERAS);
        dhOutMatrix.nMaxCameraCount=128;
        dhOutMatrix.pstuCameras=new DH_MATRIX_CAMERA_INFO[dhOutMatrix.nMaxCameraCount];
        memset(dhOutMatrix.pstuCameras,0,sizeof(DH_MATRIX_CAMERA_INFO)*(dhOutMatrix.nMaxCameraCount));
        for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
        {
            dhOutMatrix.pstuCameras[i].dwSize=sizeof(DH_MATRIX_CAMERA_INFO);
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.dwSize=sizeof(DH_REMOTE_DEVICE);
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount=128;
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs=new DH_VIDEO_INPUTS[dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount];
            memset(dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs,0,sizeof(DH_VIDEO_INPUTS)*(dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount));
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs->dwSize=sizeof(DH_VIDEO_INPUTS);
        }
        if(!m_pk->JF_MatrixGetCameras_k(m_lLoginID,&dhInMatrix,&dhOutMatrix,5000))//获取前端摄像机列表信息
        {
            for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
            {
                delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;
            }
            delete[] dhOutMatrix.pstuCameras;
            return 0;
        }
        DH_IN_MATRIX_SET_CAMERAS stInSet = {sizeof(stInSet)};
        DH_OUT_MATRIX_SET_CAMERAS stOutSet = {sizeof(stOutSet)};

        stInSet.nCameraCount = dhOutMatrix.nRetCameraCount;
        stInSet.pstuCameras=dhOutMatrix.pstuCameras;
        if(elChannel==0)
        {
            stInSet.pstuCameras[lChannel].stuRemoteDevice.bEnable = FALSE;
        }else if(elChannel!=0)
        {
            for(int i=lChannel;i<elChannel+1;i++)
            {
                stInSet.pstuCameras[i].stuRemoteDevice.bEnable = FALSE;
            }
        }
        if(m_pk->JF_MatrixSetCameras_k(m_lLoginID,&stInSet,&stOutSet,3000))
        {
            for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
            {
                delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;
            }
            delete[] dhOutMatrix.pstuCameras;
            return 1;
        }
        for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
        {
            delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;
        }
        delete[] dhOutMatrix.pstuCameras;
    }
    return 0;
}
//获取存储上所有的通道信息
bool JF_Users::JF_QueryDevInfo(JF_DeviceInfo* deviceInfo,int* theory,int* real)
{
    if(HK==m_CompanyType)
    {

    }else if(DH==m_CompanyType)
    {
        int a=0,b=0,c=0,d=0;
        NET_IN_GET_CAMERA_STATEINFO stIn={0};
        stIn.dwSize=sizeof(NET_IN_GET_CAMERA_STATEINFO);
        stIn.bGetAllFlag= true;
        NET_OUT_GET_CAMERA_STATEINFO stOut={0};
        stOut.dwSize=sizeof(NET_OUT_GET_CAMERA_STATEINFO);
        stOut.nMaxNum=128;
        stOut.pCameraStateInfo=new NET_CAMERA_STATE_INFO[stOut.nMaxNum];
        memset(stOut.pCameraStateInfo,0,sizeof(NET_CAMERA_STATE_INFO)*(stOut.nMaxNum));
        if(m_pk->JF_QueryDevInfo_k(m_lLoginID, NET_QUERY_GET_CAMERA_STATE, &stIn, &stOut, NULL, 2000))
        {
            a=1;
            for(int i=0;i<stOut.nMaxNum&&i<*theory;i++)
            {
                if(stOut.pCameraStateInfo[i].emConnectionState==EM_CAMERA_STATE_TYPE_CONNECTED)
                {
                    deviceInfo[i].linkState=true;
                } else
                {
                    deviceInfo[i].linkState=false;
                }
            }
        }
        DH_IN_MATRIX_GET_CAMERAS dhInMatrix={0};
        dhInMatrix.dwSize=sizeof(DH_IN_MATRIX_GET_CAMERAS);
        DH_OUT_MATRIX_GET_CAMERAS dhOutMatrix={0};
        dhOutMatrix.dwSize=sizeof(DH_OUT_MATRIX_GET_CAMERAS);
        dhOutMatrix.nMaxCameraCount=128;
        dhOutMatrix.pstuCameras=new DH_MATRIX_CAMERA_INFO[dhOutMatrix.nMaxCameraCount];
        memset(dhOutMatrix.pstuCameras,0,sizeof(DH_MATRIX_CAMERA_INFO)*(dhOutMatrix.nMaxCameraCount));
        for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
        {
            dhOutMatrix.pstuCameras[i].dwSize=sizeof(DH_MATRIX_CAMERA_INFO);
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.dwSize=sizeof(DH_REMOTE_DEVICE);
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount=128;
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs=new DH_VIDEO_INPUTS[dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount];
            memset(dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs,0,sizeof(DH_VIDEO_INPUTS)*(dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount));
            dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs->dwSize=sizeof(DH_VIDEO_INPUTS);
        }
        if(m_pk->JF_MatrixGetCameras_k(m_lLoginID,&dhInMatrix,&dhOutMatrix,5000))
        {
            c=1;
            *real=dhOutMatrix.nRetCameraCount;
            for(int i=0;i<dhOutMatrix.nRetCameraCount&&i<*theory;i++)
            {
                strncpy(deviceInfo[i].Passwd,(const char*)dhOutMatrix.pstuCameras[i].stuRemoteDevice.szPwdEx,32);
                strncpy(deviceInfo[i].szIP,(const char*)dhOutMatrix.pstuCameras[i].stuRemoteDevice.szIp,16);
                strncpy(deviceInfo[i].UserName,(const char*)dhOutMatrix.pstuCameras[i].stuRemoteDevice.szUserEx,32);
                strncpy(deviceInfo[i].deviceType,(const char*)dhOutMatrix.pstuCameras[i].stuRemoteDevice.szDevType,32);
                strncpy(deviceInfo[i].szSerialNo,(const char*)dhOutMatrix.pstuCameras[i].stuRemoteDevice.szSerialNo,48);
                deviceInfo[i].nPort=dhOutMatrix.pstuCameras[i].stuRemoteDevice.nPort;
            }
        }
        int rLong;
        CFG_NETWORK_INFO network={0};
        for(int i=0;i<dhOutMatrix.nRetCameraCount&&i<*theory;i++)
        {
            memset(&network,0,sizeof(CFG_NETWORK_INFO));
            if(m_pk->JF_GetNewDevConfig_k(m_lLoginID,CFG_CMD_NETWORK,i,(void*)&network,sizeof(CFG_NETWORK_INFO),&rLong,2000))
            {
                for(int j=0;j<network.nInterfaceNum;j++)
                {
                    if(strcmp(deviceInfo[i].szIP,network.stuInterfaces[j].szIP)==0)
                    {
                        strncpy((char*)deviceInfo[i].szMac,(const char*)network.stuInterfaces[j].szMacAddress,40);
                    }
                }
            }
        }
         CFG_DEVRECORDGROUP_INFO videoGroup={0};
        if(m_pk->JF_GetNewDevConfig_k(m_lLoginID,CFG_CMD_DEVRECORDGROUP,0,&videoGroup,sizeof(CFG_DEVRECORDGROUP_INFO),&rLong,3000))
        {
            d=1;
            for(int i=0;i<videoGroup.nChannelNum&&i<*theory;i++)
            {
                if(videoGroup.stuDevRecordInfo->byStatus==1)
                {
                    deviceInfo[i].vtState=true;
                } else
                {
                    deviceInfo[i].vtState= false;
                }
            }
        }
        delete[] stOut.pCameraStateInfo;
        for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
        {
            delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;
        }
        delete[] dhOutMatrix.pstuCameras;
        if(!a&&!b&&!c&&!d)
        {
            return 0;
        }
        return 1;
    }

    return 0;
}
//设置前端IPC的账号密码（用户名密码）/IP信息（IP地址与端口）以及重要参数的读取设置（编码信息、OSD信息_通道时间标签以及位置）
bool JF_Users::JF_SetCameraInfo(JF_CameraInfo* CameraInfo)
{
    if(DH==m_CompanyType)
    {
        if(CameraInfo->info==0&&CameraInfo->DevInfo==0)
        {
            DH_IN_MATRIX_GET_CAMERAS dhInMatrix={0};
            dhInMatrix.dwSize=sizeof(DH_IN_MATRIX_GET_CAMERAS);
            DH_OUT_MATRIX_GET_CAMERAS dhOutMatrix={0};
            dhOutMatrix.dwSize=sizeof(DH_OUT_MATRIX_GET_CAMERAS);
            dhOutMatrix.nMaxCameraCount=128;
            dhOutMatrix.pstuCameras=new DH_MATRIX_CAMERA_INFO[dhOutMatrix.nMaxCameraCount];
            memset(dhOutMatrix.pstuCameras,0,sizeof(DH_MATRIX_CAMERA_INFO)*(dhOutMatrix.nMaxCameraCount));
            bool a=FALSE;
            for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
            {
                dhOutMatrix.pstuCameras[i].dwSize=sizeof(DH_MATRIX_CAMERA_INFO);
                dhOutMatrix.pstuCameras[i].stuRemoteDevice.dwSize=sizeof(DH_REMOTE_DEVICE);
                dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount=128;
                dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs=new DH_VIDEO_INPUTS[dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount];
                memset(dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs,0,sizeof(DH_VIDEO_INPUTS)*(dhOutMatrix.pstuCameras[i].stuRemoteDevice.nMaxVideoInputCount));
                dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs->dwSize=sizeof(DH_VIDEO_INPUTS);
            }
            if(m_pk->JF_MatrixGetCameras_k(m_lLoginID,&dhInMatrix,&dhOutMatrix,5000))
            {
                a=TRUE;
                for(int i=0;i<dhOutMatrix.nRetCameraCount;i++)
                {
                    CameraInfo->DevCont[i].lF.nPort=dhOutMatrix.pstuCameras[i].stuRemoteDevice.nPort;
                    strncpy(CameraInfo->DevCont[i].lF.Passwd,(const char*)dhOutMatrix.pstuCameras[i].stuRemoteDevice.szPwdEx,32);
                    strncpy(CameraInfo->DevCont[i].lF.UserName,(const char*)dhOutMatrix.pstuCameras[i].stuRemoteDevice.szUserEx,32);
                    strncpy(CameraInfo->DevCont[i].lF.szIP,(const char*)dhOutMatrix.pstuCameras[i].stuRemoteDevice.szIp,16);
                }
            }
            for (int i=0;i<dhOutMatrix.nMaxCameraCount;i++)
            {
                delete[] dhOutMatrix.pstuCameras[i].stuRemoteDevice.pstuVideoInputs;
            }
            delete[] dhOutMatrix.pstuCameras;
            return a;
        }
        if(CameraInfo->DevInfo==1)
        {
            CFG_ENCODE_INFO* stEncode=new CFG_ENCODE_INFO[128];
            memset(stEncode,0,sizeof(CFG_ENCODE_INFO)*128);
            int error;
            if(m_pk->JF_GetNewDevConfig_k(m_lLoginID,CFG_CMD_ENCODE,0xFFFFFFFF,(char*)stEncode,sizeof(CFG_ENCODE_INFO)*128,&error,2000))
            {
                if(CameraInfo->info==0)
                {
                    for(int i=0;i<128;i++)
                    {
                        CameraInfo->DevCont[i].aF.emCompression = stEncode[i].stuMainStream[0].stuAudioFormat.emCompression;
                        CameraInfo->DevCont[i].aF.nDepth = stEncode[i].stuMainStream[0].stuAudioFormat.nDepth;
                        CameraInfo->DevCont[i].aF.nFrameType = stEncode[i].stuMainStream[0].stuAudioFormat.nFrameType;
                        CameraInfo->DevCont[i].aF.nFrequency = stEncode[i].stuMainStream[0].stuAudioFormat.nFrequency;
                        CameraInfo->DevCont[i].aF.nMode = stEncode[i].stuMainStream[0].stuAudioFormat.nMode;
                        CameraInfo->DevCont[i].aF.nPacketPeriod = stEncode[i].stuMainStream[0].stuAudioFormat.nPacketPeriod;
                        CameraInfo->DevCont[i].vF.nFrameType=stEncode[i].stuMainStream[0].stuVideoFormat.nFrameType;
                        CameraInfo->DevCont[i].vF.emCompression=stEncode[i].stuMainStream[0].stuVideoFormat.emCompression;
                        CameraInfo->DevCont[i].vF.emBitRateControl=stEncode[i].stuMainStream[0].stuVideoFormat.emBitRateControl;
                        CameraInfo->DevCont[i].vF.emImageQuality=stEncode[i].stuMainStream[0].stuVideoFormat.emImageQuality;
                        CameraInfo->DevCont[i].vF.nBitRate=stEncode[i].stuMainStream[0].stuVideoFormat.nBitRate;
                        CameraInfo->DevCont[i].vF.nFrameRate=stEncode[i].stuMainStream[0].stuVideoFormat.nFrameRate;
                        CameraInfo->DevCont[i].vF.nHeight=stEncode[i].stuMainStream[0].stuVideoFormat.nHeight;
                        CameraInfo->DevCont[i].vF.nIFrameInterval=stEncode[i].stuMainStream[0].stuVideoFormat.nIFrameInterval;
                        CameraInfo->DevCont[i].vF.nWidth=stEncode[i].stuMainStream[0].stuVideoFormat.nWidth;
                    }
                }
                if(CameraInfo->info==1)
                {
                    for(int i=0;i<128;i++)
                    {
                        stEncode[i].stuMainStream[0].stuAudioFormat.emCompression=CameraInfo->DevCont[i].aF.emCompression;
                        stEncode[i].stuMainStream[0].stuAudioFormat.nDepth=CameraInfo->DevCont[i].aF.nDepth;
                        stEncode[i].stuMainStream[0].stuAudioFormat.nFrameType=CameraInfo->DevCont[i].aF.nFrameType;
                        stEncode[i].stuMainStream[0].stuAudioFormat.nFrequency=CameraInfo->DevCont[i].aF.nFrequency;
                        stEncode[i].stuMainStream[0].stuAudioFormat.nMode=CameraInfo->DevCont[i].aF.nMode;
                        stEncode[i].stuMainStream[0].stuAudioFormat.nPacketPeriod=CameraInfo->DevCont[i].aF.nPacketPeriod;
                        stEncode[i].stuMainStream[0].stuVideoFormat.nFrameType=CameraInfo->DevCont[i].vF.nFrameType;
                        stEncode[i].stuMainStream[0].stuVideoFormat.emCompression=CameraInfo->DevCont[i].vF.emCompression;
                        stEncode[i].stuMainStream[0].stuVideoFormat.emBitRateControl=CameraInfo->DevCont[i].vF.emBitRateControl;
                        stEncode[i].stuMainStream[0].stuVideoFormat.emImageQuality=CameraInfo->DevCont[i].vF.emImageQuality;
                        stEncode[i].stuMainStream[0].stuVideoFormat.nBitRate=CameraInfo->DevCont[i].vF.nBitRate;
                        stEncode[i].stuMainStream[0].stuVideoFormat.nFrameRate=CameraInfo->DevCont[i].vF.nFrameRate;
                        stEncode[i].stuMainStream[0].stuVideoFormat.nHeight=CameraInfo->DevCont[i].vF.nHeight;
                        stEncode[i].stuMainStream[0].stuVideoFormat.nIFrameInterval=CameraInfo->DevCont[i].vF.nIFrameInterval;
                        stEncode[i].stuMainStream[0].stuVideoFormat.nWidth=CameraInfo->DevCont[i].vF.nWidth;
                        if(m_pk->JF_SetNewDevConfig_k(m_lLoginID,CFG_CMD_ENCODE,0xFFFFFFFF,(char*)stEncode,sizeof(CFG_ENCODE_INFO)*128,&error,0,2000))
                        {
                            delete[] stEncode;
                            return 1;
                        }
                        delete[] stEncode;
                    }
                }
            }
        }

    }else if(HK==m_CompanyType)
    {

    }

    return 0;
}
//设置账号密码IP信息
/*数值	操作类型	    opParam对应结构体类型	     subParam对应结构体类型
  0	增加用户组	 USER_GROUP_INFO_NEW
  1	删除用户组	 USER_GROUP_INFO_NEW
  2	修改用户组	 USER_GROUP_INFO_NEW	 USER_GROUP_INFO_NEW
  3	增加用户	     USER_INFO_NEW
  4	删除用户	     USER_INFO_NEW
  5	修改用户	     USER_INFO_NEW	         USER_INFO_NEW
  6	修改用户密码	 USER_INFO_NEW	         USER_INFO_NEW*/
bool JF_Users::JF_OperateUserInfoNew(int nOperateType,Login_Format* CameraInfo,int waittime)
{
    if(DH==m_CompanyType)
    {
        if(nOperateType==3)
        {
            USER_INFO_NEW stLogin={0};
            stLogin.dwSize=sizeof(USER_INFO_NEW);
            strncpy((char*)stLogin.byIsAnonymous,"0",1);
            strncpy(stLogin.name,CameraInfo->UserName,64);
            strncpy(stLogin.passWord,CameraInfo->Passwd,64);
            USER_INFO_NEW subPam={0};
            subPam.dwSize=sizeof(USER_INFO_NEW);
            if(m_pk->JF_OperateUserInfoNew_k(m_lLoginID,6,&stLogin,&subPam,NULL,waittime))
            {
                DEVICE_NET_INFO_EX pDevNetInfo={0};
                pDevNetInfo.iIPVersion=4;
                if(m_pk->JF_ModifyDevice_k(&pDevNetInfo,1000,NULL,NULL,NULL))
                {
                    return 1;
                }
            }
        }
    }else if(HK==m_CompanyType)
    {

    }
    return 0;
}
//获取设备信息包括（设备型号、系统信息（CPU、内存使用情况）、硬盘信息（硬盘总容量、剩余可用空间、锁定空间大小）、版本信息等）
bool JF_Users::JF_GetDeviceInfo(JF_Details* DetailsInfo)
{
    if(DH==m_CompanyType)
    {
        int a=0,b=0,c=0,d=0;
        DHDEV_SYSTEM_ATTR_CFG stLpOutBuffer={0};
        DWORD lpBytesReturned;
        if(m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DEVICECFG,0,&stLpOutBuffer,sizeof(DHDEV_SYSTEM_ATTR_CFG),&lpBytesReturned,1000)==0)
        {
            strncpy((char*)DetailsInfo->DeviceType,(const char*)stLpOutBuffer.szDevType,32);
            a=1;
        }
        DH_SYSTEM_STATUS pstuStatus={0};
        pstuStatus.dwSize=sizeof(DH_SYSTEM_STATUS);
        pstuStatus.pstuCPU=new DH_CPU_STATUS;
        pstuStatus.pstuFan=new DH_FAN_STATUS;
        pstuStatus.pstuMemory=new DH_MEMORY_STATUS;
        pstuStatus.pstuPower=new DH_POWER_STATUS;
        pstuStatus.pstuTemp=new DH_TEMPERATURE_STATUS;
        memset(pstuStatus.pstuCPU,0,sizeof(DH_CPU_STATUS));
        memset(pstuStatus.pstuFan,0,sizeof(DH_FAN_STATUS));
        memset(pstuStatus.pstuMemory,0,sizeof(DH_MEMORY_STATUS));
        memset(pstuStatus.pstuPower,0,sizeof(DH_POWER_STATUS));
        memset(pstuStatus.pstuTemp,0,sizeof(DH_TEMPERATURE_STATUS));
        pstuStatus.pstuMemory->dwSize=sizeof(DH_MEMORY_STATUS);
        pstuStatus.pstuMemory->stuMemory.dwSize=sizeof(DH_MEMORY_INFO);
        pstuStatus.pstuCPU->dwSize=sizeof(DH_CPU_STATUS);
        for(int i=0;i<16;i++)
        {
            pstuStatus.pstuCPU->stuCPUs[i].dwSize= sizeof(DH_CPU_INFO);
        }
        pstuStatus.pstuTemp->dwSize=sizeof(DH_TEMPERATURE_STATUS);
        for(int i=0;i<16;i++)
        {
            pstuStatus.pstuTemp->stuTemps[i].dwSize= sizeof(DH_TEMPERATURE_INFO);
        }
        pstuStatus.pstuPower->dwSize=sizeof(DH_POWER_STATUS);
        for(int i=0;i<16;i++)
        {
            pstuStatus.pstuPower->stuBatteries[i].dwSize= sizeof(DH_BATTERY_INFO);
            pstuStatus.pstuPower->stuPowers[i].dwSize=sizeof(DH_POWER_INFO);
        }
        pstuStatus.pstuFan->dwSize=sizeof(DH_FAN_STATUS);
        for(int i=0;i<16;i++)
        {
            pstuStatus.pstuFan->stuFans[i].dwSize=sizeof(DH_FAN_INFO);
        }
        if(m_pk->JF_QuerySystemStatus_k(m_lLoginID,&pstuStatus,1000))
        {
            b=1;
            DetailsInfo->nCount=pstuStatus.pstuCPU->nCount;
            for(int i=0;i<pstuStatus.pstuCPU->nCount;i++)
            {
                DetailsInfo->stuCPUs[i].nUsage=pstuStatus.pstuCPU->stuCPUs[i].nUsage;
            }
            DetailsInfo->stuMemory.dwFree=pstuStatus.pstuMemory->stuMemory.dwFree;
            DetailsInfo->stuMemory.dwTotal=pstuStatus.pstuMemory->stuMemory.dwTotal;
        }
        DH_HARDDISK_STATE pBuf={0};
        int pRetLen=0;
        if(m_pk->JF_QueryDVRState_k(m_lLoginID,DH_DEVSTATE_DISK,(char*)&pBuf,sizeof(DH_HARDDISK_STATE),&pRetLen)==0)
        {
            DetailsInfo->diskInfo.dwDiskNum=pBuf.dwDiskNum;
            for(int i=0;i<pBuf.dwDiskNum;i++)
            {
                DetailsInfo->diskInfo.stDisks[i].dwFreeSpace=pBuf.stDisks[i].dwFreeSpace;
                DetailsInfo->diskInfo.stDisks[i].dwVolume=pBuf.stDisks[i].dwVolume;
            }
            c=1;
        }
        DHDEV_VERSION_INFO softst={0};
        if(m_pk->JF_QueryDVRState_k(m_lLoginID,DH_DEVSTATE_SOFTWARE,(char*)&softst,sizeof(DHDEV_VERSION_INFO),&pRetLen)==0)
        {
            strncpy(DetailsInfo->szSoftWareVersion,softst.szSoftWareVersion,128);
            d=1;
        }
        delete pstuStatus.pstuFan;
        delete pstuStatus.pstuPower;
        delete pstuStatus.pstuTemp;
        delete pstuStatus.pstuCPU;
        delete pstuStatus.pstuMemory;
        if(!a&&!b&&!c&&!d)
        {
            return 0;
        }
        return 1;

    }
   return 0;
}
//NVR批量和单个通道的文件锁定信息读取（通道号、文件列表以及文件大小——支持文件和时间锁定及解锁功能），并支持批量和单个通道设置文件锁定计划设置
bool JF_Users::JF_MarkInfo(JF_MarkTImeInfo* stInfo)
{
    if(DH==m_CompanyType)
    {
        if(stInfo->bFlag==true)
        {
            NET_IN_SET_MARK_FILE_BY_TIME pInParam = {0};
            pInParam.dwSize = sizeof(NET_IN_SET_MARK_FILE_BY_TIME);
            pInParam.bFlag = true;
            pInParam.stuEndTime.dwDay = stInfo->stuEndTime.dwDay;
            pInParam.stuEndTime.dwHour = stInfo->stuEndTime.dwHour;
            pInParam.stuEndTime.dwMillisecond = stInfo->stuEndTime.dwMillisecond;
            pInParam.stuEndTime.dwMinute = stInfo->stuEndTime.dwMinute;
            pInParam.stuEndTime.dwMonth = stInfo->stuEndTime.dwMonth;
            pInParam.stuEndTime.dwSecond = stInfo->stuEndTime.dwSecond;
            pInParam.stuEndTime.dwYear = stInfo->stuEndTime.dwYear;
            pInParam.stuStartTime.dwDay = stInfo->stuStartTime.dwDay;
            pInParam.stuStartTime.dwHour = stInfo->stuStartTime.dwHour;
            pInParam.stuStartTime.dwMillisecond = stInfo->stuStartTime.dwMillisecond;
            pInParam.stuStartTime.dwMinute = stInfo->stuStartTime.dwMinute;
            pInParam.stuStartTime.dwMonth = stInfo->stuStartTime.dwMonth;
            pInParam.stuStartTime.dwSecond = stInfo->stuStartTime.dwSecond;
            pInParam.stuStartTime.dwYear = stInfo->stuStartTime.dwYear;
            NET_OUT_SET_MARK_FILE_BY_TIME pOutParam = {0};
            pOutParam.dwSize = sizeof(NET_OUT_SET_MARK_FILE_BY_TIME);
            if (m_pk->JF_SetMarkFileByTime_k(m_lLoginID, &pInParam, &pOutParam, 1000)) {
                return 1;
            }
        }else if(stInfo->bFlag==false)
        {
            NET_IN_GET_MARK_INFO pInParamk={0};
            pInParamk.dwSize=sizeof(NET_IN_GET_MARK_INFO);
            NET_OUT_GET_MARK_INFO pOutParamk={0};
            pOutParamk.dwSize=sizeof(NET_OUT_GET_MARK_INFO);
            if(m_pk->JF_GetMarkInfo_k(m_lLoginID,&pInParamk,&pOutParamk,1000))
            {
                return 1;
            }
        }
    }else if(HK==m_CompanyType)
    {

    }
    return 0;
}
//通过SDK控制存储抓取实时图片
bool JF_Users::JF_SnapPictureEx(LONG lChannel)
{
    if(DH==m_CompanyType)
    {
        static short g_nCmdSerial = 0;
        SNAP_PARAMS stuSnapParams;
        stuSnapParams.Channel=lChannel;
        stuSnapParams.mode=0;
        stuSnapParams.CmdSerial=++g_nCmdSerial;
        if(m_pk->JF_SnapPictureEx_k(m_lLoginID,&stuSnapParams,NULL))
        {
            return 1;
        }
    }else if(HK==m_CompanyType)
    {

    }
    return 0;
}
//控制前端设备恢复出厂设置以及重启操作
bool JF_Users::JF_ResetSystem(int stType)
{
    if(DH==m_CompanyType)
    {
        if(stType==0)
        {
            NET_IN_RESET_SYSTEM pstIn={0};
            pstIn.dwSize=sizeof(NET_IN_RESET_SYSTEM);
            NET_OUT_RESET_SYSTEM pstOut={0};
            pstOut.dwSize=sizeof(NET_OUT_RESET_SYSTEM);
            if(m_pk->JF_ResetSystem_k(m_lLoginID,&pstIn,&pstOut,1000))
            {
                return 1;
            }
        }else if(stType==1)
        {
            if(m_pk->JF_RebootDVR_k(m_lLoginID)==0)
            {
                return 1;
            }
        }else if(stType==2)
        {
            if(m_pk->JF_ShutDownDVR_k(m_lLoginID)==0)
            {
                return 1;
            }
        }
    }else if(HK==m_CompanyType)
    {

    }
    return 0;
}
//控制配置文件导出
bool JF_Users::JF_ExportConfigFile(char *szFileName,fMarkConfigPosCallBack cbUploadPos,long dwUserData)
{
    if(DH==m_CompanyType)
    {
        if(m_pk->JF_ExportConfigFile_k(m_lLoginID,DH_CONFIGFILE_ALL,szFileName,cbUploadPos,dwUserData))
        {
            return 1;
        }
    }else if(HK==m_CompanyType)
    {

    }
    return 0;
}
//控制配置文件导入
bool JF_Users::JF_ImportConfigFile(char *szFileName,fMarkConfigPosCallBack cbUploadPos,long dwUserData)
{
    if(DH==m_CompanyType)
    {
        if(m_pk->JF_ImportConfigFile_k(m_lLoginID,szFileName,cbUploadPos,dwUserData,0))
        {
            return 1;
        }
    }else if(HK==m_CompanyType)
    {

    }
    return 0;
}
//获取OSD接口
bool JF_Users::JF_GetDVRConfig_OSD(LONG lChannel,char* OSD_lpOutBuffer)
{
    if(HK==m_CompanyType)
    {
        return m_pv->JF_GetDVRConfig_OSD_v(m_lLoginID,lChannel,OSD_lpOutBuffer);
    }else if(DH==m_CompanyType)
    {
        return m_pk->JF_GetDVRConfig_OSD_k(m_lLoginID,lChannel,OSD_lpOutBuffer);
    }
}
//获取字符叠加的接口
bool JF_Users::JF_GetDVRConfig_CharOSD(LONG lChannel,char* char_lpOutBuffer)
{
    if(HK==m_CompanyType)
    {
        return m_pv->JF_GetDVRConfig_CharOSD_v(m_lLoginID,lChannel,char_lpOutBuffer);
    }else if(DH==m_CompanyType)
    {
        return m_pk->JF_GetDVRConfig_CharOSD_k(m_lLoginID,lChannel,char_lpOutBuffer);
    }
}
//设置OSD接口
bool JF_Users::JF_SetDVRConfig_OSD(LONG lChannel,char* OSD_lpInBuffer)
{
    if(HK==m_CompanyType)
    {
        return m_pv->JF_SetDVRConfig_OSD_v(m_lLoginID,lChannel,OSD_lpInBuffer);
    }else if(DH==m_CompanyType)
    {
        return m_pk->JF_SetDVRConfig_OSD_k(m_lLoginID,lChannel,OSD_lpInBuffer);
    }
}
//设置字符叠加的接口
bool JF_Users::JF_SetDVRConfig_CharOSD(LONG lChannel,char* char_lpInBuffer)
{
    if(HK==m_CompanyType)
    {
        return m_pv->JF_SetDVRConfig_CharOSD_v(m_lLoginID,lChannel,char_lpInBuffer);
    }else if(DH==m_CompanyType)
    {
        return m_pk->JF_SetDVRConfig_CharOSD_k(m_lLoginID,lChannel,char_lpInBuffer);
    }
}
//云台所有操作
bool JF_Users::JF_PTZAll(LONG lChannel,int dwPTZCommand, LONG param1, LONG param2, LONG param3,bool dwStop)
{
    NET_DVR_PTZ_PATTERN sPattern;
    NET_DVR_PTZ_PATTERN tPattern;
    NET_DVR_PTZ_PATTERN srPattern;
    NET_DVR_PTZ_PATTERN ssPattern;
    NET_DVR_LINEARSCAN lstruLinearScan = {0};
    NET_DVR_LINEARSCAN rstruLinearScan = {0};
    NET_DVR_PTZ_PARKACTION_CFG stm_struPtzParkAciton;
    NET_DVR_PTZ_PARKACTION_CFG m_struPtzParkAciton;
    NET_DVR_PTZPOS PosPostion;
    NET_DVR_PTZ_PATTERN cPattern;
    if(HK==m_CompanyType)
    {
        switch(dwPTZCommand)
        {
            case 0: //左转
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,PAN_LEFT,dwStop,param2))
                {
                    return false;
                }
                break;
            case 1: //右转
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,PAN_RIGHT,dwStop,param2))
                {
                    return false;
                }
                break;
            case 2: //上抬
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,TILT_UP,dwStop,param2))
                {
                    return false;
                }
                break;
            case 3: //下抑
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,TILT_DOWN,dwStop,param2))
                {
                    return false;
                }
                break;
            case 4: //左上
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,UP_LEFT,dwStop,param2))
                {
                    return false;
                }
                break;
            case 5: //左下
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,DOWN_LEFT,dwStop,param2))
                {
                    return false;
                }
                break;
            case 6: //右上
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,UP_RIGHT,dwStop,param2))
                {
                    return false;
                }
                break;
            case 7: //右下
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,DOWN_RIGHT,dwStop,param2))
                {
                    return false;
                }
                break;
            case 8: //变倍+
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,ZOOM_IN,dwStop,param2))
                {
                    return false;
                }
                break;
            case 9: //变倍-
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,ZOOM_OUT,dwStop,param2))
                {
                    return false;
                }
                break;
            case 10://调焦+
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,FOCUS_FAR,dwStop,param2))
                {
                    return false;
                }
                break;
            case 11://调焦-
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,FOCUS_NEAR,dwStop,param2))
                {
                    return false;
                }
                break;
            case 12: //光圈+
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,IRIS_OPEN,dwStop,param2))
                {
                    return false;
                }
                break;
            case 13: //设置预置点
                if(-1==m_pv->JF_PTZPreset_v(m_lLoginID,lChannel,SET_PRESET,param2))
                {
                    return false;
                }
                break;
            case 14: //清除预置点
                if(-1 == m_pv->JF_PTZPreset_v(m_lLoginID,lChannel,CLE_PRESET,param2))
                {
                    return false;
                }
                break;
            case 15: //转到预置点
                if(-1 == m_pv->JF_PTZPreset_v(m_lLoginID,lChannel,GOTO_PRESET,param2))
                {
                    return false;
                }
                break;
            case 16: //点间轮询
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,RUN_SEQ,param1,param2,param3))
                {
                    return false;
                }
                break;
            case 17: //停止巡航
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,STOP_SEQ,param1,param2,param3))
                {
                    return false;
                }
                break;
            case 18: //将预置点加入巡航序列
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,FILL_PRE_SEQ,param1,param2,param3))
                {
                    return false;
                }
                break;
            case 19: //将预置点从巡航序列删除
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,CLE_PRE_SEQ,param1,param2,param3))
                {
                    return false;
                }
                break;
            case 20: //开始水平旋转
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,PAN_AUTO,TRUE,param2))
                {
                    return  false;
                }
                break;
            case 21: //停止水平旋转
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,PAN_AUTO,FALSE,param2))
                {
                    return false;
                }
                break;
            case 22: //清除巡航路线
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,DEL_SEQ,param1,NULL,NULL))
                {
                    return false;
                }
                break;
            case 23: //开始记录花样扫描
                sPattern.dwSize=sizeof(NET_DVR_PTZ_PATTERN);
                sPattern.dwChannel=lChannel;
                sPattern.dwPatternCmd=STA_MEM_CRUISE;
                sPattern.dwPatternID=param1;
                if(!m_pv->JF_PTZRemoteControl_v(m_lLoginID,NET_DVR_CONTROL_PTZ_PATTERN,&sPattern,sizeof(NET_DVR_PTZ_PATTERN)))
                {
                    return false;
                }
                break;
            case 24: //停止记录花样扫描
                tPattern.dwSize=sizeof(NET_DVR_PTZ_PATTERN);
                tPattern.dwChannel=lChannel;
                tPattern.dwPatternCmd=STO_MEM_CRUISE;
                tPattern.dwPatternID=param1;
                if(!m_pv->JF_PTZRemoteControl_v(m_lLoginID,NET_DVR_CONTROL_PTZ_PATTERN,&tPattern,sizeof(NET_DVR_PTZ_PATTERN)))
                {
                    return false;
                }
                break;
            case 25: //开始花样扫描
                srPattern.dwSize=sizeof(NET_DVR_PTZ_PATTERN);
                srPattern.dwChannel=lChannel;
                srPattern.dwPatternCmd=RUN_CRUISE;
                srPattern.dwPatternID=param1;
                if(!m_pv->JF_PTZRemoteControl_v(m_lLoginID,NET_DVR_CONTROL_PTZ_PATTERN,&srPattern,sizeof(NET_DVR_PTZ_PATTERN)))
                {
                    return false;
                }
                break;
            case 26: //停止花样扫描
                ssPattern.dwSize=sizeof(NET_DVR_PTZ_PATTERN);
                ssPattern.dwChannel=lChannel;
                ssPattern.dwPatternCmd=STOP_CRUISE;
                ssPattern.dwPatternID=param1;
                if(!m_pv->JF_PTZRemoteControl_v(m_lLoginID,NET_DVR_CONTROL_PTZ_PATTERN,&ssPattern,sizeof(NET_DVR_PTZ_PATTERN)))
                {
                    return false;
                }
                break;
            case 27: //设置左边界
                lstruLinearScan.dwSize = sizeof(NET_DVR_LINEARSCAN);
                lstruLinearScan.dwChan = lChannel;
                lstruLinearScan.byLinearScanType = 1;
                if (!m_pv->JF_PTZRemoteControl_v(m_lLoginID, NET_DVR_REMOTECONTROL_LINEARSCAN, &lstruLinearScan, sizeof(lstruLinearScan)))
                {
                    return false;
                }
                break;
            case 28: //设置右边界
                rstruLinearScan.dwSize = sizeof(NET_DVR_LINEARSCAN);
                rstruLinearScan.dwChan = lChannel;
                rstruLinearScan.byLinearScanType = 2;
                if (!m_pv->JF_PTZRemoteControl_v(m_lLoginID, NET_DVR_REMOTECONTROL_LINEARSCAN, &rstruLinearScan, sizeof(rstruLinearScan)))
                {
                    return false;
                }
                break;
            case 29: //开始线扫
                m_struPtzParkAciton.dwSize = sizeof(m_struPtzParkAciton);
                m_struPtzParkAciton.byEnable = 1;
                m_struPtzParkAciton.dwParkTime = 5;
                m_struPtzParkAciton.wActionType = 8;
                m_struPtzParkAciton.wID = 0;
                m_struPtzParkAciton.byOneTouchSwitch = 0;
                if(-1==m_pv->JF_SetRemoteDevice_v(m_lLoginID,NET_DVR_GET_PTZ_PARKACTION_CFG,lChannel,&m_struPtzParkAciton, sizeof(NET_DVR_PTZ_PARKACTION_CFG)))
                {
                    return false ;
                }
                break;
            case 30: //停止线扫
                stm_struPtzParkAciton.dwSize = sizeof(m_struPtzParkAciton);
                stm_struPtzParkAciton.byEnable = 0;
                stm_struPtzParkAciton.dwParkTime = 5;
                stm_struPtzParkAciton.wActionType = 8;
                stm_struPtzParkAciton.wID = 0;
                stm_struPtzParkAciton.byOneTouchSwitch = 0;
                if(-1==m_pv->JF_SetRemoteDevice_v(m_lLoginID,NET_DVR_GET_PTZ_PARKACTION_CFG,lChannel,&stm_struPtzParkAciton, sizeof(NET_DVR_PTZ_PARKACTION_CFG)))
                {
                    return false ;
                }
                break;
            case 31: //快速定位
                PosPostion.wAction=1;
                PosPostion.wPanPos=param1;
                PosPostion.wTiltPos=param2;
                PosPostion.wZoomPos=param3;
                if(-1==m_pv->JF_SetRemoteDevice_v(m_lLoginID,NET_DVR_SET_PTZPOS,lChannel,&PosPostion,sizeof(NET_DVR_PTZPOS)))
                {
                    return false;
                }
                break;
            case 32 : //三维精确定位
                /*if(-1==m_pv->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_EXACTGOTO,param1,param2,param3,FALSE))
                {
                    return  false;
                }*/
                break;
            case 33: //三维定位重设零点
                /*if(-1==m_pv->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_RESETZERO,NULL,NULL,NULL,FALSE))
                {
                    return false;
                }*/
                break;
            case 34: //清除模式
                cPattern.dwSize=sizeof(NET_DVR_PTZ_PATTERN);
                cPattern.dwChannel=lChannel;
                cPattern.dwPatternCmd=DELETE_CRUISE;
                cPattern.dwPatternID=param1;
                if(!m_pv->JF_PTZRemoteControl_v(m_lLoginID,NET_DVR_CONTROL_PTZ_PATTERN,&cPattern,sizeof(NET_DVR_PTZ_PATTERN)))
                {
                    return false;
                }
                break;
            case 35: //光圈-
                if(!m_pv->JF_PTZControlWithSpeed_v(m_lLoginID,lChannel,IRIS_CLOSE,dwStop,param2))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
        return true;
    }
    if(DH==m_CompanyType)
    {
        switch(dwPTZCommand)
        {
            case 0: //左转
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_LEFT_CONTROL,NULL,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 1: //右转
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_RIGHT_CONTROL,NULL,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 2: //上抬
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_UP_CONTROL,NULL,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 3: //下抑
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_DOWN_CONTROL,NULL,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 4: //左上
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_LEFTTOP,param1,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 5: //左下
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_LEFTDOWN,param1,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 6: //右上
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_RIGHTTOP,param1,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 7: //右下
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_RIGHTDOWN,param1,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 8: //变倍+
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_ZOOM_ADD_CONTROL,NULL,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 9: //变倍-
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_ZOOM_DEC_CONTROL,NULL,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 10://调焦+
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_FOCUS_ADD_CONTROL,NULL,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 11://调焦-
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_FOCUS_DEC_CONTROL,NULL,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 12: //光圈+
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_APERTURE_ADD_CONTROL,NULL,param2,NULL,dwStop))
                {
                    return false;
                }
                break;
            case 13: //设置预置点
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_SET_CONTROL,NULL,param2,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 14: //清除预置点
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_DEL_CONTROL,NULL,param2,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 15: //转到预置点
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_MOVE_CONTROL,NULL,param2,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 16: //点间轮询
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_LOOP_CONTROL,param1,NULL,param3,FALSE))
                {
                    return false;
                }
                break;
            case 17: //停止巡航
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_LOOP_CONTROL,param1,NULL,param3,FALSE))
                {
                    return false;
                }
                break;
            case 18: //将预置点加入巡航序列
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_ADDTOLOOP,param1,param2,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 19: //将预置点从巡航序列删除
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_DELFROMLOOP,param1,param2,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 20: //开始水平旋转
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_STARTPANCRUISE,NULL,NULL,NULL,FALSE))
                {
                    return  false;
                }
                break;
            case 21: //停止水平旋转
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_STOPPANCRUISE,NULL,NULL,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 22: //清除巡航路线
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_CLOSELOOP,param1,NULL,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 23: //开始记录轨迹
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_SETMODESTART,param1,NULL,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 24: //停止记录轨迹
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_SETMODESTOP,param1,NULL,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 25: //开始轨迹
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_RUNMODE,param1,NULL,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 26: //停止轨迹
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_STOPMODE,param1,NULL,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 27: //设置左边界
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_SETLEFTBORDER,NULL,NULL,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 28: //设置右边界
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_SETRIGHTBORDER,NULL,NULL,NULL,FALSE))
                {
                    return false ;
                }
                break;
            case 29: //开始线扫
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_STARTLINESCAN,NULL,NULL,NULL,FALSE))
                {
                    return false ;
                }
                break;
            case 30: //停止线扫
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_CLOSELINESCAN,NULL,NULL,NULL,FALSE))
                {
                    return  false;
                }
                break;
            case 31: //快速定位
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_FASTGOTO,param1,param2,param3,FALSE))
                {
                    return false;
                }
                break;
            case 32 : //3维精确定位
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_EXACTGOTO,param1,param2,param3,FALSE))
                {
                    return  false;
                }
                break;
            case 33: //三维定位重设零点
                if(-1==m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_RESETZERO,NULL,NULL,NULL,FALSE))
                {
                    return false;
                }
                break;
            case 34: //清除模式
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_DELETEMODE,param1,NULL,NULL,FALSE))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
        return true;
    }
}
//云台基本控制                  通道号，         云台控制命令  ， 大华参数 速度，  云台开始动作或者停止动作    0-开始，1-结束
bool JF_Users::JF_PTZControl(LONG lChannel, int PTZCommand, int param, BOOL bStop)
{
    if(HK == m_CompanyType)
    {
        switch(PTZCommand)
        {
            case 0: //左转
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,PAN_LEFT,bStop))
                {
                    return false;
                }
                break;
            case 1: //右转
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,PAN_RIGHT,bStop))
                {
                    return false;
                }
                break;
            case 2: //上抬
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,TILT_UP,bStop))
                {
                    return false;
                }
                break;
            case 3: //下抑
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,TILT_DOWN,bStop))
                {
                    return false;
                }
                break;
            case 4: //左上
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,UP_LEFT,bStop))
                {
                    return false;
                }
                break;
            case 5: //左下
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,DOWN_LEFT,bStop))
                {
                    return false;
                }
                break;
            case 6: //右上
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,UP_RIGHT,bStop))
                {
                    return false;
                }
                break;
            case 7: //右下
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,DOWN_RIGHT,bStop))
                {
                    return false;
                }
                break;
            case 8: //变倍+
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,ZOOM_IN,bStop))
                {
                    return false;
                }
                break;
            case 9: //变倍-
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,ZOOM_OUT,bStop))
                {
                    return false;
                }
                break;
            case 10://调焦+
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,FOCUS_NEAR,bStop))
                {
                    return false;
                }
                break;
            case 11://调焦-
                if(-1 == m_pv->JF_PTZControl_v(m_lLoginID,lChannel,FOCUS_FAR,bStop))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        switch(PTZCommand)
        {
            case 0: //左转
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_LEFT_CONTROL,0,param,0,bStop))
                {
                    return false;
                }
                break;
            case 1: //右转
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_RIGHT_CONTROL,0,param,0,bStop))
                {
                    return false;
                }
                break;
            case 2: //上抬
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_UP_CONTROL,0,param,0,bStop))
                {
                    return false;
                }
                break;
            case 3: //下抑
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_DOWN_CONTROL,0,param,0,bStop))
                {
                    return false;
                }
                break;
            case 4: //左上
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_LEFTTOP,param,param,0,bStop))
                {
                    return false;
                }
                break;
            case 5: //左下
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_LEFTDOWN,param,param,0,bStop))
                {
                    return false;
                }
                break;
            case 6: //右上
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_RIGHTTOP,param,param,0,bStop))
                {
                    return false;
                }
                break;
            case 7: //右下
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_RIGHTDOWN,param,param,0,bStop))
                {
                    return false;
                }
                break;
            case 8: //变倍+
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_ZOOM_ADD_CONTROL,0,param,0,bStop))
                {
                    return false;
                }
                break;
            case 9: //变倍-
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_ZOOM_DEC_CONTROL,0,param,0,bStop))
                {
                    return false;
                }
                break;
            case 10://调焦+
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_FOCUS_ADD_CONTROL,0,param,0,bStop))
                {
                    return false;
                }
                break;
            case 11://调焦-
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_FOCUS_DEC_CONTROL,0,param,0,bStop))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//云台预置点操作             通道号，              大华参数 速度   云台序号
bool JF_Users::JF_PTZPreset(LONG lChannel, int PTZCommand, DWORD dwPresetIndex)
{
    if(HK == m_CompanyType)
    {
        switch(PTZCommand)
        {
            case 0: //设置预置点
                if(-1 == m_pv->JF_PTZPreset_v(m_lLoginID,lChannel,SET_PRESET,dwPresetIndex))
                {
                    return false;
                }
                break;
            case 1: //清除预置点
                if(-1 == m_pv->JF_PTZPreset_v(m_lLoginID,lChannel,CLE_PRESET,dwPresetIndex))
                {
                    return false;
                }
                break;
            case 2: //转到预置点
                if(-1 == m_pv->JF_PTZPreset_v(m_lLoginID,lChannel,GOTO_PRESET,dwPresetIndex))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        switch(PTZCommand)
        {
            case 0: //设置预置点
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_SET_CONTROL,0,dwPresetIndex,0))
                {
                    return false;
                }
                break;
            case 1: //清除预置点
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_DEL_CONTROL,0,dwPresetIndex,0))
                {
                    return false;
                }
                break;
            case 2: //转到预置点
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_MOVE_CONTROL,0,dwPresetIndex,0))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//云台巡航操作
bool JF_Users::JF_PTZCruise(LONG lChannel, int PTZCommand, BYTE byCruiseRoute, BYTE byCruisePoint_v, WORD wInput)//登陆函数返回值，通道号，操作云台巡航的命令，巡航路线，巡航点，wInput：不同巡航命令时，值不同，预置点（最大300），速度（最大255），时间（最大40）
{
    if(HK == m_CompanyType)
    {
        switch(PTZCommand)
        {
            case 0: //开始巡航
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,RUN_SEQ,byCruiseRoute,byCruisePoint_v,wInput))
                {
                    return false;
                }
                break;
            case 1: //停止巡航
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,STOP_SEQ,byCruiseRoute,byCruisePoint_v,wInput))
                {
                    return false;
                }
                break;
            case 2: //将预置点加入巡航序列
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,FILL_PRE_SEQ,byCruiseRoute,byCruisePoint_v,wInput))
                {
                    return false;
                }
                break;
            case 3: //将预置点从巡航序列删除
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,CLE_PRE_SEQ,byCruiseRoute,byCruisePoint_v,wInput))
                {
                    return false;
                }
                break;
            case 4: //设置巡航点停顿时间
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,SET_SEQ_DWELL,byCruiseRoute,byCruisePoint_v,wInput))
                {
                    return false;
                }
                break;
            case 5: //设置巡航速度
                if(-1 == m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,SET_SEQ_SPEED,byCruiseRoute,byCruisePoint_v,wInput))
                {
                    return false;
                }
                break;
            case 6: //清除巡航路线
                if(-1==m_pv->JF_PTZCruise_v(m_lLoginID,lChannel,39,byCruiseRoute,byCruisePoint_v,wInput))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        switch(PTZCommand)
        {
            case 0: //开始巡航
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_LOOP_CONTROL,byCruiseRoute,0,76))
                {
                    return false;
                }
                break;
            case 1: //停止巡航
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_PTZ_POINT_LOOP_CONTROL,byCruiseRoute,0,96))
                {
                    return false;
                }
                break;
            case 2: //将预置点加入巡航序列
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_ADDTOLOOP,byCruiseRoute,wInput,0))
                {
                    return false;
                }
                break;
            case 3: //将预置点从巡航序列删除
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_DELFROMLOOP,byCruiseRoute,wInput,0))
                {
                    return false;
                }
                break;
            case 4: //设置巡航点停顿时间
            case 5: //设置巡航速度
                break;
            case 6: //清除巡航路线
                if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_CLOSELOOP,byCruiseRoute,0,0))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//云台轨迹操作                通道号          控制命令
bool JF_Users::JF_PTZTrack(LONG lChannel, int PTZCommand)
{
    if(HK == m_CompanyType)
    {
        switch(PTZCommand)
        {
            case 0: //开始记录轨迹
                if(-1 == m_pv->JF_PTZTrack_v(m_lLoginID,lChannel,STA_MEM_CRUISE))
                {
                    return false;
                }
                break;
            case 1: //停止记录轨迹
                if(-1 == m_pv->JF_PTZTrack_v(m_lLoginID,lChannel,STO_MEM_CRUISE))
                {
                    return false;
                }
                break;
            case 2: //开始轨迹
                if(-1 == m_pv->JF_PTZTrack_v(m_lLoginID,lChannel,RUN_CRUISE))
                {
                    return false;
                }
                break;
            case 3: //ting zhi gui ji
                if(-1 == m_pv->JF_PTZTrack_v(m_lLoginID,lChannel,STOP_CRUISE))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        switch(PTZCommand)
        {
        case 0: //开始记录轨迹
            if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_SETMODESTART,1,0,0))
            {
                return false;
            }
            break;
        case 1: //停止记录轨迹
            if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_SETMODESTOP,1,0,0))
            {
                return false;
            }
            break;
        case 2: //开始轨迹
            if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_RUNMODE,1,0,0))
            {
                return false;
            }
            break;
        case 3: //tingzhi guiji
            if(-1 == m_pk->JF_PTZControl_k(m_lLoginID,lChannel,DH_EXTPTZ_STOPMODE,1,0,0))
            {
                return false;
            }
            break;
        default:
            sdkLog->WriteLog("no this command");
            //errmsg("no this command");
            return false;
        return true;
        }
    }
    else
    {
        return false;
    }
    return true;
}

#if 1
//获取设备配置信息                 设备配置命令，      通道号，        接受数据的缓冲指针，接受数据的缓冲长度，   实际接受到的数据长度指针
bool JF_Users::JF_GetDVRConfig(DWORD dwCommand, LONG lChannel, JF_DevConfig* pOutBuf, DWORD dwOutBufSize, DWORD* pBytesRet)
{
    if(HK == m_CompanyType)
    {
        switch(dwCommand)
        {
            case 0: //视频参数
               if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_PICCFG_V40,lChannel,&pOutBuf->StructPiccfg_v,sizeof(pOutBuf->StructPiccfg_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 1: //压缩参数 主流
                if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_COMPRESSCFG_V30,lChannel,&pOutBuf->StructCompressioncfg_v,sizeof(pOutBuf->StructCompressioncfg_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 2: //录像计划
                if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_RECORDCFG_V40,lChannel,&pOutBuf->StructRecord_v,sizeof(pOutBuf->StructRecord_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 3: //运动检测
                if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_MOTION_HOLIDAY_HANDLE,lChannel,&pOutBuf->StructHoliDay_v,sizeof(pOutBuf->StructHoliDay_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 4: //报警设置
                if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_IPC_GET_AUX_ALARMCFG,lChannel,&pOutBuf->StructAuxAlarmcfg_v,sizeof(pOutBuf->StructAuxAlarmcfg_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 5: //视频丢失
                if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_VILOST_HOLIDAY_HANDLE,lChannel,&pOutBuf->StructHoliDay_v,sizeof(pOutBuf->StructHoliDay_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 6: //遮挡报警
                if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_HIDE_HOLIDAY_HANDLE,lChannel,&pOutBuf->StructHoliDay_v,sizeof(pOutBuf->StructHoliDay_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 7: // 获取设备抓图配置
                if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_JPEG_CAPTURE_CFG,lChannel,&pOutBuf->StructJpegCapture_v,sizeof(pOutBuf->StructJpegCapture_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 8: // 获取抓图计划
                if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_SCHED_CAPTURECFG,lChannel,&pOutBuf->StructSched_v,sizeof(pOutBuf->StructSched_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 9: // 获取叠加字符参数
                if(-1 == m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_SHOWSTRING_V30,lChannel,&pOutBuf->StructShowString_v,sizeof(pOutBuf->StructShowString_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 10: // 获取前端参数(扩展)
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_CCDPARAMCFG_EX,lChannel,&pOutBuf->StructCameraparamcfg_v,sizeof(pOutBuf->StructCameraparamcfg_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 11: // 获取ISP前端参数配置
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_ISP_CAMERAPARAMCFG,lChannel,&pOutBuf->StructIspCameraparamcfg_v,sizeof(pOutBuf->StructIspCameraparamcfg_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 12: // 获取辅助(PIR/无线)报警参数
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_IPC_GET_AUX_ALARMCFG,lChannel,&pOutBuf->StructAuxAlarmcfg_v,sizeof(pOutBuf->StructAuxAlarmcfg_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 13: // 获取通道视频输入图像参数
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_VIDEO_INPUT_EFFECT,lChannel,&pOutBuf->StructVideoInput_v,sizeof(pOutBuf->StructVideoInput_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 14: // 获取移动侦测假日报警处理方式
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_MOTION_HOLIDAY_HANDLE,lChannel,&pOutBuf->StructHoliDay_v,sizeof(pOutBuf->StructHoliDay_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 15: // 获取视频信号丢失假日报警处理方式
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_VILOST_HOLIDAY_HANDLE,lChannel,&pOutBuf->StructHoliDay_v,sizeof(pOutBuf->StructHoliDay_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 16: //获取遮盖假日报警处理方式
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_HIDE_HOLIDAY_HANDLE,lChannel,&pOutBuf->StructHoliDay_v,sizeof(pOutBuf->StructHoliDay_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 17: // 获取假日录像参数
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_HOLIDAY_RECORD,lChannel,&pOutBuf->StructHoliDayRecord_v,sizeof(pOutBuf->StructHoliDayRecord_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 18: // 获取通道的工作状态
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_LINK_STATUS,lChannel,&pOutBuf->StructLink_v,sizeof(pOutBuf->StructLink_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 19: // 获取通道录像状态信息
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_RECORD_CHANNEL_INFO,lChannel,&pOutBuf->StructChan_v,sizeof(pOutBuf->StructChan_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 20: // 获取WD1使能开关状态
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_WD1_CFG,lChannel,&pOutBuf->StructWd1_v,sizeof(pOutBuf->StructWd1_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 21: // 获取码流压缩性能选项
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_STREAM_CABAC,lChannel,&pOutBuf->StructStream_v,sizeof(pOutBuf->StructStream_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 22: // 获取通道对应的前端相机信息
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_ACCESS_CAMERA_INFO,lChannel,&pOutBuf->StructAccess_v,sizeof(pOutBuf->StructAccess_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 23: // 获取视频的音频输入参mm_vcUserInfoE_vcUserInfoE数
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_VIDEO_AUDIOIN_CFG,lChannel,&pOutBuf->StructVideo_v,sizeof(pOutBuf->StructVideo_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 24: // 获取音频输入参数
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_AUDIO_INPUT,lChannel,&pOutBuf->StructAudio_v,sizeof(pOutBuf->StructAudio_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 25: // 获取输出音频大小
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_AUDIOOUT_VOLUME,lChannel,&pOutBuf->StructAudioOut_v,sizeof(pOutBuf->StructAudioOut_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 26: // 获取去雾参数
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_CAMERA_DEHAZE_CFG,lChannel,&pOutBuf->StructCameraDehaze_v,sizeof(pOutBuf->StructCameraDehaze_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 27: // 获取快球低照度信息
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_LOW_LIGHTCFG,lChannel,&pOutBuf->StructLowLigh_v,sizeof(pOutBuf->StructLowLigh_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 28: // 获取快球聚焦模式信息
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_FOCUSMODECFG,lChannel,&pOutBuf->StructFocusmode_v,sizeof(pOutBuf->StructFocusmode_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 29: // 获取快球红外信息
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_INFRARECFG,lChannel,&pOutBuf->StructInfrare_v,sizeof(pOutBuf->StructInfrare_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 30: // 获取快球其他参数信息
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_AEMODECFG,lChannel,&pOutBuf->StructAemode_v,sizeof(pOutBuf->StructAemode_v),pBytesRet))
                {
                    return false;
                }
                break;
            case 31: // 获取旋转功能配置
                if(-1==m_pv->JF_GetDVRConfig_v(m_lLoginID,NET_DVR_GET_CORRIDOR_MODE,lChannel,&pOutBuf->StructCorridor_v,sizeof(pOutBuf->StructCorridor_v),pBytesRet))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        switch(dwCommand)
        {
            case 0: //视频参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_CHANNELCFG,lChannel,&pOutBuf->StructChannel_k,sizeof(pOutBuf->StructChannel_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 1: //压缩参数 主流
                break;
            case 2: //录像计划
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_RECORDCFG,lChannel,&pOutBuf->StructRecodeCfg_k,sizeof(pOutBuf->StructRecodeCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 3: //运动检测
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MOTIONALARM_CFG,lChannel,&pOutBuf->StructMotion_k,sizeof(pOutBuf->StructMotion_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 4: //报警设置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_LOCALALARM_CFG,lChannel,&pOutBuf->StructAlarmCfg_k,sizeof(pOutBuf->StructAlarmCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 5: //视频丢失
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VIDEOLOSTALARM_CFG,lChannel,&pOutBuf->StructVideLost_k,sizeof(pOutBuf->StructVideLost_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 6: //遮挡报警
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_BLINDALARM_CFG,lChannel,&pOutBuf->StructBlind_k,sizeof(pOutBuf->StructBlind_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 7: //获取设备参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DEVICECFG,lChannel,&pOutBuf->StructSystem_k,sizeof(pOutBuf->StructSystem_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 8: //获取网络参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_NETCFG,lChannel,&pOutBuf->StructNet_k,sizeof(pOutBuf->StructNet_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 9: //获取图象通道参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_CHANNELCFG,lChannel,&pOutBuf->StructChannel_k,sizeof(pOutBuf->StructChannel_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 10: //获取定时录像参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_RECORDCFG,lChannel,&pOutBuf->StructRecodeCfg_k,sizeof(pOutBuf->StructRecodeCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 11: // 获取串口参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_COMMCFG,lChannel,&pOutBuf->StructCommCfg_k,sizeof(pOutBuf->StructCommCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 12: //获取报警布防参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ALARMCFG,lChannel,&pOutBuf->StructAlarm_k,sizeof(pOutBuf->StructAlarm_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 13: //获取DVR时间
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_TIMECFG,lChannel,&pOutBuf->StructTime_k,sizeof(pOutBuf->StructTime_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 14: //获取预览参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_PREVIEWCFG,lChannel,&pOutBuf->StructPreview_k,sizeof(pOutBuf->StructPreview_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 15: //获取自动维护配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_AUTOMTCFG,lChannel,&pOutBuf->StructAutomt_k,sizeof(pOutBuf->StructAutomt_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 16: //获取本机矩阵策略配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VEDIO_MARTIX,lChannel,&pOutBuf->StructVideoMatrix_k,sizeof(pOutBuf->StructVideoMatrix_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 17: //获取多ddns服务器配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MULTI_DDNS,lChannel,&pOutBuf->StructMulti_k,sizeof(pOutBuf->StructMulti_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 18: //获取抓图相关配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SNAP_CFG,lChannel,&pOutBuf->StructSnapCfg_k,sizeof(pOutBuf->StructSnapCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 19: //获取http路径配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_WEB_URL_CFG,lChannel,&pOutBuf->StructUrl_k,sizeof(pOutBuf->StructUrl_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 20: //ftp上传配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_FTP_PROTO_CFG,lChannel,&pOutBuf->StructFtpProto_k,sizeof(pOutBuf->StructFtpProto_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 21: //区域遮挡配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VIDEO_COVER,lChannel,&pOutBuf->StructVideocover_k,sizeof(pOutBuf->StructVideocover_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 22: //传输策略配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_TRANS_STRATEGY,lChannel,&pOutBuf->StructTransfer_k,sizeof(pOutBuf->StructTransfer_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 23: //录象下载策略配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DOWNLOAD_STRATEGY,lChannel,&pOutBuf->StructDownload_k,sizeof(pOutBuf->StructDownload_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 24: //图象水印配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_WATERMAKE_CFG,lChannel,&pOutBuf->StructWatermake_k,sizeof(pOutBuf->StructWatermake_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 25: //配置无线网络信息
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_WLAN_CFG,lChannel,&pOutBuf->StructWlan_k,sizeof(pOutBuf->StructWlan_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 26: //搜索无线设备
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_WLAN_DEVICE_CFG,lChannel,&pOutBuf->StructWlanDevice_k,sizeof(pOutBuf->StructWlanDevice_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 27: //主动注册参数配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_REGISTER_CFG,lChannel,&pOutBuf->StructRegister_k,sizeof(pOutBuf->StructRegister_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 28: //摄像头属性配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_CAMERA_CFG,lChannel,&pOutBuf->StructCameracfg_k,sizeof(pOutBuf->StructCameracfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 29: //红外报警参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_INFRARED_CFG,lChannel,&pOutBuf->StructInfrared_k,sizeof(pOutBuf->StructInfrared_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 30: //邮件配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MAIL_CFG,lChannel,&pOutBuf->StructMail_k,sizeof(pOutBuf->StructMail_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 31: //DNS服务器配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DNS_CFG,lChannel,&pOutBuf->StructDns_k,sizeof(pOutBuf->StructDns_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 32: //NTP配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_NTP_CFG,lChannel,&pOutBuf->StructNtp_k,sizeof(pOutBuf->StructNtp_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 33: //Sniffer抓包配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SNIFFER_CFG,lChannel,&pOutBuf->StructSniffer_k,sizeof(pOutBuf->StructSniffer_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 34: //音频检测报警参数
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_AUDIO_DETECT_CFG,lChannel,&pOutBuf->StructAudioDetect_k,sizeof(pOutBuf->StructAudioDetect_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 35: //存储位置配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_STORAGE_STATION_CFG,lChannel,&pOutBuf->StructStorage_k,sizeof(pOutBuf->StructStorage_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 36: //夏令时配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DST_CFG,lChannel,&pOutBuf->StructDst_k,sizeof(pOutBuf->StructDst_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 37: //视频OSD叠加配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VIDEO_OSD_CFG,lChannel,&pOutBuf->StructVideoosd_k,sizeof(pOutBuf->StructVideoosd_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 38: //报警中心配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ALARM_CENTER_CFG,lChannel,&pOutBuf->StructAlarmCenter_k,sizeof(pOutBuf->StructAlarmCenter_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 39: //CDMA\GPRS网络配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_CDMAGPRS_CFG,lChannel,&pOutBuf->StructCdmagprs_k,sizeof(pOutBuf->StructCdmagprs_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 40: //IP过滤配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_IPFILTER_CFG,lChannel,&pOutBuf->StructIpifilter_k,sizeof(pOutBuf->StructIpifilter_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 41: //语音对讲编码配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_TALK_ENCODE_CFG,lChannel,&pOutBuf->StructTalk_k,sizeof(pOutBuf->StructTalk_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 42: //录像打包长度配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_RECORD_PACKET_CFG,lChannel,&pOutBuf->StructRecordPacket_k,sizeof(pOutBuf->StructRecordPacket_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 43: //短信MMS配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MMS_CFG,lChannel,&pOutBuf->StructMms_k,sizeof(pOutBuf->StructMms_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 44: //短信激活无线连接配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SMSACTIVATION_CFG,lChannel,&pOutBuf->StructSmsacTivation_k,sizeof(pOutBuf->StructSmsacTivation_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 45: //拨号激活无线连接配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DIALINACTIVATION_CFG,lChannel,&pOutBuf->StructDialinactivation_k,sizeof(pOutBuf->StructDialinactivation_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 46: //网络抓包配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SNIFFER_CFG_EX,lChannel,&pOutBuf->StructSnifferCfg_k,sizeof(pOutBuf->StructSnifferCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 47: //下载速度限制
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DOWNLOAD_RATE_CFG,lChannel,&pOutBuf->StructInt_k,sizeof(pOutBuf->StructInt_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 48: //全景切换报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_PANORAMA_SWITCH_CFG,lChannel,&pOutBuf->StructPanotama_k,sizeof(pOutBuf->StructPanotama_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 49: //失去焦点报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_LOST_FOCUS_CFG,lChannel,&pOutBuf->StructLost_k,sizeof(pOutBuf->StructLost_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 50: //报警解码器配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ALARM_DECODE_CFG,lChannel,&pOutBuf->StructAlarmdec_k,sizeof(pOutBuf->StructAlarmdec_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 51: //视频输出参数配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VIDEOOUT_CFG,lChannel,&pOutBuf->StructVideoOut_k,sizeof(pOutBuf->StructVideoOut_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 52: //预制点使能配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_POINT_CFG,lChannel,&pOutBuf->StructPoint_k,sizeof(pOutBuf->StructPoint_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 53: // Ip冲突检测报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_IP_COLLISION_CFG,lChannel,&pOutBuf->StructIp_k,sizeof(pOutBuf->StructIp_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 54: //OSD叠加使能配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_OSD_ENABLE_CFG,lChannel,&pOutBuf->StructOsd_k,sizeof(pOutBuf->StructOsd_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 55: //本地报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_LOCALALARM_CFG,lChannel,&pOutBuf->StructAlarmCfg_k,sizeof(pOutBuf->StructAlarmCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 56: //网络报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_NETALARM_CFG,lChannel,&pOutBuf->StructAlarmCfg_k,sizeof(pOutBuf->StructAlarmCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 57: //动检报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MOTIONALARM_CFG,lChannel,&pOutBuf->StructMotion_k,sizeof(pOutBuf->StructMotion_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 58: //视频丢失报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VIDEOLOSTALARM_CFG,lChannel,&pOutBuf->StructVideLost_k,sizeof(pOutBuf->StructVideLost_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 59: //视频遮挡报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_BLINDALARM_CFG,lChannel,&pOutBuf->StructBlind_k,sizeof(pOutBuf->StructBlind_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 60: //硬盘报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DISKALARM_CFG,lChannel,&pOutBuf->StructDiskAlarm_k,sizeof(pOutBuf->StructDiskAlarm_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 61: //网络中断报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_NETBROKENALARM_CFG,lChannel,&pOutBuf->StructNetBroken_k,sizeof(pOutBuf->StructNetBroken_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 62: //数字通道的前端编码器信息
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ENCODER_CFG,lChannel,&pOutBuf->StructEncoder_k,sizeof(pOutBuf->StructEncoder_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 63: //TV调节配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_TV_ADJUST_CFG,lChannel,&pOutBuf->StructTvadjust_k,sizeof(pOutBuf->StructTvadjust_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 64: //车载相关配置，北京公交使用
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ABOUT_VEHICLE_CFG,lChannel,&pOutBuf->StructAbout_k,sizeof(pOutBuf->StructAbout_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 65: //获取atm叠加支持能力信息
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ATM_OVERLAY_ABILITY,lChannel,&pOutBuf->StructAtmover_k,sizeof(pOutBuf->StructAtmover_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 66: //atm叠加配置，新atm特有
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ATM_OVERLAY_CFG,lChannel,&pOutBuf->StructATMoverlay_k,sizeof(pOutBuf->StructATMoverlay_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 67: //解码器解码轮巡配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DECODER_TOUR_CFG,lChannel,&pOutBuf->StructTour_k,sizeof(pOutBuf->StructTour_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 68: //SIP配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SIP_CFG,lChannel,&pOutBuf->StructSip_k,sizeof(pOutBuf->StructSip_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 69: //WIFI AP配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VICHILE_WIFI_AP_CFG,lChannel,&pOutBuf->StructVehicleWifi_k,sizeof(pOutBuf->StructVehicleWifi_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 70: //静态报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_STATICALARM_CFG,lChannel,&pOutBuf->StructStatic_k,sizeof(pOutBuf->StructStatic_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 71: //设备的解码策略配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DECODE_POLICY_CFG,lChannel,&pOutBuf->StructDecodePolicy_k,sizeof(pOutBuf->StructDecodePolicy_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 72: //机器相关的配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MACHINE_CFG,lChannel,&pOutBuf->StructMachine_k,sizeof(pOutBuf->StructMachine_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 73: //MAC冲突检测配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MAC_COLLISION_CFG,lChannel,&pOutBuf->StructMacColision_k,sizeof(pOutBuf->StructMacColision_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 74: //RTSP配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_RTSP_CFG,lChannel,&pOutBuf->StructRtsp_k,sizeof(pOutBuf->StructRtsp_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 75: //232串口卡号信号事件配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_485_COM_CARD_CFG,lChannel,&pOutBuf->StructLinkCfg_k,sizeof(pOutBuf->StructLinkCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 76: //485串口卡号信号事件配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MAC_COLLISION_CFG,lChannel,&pOutBuf->StructLinkCfg_k,sizeof(pOutBuf->StructLinkCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 77: //FTP上传扩展配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_FTP_PROTO_CFG_EX,lChannel,&pOutBuf->StructFtpProto_k,sizeof(pOutBuf->StructFtpProto_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 78: //SYSLOG 远程服务器配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SYSLOG_REMOTE_SERVER,lChannel,&pOutBuf->StructSysLog_k,sizeof(pOutBuf->StructSysLog_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 79: //79扩展串口属性配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_COMMCFG_EX,lChannel,&pOutBuf->StructCommCfg_k,sizeof(pOutBuf->StructCommCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 80: //卡口信息配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_NETCARD_CFG,lChannel,&pOutBuf->StructNetcard_k,sizeof(pOutBuf->StructNetcard_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 81: //视频备份格式配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_BACKUP_VIDEO_FORMAT,lChannel,&pOutBuf->StructBackUp_k,sizeof(pOutBuf->StructBackUp_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 82: //码流加密配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_STREAM_ENCRYPT_CFG,lChannel,&pOutBuf->StructStreamEncrypt_k,sizeof(pOutBuf->StructStreamEncrypt_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 83: //IP过滤配置扩展
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_IPFILTER_CFG_EX,lChannel,&pOutBuf->StructIpifilterCfg_k,sizeof(pOutBuf->StructIpifilterCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 84: //用户自定义配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_CUSTOM_CFG,lChannel,&pOutBuf->StructCustom_k,sizeof(pOutBuf->StructCustom_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 85: //搜索无线设备扩展配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_WLAN_DEVICE_CFG_EX,lChannel,&pOutBuf->StructWlanDeviceList_k,sizeof(pOutBuf->StructWlanDeviceList_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 86: //ACC断线事件配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ACC_POWEROFF_CFG,lChannel,&pOutBuf->StructAcc_k,sizeof(pOutBuf->StructAcc_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 87: // 防爆盒报警事件配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_EXPLOSION_PROOF_CFG,lChannel,&pOutBuf->StructExpLosion_k,sizeof(pOutBuf->StructExpLosion_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 88: //网络扩展配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_NETCFG_EX,lChannel,&pOutBuf->StructNetCfg_k,sizeof(pOutBuf->StructNetCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 89: //灯光控制配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_LIGHTCONTROL_CFG,lChannel,&pOutBuf->StructLight_k,sizeof(pOutBuf->StructLight_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 90: //3G流量信息配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_3GFLOW_CFG,lChannel,&pOutBuf->Struct3gflow_k,sizeof(pOutBuf->Struct3gflow_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 91: //IPv6配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_IPV6_CFG,lChannel,&pOutBuf->StructIpv6_k,sizeof(pOutBuf->StructIpv6_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 92: //Snmp配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SNMP_CFG,lChannel,&pOutBuf->StructNetSnmp_k,sizeof(pOutBuf->StructNetSnmp_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 93: //抓图开关配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SNAP_CONTROL_CFG,lChannel,&pOutBuf->StructSnapControl_k,sizeof(pOutBuf->StructSnapControl_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 94: //GPS定位模式配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_GPS_MODE_CFG,lChannel,&pOutBuf->StructGps_k,sizeof(pOutBuf->StructGps_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 95: //图片上传配置信息
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SNAP_UPLOAD_CFG,lChannel,&pOutBuf->StructSnapUpload_k,sizeof(pOutBuf->StructSnapUpload_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 96: //限速配置信息
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SPEED_LIMIT_CFG,lChannel,&pOutBuf->StructSpeed_k,sizeof(pOutBuf->StructSpeed_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 97: //iSCSI配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ISCSI_CFG,lChannel,&pOutBuf->StructIscsi_k,sizeof(pOutBuf->StructIscsi_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 98: //无线路由配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_WIRELESS_ROUTING_CFG,lChannel,&pOutBuf->StructWireless_k,sizeof(pOutBuf->StructWireless_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 99: //电子围栏配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ENCLOSURE_CFG,lChannel,&pOutBuf->StructEnclosure_k,sizeof(pOutBuf->StructEnclosure_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 100: //电子围栏版本号配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ENCLOSURE_VERSION_CFG,lChannel,&pOutBuf->StructVersion_k,sizeof(pOutBuf->StructVersion_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 101: //Raid事件配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_RIAD_EVENT_CFG,lChannel,&pOutBuf->StructRaid_k,sizeof(pOutBuf->StructRaid_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 102: //火警报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_FIRE_ALARM_CFG,lChannel,&pOutBuf->StructFire_k,sizeof(pOutBuf->StructFire_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 103: //本地名称报警配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_LOCALALARM_NAME_CFG,lChannel,&pOutBuf->StructChar_k,sizeof(pOutBuf->StructChar_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 104: //紧急存储配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_URGENCY_RECORD_CFG,lChannel,&pOutBuf->StructUrgency_k,sizeof(pOutBuf->StructUrgency_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 105: //电梯运行参数配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ELEVATOR_ATTRI_CFG,lChannel,&pOutBuf->StructElevator_k,sizeof(pOutBuf->StructElevator_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 106: //atm叠加配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ATM_OVERLAY_CFG_EX,lChannel,&pOutBuf->StructATMoverlay_k,sizeof(pOutBuf->StructATMoverlay_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 107: //MAC过滤配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MACFILTER_CFG,lChannel,&pOutBuf->StructMacFilter_k,sizeof(pOutBuf->StructMacFilter_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 108: //MAC,IP过滤
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MACIPFILTER_CFG,lChannel,&pOutBuf->StructMacipfilter_k,sizeof(pOutBuf->StructMacipfilter_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 109: //码流加密(加密计划)配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_STREAM_ENCRYPT_TIME_CFG,lChannel,&pOutBuf->StructStreamEncrypt_k,sizeof(pOutBuf->StructStreamEncrypt_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 110: //限码流配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_LIMIT_BIT_RATE_CFG,lChannel,&pOutBuf->StructLimit_k,sizeof(pOutBuf->StructLimit_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 111: //抓图相关配置扩展
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_SNAP_CFG_EX,lChannel,&pOutBuf->StructSnapCfg_k,sizeof(pOutBuf->StructSnapCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 112: //解码器url配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_DECODER_URL_CFG,lChannel,&pOutBuf->StructDecoder_k,sizeof(pOutBuf->StructDecoder_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 113: //轮巡使能配置
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_TOUR_ENABLE_CFG,lChannel,&pOutBuf->StructTourEnable_k,sizeof(pOutBuf->StructTourEnable_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 114: // wifi ap配置扩展
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VICHILE_WIFI_AP_CFG_EX,lChannel,&pOutBuf->StructVehicleWifi_k,sizeof(pOutBuf->StructVehicleWifi_k),pBytesRet))
                {
                    return false;
                }
                break;
            case 115: //数字通道的前端编码器信息扩展
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_ENCODER_CFG_EX,lChannel,&pOutBuf->StructEncoderCfg_k,sizeof(pOutBuf->StructEncoderCfg_k),pBytesRet))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//设置设备                       设置命令 ，       通道号，       输入数据的缓冲指针，输入数据的缓冲长度
bool JF_Users::JF_SetDVRConfig(DWORD dwCommand, LONG lChannel, void* pInBuf, DWORD dwInBufSize)
{
    if(HK == m_CompanyType)
    {
        switch(dwCommand)
        {
            case 0: //视频参数
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_GET_PICCFG_V40,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 1: //压缩参数 主流
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_GET_COMPRESSCFG_V30,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 2: //录像计划
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_GET_RECORDCFG_V40,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 3: //运动检测
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_GET_MOTION_HOLIDAY_HANDLE,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 4: //报警设置
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_IPC_GET_AUX_ALARMCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 5: //视频丢失
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_GET_VILOST_HOLIDAY_HANDLE,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 6: //遮挡报警
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_GET_HIDE_HOLIDAY_HANDLE,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 7: // 设置设备抓图配置
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_JPEG_CAPTURE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 8: // 设置抓图计划
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_SCHED_CAPTURECFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 9: // 设置叠加字符参数
                if(-1 == m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_SHOWSTRING_V30,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 10: // 设置前端参数(扩展)
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_CCDPARAMCFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 11: // 设置ISP前端参数配置
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_ISP_CAMERAPARAMCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 12: // 设置辅助(PIR/无线)报警参数
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_IPC_SET_AUX_ALARMCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 13: // 设置通道视频输入图像参数
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_VIDEO_INPUT_EFFECT,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 14: // 设置移动侦测假日报警处理方式
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_MOTION_HOLIDAY_HANDLE,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 15: // 设置视频信号丢失假日报警处理方式
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_VILOST_HOLIDAY_HANDLE,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 16: //设置遮盖假日报警处理方式
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_HIDE_HOLIDAY_HANDLE,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 17: // 设置假日录像参数
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_HOLIDAY_RECORD,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 18: // 设置通道录像状态信息
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_RECORD_CHANNEL_INFO,lChannel,pInBuf,dwInBufSize))
                {
                     return false;
                }
                break;
            case 19: // 设置WD1使能开关状态
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_WD1_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 20: // 设置码流压缩性能选项
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_STREAM_CABAC,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 21: // 设置通道对应的前端相机信息
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_ACCESS_CAMERA_INFO,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 22: // 设置视频的音频输入参数
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_VIDEO_AUDIOIN_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 23: // 设置音频输入参数
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_AUDIO_INPUT,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 24: // 设置输出音频大小
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_AUDIOOUT_VOLUME,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 25: // 设置去雾参数
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_CAMERA_DEHAZE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 26: //  设置快球低照度信息
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_LOW_LIGHTCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 27: // 设置快球聚焦模式信息
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_FOCUSMODECFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 28: // 设置快球红外信息
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_INFRARECFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 29: // 设置快球其他参数信息
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_AEMODECFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 30: // 设置旋转功能配置
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_CORRIDOR_MODE,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 31: // 设置信号灯同步配置参数
                if(-1==m_pv->JF_SetDVRConfig_v(m_lLoginID,NET_DVR_SET_SIGNAL_SYNC,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        switch(dwCommand)
        {
            case 0: //视频参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_CHANNELCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 1: //压缩参数 主流
                break;
            case 2: //录像计划
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_RECORDCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 3: //运动检测
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MOTIONALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 4: //报警设置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_LOCALALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 5: //视频丢失
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_VIDEOLOSTALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 6: //遮挡报警
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_BLINDALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 7: //设置设备参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DEVICECFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 8: //设置网络参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_NETCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 9: //设置图象参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_CHANNELCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 10: //设置定时录像参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_RECORDCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 11: // 设置串口参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_COMMCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 12: //设置报警布防参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ALARMCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 13: //设置DVR时间
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_TIMECFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 14: //设置预览参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_PREVIEWCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 15: //设置自动维护配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_AUTOMTCFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 16: //设置本机矩阵策略配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_VEDIO_MARTIX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 17: //设置多ddns服务器配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MULTI_DDNS,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 18: //设置抓图相关配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SNAP_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 19: //设置http路径配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_WEB_URL_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 20: //ftp上传配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_FTP_PROTO_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 21: //区域遮挡配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_VIDEO_COVER,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 22: //传输策略配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_TRANS_STRATEGY,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 23: //录象下载策略配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DOWNLOAD_STRATEGY,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 24: //图象水印配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_WATERMAKE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 25: //配置无线网络信息
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_WLAN_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 26: //无线设备配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_WLAN_DEVICE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 27: //主动注册参数配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_REGISTER_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 28: //摄像头属性配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_CAMERA_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 29: //红外报警参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_INFRARED_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 30: //邮件配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MAIL_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 31: //DNS服务器配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DNS_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 32: //NTP配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_NTP_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 33: //Sniffer抓包配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SNIFFER_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 34: //音频检测报警参数
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_AUDIO_DETECT_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 35: //存储位置配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_STORAGE_STATION_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 36: //夏令时配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DST_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 37: //视频OSD叠加配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_VIDEO_OSD_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 38: //报警中心配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ALARM_CENTER_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 39: //CDMA\GPRS网络配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_CDMAGPRS_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 40: //IP过滤配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_IPFILTER_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 41: //语音对讲编码配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_TALK_ENCODE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 42: //录像打包长度配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_RECORD_PACKET_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 43: //短信MMS配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MMS_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 44: //短信激活无线连接配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SMSACTIVATION_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 45: //拨号激活无线连接配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DIALINACTIVATION_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 46: //网络抓包配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SNIFFER_CFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 47: //下载速度限制
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DOWNLOAD_RATE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 48: //全景切换报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_PANORAMA_SWITCH_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 49: //失去焦点报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_LOST_FOCUS_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 50: //报警解码器配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ALARM_DECODE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 51: //视频输出参数配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_VIDEOOUT_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 52: //预制点使能配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_POINT_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 53: // Ip冲突检测报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_IP_COLLISION_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 54: //OSD叠加使能配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_OSD_ENABLE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 55: //本地报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_LOCALALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 56: //网络报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_NETALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 57: //动检报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MOTIONALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 58: //视频丢失报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_VIDEOLOSTALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 59: //视频遮挡报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_BLINDALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 60: //硬盘报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DISKALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 61: //网络中断报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_NETBROKENALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 62: //数字通道的前端编码器信息
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ENCODER_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 63: //TV调节配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_TV_ADJUST_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 64: //车载相关配置，北京公交使用
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ABOUT_VEHICLE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 65: //设置atm叠加支持能力信息
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ATM_OVERLAY_ABILITY,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 66: //atm叠加配置，新atm特有
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ATM_OVERLAY_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 67: //解码器解码轮巡配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DECODER_TOUR_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 68: //SIP配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SIP_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 69: //WIFI AP配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_VICHILE_WIFI_AP_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 70: //静态报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_STATICALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
            break;
            case 71: //设备的解码策略配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DECODE_POLICY_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 72: //机器相关的配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MACHINE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            /*case 73: //MAC冲突检测配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MAC_COLLISION_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 74: //RTSP配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_RTSP_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 75: //232串口卡号信号事件配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_485_COM_CARD_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 76: //485串口卡号信号事件配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MAC_COLLISION_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;*/
            case 77: //FTP上传扩展配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_FTP_PROTO_CFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 78: //SYSLOG 远程服务器配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SYSLOG_REMOTE_SERVER,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 79: //扩展串口属性配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_COMMCFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 80: //卡口信息配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_NETCARD_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 81: //视频备份格式配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_BACKUP_VIDEO_FORMAT,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 82: //码流加密配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_STREAM_ENCRYPT_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 83: //IP过滤配置扩展
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_IPFILTER_CFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 84: //用户自定义配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_CUSTOM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 85: //搜索无线设备扩展配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_WLAN_DEVICE_CFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 86: //ACC断线事件配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ACC_POWEROFF_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 87: // 防爆盒报警事件配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_EXPLOSION_PROOF_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 88: //网络扩展配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_NETCFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 89: //灯光控制配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_LIGHTCONTROL_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 90: //3G流量信息配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_3GFLOW_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 91: //IPv6配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_IPV6_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 92: //Snmp配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SNMP_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 93: //抓图开关配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SNAP_CONTROL_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 94: //GPS定位模式配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_GPS_MODE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 95: //图片上传配置信息
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SNAP_UPLOAD_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 96: //限速配置信息
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SPEED_LIMIT_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 97: //iSCSI配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ISCSI_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 98: //无线路由配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_WIRELESS_ROUTING_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 99: //电子围栏配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ENCLOSURE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 100: //电子围栏版本号配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ENCLOSURE_VERSION_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 101: //Raid事件配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_RIAD_EVENT_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 102: //火警报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_FIRE_ALARM_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 103: //本地名称报警配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_LOCALALARM_NAME_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 104: //紧急存储配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_URGENCY_RECORD_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 105: //电梯运行参数配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ELEVATOR_ATTRI_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 106: //atm叠加配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ATM_OVERLAY_CFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 107: //MAC过滤配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MACFILTER_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 108: //MAC,IP过滤
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_MACIPFILTER_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 109: //码流加密(加密计划)配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_STREAM_ENCRYPT_TIME_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 110: //限码流配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_LIMIT_BIT_RATE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 111: //抓图相关配置扩展
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_SNAP_CFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 112: //解码器url配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_DECODER_URL_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 113: //轮巡使能配置
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_TOUR_ENABLE_CFG,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 114: // wifi ap配置扩展
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_VICHILE_WIFI_AP_CFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            case 115: //数字通道的前端编码器信息扩展
                if(-1 == m_pk->JF_SetDVRConfig_k(m_lLoginID,DH_DEV_ENCODER_CFG_EX,lChannel,pInBuf,dwInBufSize))
                {
                    return false;
                }
                break;
            default:
                sdkLog->WriteLog("no this command");
                //errmsg("no this command");
                return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

#else
int JF_Users::JF_GetDVRConfig(DWORD dwCommand, LONG lChannel, void* pOutBuf, DWORD dwOutBufSize, DWORD* pBytesRet)
{
    if(HK == m_CompanyType) //海康
    {
        switch(dwCommand)
        {
            case 0: //视频参数
                break;
            case 1: //压缩参数 主流
                break;
            case 2: //录像计划
                break;
            case 3: //运动检测
                break;
            case 4: //报警设置
                break;
            case 5: //视频丢失
                break;
            case 6: //遮挡报警
                break;
            default:
                errmsg("no this command");
                return -1;
        }
    }
    else if (DH == m_CompanyType)   //大华
    {
        unsigned int ret = -1;
        switch(dwCommand)
        {
            case 0: //视频参数
                ParamVideo *pVideo = (ParamVideo*)pOutBuf;
                memset(pVideo, 0, sizeof(ParamVideo));
                DHDEV_CHANNEL_CFG *struChan = new DHDEV_CHANNEL_CFG;
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_CHANNELCFG,lChannel,struChan,sizeof(DHDEV_CHANNEL_CFG),&ret))
                {
                    return -1;
                }
                pVideo->struSTR_VideoParam.m_u16Brightness = struChan->stColorCfg[0].byBrightness;
                pVideo->struSTR_VideoParam.m_u16Hue        = struChan->stColorCfg[0].byHue;
                pVideo->struSTR_VideoParam.m_u16Contrast   = struChan->stColorCfg[0].byContrast;
                pVideo->struSTR_VideoParam.m_u16Saturation = struChan->stColorCfg[0].bySaturation;
                pVideo->struSTR_VideoParam.strctTempletTime.iStartHour = struChan->stColorCfg[0].stSect.iBeginHour;
                pVideo->struSTR_VideoParam.strctTempletTime.iStartMin  = struChan->stColorCfg[0].stSect.iBeginMin;
                pVideo->struSTR_VideoParam.strctTempletTime.iStopHour  = struChan->stColorCfg[0].stSect.iEndHour;
                pVideo->struSTR_VideoParam.strctTempletTime.iStopMin   = struChan->stColorCfg[0].stSect.iEndMin;
                pVideo->struSTR_VideoParam.strctTempletTime.iRecordMode= struChan->stColorCfg[0].stSect.bEnable;
                pVideo->_OSDTYPE_TIME.PosX  = struChan->stTimeOSD.rcRect.left;
                pVideo->_OSDTYPE_TIME.PosY  = struChan->stTimeOSD.rcRect.top;
                pVideo->_OSDTYPE_TIME.Enabled   = struChan->stTimeOSD.bShow;
                pVideo->_TEXT_ON_VIDEO.PosX = struChan->stChannelOSD.rcRect.left;
                pVideo->_TEXT_ON_VIDEO.PosY = struChan->stChannelOSD.rcRect.top;
                strcpy(pVideo->_TEXT_ON_VIDEO.Text, struChan->szChannelName);
                delete struChan;
                struChan = NULL;

                DH_DVR_VIDEOOSD_CFG *struVideoOSD = new DH_DVR_VIDEOOSD_CFG;
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VIDEO_OSD_CFG,lChannel,struVideoOSD,sizeof(DH_DVR_VIDEOOSD_CFG),&ret))
                {
                    return -1;
                }
                strcpy(pVideo->OSDText, struVideoOSD->StOSDTitleOpt[0].SzOSD_Name);
                pVideo->TextColor = struVideoOSD->StOSDTitleOpt[0].StOSD_POS.rgbaFrontground;
                if(TRUE == struVideoOSD->StOSDTitleOpt[0].StOSD_POS.bPreviewBlend)
                {
                    pVideo->OsdType = 0;
                }
                else
                {
                    pVideo->OsdType = 1;
                }
                pVideo->_OSDTYPE_TITLE.PosX = struVideoOSD->StOSDTitleOpt[0].StOSD_POS.rcRelativePos.left;
                pVideo->_OSDTYPE_TITLE.PosY = struVideoOSD->StOSDTitleOpt[0].StOSD_POS.rcRelativePos.top;
                delete struVideoOSD;
                struVideoOSD = NULL;

                DHDEV_VIDEOCOVER_CFG *struVideoCover = new DHDEV_VIDEOCOVER_CFG;
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_VIDEO_COVER,lChannel,struVideoCover,sizeof(DHDEV_VIDEOCOVER_CFG),&ret))
                {
                    return -1;
                }
                for(int i=0; i<4; ++i)
                {
                    memcpy(&pVideo->_rect[i], &struVideoCover->CoverBlock[i], sizeof(RECT));
                }
                delete struVideoCover;
                struVideoCover = NULL;
                break;
            //case 1: //压缩参数 主流
                //break;
            case 2: //录像计划
                DHDEV_RECORD_CFG *struRecord = new DHDEV_RECORD_CFG;
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_RECORDCFG,lChannel,struRecord,sizeof(DHDEV_RECORD_CFG),&ret))
                {
                    return -1;
                }
                ParamRecord *pRecord = (ParamRecord*)pOutBuf;
                //pRecord->

                delete struRecord;
                struRecord = NULL;
                break;
            case 3: //运动检测
                DH_MOTION_DETECT_CFG_EX *struMotion = new DH_MOTION_DETECT_CFG_EX;
                if(-1 == m_pk->JF_GetDVRConfig_k(m_lLoginID,DH_DEV_MOTIONALARM_CFG,lChannel,struMotion,sizeof(DH_MOTION_DETECT_CFG_EX),&ret))
                {
                    return -1;
                }
                ParamMotion *pMotion = (ParamMotion*)pOutBuf;
                pMotion->Threshold  = struMotion->wSenseLevel;
                pMotion->Enabled    = struMotion->byMotionEn;
                for(int i=0; i<22; ++i)
                {
                    for(int j=0; j<18; ++j)
                    {
                        pMotion->_posxy[i][j] = struMotion->byDetected[i][j];
                    }
                }
                
                delete struMotion;
                struMotion = NULL;
                break;
            //case 4: //报警设置
                //break;
            //case 5: //视频丢失
                //break;
            //case 6: //遮挡报警
                //break;
            default:
                errmsg("no this command");
                return -1;
        }
    }
    else    //未知厂商
    {
        return -1;
    }

    return 0;
}

int JF_Users::JF_SetDVRConfig(DWORD dwCommand, LONG lChannel, void* pInBuf, DWORD dwInBufSize)
{
    if(HK == m_CompanyType)
    {

    }
    else if (DH == m_CompanyType)
    {
        
    }
    else
    {
        return -1;
    }

    return 0;
}
#endif

#if 1   //启用线程方式
void* thread_work(void* args)
{
    pthread_detach(pthread_self());//线程分离
    JF_FindFileArgs *arg = (JF_FindFileArgs*)args;
    LONG lhFindFile = -1;

    if(HK == arg->companyType)//查找文件
    {
        lhFindFile = arg->pv->JF_FindFile_v(arg->loginID,arg->lChannel,arg->dwFileType,arg->stime,
        arg->etime,arg->cbFindFile,arg->UserData,arg->byStreamType_v,arg->lhFindFile,arg->sem);
    }
    else if (DH == arg->companyType)
    {
        lhFindFile = arg->pk->JF_FindFile_k(arg->loginID,arg->lChannel,arg->dwFileType,
        arg->stime,arg->etime,arg->cbFindFile,arg->UserData,arg->lhFindFile,arg->sem);
    }

    pthread_exit(NULL);
}
//查找文件
LLONG JF_Users::JF_FindFile(LONG lChannel, DWORD dwFileType, JF_Timer *stime, JF_Timer *etime, fFindFileLog cbFindFile,long dwUser, BYTE byStreamType_v)
{
    LLONG lhFindFile = -1;
    dwFileType = 0;
    pthread_t nthread;

    sem_t sem;
    if(-1 == sem_init(&sem,0,0))//信号量初始化，为当前进程所共享，初始值为0
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return -1;
    }

    JF_FindFileArgs *args = new JF_FindFileArgs();//结构体，JF_Define.h定义的结构体
    args->loginID    = m_lLoginID;
    args->companyType= m_CompanyType;
    args->lhFindFile = &lhFindFile;
    args->sem        = &sem;
    args->lChannel   = lChannel;
    args->dwFileType = dwFileType;
    args->stime      = stime;
    args->etime      = etime;
    args->cbFindFile = cbFindFile;
    args->byStreamType_v = byStreamType_v;
    args->UserData=dwUser;
    args->pUser      = const_cast<JF_Users*> (this);
    args->pk         = m_pk;
    args->pv         = m_pv;

    if(0 != pthread_create(&nthread,NULL,thread_work,args))//创建线程
    {
        sem_destroy(&sem);
        delete args;
        args = NULL;
        fprintf(stderr, "%s\n", strerror(errno));
        return -1;
    }
    sem_wait(&sem); //等待信号量
    sem_destroy(&sem);//销毁信号量
    if(-1 == lhFindFile)
    {
        delete args;
        args = NULL;
        return -1;
    }

    m_IDtoThread[lhFindFile] = nthread;//模板的map(LLONG,pthread_t)
    m_IDtoPoint[lhFindFile] = args;//模板的map(LLONG,void*)

    return lhFindFile;
}
//停止文件查找
bool JF_Users::JF_StopFindFile(LLONG lhFindFile)
{
    if(-1 == lhFindFile)
    {
        return false;
    }

    if(HK == m_CompanyType)
    {
        if(-1 == m_pv->JF_StopFindFile_v(lhFindFile))
        {
            sdkLog->WriteLog("stop find file fault");
            //errmsg("stop find file fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pk->JF_StopFindFile_k(lhFindFile))
        {
            sdkLog->WriteLog("stop find file fault");
            //errmsg("stop find file fault");
            return false;
        }
    }
    else
    {
        return false;
    }

    std::map<LLONG,pthread_t>::iterator it;//清理线程管理map的内容
    it = m_IDtoThread.find(lhFindFile);//查找
    if(it != m_IDtoThread.end())
    {
        if(0 == pthread_kill(it->second,0))//向指定的线程发送信号0；
        {
            pthread_cancel(it->second);//发送终止信号给线程
        }
        m_IDtoThread.erase(it);//移除
    }
        
    std::map<LLONG,void*>::iterator ip;
    ip = m_IDtoPoint.find(lhFindFile);
    if(ip != m_IDtoPoint.end())
    {
        JF_FindFileArgs *pFindArg = (JF_FindFileArgs*)ip->second;
        try{
           delete pFindArg;
           pFindArg = NULL; 
        }catch(...)
        {}
        m_IDtoPoint.erase(ip);
    }

    return true;
}
#else   //不启用线程方式
LLONG JF_Users::JF_FindFile(LONG lChannel, DWORD dwFileType, JF_Timer *stime, JF_Timer *etime, fFindFileLog cbFindFile, BYTE byStreamType_v)
{
    LLONG lhFindFile = -1;
    dwFileType = 0;

    if(HK == m_CompanyType)
    {
        lhFindFile = m_pv->JF_FindFile_v(m_lLoginID,lChannel,dwFileType,stime,etime,cbFindFile,byStreamType_v);
        if(-1 == lhFindFile)
        {
            errmsg("findfile fault");
        }
    }
    else if (DH == m_CompanyType)
    {
        lhFindFile = m_pk->JF_FindFile_k(m_lLoginID,lChannel,dwFileType,stime,etime,cbFindFile);
        if(-1 == lhFindFile)
        {
            errmsg("findfile fault");
        }
    }
    else
    {
        return -1;
    }

    return lhFindFile;
}

bool JF_Users::JF_StopFindFile(LLONG lhFindFile)
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pv->JF_StopFindFile_v(lhFindFile))
        {
            errmsg("stop find file fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pk->JF_StopFindFile_k(lhFindFile))
        {
            errmsg("stop find file fault");
            return false;
        } NET_DVR_GetFileByName
    }
    else
    {
        return false;
    }

    return true;
}
#endif
//查找日志
LLONG JF_Users::JF_FindLog(JF_FindLogArgs *findLog)
{
    if(HK == m_CompanyType)
    {
        LLONG lhFindLog = m_pv->JF_FindLog_v(m_lLoginID,findLog->lSelectMode,findLog->dwMajorType,
        findLog->dwMinorType,findLog->stime,findLog->etime,findLog->cbFindLog_v,findLog->userData);
        if(-1 == lhFindLog)
        {
            sdkLog->WriteLog("find log fault");
            //errmsg("find log fault");
        }
        return lhFindLog;
    }
    else if (DH == m_CompanyType)
    {
        char *chpLogInfo = new char[findLog->nLogNum*sizeof(DH_LOG_ITEM)]();
        int nLogInfoLen = -1;
        if(-1 == m_pk->JF_FindLog_k(m_lLoginID,findLog->logType,chpLogInfo,(findLog->nLogNum)*sizeof(DH_LOG_ITEM),&nLogInfoLen))
        {
            sdkLog->WriteLog("find log fault");
            //errmsg("find log fault");
            delete chpLogInfo;
            return -1;
        }

        for(int i=0; i<findLog->nLogNum; ++i)//转换为通用结构体
        {
            char *pNowIn = findLog->pLogBuffer+i*sizeof(JF_LogInfo);
            DH_LOG_ITEM logInfo = {0};
            memcpy(&logInfo, pNowIn, sizeof(logInfo));
            JF_LogInfo loginfo = {0};
            loginfo.StartTime.tm_year= logInfo.time.year;
            loginfo.StartTime.tm_mon = logInfo.time.month;
            loginfo.StartTime.tm_mday= logInfo.time.day;
            loginfo.StartTime.tm_hour= logInfo.time.hour;
            loginfo.StartTime.tm_min = logInfo.time.minute;
            loginfo.StartTime.tm_sec = logInfo.time.second;
            loginfo.Type = logInfo.type;
            loginfo.dwInfoLen = strlen((const char*)logInfo.context);
            strcpy(loginfo.LogContent, (const char*)logInfo.context);

            char *pNowOut = findLog->pLogBuffer+i*sizeof(JF_LogInfo);
            memcpy(pNowOut, &loginfo, sizeof(loginfo));
        }

        delete[] chpLogInfo;
        chpLogInfo = NULL;
        return 0;
    }
    else
    {
        return -1;
    }
}
//停止日志查找
bool JF_Users::JF_StopFindLog(LLONG lhFindLog)
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pv->JF_StopFindLog_v(lhFindLog))
        {
            sdkLog->WriteLog("stop find log fault");
            //errmsg("stop find log fault");
            return false;
        }
    }

    return true;
}
//v类监听回调函数，将数据转换为外部通用结构
void g_cbListen_v(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void *pUser)
{
    //pListenArgs = (JF_ListenCallbackArgs*)pUser;
    AlarmDVRInfo struDVRInfo = {0};
    struDVRInfo.pchDVRIP = pAlarmer->sDeviceIP;
    struDVRInfo.lDVRPort = pAlarmer->wLinkPort;
    memset(struDVRInfo.deviceType,0,10);
    strncpy(struDVRInfo.deviceType,"HK",2);
    pListenArgs->cbAlarmMsg(lCommand, &struDVRInfo, pAlarmInfo, dwBufLen, pListenArgs->pAlarmUserData);
    //delete pListenArgs;
    //pListenArgs = NULL;
}
//k类监听回调函数，将数据转换为外部通用结构
BOOL g_cbListen_k(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG lDVRPort, LDWORD dwUser)
{
    pListenArgs= (JF_ListenCallbackArgs*)dwUser;
    AlarmDVRInfo struDVRInfo = {0};
    struDVRInfo.pchDVRIP = pchDVRIP;
    struDVRInfo.lDVRPort = lDVRPort;
    memset(struDVRInfo.deviceType,0,10);
    strncpy(struDVRInfo.deviceType,"DH",2);
    pListenArgs->cbAlarmMsg(lCommand, &struDVRInfo, pBuf, dwBufLen, pListenArgs->pAlarmUserData);
    //delete pListenArgs;
    //pListenArgs = NULL;
}
//开启报警监听
LLONG JF_Users::JF_StartListen(int hLoginID,fAlarmMSGCallBack cbAlarmMsg, long pAlarmUserData, WORD wPort, char* strHostIP, fServiceCallBack_k cbService_k, long dwServiceUserData_k)
{
    LLONG lhListen = -1;
    if(HK == m_CompanyType)
    {
        pListenArgs = new JF_ListenCallbackArgs();
        pListenArgs->cbAlarmMsg     = cbAlarmMsg;
        pListenArgs->pAlarmUserData = (void*)pAlarmUserData;
        lhListen = m_pv->JF_StartListen_v(hLoginID,g_cbListen_v, wPort, strHostIP, pAlarmUserData);
        if(-1 == lhListen)
        {
            delete pListenArgs;
            pListenArgs = NULL;
            sdkLog->WriteLog("start alarm listen fault");
            //errmsg("start alarm listen fault");
            return -1;
        }
    }
    else if (DH == m_CompanyType)
    {
        pListenArgs = new JF_ListenCallbackArgs();
        pListenArgs->cbAlarmMsg     = cbAlarmMsg;
        pListenArgs->pAlarmUserData = (void*)pAlarmUserData;
        LDWORD dwUserData = (LDWORD)pListenArgs;
        lhListen = m_pk->JF_StartListen_k(m_lLoginID, wPort, strHostIP, g_cbListen_k, dwUserData, cbService_k, dwServiceUserData_k);
        if(-1 == lhListen)
        {
            delete pListenArgs;
            pListenArgs = NULL;
            sdkLog->WriteLog("start alarm listen fault");
            //errmsg("start alarm listen fault");
            return -1;
        }
    }
    else
    {
        return -1;
    }
    return lhListen;
}
//停止报警监听
bool JF_Users::JF_StopListen(LLONG lhListen)
{
    if(-1 == lhListen)
    {
        return false;
    }
    delete pListenArgs;
    pListenArgs = NULL;
    if(HK == m_CompanyType)
    {
        if(-1 == m_pv->JF_StopListen_v(lhListen))
        {
            sdkLog->WriteLog("stop listen fault");
            //errmsg("stop listen fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pk->JF_StopListen_k(m_lLoginID,lhListen))
        {
            sdkLog->WriteLog("stop listen fault");
            //errmsg("stop listen fault");
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}
//重起设备
bool JF_Users::JF_RebootDVR()
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pv->JF_RebootDVR_v(m_lLoginID))
        {
            sdkLog->WriteLog("reboot device fault");
            //errmsg("reboot device fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pk->JF_RebootDVR_k(m_lLoginID))
        {
            sdkLog->WriteLog("reboot device fault");
            //errmsg("reboot device fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//关闭设备
bool JF_Users::JF_ShutDownDVR()
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pv->JF_ShutDownDVR_v(m_lLoginID))
        {
            sdkLog->WriteLog("shutdown device fault");
            //errmsg("shutdown device fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pk->JF_ShutDownDVR_k(m_lLoginID))
        {
            sdkLog->WriteLog("shutdown device fault");
            //errmsg("shutdown device fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//查询设备状态
bool JF_Users::JF_QueryDVRState(JF_DVRState *state, LONG lChannel_v,DWORD dwCommand)
{
    if(HK == m_CompanyType)
    {
        unsigned int status;
        if(-1 == m_pv->JF_QueryDVRState_v(m_lLoginID,dwCommand,1,NULL,0,
        &status,&(state->device_state),sizeof(state->device_state)))
        {
            sdkLog->WriteLog("query device state fault");
            //errmsg("query device state fault");
            return false;
        }
        if(0 < status)
        {
            sdkLog->WriteLog("query device state fault");
            //errmsg("query device state fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        int retLen = -1;
        switch(dwCommand)
        {
        case 1:
             //查询普通报警信息
            if(-1 == m_pk->JF_QueryDVRState_k(m_lLoginID,DH_DEVSTATE_COMM_ALARM,(char*)&(state->client_state),sizeof(NET_CLIENT_STATE_EX),&retLen))
            {
                sdkLog->WriteLog("query device state fault");
                //errmsg("query device state fault");
                return false;
            }
            //cout<<"retLen:"<<retLen<<endl;
            break;
        case 2:
            //查询硬盘状态
            if(-1 == m_pk->JF_QueryDVRState_k(m_lLoginID,DH_DEVSTATE_DISK,(char*)&(state->disk_state),sizeof(DH_HARDDISK_STATE),&retLen))
            {
                sdkLog->WriteLog("query device state fault");
                //errmsg("query device state fault");
                return false;
            }
            //cout<<"retLen:"<<retLen<<endl;
            break;
        case 3:
            //查询遮挡报警信息
            if(-1 == m_pk->JF_QueryDVRState_k(m_lLoginID,DH_DEVSTATE_SHELTER_ALARM,(char*)state->shelterAlarm,16,&retLen))
            {
                sdkLog->WriteLog("query device state fault");
                //errmsg("query device state fault");
                return false;
            }
            //cout<<"retLen:"<<retLen<<endl;
            break;
        case 4:
            //查询录像状态
            if(-1 == m_pk->JF_QueryDVRState_k(m_lLoginID,DH_DEVSTATE_RECORDING,(char*)state->recording,16,&retLen))
            {
                sdkLog->WriteLog("query device state fault");
                //errmsg("query device state fault");
                return false;
            }
            //cout<<"retLen:"<<retLen<<endl;
            break;
        case 5:
            //查询系统的资源状态
            if(-1 == m_pk->JF_QueryDVRState_k(m_lLoginID,DH_DEVSTATE_RESOURCE,(char*)state->resource,3*sizeof(DWORD),&retLen))
            {
                sdkLog->WriteLog("query device state fault");
                //errmsg("query device state fault");
                return false;
            }
            break;
        default :
            break;

        }
    }
    else
    {
        return false;
    }
    return true;
}
//设置时间
bool JF_Users::JF_SetDVRDateTime(JF_Timer *tm)
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pv->JF_SetDVRDateTime_v(m_lLoginID,tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec))
        {
            sdkLog->WriteLog("set device time fault");
            //errmsg("set device time fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pk->JF_SetDVRDateTime_k(m_lLoginID,tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec))
        {
            sdkLog->WriteLog("set device time fault");
            //errmsg("set device time fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//按文件名下载录像
JF_DownloadFile* JF_Users::JF_DownloadFileByName(JF_RecordFileInfo &struFileInfo, char* strSaveFileName, int streamType_k, JF_K_DownloadFileArgs *args_k)
{
    JF_DownloadFile *pdl = new JF_DownloadFile(m_lLoginID,m_CompanyType,struFileInfo,strSaveFileName,streamType_k,args_k);
    if(!pdl->getFlag())//判断是否成功开始
    {
        sdkLog->WriteLog("download file fault");
        //errmsg("download file fault");
        pdl->Release();
        delete pdl;
        pdl = NULL;
    }
    return pdl;
}
//按时间段下载录像
JF_DownloadFile* JF_Users::JF_DownloadFileByTime(LONG lChannel, char* strSaveFileName, JF_Timer *stime, JF_Timer *etime, int streamType, int fileType_k, JF_K_DownloadFileArgs *args_k)
{
    string szLine,szMainKey,szSubKey;
    char strLine[256]={0};
    int nIndexPos=-1;
    int nLeftPos=-1;
    int nRightPos=-1;
    FILE* m_fp=fopen("./nSCtype.ini","r");
    if(NULL==m_fp)
     {
	nSCtype=EM_REAL_DATA_TYPE_GBPS;
    }else
     {
	while(fgets(strLine,256,m_fp))
	{
        szLine.assign(strLine);
	nLeftPos=szLine.find("\n");
	if(string::npos!=nLeftPos)
 	{
	    szLine.erase(nLeftPos,1);
        }
	nLeftPos=szLine.find("\r");
	if(string::npos!=nLeftPos)
	{
	    szLine.erase(nLeftPos,1);
	}
	nIndexPos=szLine.find("=");
	if(nIndexPos!=string::npos)
	{
             szSubKey=szLine.substr(nIndexPos+1,szLine.length()-nIndexPos-1);
	     if(IP==szSubKey)
	      {
		fgets(strLine,256,m_fp);
		nIndexPos=szLine.find("=");
		if(string::npos!=nIndexPos)
		{
                    szMainKey=szLine.substr(nIndexPos+1,szLine.length()-nIndexPos-1);
		    if(lChannel==atoi(szMainKey.c_str()))
		     {
			fgets(strLine,256,m_fp);
                        nIndexPos=szLine.find("=");
                        switch(atoi(szLine.substr(nIndexPos+1,szLine.length()-nIndexPos-1).c_str()))
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
                        }
		     }
		}
	    }else if("1.3.5.7"==szSubKey)
	     {
                nSCtype=EM_REAL_DATA_TYPE_TS;
	     }
	}
	}
     }
    JF_DownloadFile *pdl = new JF_DownloadFile(m_lLoginID,m_CompanyType,nSCtype,lChannel,strSaveFileName,stime,etime,streamType,fileType_k,args_k);
    if(!pdl->getFlag())
    {
        sdkLog->WriteLog("download file fault");
        //errmsg("download file fault");
        pdl->Release();
        delete pdl;
        pdl = NULL;
    }
    return pdl;
}
//关闭录像下载
bool JF_Users::JF_StopDownloadFile(JF_DownloadFile* &pdl)
{
    if(NULL != pdl)
    {
        if(pdl->Release())
        {
            delete pdl;
            pdl = NULL;
            return true;
        }
    }
    return false;
}
//按文件名回放录像
JF_PlayBack* JF_Users::JF_PlaybackByName(JF_RecordFileInfo &struFileInfo, HWND hWnd, fDataCallBacks cbPlaybackData, long pUserData, fPlayBackPosCallBack_k cbDownloadPos_k, long dwPosUserData_k)
{
    JF_PlayBack *ppb = new JF_PlayBack(m_lLoginID,m_CompanyType,struFileInfo,hWnd,cbPlaybackData,pUserData,cbDownloadPos_k,dwPosUserData_k);
    if(!ppb->getFlag())
    {
        sdkLog->WriteLog("playback fault");
        //errmsg("playback fault");
        ppb->Release();
        delete ppb;
        ppb = NULL;
    }
    return ppb;
}
//按时间段回放录像
JF_PlayBack* JF_Users::JF_PlaybackByTime(LONG lChannel, JF_Timer *stime, JF_Timer *etime, HWND hWnd, BYTE byStreamType_v, fDataCallBacks cbPlayData, long pUserData, fPlayBackPosCallBack_k cbDownloadPos_k, long dwPosUserData_k)
{
    string szLine,szMainKey,szSubKey;
    char strLine[256]={0};
    int nIndexPos=-1;
    int nLeftPos=-1;
    int nRightPos=-1;
    FILE* m_fp=fopen("./nSCtype.ini","r");
    if(NULL==m_fp)
     {
	nSCtype=EM_REAL_DATA_TYPE_GBPS;
    }else
     {
	while(fgets(strLine,256,m_fp))
	{
        szLine.assign(strLine);
	nLeftPos=szLine.find("\n");
	if(string::npos!=nLeftPos)
 	{
	    szLine.erase(nLeftPos,1);
        }
	nLeftPos=szLine.find("\r");
	if(string::npos!=nLeftPos)
	{
	    szLine.erase(nLeftPos,1);
	}
	nIndexPos=szLine.find("=");
	if(nIndexPos!=string::npos)
	{
             szSubKey=szLine.substr(nIndexPos+1,szLine.length()-nIndexPos-1);
	     if(IP==szSubKey)
	      {
		fgets(strLine,256,m_fp);
		nIndexPos=szLine.find("=");
		if(string::npos!=nIndexPos)
		{
                    szMainKey=szLine.substr(nIndexPos+1,szLine.length()-nIndexPos-1);
		    if(lChannel==atoi(szMainKey.c_str()))
		     {
			fgets(strLine,256,m_fp);
                        nIndexPos=szLine.find("=");
                        switch(atoi(szLine.substr(nIndexPos+1,szLine.length()-nIndexPos-1).c_str()))
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
                        }
		     }
		}
	    }else if("1.3.5.7"==szSubKey)
	     {
                nSCtype=EM_REAL_DATA_TYPE_TS;
	     }
	}
	}
     }

    pbhWnd=hWnd;
    //cout<<pbhWnd<<endl;
    JF_PlayBack *ppb = new JF_PlayBack(m_lLoginID,m_CompanyType,lChannel,nSCtype,stime,etime,hWnd,byStreamType_v,cbPlayData,pUserData,cbDownloadPos_k,dwPosUserData_k);
    if(!ppb->getFlag())
    {
        sdkLog->WriteLog("playback fault");
        //errmsg("playback fault");
        ppb->Release();
        delete ppb;
        ppb = NULL;
    }
    return ppb;
}
 //关闭录像回放
bool JF_Users::JF_StopPlayback(JF_PlayBack* &ppb)
{
    if(NULL != ppb)
    {
        if(ppb->Release())
        {
            delete ppb;
            ppb = NULL;
            return true;
        }
    }
    return false;
}
 //开启语音对讲  
JF_VoiceCom* JF_Users::JF_Voicecom(fVoiceDataCallBack cbVoiceData, long dwUser, BOOL CSMode_k)
{
    JF_VoiceCom *pvc = new JF_VoiceCom(m_lLoginID,m_CompanyType,cbVoiceData,dwUser,CSMode_k);
    if(!pvc->getFlag())
    {
        sdkLog->WriteLog("voice intercom fault");
        //errmsg("voice intercom fault");
        pvc->Release();
        delete pvc;
        pvc = NULL;
    }
    return pvc;
}
 //关闭语音对讲
bool JF_Users::JF_StopVoicecom(JF_VoiceCom* &pvc)
{
    if(NULL != pvc)
    {
        if(pvc->Release())
        {
            delete pvc;
            pvc = NULL;
            return true;
        }
    }
    return false;
}
 //开启实时预览
JF_RealPlay* JF_Users::JF_Realplay(LONG lChannel,long pUserData, unsigned int playType_k, JF_V_RealPlayArgs *args_v, HWND hWnd, fRealDataCallBackEx cbRealPlayData, fRealDataCallBack fcallback,JF_Users *User)
{
    ghWnd=hWnd;
    /*string szLine,szMainKey,szSubKey;
    char strLine[256]={0};
    int nIndexPos=-1;
    int nLeftPos=-1;
    int nRightPos=-1;
    FILE* m_fp=fopen("./nSCtype.ini","r");
    if(NULL==m_fp)
    {
        nSCtype=EM_REAL_DATA_TYPE_GBPS;
    }else
    {
	while(fgets(strLine,256,m_fp))
	{
        szLine.assign(strLine);
	nLeftPos=szLine.find("\n");
	if(string::npos!=nLeftPos)
 	{
	    szLine.erase(nLeftPos,1);
        }
	nLeftPos=szLine.find("\r");
	if(string::npos!=nLeftPos)
	{
	    szLine.erase(nLeftPos,1);
	}
	nIndexPos=szLine.find("=");
	if(nIndexPos!=string::npos)
	{
             szSubKey=szLine.substr(nIndexPos+1,szLine.length()-nIndexPos-1);
	     if(IP==szSubKey)
	      {
		fgets(strLine,256,m_fp);
                nIndexPos=szLine.find("=");
		if(string::npos!=nIndexPos)
		{
                    szMainKey=szLine.substr(nIndexPos+1,szLine.length()-nIndexPos-1);
		    if(lChannel==atoi(szMainKey.c_str()))
		     {
                        fgets(strLine,256,m_fp);
                        nIndexPos=szLine.find("=");
                        playType_k=atoi(szLine.substr(nIndexPos+1,szLine.length()-nIndexPos-1).c_str());
		     }
		}
	    }else if("1.3.5.7"==szSubKey)
	     {
		playType_k=EM_REAL_DATA_TYPE_TS;
	     }
	}
	}
     }*/

    JF_RealPlay *prp = new JF_RealPlay(m_lLoginID,m_CompanyType ,lChannel,playType_k,args_v,hWnd,cbRealPlayData,pUserData,fcallback);
    if(!prp->getFlag())
    {
        sdkLog->WriteLog("real play fault");
        //errmsg("real play fault");
        prp->Release();
        delete prp;
        prp = NULL;
    }
    return prp;
}
  //关闭实时预览
bool JF_Users::JF_StopRealplay(JF_RealPlay* &prl)
{
    if(NULL != prl)
    {
        if(prl->Release())
        {
            delete prl;
            prl = NULL;
            return true;
        }
    }
    return false;
}

//-------------------------------------------------------
//|------------> JF_DownloadFile 类 <-------------------|
//-------------------------------------------------------
JF_DownloadFile::JF_DownloadFile(LONG loginID, int companyType, JF_RecordFileInfo &struFileInfo, char* strSaveFileName, int streamType_k, JF_K_DownloadFileArgs *args_k)
                :m_lLoginID(loginID),m_CompanyType(companyType),
                m_pk(/*JF_SDK::*/m_pk),m_pv(/*JF_SDK::*/m_pv)
{
    m_pDlv = NULL;
    m_pDlk = NULL;

    if(HK == m_CompanyType)
    {
        //根据参数的不同，构造不同的文件下载类：JF_DownloadFile_v
        m_pDlv = m_pv->JF_DownloadFile_v(m_lLoginID,struFileInfo.filename,strSaveFileName);
        if(NULL != m_pDlv)
        {
            m_lhDownloadFile = m_pDlv->getHandle();
        }
    }
    else if (DH == m_CompanyType)
    {
        //根据参数的不同，构造不同的文件下载类：JF_DownloadFile_k
        m_pDlk = m_pk->JF_DownloadFile_k(m_lLoginID,*(NET_RECORDFILE_INFO*)&struFileInfo,strSaveFileName,streamType_k,
        args_k->cbFileDownloadPos,args_k->dwPosUserData,args_k->cbFileDownloadData,args_k->dwDataUser);
        if(NULL != m_pDlk)
        {
            m_lhDownloadFile = m_pDlk->getHandle();
        }
    }
}

JF_DownloadFile::JF_DownloadFile(LONG loginID, int companyType, EM_REAL_DATA_TYPE nSCtype,LONG lChannel, char* strSaveFileName, JF_Timer *stime, JF_Timer *etime, int streamType, int fileType_k, JF_K_DownloadFileArgs *args_k)
                :m_lLoginID(loginID),m_CompanyType(companyType),
                m_pk(/*JF_SDK::*/m_pk),m_pv(/*JF_SDK::*/m_pv)
{
    m_pDlv = NULL;
    m_pDlk = NULL;

    if(HK == m_CompanyType)
    {
        m_pDlv = m_pv->JF_DownloadFile_v(m_lLoginID,strSaveFileName,lChannel,stime,etime,streamType);
        if(NULL != m_pDlv)
        {
            m_lhDownloadFile = m_pDlv->getHandle();
        }
    }
    else if (DH == m_CompanyType)
    {
        if(0 == streamType) //转回K类码流标号
        {
            streamType = 1;
        }
        else if(1 == streamType)
        {
            streamType = 2;
        }
        m_pDlk = m_pk->JF_DownloadFile_k(m_lLoginID,nSCtype,lChannel,stime,etime,strSaveFileName,fileType_k,streamType,
        args_k->cbFileDownloadPos,args_k->dwPosUserData,args_k->cbFileDownloadData,args_k->dwDataUser);
        if(NULL != m_pDlk)
        {
            m_lhDownloadFile = m_pDlk->getHandle();
        }
    }
}
//停止录像下载
bool JF_DownloadFile::Release()
{
    if(HK == m_CompanyType)
    {
        if(NULL != m_pDlv)
        {
            if(m_pDlv->Release())
            {
                delete m_pDlv;
                m_pDlv = NULL;
                return true;
            }
        }
    }
    else if (DH == m_CompanyType)
    {
        if(NULL != m_pDlk)
        {
            if(m_pDlk->Release())
            {
                delete m_pDlk;
                m_pDlk = NULL;
                return true;
            }
        }
    }
    return false;
}
 //获取下载进度
JF_DownloadFile::~JF_DownloadFile()
{}

int JF_DownloadFile::JF_GetDownloadPos()
{
    int nPos = -1;
    if(HK == m_CompanyType)
    {
        nPos = m_pDlv->JF_GetDownloadPos_v();
    }
    else if (DH == m_CompanyType)
    {
        int nDownloaded = -1;
        int nTotalSize = -1;
        if(-1 != m_pDlk->JF_GetDownloadPos_k(&nDownloaded,&nTotalSize))
        {
            nPos = nDownloaded*100/nTotalSize;
        }
    }

    return nPos;
}


//-------------------------------------------------------
//|------------>   JF_PlayBack 类   <-------------------|
//-------------------------------------------------------
typedef struct {
    fDataCallBacks cbPlaybackData;
    LDWORD         dwUserData;
}JF_CallbackUserData;

int g_cbPlaybackData_k(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufferSize, LDWORD dwUser)
{
    /*if(TRUE==PLAY_SetStreamOpenMode(0,STREAME_REALTIME))
    {
           // cout<<"设置流播放模式成功"<<endl;
    }
    if(TRUE==PLAY_OpenStream(0,NULL,0,SOURCE_BUF_MIN)) //打开流
    {
       // cout<<"打开刘 成功"<<endl;
    }
    if(TRUE==PLAY_SetDecCBStream(0,3))
    {
        //cout<<"设置解码回调流类型成功"<<endl;
    }
    if(TRUE==PLAY_Play(0,pbhWnd))//播放流
    {
        //cout<<"播放刘成功"<<endl;
    }
    PLAY_InputData(0,pBuffer,dwBufferSize); //送流*/
}

JF_PlayBack::JF_PlayBack(LONG loginID, int companyType, JF_RecordFileInfo &struFileInfo, HWND hWnd, fDataCallBacks cbPlaybackData, long pUserData, fPlayBackPosCallBack_k cbPlaybackPos_k, long dwPosUserData_k)
            :m_lLoginID(loginID),m_CompanyType(companyType),
            m_pk(/*JF_SDK::*/m_pk),m_pv(/*JF_SDK::*/m_pv)
{
    m_pPbv = NULL;
    m_pPbk = NULL;
    
    if(HK == m_CompanyType)
    {
        m_pPbv = m_pv->JF_PlayBack_v(m_lLoginID,struFileInfo.filename,hWnd,(fDataCallBack_v)cbPlaybackData,pUserData);
        if(NULL != m_pPbv)
        {
            m_lhPlayBack = m_pPbv->getHandle();
        }
    }
    else if (DH == m_CompanyType)
    {
        JF_CallbackUserData *struUserData = new JF_CallbackUserData();
        struUserData->cbPlaybackData = cbPlaybackData;
        if(NULL != pUserData)
        {
            struUserData->dwUserData = *(LDWORD*)pUserData;
        }
        else
        {
            struUserData->dwUserData = 0;
        }
        LDWORD uData = (LDWORD)struUserData;
        m_pPbk = m_pk->JF_PlayBack_k(m_lLoginID,*(NET_RECORDFILE_INFO*)&struFileInfo,hWnd,cbPlaybackPos_k,dwPosUserData_k,g_cbPlaybackData_k,pUserData);
        if(NULL != m_pPbk)
        {
            m_lhPlayBack = m_pPbk->getHandle();
        }
    }
}

JF_PlayBack::JF_PlayBack(LONG loginID, int companyType, LONG lChannel,EM_REAL_DATA_TYPE nSCtype, JF_Timer *stime, JF_Timer *etime, HWND hWnd, BYTE byStreamType_v, fDataCallBacks cbPlayData, long pUserData, fPlayBackPosCallBack_k cbPlaybackPos_k, long dwPosUserData_k)
            :m_lLoginID(loginID),m_CompanyType(companyType),
            m_pk(/*JF_SDK::*/m_pk),m_pv(/*JF_SDK::*/m_pv)
{
    m_pPbv = NULL;
    m_pPbk = NULL;
    
    if(HK == m_CompanyType)
    {
        m_pPbv = m_pv->JF_PlayBack_v(m_lLoginID,lChannel,stime,etime,byStreamType_v,hWnd,(fDataCallBack_v)cbPlayData,pUserData);
        if(NULL != m_pPbv)
        {
            m_lhPlayBack = m_pPbv->getHandle();
        }
    }
    else if (DH == m_CompanyType)
    {
        JF_CallbackUserData *struUserData = new JF_CallbackUserData();
        struUserData->cbPlaybackData = cbPlayData;
        if(NULL != pUserData)
        {
            struUserData->dwUserData = *(LDWORD*)pUserData;
        }
        else
        {
            struUserData->dwUserData = 0;
        }
        m_pPbk = m_pk->JF_PlayBack_k(m_lLoginID,nSCtype,lChannel,stime,etime,hWnd,cbPlaybackPos_k,dwPosUserData_k,g_cbPlaybackData_k,(LDWORD)struUserData);
        if(NULL != m_pPbk)
        {
            m_lhPlayBack = m_pPbk->getHandle();
        }
    }
}
//停止录像回放
bool JF_PlayBack::Release()
{
    if(HK == m_CompanyType)
    {
        if(NULL != m_pPbv)
        {
            if(m_pPbv->Release())
            {
                delete m_pPbv;
                m_pPbv = NULL;
                return true;
            }
        }
    }
    else if (DH == m_CompanyType)
    {
        if(NULL != m_pPbk)
        {
            if(m_pPbk->Release())
            {
                delete m_pPbk;
                m_pPbk = NULL;
                return true;
            }
        }
    }
    return false;
}

JF_PlayBack::~JF_PlayBack()
{}
 //获取/更改播放进度
bool JF_PlayBack::JF_SGPlayBackPos(unsigned int* nPos, BOOL SGetPos_v)
{
    if(NULL == nPos)
    {
        sdkLog->WriteLog("nPos mustn't be NULL");
        //errmsg("nPos mustn't be NULL");
        return false;
    }
    
    if(HK == m_CompanyType)
    {
        if(FALSE == m_pPbv->JF_SGPlayBackPos_v(nPos,SGetPos_v))
        {
            sdkLog->WriteLog("Get/Set pos fault");
            //errmsg("Get/Set pos fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(FALSE == m_pPbk->JF_SeekPlayBack_k(*nPos))
        {
            sdkLog->WriteLog("Get/Set pos fault");
            //errmsg("Get/Set pos fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//快放
bool JF_PlayBack::JF_FastPlayBack()
{
    if(HK == m_CompanyType)
    {
        if(FALSE == m_pPbv->JF_FastPlayBack_v())
        {
            sdkLog->WriteLog("fast play fault");
            //errmsg("fast play fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(FALSE == m_pPbk->JF_FastPlayBack_k())
        {
            sdkLog->WriteLog("fast play fault");
            //errmsg("fast play fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//慢放
bool JF_PlayBack::JF_SlowPlayBack()
{
    if(HK == m_CompanyType)
    {
        if(FALSE == m_pPbv->JF_SlowPlayBack_v())
        {
            sdkLog->WriteLog("slow play fault");
            //errmsg("slow play fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(FALSE == m_pPbk->JF_SlowPlayBack_k())
        {
            sdkLog->WriteLog("slow play fault");
            //errmsg("slow play fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//暂停/继续
bool JF_PlayBack::JF_PausePlayBack()
{
    if(HK == m_CompanyType)
    {
        if(FALSE == m_pPbv->JF_PausePlayBack_v())
        {
            sdkLog->WriteLog("pause play fault");
            //errmsg("pause play fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(FALSE == m_pPbk->JF_PausePlayBack_k())
        {
            sdkLog->WriteLog("pause play fault");
            //errmsg("pause play fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//恢复正常播放
bool JF_PlayBack::JF_NormalPlayBack()
{
    if(HK == m_CompanyType)
    {
        if(FALSE == m_pPbv->JF_NormalPlayBack_v())
        {
            sdkLog->WriteLog("normal play fault");
            //errmsg("normal play fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(FALSE == m_pPbk->JF_NormalPlayBack_k())
        {
            sdkLog->WriteLog("normal play fault");
            //errmsg("normal play fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//-------------------------------------------------------
//|------------>   JF_VoiceCom 类   <-------------------|
//-------------------------------------------------------
JF_VoiceCom::JF_VoiceCom(LONG loginID, int companyType, fVoiceDataCallBack cbVoiceData, long dwUser, BOOL CSMode_k)
            :m_lLoginID(loginID),m_CompanyType(companyType),
            m_pk(/*JF_SDK::*/m_pk),m_pv(/*JF_SDK::*/m_pv)
{
    m_pVcv = NULL;
    m_pVck = NULL;

    if(HK == m_CompanyType)
    {
        m_pVcv = m_pv->JF_VoiceCom_v(m_lLoginID,cbVoiceData,dwUser,CSMode_k);
        if(NULL != m_pVcv)
        {
            m_lhVoiceCom = m_pVcv->getHandle();
        }
    }
    else if (DH == m_CompanyType)
    {
        m_pVck = m_pk->JF_VoiceCom_k(m_lLoginID,FALSE,CSMode_k,cbVoiceData,dwUser);
        if(NULL != m_pVck)
        {
            m_lhVoiceCom = m_pVck->getHandle();
        }
    }
}
//停止语音对讲
bool JF_VoiceCom::Release()
{
    if(HK == m_CompanyType)
    {
        if(NULL != m_pVcv)
        {
            if(m_pVcv->Release())
            {
                delete m_pVcv;
                m_pVcv = NULL;
                return true;
            }
        }
    }
    else if (DH == m_CompanyType)
    {
        if(NULL != m_pVck)
        {
            if(m_pVck->Release())
            {
                delete m_pVck;
                m_pVck = NULL;
                return true;
            }
        }
    }
    return false;
}

JF_VoiceCom::~JF_VoiceCom()
{}
//调节音量
bool JF_VoiceCom::JF_SetVolume(unsigned int nVolume)
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pVcv->JF_SetVolume_v(nVolume))
        {
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pVck->JF_SetVolume_k(nVolume))
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}


//-------------------------------------------------------
//|------------>   JF_RealPlay 类   <-------------------|
//-------------------------------------------------------
void g_cbRealPlayData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LONG param, LDWORD dwUser)
{
    /*if(TRUE==PLAY_SetStreamOpenMode(0,STREAME_REALTIME))
    {
           // cout<<"设置流播放模式成功"<<endl;
    }
    if(TRUE==PLAY_OpenStream(0,NULL,0,SOURCE_BUF_MIN)) //打开流
    {
       // cout<<"打开刘 成功"<<endl;
    }
    if(TRUE==PLAY_SetDecCBStream(0,3))
    {
        //cout<<"设置解码回调流类型成功"<<endl;
    }
    //cout<<ghWnd<<endl;
    if(TRUE==PLAY_Play(0,ghWnd))//播放流
    {
        //cout<<"播放刘成功"<<endl;
    }
    PLAY_InputData(0,pBuffer,dwBufSize); //送流*/
    //JF_RealPlay::InClassBcak((LLONG)lRealHandle,dwDataType,pBuffer,(LONG)dwBufSize,dwBufSize,(LDWORD)dwUser);
}
void fDataCallBack_iv(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
    JF_RealPlay::InClassBcak((LLONG)lPlayHandle,dwDataType,pBuffer,(LONG)dwBufSize,dwBufSize,(LDWORD)pUser);
}

JF_RealPlay::JF_RealPlay(LLONG loginID, int companyType, LONG lChannel, unsigned int playType_k, JF_V_RealPlayArgs *args_v, HWND hWnd, fRealDataCallBackEx cbRealPlayData, long pUserData,fRealDataCallBack fcallback)
            :m_lLoginID(loginID),m_CompanyType(companyType),
              m_pk(/*JF_SDK::*/m_pk),m_pv(/*JF_SDK::*/m_pv)
{
    m_pRpv = NULL;
    m_pRpk = NULL;
    InClassBcak=cbRealPlayData;
    if(HK == m_CompanyType)
    {
        m_pRpv = m_pv->JF_RealPlay_v(m_lLoginID,lChannel,args_v->dwPlayBufNum,args_v->dwStreamType,args_v->dwLinkMode,hWnd,(fDataCallBack_v)fDataCallBack_iv,pUserData);
        if(NULL != m_pRpv)
        {
            m_lhRealPlay = m_pRpv->getHandle();
        }
    }
    else if (DH == m_CompanyType)
    {
        JF_CallbackUserData *struUserData = new JF_CallbackUserData();
        //struUserData->cbPlaybackData = cbRealPlayData;
        if(NULL != pUserData)
        {
            struUserData->dwUserData = (LDWORD)pUserData;
        }
        else
        {
            struUserData->dwUserData = 0;
        }
        LDWORD uData = (LDWORD)struUserData->dwUserData;
        m_pRpk = m_pk->JF_RealPlay_k(m_lLoginID,lChannel,hWnd,playType_k,args_v->dwStreamType,cbRealPlayData,pUserData,fcallback);
        if(NULL != m_pRpk)
        {
            m_lhRealPlay = m_pRpk->getHandle();
        }
    }
}
//关闭实时预览
bool JF_RealPlay::Release()
{
    if(HK == m_CompanyType)
    {
        if(NULL != m_pRpv)
        {
            if(m_pRpv->Release())
            {
                delete m_pRpv;
                m_pRpv = NULL;
                return true;
            }
        }
    }
    else if (DH == m_CompanyType)
    {
        if(NULL != m_pRpk)
        {
            if(m_pRpk->Release())
            {
                delete m_pRpk;
                m_pRpk = NULL;
                return true;
            }
        }
    }
    return false;
}

JF_RealPlay::~JF_RealPlay()
{}
//设置丢帧数
bool JF_RealPlay::JF_ThrowBFrame(DWORD dwNum_v)
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pRpv->JF_ThrowBFrame_v(dwNum_v))
        {
            return false;
        }
    }
    else if (DH != m_CompanyType)
    {
        return true;
    }
    return true;
}
//设置图像效果
bool JF_RealPlay::JF_SetVideoEffect(JF_VideoEffect* struVidEff, LONG lChannel_v)
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pRpv->JF_SetVideoEffect_v(struVidEff,lChannel_v))
        {
            sdkLog->WriteLog("set video effect fault");
            //errmsg("set video effect fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pRpk->JF_SetVideoEffect_k(struVidEff))
        {
            sdkLog->WriteLog("set video effect fault");
            //errmsg("set video effect fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//获取图像效果
bool JF_RealPlay::JF_GetVideoEffect(JF_VideoEffect* struVidEff, LONG lChannel_v)
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pRpv->JF_GetVideoEffect_v(struVidEff,lChannel_v))
        {
            sdkLog->WriteLog("get video effect fault");
            //errmsg("get video effect fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pRpk->JF_GetVideoEffect_k(struVidEff))
        {
            sdkLog->WriteLog("get video effect fault");
            //errmsg("get video effect fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
 //打开声音
bool JF_RealPlay::JF_OpenSound()
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pRpv->JF_OpenSound_v())
        {
            sdkLog->WriteLog("open sound fault");
            //errmsg("open sound fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pRpk->JF_OpenSound_k())
        {
            sdkLog->WriteLog("open sound fault");
            //errmsg("open sound fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//关闭声音
bool JF_RealPlay::JF_CloseSound()
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pRpv->JF_CloseSound_v())
        {
            errmsg("close sound fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pRpk->JF_CloseSound_k())
        {
            errmsg("close sound fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//调节音量
bool JF_RealPlay::JF_SetVolume(unsigned int nVolume)
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pRpv->JF_SetVolume_v(nVolume))
        {
            errmsg("set volume fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pRpk->JF_SetVolume_k(nVolume))
        {
            errmsg("set volume fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//保存实时数据
bool JF_RealPlay::JF_SaveRealData(char *strFileName)
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pRpv->JF_SaveRealData_v(strFileName))
        {
            errmsg("save realplay data fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pRpk->JF_SaveRealData_k(strFileName))
        {
            errmsg("save realplay data fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//停止实时数据保存
bool JF_RealPlay::JF_StopSaveRealData()
{
    if(HK == m_CompanyType)
    {
        if(-1 == m_pRpv->JF_StopSaveRealData_v())
        {
            errmsg("stop save realplay data fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pRpk->JF_StopSaveRealData_k())
        {
            errmsg("stop save realplay data fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
//实时预览抓图
bool JF_RealPlay::JF_CapturePicture(char *strPictureName, unsigned int picFormat, DWORD dwBufSize_v,fSnapRev OnSnapRevMessage)
{
    if(HK == m_CompanyType)
    {
        if(0 != picFormat)
        {
            picFormat = 1;
        }

        if(-1 == m_pRpv->JF_CapturePicture_v(strPictureName,picFormat,dwBufSize_v))
        {
            errmsg("capture picture fault");
            return false;
        }
    }
    else if (DH == m_CompanyType)
    {
        if(-1 == m_pRpk->JF_CapturePicture_k(strPictureName,picFormat,OnSnapRevMessage))
        {
            errmsg("capture picture fault");
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}
