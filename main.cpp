#include <iostream>

#include "main.h"
#include "mdspi.h"
#include "traderspi.h"

int requestId = 0; //请求编号

Strategy *g_strategy;//策略类指针

CtpTraderSpi *g_pUserSpi_tradeAll;//全局的TD回调处理类对象，人机交互函数需用到

int main() {
    std::cout << "Hello, World!" << std::endl;

    //--------------读取配置文件，获取账户信息、服务器地址、交易的合约代码--------------
    ReadMessage readMessage;
    memset(&readMessage, 0, sizeof(readMessage));
    SetMessage(readMessage);
    //--------------初始化行情UserApi，创建行情API实例----------------------------------
    string pszFlowPath = "MDflow";
    CThostFtdcMdApi *pUserApi_md = CThostFtdcMdApi::CreateFtdcMdApi(pszFlowPath.c_str());
    CtpMdSpi *pUserSpi_md = new CtpMdSpi(pUserApi_md);//创建回调处理类对象MdSpi
    pUserApi_md->RegisterSpi(pUserSpi_md);// 回调对象注入接口类
    pUserApi_md->RegisterFront(readMessage.m_mdFront);// 注册行情前置地址

    pUserSpi_md->setAccount(readMessage.m_appId, readMessage.m_userId, readMessage.m_passwd);//经纪公司编号，用户名，密码

    pUserSpi_md->setInstId(readMessage.m_read_contract);//MD所需订阅行情的合约，即策略交易的合约


    //--------------初始化交易UserApi，创建交易API实例----------------------------------
    CThostFtdcTraderApi *pUserApi_trade = CThostFtdcTraderApi::CreateFtdcTraderApi(pszFlowPath.c_str());
    CtpTraderSpi *pUserSpi_trade = new CtpTraderSpi(pUserApi_trade, pUserApi_md, pUserSpi_md);//构造函数初始化三个变量
    pUserApi_trade->RegisterSpi((CThostFtdcTraderSpi *) pUserSpi_trade);// 注册事件类
    pUserApi_trade->SubscribePublicTopic(THOST_TERT_RESTART);// 注册公有流
    pUserApi_trade->SubscribePrivateTopic(THOST_TERT_QUICK);// 注册私有流THOST_TERT_QUICK
    pUserApi_trade->RegisterFront(readMessage.m_tradeFront);// 注册交易前置地址

    pUserSpi_trade->setAccount(readMessage.m_appId, readMessage.m_userId, readMessage.m_passwd);//经纪公司编号，用户名，密码

    pUserSpi_trade->setInstId(readMessage.m_read_contract);//策略交易的合约

    g_pUserSpi_tradeAll = pUserSpi_trade;//全局的TD回调处理类对象，人机交互函数需用到

    //--------------创建策略实例--------------------------------------------------------
    g_strategy = new Strategy(pUserSpi_trade);
    g_strategy->setInstId(readMessage.m_read_contract);

    //--------------TD线程先启动--------------------------------------------------------
    pUserApi_trade->Init();//TD初始化完成后，再进行MD的初始化

    //--------------人机交互模块--------------------------------------------------------
    //  HANDLE hThread1=CreateThread(NULL,0,ThreadProc,NULL,0,NULL);
    // CloseHandle(hThread1);
    // WaitForSingleObject(hThread1, INFINITE);


    pUserApi_md->Join();//等待接口线程退出
    pUserApi_trade->Join();


    return 0;
}