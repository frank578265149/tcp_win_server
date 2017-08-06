//
// Created by frank on 17-8-6.
//

#include "strategy.h"
#include "strategy.h"
#include <fstream>
#include <cstring>
#include <unistd.h>
#include "time.h"
#include "stdio.h"

using namespace std;

#define LIRUNLV 0.0005
#define ZHIYINGLV 0.0002
#define ZHISUN_DUO 0.002
#define ZHISUN_KONG 0.001
#define KXIAN_SHULIANG 5
#define KAICANG_BILI 0.05;
#define JUNXIAN_KXIANSHULIANG 12
#define JUNXIAN_GENSHU 5

void Strategy::OnTickData(CThostFtdcDepthMarketDataField *pDepthMarketData) {

    //�����˻���ӯ����Ϣ
    CalculateEarningsInfo(pDepthMarketData);


    if (strcmp(pDepthMarketData->InstrumentID, m_instId) == 0) {
        //cerr<<"�����յ�����:"<<pDepthMarketData->InstrumentID<<","<<pDepthMarketData->TradingDay<<","<<pDepthMarketData->UpdateTime<<",���¼�:"<<pDepthMarketData->LastPrice<<",��ͣ��:"<<pDepthMarketData->UpperLimitPrice<<",��ͣ��:"<<pDepthMarketData->LowerLimitPrice<<endl;

        //�������ݵ�vector
        SaveDataVec(pDepthMarketData);

        //�������ݵ�txt��csv
        SaveDataTxtCsv(pDepthMarketData);

        //����׷��,�����ɹ�����׷��
        TDSpi_stgy->CancelOrder(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice);

        if (!m_allow_open)
            return;

        //����ƽ�֣��������߼��ļ��㣩
        StrategyCalculate(pDepthMarketData);


    }


}


//���ý��׵ĺ�Լ����
void Strategy::setInstId(string instId) {
    strcpy(m_instId, instId.c_str());

}


//�������߼��ļ��㣬70������������0.6Ԫ�������࣬�µ�������
void Strategy::StrategyCalculate(CThostFtdcDepthMarketDataField *pDepthMarketData) {

    cerr << "Time:" << futureData.time;
    if (futureData.time.size() > 0) {
        char m0 = futureData.time[0];
        char m1 = futureData.time[1];
        char m3 = futureData.time[3];
        char m4 = futureData.time[4];

        if (m0 == '1' && m1 == '4') {
            cerr << "14dian" << endl;
            if (m3 == '5' && m4 == '5') {
                //TDSpi_stgy->ForceClose();
                set_allow_open(false);
                exit(0);
            }
        }
    }
    AllLongPosition = TDSpi_stgy->GetAllLongPosition();
    AllShortPosition = TDSpi_stgy->GetAllShortPosition();

    DrawMA(futureData, m_currentIndex, JUNXIAN_KXIANSHULIANG, JUNXIAN_GENSHU);

    int max = 60;
    bool isOneLineEnds = DrawKLine(futureData, m_currentIndex, max);
    m_currentIndex++;

    if (m_KLineHigh.size() > 55 && isOneLineEnds) {
        CalculateThreshold(Th_50, 50);
        CalculateThreshold(Th_30, 30);
    }
    if (m_KLineHigh.size() > 55) {
        StrategyCal(pDepthMarketData);
        cerr << "30K��:" << Th_30.m_MaxPriceForHigh << "," << Th_30.m_MaxPriceForLow << "," << Th_30.m_MinPriceForHigh
             << "," << Th_30.m_MinPriceForLow << endl;
        cerr << "50K��:" << Th_50.m_MaxPriceForHigh << "," << Th_50.m_MaxPriceForLow << "," << Th_50.m_MinPriceForHigh
             << "," << Th_50.m_MinPriceForLow << endl;
        cerr << "�����ʽ�" << m_MoneyAvailable;

        RiskCal();
    }

    /*
    TThostFtdcInstrumentIDType    instId;//��Լ,��Լ�����ڽṹ�����Ѿ�����
    TThostFtdcDirectionType       dir;//����,'0'��'1'��
    TThostFtdcCombOffsetFlagType  kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
    TThostFtdcPriceType           price;//�۸�0���м�,��������֧��
    TThostFtdcVolumeType          vol;//����


    if(m_futureData_vec.size() >= 70)
    {

        //�ֲֲ�ѯ�����в�λ����
        if(strcmp(pDepthMarketData->InstrumentID, m_instId) == 0)
        {
            if(m_futureData_vec.size() % 20 == 0)
            {
                if(TDSpi_stgy->SendHolding_long(m_instId) + TDSpi_stgy->SendHolding_short(m_instId) < 3)//��չ�����3��
                {
                    //����
                    if(m_futureData_vec[m_futureData_vec.size()-1].new1 - m_futureData_vec[m_futureData_vec.size()-70].new1 >= 0.6)
                    {
                        strcpy_s(instId, m_instId);
                        dir = '0';
                        strcpy_s(kpp, "0");
                        price = pDepthMarketData->LastPrice + 3;
                        vol = 1;

                        if(m_allow_open == true)
                        {
                            TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);

                        }
                    }
                    //����
                    else if(m_futureData_vec[m_futureData_vec.size()-70].new1 - m_futureData_vec[m_futureData_vec.size()-1].new1 >= 0.6)
                    {
                        strcpy_s(instId, m_instId);
                        dir = '1';
                        strcpy_s(kpp, "0");
                        price = pDepthMarketData->LastPrice - 3;
                        vol = 1;
                        if(m_allow_open == true)
                        {
                            TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);

                        }
                    }

                }
            }

        }*/


    //ǿƽ���гֲ�
    /*if(m_futureData_vec.size() % 140 == 0)
        TDSpi_stgy->ForceClose();*/


}


