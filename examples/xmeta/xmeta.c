#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cmeta.h>
#include "xmeta.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

static void _xmeta_serialize(void * obj, const xmeta_struct_t * xmeta, xmlNode * xparent, const char * key) {
    xmlNode * xnode = xmlNewChild(xparent, NULL, (unsigned char *)key, NULL);
    for (int i = 0; i < xmeta->fieldsSize; i++) {
        const xmeta_field_t * xfield = xmeta->fields + i;
        const cmeta_field_t * cfield = cmeta_struct_getField(xmeta->metaType, xfield->fieldName);
        if (NULL == cfield) {
            // @todo error
            return;
        }
        const char * xname = xfield->name;
        const char * xvalue = NULL;
        if (cmeta_isObject(cfield)) {
            void * innerObj = cmeta_getObject(obj, xmeta->metaType, xfield->fieldName);
            _xmeta_serialize(innerObj, xfield->type, xnode, xname);
            continue;
        } else if (cmeta_isArray(cfield)) {
            xmlNode * listNode = xmlNewChild(xnode, NULL, (unsigned char *)xname, NULL);
            size_t arrSize = cmeta_getArraySize(xmeta->metaType, xfield->fieldName);
            for (int itemIndex = 0; itemIndex < arrSize; itemIndex++) {
                void * arrItem = cmeta_getArrayItem(obj, xmeta->metaType, xfield->fieldName, itemIndex);
                _xmeta_serialize(arrItem, xfield->type, listNode, xfield->arrItemName);
            }
        } else if (cmeta_type_eq(cfield->type, &CBOOLEAN)) {
            bool value = cmeta_getBoolean(obj, xmeta->metaType, xfield->fieldName);
            xvalue = (value) ? "true" : "false";
        } else if (cmeta_type_eq(cfield->type, &CINTEGER)) {
            int value = cmeta_getInteger(obj, xmeta->metaType, xfield->fieldName);
            char buf[20] = "";
            snprintf(buf, 20, "%i", value);
            xvalue = buf;
        } else if (cmeta_type_eq(cfield->type, &CDOUBLE)) {
            double value = cmeta_getDouble(obj, xmeta->metaType, xfield->fieldName);
            char buf[20] = "";
            snprintf(buf, 20, "%f", value);
            xvalue = buf;
        } else if (cmeta_type_eq(cfield->type, &CSTRING)) {
            const char * value = cmeta_getString(obj, xmeta->metaType, xfield->fieldName);
            xvalue = value;
        }
        if (xfield->isAttribute) {
            xmlNewProp(xnode, (unsigned char *)xname, (unsigned char *)xvalue);
        } else {
            xmlNewChild(xnode, NULL, (unsigned char *)xname, (unsigned char *)xvalue);
        }
    }
}

const char * xmeta_serialize(void * obj, const xmeta_struct_t * xmeta) {
    xmlDoc * xdoc = xmlNewDoc((unsigned char *)"1.0");
    _xmeta_serialize(obj, xmeta, (xmlNode *)xdoc, "root");
    xmlBuffer * bufferPtr = xmlBufferCreate();
	xmlNodeDump(bufferPtr, NULL, (xmlNode *)xdoc, 0, 1);
	char * strdup = malloc(sizeof(char) * strlen(bufferPtr->content) + 1);
	strcpy(strdup, bufferPtr->content);
	xmlBufferFree(bufferPtr);
    return (const char *)strdup;
}

void xmeta_deserialize(void * obj, const xmeta_struct_t * xtype, const char * xstr) {
    // @todo
}
