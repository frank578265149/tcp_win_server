//
// Created by frank on 17-8-6.
//

#include "tcp/ThostFtdcTraderApi.h"
#include "traderspi.h"
#include "mdspi.h"
#include "StructFunction.h"
#include "strategy.h"
#include <fstream>
#include <cstring>
#include <unistd.h>

#pragma warning(disable :4996)

extern int requestId; //������

extern Strategy *g_strategy;//������ָ��


int frontId;    //ǰ�ñ��
int sessionId;    //�Ự���
char orderRef[13];//��������

char MapDirection(char src, bool toOrig);

char MapOffset(char src, bool toOrig);


void CtpTraderSpi::OnFrontConnected() {
    cerr << "Trader Init()���óɹ�" << "Trader ���ӽ���ǰ��...�ɹ�" << endl;

    //��¼�ڻ��˺�
    ReqUserLogin(m_appId, m_userId, m_passwd);

    // SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqUserLogin(TThostFtdcBrokerIDType vAppId,
                                TThostFtdcUserIDType vUserId, TThostFtdcPasswordType vPasswd) {
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, vAppId);
    strcpy(req.UserID, vUserId);
    strcpy(req.Password, vPasswd);
    int ret = m_pUserApi_td->ReqUserLogin(&req, ++requestId);
    cerr << "Trader ���� | ���͵�¼..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << endl;
}


void CtpTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                                  CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {
        // ����Ự����
        frontId = pRspUserLogin->FrontID;
        sessionId = pRspUserLogin->SessionID;
        int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
        sprintf(orderRef, "%d", ++nextOrderRef);

        cerr << "TD_frontId:" << frontId << " sessionId:" << sessionId << " orderRef:" << orderRef << endl;

        cerr << "����ģ���¼�ɹ�" << endl << endl;

        cerr << "Trader ��Ӧ | ��¼�ɹ�...��ǰ������:"
             << pRspUserLogin->TradingDay << endl;

        m_tradeDate = pRspUserLogin->TradingDay;

        ReqSettlementInfoConfirm();//���㵥ȷ��

    }

    if (bIsLast)
        cerr << "" << endl;
    // SetEvent(g_hEvent)n;
}


void CtpTraderSpi::ReqSettlementInfoConfirm() {
    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, m_appId);
    strcpy(req.InvestorID, m_userId);
    int ret = m_pUserApi_td->ReqSettlementInfoConfirm(&req, ++requestId);

    cerr << " ���� | ���ͽ��㵥ȷ��..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << endl;
}


void CtpTraderSpi::OnRspSettlementInfoConfirm(
        CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

    if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm) {

        cerr << " ��Ӧ | ���㵥..." << pSettlementInfoConfirm->InvestorID
             << "...<" << pSettlementInfoConfirm->ConfirmDate
             << " " << pSettlementInfoConfirm->ConfirmTime << ">...ȷ��" << endl << endl;


        cerr << "���㵥ȷ���������״β�ѯ����:" << endl;
        sleep(3000);//���ʵ�������ͣʱ�䣬��CTP��̨�г������Ӧʱ�䣬Ȼ��ſ�ʼ���в�ѯ����
        ReqQryOrder();


    }


    if (bIsLast)
        cerr << "" << endl;
    // SetEvent(g_hEvent);

}


///�����ѯ��Լ��Ϣ
void CtpTraderSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId) {
    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, instId);//Ϊ�ձ�ʾ��ѯ���к�Լ
    int ret = m_pUserApi_td->ReqQryInstrument(&req, ++requestId);
    cerr << " ���� | ���ͺ�Լ��ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << endl;
}


///�����ѯ��Լ��Ϣ�����к�Լ
void CtpTraderSpi::ReqQryInstrument_all() {
    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));

    int ret = m_pUserApi_td->ReqQryInstrument(&req, ++requestId);
    cerr << " ���� | ���ͺ�Լ��ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << endl;
    cerr << " ret:" << ret << endl;
}


//�����ѯ��Լ��Ӧ����ѯ��Լ������TD��ʼ������
void CtpTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
                                      CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    if (!IsErrorRspInfo(pRspInfo) && pInstrument) {
        if (first_inquiry_Instrument == true) {
            m_Instrument_all = m_Instrument_all + pInstrument->InstrumentID + ",";

            //�������к�Լ����Ϣ��map
            CThostFtdcInstrumentField *instField = new CThostFtdcInstrumentField();
            memcpy(instField, pInstrument, sizeof(CThostFtdcInstrumentField));
            m_instMessage_map.insert(pair<string, CThostFtdcInstrumentField *>(instField->InstrumentID, instField));


            //���Խ��׵ĺ�Լ
            if (strcmp(m_instId, pInstrument->InstrumentID) == 0) {
                cerr << " ��Ӧ | ��Լ:" << pInstrument->InstrumentID
                     << " ��Լ����:" << pInstrument->InstrumentName
                     << " ��Լ�ڽ������Ĵ���:" << pInstrument->ExchangeInstID
                     << " ��Ʒ����:" << pInstrument->ProductID
                     << " ��Ʒ����:" << pInstrument->ProductClass
                     << " ������:" << pInstrument->DeliveryMonth
                     << " ��ͷ��֤����:" << pInstrument->LongMarginRatio
                     << " ��ͷ��֤����:" << pInstrument->ShortMarginRatio
                     << " ��Լ��������:" << pInstrument->VolumeMultiple
                     << " ��С�䶯��λ:" << pInstrument->PriceTick
                     << " ����������:" << pInstrument->ExchangeID
                     << " �������:" << pInstrument->DeliveryYear
                     << " ������:" << pInstrument->DeliveryMonth
                     << " ������:" << pInstrument->CreateDate
                     << " ������:" << pInstrument->OpenDate
                     << " ������:" << pInstrument->ExpireDate
                     << " ��ʼ������:" << pInstrument->StartDelivDate
                     << " ����������:" << pInstrument->EndDelivDate
                     << " ��Լ��������״̬:" << pInstrument->InstLifePhase
                     << " ��ǰ�Ƿ���:" << pInstrument->IsTrading
                     << endl;

            }


            if (bIsLast) {
                first_inquiry_Instrument = false;

                m_Instrument_all = m_Instrument_all.substr(0, m_Instrument_all.length() - 1);

                cerr << "m_Instrument_all��С��" << m_Instrument_all.length() << "," << m_Instrument_all << endl;

                cerr << "map��С(��Լ��������" << m_instMessage_map.size() << endl;

                //����Լ-��Լ��Ϣ�ṹ���map���Ƶ�������
                g_strategy->set_instMessage_map_stgy(m_instMessage_map);


                cerr << "----------------------------�����Լ��Ϣmap������----------------------------" << endl;
                //showInstMessage();

                //����ȫ�г���Լ����TD���У���Ҫ����ȫ�г���Լ����ʱ������
                //m_MDSpi->set_instIdList_all(m_Instrument_all);


                cerr << "TD��ʼ����ɣ�����MD:" << endl;
                m_pMDUserApi_td->Init();

                //  SetEvent(g_hEvent);
            }

        }


    }


}


void CtpTraderSpi::ReqQryTradingAccount() {
    CThostFtdcQryTradingAccountField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, m_appId);
    strcpy(req.InvestorID, m_userId);
    int ret = m_pUserApi_td->ReqQryTradingAccount(&req, ++requestId);
    cerr << " ���� | �����ʽ��ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << endl;

}


//�����ѯ�ʽ���Ӧ
void CtpTraderSpi::OnRspQryTradingAccount(
        CThostFtdcTradingAccountField *pTradingAccount,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    cerr << "�����ѯ�ʽ���Ӧ:OnRspQryTradingAccount" << " pTradingAccount:" << pTradingAccount << endl;

    if (!IsErrorRspInfo(pRspInfo) && pTradingAccount) {
        cerr << "��Ӧ | Ͷ�����ʺ�:" << pTradingAccount->AccountID
             << " �ϴν���׼����(��̬Ȩ�棬�ڳ�Ȩ��):" << pTradingAccount->PreBalance
             << " �ڻ�����׼����(��̬Ȩ��):" << pTradingAccount->Balance
             << " �����ʽ�:" << pTradingAccount->Available
             << " ��ȡ�ʽ�:" << pTradingAccount->WithdrawQuota
             << " ��ǰ��֤���ܶ�:" << pTradingAccount->CurrMargin
             << " ƽ��ӯ��:" << pTradingAccount->CloseProfit
             << " �ֲ�ӯ��" << pTradingAccount->PositionProfit
             << " ������:" << pTradingAccount->Commission
             << " ���ᱣ֤��:" << pTradingAccount->FrozenMargin
             << endl;

        m_MoneyAvailable = pTradingAccount->Available;

        if (firs_inquiry_TradingAccount == true) {
            firs_inquiry_TradingAccount = false;
            sleep(1000);

            cerr << "�ʽ��ѯ��������ѯͶ���ֲ߳�:" << endl;

            ReqQryInvestorPosition_all();//��ѯ���к�Լ�ֲ���Ϣ

            ///ReqQryInvestorPosition(m_instId);//��ѯ����Լ�ֲ���Ϣ
        }


    } else {
        if (firs_inquiry_TradingAccount == true) {
            firs_inquiry_TradingAccount = false;
            sleep(1000);

            cerr << "�ʽ��ѯ����,��ѯͶ���ֲ߳�:" << endl;

            ReqQryInvestorPosition_all();//��ѯ���к�Լ�ֲ���Ϣ

            ///ReqQryInvestorPosition(m_instId);//��ѯ����Լ�ֲ���Ϣ
        }

    }
    if (bIsLast)
        //  SetEvent(g_hEvent);
        cerr << "" << endl;
    cerr << "-----------------------------------------------�����ѯ�ʽ���Ӧ����" << endl;
}