//�����Ƿ�������
void Strategy::set_allow_open(bool x) {
    m_allow_open = x;

    if (m_allow_open == true) {
        cerr << "��ǰ���ý����������" << endl;


    } else if (m_allow_open == false) {
        cerr << "��ǰ���ý������ֹ����" << endl;

    }

}


//�������ݵ�txt��csv
void Strategy::SaveDataTxtCsv(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    //�������鵽txt
    string date = pDepthMarketData->TradingDay;
    string time = pDepthMarketData->UpdateTime;
    double buy1price = pDepthMarketData->BidPrice1;
    int buy1vol = pDepthMarketData->BidVolume1;
    double new1 = pDepthMarketData->LastPrice;
    double sell1price = pDepthMarketData->AskPrice1;
    int sell1vol = pDepthMarketData->AskVolume1;
    double vol = pDepthMarketData->Volume;
    double openinterest = pDepthMarketData->OpenInterest;//�ֲ���



    string instId = pDepthMarketData->InstrumentID;

    //����date�ĸ�ʽ
    string a = date.substr(0, 4);
    string b = date.substr(4, 2);
    string c = date.substr(6, 2);

    string date_new = a + "-" + b + "-" + c;


    ofstream fout_data("output/" + instId + "_" + date + ".txt", ios::app);

    fout_data << date_new << "," << time << "," << buy1price << "," << buy1vol << "," << new1 << "," << sell1price
              << "," << sell1vol << "," << vol << "," << openinterest << endl;

    fout_data.close();

    ofstream fout_data_history("input//��ʷK������//K������.txt", ios::app);

    fout_data_history << date_new << "," << time << "," << buy1price << "," << buy1vol << "," << new1 << ","
                      << sell1price << "," << sell1vol << "," << vol << "," << openinterest << endl;

    fout_data_history.close();


    ofstream fout_data_csv("output/" + instId + "_" + date + ".csv", ios::app);

    fout_data_csv << date_new << "," << time << "," << buy1price << "," << buy1vol << "," << new1 << "," << sell1price
                  << "," << sell1vol << "," << vol << "," << openinterest << endl;

    fout_data_csv.close();

}


//�������ݵ�vector
void Strategy::SaveDataVec(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    string date = pDepthMarketData->TradingDay;
    string time = pDepthMarketData->UpdateTime;
    double buy1price = pDepthMarketData->BidPrice1;
    int buy1vol = pDepthMarketData->BidVolume1;
    double new1 = pDepthMarketData->LastPrice;
    double sell1price = pDepthMarketData->AskPrice1;
    int sell1vol = pDepthMarketData->AskVolume1;
    double vol = pDepthMarketData->Volume;
    double openinterest = pDepthMarketData->OpenInterest;//�ֲ���


    futureData.date = date;
    futureData.time = time;
    futureData.buy1price = buy1price;
    futureData.buy1vol = buy1vol;
    futureData.new1 = new1;
    futureData.sell1price = sell1price;
    futureData.sell1vol = sell1vol;
    futureData.vol = vol;
    futureData.openinterest = openinterest;

    m_futureData_vec.push_back(futureData);

}


