//
// Created by svalov on 12/8/16.
//

#ifndef MOARSTACK_MOARSETTINGSBINDER_H
#define MOARSTACK_MOARSETTINGSBINDER_H

typedef enum{
	FieldType_int,
} FieldType_T;

typedef struct{
	char* Name; // lower case field name
	FieldType_T FieldType;
	void* Offset; // offset in structure
}SettingsBind_T;

//make binding macro

#define MAKENAME(strct, field) (#strct "." #field)
#define MAKEOFFSET(strct, field) ((void*)&(((strct*)NULL)->field))
#define BINDINGMAKE(b, s, f, t) bindingMake(b, MAKENAME(s,f), MAKEOFFSET(s, f), t);
//__BEGIN_DECLS
//
extern void bindingFreeName(SettingsBind_T* binding);
extern int bindingMake(SettingsBind_T* binding, char* name, void* offset, FieldType_T type);

//__END_DECLS

#endif //MOARSTACK_MOARSETTINGSBINDER_H
