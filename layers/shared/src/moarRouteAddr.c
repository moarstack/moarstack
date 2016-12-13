//
// Created by svalov on 10/11/16.
//

#include <moarRouting.h>
#include <memory.h>

bool routeAddrEqualPtr(const RouteAddr_T* first, const RouteAddr_T* second){
	if(NULL == first || NULL == second)
		return false;
	int res = memcmp(first, second, sizeof(RouteAddr_T));
	return 0==res;
}