void Strategy::set_instMessage_map_stgy(map<string, CThostFtdcInstrumentField *> &instMessage_map_stgy) {
    m_instMessage_map_stgy = instMessage_map_stgy;
    cerr << "�յ���Լ����:" << m_instMessage_map_stgy.size() << endl;

}


//�����˻���ӯ����Ϣ
void Strategy::CalculateEarningsInfo(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    //���º�Լ�����¼ۣ�û�гֲ־Ͳ���Ҫ���£��гֲֵģ����ǽ��׵ĺ�ԼҲҪ���¡�Ҫ�ȼ���ӯ����Ϣ�ټ�������߼�

    //�жϸú�Լ�Ƿ��гֲ�
    if (TDSpi_stgy->send_trade_message_map_KeyNum(pDepthMarketData->InstrumentID) > 0)
        TDSpi_stgy->setLastPrice(pDepthMarketData->InstrumentID, pDepthMarketData->LastPrice);


    //�����˻��ĸ���ӯ��,�����ּ���
    m_openProfit_account = TDSpi_stgy->sendOpenProfit_account(pDepthMarketData->InstrumentID,
                                                              pDepthMarketData->LastPrice);

    //�����˻���ƽ��ӯ��
    m_closeProfit_account = TDSpi_stgy->sendCloseProfit();

    cerr << " ƽ��ӯ��:" << m_closeProfit_account << ",����ӯ��:" << m_openProfit_account << "��ǰ��Լ:"
         << pDepthMarketData->InstrumentID << " ���¼�:" << pDepthMarketData->LastPrice << " ʱ��:"
         << pDepthMarketData->UpdateTime << endl;//double����Ϊ0��ʱ�����-1.63709e-010����С��0�ģ���+1���ֵ��1

    TDSpi_stgy->printTrade_message_map();

}


//��ȡ��ʷ����
void Strategy::GetHistoryData() {
    vector<string> data_fileName_vec;
    string filename;
    Store_fileName("input/��ʷK������", data_fileName_vec);

    cout << "��ʷK���ı���:" << data_fileName_vec.size() << endl;

    for (vector<string>::iterator iter = data_fileName_vec.begin(); iter != data_fileName_vec.end(); iter++) {
        cout << *iter << endl;
        ReadDatas(*iter, history_data_vec);

    }

    cout << "K������:" << history_data_vec.size() << endl;

}


void Strategy::CalculateThreshold(Thereshold &th, int KLineCount) {
    if (KLineCount > m_KLineHigh.size() + 1)
        return;

    int index_last = m_KLineHigh.size() - 2;
    double maxforhigh = m_KLineHigh[index_last];
    double maxforlow = m_KLineLow[index_last];
    double minforhigh = m_KLineHigh[index_last];
    double minforlow = m_KLineLow[index_last];

    for (int i = 1; i < KLineCount; i++) {
        if (maxforhigh < m_KLineHigh[index_last - i])
            maxforhigh = m_KLineHigh[index_last - i];
        if (minforhigh > m_KLineHigh[index_last - i])
            minforhigh = m_KLineHigh[index_last - i];
        if (maxforlow < m_KLineLow[index_last - i])
            maxforlow = m_KLineLow[index_last - i];
        if (minforlow > m_KLineLow[index_last - i])
            minforlow = m_KLineLow[index_last - i];
    }

    th.m_MaxPriceForHigh = maxforhigh;
    th.m_MaxPriceForLow = maxforlow;
    th.m_MinPriceForHigh = minforhigh;
    th.m_MinPriceForLow = minforlow;
}

