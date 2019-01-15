//
// Created by chenbaiyi on 18-4-25.
//

#ifndef TEST_JFCLOG_H
#define TEST_JFCLOG_H

#include "GCCondBindMutex.h"
#include <QtCore/QString>
#include <log4cplus/helpers/timehelper.h>

class CLog
{
public:
    const uint32_t LOG_BUF_LEN;
    enum LOGLEVEL//输出级别
    {
        OFF_LOG_LEVEL,//0     0

        FATAL_LEVEL,//1
        ERROR_LEVEL,//2
        WARN_LEVEL,//3        1
        INFO_LEVEL,//4

        DEBUG_LEVEL,//5       2
        TRACE_LEVEL,//6

        ALL_LEVEL//7          3
    };

    enum LOGMODE //输出模式
    {
        UNKNOW_MODE = 1,//未知模式
        SCRIPT_MODE = 2,//脚本配置
        CONSOLE_MODE = 4,//控制台模式
        FILE_MODE = 8,//文件模式---默认dailyrollingfileappender
        SOCKET_MODE = 16//重定向远程服务器
    };

    //30（黑色）、31（红色）、32（绿色）、 33（黄色）、34（蓝色）、35（洋红）、36（青色）、37（白色）
    enum LOGCOLOR//颜色
    {
        BLACK,//黑色 0
        RED,//红色  1
        GREEN,//绿色  2
        YELLOW,//黄色  3
        BLUE,//蓝色  4
        WHITE,//白色 5
        PURPLE,//紫色 6
        BOTTLE_GREEN,//深绿 7
    };

    CLog();
    ~CLog();

public://供外部调用的具体函数
    bool InitModule(const char* s_moudle_name, unsigned int max_file_num = 30);    //通过输出模式初始化
    bool WriteLog(const char* log_buffer, LOGLEVEL log_level = INFO_LEVEL, LOGCOLOR log_color = GREEN);//写入日志数据
    bool WriteLog(QString qs_log_data, LOGLEVEL log_level = INFO_LEVEL, LOGCOLOR log_color = GREEN);//写入日志数据

    static CLog* getGlobalLogHandle();

private:
    bool Write(const char* log_buffer, uint32_t buff_size, LOGLEVEL& log_level);//实际写入的函数
    bool checkFile();
    void backupFile();

    //模式标识
    unsigned int m_log_mode;
    std::string m_logMoudleName;
    //日志实际写入操作句柄
    void* m_log_logger;
    //日志输出等级
    uint8_t m_logLevel;
    //
    char* m_logBuff;

    //日志文件路径
    std::string m_logFilePath;
    //日志文件名称
    std::string m_logFileName;

    //文件描述符
    static int m_writeFD;
    //单个日志文件最大值
    static uint32_t m_fileMaxSize;
    //日志文件备份序号最大值
    static uint8_t m_fileBackupIndex;
    //日志文件当前备份序号
    static uint8_t m_fileCurrBackupIndex;
    //日志文件已写入大小
    static uint32_t m_logFileWritedSize;

    //多线程写入全局句柄
    static GCCondBindMutex m_locker;
    //用于全局日志输出
    static CLog* gLogHandle;
};

//export interface
extern "C"
{
    CLog *getLogInstance();
    void releaseLogInstance(CLog *);
}


#endif //TEST_JFCLOG_H
