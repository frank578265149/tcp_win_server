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

    //计算账户的盈亏信息
    CalculateEarningsInfo(pDepthMarketData);


    if (strcmp(pDepthMarketData->InstrumentID, m_instId) == 0) {
        //cerr<<"策略收到行情:"<<pDepthMarketData->InstrumentID<<","<<pDepthMarketData->TradingDay<<","<<pDepthMarketData->UpdateTime<<",最新价:"<<pDepthMarketData->LastPrice<<",涨停价:"<<pDepthMarketData->UpperLimitPrice<<",跌停价:"<<pDepthMarketData->LowerLimitPrice<<endl;

        //保存数据到vector
        SaveDataVec(pDepthMarketData);

        //保存数据到txt和csv
        SaveDataTxtCsv(pDepthMarketData);

        //撤单追单,撤单成功后再追单
        TDSpi_stgy->CancelOrder(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice);

        if (!m_allow_open)
            return;

        //开仓平仓（策略主逻辑的计算）
        StrategyCalculate(pDepthMarketData);


    }


}


//设置交易的合约代码
void Strategy::setInstId(string instId) {
    strcpy(m_instId, instId.c_str());

}


//策略主逻辑的计算，70条行情里涨了0.6元，则做多，下跌则做空
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
        cerr << "30K线:" << Th_30.m_MaxPriceForHigh << "," << Th_30.m_MaxPriceForLow << "," << Th_30.m_MinPriceForHigh
             << "," << Th_30.m_MinPriceForLow << endl;
        cerr << "50K线:" << Th_50.m_MaxPriceForHigh << "," << Th_50.m_MaxPriceForLow << "," << Th_50.m_MinPriceForHigh
             << "," << Th_50.m_MinPriceForLow << endl;
        cerr << "可用资金" << m_MoneyAvailable;

        RiskCal();
    }

    /*
    TThostFtdcInstrumentIDType    instId;//合约,合约代码在结构体里已经有了
    TThostFtdcDirectionType       dir;//方向,'0'买，'1'卖
    TThostFtdcCombOffsetFlagType  kpp;//开平，"0"开，"1"平,"3"平今
    TThostFtdcPriceType           price;//价格，0是市价,上期所不支持
    TThostFtdcVolumeType          vol;//数量


    if(m_futureData_vec.size() >= 70)
    {

        //持仓查询，进行仓位控制
        if(strcmp(pDepthMarketData->InstrumentID, m_instId) == 0)
        {
            if(m_futureData_vec.size() % 20 == 0)
            {
                if(TDSpi_stgy->SendHolding_long(m_instId) + TDSpi_stgy->SendHolding_short(m_instId) < 3)//多空共满仓3手
                {
                    //做多
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
                    //做空
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


    //强平所有持仓
    /*if(m_futureData_vec.size() % 140 == 0)
        TDSpi_stgy->ForceClose();*/


}


//设置是否允许开仓
void Strategy::set_allow_open(bool x) {
    m_allow_open = x;

    if (m_allow_open == true) {
        cerr << "当前设置结果：允许开仓" << endl;


    } else if (m_allow_open == false) {
        cerr << "当前设置结果：禁止开仓" << endl;

    }

}


//保存数据到txt和csv
void Strategy::SaveDataTxtCsv(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    //保存行情到txt
    string date = pDepthMarketData->TradingDay;
    string time = pDepthMarketData->UpdateTime;
    double buy1price = pDepthMarketData->BidPrice1;
    int buy1vol = pDepthMarketData->BidVolume1;
    double new1 = pDepthMarketData->LastPrice;
    double sell1price = pDepthMarketData->AskPrice1;
    int sell1vol = pDepthMarketData->AskVolume1;
    double vol = pDepthMarketData->Volume;
    double openinterest = pDepthMarketData->OpenInterest;//持仓量



    string instId = pDepthMarketData->InstrumentID;

    //更改date的格式
    string a = date.substr(0, 4);
    string b = date.substr(4, 2);
    string c = date.substr(6, 2);

    string date_new = a + "-" + b + "-" + c;


    ofstream fout_data("output/" + instId + "_" + date + ".txt", ios::app);

    fout_data << date_new << "," << time << "," << buy1price << "," << buy1vol << "," << new1 << "," << sell1price
              << "," << sell1vol << "," << vol << "," << openinterest << endl;

    fout_data.close();

    ofstream fout_data_history("input//历史K线数据//K线行情.txt", ios::app);

    fout_data_history << date_new << "," << time << "," << buy1price << "," << buy1vol << "," << new1 << ","
                      << sell1price << "," << sell1vol << "," << vol << "," << openinterest << endl;

    fout_data_history.close();


    ofstream fout_data_csv("output/" + instId + "_" + date + ".csv", ios::app);

    fout_data_csv << date_new << "," << time << "," << buy1price << "," << buy1vol << "," << new1 << "," << sell1price
                  << "," << sell1vol << "," << vol << "," << openinterest << endl;

    fout_data_csv.close();

}


