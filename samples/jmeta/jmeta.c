#include <stdlib.h>
#include <stdio.h>

#include "jmeta.h"
#include "cJSON.h"

static cJSON * _jmeta_serialize(void * obj, const jmeta_struct_t * jmeta) {
    cJSON * j = cJSON_CreateObject();
    for (int i = 0; i < jmeta->fieldsSize; i++) {
        const jmeta_field_t * jfield = jmeta->fields + i;
        const cmeta_field_t * cfield = cmeta_struct_getField(jmeta->metaType, jfield->fieldName);
        if (NULL == cfield) {
            // @todo error
            return NULL;
        }
        const char * cname = cfield->name;
        const char * jsonKey = jfield->name;
        cJSON * jItem = NULL;
        if (cmeta_isObject(cfield)) {
            void * innerObj = cmeta_getObject(obj, jmeta->metaType, cname);
            jItem = _jmeta_serialize(innerObj, jfield->type);
        } else if (cmeta_isArray(cfield)) {
            jItem = cJSON_CreateArray();
            size_t arrSize = cmeta_getArraySize(jmeta->metaType, cname);
            for (int itemIndex = 0; itemIndex < arrSize; itemIndex++) {
                void * arrItem = cmeta_getArrayItem(obj, jmeta->metaType, cname, itemIndex);
                cJSON * jArrItem = _jmeta_serialize(arrItem, jfield->type);
                cJSON_AddItemToArray(jItem, jArrItem);
            }
        } else if (cmeta_type_eq(cfield->type, &CBOOLEAN)) {
            bool value = cmeta_getBoolean(obj, jmeta->metaType, cname);
            jItem = cJSON_CreateBool((int)value);
        } else if (cmeta_type_eq(cfield->type, &CINTEGER)) {
            int intValue = cmeta_getInteger(obj, jmeta->metaType, cname);
            jItem = cJSON_CreateNumber((double)intValue);
        } else if (cmeta_type_eq(cfield->type, &CDOUBLE)) {
            double value = cmeta_getDouble(obj, jmeta->metaType, cname);
            jItem = cJSON_CreateNumber((double)value);
        } else if (cmeta_type_eq(cfield->type, &CSTRING)) {
            const char * value = cmeta_getString(obj, jmeta->metaType, cname);
            jItem = cJSON_CreateString(value);
        }
        cJSON_AddItemToObject(j, jsonKey, jItem);
    }
    return j;
}

const char * jmeta_serialize(void * obj, const jmeta_struct_t * jmeta) {
    cJSON * j = _jmeta_serialize(obj, jmeta);
    const char * jStr = cJSON_Print(j);
    cJSON_Delete(j);
    return jStr;
}

// @todo implement IGNORE (default) | STRICT | DYNAMIC modes
void _jmeta_deserialize(void * obj, const jmeta_struct_t * jmeta, cJSON * j) {
    const cmeta_struct_t * cmetatype = jmeta->metaType;
    for (int i = 0; i < jmeta->fieldsSize; i++) {
        const jmeta_field_t * jfield = jmeta->fields + i;
        const cmeta_field_t * cfield = cmeta_struct_getField(cmetatype, jfield->fieldName);
        if (NULL == cfield) {
            // @todo error
            return;
        }
        const char * cname = cfield->name;
        const char * jsonKey = jfield->name;
        cJSON * jItem = cJSON_GetObjectItem(j, jsonKey);
        if (NULL == jItem) {
            // @todo or error on STRICT
            continue;
        }
        if (cmeta_isObject(cfield)) {
            void * cobj = cmeta_getObject(obj, cmetatype, cname);
            _jmeta_deserialize(cobj, jfield->type, jItem);
        } else if (cmeta_isArray(cfield)) {
            size_t cArrSize = cmeta_getArraySize(cmetatype, cname);
            size_t jArrSize = (size_t)cJSON_GetArraySize(jItem);
            // @todo check and fail on STRICT
            for (int index = 0; (index < jArrSize) && (index < cArrSize); index++) {
                cJSON * jEl = cJSON_GetArrayItem(jItem, index);
                void * cItem = cmeta_getArrayItem(obj, cmetatype, cname, index);
                _jmeta_deserialize(cItem, jfield->type, jEl);
            }
        } else if (cmeta_type_eq(cfield->type, &CBOOLEAN)) {
            bool val = jItem->valueint;
            cmeta_setBoolean(obj, jmeta->metaType, cname, val);
        } else if (cmeta_type_eq(cfield->type, &CINTEGER)) {
            int val = jItem->valueint;
            cmeta_setInteger(obj, jmeta->metaType, cname, val);
        } else if (cmeta_type_eq(cfield->type, &CDOUBLE)) {
            double val = jItem->valuedouble;
            cmeta_setDouble(obj, jmeta->metaType, cname, val);
        } else if (cmeta_type_eq(cfield->type, &CSTRING)) {
            if (cfield->isPointer) {
                // @todo or fail or malloc&copy
                continue;
            }
            const char * val = (const char *)jItem->valuestring;
            cmeta_setString(obj, jmeta->metaType, jfield->fieldName, val);
        }
    }
}

void jmeta_deserialize(void * obj, const jmeta_struct_t * jtype, const char * jstr) {
    cJSON * jobj = cJSON_Parse(jstr);
    if (NULL == jobj) {
        fprintf(stderr, "JSON Parse error at: `%s`.\n", cJSON_GetErrorPtr());
        return;
    }
    _jmeta_deserialize(obj, jtype, jobj);
    cJSON_Delete(jobj);
}
