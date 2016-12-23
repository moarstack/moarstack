//
// Created by svalov on 10/11/16.
//

#include <moarRouting.h>
#include <memory.h>
#include <funcResults.h>
#include <moarSettingsBinder.h>

int routeAddrFromStr(char* address, RouteAddr_T* routeAddr){
	if(NULL == address || NULL == routeAddr)
		return FUNC_RESULT_FAILED_ARGUMENT;
	RouteAddr_T intAddress = {0};
	//int res = bindingSet_ByteArray(routeAddr, &intAddress, sizeof(RouteAddr_T));
	int res = bindingSet_ByteArray(&intAddress, address, sizeof(RouteAddr_T));
	*routeAddr = intAddress;
	return res;
}


bool routeAddrEqualPtr(const RouteAddr_T* first, const RouteAddr_T* second){
	if(NULL == first || NULL == second)
		return false;
	int res = memcmp(first, second, sizeof(RouteAddr_T));
	return 0==res;
}