//保存数据到vector
void Strategy::SaveDataVec(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    string date = pDepthMarketData->TradingDay;
    string time = pDepthMarketData->UpdateTime;
    double buy1price = pDepthMarketData->BidPrice1;
    int buy1vol = pDepthMarketData->BidVolume1;
    double new1 = pDepthMarketData->LastPrice;
    double sell1price = pDepthMarketData->AskPrice1;
    int sell1vol = pDepthMarketData->AskVolume1;
    double vol = pDepthMarketData->Volume;
    double openinterest = pDepthMarketData->OpenInterest;//持仓量


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
    cerr << "收到合约个数:" << m_instMessage_map_stgy.size() << endl;

}


//计算账户的盈亏信息
void Strategy::CalculateEarningsInfo(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    //更新合约的最新价，没有持仓就不需要更新，有持仓的，不是交易的合约也要更新。要先计算盈亏信息再计算策略逻辑

    //判断该合约是否有持仓
    if (TDSpi_stgy->send_trade_message_map_KeyNum(pDepthMarketData->InstrumentID) > 0)
        TDSpi_stgy->setLastPrice(pDepthMarketData->InstrumentID, pDepthMarketData->LastPrice);


    //整个账户的浮动盈亏,按开仓价算
    m_openProfit_account = TDSpi_stgy->sendOpenProfit_account(pDepthMarketData->InstrumentID,
                                                              pDepthMarketData->LastPrice);

    //整个账户的平仓盈亏
    m_closeProfit_account = TDSpi_stgy->sendCloseProfit();

    cerr << " 平仓盈亏:" << m_closeProfit_account << ",浮动盈亏:" << m_openProfit_account << "当前合约:"
         << pDepthMarketData->InstrumentID << " 最新价:" << pDepthMarketData->LastPrice << " 时间:"
         << pDepthMarketData->UpdateTime << endl;//double类型为0有时候会是-1.63709e-010，是小于0的，但+1后的值是1

    TDSpi_stgy->printTrade_message_map();

}


//读取历史数据
void Strategy::GetHistoryData() {
    vector<string> data_fileName_vec;
    string filename;
    Store_fileName("input/历史K线数据", data_fileName_vec);

    cout << "历史K线文本数:" << data_fileName_vec.size() << endl;

    for (vector<string>::iterator iter = data_fileName_vec.begin(); iter != data_fileName_vec.end(); iter++) {
        cout << *iter << endl;
        ReadDatas(*iter, history_data_vec);

    }

    cout << "K线条数:" << history_data_vec.size() << endl;

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
        //cerr<<"K线1高"<<m_KLineHigh[0]<<"K线1低"<<m_KLineLow[0]<<endl;
        //if(m_KLineHigh.size() > 1)
        //	cerr<<"K线末高"<<m_KLineHigh[m_KLineHigh.size() - 2]<<"K线末低"<<m_KLineLow[m_KLineLow.size() - 2]<<endl;
        //cerr<<"K线数量"<<m_KLineHigh.size()<<endl;
        return true;
    }
}

