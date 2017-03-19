#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jmeta.h>
#include <jansson.h>

static json_t * _jmeta_serialize(void * obj, const jmeta_struct_t * jmeta) {
    const cmeta_struct_t * cmetatype = jmeta->metaType;
    cmeta_object_t * metaObj = cmeta_cast_object(obj, cmetatype);
    json_t * j = json_object();
    for (int i = 0; i < jmeta->fieldsSize; i++) {
        const jmeta_field_t * jfield = jmeta->fields + i;
        const cmeta_field_t * cfield = cmeta_struct_getField(jmeta->metaType, jfield->fieldName);
        if (NULL == cfield) {
            // @todo error
            return NULL;
        }
        const char * cname = cfield->name;
        const char * jsonKey = jfield->name;
        json_t * jItem = NULL;
        if (cmeta_isObject(cfield)) {
            void * innerObj = cmeta_get(metaObj, cname, void *);
            jItem = _jmeta_serialize(innerObj, jfield->type);
        } else if (cmeta_isArray(cfield)) {
            jItem = json_array();
            size_t arrSize = cmeta_getArraySize(metaObj, cname);
            for (int itemIndex = 0; itemIndex < arrSize; itemIndex++) {
                void * arrItem = cmeta_getArrayItem(metaObj, cname, itemIndex);
                json_t * jArrItem = _jmeta_serialize(arrItem, jfield->type);
                json_array_append_new(jItem, jArrItem);
            }
        } else if (cmeta_type_eq(cfield->type, &CBOOLEAN)) {
            jItem = json_boolean(cmeta_get(metaObj, cname, bool));
        } else if (cmeta_type_eq(cfield->type, &CINTEGER)) {
            jItem = json_integer(cmeta_get(metaObj, cname, int));
        } else if (cmeta_type_eq(cfield->type, &CDOUBLE)) {
            jItem = json_real(cmeta_get(metaObj, cname, double));
        } else if (cmeta_type_eq(cfield->type, &CSTRING)) {
            jItem = json_string(cmeta_get(metaObj, cname, const char *));
        }
        json_object_set_new(j, jsonKey, jItem);
    }
    return j;
}

const char * jmeta_serialize(void * obj, const jmeta_struct_t * jmeta) {
    json_t * j = _jmeta_serialize(obj, jmeta);
    const char * jStr = json_dumps(j, JSON_INDENT(2) | JSON_PRESERVE_ORDER);
    json_decref(j);
    return jStr;
}

// @todo implement IGNORE (default) | STRICT | DYNAMIC modes
void _jmeta_deserialize(void * obj, const jmeta_struct_t * jmeta, json_t * j) {
    if (obj == NULL) {
        // @todo error deserialising into NULL pointer
        return;
    }
    const cmeta_struct_t * cmetatype = jmeta->metaType;
    cmeta_object_t * metaObj = cmeta_cast_object(obj, cmetatype);
    for (int i = 0; i < jmeta->fieldsSize; i++) {
        const jmeta_field_t * jfield = jmeta->fields + i;
        const cmeta_field_t * cfield = cmeta_struct_getField(cmetatype, jfield->fieldName);
        if (NULL == cfield) {
            // @todo error
            return;
        }
        const cmeta_struct_t * ctype = cfield->type;
        const char * cname = cfield->name;
        const char * jsonKey = jfield->name;
        json_t * jItem = json_object_get(j, jsonKey);
        if (NULL == jItem) {
            // @todo or error on STRICT
            continue;
        }
        if (cmeta_isObject(cfield)) {
            void * cobj = cmeta_get(metaObj, cname, void *);
            _jmeta_deserialize(cobj, jfield->type, jItem);
        } else if (cmeta_isArray(cfield)) {
            size_t cArrSize = cmeta_getArraySize(metaObj, cname);
            // @todo check and fail on STRICT
            size_t index = 0;
            json_t * jEl = NULL;
            json_array_foreach(jItem, index, jEl) {
                if (index < cArrSize) {
                    // @todo overflow error
                    break;
                }
                void * cItem = cmeta_getArrayItem(metaObj, cname, index);
                _jmeta_deserialize(cItem, jfield->type, jEl);
            }
        } else if (cmeta_type_eq(ctype, &CBOOLEAN)) {
            cmeta_set(metaObj, cname, (bool)json_boolean_value(jItem));
        } else if (cmeta_type_eq(ctype, &CINTEGER)) {
            cmeta_set(metaObj, cname, (int)json_integer_value(jItem));
        } else if (cmeta_type_eq(ctype, &CDOUBLE)) {
            cmeta_set(metaObj, cname, (double)json_real_value(jItem));
        } else if (cmeta_type_eq(ctype, &CSTRING)) {
            if (cfield->isPointer) {
                // @todo or fail or malloc&copy
                continue;
            }
            cmeta_set(metaObj, jfield->fieldName, (const char *)json_string_value(jItem));
        }
    }
}

void jmeta_deserialize(void * obj, const jmeta_struct_t * jtype, const char * jstr) {
    json_error_t err;  // @todo use this
    json_t * jobj = json_loads(jstr, 0, &err);
    if (NULL == jobj) {
        //  @todo fprintf(stderr, "JSON Parse error at: `%s`.\n", cJSON_GetErrorPtr());
        return;
    }
    _jmeta_deserialize(obj, jtype, jobj);
    json_decref(jobj);
}

void * jmeta_deserialize_new(const jmeta_struct_t * jtype, const char * jstr) {
    void * obj = malloc(jtype->metaType->size);
    memset(obj, 0, jtype->metaType->size);
    jmeta_deserialize(obj, jtype, jstr);
    return obj;
}
