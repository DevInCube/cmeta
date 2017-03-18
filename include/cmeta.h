#pragma once

#include <stdlib.h>
#include <stddef.h>  /* size_t offsetof(type, member); */
#include <stdint.h>
#include <stdbool.h>

typedef struct cmeta_field_s cmeta_field_t;
typedef struct cmeta_struct_s cmeta_struct_t;
typedef struct cmeta_object_s cmeta_object_t;

/**
	@struct cmeta_field_s
	@brief holds metainformation about c struct field
*/
struct cmeta_field_s {
	const char *    name;			/**< struct field name */
	const cmeta_struct_t * type;	/**< field metatype */
	const size_t    offset;			/**< struct field offset */
	const int       isPointer;		/**< for pointer fields */
	const size_t    arrSize;		/**< for array fields */
};

/**
	@struct cmeta_struct_s
	@brief holds metainformation about c struct 
*/
struct cmeta_struct_s {
	const char *	name;
	const size_t	size;
	const cmeta_field_t * fields;
	const size_t	fieldsSize;
};

struct cmeta_object_s {
	const cmeta_struct_t * type;
	const void * ptr;
};

#define __LEN(X) (sizeof((X)) / sizeof((X)[0]))
#define __CMETA_FIELDS(CMETATYPENAME) __ ## CMETATYPENAME ## __fields

#define CMETA_TYPE_VAR 	static const cmeta_struct_t
#define CMETA_FIELD_VAR static const cmeta_field_t

/**
	@param TYPENAME
	@param TYPE
*/
#define CMETA_TYPE(TYPENAME, TYPE)  \
    CMETA_TYPE_VAR TYPENAME = {     \
        .name       = #TYPENAME,    \
        .size       = sizeof(TYPE), \
        .fields     = NULL,         \
        .fieldsSize = 0,            \
    }

/**
	@brief a macro to define meta type for existing c struct
	@param CLASSNAME - a name for meta type
	@param TYPE	- a name of exiting c struct
	@param ... - a variadic list of cmeta_field_t items
*/
#define CMETA_STRUCT(CLASSNAME, TYPE, ...)						\
	CMETA_FIELD_VAR __CMETA_FIELDS(CLASSNAME)[] = __VA_ARGS__;	\
	CMETA_TYPE_VAR CLASSNAME = { 								\
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

void cmeta_struct_print(const cmeta_struct_t *metaClass);
const char * cmeta_toString(const cmeta_struct_t *meta);

int cmeta_type_eq(const cmeta_struct_t *a, const cmeta_struct_t *b);
int cmeta_isBasicType(const cmeta_field_t * field);
int cmeta_isObject(const cmeta_field_t * field);
int cmeta_isArray(const cmeta_field_t * field);

const cmeta_field_t * cmeta_struct_getField(const cmeta_struct_t *meta, const char * fieldName);

const char * cmeta_getString(cmeta_object_t * self, const char * fieldName);
void cmeta_setString(cmeta_object_t * self, const char * fieldName, const char * value);

bool cmeta_getBoolean(cmeta_object_t * self, const char * fieldName);
void cmeta_setBoolean(cmeta_object_t * self, const char * fieldName, bool value);

int cmeta_getInteger(cmeta_object_t * self, const char * fieldName);
void cmeta_setInteger(cmeta_object_t * self, const char * fieldName, int value);

double cmeta_getDouble(cmeta_object_t * self, const char * fieldName);
void cmeta_setDouble(cmeta_object_t * self, const char * fieldName, double value);

void * cmeta_getObject(cmeta_object_t * self, const char * fieldName);
void cmeta_setObject(cmeta_object_t * self, const char * fieldName, void * value);

int cmeta_getArraySize(cmeta_object_t * self, const char * fieldName);
void * cmeta_getArrayItem(cmeta_object_t * self, const char * fieldName, int index);
void cmeta_setArrayItem(cmeta_object_t * self, const char * fieldName, int index, void * value);

/**
	@brief a generalized macro for get-functions
	Note that bools are treated as ints here
*/
#define cmeta_get(SELF, FIELD_NAME, TYPE)  \
	_Generic((TYPE)0,							\
		const char *: cmeta_getString, 			\
			  char *: cmeta_getString, 			\
			  	bool: cmeta_getBoolean,			\
				 int: cmeta_getInteger,			\
			  double: cmeta_getDouble,			\
			 default: cmeta_getObject			\
	)(SELF, FIELD_NAME)

/**
	@brief a generalized macro for set-functions
	Note that bools are treated as ints here
*/
#define cmeta_set(SELF, FIELD_NAME, VAL)	\
	_Generic((VAL),								\
		const char *: cmeta_setString, 			\
			  char *: cmeta_setString, 			\
				 int: cmeta_setInteger,			\
			  double: cmeta_setDouble,			\
			 default: cmeta_setObject			\
	)(SELF, FIELD_NAME, VAL)