bool Strategy::DrawMA(FutureData CurData, int index, int rate, int max) {
    if (index == 0 || index % rate != 0)
        return false;
    m_MALine.push_back(CurData.new1);
    if (m_MALine.size() > max) {
        m_MALine.erase(m_MALine.begin());
        //均线画完，计算均值。 To Do：应该将计算用单独的函数写，以便于扩展
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
    TThostFtdcInstrumentIDType instId;//合约,合约代码在结构体里已经有了
    TThostFtdcDirectionType dir;//方向,'0'买，'1'卖
    TThostFtdcCombOffsetFlagType kpp;//开平，"0"开，"1"平,"3"平今
    TThostFtdcPriceType price;//价格，0是市价,上期所不支持
    TThostFtdcVolumeType vol;//数量

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

    cerr << "均线" << m_MAThreshold << endl;


    if (m_MoneyAvailable == 0)
        UpdateAvailable();

    if (m_futureData_vec.size() == 0)
        return;

    //检查是否有未处理完成的报单
    vector<CThostFtdcOrderField *> orderlist = TDSpi_stgy->GetOrderList();
    cerr << "orderlist:" << orderlist.size() << endl;

    //To do: 减少orderlist中无必要项的检测
    if (orderlist.size() > 0) {
        int i = orderlist.size();

        for (vector<CThostFtdcOrderField *>::iterator iter = orderlist.begin(); iter != orderlist.end(); iter++) {
            if ((*iter)->OrderStatus == '1' || (*iter)->OrderStatus == '3' || (*iter)->OrderStatus == '4' ||
                (*iter)->OrderStatus == 'a') {
                cerr << "尚有未处理完报单" << endl;
                return;
            }
        }
    }

    //平多仓
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
    //平空仓
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

    //开多仓
    if (m_futureData_vec[m_futureData_vec.size() - 1].new1 > Th_50.m_MaxPriceForHigh) {
        if (TDSpi_stgy->SendHolding_long(m_instId) + TDSpi_stgy->SendHolding_short(m_instId) == 0) {
            strcpy(instId, m_instId);
            dir = '0';
            strcpy(kpp, "0");
            price = pDepthMarketData->LastPrice; //开仓价格
            double money = m_MoneyAvailable * KAICANG_BILI;
            vol = int(money / price);

            //均线判断
            if (m_MAThreshold == 0 && price < m_MAThreshold)
                return;

            if (m_allow_open == true) {
                TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);
                cerr << "maxpriceforhigh" << Th_50.m_MaxPriceForHigh << "new1" << futureData.new1 << "es-buy" << endl;
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                cerr << "kaiduocang success" << endl;
                m_dThreshold_zhiying = m_futureData_vec[m_futureData_vec.size() - 1].new1; //设置止盈初始价格
                PrintTradeInfo("kaiduo", "5");
                sleep(1000);
                UpdateAvailable();
            }
        }
    }
    //开空仓
    if (m_futureData_vec[m_futureData_vec.size() - 1].new1 < Th_50.m_MinPriceForLow) {
        if (TDSpi_stgy->SendHolding_long(m_instId) + TDSpi_stgy->SendHolding_short(m_instId) == 0) {
            strcpy(instId, m_instId);
            dir = '1';
            strcpy(kpp, "0");
            price = pDepthMarketData->LastPrice /* + 5 */;//开仓价格
            double money = m_MoneyAvailable * KAICANG_BILI;
            vol = int(money / price);

            //均线判断
            if (m_MAThreshold == 0 && price > m_MAThreshold)
                return;

            if (m_allow_open == true) {
                TDSpi_stgy->ReqOrderInsert(instId, dir, kpp, price, vol);

                cerr << "minpriceforlow" << Th_50.m_MinPriceForLow << "new1" << futureData.new1 << "es-sell" << endl;
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();

                m_dThreshold_zhiying = m_futureData_vec[m_futureData_vec.size() - 1].new1; //设置止盈初始价格
                PrintTradeInfo("kaikong", "5");
                sleep(1000);
                UpdateAvailable();
            }
        }

    }

    //___________________________________
}

//止盈止损
void Strategy::RiskCal() {
    if (TDSpi_stgy->SendHolding_long(m_instId) + TDSpi_stgy->SendHolding_long_YD(m_instId) > 0) {
        for (int i = 0; i < AllLongPosition.size(); i++) {
            if (AllLongPosition[i]->Volume == 0)
                continue;
            double threshold = AllLongPosition[i]->Price * (1 - ZHISUN_DUO);
            //止损
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
            //止盈
            if (futureData.new1 > AllLongPosition[i]->Price * (1 + LIRUNLV) && futureData.new1 > m_dThreshold_zhiying)
                m_dThreshold_zhiying = futureData.new1;
            if (futureData.new1 < m_dThreshold_zhiying * (1 - ZHIYINGLV)) {
                TDSpi_stgy->ForceClose();
                PrintTradeInfo("zhiying", "all");
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                cerr << futureData.new1 << threshold << "止损完成" << endl;
                UpdateAvailable();
            }

        }
    }
    //止损
    if (TDSpi_stgy->SendHolding_short(m_instId) + TDSpi_stgy->SendHolding_short_YD(m_instId) > 0) {
        for (int i = 0; i < AllShortPosition.size(); i++) {
            if (AllShortPosition[i]->Volume == 0)
                continue;
            double threshold = AllShortPosition[i]->Price * (1 + ZHISUN_KONG);
            if (futureData.new1 > threshold) {
                TDSpi_stgy->ForceClose();
                PrintTradeInfo("zhisun", "5");
                cerr << futureData.new1 << threshold << "止损完成" << endl;
                AllLongPosition = TDSpi_stgy->GetAllLongPosition();
                AllShortPosition = TDSpi_stgy->GetAllShortPosition();
                UpdateAvailable();
                return;
                /*
                cerr<<AllShortPosition[i]->TradingDay<<AllShortPosition[i]->TradeTime<<endl;
                TDSpi_stgy->ReqOrderInsert(m_instId,'0',"3",futureData.new1,AllShortPosition[i]->Volume);*/
            }
            //止盈
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

    //画均线
    for (int i = 0; i < count; i++) {
        FutureData newdata;
        newdata.new1 = history_data_vec[count - 1 - i].new1;
        if (DrawMA(newdata, i, JUNXIAN_KXIANSHULIANG, JUNXIAN_GENSHU))
            break;
    }

    //获取持仓信息
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