///�����ѯͶ���ֲ߳�
void CtpTraderSpi::ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId) {
    CThostFtdcQryInvestorPositionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, m_appId);
    strcpy(req.InvestorID, m_userId);
    strcpy(req.InstrumentID, instId);
    int ret = m_pUserApi_td->ReqQryInvestorPosition(&req, ++requestId);
    cerr << " ���� | ���ͳֲֲ�ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << endl;
}


///�����ѯͶ���ֲ߳�,���к�Լ�ĳֲ�
void CtpTraderSpi::ReqQryInvestorPosition_all() {
    CThostFtdcQryInvestorPositionField req;
    memset(&req, 0, sizeof(req));
    //strcpy(req.BrokerID, appId);
    //strcpy(req.InvestorID, userId);
    //strcpy(req.InstrumentID, instId);
    int ret = m_pUserApi_td->ReqQryInvestorPosition(&req, ++requestId);
    cerr << " ���� | ���ͳֲֲ�ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << endl;
}


//�ֲֲ�ѯ�ص�����,�Ѿ�ƽ�ֵĵ��ӣ��ֲ���Ϊ0�˻��᷵��
void CtpTraderSpi::OnRspQryInvestorPosition(
        CThostFtdcInvestorPositionField *pInvestorPosition,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    //cout<<"�ֲֲ�ѯ�ص�����OnRspQryInvestorPosition()"<<endl;

    if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition) {
        //�˺������к�Լ
        if (firs_inquiry_Position == true) {
            cerr << "�����ѯ�ֲ���Ӧ:OnRspQryInvestorPosition" << " pInvestorPosition:" << pInvestorPosition
                 << endl;//������Ѿ�ƽ�ֶ�û�гֲ��˵ĳֲּ�¼������Ӧ�ù��˵�

            cerr << " ��Ӧ | ��Լ:" << pInvestorPosition->InstrumentID << endl
                 << " �ֲֶ�շ���:" << pInvestorPosition->PosiDirection << endl//2��3��
                 << " ӳ���ķ���:" << MapDirection(pInvestorPosition->PosiDirection - 2, false) << endl
                 << " �ֲܳ�(���ճֲ�,��������):" << pInvestorPosition->Position << endl
                 << " ���ճֲ�:" << pInvestorPosition->YdPosition << endl
                 << " ���ճֲ�:" << pInvestorPosition->TodayPosition << endl
                 << " ��֤��:" << pInvestorPosition->UseMargin << endl
                 << " �ֲֳɱ�:" << pInvestorPosition->PositionCost << endl
                 << " ������:" << pInvestorPosition->OpenVolume << endl
                 << " ƽ����:" << pInvestorPosition->CloseVolume << endl
                 << " �ֲ�����:" << pInvestorPosition->PositionDate << endl//����1��ʾ��֣�2��ʾ���
                 << " ƽ��ӯ��(�����):" << pInvestorPosition->CloseProfit << endl
                 << " �ֲ�ӯ��:" << pInvestorPosition->PositionProfit << endl
                 << " ���ն���ƽ��ӯ��(�����):" << pInvestorPosition->CloseProfitByDate << endl//������ʾ��������ֶε�ֵ
                 << " ��ʶԳ�ƽ��ӯ��(����ƽ�ּ�):" << pInvestorPosition->CloseProfitByTrade << endl//���ֶ��ڽ����бȽ�������
                 << endl;


            //�����Լ��Ӧ������Ϣ�Ľṹ���map

            bool find_trade_message_map = false;

            for (map<string, trade_message *>::iterator iter = m_trade_message_map.begin();
                 iter != m_trade_message_map.end(); iter++) {
                if (strcmp((iter->first).c_str(), pInvestorPosition->InstrumentID) == 0)//��Լ�Ѵ���
                {
                    find_trade_message_map = true;
                    break;
                }
            }

            if (!find_trade_message_map) {
                cerr << "--------------------------------------------û�������Լ�����콻����Ϣ�ṹ��" << endl;

                trade_message *trade_message_p = new trade_message();


                trade_message_p->instId = pInvestorPosition->InstrumentID;

                m_trade_message_map.insert(
                        pair<string, trade_message *>(pInvestorPosition->InstrumentID, trade_message_p));
            }


            if (pInvestorPosition->PosiDirection == '2')//�൥
            {
                //�൥�ֲ���
                //m_trade_message_map[pInvestorPosition->InstrumentID]->holding_long = pInvestorPosition->Position;
                ////////////////////////��
                m_trade_message_map[pInvestorPosition->InstrumentID]->holding_long =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->holding_long +
                        pInvestorPosition->Position;


                //�൥���
                //m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_long = pInvestorPosition->TodayPosition;
                //���㷨����������ֺ���ַֿ����η��أ��ȷ��ؽ�֣�����ֹ����ڽ����ȿ��ˣ������������ȷ��ؽ�ֵļ�¼����ʱ�����������ȷ�ģ����ٷ�����ֵļ�¼����ʱ��ֱ���ֵΪ0�����³���
                m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_long =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_long +
                        pInvestorPosition->TodayPosition;


                //�൥��� = �൥�ֲ��� - �൥���
                //m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_long = pInvestorPosition->Position - m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_long;//Ҳ����
                /////m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_long = pInvestorPosition->Position - pInvestorPosition->TodayPosition;
                ////////////////////////��
                m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_long =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->holding_long -
                        m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_long;


                //�൥ƽ��ӯ��
                //m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_long =  pInvestorPosition->CloseProfit;
                ////////////////////////��
                m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_long =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_long +
                        pInvestorPosition->CloseProfit;


                //�൥����ӯ��(��ʵ�ǳֲ�ӯ������������)
                //m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_long = pInvestorPosition->PositionProfit;
                ////////////////////////��
                m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_long =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_long +
                        pInvestorPosition->PositionProfit;


            } else if (pInvestorPosition->PosiDirection == '3')//�յ�
            {
                //�յ��ֲ���
                //m_trade_message_map[pInvestorPosition->InstrumentID]->holding_short = pInvestorPosition->Position;
                ////////////////////////��
                m_trade_message_map[pInvestorPosition->InstrumentID]->holding_short =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->holding_short +
                        pInvestorPosition->Position;


                //�յ����
                //m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_short = pInvestorPosition->TodayPosition;
                //���㷨����������ֺ���ַֿ����η��أ��ȷ��ؽ�֣�����ֹ����ڽ����ȿ��ˣ������������ȷ��ؽ�ֵļ�¼����ʱ�����������ȷ�ģ����ٷ�����ֵļ�¼����ʱ��ֱ���ֵΪ0�����³���
                m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_short =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_short +
                        pInvestorPosition->TodayPosition;


                //�յ���� = �յ��ֲ��� - �յ����
                //m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_short = pInvestorPosition->Position - m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_short;
                ////////////////////////��
                m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_short =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->holding_short -
                        m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_short;


                //�յ�ƽ��ӯ��
                //m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_short = pInvestorPosition->CloseProfit;
                ////////////////////////��
                m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_short =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_short +
                        pInvestorPosition->CloseProfit;

                //�յ�����ӯ��
                //m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_short = pInvestorPosition->PositionProfit;
                ////////////////////////��
                m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_short =
                        m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_short +
                        pInvestorPosition->PositionProfit;
            }

            if (bIsLast) {
                firs_inquiry_Position = false;


                for (map<string, trade_message *>::iterator iter = m_trade_message_map.begin();
                     iter != m_trade_message_map.end(); iter++) {
                    cerr << "��Լ����:" << iter->first << "," << iter->second->instId << endl
                         << " �൥�ֲ���:" << iter->second->holding_long << endl
                         << " �յ��ֲ���:" << iter->second->holding_short << endl
                         << " �൥���ճֲ�:" << iter->second->TodayPosition_long << endl
                         << " �൥���ճֲ�:" << iter->second->YdPosition_long << endl
                         << " �յ����ճֲ�:" << iter->second->TodayPosition_short << endl
                         << " �յ����ճֲ�:" << iter->second->YdPosition_short << endl
                         << " �൥ƽ��ӯ��:" << iter->second->closeProfit_long << endl
                         << " �൥����ӯ��:" << iter->second->OpenProfit_long << endl
                         << " �յ�ƽ��ӯ��:" << iter->second->closeProfit_short << endl
                         << " �յ�����ӯ��:" << iter->second->OpenProfit_short << endl
                         << endl;

                    //cerr<<"mapԪ�ظ���:"<<m_trade_message_map.size()<<","<<"�൥ƽ��ӯ��:"<<iter->second->closeProfit_long<<","<<"�յ�ƽ��ӯ��:"<<iter->second->closeProfit_short<<",�˻�ƽ��ӯ��"<<m_closeProfit<<endl;


                    //�˻�ƽ��ӯ��
                    m_closeProfit = m_closeProfit + iter->second->closeProfit_long + iter->second->closeProfit_short;

                    //�˻�����ӯ��
                    m_OpenProfit = m_OpenProfit + iter->second->OpenProfit_long + iter->second->OpenProfit_short;


                }

                cerr << "��ѯ�ֲַ��ؽ���:" << endl;
                cerr << "�˻�ƽ��ӯ��:" << m_closeProfit << endl
                     << "�˻�����ӯ��:" << m_OpenProfit << endl
                     << endl;

                sleep(1000);
                cerr << "��ѯ�ֲ��������״β�ѯ��Լ�����к�Լ��:" << endl;
                ReqQryInstrument_all();

            }

        }


    } else {
        if (firs_inquiry_Position == true) {
            firs_inquiry_Position = false;

            cerr << "��ѯ�ֲֳ�����û�гֲ֣��״β�ѯ��Լ�����к�Լ��:" << endl;
            sleep(1000);
            ReqQryInstrument_all();
        }

    }

    cerr << "-----------------------------------------------��ѯ�ֲַ��ص��ν���" << endl;
}