bool Strategy::DrawKLine(FutureData CurData, int index, int max) {
    if (index % KXIAN_SHULIANG != 0) {
        if (m_KLineHigh[m_KLineHigh.size() - 1] < CurData.new1)
            m_KLineHigh[m_KLineHigh.size() - 1] = CurData.new1;
        if (m_KLineLow[m_KLineLow.size() - 1] > CurData.new1)
            m_KLineLow[m_KLineLow.size() - 1] = CurData.new1;
        return false;
    } else {
        m_KLineHigh.push_back(CurData.new1);
        m_KLineLow.push_back(CurData.new1);

        if (m_KLineHigh.size() > max + 1) {
            m_KLineHigh.erase(m_KLineHigh.begin());
            m_KLineLow.erase(m_KLineLow.begin());
        }
        //cerr<<"K��1��"<<m_KLineHigh[0]<<"K��1��"<<m_KLineLow[0]<<endl;
        //if(m_KLineHigh.size() > 1)
        //	cerr<<"K��ĩ��"<<m_KLineHigh[m_KLineHigh.size() - 2]<<"K��ĩ��"<<m_KLineLow[m_KLineLow.size() - 2]<<endl;
        //cerr<<"K������"<<m_KLineHigh.size()<<endl;
        return true;
    }
}

bool Strategy::DrawMA(FutureData CurData, int index, int rate, int max) {
    if (index == 0 || index % rate != 0)
        return false;
    m_MALine.push_back(CurData.new1);
    if (m_MALine.size() > max) {
        m_MALine.erase(m_MALine.begin());
        //���߻��꣬�����ֵ�� To Do��Ӧ�ý������õ����ĺ���д���Ա�����չ
        m_MAThreshold = 0.00;
        for (int i = 0; i < m_MALine.size(); i++) {
            m_MAThreshold += m_MALine[i];
        }
        m_MAThreshold = m_MAThreshold / max;
        return true;
    }
    return false;
}

