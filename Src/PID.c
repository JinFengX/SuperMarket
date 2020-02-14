#include "PID.h"

struct PID
{
    float P;
    float I;
    float D;
};
struct PID dis_PID = {6000, 0, 0};

//参数：设定值，当前值，清除历史值（1清除，0不清除）
#define MAX ratedSpd  //限制最大转速
#define MIN -ratedSpd //限制最小转速
float Motor_PID(float set, float now, uint8_t clear)
{
    float error = 0;
    float result = 0;
    static float lastError = 0; //上次误差

    if (clear == 0)
    {
        error = set - now;
        result = dis_PID.P * error + dis_PID.D * (error - lastError); //PD控制算法
        lastError = error;                                            //储存历史值
    }
    else
        lastError = 0;

    if (result > MAX)
        result = MAX;
    if (result < MIN)
        result = MIN;

    return -result;
}
