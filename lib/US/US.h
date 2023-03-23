#include<Arduino.h>
#include<MA.h>


class us{
    public:
        us();
        int readFar();
    private:
        MA us_ma;
        int pingPin = 32;
};