void Strategy::StrategyCal(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    TThostFtdcInstrumentIDType instId;//��Լ,��Լ�����ڽṹ�����Ѿ�����
    TThostFtdcDirectionType dir;//����,'0'��'1'��
    TThostFtdcCombOffsetFlagType kpp;//��ƽ��"0"����"1"ƽ,"3"ƽ��
    TThostFtdcPriceType price;//�۸�0���м�,��������֧��
    TThostFtdcVolumeType vol;//����

    int c1 = AllLongPosition.size();
    int c2 = AllShortPosition.size();
    for (int i = 0; i < AllLongPosition.size(); i++) {
        if (AllLongPosition[i]->Volume == 0)
            c1--;
    }
    for (int i = 0; i < AllShortPosition.size(); i++) {
        if (AllShortPosition[i]->Volume == 0)
            c2--;
    }

    cerr << "����" << m_MAThreshold << endl;


    if (m_MoneyAvailable == 0)
        UpdateAvailable();

    if (m_futureData_vec.size() == 0)
        return;

    //����Ƿ���δ������ɵı���
    vector<CThostFtdcOrderField *> orderlist = TDSpi_stgy->GetOrderList();
    cerr << "orderlist:" << orderlist.size() << endl;

    //To do: ����orderlist���ޱ�Ҫ��ļ��
    if (orderlist.size() > 0) {
        int i = orderlist.size();

        for (vector<CThostFtdcOrderField *>::iterator iter = orderlist.begin(); iter != orderlist.end(); iter++) {
            if ((*iter)->OrderStatus == '1' || (*iter)->OrderStatus == '3' || (*iter)->OrderStatus == '4' ||
                (*iter)->OrderStatus == 'a') {
                cerr << "����δ�����걨��" << endl;
                return;
            }
        }
    }

    //ƽ���
    if (m_futureData_vec[m_futureData_vec.size() - 1].new1 < Th_30.m_MinPriceForLow) {
        int exsitedVol = TDSpi_stgy->SendHolding_long(m_instId);
        int ydVol = TDSpi_stgy->SendHolding_long_YD(m_instId);
        if (exsitedVol > 0) {
            strcpy(instId, m_instId);
            dir = '1';
            price = pDepthMarketData->LastPrice;

            if (ydVol > 0) {
                strcpy(kpp, "1");
                vol = ydVol;
            } else {
                strcpy(kpp, "3");
                vol = exsitedVol - ydVol;
            }

            if (m_allow_open == true) {
                TDSpi_stgy->ForceClose();
                //TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                cerr << "minpriceforlow" << Th_30.m_MinPriceForLow << "new1" << futureData.new1 << "close-buy" << endl;
                PrintTradeInfo("pingduo", "5");
                sleep(1000);
                UpdateAvailable();
            }
        }
    }
    //ƽ�ղ�
    if (m_futureData_vec[m_futureData_vec.size() - 1].new1 > Th_30.m_MaxPriceForHigh) {
        int exsitedVol = TDSpi_stgy->SendHolding_short(m_instId);
        int ydVol = TDSpi_stgy->SendHolding_short_YD(m_instId);
        if (exsitedVol > 0) {
            strcpy(instId, m_instId);
            dir = '0';
            price = pDepthMarketData->LastPrice;
            if (ydVol > 0) {
                strcpy(kpp, "1");
                vol = ydVol;
            } else {
                strcpy(kpp, "3");
                vol = exsitedVol - ydVol;
            }

            if (m_allow_open == true) {
                //TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);
                TDSpi_stgy->ForceClose();
                cerr << "maxpriceforhigh" << Th_30.m_MaxPriceForHigh << "new1" << futureData.new1 << "close-sell"
                     << endl;
                PrintTradeInfo("pingkong", "5");
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                sleep(1000);
                UpdateAvailable();
            }
        }
    }

    //�����
    if (m_futureData_vec[m_futureData_vec.size() - 1].new1 > Th_50.m_MaxPriceForHigh) {
        if (TDSpi_stgy->SendHolding_long(m_instId) + TDSpi_stgy->SendHolding_short(m_instId) == 0) {
            strcpy(instId, m_instId);
            dir = '0';
            strcpy(kpp, "0");
            price = pDepthMarketData->LastPrice; //���ּ۸�
            double money = m_MoneyAvailable * KAICANG_BILI;
            vol = int(money / price);

            //�����ж�
            if (m_MAThreshold == 0 && price < m_MAThreshold)
                return;

            if (m_allow_open == true) {
                TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);
                cerr << "maxpriceforhigh" << Th_50.m_MaxPriceForHigh << "new1" << futureData.new1 << "es-buy" << endl;
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                cerr << "kaiduocang success" << endl;
                m_dThreshold_zhiying = m_futureData_vec[m_futureData_vec.size() - 1].new1; //����ֹӯ��ʼ�۸�
                PrintTradeInfo("kaiduo", "5");
                sleep(1000);
                UpdateAvailable();
            }
        }
    }
    //���ղ�
    if (m_futureData_vec[m_futureData_vec.size() - 1].new1 < Th_50.m_MinPriceForLow) {
        if (TDSpi_stgy->SendHolding_long(m_instId) + TDSpi_stgy->SendHolding_short(m_instId) == 0) {
            strcpy(instId, m_instId);
            dir = '1';
            strcpy(kpp, "0");
            price = pDepthMarketData->LastPrice /* + 5 */;//���ּ۸�
            double money = m_MoneyAvailable * KAICANG_BILI;
            vol = int(money / price);

            //�����ж�
            if (m_MAThreshold == 0 && price > m_MAThreshold)
                return;

            if (m_allow_open == true) {
                TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);

                cerr << "minpriceforlow" << Th_50.m_MinPriceForLow << "new1" << futureData.new1 << "es-sell" << endl;
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();

                m_dThreshold_zhiying = m_futureData_vec[m_futureData_vec.size() - 1].new1; //����ֹӯ��ʼ�۸�
                PrintTradeInfo("kaikong", "5");
                sleep(1000);
                UpdateAvailable();
            }
        }

    }

    //___________________________________
}