void CtpTraderSpi::ReqOrderInsert(TThostFtdcInstrumentIDType instId,
                                  TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
                                  TThostFtdcPriceType price, TThostFtdcVolumeType vol) {
    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, m_appId);  //Ӧ�õ�Ԫ����
    strcpy(req.InvestorID, m_userId); //Ͷ���ߴ���
    strcpy(req.InstrumentID, instId); //��Լ����
    strcpy(req.OrderRef, orderRef);  //��������
    int nextOrderRef = atoi(orderRef);
    sprintf(orderRef, "%d", ++nextOrderRef);

    req.LimitPrice = price;    //�۸�
    if (0 == req.LimitPrice) {
        req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;//�۸�����=�м�
        req.TimeCondition = THOST_FTDC_TC_IOC;//��Ч������:������ɣ�������
    } else {
        req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//�۸�����=�޼�
        req.TimeCondition = THOST_FTDC_TC_GFD;  //��Ч������:������Ч
    }

    req.Direction = MapDirection(dir, true);  //��������
    req.CombOffsetFlag[0] = MapOffset(kpp[0], true); //��Ͽ�ƽ��־:����

    //����Ҳ����
    /*
    req.Direction = dir;
    req.CombOffsetFlag[0] = kpp[0];
    */

    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;      //���Ͷ���ױ���־
    req.VolumeTotalOriginal = vol;    ///����
    req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
    req.MinVolume = 1;    //��С�ɽ���:1
    req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����

    //TThostFtdcPriceType	StopPrice;  //ֹ���
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;    //ǿƽԭ��:��ǿƽ
    req.IsAutoSuspend = 0;  //�Զ������־:��
    req.UserForceClose = 0;   //�û�ǿ����־:��

    int ret = m_pUserApi_td->ReqOrderInsert(&req, ++requestId);

    cerr << " ���� | ���ͱ���..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << endl;

    cerr << " ReqOrderInsert�����ڲ�:" << "instId:" << instId << " dir:" << dir << " kpp:" << kpp << " price:" << price
         << " vol:" << vol << endl;

}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
                                    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    if (!IsErrorRspInfo(pRspInfo) && pInputOrder) {
        cerr << "��Ӧ | �����ύ�ɹ�...��������:" << pInputOrder->OrderRef << endl;
    }

    // if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqOrderAction(TThostFtdcSequenceNoType orderSeq)//���͹�˾�������,int
{
    bool found = false;
    unsigned int i = 0;
    for (i = 0; i < orderList.size(); i++) {
        if (orderList[i]->BrokerOrderSeq == orderSeq) {
            found = true;
            break;
        }
    }
    if (!found) {
        cerr << " ���� | ����������." << endl;
        return;
    }

    CThostFtdcInputOrderActionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, m_appId);   //���͹�˾����
    strcpy(req.InvestorID, m_userId); //Ͷ���ߴ���
    //strcpy(req.OrderRef, pOrderRef); //��������
    //req.FrontID = frontId;           //ǰ�ñ��
    //req.SessionID = sessionId;       //�Ự���
    strcpy(req.ExchangeID, orderList[i]->ExchangeID);//����������
    strcpy(req.OrderSysID, orderList[i]->OrderSysID);//�������
    req.ActionFlag = THOST_FTDC_AF_Delete;  //������־,����

    int ret = m_pUserApi_td->ReqOrderAction(&req, ++requestId);
    cerr << " ���� | ���ͳ���..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << endl;
}

void CtpTraderSpi::OnRspOrderAction(
        CThostFtdcInputOrderActionField *pInputOrderAction,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    cerr << "OnRspOrderAction:Ҫ���������ִ��" << endl;
    if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction) {
        cerr << " ��Ӧ | �����ɹ�..."
             << "������:" << pInputOrderAction->ExchangeID
             << " �������:" << pInputOrderAction->OrderSysID
             << " ������������:" << pInputOrderAction->OrderActionRef
             << " ��������:" << pInputOrderAction->OrderRef
             << " ������:" << pInputOrderAction->RequestID
             << " ǰ�ñ��:" << pInputOrderAction->FrontID
             << " �Ự���:" << pInputOrderAction->SessionID
             << " ������־:" << pInputOrderAction->ActionFlag
             << " �۸�:" << pInputOrderAction->LimitPrice
             << " �����仯:" << pInputOrderAction->VolumeChange
             << " ��Լ����:" << pInputOrderAction->InstrumentID
             << endl;
    }

    //�����ɹ������·�ί��

    // if(bIsLast) SetEvent(g_hEvent);
}

///�����ر�
void CtpTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder) {

    cerr << "�����ر�:ǰ�ñ��FrontID:" << pOrder->FrontID << " �Ự���SessionID:" << pOrder->SessionID << " OrderRef:"
         << pOrder->OrderRef << endl;

    //���к�Լ
    CThostFtdcOrderField *order = new CThostFtdcOrderField();
    memcpy(order, pOrder, sizeof(CThostFtdcOrderField));
    bool founded = false;
    unsigned int i = 0;
    for (i = 0; i < orderList.size(); i++) {
        if (orderList[i]->BrokerOrderSeq == order->BrokerOrderSeq) {//���͹�˾�������
            founded = true;
            break;
        }
    }
    if (founded) orderList[i] = order;
    else orderList.push_back(order);

    cerr << "---------------------------------------------------------" << endl;
    cerr << " �ر� | �������ύ...���(���͹�˾�������):" << order->BrokerOrderSeq << endl//���͹�˾������ţ�����ʱ�����ҵ�����
         << " �������(ͬ�ɽ��ر�):" << order->OrderSysID << endl//��ɽ��ر�ͬ
         << " ����״̬��" << order->OrderStatus << endl
         << " ״̬��Ϣ:" << order->StatusMsg << endl
         << " ����:" << order->VolumeTotalOriginal << endl
         << " ʣ������:" << order->VolumeTotal << endl
         << " ��Լ����:" << order->InstrumentID << endl
         << " �����۸�����:" << order->OrderPriceType << endl
         << " ��������:" << order->Direction << endl
         << " ��Ͽ�ƽ��־:" << order->CombOffsetFlag << endl
         //<<" ���Ͷ���ױ���־:"<<order->CombHedgeFlag<<endl
         << " �۸�:" << order->LimitPrice << endl
         //<<" �����ύ״̬:"<<order->OrderSubmitStatus<<endl
         //<<" ��������:"<<order->OrderType<<endl
         //<<" �û�ǿ����־:"<<order->UserForceClose<<endl
         << endl;
    cerr << "---------------------------------------------------------" << endl;




    //�ѳ����ɹ����������׷��
    if (order->OrderStatus == '5') {
        TThostFtdcInstrumentIDType instId;//��Լ,��Լ�����ڽṹ�����Ѿ�����
        TThostFtdcDirectionType dir;//����,'0'��'1'��
        TThostFtdcCombOffsetFlagType kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
        TThostFtdcPriceType price;//�۸�0���м�,��������֧��
        TThostFtdcVolumeType vol;//����

        strcpy(instId, order->InstrumentID);
        dir = order->Direction;
        strcpy(kpp, order->CombOffsetFlag);

        if (order->Direction == '0')
            price = order->LimitPrice + 10;
        else if (order->Direction == '1')
            price = order->LimitPrice - 10;

        vol = order->VolumeTotal;//Ҫ��ʣ������

        //ReqOrderInsert(instId,dir,kpp,price,vol);
    }


    // SetEvent(g_hEvent);

}


