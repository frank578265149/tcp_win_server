//
// Created by frank on 17-8-6.
//

#ifndef TCP_WIN_STRATEGY_H
#define TCP_WIN_STRATEGY_H

#include "tcp/ThostFtdcMdApi.h"
#include <vector>
#include "StructFunction.h"
#include <string>
#include <iostream>
#include <map>
#include "traderspi.h"

class Strategy {
public:
    Strategy(CtpTraderSpi *TDSpi) : TDSpi_stgy(TDSpi) {
        //m_allow_open = false; //默认关闭
        m_allow_open = true; //默认开仓
        GetHistoryData();
        m_currentIndex = 0;
        m_bIsKLineFinished = false;
        GetKLineFromHistory();
        UpdateAvailable();
        m_MoneyAvailable = 0.00;
    }

    //行情回调函数，每收到一笔行情就触发一次
    void OnTickData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //设置交易的合约代码
    void setInstId(string instId);

    //策略主逻辑的计算，70条行情里涨了0.6元，则做多，下跌则做空（系统默认禁止开仓，可在界面输入"yxkc"来允许开仓）
    void StrategyCalculate(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //设置是否允许开仓
    void set_allow_open(bool x);

    //保存行情数据到vector
    void SaveDataVec(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //保存行情数据到txt和csv
    void SaveDataTxtCsv(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //设置合约-合约信息结构体的map
    void set_instMessage_map_stgy(map<string, CThostFtdcInstrumentField *> &instMessage_map_stgy);

    //计算账户的盈亏信息
    void CalculateEarningsInfo(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //读取历史数据
    void GetHistoryData();

    struct Thereshold {
        double m_MaxPriceForHigh;
        double m_MaxPriceForLow;
        double m_MinPriceForHigh;
        double m_MinPriceForLow;
    };

#define MAXFORHIGH 0
#define MAXFORLOW 1
#define MINFORHIGH 2
#define MINFORLOW 3

    //计算阙值
    void CalculateThreshold(Thereshold &th, int KLineCount);

    //画K线
    bool DrawKLine(FutureData CurData, int index, int max);

    //策略计算
    void StrategyCal(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //止盈止损线计算
    void RiskCal();

    //打印成交信息
    void PrintTradeInfo(string dir, string vol);

    //读取历史K线
    void GetKLineFromHistory();

    //更新账户可用资金
    void UpdateAvailable();

    //更新持仓列表
    void UpdatePosition();

    //均线
    bool DrawMA(FutureData CurData, int index, int rate, int max);

private:

    CtpTraderSpi *TDSpi_stgy;//TD指针

    TThostFtdcInstrumentIDType m_instId;//合约代码

    FutureData futureData;//自定义的行情数据结构体

    vector<FutureData> m_futureData_vec;//保存行情数据的vector,如果用指针需要new后再保存

    bool m_allow_open;//TRUE允许开仓，FALSE禁止开仓

    map<string, CThostFtdcInstrumentField *> m_instMessage_map_stgy;//保存合约信息的map,通过set_instMessage_map_stgy()函数从TD复制

    double m_openProfit_account;//整个账户的浮动盈亏,按开仓价算

    double m_closeProfit_account;//整个账户的平仓盈亏

    double m_MoneyAvailable; //账户可用资金

    vector<History_data> history_data_vec;//保存历史数据的vector

    vector<double> m_KLineHigh;
    vector<double> m_KLineLow;

    Thereshold Th_50;
    Thereshold Th_30;

    double m_dThreshold_zhiying;

    int m_currentIndex;
    bool m_bIsKLineFinished; //是否已生成足够运算的K线

    vector<CThostFtdcTradeField *> AllLongPosition;
    vector<CThostFtdcTradeField *> AllShortPosition;

    vector<double> m_MALine;
    double m_MAThreshold;

    //bool HavePosition;
};


#endif //TCP_WIN_STRATEGY_H
