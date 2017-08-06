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
        //m_allow_open = false; //Ĭ�Ϲر�
        m_allow_open = true; //Ĭ�Ͽ���
        GetHistoryData();
        m_currentIndex = 0;
        m_bIsKLineFinished = false;
        GetKLineFromHistory();
        UpdateAvailable();
        m_MoneyAvailable = 0.00;
    }

    //����ص�������ÿ�յ�һ������ʹ���һ��
    void OnTickData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //���ý��׵ĺ�Լ����
    void setInstId(string instId);

    //�������߼��ļ��㣬70������������0.6Ԫ�������࣬�µ������գ�ϵͳĬ�Ͻ�ֹ���֣����ڽ�������"yxkc"�������֣�
    void StrategyCalculate(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //�����Ƿ�������
    void set_allow_open(bool x);

    //�����������ݵ�vector
    void SaveDataVec(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //�����������ݵ�txt��csv
    void SaveDataTxtCsv(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //���ú�Լ-��Լ��Ϣ�ṹ���map
    void set_instMessage_map_stgy(map<string, CThostFtdcInstrumentField *> &instMessage_map_stgy);

    //�����˻���ӯ����Ϣ
    void CalculateEarningsInfo(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //��ȡ��ʷ����
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

    //������ֵ
    void CalculateThreshold(Thereshold &th, int KLineCount);

    //��K��
    bool DrawKLine(FutureData CurData, int index, int max);

    //���Լ���
    void StrategyCal(CThostFtdcDepthMarketDataField *pDepthMarketData);

    //ֹӯֹ���߼���
    void RiskCal();

    //��ӡ�ɽ���Ϣ
    void PrintTradeInfo(string dir, string vol);

    //��ȡ��ʷK��
    void GetKLineFromHistory();

    //�����˻������ʽ�
    void UpdateAvailable();

    //���³ֲ��б�
    void UpdatePosition();

    //����
    bool DrawMA(FutureData CurData, int index, int rate, int max);

private:

    CtpTraderSpi *TDSpi_stgy;//TDָ��

    TThostFtdcInstrumentIDType m_instId;//��Լ����

    FutureData futureData;//�Զ�����������ݽṹ��

    vector<FutureData> m_futureData_vec;//�����������ݵ�vector,�����ָ����Ҫnew���ٱ���

    bool m_allow_open;//TRUE�����֣�FALSE��ֹ����

    map<string, CThostFtdcInstrumentField *> m_instMessage_map_stgy;//�����Լ��Ϣ��map,ͨ��set_instMessage_map_stgy()������TD����

    double m_openProfit_account;//�����˻��ĸ���ӯ��,�����ּ���

    double m_closeProfit_account;//�����˻���ƽ��ӯ��

    double m_MoneyAvailable; //�˻������ʽ�

    vector<History_data> history_data_vec;//������ʷ���ݵ�vector

    vector<double> m_KLineHigh;
    vector<double> m_KLineLow;

    Thereshold Th_50;
    Thereshold Th_30;

    double m_dThreshold_zhiying;

    int m_currentIndex;
    bool m_bIsKLineFinished; //�Ƿ��������㹻�����K��

    vector<CThostFtdcTradeField *> AllLongPosition;
    vector<CThostFtdcTradeField *> AllShortPosition;

    vector<double> m_MALine;
    double m_MAThreshold;

    //bool HavePosition;
};


#endif //TCP_WIN_STRATEGY_H