///�ɽ�֪ͨ
void CtpTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade) {
    cerr << "�ɽ�֪ͨ:OnRtnTrade:" << pTrade->InstrumentID << " ��������OrderRef:" << pTrade->OrderRef << " �������(ͬ�ɽ��ر�):"
         << pTrade->OrderSysID << " �ɽ����:" << pTrade->TradeID << " ���ر������:" << pTrade->OrderLocalID << " ���͹�˾�������:"
         << pTrade->BrokerOrderSeq << endl;


    //�˻��µ����к�Լ

    //�³ɽ��ĺ�Լ��Ҫ�������飬����׼ȷ�����˻�ӯ����Ϣ

    bool find_instId_Trade = false;

    for (unsigned int i = 0; i < subscribe_inst_vec.size(); i++) {
        if (strcmp(subscribe_inst_vec[i].c_str(), pTrade->InstrumentID) == 0)//��Լ�Ѵ��ڣ��Ѷ��Ĺ�����
        {
            find_instId_Trade = true;
            break;
        }
    }

    if (!find_instId_Trade) {
        cerr << "-------------------------------------------------------------OnRtnTrade,�³ɽ��ĺ�Լ���������飺" << endl;
        m_MDSpi->SubscribeMarketData(pTrade->InstrumentID);
        subscribe_inst_vec.push_back((string) pTrade->InstrumentID);
    }




    //�³ɽ��ĺ�Լ��Ҫ�����Լ��Ӧ������Ϣ�Ľṹ���map,��count�Ϳ����ж�

    bool find_trade_message_map_onTrade = false;

    for (map<string, trade_message *>::iterator iter = m_trade_message_map.begin();
         iter != m_trade_message_map.end(); iter++) {
        if (strcmp((iter->first).c_str(), pTrade->InstrumentID) == 0)//��Լ�Ѵ���
        {
            find_trade_message_map_onTrade = true;
            break;
        }
    }

    if (!find_trade_message_map_onTrade) {
        cerr << "-------------------------------------------------------------OnRtnTrade,�³ɽ��ĺ�Լ������������Ϣmap��" << endl;

        trade_message *trade_message_p = new trade_message();

        trade_message_p->instId = pTrade->InstrumentID;

        m_trade_message_map.insert(pair<string, trade_message *>(pTrade->InstrumentID, trade_message_p));
    }


    CThostFtdcTradeField *trade_account = new CThostFtdcTradeField();
    memcpy(trade_account, pTrade, sizeof(CThostFtdcTradeField));
    bool founded = false;
    unsigned int i = 0;
    for (i = 0; i < tradeList.size(); i++) {
        if (tradeList[i]->TradeID == trade_account->TradeID) {
            founded = true;
            break;
        }
    }
    if (founded) tradeList[i] = trade_account;//�������ֶ�γɽ����ж�εĳɽ�֪ͨ����ÿ�γɽ���Ŷ���һ��
    else tradeList.push_back(trade_account);


    cerr << endl
         << " �ر� | �����ѳɽ�...�ɽ����:" << trade_account->TradeID
         << " �������:" << trade_account->OrderSysID
         << " ��Լ����:" << trade_account->InstrumentID
         << " ��������:" << trade_account->Direction
         << " ��ƽ��־:" << trade_account->OffsetFlag
         << " Ͷ���ױ���־:" << trade_account->HedgeFlag
         << " �۸�:" << trade_account->Price
         << " ����:" << trade_account->Volume
         << " �ɽ�ʱ��:" << trade_account->TradeDate
         << " �ɽ�ʱ��:" << trade_account->TradeTime
         << " ���͹�˾����:" << trade_account->BrokerID
         << " Ͷ���ߴ���:" << trade_account->InvestorID//Ҳ��userId
         << endl;


    int close_num_account_long = 0;//ƽ�ֵĶ൥����������еĻ�
    int close_num_account_short = 0;//ƽ�ֵĿյ�����������еĻ�


    //���ǿ��ֵ����򱣴浽tradeList_notClosed_account_long��tradeList_notClosed_account_short

    if (trade_account->OffsetFlag == '0')//����
    {
        if (trade_account->Direction == '0')//�൥
        {
            //�൥�ֱֲ����Ӽ�¼
            tradeList_notClosed_account_long.push_back(trade_account);
            //�൥�ֲ���
            m_trade_message_map[trade_account->InstrumentID]->holding_long =
                    m_trade_message_map[trade_account->InstrumentID]->holding_long + trade_account->Volume;
            //�൥���ճֲ�
            m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long =
                    m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long + trade_account->Volume;
        } else if (trade_account->Direction == '1')//�յ�
        {
            //�յ��ֱֲ����Ӽ�¼
            tradeList_notClosed_account_short.push_back(trade_account);
            //�յ��ֲ���
            m_trade_message_map[trade_account->InstrumentID]->holding_short =
                    m_trade_message_map[trade_account->InstrumentID]->holding_short + trade_account->Volume;
            //�յ����ճֲ�
            m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short =
                    m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short + trade_account->Volume;
        }
    }
        //ƽ�ֵ���ɾ����¼
    else if (trade_account->OffsetFlag == '1' || trade_account->OffsetFlag == '3' ||
             trade_account->OffsetFlag == '4')//1��ƽ�֣�Ҳ����ƽ�򣩣�3��ƽ��4��ƽ��
    {
        if (trade_account->Direction == '1')//������ʾƽ��,����ֺͽ��ʱ����ʱ��˳����ƽ���
        {
            close_num_account_long = trade_account->Volume;

            for (vector<CThostFtdcTradeField *>::iterator iter = tradeList_notClosed_account_long.begin();
                 iter != tradeList_notClosed_account_long.end(); iter++) {
                if (strcmp(trade_account->InstrumentID, (*iter)->InstrumentID) == 0) {
                    if ((*iter)->Volume < close_num_account_long)//û�к��Ե�tradeList_notClosed������Ϊ0�ĵ��ӣ�����Ӱ�������
                    {
                        close_num_account_long -= (*iter)->Volume;
                        //ƽ��ӯ��
                        m_trade_message_map[trade_account->InstrumentID]->closeProfit_long =
                                m_trade_message_map[trade_account->InstrumentID]->closeProfit_long +
                                (trade_account->Price - (*iter)->Price) * (*iter)->Volume *
                                m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
                        (*iter)->Volume = 0;
                    } else if ((*iter)->Volume == close_num_account_long) {
                        (*iter)->Volume = 0;
                        //ƽ��ӯ��
                        m_trade_message_map[trade_account->InstrumentID]->closeProfit_long =
                                m_trade_message_map[trade_account->InstrumentID]->closeProfit_long +
                                (trade_account->Price - (*iter)->Price) * close_num_account_long *
                                m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
                        break;
                    } else if ((*iter)->Volume > close_num_account_long) {
                        (*iter)->Volume = (*iter)->Volume - close_num_account_long;
                        //ƽ��ӯ��
                        m_trade_message_map[trade_account->InstrumentID]->closeProfit_long =
                                m_trade_message_map[trade_account->InstrumentID]->closeProfit_long +
                                (trade_account->Price - (*iter)->Price) * close_num_account_long *
                                m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
                        break;
                    }


                }
            }

            //�൥�ֲ���
            m_trade_message_map[trade_account->InstrumentID]->holding_long =
                    m_trade_message_map[trade_account->InstrumentID]->holding_long - trade_account->Volume;
            //��ֲֳ������������Ҫ���������ͷ�������


            //��������������ֻ����������Ч
            if (trade_account->OffsetFlag == '1')
                m_trade_message_map[trade_account->InstrumentID]->YdPosition_long =
                        m_trade_message_map[trade_account->InstrumentID]->YdPosition_long -
                        trade_account->Volume;//��֣�Ҳ����ƽ���ǽ��
            else if (trade_account->OffsetFlag == '3')
                m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long =
                        m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long -
                        trade_account->Volume;//���
                //////////////////
            else if (trade_account->OffsetFlag == '4')
                m_trade_message_map[trade_account->InstrumentID]->YdPosition_long =
                        m_trade_message_map[trade_account->InstrumentID]->YdPosition_long - trade_account->Volume;//���

            //������5�֣����1��ƽ�ֶ��Ƿ�'1'������ƽ��2�֣����������-1����ֻ���5�֣�ʵ��Ӧ���ǽ��5-1�����0
            //3����֣�5�ֽ�֣���'1'ƽ����4��,���������-1����ֻ���5�֣�ʵ��Ӧ���ǽ��5-1�����0

            if (m_trade_message_map[trade_account->InstrumentID]->YdPosition_long < 0) {
                m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long =
                        m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long +
                        m_trade_message_map[trade_account->InstrumentID]->YdPosition_long;
                m_trade_message_map[trade_account->InstrumentID]->YdPosition_long = 0;

            }


        } else if (trade_account->Direction == '0')//ƽ��
        {
            close_num_account_short = trade_account->Volume;

            for (vector<CThostFtdcTradeField *>::iterator iter = tradeList_notClosed_account_short.begin();
                 iter != tradeList_notClosed_account_short.end(); iter++) {
                if (strcmp(trade_account->InstrumentID, (*iter)->InstrumentID) == 0) {
                    if ((*iter)->Volume < close_num_account_short)//û�к��Ե�tradeList_notClosed������Ϊ0�ĵ��ӣ�����Ӱ�������
                    {
                        close_num_account_short -= (*iter)->Volume;
                        //ƽ��ӯ��
                        m_trade_message_map[trade_account->InstrumentID]->closeProfit_short =
                                m_trade_message_map[trade_account->InstrumentID]->closeProfit_short +
                                ((*iter)->Price - trade_account->Price) * (*iter)->Volume *
                                m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
                        (*iter)->Volume = 0;
                    } else if ((*iter)->Volume == close_num_account_short) {
                        (*iter)->Volume = 0;
                        //ƽ��ӯ��
                        m_trade_message_map[trade_account->InstrumentID]->closeProfit_short =
                                m_trade_message_map[trade_account->InstrumentID]->closeProfit_short +
                                ((*iter)->Price - trade_account->Price) * close_num_account_short *
                                m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
                        break;
                    } else if ((*iter)->Volume > close_num_account_short) {
                        (*iter)->Volume = (*iter)->Volume - close_num_account_short;
                        //ƽ��ӯ��
                        m_trade_message_map[trade_account->InstrumentID]->closeProfit_short =
                                m_trade_message_map[trade_account->InstrumentID]->closeProfit_short +
                                ((*iter)->Price - trade_account->Price) * close_num_account_short *
                                m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
                        break;
                    }

                }

            }

            //�յ��ֲ���
            m_trade_message_map[trade_account->InstrumentID]->holding_short =
                    m_trade_message_map[trade_account->InstrumentID]->holding_short - trade_account->Volume;

            //�յ����ճֲ�
            //m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short - trade_account->Volume;



            //��������������ֻ����������Ч
            if (trade_account->OffsetFlag == '1')
                m_trade_message_map[trade_account->InstrumentID]->YdPosition_short =
                        m_trade_message_map[trade_account->InstrumentID]->YdPosition_short - trade_account->Volume;//���
            else if (trade_account->OffsetFlag == '3')
                m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short =
                        m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short -
                        trade_account->Volume;//���
                //////////////////
            else if (trade_account->OffsetFlag == '4')
                m_trade_message_map[trade_account->InstrumentID]->YdPosition_short =
                        m_trade_message_map[trade_account->InstrumentID]->YdPosition_short - trade_account->Volume;//���


            if (m_trade_message_map[trade_account->InstrumentID]->YdPosition_short < 0) {
                m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short =
                        m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short +
                        m_trade_message_map[trade_account->InstrumentID]->YdPosition_short;
                m_trade_message_map[trade_account->InstrumentID]->YdPosition_short = 0;

            }
        }

    }


}


