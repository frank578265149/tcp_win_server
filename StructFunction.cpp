//
// Created by frank on 17-8-6.
//
#include<fstream>
#include<io.h>
#include<algorithm>
#include <cstring>

#include "StructFunction.h"

//��ȡ��ʷK��
void ReadDatas(string fileName, vector<History_data> &history_data_vec) {
    History_data stu;
    char dataline[512];//��������
    ifstream file1(fileName, ios::in);    //��ֻ����ʽ����,��ȡԭʼ����
    if (!file1) {
        cout << "no such file!" << endl;
        abort();
    }
    while (file1.getline(dataline, 1024, '\n'))//while��ʼ����ȡһ��1024����
    {
        int len = strlen(dataline);
        char tmp[20];
        for (int i = 0, j = 0, count = 0; i <= len; i++) {
            if (dataline[i] != ',' && dataline[i] != '\0') {
                tmp[j++] = dataline[i];
            } else {
                tmp[j] = '\0';
                count++;
                j = 0;
                switch (count) {
                    case 1:
                        stu.date = tmp;
                        break;
                    case 2:
                        stu.time = tmp;
                        break;
                    case 3:
                        stu.buy1price = atof(tmp);
                        break;
                    case 4:
                        stu.buy1vol = atof(tmp);
                        break;
                    case 5:
                        stu.new1 = atof(tmp);
                        break;
                    case 6:
                        stu.sell1price = atof(tmp);
                        break;
                    case 7:
                        stu.sell1vol = atof(tmp);
                        break;

                        break;
                    case 8:
                        stu.close = atof(tmp);
                        break;

                }
            }
        }
        history_data_vec.push_back(stu);
    }

    file1.close();
}


int Store_fileName(string path, vector<string> &FileName) {
/*    struct _finddata_t fileinfo;
    string in_path;
    string in_name;

    //in_path="d:\\future_data";
    in_path = path;
    string curr = in_path+"\\*.txt";
    long handle;

    if((handle=_findfirst(curr.c_str(),&fileinfo))==-1L)
    {
        cout<<"û���ҵ�ƥ���ļ�!"<<endl;

        return 0;
    }

    else
    {
        do
        {
            in_name=in_path + "\\" +fileinfo.name;
            FileName.push_back(in_name);
        }while(!(_findnext(handle,&fileinfo)));

        _findclose(handle);

        return 0;

    }*/
    return 0;
}