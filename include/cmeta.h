#pragma once

#include <stdlib.h>
#include <stddef.h>  /* size_t offsetof(type, member); */
#include <stdint.h>
#include <stdbool.h>

typedef struct cmeta_field_s cmeta_field_t;
typedef struct cmeta_struct_s cmeta_struct_t;
typedef const cmeta_struct_t * cmeta_type;

struct cmeta_field_s {
	const char *    name;
	cmeta_type      type;
	const size_t    offset;
	const int       isPointer;
	const size_t    arrSize;
};

struct cmeta_struct_s {
	const char *	name;
	const size_t	size;
	const cmeta_field_t * fields;
	const size_t	fieldsSize;
};

#define __LEN(X) (sizeof(X)/sizeof(X[0]))
#define __CMETA_FIELDS(CMETATYPENAME) __ ## CMETATYPENAME ## __fields

#define CMETA_TYPE(TYPENAME, TYPE)  \
    static const cmeta_struct_t TYPENAME = {    \
        .name       = #TYPENAME,    \
        .size       = sizeof(TYPE), \
        .fields     = NULL,         \
        .fieldsSize = 0,            \
    }

#define CMETA_STRUCT(CLASSNAME, TYPE, ...)						\
	static const cmeta_field_t __CMETA_FIELDS(CLASSNAME)[] =	__VA_ARGS__;	\
	static const cmeta_struct_t CLASSNAME = { 					\
		.name 		= #CLASSNAME,								\
		.size 		= sizeof(TYPE),								\
		.fields 	= __CMETA_FIELDS(CLASSNAME), 				\
		.fieldsSize = __LEN(__CMETA_FIELDS(CLASSNAME)), 		\
	}

CMETA_TYPE(CBOOLEAN, bool);
CMETA_TYPE(CINTEGER, int);
CMETA_TYPE(CDOUBLE, double);
CMETA_TYPE(CSTRING, char[1]);

#define CMETA(TYPE, FIELD, CMETATYPE) \
	{ #FIELD, & CMETATYPE, offsetof(TYPE, FIELD), 0, 0 }

#define CMETA_STR(TYPE, FIELD, LENGTH) \
	{ #FIELD, & CSTRING, offsetof(TYPE, FIELD), 0, LENGTH }

#define CMETA_STR_PTR(TYPE, FIELD) \
	{ #FIELD, & CSTRING, offsetof(TYPE, FIELD), 1, 0 }

#define CMETA_OBJ(TYPE, FIELD, CMETATYPE) \
	{ #FIELD, & CMETATYPE, offsetof(TYPE, FIELD), 0, 0 }

#define CMETA_OBJ_PTR(TYPE, FIELD, CMETATYPE) \
	{ #FIELD, & CMETATYPE, offsetof(TYPE, FIELD), 1, 0 }

#define CMETA_ARR(TYPE, FIELD, CMETATYPE, ARRLEN) \
	{ #FIELD, & CMETATYPE, offsetof(TYPE, FIELD), 0, ARRLEN}

#define CMETA_OFFSET(PTR, OFFSET) ((void *)((char *)PTR + OFFSET))
#define VALUE_POINTER(POINTER) ((void *)(*(intptr_t *)POINTER))

void cmeta_struct_print(const cmeta_struct_t * metaClass);
const char * cmeta_toString(const cmeta_struct_t * meta);

int cmeta_type_eq(const cmeta_struct_t * a, const cmeta_struct_t * b);
int cmeta_isBasicType(const cmeta_field_t * field);
int cmeta_isObject(const cmeta_field_t * field);
int cmeta_isArray(const cmeta_field_t * field);

const cmeta_field_t * cmeta_struct_getField(const cmeta_struct_t * meta, const char * fieldName);

const char * cmeta_getString(void * obj, const cmeta_struct_t * meta, const char * fieldName);
void cmeta_setString(void * obj, const cmeta_struct_t * meta, const char * fieldName, const char * value);

bool cmeta_getBoolean(void * obj, const cmeta_struct_t * meta, const char * fieldName);
void cmeta_setBoolean(void * obj, const cmeta_struct_t * meta, const char * fieldName, bool value);

int cmeta_getInteger(void * obj, const cmeta_struct_t * meta, const char * fieldName);
void cmeta_setInteger(void * obj, const cmeta_struct_t * meta, const char * fieldName, int value);

double cmeta_getDouble(void * obj, const cmeta_struct_t * meta, const char * fieldName);
void cmeta_setDouble(void * obj, const cmeta_struct_t * meta, const char * fieldName, double value);

void * cmeta_getObject(void * obj, const cmeta_struct_t * meta, const char * fieldName);
void cmeta_setObject(void * obj, const cmeta_struct_t * meta, const char * fieldName, void * value);

int cmeta_getArraySize(const cmeta_struct_t * meta, const char * fieldName);
void * cmeta_getArrayItem(void * obj, const cmeta_struct_t * meta, const char * fieldName, int index);
void cmeta_setArrayItem(void * obj, const cmeta_struct_t * meta, const char * fieldName, int index, void * value);