void CtpTraderSpi::OnFrontDisconnected(int nReason) {
    cerr << " ��Ӧ | �����ж�..."
         << " reason=" << nReason << endl;
}


void CtpTraderSpi::OnHeartBeatWarning(int nTimeLapse) {
    cerr << " ��Ӧ | ������ʱ����..."
         << " TimerLapse = " << nTimeLapse << endl;
}


void CtpTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    IsErrorRspInfo(pRspInfo);
}


bool CtpTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo) {
    // ���ErrorID != 0, ˵���յ��˴������Ӧ
    bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
    if (ret) {
        cerr << " ��Ӧ | " << pRspInfo->ErrorMsg << endl;
    }
    return ret;
}


void CtpTraderSpi::PrintOrders() {
    CThostFtdcOrderField *pOrder;
    for (unsigned int i = 0; i < orderList.size(); i++) {
        pOrder = orderList[i];
        cerr << " ���� | ��Լ:" << pOrder->InstrumentID
             << " ����:" << MapDirection(pOrder->Direction, false)
             << " ��ƽ:" << MapOffset(pOrder->CombOffsetFlag[0], false)
             << " �۸�:" << pOrder->LimitPrice
             << " ����:" << pOrder->VolumeTotalOriginal
             << " ���:" << pOrder->BrokerOrderSeq
             << " �������:" << pOrder->OrderSysID
             << " ״̬:" << pOrder->StatusMsg << endl;
    }
    //SetEvent(g_hEvent);
}


void CtpTraderSpi::PrintTrades() {
    CThostFtdcTradeField *pTrade;
    for (unsigned int i = 0; i < tradeList.size(); i++) {
        pTrade = tradeList[i];
        cerr << " �ɽ� | ��Լ:" << pTrade->InstrumentID
             << " ����:" << MapDirection(pTrade->Direction, false)
             << " ��ƽ:" << MapOffset(pTrade->OffsetFlag, false)
             << " �۸�:" << pTrade->Price
             << " ����:" << pTrade->Volume
             << " �������:" << pTrade->OrderSysID
             << " �ɽ����:" << pTrade->TradeID << endl;
    }
    // SetEvent(g_hEvent);
}


char MapDirection(char src, bool toOrig = true) {
    if (toOrig) {
        if ('b' == src || 'B' == src) { src = '0'; } else if ('s' == src || 'S' == src) { src = '1'; }
    } else {
        if ('0' == src) { src = 'B'; } else if ('1' == src) { src = 'S'; }
    }
    return src;
}


char MapOffset(char src, bool toOrig = true) {
    if (toOrig) {
        if ('o' == src || 'O' == src) { src = '0'; }
        else if ('c' == src || 'C' == src) { src = '1'; }
        else if ('j' == src || 'J' == src) { src = '3'; }
    } else {
        if ('0' == src) { src = 'O'; }
        else if ('1' == src) { src = 'C'; }
        else if ('3' == src) { src = 'J'; }
    }
    return src;
}


//�����ѯ����
void CtpTraderSpi::ReqQryOrder() {
    CThostFtdcQryOrderField req;
    memset(&req, 0, sizeof(req));

    strcpy(req.InvestorID, m_userId);//Ͷ���ߴ���,Ҳ��userId

    int ret = m_pUserApi_td->ReqQryOrder(&req, ++requestId);


    cerr << " ���� | ���Ͳ�ѯ����..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret << endl;//retֵΪ-3��ʾÿ�뷢�����������������

}


///�����ѯ������Ӧ,Ҫ�ֳ�������ʱ��һ�β�ѯ��֮��Ĳ�ѯ
void CtpTraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                 bool bIsLast) {
    cerr << "�����ѯ������Ӧ:OnRspQryOrder" << " pOrder:" << pOrder << endl;


    if (!IsErrorRspInfo(pRspInfo) && pOrder) {
        cerr << "�����ѯ������Ӧ:ǰ�ñ��FrontID:" << pOrder->FrontID << " �Ự���SessionID:" << pOrder->SessionID << " OrderRef:"
             << pOrder->OrderRef << endl;

        //���к�Լ
        if (first_inquiry_order == true) {
            CThostFtdcOrderField *order = new CThostFtdcOrderField();
            memcpy(order, pOrder, sizeof(CThostFtdcOrderField));
            orderList.push_back(order);


            if (bIsLast) {
                first_inquiry_order = false;

                cerr << "���к�Լ����������" << orderList.size() << endl;

                cerr << "--------------------------------------------------------------------����start" << endl;
                for (vector<CThostFtdcOrderField *>::iterator iter = orderList.begin(); iter != orderList.end(); iter++)
                    cerr << "���͹�˾����:" << (*iter)->BrokerID << endl << " Ͷ���ߴ���:" << (*iter)->InvestorID << endl
                         << " �û�����:" << (*iter)->UserID << endl << " ��Լ����:" << (*iter)->InstrumentID << endl << " ��������:"
                         << (*iter)->Direction << endl
                         << " ��Ͽ�ƽ��־:" << (*iter)->CombOffsetFlag << endl << " �۸�:" << (*iter)->LimitPrice << endl
                         << " ����:" << (*iter)->VolumeTotalOriginal << endl << " ��������:" << (*iter)->OrderRef << endl
                         << " �ͻ�����:" << (*iter)->ClientID << endl
                         << " ����״̬:" << (*iter)->OrderStatus << endl << " ί��ʱ��:" << (*iter)->InsertTime << endl
                         << " �������:" << (*iter)->OrderSysID << endl << " GTD����:" << (*iter)->GTDDate << endl << " ������:"
                         << (*iter)->TradingDay << endl
                         << " ��������:" << (*iter)->InsertDate << endl
                         << endl;

                cerr << "--------------------------------------------------------------------����end" << endl;


                sleep(1000);
                cerr << "��ѯ�����������״β�ѯ�ɽ�:" << endl;
                ReqQryTrade();

                //  SetEvent(g_hEvent);

            }
        }


    } else {
        if (first_inquiry_order == true) {
            first_inquiry_order = false;
            sleep(1000);
            cerr << "��ѯ����������û�б������״β�ѯ�ɽ�:" << endl;
            ReqQryTrade();
        }

    }

    cerr << "-----------------------------------------------�����ѯ������Ӧ���ν���" << endl;

}


