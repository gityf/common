// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/03/2015
// Description: file of socket openssl.

#include <stdio.h>
#include "sslsocket.h"

using std::string;

namespace {
    // extract error messages from error queue
    void buildErrors(std::string& errors, int errno_copy) {
        unsigned long  errorCode;
        char   message[256];

        errors.reserve(512);
        while ((errorCode = ERR_get_error()) != 0) {
            if (!errors.empty()) {
                errors += "; ";
            }
            const char* reason = ERR_reason_error_string(errorCode);
            if (reason != NULL) {
                errors += reason;
            }
        }
        if (errors.empty()) {
            if (errno_copy != 0) {
                errors += strerror(errno_copy);
            }
        }
    }
}
// SSLContext implementation
SSLContext::SSLContext(const SSLProtocol& protocol) {
    switch (protocol)
    {
    case SSLTLS:
        ctx_ = SSL_CTX_new(SSLv23_method());
        break;
    case SSLv3:
        ctx_ = SSL_CTX_new(SSLv3_method());
        break;
    case TLSv1_0:
        ctx_ = SSL_CTX_new(TLSv1_method());
        break;
    case TLSv1_1:
        ctx_ = SSL_CTX_new(TLSv1_1_method());
        break;
    case TLSv1_2:
        ctx_ = SSL_CTX_new(TLSv1_2_method());
        break;
    default:
        break;
    }
    if (ctx_ == NULL) {
        std::string errors;
        buildErrors(errors, errno);
        throw SSLException("SSL_CTX_new: " + errors);
    }
    SSL_CTX_set_mode(ctx_, SSL_MODE_AUTO_RETRY);

    // Disable horribly insecure SSLv2!
    if (protocol == SSLTLS) {
        SSL_CTX_set_options(ctx_, SSL_OP_NO_SSLv2);
    }
}

SSLContext::~SSLContext() {
    if (ctx_ != NULL) {
        SSL_CTX_free(ctx_);
        ctx_ = NULL;
    }
}

SSL* SSLContext::createSSL() {
    SSL* ssl = SSL_new(ctx_);
    if (ssl == NULL) {
        string errors;
        buildErrors(errors, errno);
        throw SSLException("SSL_new: " + errors);
    }
    return ssl;
}

SSLSocket::SSLSocket() {
}

SSLSocket::~SSLSocket() {
    SSL_UnInit();
}

int SSLSocket::SSL_Init(const std::string& aCACertFile,
    const std::string& aPrivKeyFile,
    const std::string& aCipher,
    bool aIsClient) {
    // SSL library initialize.
    SSL_library_init();
    // load all SSL cipher.
    OpenSSL_add_all_algorithms();
    // load all SSL error message.
    SSL_load_error_strings();
    if (aIsClient) {
        // SSL client.
        mSsl_ctx = new SSLContext(SSLTLS);
        if (mSsl_ctx == NULL) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
    }
    else {
        // SSL server.
        // SSL_CTX(SSL Content Text) is created by SSL V2 and V3.
        mSsl_ctx = new SSLContext(SSLTLS);
        // V2 by SSLv2_server_method(), V3 by SSLv3_server_method().
        if (mSsl_ctx == NULL) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
        // load user CA, it is sent to client. public key is in this CA.
        if (SSL_CTX_use_certificate_file(mSsl_ctx->get(), aCACertFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
        // load user private key.
        if (SSL_CTX_use_PrivateKey_file(mSsl_ctx->get(), aPrivKeyFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
        // check whether user private key is OK.
        if (!SSL_CTX_check_private_key(mSsl_ctx->get())) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
        if (!aCipher.empty()) {
            if (SSL_CTX_set_cipher_list(mSsl_ctx->get(), aCipher.c_str()) == 0) {
                ERR_print_errors_fp(stderr);
                return RET_ERROR;
            }
        }
    }
    return RET_OK;
}

void SSLSocket::SSL_UnInit() {
    std::map<int, SSL*>::iterator it = mSsl.begin();
    for (; it != mSsl.end(); ++it) {
        SSL_shutdown(it->second);
        SSL_free(it->second);
    }
    mSsl.clear();
    if (mSsl_ctx != NULL) {
        delete mSsl_ctx;
        mSsl_ctx = NULL;
    }
}

int SSLSocket::SSL_Read(int aSockId, char *aBuffer, int aLen) {
    int32_t bytes = 0;
    SSL* ssl = mSsl[aSockId];
    for (int32_t retries = 0; retries < 5; retries++){
        bytes = SSL_read(ssl, aBuffer, aLen);
        if (bytes >= 0)
            break;
        if (SSL_get_error(ssl, bytes) == SSL_ERROR_SYSCALL) {
            if (ERR_get_error() == 0 && errno == EINTR) {
                continue;
            }
        }
        std::string errors;
        buildErrors(errors, errno);
        throw SSLException(errors);
    }
    return bytes;
}

int SSLSocket::SSL_Write(int aSockId, char *aBuffer, int aLen) {
    uint32_t written = 0;
    SSL* ssl = mSsl[aSockId];
    while (written < aLen) {
        int32_t bytes = SSL_write(ssl, &aBuffer[written], aLen - written);
        if (bytes <= 0) {
            std::string errors;
            buildErrors(errors, errno);
            throw SSLException(errors);
        }
        written += bytes;
    }
    return written;
}

int SSLSocket::SSL_ConnHandShake(int aSockId, bool aIsClient) {
    if (mSsl.find(aSockId) != mSsl.end()) {
        SSL_SockClose(aSockId);
    }
    // create new SSL base on ctx.
    mSsl[aSockId] = mSsl_ctx->createSSL();
    // socket id is added into SSL.
    SSL_set_fd(mSsl[aSockId], aSockId);
    if (aIsClient) {
        // create SSL client connection.
        if (SSL_connect(mSsl[aSockId]) == RET_ERROR) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
    }
    else {
        // create SSL Server connection.
        if (SSL_accept(mSsl[aSockId]) == RET_ERROR) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
    }
    return RET_OK;
}

int SSLSocket::SSL_SockClose(int fd) {
    if (mSsl.find(fd) == mSsl.end()) {
        return RET_ERROR;
    }
    
    SSL_shutdown(mSsl[fd]);
    SSL_free(mSsl[fd]);
    mSsl.erase(fd);
    return RET_OK;
}

void SSLSocket::ShowCerts(SSL * ssl) {
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        fprintf(stderr, "CA Infos:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        fprintf(stderr, "CA: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        fprintf(stderr, "Author: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else {
        fprintf(stderr, "no CA Infos.\n");
    }
}
