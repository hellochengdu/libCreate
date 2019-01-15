//
// Created by chenbaiyi on 18-4-25.
//

#include "../include/jf_inc/common_func.h"

#include "../include/jf_inc/CLog.h"
#include "../include/jf_inc/Config.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <uuid/uuid.h>


//log4cplus模块引用的头文件
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/helpers/stringhelper.h>

//qt模块
#include <QtCore/QString>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>

//创建uuid
std::string createUUID()
{
    uuid_t id;
    char temp[40] = {0};

    uuid_generate(id);
    uuid_unparse(id, temp);

    std::string ret = temp;

    //去掉-
    for (int i = 0; i < ret.size();)
    {
        if (ret.at(i) == '-')
        {
            ret.erase(i, 1);
            i = 0;
        }
        else
        {
            i++;
        }
    }

    return ret;
}

CLog* CLog::gLogHandle = nullptr;

CLog* CLog::getGlobalLogHandle()
{
    if (gLogHandle == nullptr)
    {
        gLogHandle = new CLog;

        //初始化全局日志模块
        if (gLogHandle->InitModule("GlobalHlr"))
        {
            gLogHandle->WriteLog("GlobalLogMoudler init moudle successful");
            //printf("GlobalLogMoudler init successful\n");
        }
        else
        {
            printf("GlobalLogMoudler init failed\n");
            delete gLogHandle;
            gLogHandle = nullptr;
        }
    }

    return gLogHandle;
}

//单个日志文件最大值
uint32_t CLog::m_fileMaxSize = 10 * 1024 * 1024;
//日志文件备份序号最大值
uint8_t CLog::m_fileBackupIndex = 10;
//日志文件当前备份序号
uint8_t CLog::m_fileCurrBackupIndex = 0;
//日志文件已写入大小
uint32_t CLog::m_logFileWritedSize = 0;
//文件描述符
int CLog::m_writeFD = -1;
//全局锁
GCCondBindMutex CLog::m_locker;

//构造函数
CLog::CLog()
    : LOG_BUF_LEN(1024)
{
    //zero
    m_log_logger = nullptr;
    m_logBuff =  new char[LOG_BUF_LEN];

    //初始化日志输出模式
    //检查是否存在jflog.properties文件存在---即通过脚本配置日志输出模式
    m_log_mode = UNKNOW_MODE;

    std::string s_fullpath = get_app_fullpath();
    std::string s_path_properties = s_fullpath + std::string("log.properties");
    std::string s_path_socket = s_fullpath + std::string("log.socket");
    std::string s_path_console = s_fullpath + std::string("log.console");
    std::string s_path_file = s_fullpath + std::string("log.file");

    if (access(s_path_properties.c_str(), F_OK) != -1)
    {
        m_log_mode = LOGMODE::SCRIPT_MODE;//脚本模式
    }
    else if (access(s_path_socket.c_str(), F_OK) != -1)//检查是否存在jflog.socket文件
    {
        m_log_mode = LOGMODE::SOCKET_MODE;
    }
    else
    {
        //控制台输出和文件输出两者可以并存
        if (access(s_path_console.c_str(), F_OK) != -1)//检查是否存在jflog.console文件
        {
            m_log_mode = LOGMODE::CONSOLE_MODE;
        }

        if (access(s_path_file.c_str(), F_OK) != -1)//检查是否存在jflog.file文件
        {
            //m_log_mode = m_log_mode | LOGMODE::FILE_MODE;
        }
    }
}


CLog::~CLog()
{
    //delete
    if (m_log_logger != nullptr)
    {
        delete reinterpret_cast<log4cplus::Logger*>(m_log_logger);
        m_log_logger = nullptr;
    }

    //free mem
    if (m_logBuff)
    {
        delete[] m_logBuff;
        m_logBuff = nullptr;
    }
}

// purpose: 初始化该模块
// param:
    //s_moudle_name: 模块名称
// ret:
    //true表示初始化成功，false表示初始化失败