void
CtpTraderSpi::setAccount(TThostFtdcBrokerIDType appId, TThostFtdcUserIDType userId, TThostFtdcPasswordType passwd) {
    strcpy(m_appId, appId);
    strcpy(m_userId, userId);
    strcpy(m_passwd, passwd);

}


//����������׷�������ڱ����ر�����ȳ����ɹ����ٽ���
void CtpTraderSpi::CancelOrder(const string &MDtime, double MDprice) {
    string InsertTime_str;//ί��ʱ��

    //TThostFtdcInstrumentIDType    instId;//��Լ,��Լ�����ڽṹ�����Ѿ�����
    //TThostFtdcDirectionType       dir;//����,'0'��'1'��
    //TThostFtdcCombOffsetFlagType  kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
    //TThostFtdcPriceType           price;//�۸�0���м�,��������֧��
    //TThostFtdcVolumeType          vol;//����

    int MDtime_last2;

    if (orderList.size() > 0) {
        //cerr<<"orderList.size():"<<orderList.size()<<endl;
        for (vector<CThostFtdcOrderField *>::iterator iter = orderList.begin();
             iter != orderList.end(); iter++)//���������break
        {
            /*
            cout<<"��Լ:"<<(*iter)->InstrumentID<<" ��������OrderRef:"<<(*iter)->OrderRef<<" ǰ�ñ��FrontID:"<<(*iter)->FrontID<<" �Ự���SessionID:"<<(*iter)->SessionID
            <<" ����״̬:"<<(*iter)->OrderStatus<<" ���͹�˾�������:"<<(*iter)->BrokerOrderSeq
            <<endl;
            */

            if ((*iter)->OrderStatus == '3' || (*iter)->OrderStatus == '1')//δ�ɽ����ڶ�����,���ֳɽ����ڶ�����
            {

                InsertTime_str = (*iter)->InsertTime;//ί��ʱ��"21:47:01"

                MDtime_last2 = atoi(MDtime.substr(6, 2).c_str());

                if (MDtime_last2 < atoi(InsertTime_str.substr(6, 2).c_str()))//����ʱ�������λС��ί��ʱ��������λ
                    MDtime_last2 += 60;//����ʱ���60��

                if (MDtime_last2 - atoi(InsertTime_str.substr(6, 2).c_str()) >= 6)//ί�д���6��δ�ɽ�
                {
                    cerr << "����:" << endl;
                    //����
                    ReqOrderAction((*iter)->BrokerOrderSeq);

                    //���·�ί�У�Ӧ���ڳ����ɹ����ٽ���

                    //strcpy(instId, (*iter)->InstrumentID);
                    //dir = (*iter)->Direction;
                    //strcpy(kpp, (*iter)->CombOffsetFlag);

                    //if((*iter)->Direction == '0')
                    //	price = MDprice + 10;
                    //else if((*iter)->Direction == '1')
                    //	price = MDprice - 10;

                    //vol = (*iter)->VolumeTotal;//Ҫ��ʣ������

                    //cerr<<"���·���:"<<endl;
                    //ReqOrderInsert(instId,dir,kpp,price,vol);

                }


            }
        }
    }
}


//�����ѯ�ɽ�
void CtpTraderSpi::ReqQryTrade() {
    CThostFtdcQryTradeField req;
    memset(&req, 0, sizeof(req));

    strcpy(req.InvestorID, m_userId);//Ͷ���ߴ���,Ҳ��userId

    int ret = m_pUserApi_td->ReqQryTrade(&req, ++requestId);

    cerr << " ���� | ���ͳɽ���ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret << endl;//retֵΪ-3��ʾÿ�뷢�����������������

}


//�����ѯ�ɽ���Ӧ,Ҫ�ֳ�������ʱ��һ�β�ѯ��֮��Ĳ�ѯ
//ֻ�ܲ�ѯ����ģ���ֲ��ܣ����Ի�Ҫ��ѯ�ֲ���ϸOnRspQryInvestorPositionDetail()
void CtpTraderSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                 bool bIsLast) {
    cerr << "�����ѯ�ɽ���Ӧ:OnRspQryTrade" << " pTrade:" << pTrade << endl;

    if (!IsErrorRspInfo(pRspInfo) && pTrade) {
        //cerr<<"�����ѯ�ɽ���Ӧ��OnRspQryTrade"<<" pTrade:"<<pTrade<<endl;

        //���к�Լ
        if (first_inquiry_trade == true) {
            CThostFtdcTradeField *trade = new CThostFtdcTradeField();
            memcpy(trade, pTrade, sizeof(CThostFtdcTradeField));
            tradeList.push_back(trade);


            if (bIsLast) {
                first_inquiry_trade = false;

                cerr << "�ɽ�����:" << tradeList.size() << endl;

                cerr << "--------------------------------------------------------------------�ɽ�start" << endl;
                for (vector<CThostFtdcTradeField *>::iterator iter = tradeList.begin();
                     iter != tradeList.end(); iter++) {
                    cerr << "��Լ����:" << (*iter)->InstrumentID << endl << " �û�����:" << (*iter)->UserID << endl << " �ɽ����:"
                         << (*iter)->TradeID << endl << " ��������:" << (*iter)->Direction << endl << " ��ƽ��־:"
                         << (*iter)->OffsetFlag << endl
                         << " Ͷ���ױ���־:" << (*iter)->HedgeFlag << endl << " �۸�:" << (*iter)->Price << endl << " ����:"
                         << (*iter)->Volume << endl << " �ɽ�ʱ��:" << (*iter)->TradeTime << endl
                         << " �ɽ�����:" << (*iter)->TradeType << endl << " �������:" << (*iter)->OrderSysID << endl
                         << " ��������:" << (*iter)->OrderRef << endl << " ���ر������:" << (*iter)->OrderLocalID << endl
                         << " ҵ��Ԫ:" << (*iter)->BusinessUnit << endl << " ���:" << (*iter)->SequenceNo << endl
                         << " ���͹�˾�������:" << (*iter)->BrokerOrderSeq << endl
                         << " �ɽ�ʱ��:" << (*iter)->TradeDate << endl << " ������:" << (*iter)->TradingDay

                         << endl << endl;
                }
                cerr << "--------------------------------------------------------------------�ɽ�end" << endl;


                sleep(1000);
                //�����ѯͶ���ֲ߳���ϸ
                cerr << "��ѯ�ɽ��������״β�ѯͶ���ֲ߳���ϸ:" << endl;
                ReqQryInvestorPositionDetail();

                // SetEvent(g_hEvent);
            }
        }


    } else {
        if (first_inquiry_trade == true) {
            first_inquiry_trade = false;
            sleep(1000);
            //�����ѯͶ���ֲ߳���ϸ
            cerr << "��ѯ�ɽ�������û�гɽ����״β�ѯͶ���ֲ߳���ϸ:" << endl;
            ReqQryInvestorPositionDetail();
        }

    }

    cerr << "-----------------------------------------------�����ѯ�ɽ���Ӧ���ν���" << endl;

}


//�����ѯͶ���ֲ߳���ϸ
void CtpTraderSpi::ReqQryInvestorPositionDetail() {
    CThostFtdcQryInvestorPositionDetailField req;
    memset(&req, 0, sizeof(req));

    strcpy(req.InvestorID, m_userId);//Ͷ���ߴ���,Ҳ��userId

    //strcpy(req.InstrumentID, "IF1402");

    int ret = m_pUserApi_td->ReqQryInvestorPositionDetail(&req, ++requestId);

    cerr << " ���� | ����Ͷ���ֲ߳���ϸ��ѯ..." << ((ret == 0) ? "�ɹ�" : "ʧ��") << " ret:" << ret << endl;//retֵΪ-3��ʾÿ�뷢�����������������

}


