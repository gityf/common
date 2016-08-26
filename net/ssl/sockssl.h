// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/03/2015
// Description: file of socket openssl.

#ifndef _SOCK_SSL_H_
#define _SOCK_SSL_H_
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#define RET_ERROR -1
#define RET_OK     0
#define MAX_SOCKET_ID 1024
class SockSSL
{
public:
    SockSSL();
    ~SockSSL();

    /*
    * aCertFile:  cert file.
    * aCipher:    cipher.
    */
    int SSL_Init(const std::string& aCACertFile,
                 const std::string& aPrivKeyFile,
                 const std::string& aCipher,
                 bool aIsClient);

    /*
    * SSL free.
    *
    */
    void SSL_UnInit();

    /*
    * aBuffer:    buffer for reading or writing.
    * aLen:       buffer length for reading or writing.
    */
    int SSL_Read(int aSockId, char *aBuffer, int aLen);
    int SSL_Write(int aSockId, char *aBuffer, int aLen);

    /*
    *  aIsClient:      false:server, true:client.
    *
    */
    int SSL_ConnHandShake(int aSockId, bool aIsClient);

    /*
    *  close SSL socket id.
    *
    */
    int SSL_SockClose(int aSockId);
private:
    /*
    *  show CA information.
    *
    */
    void ShowCerts(SSL * ssl);
    SSL_CTX* iSsl_ctx;
    SSL *iSsl[MAX_SOCKET_ID];
};
#endif // _SOCK_SSL_H_