bool CLog::InitModule(const char* s_moudle_name, unsigned int max_file_num)
{
    if (m_log_logger != nullptr)
    {
        return false;
    }

    m_logMoudleName = s_moudle_name;
    bool b_ret = false;

    //未知模式
    if (!s_moudle_name || m_log_mode == LOGMODE::UNKNOW_MODE)
    {
        return b_ret;//初始化失败
    }

    //脚本模式
    if (m_log_mode == LOGMODE::SCRIPT_MODE)
    {
//        try
//        {
//            //获取根节点
//           // m_log_logger = new log4cplus::Logger(log4cplus::Logger::getRoot());
//            //加载脚本配置
//
//            std::string s_path_properties = get_app_fullpath();
//            s_path_properties.append("log.properties");
//            log4cplus::PropertyConfigurator::doConfigure(s_path_properties.c_str());
//        }
//        catch (...)
//        {
//            printf("LOG InitModule Failed\n");
//        }
    }
    else if (m_log_mode == LOGMODE::SOCKET_MODE)//远程重定向
    {

    }
    else
    {
        try
        {
            std::string moudleName = s_moudle_name;
            moudleName.append(createUUID());
            //通过logger.console获取句柄
            m_log_logger = new log4cplus::Logger(log4cplus::Logger::getInstance(moudleName.data()));

            //输出格式
            std::string pattern = "%D.%d{%q}  [";
            pattern.append(s_moudle_name);
            pattern.append("]%X  %m%n");
            //pattern.append("] %d{%Y/%m/%d %D %H:%M:%S.%q}  %X  [%m]%n");
            //pattern.append("] %D.%d{%q}  %X  [%m]%n");

            std::string currPath = get_app_fullpath();
            currPath.append("config.ini");
            QString tempString = "";

            //读取配置文件对象
            CConfig config;
            if (!config.InitMoudle(currPath.c_str()))
            {
                return false;
            }

            if ((m_log_mode & LOGMODE::CONSOLE_MODE) == LOGMODE::CONSOLE_MODE)
            {
                //输出格式
                std::unique_ptr<log4cplus::Layout> layout(new log4cplus::PatternLayout(pattern));

                log4cplus::helpers::SharedObjectPtr<log4cplus::Appender> appender(new log4cplus::ConsoleAppender());
                appender->setName(LOG4CPLUS_TEXT("appener.console"));//设置名称

                //设置输出格式
                appender->setLayout(std::move(layout));

                //将构造的适配添加到logger句柄中---此处可添加多个
                ((log4cplus::Logger*)m_log_logger)->addAppender(appender);//

                //设置终端前景色和背景色
                //printf("\033[1;32;40m");
                printf("\033[;32m");
            }

            tempString = config.GetValue("LOG", "level");
            int logLevel = tempString.toUInt();


            //如果等级为３则判断是否有私密文件存在　不存在默认为１

            //异常日志等级
            if (logLevel > 3 || logLevel < 0)
            {
                logLevel = 1;//默认为1 只输出info及其以下
            }
            m_logLevel = logLevel;

            //查看日志等级
            switch (logLevel)
            {
                case 0:
                    ((log4cplus::Logger*)m_log_logger)->setLogLevel(log4cplus::OFF_LOG_LEVEL);
                    break;
                case 1:
                    ((log4cplus::Logger*)m_log_logger)->setLogLevel(log4cplus::INFO_LOG_LEVEL);
                    break;
                case 2:
                    ((log4cplus::Logger*)m_log_logger)->setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
                    break;
                case 3:
                    ((log4cplus::Logger*)m_log_logger)->setLogLevel(log4cplus::ALL_LOG_LEVEL);
                    break;
            }
        }
        catch (...) {
            printf("LOG InitModule Failed\n");
        }
    }

    b_ret = true;
    return b_ret;
}


// purpose: 写入日志数据
// param:
    //log_buffer: 写入数据buffer
    //log_level: 日志输出级别---初始化的时候默认拥有最高级别
// ret:
    //true表示写入成功，false表示写入失败
bool CLog::WriteLog(const char* log_buffer, LOGLEVEL log_level, LOGCOLOR log_color)
{
    if (!CLog::m_locker.TryLock())
    {
        return false;
    }

    //判断是否要修改字体颜色
    if (log_color != LOGCOLOR::GREEN)
    {
        //30（黑色）、31（红色）、32（绿色）、 33（黄色）、34（蓝色）、35（洋红）、36（青色）、37（白色）
        std::string colorFmt = "\033[;";
        switch ((int)log_color)
        {
            case BLACK:
                colorFmt.append("30m");
                break;
            case RED:
                colorFmt.append("31m");
                break;
            case GREEN:
                colorFmt.append("32m");
                break;
            case YELLOW:
                colorFmt.append("33m");
                break;
            case BLUE:
                colorFmt.append("34m");
                break;
            case WHITE:
                colorFmt.append("37m");
                break;
            case PURPLE:
                colorFmt.append("35m");
                break;
            case BOTTLE_GREEN:
                colorFmt.append("36m");
                break;
        }
        //printf("\033[;31m");

        //设置终端前景色和背景色
        printf(colorFmt.c_str());
    }

    //未进行初始化
    if (log_buffer == nullptr || m_log_logger == nullptr)
    {
        CLog::m_locker.Unlock();
        return false;
    }

    bool ret = Write(log_buffer, strlen(log_buffer), log_level);

    //printf("\033[1;32;42m");
    if (log_color != LOGCOLOR::GREEN)
    {
        printf("\033[;32m");
    }

    CLog::m_locker.Unlock();
    return ret;
}