///�����ѯͶ���ֲ߳���ϸ��Ӧ,Ҫ�ֳ�������ʱ��һ�β�ѯ��֮��Ĳ�ѯ
void CtpTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail,
                                                  CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    cerr << "�����ѯͶ���ֲ߳���ϸ��Ӧ��OnRspQryInvestorPositionDetail" << " pInvestorPositionDetail:" << pInvestorPositionDetail
         << endl;

    if (!IsErrorRspInfo(pRspInfo) && pInvestorPositionDetail) {
        //��ʱ���Ⱥ����򷵻أ�����Լ����

        //���к�Լ
        if (firs_inquiry_Detail == true) {
            //�������к�Լ����������ƽ�ֵģ�ֻ����δƽ�ֵ�
            //����������ǰ�ĳֲּ�¼���浽δƽ������tradeList_notClosed_account_long��tradeList_notClosed_account_short
            //Ϊʲô���ڲ�ѯ�ɽ��ص�����OnRspQryTrade()������,��Ϊֻ�ܲ�ѯ�����
            //ʹ�ýṹ����CThostFtdcTradeField����Ϊ����ʱ���ֶΣ���CThostFtdcInvestorPositionDetailFieldû��ʱ���ֶ�

            CThostFtdcTradeField *trade = new CThostFtdcTradeField();

            strcpy(trade->InvestorID, pInvestorPositionDetail->InvestorID);///Ͷ���ߴ���
            strcpy(trade->InstrumentID, pInvestorPositionDetail->InstrumentID);///��Լ����
            strcpy(trade->ExchangeID, pInvestorPositionDetail->ExchangeID);///����������
            trade->Direction = pInvestorPositionDetail->Direction;///��������
            trade->Price = pInvestorPositionDetail->OpenPrice;///�۸�
            trade->Volume = pInvestorPositionDetail->Volume;///����
            strcpy(trade->TradeDate, pInvestorPositionDetail->OpenDate);///�ɽ�ʱ��


            if (pInvestorPositionDetail->Volume > 0)//ɸѡδƽ�ֵ�
            {
                if (trade->Direction == '0')
                    tradeList_notClosed_account_long.push_back(trade);
                else if (trade->Direction == '1')
                    tradeList_notClosed_account_short.push_back(trade);


                //�ռ��ֲֺ�Լ�ĺ�Լ����

                bool find_instId = false;

                for (unsigned int i = 0; i < subscribe_inst_vec.size(); i++) {
                    if (strcmp(subscribe_inst_vec[i].c_str(), trade->InstrumentID) == 0)//��Լ�Ѵ��ڣ��Ѷ��Ĺ�����
                    {
                        find_instId = true;
                        break;
                    }
                }

                if (!find_instId) {
                    cerr << "--------------------------------------�óֲֺ�Լû�ж��Ĺ����飬�ӽ������б�" << endl;
                    subscribe_inst_vec.push_back((string) trade->InstrumentID);
                }


            }


            //������к�Լ�ĳֲ���ϸ��Ҫ����߽�����һ���Ĳ�ѯReqQryTradingAccount();
            if (bIsLast) {
                firs_inquiry_Detail = false;


                string inst_holding;//����ֲֵĺ�Լ�б�

                for (unsigned int i = 0; i < subscribe_inst_vec.size(); i++)
                    inst_holding = inst_holding + subscribe_inst_vec[i] + ",";

                inst_holding = inst_holding.substr(0, inst_holding.length() - 1);//ȥ�������Ķ���

                cerr << "��������ǰ�ĳֲ��б�:" << inst_holding << "," << inst_holding.length() << "," << inst_holding.size()
                     << endl;

                if (inst_holding.length() > 0)
                    m_MDSpi->setInstIdList_holding_md(inst_holding);//���ó�������ǰ�����֣�����Ҫ��������ĺ�Լ




                cerr << "�˻����к�Լδƽ�ֵ�����(��������,һ�ʿɶ�Ӧ����):�൥" << tradeList_notClosed_account_long.size() << " �յ�"
                     << tradeList_notClosed_account_short.size() << endl;

                cerr << "----------------------------------------------��������ǰδƽ�ֵ�(�ȶ���)start" << endl;

                for (vector<CThostFtdcTradeField *>::iterator iter = tradeList_notClosed_account_long.begin();
                     iter != tradeList_notClosed_account_long.end(); iter++) {
                    cerr << "Ͷ���ߴ���:" << (*iter)->InvestorID << endl
                         << " ��Լ����:" << (*iter)->InstrumentID << endl
                         << " ����������:" << (*iter)->ExchangeID << endl
                         << " ��������:" << (*iter)->Direction << endl
                         << " �۸�:" << (*iter)->Price << endl
                         << " ����:" << (*iter)->Volume << endl
                         << " �ɽ�ʱ��:" << (*iter)->TradeDate << endl
                         << " �ɽ�ʱ��(�ֲ���ϸ�ṹ����):" << (*iter)->TradeTime << endl << endl;
                }

                for (vector<CThostFtdcTradeField *>::iterator iter = tradeList_notClosed_account_short.begin();
                     iter != tradeList_notClosed_account_short.end(); iter++) {
                    cerr << "Ͷ���ߴ���:" << (*iter)->InvestorID << endl
                         << " ��Լ����:" << (*iter)->InstrumentID << endl
                         << " ����������:" << (*iter)->ExchangeID << endl
                         << " ��������:" << (*iter)->Direction << endl
                         << " �۸�:" << (*iter)->Price << endl
                         << " ����:" << (*iter)->Volume << endl
                         << " �ɽ�ʱ��:" << (*iter)->TradeDate << endl
                         << " �ɽ�ʱ��(�ֲ���ϸ�ṹ����):" << (*iter)->TradeTime << endl << endl;
                }

                cerr << "------------------------------------------------��������ǰδƽ�ֵ�(�ȶ���)end" << endl;


                sleep(1000);
                cerr << "��ѯͶ���ֲ߳���ϸ�����������ѯ�ʽ��˻�:" << endl;
                ReqQryTradingAccount();

            }
        }


    } else {
        if (firs_inquiry_Detail == true) {
            firs_inquiry_Detail = false;
            sleep(1000);

            cerr << "��ѯͶ���ֲ߳���ϸ������û��Ͷ���ֲ߳���ϸ����ѯ�ʽ��˻�:" << endl;
            ReqQryTradingAccount();
        }

    }

    cerr << "-----------------------------------------------�����ѯͶ���ֲ߳���ϸ��Ӧ����" << endl;

}


//���ý��׵ĺ�Լ����
void CtpTraderSpi::setInstId(string instId) {
    strcpy(m_instId, instId.c_str());
}


