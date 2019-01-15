#ifndef _COMMON_FUNC_HPP_
#define _COMMON_FUNC_HPP_

#include "../include/jf_inc/common_func.h"

#include <string>
#include <cstring>
#include <zconf.h>
/*
#include <uuid/uuid.h>

#include <openssl/aes.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
*/
#include <QtCore/QString>
#include <uuid/uuid.h>
//#include <QtCore/QJsonValue>

typedef struct
{
    uint32_t   tm_year;
    uint32_t   tm_mon;
    uint32_t   tm_mday;
    uint32_t   tm_hour;
    uint32_t   tm_min;
    uint32_t   tm_sec;
}YMDTime;


//----------------------   工具函数  begin ---------------------------

std::string get_app_fullpath()
{
    char s_buff[1024] = { 0 };
    int len = readlink("/proc/self/exe", s_buff, 1024);

    std::string s_ret = s_buff;

    //去掉 ----> '/程序名'
    int n_reverse_index = s_ret.rfind("/");
    if (n_reverse_index == -1)//未查找成功
    {
        return "";
    }

    //截断
    s_ret.replace(n_reverse_index, s_ret.size() - n_reverse_index, "/");

    return s_ret;
}




//////int base64_encode(char *in_str, int in_len, char *out_str)
//////{
//////    BIO *b64, *bio;
//////    BUF_MEM *bptr = NULL;
//////    size_t size = 0;
//////
//////    if (in_str == NULL || out_str == NULL)
//////        return -1;
//////
//////    b64 = BIO_new(BIO_f_base64());
//////    bio = BIO_new(BIO_s_mem());
//////    bio = BIO_push(b64, bio);
//////
//////    BIO_write(bio, in_str, in_len);
//////    BIO_flush(bio);
//////
//////    BIO_get_mem_ptr(bio, &bptr);
//////    memcpy(out_str, bptr->data, bptr->length);
//////    out_str[bptr->length] = '\0';
//////    size = bptr->length;
//////
//////    BIO_free_all(bio);
//////    return size;
//////}
//////
//////int base64_decode(char *in_str, int in_len, char *out_str)
//////{
//////    BIO *b64, *bio;
//////    BUF_MEM *bptr = NULL;
//////    int counts;
//////    int size = 0;
//////
//////    if (in_str == NULL || out_str == NULL)
//////        return -1;
//////
//////    b64 = BIO_new(BIO_f_base64());
//////    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
//////
//////    bio = BIO_new_mem_buf(in_str, in_len);
//////    bio = BIO_push(b64, bio);
//////
//////    size = BIO_read(bio, out_str, in_len);
//////    out_str[size] = '\0';
//////
//////    BIO_free_all(bio);
//////    return size;
//////}
////
////std::string aesDecrypt(std::string key, std::string decryptData);
//////aes加密
////std::string aesEncrypt(std::string key, std::string encryptData)
////{
////    //获取加密key
////    AES_KEY encryptKey;
////
////    //通过uuid获取加密字符串
////    //拆出加密key
////    unsigned char userKey[AES_BLOCK_SIZE] = {0};
////    for (int i = 0, j = 0; i < key.size() && j < AES_BLOCK_SIZE; i += 4)
////    {
////        userKey[j++] = key.at(i);
////        userKey[j++] = key.at(i + 1);
////    }
////
////    int setKeyRet = AES_set_encrypt_key(userKey, 128, &encryptKey);
////    if (setKeyRet)//创建加密key失败直接返回空---0成功，-1加密key为空 -2秘钥长度不对
////    {
////        return "";
////    }
////
////    //循环加密，每次只能加密AES_BLOCK_SIZE长度的数据
////    int len = 0;
////    unsigned char outData[64] = {0};
////    std::string encryptRet = "";
////
////    while (len < encryptData.size())
////    {
////        memset(outData, 0, 64);
////        AES_encrypt((const unsigned char*)encryptData.c_str() + len, outData, &encryptKey);
////        encryptRet.append((char*)outData);
////        len += AES_BLOCK_SIZE;
////    }
////
////    //进行base64加密
////    char outBase64Data[256] = {0};
////    base64_encode((char*)encryptRet.c_str(), encryptRet.size(), outBase64Data);
////
////    encryptRet.clear();
////    encryptRet.append(outBase64Data);
////
////    return encryptRet;
////}
////
//////aes解密
////std::string aesDecrypt(std::string key, std::string decryptData)
////{
////    //先用base64解密
////    char outBase64Data[256] = {0};
////    base64_decode((char*)decryptData.c_str(), decryptData.size(), outBase64Data);
////    decryptData.clear();
////    decryptData.append(outBase64Data);
////
////    //获取加密key
////    AES_KEY encryptKey;
////
////    unsigned char userKey[AES_BLOCK_SIZE] = {0};
////    for (int i = 0, j = 0; i < key.size() && j < AES_BLOCK_SIZE; i += 4)
////    {
////        userKey[j++] = key.at(i);
////        userKey[j++] = key.at(i + 1);
////    }
////
////    int setKeyRet = AES_set_decrypt_key(userKey, 128, &encryptKey);
////    if (setKeyRet)//创建解密key失败直接返回空---0成功，-1加密key为空 -2秘钥长度不对
////    {
////        return "";
////    }
////
////    //循环加密，每次只能加密AES_BLOCK_SIZE长度的数据
////    int len = 0;
////    unsigned char outData[64] = {0};
////    std::string decryptRet = "";
////
////    while (len < decryptData.size())
////    {
////        memset(outData, 0, 64);
////        AES_decrypt((unsigned char*)decryptData.c_str() + len, outData, &encryptKey);
////        decryptRet.append((char*)outData);
////        len += AES_BLOCK_SIZE;
////    }
////
////    return decryptRet;
////}
//
////时间转换
//bool secsTimeToJFTime(time_t secsTime, void* time)
//{
//    YMDTime* jfTime  = (YMDTime*)time;
//
//    struct tm* timeYMD = localtime(&secsTime);
//    if (timeYMD == nullptr)//时间转换出错
//    {
//        return false;
//    }
//
//    jfTime->tm_year = timeYMD->tm_yday + 1900;
//    jfTime->tm_mon = timeYMD->tm_mon + 1;
//    jfTime->tm_mday = timeYMD->tm_mday ;
//    jfTime->tm_hour = timeYMD->tm_hour ;
//    jfTime->tm_min = timeYMD->tm_min ;
//    jfTime->tm_sec = timeYMD->tm_sec ;
//
//    return true;
//}
//
////获取jsonvalue里边的数据
//bool getJsonData(QJsonValue jsonValue, QString& qStr)
//{
//    qStr.clear();
//    bool ret = false;
//
//    switch (jsonValue.type())
//    {
//        case QJsonValue::Null:
//            break;
//        case QJsonValue::Bool:
//            qStr = QString::number(jsonValue.toBool());
//            ret = true;
//            break;
//        case QJsonValue::Double:
//            qStr = QString::number(jsonValue.toDouble());
//            ret = true;
//            break;
//        case QJsonValue::String:
//            qStr = jsonValue.toString();
//            ret = true;
//            break;
//        case QJsonValue::Array:
//            break;
//        case QJsonValue::Object:
//            break;
//        case QJsonValue::Undefined:
//            break;
//        default:
//            break;
//    }
//
//    return ret;
//}


//----------------------   工具函数  end  ---------------------------

//----------------------   安全函数  begin ---------------------------

void memcpy_s(void* p_des, unsigned  int n_des_len, const void* p_src, unsigned int n_src_len)
{
    //参数异常
    if (p_des == nullptr || p_src == nullptr || !n_des_len || !n_src_len)
    {
        return;
    }

    //源数据和目的数据长度校验
    if (n_src_len > n_des_len)
    {
        n_src_len = n_des_len;
    }

    //拷贝
    memcpy(p_des, p_src, n_src_len);
}

char* strcpy_s(char* p_des, unsigned int n_des_len, const char* p_src, unsigned int n_src_len)
{
    //参数异常
    if (p_des == nullptr || p_src == nullptr || !n_des_len || !n_src_len)
    {
        return p_des;
    }

    //纠正拷贝的长度
    unsigned  int n_temp_len = strlen((char*)p_src);
    if (n_src_len > n_temp_len)
    {
        n_src_len = n_temp_len;
    }

    //源数据和目的数据长度校验
    if (n_src_len > n_des_len)
    {
        n_src_len = n_des_len;
    }

    strncpy(p_des, p_src, n_src_len);

    return p_des;
}




//----------------------   end   ---------------------------

#endif