// purpose: 写入日志数据
// param:
    //qs_log_data: 写入数据,针对于整形
    //log_level: 日志输出级别---初始化的时候默认拥有最高级别
// ret:
    //true表示写入成功，false表示写入失败
bool CLog::WriteLog(QString qs_log_data, CLog::LOGLEVEL log_level, LOGCOLOR log_color)
{
    if (!CLog::m_locker.TryLock()) {
        return false;
    }

    //未进行初始化
    if (m_log_logger == nullptr) {
        CLog::m_locker.Unlock();
        return false;
    }

    //判断是否要修改字体颜色
    if (log_color != LOGCOLOR::GREEN)
    {
        //30（黑色）、31（红色）、32（绿色）、 33（黄色）、34（蓝色）、35（洋红）、36（青色）、37（白色）
        std::string colorFmt = "\033[;";
        switch ((int) log_color) {
            case BLACK:
                colorFmt.append("30m");
                break;
            case RED:
                colorFmt.append("31m");
                break;
            case GREEN:
                colorFmt.append("32m");
                break;
            case YELLOW:
                colorFmt.append("33m");
                break;
            case BLUE:
                colorFmt.append("34m");
                break;
            case WHITE:
                colorFmt.append("37m");
                break;
            case PURPLE:
                colorFmt.append("35m");
                break;
            case BOTTLE_GREEN:
                colorFmt.append("36m");
                break;
        }

        //设置终端前景色和背景色
        //printf("\033[;31m");
        //colorFmt.append("m");
        printf(colorFmt.c_str());
    }

    QByteArray byteData = qs_log_data.toLocal8Bit();
    bool ret = Write(byteData.data(), byteData.size(), log_level);

    //printf("\033[1;32;42m");
    if (log_color != LOGCOLOR::GREEN)
    {
        printf("\033[;32m");
    }

    CLog::m_locker.Unlock();
    return ret;
}

//写入日志泛接口
bool CLog::Write(const char* log_buffer, uint32_t buff_size, LOGLEVEL& log_level)
{
    if (log_buffer == nullptr)
    {
        return false;
    }

    bool b_ret = false;
    //根据级别输出日志
    switch ((int)log_level)
    {
        case INFO_LEVEL:
            LOG4CPLUS_INFO(*reinterpret_cast<log4cplus::Logger*>(m_log_logger), log_buffer);
            break;
        case DEBUG_LEVEL:
            LOG4CPLUS_DEBUG(*reinterpret_cast<log4cplus::Logger*>(m_log_logger), log_buffer);
            break;
        case TRACE_LEVEL:
            LOG4CPLUS_TRACE(*reinterpret_cast<log4cplus::Logger*>(m_log_logger), log_buffer);
            break;
        case ERROR_LEVEL:
            LOG4CPLUS_ERROR(*reinterpret_cast<log4cplus::Logger*>(m_log_logger), log_buffer);
            break;
        case WARN_LEVEL:
            LOG4CPLUS_WARN(*reinterpret_cast<log4cplus::Logger*>(m_log_logger), log_buffer);
            break;
        case FATAL_LEVEL:
            LOG4CPLUS_FATAL(*reinterpret_cast<log4cplus::Logger*>(m_log_logger), log_buffer);
            break;
        default:
            break;
    }

    //是否需要输出到文件
//    if ((m_log_mode & LOGMODE::FILE_MODE) == LOGMODE::FILE_MODE)
//    {
//        //先检查文件
//        if (checkFile())
//        {
//            //写数据
//            memset(m_logBuff, 0, 256);
//
//            if (buff_size < LOG_BUF_LEN)
//            {
//                struct timeval tv;
//                struct timezone tz;
//                struct tm *t;
//                gettimeofday(&tv, &tz);
//                t = localtime(&tv.tv_sec);
//                //printf("time_now:%d-%d-%d %d:%d:%d.%ld\n", 1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday, t->tm_hour,
//                  //     t->tm_min, t->tm_sec, tv.tv_usec);
//                sprintf(m_logBuff, "%d-%d-%d %d:%d:%d.%ld  [%s]  %s\r\n",
//                        1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec,
//                        m_logMoudleName.data(), log_buffer);
//
//                m_logFileWritedSize += write(CLog::m_writeFD, m_logBuff, strlen(m_logBuff));
//            }
//        }
//        else
//        {
//            printf("check file failed---->");
//            printf(m_logMoudleName.data());
//            printf("\n");
//        }
//
//    }

    //判断日志等级

    b_ret = true;
    return b_ret;
}

