//
// Created by svalov on 12/8/16.
//

#ifndef MOARSTACK_MOARSETTINGSBINDER_H
#define MOARSTACK_MOARSETTINGSBINDER_H

#include <stdint.h>
#include <hashTable.h>

typedef uint64_t Offset_T;

typedef enum{
	FieldType_int,

	FieldType_char,
	FieldType_uint64_t,
	FieldType_uint32_t,
	FieldType_uint16_t,
	FieldType_uint8_t,
//	FieldType_RouteAddr_t,
//	FieldType_ChannelAddr_t,
//	FieldType_IfaceAddr_t,
//	FieldType_moarTime_t,
} FieldType_T;

typedef struct{
	char* Name; // lower case field name
	FieldType_T FieldType;
	Offset_T Offset; // offset in structure
}SettingsBind_T;

//make binding macro

#define MAKENAME(strct, field) (#strct "." #field)
#define MAKEOFFSET(strct, field) ((Offset_T)&(((strct*)NULL)->field))
#define BINDINGMAKE(b, s, f, t) bindingMake(b, MAKENAME(s,f), MAKEOFFSET(s, f), t)
//__BEGIN_DECLS
//
extern void bindingFreeName(SettingsBind_T* binding);
extern int bindingMake(SettingsBind_T* binding, char* name, Offset_T offset, FieldType_T type);
extern int bindingBind(SettingsBind_T* binding, void* targetStruct, char* val);
extern int bindingBindStruct(hashTable_T* settings, SettingsBind_T* binding, int bindCount, void* targetStruct);
//__END_DECLS

#endif //MOARSTACK_MOARSETTINGSBINDER_H
