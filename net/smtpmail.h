/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class SmtpMail.
*/
#ifndef _COMMON_SMTP_MAIL_H_
#define _COMMON_SMTP_MAIL_H_
#include <string>
using std::string;
struct SEmailInfos{
    string mEmailServer;
    string mEmailTo;
    string mUserName;
    string mSubJect;
    string mUserPass;
    string mEmailContent;
};
class Socket;
class SmtpMail
{
public:
    /*
    * Initial operation.
    *
    */
    int InitData();

    /*
    * send email.
    *
    */
    void SendMail(struct SEmailInfos& aMailIffo);

    /*
    * constructor.
    *
    */
    SmtpMail();

    /*
    * destructor.
    *
    */
    ~SmtpMail();

    /*
    * communication with mail server.
    *
    */
    int MailSendRecv(char *aMsg);

private:
    /*
    * socket instance pointer to Socket.
    *
    */
    Socket *mSocket;

    /*
    * is initializing.
    *
    */
    bool mIsInitialed;

    /*
    * current socket id.
    *
    */
    int mCurSockId;

    /*
    * socket timeout.
    *
    */
    int mSocketTimeOut;
};

#endif // _COMMON_SMTP_MAIL_H_