bool CLog::checkFile()
{
    //文件参数是否正常
    if (!m_logFilePath.size() || !m_logFileName.size())
    {
        return false;
    }

    //最新日志文件全路径
    std::stringstream ssLogPullPath;
    ssLogPullPath << m_logFilePath << "/" << m_logFileName << ".log";

    //文件是否已经打开
    if (CLog::m_writeFD >= 0)
    {
        //打开则判断该文件是否已经超过大小
        if (m_logFileWritedSize >= m_fileMaxSize)
        {
            //先关闭文件
            close(CLog::m_writeFD);
            CLog::m_writeFD = -1;

            //向后备份
            backupFile();
            m_logFileWritedSize = 0;

            //不存在则创建
            CLog::m_writeFD = open(ssLogPullPath.str().data(), O_RDWR | O_CREAT, S_IRWXU);
            if (CLog::m_writeFD < 0)
            {
                return false;//打开文件失败
            }
        }
    }
    else
    {
        //最新文件是否存在
        if (access(ssLogPullPath.str().data(), F_OK) != -1)//存在则检查文件大小是否超过最大值
        {
            //检查最新文件大小是否已经超过最大值
            struct stat statBuf;
            if (stat(ssLogPullPath.str().data(),&statBuf)==0) {
                if (statBuf.st_size >= m_fileMaxSize)//超过最大值则向后备份
                {
                    //备份文件
                    backupFile();
                    m_logFileWritedSize = 0;

                    //备份完成则打开文件
                    CLog::m_writeFD = open(ssLogPullPath.str().data(), O_RDWR | O_CREAT, S_IRWXU);
                } else {
                    CLog::m_writeFD = open(ssLogPullPath.str().data(), O_RDWR | O_APPEND);
                }
            }
            else
            {
                printf("errno: %s\n",strerror(errno));
                return false;
            }
        }
        else
        {
            //不存在则创建
            CLog::m_writeFD = open(ssLogPullPath.str().data(), O_RDWR | O_CREAT, S_IRWXU);
        }

        //文件描述符不正常
        if (CLog::m_writeFD < 0)
        {
            return false;//打开文件失败
        }
    }

    return true;
}

void CLog::backupFile()
{
    std::stringstream ssLogPullPath;
    uint8_t index = 0;

    for (int i =  m_fileCurrBackupIndex + 1; i < m_fileBackupIndex; i++)
    {
        ssLogPullPath.str("");
        ssLogPullPath << m_logFilePath << "/" << m_logFileName << i << ".log";
        if (access(ssLogPullPath.str().c_str(), F_OK) != -1)//该序号已经被占用
        {
            continue;
        }

        index = i;
        break;
    }

    //没找到则判断当前文件备份序号　－－如果是０则从１开始，否则＋＋
    if (index == 0)//没找到
    {
        if (m_fileCurrBackupIndex == 0 || (m_fileCurrBackupIndex + 1 >= m_fileBackupIndex))
        {
            m_fileCurrBackupIndex = 1;
        }
        else
        {
            ++m_fileCurrBackupIndex;
        }

    }
    else
    {
        m_fileCurrBackupIndex = index;
    }

    //将当前文件向后备份
    std::stringstream shellCMD;
    shellCMD <<"mv " << m_logFilePath << "/" << m_logFileName << ".log" << " "  <<
             m_logFilePath << "/" << m_logFileName << (int)m_fileCurrBackupIndex << ".log";

    //执行shell命令
    system(shellCMD.str().data());
}

