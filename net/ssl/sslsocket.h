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
#include <map>
#define RET_ERROR -1
#define RET_OK     0
enum SSLProtocol {
    SSLTLS = 0,	// Supports SSLv3 and TLSv1.
    //SSLv2		= 1,	// HORRIBLY INSECURE!
    SSLv3 = 2,	// Supports SSLv3 only.
    TLSv1_0 = 3,	// Supports TLSv1_0 only.
    TLSv1_1 = 4,	// Supports TLSv1_1 only.
    TLSv1_2 = 5 	// Supports TLSv1_2 only.
};

/**
* Wrap OpenSSL SSL_CTX into a class.
*/
class SSLContext {
public:
    SSLContext(const SSLProtocol& protocol = SSLTLS);
    virtual ~SSLContext();
    SSL* createSSL();
    SSL_CTX* get() { return ctx_; }
private:
    SSL_CTX* ctx_;
};

/**
* SSL exception.
*/
class SSLException {
public:
    SSLException(const std::string& message) : message_(message) {}

    const char* what() const throw() {
        if (message_.empty()) {
            return "TSSLException";
        }
        else {
            return message_.c_str();
        }
    }
private:
    std::string message_;
};

class SSLSocket {
public:
    SSLSocket();
    ~SSLSocket();

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
    SSLContext* mSsl_ctx;
    std::map<int, SSL*> mSsl;
};
#endif // _SOCK_SSL_H_
