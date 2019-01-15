//
// Created by chenbaiyi on 18-5-7.
//


#include <QtCore/QSettings>
#include <QtCore/QTextCodec>
#include <zconf.h>
#include "../include/jf_inc/Config.h"


//************************************
// Name:
// Des:    设置组名称
// Arg:

// Ret:
// Date:   	2017/01/10
// Author: 	ChenRS
//************************************
bool CConfig::SetGroupValue(QString group_name)
{
    if (m_config_handle == nullptr)
    {
        return false;
    }

    m_config_handle->beginGroup(group_name);
    m_config_handle->endGroup();

    return true;
}

//************************************
// Name:
// Des:    设置组内部单个字段的名称
// Arg:

// Ret:
// Date:   	2017/01/10
// Author: 	ChenRS
//************************************
bool CConfig::SetValue(QString group_name, QString field, QString value)
{
    if (m_config_handle == nullptr)
    {
        return false;
    }

    m_config_handle->beginGroup(group_name);
    m_config_handle->setValue(field, value);
    m_config_handle->endGroup();


    return true;
}

//************************************
// Name:
// Des:    设置组内多个字段名称
// Arg:

// Ret:
// Date:   	2017/01/10
// Author: 	ChenRS
//************************************
bool CConfig::SetValue(QString group_name, QStringList field, QStringList value)
{
    if (m_config_handle == nullptr)
    {
        return false;
    }

    m_config_handle->beginGroup(group_name);
    for (int i = 0; i < field.size(); i++)
    {
        m_config_handle->setValue(field.at(i), value.at(i));
    }
    m_config_handle->endGroup();

    return true;
}

//************************************
// Name:
// Des:    获取组内部单个字段的名称
// Arg:

// Ret:
// Date:   	2017/01/10
// Author: 	ChenRS
//************************************
QString CConfig::GetValue(QString group_name, QString field)
{
    if (m_config_handle == nullptr)
    {
        return "";
    }

    return m_config_handle->value(group_name + '/' + field).toString();
}

//************************************
// Name:
// Des:    获取组内多个字段名称
// Arg:

// Ret:
// Date:   	2017/01/10
// Author: 	ChenRS
//************************************
QStringList CConfig::GetValue(QString group_name)
{
    QStringList ret;

    if (m_config_handle == nullptr)
    {
        return ret;
    }

    m_config_handle->beginGroup(group_name);
    QStringList keys = m_config_handle->childKeys();
    for (int i = 0; i < keys.size(); i++)
    {
        ret.append(m_config_handle->value(keys.at(i)).toString());
    }
    m_config_handle->endGroup();

    return ret;
}

//************************************
// Name:
// Des:    通过某组内部的值，获取其key，注：该配置文件组内值唯一
// Arg:

// Ret:
// Date:   	2017/01/13
// Author: 	ChenRS
//************************************
QString CConfig::GetKey(QString group_name, QString value)
{
    if (m_config_handle == nullptr)
    {
        return "";
    }

    m_config_handle->beginGroup(group_name);
    QStringList keys = m_config_handle->childKeys();
    QString ret = "";
    bool b_flag = 0;
    for (int i = 0; i < keys.size(); i++)
    {
        ret = m_config_handle->value(keys.at(i)).toString();
        //if (ret.indexOf(value) != -1)
        if (ret.compare(value) == 0)
        {
            ret = keys.at(i);
            b_flag = true;
            break;
        }
    }
    m_config_handle->endGroup();

    if (!b_flag)
        return "";
    return ret;
}

//************************************
// Name:
// Des:    清除某一个键
// Arg:

// Ret:
// Date:   	2017/01/18
// Author: 	ChenRS
//************************************
void CConfig::RemoveKey(QString group_name)
{
    if (m_config_handle == nullptr)
    {
        return;
    }

    m_config_handle->remove(group_name);
}

//************************************
// Name:
// Des:    后去某一组内的所有key值
// Arg:

// Ret:
// Date:   	2017/02/28
// Author: 	ChenRS
//************************************
QStringList CConfig::GetAllKeys(QString group_name)
{
    QStringList ret;
    if (m_config_handle == nullptr)
    {
        return ret;
    }

    m_config_handle->beginGroup(group_name);
    ret = m_config_handle->allKeys();
    m_config_handle->endGroup();

    return ret;
}

CConfig::CConfig()
{
    m_config_handle = nullptr;
}

CConfig::~CConfig()
{
    if (m_config_handle != nullptr)
    {
        delete m_config_handle;
        m_config_handle = nullptr;
    }
}

bool CConfig::InitMoudle(const char *s_conf_path)
{
    //检查文件路径是否正常
    if (s_conf_path == nullptr || (access(s_conf_path, F_OK) == -1))
    {
        return false;
    }

    m_config_handle = new QSettings(s_conf_path, QSettings::IniFormat);
    m_config_handle->setIniCodec(QTextCodec::codecForName("GBK"));;

    return true;
}

