//ǿ��ƽ�֣������˻����к�Լ
void CtpTraderSpi::ForceClose() {
    TThostFtdcInstrumentIDType instId;//��Լ
    TThostFtdcDirectionType dir;//����,'0'��'1'��
    TThostFtdcCombOffsetFlagType kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
    TThostFtdcPriceType price;//�۸�0���м�,��������֧��
    TThostFtdcVolumeType vol;//����


    for (map<string, trade_message *>::iterator iter = m_trade_message_map.begin();
         iter != m_trade_message_map.end(); iter++) {

        cerr << "��Լ����:" << iter->first << "," << iter->second->instId << endl
             << " �൥�ֲ���:" << iter->second->holding_long << endl
             << " �յ��ֲ���:" << iter->second->holding_short << endl
             << " �൥���ճֲ�:" << iter->second->TodayPosition_long << endl
             << " �൥���ճֲ�:" << iter->second->YdPosition_long << endl
             << " �յ����ճֲ�:" << iter->second->TodayPosition_short << endl
             << " �յ����ճֲ�:" << iter->second->YdPosition_short << endl
             << " �൥ƽ��ӯ��:" << iter->second->closeProfit_long << endl
             << " �൥����ӯ��:" << iter->second->OpenProfit_long << endl
             << " �յ�ƽ��ӯ��:" << iter->second->closeProfit_short << endl
             << " �յ�����ӯ��:" << iter->second->OpenProfit_short << endl
             << endl;



        //ƽ��
        if (iter->second->holding_long > 0) {
            strcpy(instId, iter->second->instId.c_str());//��strcpy(instId, iter->first.c_str());
            dir = '1';
            price = iter->second->lastPrice - 5 * m_instMessage_map[instId]->PriceTick; //ƽ�ָ���

            //������
            if (strcmp(m_instMessage_map[instId]->ExchangeID, "SHFE") == 0) {
                if (iter->second->YdPosition_long == 0)//û�����
                {
                    cerr << "�൥������ȫ��ƽ��" << endl;

                    strcpy(kpp, "3");//ƽ��
                    vol = iter->second->holding_long;
                    ReqOrderInsert(instId, dir, kpp, price, vol);

                } else if (iter->second->TodayPosition_long == 0)//û�н��
                {
                    cerr << "�൥������ȫ��ƽ��" << endl;

                    strcpy(kpp, "1");//ƽ��
                    vol = iter->second->holding_long;
                    ReqOrderInsert(instId, dir, kpp, price, vol);

                }
                    //ͬʱ������ֺͽ��
                else if (iter->second->YdPosition_long > 0 && iter->second->TodayPosition_long > 0) {
                    cerr << "�൥������ͬʱƽ��ƽ��" << endl;

                    strcpy(kpp, "3");//ƽ��
                    vol = iter->second->TodayPosition_long;
                    ReqOrderInsert(instId, dir, kpp, price, vol);

                    strcpy(kpp, "1");//ƽ��
                    vol = iter->second->YdPosition_long;
                    ReqOrderInsert(instId, dir, kpp, price, vol);

                }

            }
                //��������
            else {
                cerr << "���������൥ƽ��:" << endl;

                strcpy(kpp, "1");
                vol = iter->second->holding_long;
                ReqOrderInsert(instId, dir, kpp, price, vol);

            }


        }


        //ƽ��
        if (iter->second->holding_short > 0) {
            strcpy(instId, iter->second->instId.c_str());//��strcpy(instId, iter->first.c_str());
            dir = '0';
            price = iter->second->lastPrice + 5 * m_instMessage_map[instId]->PriceTick;

            //������
            if (strcmp(m_instMessage_map[instId]->ExchangeID, "SHFE") == 0) {
                if (iter->second->YdPosition_short == 0)//û�����
                {
                    cerr << "�յ�������ȫ��ƽ��" << endl;

                    strcpy(kpp, "3");//ƽ��
                    vol = iter->second->holding_short;
                    ReqOrderInsert(instId, dir, kpp, price, vol);

                } else if (iter->second->TodayPosition_short == 0)//û�н��
                {
                    cerr << "�յ�������ȫ��ƽ��" << endl;

                    strcpy(kpp, "1");//ƽ��
                    vol = iter->second->holding_short;
                    ReqOrderInsert(instId, dir, kpp, price, vol);

                }
                    //ͬʱ������ֺͽ��
                else if (iter->second->YdPosition_short > 0 && iter->second->TodayPosition_short > 0) {
                    cerr << "�յ�������ͬʱƽ��ƽ��" << endl;

                    strcpy(kpp, "3");//ƽ��
                    vol = iter->second->TodayPosition_short;
                    ReqOrderInsert(instId, dir, kpp, price, vol);

                    strcpy(kpp, "1");//ƽ��
                    vol = iter->second->YdPosition_short;
                    ReqOrderInsert(instId, dir, kpp, price, vol);

                }

            }
                //��������
            else {
                cerr << "���������յ�ƽ��:" << endl;

                strcpy(kpp, "1");
                vol = iter->second->holding_short;
                ReqOrderInsert(instId, dir, kpp, price, vol);

            }


        }


    }

}


//����ƽ��ӯ����ƽ��ӯ���ڳɽ��ر��и���
double CtpTraderSpi::sendCloseProfit() {
    double closeProfit_account = 0;

    for (map<string, trade_message *>::iterator iter = m_trade_message_map.begin();
         iter != m_trade_message_map.end(); iter++) {
        //ƽ��ӯ��
        closeProfit_account = closeProfit_account + iter->second->closeProfit_long + iter->second->closeProfit_short;
    }

    m_closeProfit = closeProfit_account;

    return closeProfit_account;

}


//�����˻��ĸ���ӯ�����Կ��ּ���,�����޶��ֲ�������0�ģ�����ӯ�������걣�浽m_trade_message_map
//���Ҫ�õ������˻��ĸ���ӯ�����ǲ���Ҫ�ȵ����к�Լ�����º����
double CtpTraderSpi::sendOpenProfit_account(string instId, double lastPrice) {
    double openProfit_account_long = 0, openProfit_account_short = 0, openProfit_account_all = 0;

    //�൥�ĸ���ӯ��
    for (vector<CThostFtdcTradeField *>::iterator iter = tradeList_notClosed_account_long.begin();
         iter != tradeList_notClosed_account_long.end(); iter++) {
        if (strcmp(instId.c_str(), (*iter)->InstrumentID) == 0)
            openProfit_account_long = openProfit_account_long + (lastPrice - (*iter)->Price) * (*iter)->Volume *
                                                                m_instMessage_map[(*iter)->InstrumentID]->VolumeMultiple;

    }

    if (m_trade_message_map.count(instId) > 0)//�ú�Լ�гֲ�
        m_trade_message_map[instId]->OpenProfit_long = openProfit_account_long;//txt�еĺ�Լû�гֲ֣�������ص����ܵ���


    //�յ��ĸ���ӯ��
    for (vector<CThostFtdcTradeField *>::iterator iter = tradeList_notClosed_account_short.begin();
         iter != tradeList_notClosed_account_short.end(); iter++) {
        if (strcmp(instId.c_str(), (*iter)->InstrumentID) == 0)
            openProfit_account_short = openProfit_account_short + ((*iter)->Price - lastPrice) * (*iter)->Volume *
                                                                  m_instMessage_map[(*iter)->InstrumentID]->VolumeMultiple;
    }

    if (m_trade_message_map.count(instId) > 0)//�ú�Լ�гֲ�
        m_trade_message_map[instId]->OpenProfit_short = openProfit_account_short;

    for (map<string, trade_message *>::iterator iter = m_trade_message_map.begin();
         iter != m_trade_message_map.end(); iter++) {
        //����ӯ��
        openProfit_account_all =
                openProfit_account_all + iter->second->OpenProfit_long + iter->second->OpenProfit_short;

    }

    m_OpenProfit = openProfit_account_all;

    return openProfit_account_all;


}


//��ӡ���к�Լ��Ϣ
void CtpTraderSpi::showInstMessage() {
    for (map<string, CThostFtdcInstrumentField *>::iterator iter = m_instMessage_map.begin();
         iter != m_instMessage_map.end(); iter++) {
        cerr << iter->first << "," << iter->second->InstrumentID << "," << iter->second->PriceTick << ","
             << iter->second->VolumeMultiple << endl;

    }

    cerr << "m_instMessage_map.size():" << m_instMessage_map.size() << endl;


}


//��ӡ�ֲ���Ϣm_trade_message_map
void CtpTraderSpi::printTrade_message_map() {
    cerr << "------------------------------------------------printTrade_message_map��ʼ" << endl;
    for (map<string, trade_message *>::iterator iter = m_trade_message_map.begin();
         iter != m_trade_message_map.end(); iter++) {
        if (iter->second->holding_long > 0 || iter->second->holding_short > 0) {
            cerr << "��Լ����:" << iter->first << "," << iter->second->instId << endl
                 << " �൥�ֲ���:" << iter->second->holding_long << endl
                 << " �յ��ֲ���:" << iter->second->holding_short << endl
                 << " �൥���ճֲ�:" << iter->second->TodayPosition_long << endl
                 << " �൥���ճֲ�:" << iter->second->YdPosition_long << endl
                 << " �յ����ճֲ�:" << iter->second->TodayPosition_short << endl
                 << " �յ����ճֲ�:" << iter->second->YdPosition_short << endl
                 << " �൥ƽ��ӯ��:" << iter->second->closeProfit_long << endl
                 << " �൥����ӯ��:" << iter->second->OpenProfit_long << endl
                 << " �յ�ƽ��ӯ��:" << iter->second->closeProfit_short << endl
                 << " �յ�����ӯ��:" << iter->second->OpenProfit_short << endl
                 << endl;
        }
    }
    cerr << "------------------------------------------------printTrade_message_map����" << endl;

}


//���º�Լ�����¼�
void CtpTraderSpi::setLastPrice(string instID, double price) {
    m_trade_message_map[instID]->lastPrice = price;

}


//��Լ������Ϣ�ṹ���map��KEY�ĸ�����Ϊ0��ʾû��
int CtpTraderSpi::send_trade_message_map_KeyNum(string instID) {
    return m_trade_message_map.count(instID);

}


int CtpTraderSpi::SendHolding_long(string instID) {
    //cerr<<"m_trade_message_map.count(instID):"<<m_trade_message_map.count(instID)<<endl;

    if (m_trade_message_map.count(instID) == 0)
        return 0;
    else
        return m_trade_message_map[instID]->holding_long;

}


int CtpTraderSpi::SendHolding_short(string instID) {
    if (m_trade_message_map.count(instID) == 0)
        return 0;
    else
        return m_trade_message_map[instID]->holding_short;

}

int CtpTraderSpi::SendHolding_short_YD(string instID) {
    if (m_trade_message_map.count(instID) == 0)
        return 0;
    else
        return m_trade_message_map[instID]->YdPosition_short;

}

int CtpTraderSpi::SendHolding_long_YD(string instID) {
    if (m_trade_message_map.count(instID) == 0)
        return 0;
    else
        return m_trade_message_map[instID]->YdPosition_long;

}