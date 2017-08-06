#include <iostream>

#include "main.h"
#include "mdspi.h"
#include "traderspi.h"

int requestId = 0; //������

Strategy *g_strategy;//������ָ��

CtpTraderSpi *g_pUserSpi_tradeAll;//ȫ�ֵ�TD�ص�����������˻������������õ�

int main() {
    std::cout << "Hello, World!" << std::endl;

    //--------------��ȡ�����ļ�����ȡ�˻���Ϣ����������ַ�����׵ĺ�Լ����--------------
    ReadMessage readMessage;
    memset(&readMessage, 0, sizeof(readMessage));
    SetMessage(readMessage);
    //--------------��ʼ������UserApi����������APIʵ��----------------------------------
    string pszFlowPath = "MDflow";
    CThostFtdcMdApi *pUserApi_md = CThostFtdcMdApi::CreateFtdcMdApi(pszFlowPath.c_str());
    CtpMdSpi *pUserSpi_md = new CtpMdSpi(pUserApi_md);//�����ص����������MdSpi
    pUserApi_md->RegisterSpi(pUserSpi_md);// �ص�����ע��ӿ���
    pUserApi_md->RegisterFront(readMessage.m_mdFront);// ע������ǰ�õ�ַ

    pUserSpi_md->setAccount(readMessage.m_appId, readMessage.m_userId, readMessage.m_passwd);//���͹�˾��ţ��û���������

    pUserSpi_md->setInstId(readMessage.m_read_contract);//MD���趩������ĺ�Լ�������Խ��׵ĺ�Լ


    //--------------��ʼ������UserApi����������APIʵ��----------------------------------
    CThostFtdcTraderApi *pUserApi_trade = CThostFtdcTraderApi::CreateFtdcTraderApi(pszFlowPath.c_str());
    CtpTraderSpi *pUserSpi_trade = new CtpTraderSpi(pUserApi_trade, pUserApi_md, pUserSpi_md);//���캯����ʼ����������
    pUserApi_trade->RegisterSpi((CThostFtdcTraderSpi *) pUserSpi_trade);// ע���¼���
    pUserApi_trade->SubscribePublicTopic(THOST_TERT_RESTART);// ע�ṫ����
    pUserApi_trade->SubscribePrivateTopic(THOST_TERT_QUICK);// ע��˽����THOST_TERT_QUICK
    pUserApi_trade->RegisterFront(readMessage.m_tradeFront);// ע�ύ��ǰ�õ�ַ

    pUserSpi_trade->setAccount(readMessage.m_appId, readMessage.m_userId, readMessage.m_passwd);//���͹�˾��ţ��û���������

    pUserSpi_trade->setInstId(readMessage.m_read_contract);//���Խ��׵ĺ�Լ

    g_pUserSpi_tradeAll = pUserSpi_trade;//ȫ�ֵ�TD�ص�����������˻������������õ�

    //--------------��������ʵ��--------------------------------------------------------
    g_strategy = new Strategy(pUserSpi_trade);
    g_strategy->setInstId(readMessage.m_read_contract);

    //--------------TD�߳�������--------------------------------------------------------
    pUserApi_trade->Init();//TD��ʼ����ɺ��ٽ���MD�ĳ�ʼ��

    //--------------�˻�����ģ��--------------------------------------------------------
    //  HANDLE hThread1=CreateThread(NULL,0,ThreadProc,NULL,0,NULL);
    // CloseHandle(hThread1);
    // WaitForSingleObject(hThread1, INFINITE);


    pUserApi_md->Join();//�ȴ��ӿ��߳��˳�
    pUserApi_trade->Join();


    return 0;
}