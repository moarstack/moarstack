//
// Created by svalov on 7/20/16.
//

#ifndef MOARSTACK_FUNCRESULTS_H
#define MOARSTACK_FUNCRESULTS_H

#define FUNC_RESULT_SUCCESS						0
#define FUNC_RESULT_FAILED						-1
#define FUNC_RESULT_FAILED_ARGUMENT				-2
#define FUNC_RESULT_FAILED_IO					-3
#define FUNC_RESULT_FAILED_MEM_ALLOCATION		-4
#define FUNC_RESULT_FAILED_NEIGHBORS			-5
#define FUNC_RESULT_FAILED_UNEXPECTED_COMMAND   -6
#define FUNC_RESULT_FAILED_APPID_INUSE          -7

#define FUNC_RESULT_MINIMUM_VALUE	FUNC_RESULT_FAILED_APPID_INUSE  // keep it minimal
#define CHECK_RESULT(r)				do{ int __res = (r); if( FUNC_RESULT_SUCCESS != __res ) return __res; }while( 0 )

#pragma pack(push, 1)

typedef int	FUNC_RESULT;

#pragma pack(pop)


#endif //MOARSTACK_FUNCRESULTS_H

