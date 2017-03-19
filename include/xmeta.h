#pragma once

#include <stdlib.h>
#include <cmeta.h>

typedef struct xmeta_field_s xmeta_field_t;
typedef struct xmeta_struct_s xmeta_struct_t;

struct xmeta_field_s {
    const char *    name;
    const char *    fieldName;
    const xmeta_struct_t * type;
    const int       isAttribute;
    const char *    arrItemName;
};

struct xmeta_struct_s {
    const char *            name;
    const cmeta_struct_t *  metaType;
    const xmeta_field_t *   fields;
    const size_t            fieldsSize;
};

#define __XMETA_FIELDS(XMETATYPENAME) __ ## XMETATYPENAME ## __fields

#define XMETA_STRUCT(XTYPENAME, CTYPENAME, ...)                 \
    static const xmeta_field_t __XMETA_FIELDS(XTYPENAME)[] =	__VA_ARGS__;	\
	static const xmeta_struct_t XTYPENAME = { 					\
		.name 		= #XTYPENAME,								\
		.metaType	= & CTYPENAME,								\
		.fields 	= __XMETA_FIELDS(XTYPENAME), 				\
		.fieldsSize = __LEN(__XMETA_FIELDS(XTYPENAME)), 		\
	}

#define AUTO NULL
#define XATTR 1
#define XELEM 0

#define XMETA(FIELD, NAME)  \
    { ((NAME) ? NAME : #FIELD),  #FIELD, NULL, XELEM, NULL }

#define XMETA_ATTR(FIELD, NAME)  \
    { ((NAME) ? NAME : #FIELD),  #FIELD, NULL, XATTR,NULL }

#define XMETA_OBJ(FIELD, XMETATYPE, NAME) \
    { ((NAME) ? NAME : #FIELD), #FIELD, & XMETATYPE, XELEM, NULL }

#define XMETA_ARR(FIELD, XMETATYPE, ITEMNAME, NAME) \
    { ((NAME) ? NAME : #FIELD), #FIELD, & XMETATYPE, XELEM, ITEMNAME }

const char * xmeta_serialize(void * obj, const xmeta_struct_t * xmeta);
const char * xmeta_serialize_root(void * obj, const xmeta_struct_t * xmeta, const char * rootName);
void xmeta_deserialize(void * obj, const xmeta_struct_t * xmeta, const char * xmlStr);
