#include<angle.h>
#include<MA.h>
#include<line.h>
#define moter_max 5              //移動平均で使う配列の大きさ

class moter{
    public:
        moter();
        void moveMoter(angle ang,int val,double ac_val,int stop_flag,LINE line);
        void moter_0();
    private:
        const int ena[4] = {0,2,4,28};
        const int pah[4] = {1,3,5,29};
        double mSin[4] = {1,1,-1,-1};  //行列式のsinの値
        double mCos[4] = {1,-1,-1,1};  //行列式のcosの値
        MA Moter[4];
};