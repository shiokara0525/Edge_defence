#include<angle.h>
#include<MA.h>
#include<line.h>
#define moter_max 3              //移動平均で使う配列の大きさ

class moter{
    public:
        moter();
        void moveMoter(angle ang,int val,double ac_val,int stop_flag,LINE line);
        void moveMoter_0(angle ang,int val,double ac_val);
        void moter_0();
        void moter_ac(float);
        int line_val = 2;
    private:
        const int pah[4] = {2,4,6,8};
        const int ena[4] = {3,5,7,9};
        double mSin[4] = {1,1,-1,-1};  //行列式のsinの値
        double mCos[4] = {1,-1,-1,1};  //行列式のcosの値
        MA Moter[4];
};