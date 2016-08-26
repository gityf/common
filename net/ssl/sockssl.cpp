// Copyright (c) 2015, Wang Yaofu
// All rights reserved.
//
// Author: Wang Yaofu, voipman@qq.com.
// Created: 06/03/2015
// Description: file of socket openssl.

#include <stdio.h>
#include "sockssl.h"

SockSSL::SockSSL() {
}

SockSSL::~SockSSL() {
    SSL_UnInit();
}

int SockSSL::SSL_Init(const std::string& aCACertFile,
                       const std::string& aPrivKeyFile,
                       const std::string& aCipher,
                       bool aIsClient) {
    for (int ii = 0; ii < MAX_SOCKET_ID; ii++) {
        iSsl[ii] = NULL;
    }
    // SSL library initialize.
    SSL_library_init();
    // load all SSL cipher.
    OpenSSL_add_all_algorithms();
    // load all SSL error message.
    SSL_load_error_strings();
    if (aIsClient) {
        // SSL client.
        iSsl_ctx = SSL_CTX_new(SSLv23_client_method());
        if (iSsl_ctx == NULL) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
    } else {
        // SSL server.
        // SSL_CTX(SSL Content Text) is created by SSL V2 and V3.
        iSsl_ctx = SSL_CTX_new(SSLv23_server_method());
        // V2 by SSLv2_server_method(), V3 by SSLv3_server_method().
        if  (iSsl_ctx == NULL) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
        // load user CA, it is sent to client. public key is in this CA.
        if (SSL_CTX_use_certificate_file(iSsl_ctx, aCACertFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
        // load user private key.
        if (SSL_CTX_use_PrivateKey_file(iSsl_ctx, aPrivKeyFile.c_str(), SSL_FILETYPE_PEM) <= 0) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
        // check whether user private key is OK.
        if (!SSL_CTX_check_private_key(iSsl_ctx)) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
        if (!aCipher.empty()) {
            if (SSL_CTX_set_cipher_list(iSsl_ctx, aCipher.c_str()) == 0) {
                ERR_print_errors_fp(stderr);
                return RET_ERROR;
            }
        }
    }
    return RET_OK;
}

void SockSSL::SSL_UnInit() {
    for (int ii = 0; ii < MAX_SOCKET_ID; ii++) {
        if (iSsl[ii] != NULL) {
            SSL_SockClose(ii);
        }
    }
    SSL_CTX_free(iSsl_ctx);
}

int SockSSL::SSL_Read(int aSockId, char *aBuffer, int aLen) {
    if (aSockId >= MAX_SOCKET_ID) {
        return RET_ERROR;
    }
    // read.
    return SSL_read(iSsl[aSockId], aBuffer, aLen);
}

int SockSSL::SSL_Write(int aSockId, char *aBuffer, int aLen) {
    if (aSockId >= MAX_SOCKET_ID) {
        return RET_ERROR;
    }
    // write.
    return SSL_write(iSsl[aSockId], aBuffer, aLen);
}

int SockSSL::SSL_ConnHandShake(int aSockId, bool aIsClient) {
    if (aSockId >= MAX_SOCKET_ID) {
        return RET_ERROR;
    }
    if (iSsl[aSockId] != NULL) {
        SSL_SockClose(aSockId);
    }
    // create new SSL base on ctx.
    iSsl[aSockId] = SSL_new(iSsl_ctx);
    // socket id is added into SSL.
    SSL_set_fd(iSsl[aSockId], aSockId);
    if (aIsClient) {
        // create SSL client connection.
        if (SSL_connect(iSsl[aSockId]) == RET_ERROR) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }
    } else {
        // create SSL Server connection.
        if (SSL_accept(iSsl[aSockId]) == RET_ERROR) {
            ERR_print_errors_fp(stderr);
            return RET_ERROR;
        }        
    }
    return RET_OK;
}

int SockSSL::SSL_SockClose(int aSockId) {
    if (aSockId >= MAX_SOCKET_ID) {
        return RET_ERROR;
    }
    SSL_shutdown(iSsl[aSockId]);
    SSL_free(iSsl[aSockId]);
    iSsl[aSockId] = NULL;
    return RET_OK;
}

void SockSSL::ShowCerts(SSL * ssl) {
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
    } else {
        fprintf(stderr, "no CA Infos.\n");
    }
}
