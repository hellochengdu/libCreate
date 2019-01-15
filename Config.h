//
// Created by chenbaiyi on 18-5-7.
//

#ifndef JF_MDB_JFCCONFIG_H
#define JF_MDB_JFCCONFIG_H


#include <QtCore/QString>
#include <QtCore/QStringList>

class QSettings;


class CConfig
{
public:

    explicit  CConfig();
    ~CConfig();

public:
    bool InitMoudle(const char* s_conf_path);
    bool SetGroupValue(QString group_name);//设置组名称
    bool SetValue(QString group_name, QString field, QString value);//设置组内部单个字段的名称
    bool SetValue(QString group_name, QStringList field, QStringList value);//设置组内多个字段名称
    QString GetValue(QString group_name, QString field);//获取组内部单个字段的名称
    QStringList GetValue(QString group_name);//获取组内多个字段名称
    QString GetKey(QString group_name, QString value);//通过某组内部的值，获取其key，注：该配置文件组内值唯一
    void RemoveKey(QString group_name);
    QStringList GetAllKeys(QString group_name);

private:
    QSettings* m_config_handle;//实际操作句柄
};


#endif //JF_MDB_JFCCONFIG_H