//ֹӯֹ��
void Strategy::RiskCal() {
    if (TDSpi_stgy->SendHolding_long(m_instId) + TDSpi_stgy->SendHolding_long_YD(m_instId) > 0) {
        for (int i = 0; i < AllLongPosition.size(); i++) {
            if (AllLongPosition[i]->Volume == 0)
                continue;
            double threshold = AllLongPosition[i]->Price * (1 - ZHISUN_DUO);
            //ֹ��
            if (futureData.new1 < threshold) {
                TDSpi_stgy->ForceClose();
                PrintTradeInfo("zhisun", "all");
                UpdateAvailable();
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                return;
                /*
                cerr<<AllLongPosition[i]->TradingDay<<AllLongPosition[i]->TradeTime<<endl;
                TDSpi_stgy->ReqOrderInsert(m_instId,'1',"3",futureData.new1,AllLongPosition[i]->Volume);*/
            }
            //ֹӯ
            if (futureData.new1 > AllLongPosition[i]->Price * (1 + LIRUNLV) && futureData.new1 > m_dThreshold_zhiying)
                m_dThreshold_zhiying = futureData.new1;
            if (futureData.new1 < m_dThreshold_zhiying * (1 - ZHIYINGLV)) {
                TDSpi_stgy->ForceClose();
                PrintTradeInfo("zhiying", "all");
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                cerr << futureData.new1 << threshold << "ֹ�����" << endl;
                UpdateAvailable();
            }

        }
    }
    //ֹ��
    if (TDSpi_stgy->SendHolding_short(m_instId) + TDSpi_stgy->SendHolding_short_YD(m_instId) > 0) {
        for (int i = 0; i < AllShortPosition.size(); i++) {
            if (AllShortPosition[i]->Volume == 0)
                continue;
            double threshold = AllShortPosition[i]->Price * (1 + ZHISUN_KONG);
            if (futureData.new1 > threshold) {
                TDSpi_stgy->ForceClose();
                PrintTradeInfo("zhisun", "5");
                cerr << futureData.new1 << threshold << "ֹ�����" << endl;
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                UpdateAvailable();
                return;
                /*
                cerr<<AllShortPosition[i]->TradingDay<<AllShortPosition[i]->TradeTime<<endl;
                TDSpi_stgy->ReqOrderInsert(m_instId,'0',"3",futureData.new1,AllShortPosition[i]->Volume);*/
            }
            //ֹӯ
            if (futureData.new1 < AllShortPosition[i]->Price * (1 - LIRUNLV) && futureData.new1 < m_dThreshold_zhiying)
                m_dThreshold_zhiying = futureData.new1;
            if (futureData.new1 > m_dThreshold_zhiying * (1 + ZHIYINGLV)) {
                TDSpi_stgy->ForceClose();
                PrintTradeInfo("zhiying", "5");
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                UpdateAvailable();
            }
        }
    }
}

void Strategy::PrintTradeInfo(string dir, string vol) {
    string instId = m_instId;

    ofstream fout_data_csv("output/" + instId + "trade" + "_" + ".csv", ios::app);

    fout_data_csv << futureData.time << "," << futureData.new1 << "," << dir << "," << vol << endl;

    fout_data_csv.close();
}

void Strategy::GetKLineFromHistory() {
    int count = history_data_vec.size();
    cerr << "count" << count << endl;
    if (count >= 55 * KXIAN_SHULIANG) {
        //count = 55 * KXIAN_SHULIANG;
    }
    for (int i = 0; i < count; i++) {
        FutureData newdata;
        newdata.new1 = history_data_vec[count - 1 - i].new1;
        if (DrawKLine(newdata, i, 50) && m_KLineHigh.size() > 50) {
            CalculateThreshold(Th_50, 50);
            CalculateThreshold(Th_30, 30);
            break;
        }
        cerr << m_KLineHigh.size();
    }

    //������
    for (int i = 0; i < count; i++) {
        FutureData newdata;
        newdata.new1 = history_data_vec[count - 1 - i].new1;
        if (DrawMA(newdata, i, JUNXIAN_KXIANSHULIANG, JUNXIAN_GENSHU))
            break;
    }

    //��ȡ�ֲ���Ϣ
    AllLongPosition = TDSpi_stgy->GetAllLongPosition();
    AllShortPosition = TDSpi_stgy->GetAllShortPosition();

    if (AllLongPosition.size() > 0)
        m_dThreshold_zhiying = AllLongPosition[0]->Price;
    if (AllShortPosition.size() > 0)
        m_dThreshold_zhiying = AllShortPosition[0]->Price;

}

void Strategy::UpdateAvailable() {
    TDSpi_stgy->ReqQryTradingAccount();
    m_MoneyAvailable = (double) TDSpi_stgy->GetAccountAvailable();
}

void Strategy::UpdatePosition() {
    AllLongPosition = TDSpi_stgy->GetAllLongPosition();
    AllShortPosition = TDSpi_stgy->GetAllShortPosition();
}