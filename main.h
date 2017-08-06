//
// Created by frank on 17-8-6.
//

#ifndef TCP_WIN_MAIN_H_H
#define TCP_WIN_MAIN_H_H

#include "tcp/ThostFtdcUserApiDataType.h"
#include "RdWrini.h"
#include <string>
#include <iostream>
#include <cstring>

using namespace std;


//�����ȡ����Ϣ�Ľṹ��
struct ReadMessage {
    TThostFtdcBrokerIDType m_appId;//���͹�˾����
    TThostFtdcUserIDType m_userId;//�û���
    TThostFtdcPasswordType m_passwd;//����
    char m_mdFront[50];//�����������ַ
    char m_tradeFront[50];//���׷�������ַ

    string m_read_contract;//��Լ����
};


void SetMessage(ReadMessage &readMessage)//Ҫ������
{
    //-------------------------------��ȡ�˺�ģ��-------------------------------
    string read_brokerID = "9999";
    // string brokerID="brokerID";
    // char read_brokerID_buffer[MAX_PATH];
    //GetPrivateProfileString("Account",brokerID.c_str(),"brokerID_error",read_brokerID_buffer,MAX_PATH,"input/AccountParam.ini");

    string read_userId = "092597";
    // string userId="userId";
    // char read_userId_buffer[MAX_PATH];
    //GetPrivateProfileString("Account",userId.c_str(),"userId_error",read_userId_buffer,MAX_PATH,"input/AccountParam.ini");

    string read_passwd = "885888";
    // string passwd="passwd";
    // char read_passwd_buffer[MAX_PATH];
    // GetPrivateProfileString("Account",passwd.c_str(),"passwd_error",read_passwd_buffer,MAX_PATH,"input/AccountParam.ini");

    strcpy(readMessage.m_appId, read_brokerID.c_str());
    strcpy(readMessage.m_userId, read_userId.c_str());
    strcpy(readMessage.m_passwd, read_passwd.c_str());



    //-------------------------------��ȡ��ַģ��-------------------------------
    string read_MDAddress = "tcp://180.168.146.187:10010";
    // string MDAddress="MDAddress";
    // char read_MDAddress_buffer[MAX_PATH];
    // GetPrivateProfileString("FrontAddress",MDAddress.c_str(),"MDAddress_error",read_MDAddress_buffer,MAX_PATH,"./input/AccountParam.ini");

    string read_TDAddress = "tcp://180.168.146.187:10000";
    // string TDAddress="TDAddress";
    // char read_TDAddress_buffer[MAX_PATH];
    // GetPrivateProfileString("FrontAddress",TDAddress.c_str(),"TDAddress_error",read_TDAddress_buffer,MAX_PATH,"./input/AccountParam.ini");

    strcpy(readMessage.m_mdFront, read_MDAddress.c_str());
    strcpy(readMessage.m_tradeFront, read_TDAddress.c_str());


    //-------------------------------���ú�Լģ��-------------------------------
    string read_contract = "rb1710";
    // string contract="contract";
    //  char read_contract_buffer[MAX_PATH];
    // GetPrivateProfileString("Contract",contract.c_str(),"contract_error",read_contract_buffer,MAX_PATH,"./input/AccountParam.ini");

    readMessage.m_read_contract = read_contract;

}

#endif //TCP_WIN_MAIN_H_H
