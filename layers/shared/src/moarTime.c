//
// Created by svalov on 8/9/16.
//
#include <moarTime.h>
#include <sys/time.h>
#include <time.h>

moarTime_T timeGetCurrent(){
	struct timeval tv;
	// TODO switch to clock_gettime later
	int res = gettimeofday(&tv, NULL);
	if(0 ==  res){
		moarTime_T seconds = tv.tv_sec;
		moarTime_T curTime = seconds * 1000 + tv.tv_usec / 1000;
		return curTime;
	}
	return (moarTime_T)-1;

}
moarTimeInterval_T timeGetDifference(moarTime_T first, moarTime_T second){
	return first-second;
}
int timeCompare(moarTime_T first, moarTime_T second){
	if(first == second)
		return 0;
	if(first < second)
		return -1;
	else return  1;
}
moarTime_T timeAddInterval(moarTime_T time, moarTimeInterval_T interval){
	return time + interval;
}