#include <stdlib.h>

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
        const char * jKey = (AUTO == jfield->name) ? jfield->fieldName : jfield->name;
        cJSON * jItem = NULL;
        if (cmeta_isObject(cfield)) {
            void * innerObj = cmeta_getObject(obj, jmeta->metaType, jfield->fieldName);
            jItem = _jmeta_serialize(innerObj, jfield->type);
        } else if (cmeta_isArray(cfield)) {
            jItem = cJSON_CreateArray();
            size_t arrSize = cmeta_getArraySize(jmeta->metaType, jfield->fieldName);
            for (int itemIndex = 0; itemIndex < arrSize; itemIndex++) {
                void * arrItem = cmeta_getArrayItem(obj, jmeta->metaType, jfield->fieldName, itemIndex);
                cJSON * jArrItem = _jmeta_serialize(arrItem, jfield->type);
                cJSON_AddItemToArray(jItem, jArrItem);
            }
        } else if (cmeta_type_eq(cfield->type, &CBOOLEAN)) {
            bool value = cmeta_getBoolean(obj, jmeta->metaType, jfield->fieldName);
            jItem = cJSON_CreateBool((int)value);
        } else if (cmeta_type_eq(cfield->type, &CINTEGER)) {
            int intValue = cmeta_getInteger(obj, jmeta->metaType, jfield->fieldName);
            jItem = cJSON_CreateNumber((double)intValue);
        } else if (cmeta_type_eq(cfield->type, &CDOUBLE)) {
            double value = cmeta_getDouble(obj, jmeta->metaType, jfield->fieldName);
            jItem = cJSON_CreateNumber((double)value);
        } else if (cmeta_type_eq(cfield->type, &CSTRING)) {
            const char * value = cmeta_getString(obj, jmeta->metaType, jfield->fieldName);
            jItem = cJSON_CreateString(value);
        }
        cJSON_AddItemToObject(j, jKey, jItem);
    }
    return j;
}

const char * jmeta_serialize(void * obj, const jmeta_struct_t * jmeta) {
    cJSON * j = _jmeta_serialize(obj, jmeta);
    const char * jStr = cJSON_Print(j);
    cJSON_Delete(j);
    return jStr;
}
