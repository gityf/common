/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class SmtpMail.
*/
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include "base/localdef.h"
#include "base/log.h"
#include "base/base64.h"
#include "socket.h"
#include "smtpmail.h"

// ---------------------------------------------------------------------------
// SmtpMail::SmtpMail()
//
// constructor
// ---------------------------------------------------------------------------
//
SmtpMail::SmtpMail() {
    mSocket = NULL;
    mIsInitialed = false;
}

// ---------------------------------------------------------------------------
// SmtpMail::~SmtpMail()
//
// destructor.
// ---------------------------------------------------------------------------
//
SmtpMail::~SmtpMail() {
    if (mSocket != NULL) {
        delete mSocket;
        mSocket = NULL;
    }
}

// ---------------------------------------------------------------------------
// int SmtpMail::InitData()
//
// init operation.
// ---------------------------------------------------------------------------
//
int SmtpMail::InitData() {
    if (!mSocket) {
        mSocket = new Socket();
    }
    mIsInitialed = true;
    mSocketTimeOut = 5;
    mCurSockId = 0;
    return RET_OK;
}

// ---------------------------------------------------------------------------
// void SmtpMail::SendMail(struct SEmailInfos aMailIffo)
//
// send email.
// ---------------------------------------------------------------------------
//
void SmtpMail::SendMail(struct SEmailInfos& aMailIffo) {
    if (!mIsInitialed) {
        InitData();
    }

    if (aMailIffo.mEmailServer.size() <= 0) {
        aMailIffo.mEmailServer = "mail.AsiaInfo.com";
    }
    char emailBuffer[MAX_SIZE_2K*2] = {0};
    if (!mSocket->Name2IPv4(aMailIffo.mEmailServer.c_str(), emailBuffer)) {
        LOG(LL_ERROR, "bad email server name.");
        return;
    }
    LOG(LL_VARS, "IP of %s is : %s\n",
        aMailIffo.mEmailServer.c_str(), emailBuffer);

    struct SAddrInfo addrInfo;
    addrInfo.mAddr = emailBuffer;
    addrInfo.mPort = 25;
    addrInfo.mProtocol = TCP;
    // receiving welcome informations.
    int tryTime = 3;
    int ret = 0;
    do {
        if (RET_ERROR == mSocket->CreateSocket(&addrInfo)) {
            LOG(LL_ERROR, "CreateSocket error.");
            return;
        }
        if (RET_ERROR == mSocket->TcpConnect(addrInfo)) {
            LOG(LL_ERROR, "TcpConnect error.");
            return;
        }
        ret = mSocket->SelectSockId(addrInfo.mSockId, 5, FD_READABLE);
        if (ret & FD_READABLE) {
            ret = mSocket->TcpRecvOnce(addrInfo.mSockId, emailBuffer, MAX_SIZE_2K);
        }
    } while (ret == RET_ERROR && tryTime-- > 0);
    mCurSockId = addrInfo.mSockId;
    emailBuffer[ret] = 0x00;
    LOG(LL_INFO, "Welcome Informations:\n%s", emailBuffer);

    // EHLO
    string tmpOnlyUser;
    if (aMailIffo.mUserName.find_last_of('@') > 0) {
        tmpOnlyUser = aMailIffo.mUserName.substr(0, aMailIffo.mUserName.find_last_of('@'));
    } else {
        tmpOnlyUser = aMailIffo.mUserName;
    }
    sprintf(emailBuffer, "EHLO %s\r\n", tmpOnlyUser.c_str());
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "MailSendRecv EHLO error.");
        return;
    }
    LOG(LL_INFO, "EHLO REceive:%s", emailBuffer);

    // AUTH LOGIN
    sprintf(emailBuffer, "AUTH LOGIN\r\n");
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "MailSendRecv AUTH LOGIN error.");
        return;
    }
    LOG(LL_INFO, "Auth Login Receive:%s", emailBuffer);

    // USER
    memset(emailBuffer, 0, sizeof(emailBuffer));
    size_t tmplen = 0;
    Base64Encode(aMailIffo.mUserName, emailBuffer, &tmplen);
    strcat(emailBuffer, "\r\n");
    LOG(LL_INFO, "Base64 UserName:%s", emailBuffer);
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "MailSendRecv USER error.");
        return;
    }
    LOG(LL_INFO, "User Login Receive:%s", emailBuffer);

    // PASSWORD
    memset(emailBuffer, 0, sizeof(emailBuffer));
    Base64Encode(aMailIffo.mUserPass, emailBuffer, &tmplen);
    strcat(emailBuffer, "\r\n");
    LOG(LL_INFO, "Base64 Password:%s", emailBuffer);
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "MailSendRecv PASSWORD error.");
        return;
    }
    LOG(LL_INFO, "Send Password Receive:%s", emailBuffer);

    // MAIL FROM
    sprintf(emailBuffer, "MAIL FROM: <%s>\r\n", aMailIffo.mUserName.c_str());
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "MailSendRecv MAIL FROM error.");
        return;
    }
    LOG(LL_INFO, "set Mail From Receive:%s", emailBuffer);

    // RCPT TO
    int ii = 0, pos = 0, offset = 0;
    while (ii < aMailIffo.mEmailTo.length()) {
        if (aMailIffo.mEmailTo.at(ii) == ',') {
            offset = sprintf(emailBuffer+offset,
                "RCPT TO:<%s>\r\n",
                aMailIffo.mEmailTo.substr(pos, ii - pos).c_str());
            pos = 1 + ii;
        }
        ii++;
    }
    if (ii > pos) {
        offset = sprintf(emailBuffer+offset,
            "RCPT TO:<%s>\r\n",
            aMailIffo.mEmailTo.substr(pos, ii - pos).c_str());
    }

    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "MailSendRecv RCPT TO error.");
        return;
    }
    LOG(LL_INFO, "Tell Sendto Receive:%s", emailBuffer);

    // DATA
    sprintf(emailBuffer, "DATA\r\n");
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "MailSendRecv DATA error.");
        return;
    }
    LOG(LL_INFO, "Send Mail Prepare Receive:%s", emailBuffer);

    // CONTENTS
    snprintf(emailBuffer,
        MAX_SIZE_2K,
        "From:%s\r\n"
        "To:%s\r\n"
        "Subject:%s\r\n"
        "Content-Type: text/plain;"
        "\r\n\r\n%s\r\n.\r\n",
        aMailIffo.mUserName.c_str(),
        aMailIffo.mEmailTo.c_str(),
        aMailIffo.mSubJect.c_str(),
        aMailIffo.mEmailContent.c_str());
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "MailSendRecv CONTENTS error.");
        return;
    }
    LOG(LL_INFO, "Send Mail Receive:%s", emailBuffer);

    // QUIT
    sprintf(emailBuffer, "QUIT\r\n");
    if (RET_ERROR == MailSendRecv(emailBuffer)) {
        LOG(LL_ERROR, "MailSendRecv QUIT error.");
        return;
    }
    LOG(LL_INFO, "Quit Receive:%s", emailBuffer);

    // clean
    close(addrInfo.mSockId);
    return;
}

// ---------------------------------------------------------------------------
// int SmtpMail::MailSendRecv(string aMsg)
//
// send email info.
// ---------------------------------------------------------------------------
//
int SmtpMail::MailSendRecv(char *aMsg) {
    int bufLen = strlen(aMsg);
    if (bufLen != mSocket->TcpSend(mCurSockId, aMsg, bufLen, mSocketTimeOut)) {
        LOG(LL_ERROR, "MailSendRecv:TcpSend %s error.", aMsg);
        return RET_ERROR;
    }
    int ret = mSocket->SelectSockId(mCurSockId, mSocketTimeOut, FD_READABLE);
    if (ret & FD_READABLE) {
        ret = mSocket->TcpRecvOnce(mCurSockId, aMsg, MAX_SIZE_2K);
        if (ret > 0) {
            aMsg[ret] = 0x00;
        }
    }
    return ret;
}
// end of local file.
