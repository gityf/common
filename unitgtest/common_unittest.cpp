#include <unistd.h>
#include <string.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "ut/test_harness.h"
#include "base/process.h"
#include "base/singleton.h"
#include "base/stringutils.h"
#include "ipc/msgqueue.h"
#include "base/colorprint.h"
using namespace std;
using namespace common;
TEST(ProcessTest, RunShellTest)
{
    Process *proc = Singleton<Process>::Instance();
    string shell = "ps -ef | grep common_unittest | wc -l";
    string f = "test.shell.res";
    string ress = proc->RunShell(shell, f);

    EXPECT_EQ(2, atoi(ress.c_str()));
}

TEST(CMsgQueuetTest, sendBlock)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_EXCL | IPC_CREAT);
    EXPECT_LT(0, msgid);
    SMsgBuffer msgBuf;
    msgBuf.mMsgType = getpid();
    sprintf(msgBuf.mMsgStr, "Msg:Test at:%u", time(NULL));
    for (int i = 0; i < 20; ++i) {
        EXPECT_EQ(RET_OK, msgQueue->MsgSend(msgid, &msgBuf));
    }
    common::ColorGreen(cout) << "[ INFO     ] sendBlock:sendstr:" << msgBuf.mMsgStr
        << " type:" << msgBuf.mMsgType
        << " times: 20" 
        << " msgid: " << msgid << endl;
    common::ColorRestore(cout);

    delete msgQueue;
}

TEST(CMsgQueuetTest, set)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    char *p = getlogin();
    if (p != NULL && memcmp("root", p, 4) == 0) {
        EXPECT_EQ(RET_OK, msgQueue->MsgSet(msgid, 12345678));
    } else {
        EXPECT_EQ(RET_ERROR, msgQueue->MsgSet(msgid, 12345678));
    }

    delete msgQueue;
}

TEST(CMsgQueuetTest, stat)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    string statStr = msgQueue->MsgStat(msgid);    
    common::ColorGreen(cout) << "[ INFO     ] MSG_STAT:" << statStr << endl;
    common::ColorRestore(cout);
    delete msgQueue;
}

TEST(CMsgQueuetTest, recvBlock)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    SMsgBuffer msgBuf;
    msgBuf.mMsgType = getpid();
    for (int i = 0; i < 20; ++i) {
        EXPECT_LT(RET_OK,
            msgQueue->MsgRecv(msgid, &msgBuf, sizeof(msgBuf.mMsgStr)));
    }
    common::ColorGreen(cout) << "[ INFO     ] recvBlock:recvstr:" << msgBuf.mMsgStr
        << " type:" << msgBuf.mMsgType
        << " times: 20" 
        << " msgid: " << msgid << endl;
    common::ColorRestore(cout);
    delete msgQueue;
}

TEST(CMsgQueuetTest, sendUnBlock)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_EXCL | IPC_CREAT);
    EXPECT_LT(0, msgid);
    SMsgBuffer msgBuf;
    msgBuf.mMsgType = getpid();
    sprintf(msgBuf.mMsgStr, "Msg:Test at:%u", time(NULL));
    for (int i = 0; i < 20; ++i) {
        EXPECT_EQ(RET_OK, msgQueue->MsgSend(msgid, &msgBuf, IPC_NOWAIT));
    }
    common::ColorGreen(cout) << "[ INFO     ] sendUnBlock:sendstr:" << msgBuf.mMsgStr
        << " type:" << msgBuf.mMsgType
        << " times: 20" 
        << " msgid: " << msgid << endl;
    common::ColorRestore(cout);

    delete msgQueue;
}

TEST(CMsgQueuetTest, recvUnBlock)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    SMsgBuffer msgBuf;
    msgBuf.mMsgType = getpid();
    for (int i = 0; i < 20; ++i) {
        EXPECT_LT(RET_OK,
            msgQueue->MsgRecv(msgid, &msgBuf, sizeof(msgBuf.mMsgStr), IPC_NOWAIT));
    }
    common::ColorGreen(cout) << "[ INFO     ] recvUnBlock:recvstr:" << msgBuf.mMsgStr
        << " type:" << msgBuf.mMsgType
        << " times: 20" 
        << " msgid: " << msgid << endl;
    common::ColorRestore(cout);
    delete msgQueue;
}

TEST(CMsgQueuetTest, rm)
{
    CMsgQueue* msgQueue = new CMsgQueue();
    int msgid = msgQueue->MsgGet(getpid(), IPC_CREAT);
    EXPECT_LT(0, msgid);
    EXPECT_EQ(RET_OK, msgQueue->MsgRm(msgid));
    SMsgBuffer msgBuf;
    msgBuf.mMsgType = getpid();
    sprintf(msgBuf.mMsgStr, "Msg:Test at:%u", time(NULL));
    EXPECT_EQ(RET_ERROR, msgQueue->MsgSend(msgid, &msgBuf));
    delete msgQueue;
}

// CStringUtils Unit test
TEST(CStringUtilsTest, Basic)
{
    string format = "123:44.56:strstrtr:0123";
    string format_str;
    StrUtils::Format(format_str, "%d:%.2f:%s:%04d", 123, 44.56, "strstrtr", 123);
    EXPECT_EQ(format, format_str);
    EXPECT_EQ(true, StrUtils::IsUnderLine('_'));
    EXPECT_EQ(true, StrUtils::IsLetter('a'));
    EXPECT_EQ(true, StrUtils::IsLetter('z'));
    EXPECT_EQ(true, StrUtils::IsLetter('A'));
    EXPECT_EQ(true, StrUtils::IsLetter('Z'));
    EXPECT_EQ(false, StrUtils::IsLetter('1'));
    EXPECT_EQ(true, StrUtils::IsNumber('1'));
    EXPECT_EQ(true, StrUtils::IsNumber('0'));
    EXPECT_EQ(true, StrUtils::IsNumber('9'));
    EXPECT_EQ(false, StrUtils::IsNumber('@'));
    EXPECT_EQ("0ABC123E", StrUtils::Hex("\x0a\xbc\x12\x3e", 4, true));
    EXPECT_EQ("0abc123e", StrUtils::Hex("\x0a\xbc\x12\x3e"));
    std::vector<string> vvss;
    vvss.push_back("11");
    vvss.push_back("22");
    vvss.push_back("3333");
    EXPECT_EQ("11&22&3333", StrUtils::JoinStr(vvss, '&'));
    EXPECT_EQ("11&&22&&3333", StrUtils::JoinStr(vvss, "&&"));
    EXPECT_EQ("abcfdgf123", StrUtils::ToLowerCase("aBCfDGF123"));
    EXPECT_EQ("ABCFDGF123", StrUtils::ToUpperCase("aBCfdgf123"));
    EXPECT_EQ(12345678901, StrUtils::stol("12345678901"));
    EXPECT_EQ(1234567890, StrUtils::stoi("1234567890"));
}
