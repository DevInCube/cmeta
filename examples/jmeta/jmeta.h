#pragma once

#include <stdlib.h>
#include <cmeta.h>

typedef struct jmeta_field_s jmeta_field_t;
typedef struct jmeta_struct_s jmeta_struct_t;

struct jmeta_field_s {
    const char *    name;
    const char *    fieldName;
    const jmeta_struct_t * type;
};

struct jmeta_struct_s {
    const char *            name;
    const cmeta_struct_t *  metaType;
    const jmeta_field_t *   fields;
    const size_t            fieldsSize;
};

#define __JMETA_FIELDS(JMETATYPENAME) __ ## JMETATYPENAME ## __fields

#define JMETA_TYPE_VAR 	static const jmeta_struct_t
#define JMETA_FIELD_VAR static const jmeta_field_t

/**
    @param JTYPENAME - new json meta type name
    @param CTYPENAME - c struct meta type name defined above
    @param ... - a variadic list of jmeta_field_t items
*/
#define JMETA_STRUCT(JTYPENAME, CTYPENAME, ...)                 \
    JMETA_TYPE_VAR __JMETA_FIELDS(JTYPENAME)[] = __VA_ARGS__;	\
	JMETA_FIELD_VAR JTYPENAME = { 					            \
		.name 		= #JTYPENAME,								\
		.metaType	= & CTYPENAME,								\
		.fields 	= __JMETA_FIELDS(JTYPENAME), 				\
		.fieldsSize = __LEN(__JMETA_FIELDS(JTYPENAME)), 		\
	}

#define AUTO NULL

#define JMETA(FIELD, NAME)  \
    { ((NAME) ? NAME : #FIELD),  #FIELD, NULL }
#define JMETA_OBJ(FIELD, JMETATYPE, NAME) \
    { ((NAME) ? NAME : #FIELD), #FIELD, & JMETATYPE }
#define JMETA_ARR(FIELD, JMETATYPE, NAME) \
    { ((NAME) ? NAME : #FIELD), #FIELD, & JMETATYPE }

const char * jmeta_serialize(void * obj, const jmeta_struct_t * jmeta);
void jmeta_deserialize(void * obj, const jmeta_struct_t * jtype, const char * jstr);
