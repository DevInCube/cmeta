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
	const char * content = (const char *)bufferPtr->content;
	char * strdup = malloc(sizeof(char) * strlen(content) + 1);
	strcpy(strdup, content);
	xmlBufferFree(bufferPtr);
    return (const char *)strdup;
}

static xmlNode * _xml_getNodeByKey(xmlNode * xnode, const char * key) {
    for (xmlNode * xI = xnode->children; xI; xI = xI->next) {
        if(xmlStrcmp(xI->name, key) == 0) {
            return xI;
        }
    }
    return NULL;
}

static const char * _xml_getContentByKey(xmlNode * xnode, const char * key) {
    xmlNode * xI = _xml_getNodeByKey(xnode, key);
    if (NULL == xI) {
        return NULL;
    }
    xmlChar * xvalue = xmlNodeGetContent(xI);
    char * value = malloc(sizeof(char) * strlen((char *)xvalue) + 1);
    strncpy(value, xvalue, strlen(xvalue) + 1);
    return (const char *)value;
}

void _xmeta_deserialize(void * obj, const xmeta_struct_t * xmeta, xmlNode * xnode) {
    for (int i = 0; i < xmeta->fieldsSize; i++) {
        const xmeta_field_t * xfield = xmeta->fields + i;
        const cmeta_field_t * cfield = cmeta_struct_getField(xmeta->metaType, xfield->fieldName);
        if (NULL == cfield) {
            // @todo error
            return;
        }
        const char * xname = xfield->name;
        if (cmeta_isObject(cfield)) {
            xmlNode * innerNode = _xml_getNodeByKey(xnode, xname);
            if (NULL == innerNode) {
                // @todo
                continue;
            }
            void * innerObj = cmeta_getObject(obj, xmeta->metaType, xfield->fieldName);
            _xmeta_deserialize(innerObj, xfield->type, innerNode);
            continue;
        } else if (cmeta_isArray(cfield)) {
            xmlNode * innerNode = _xml_getNodeByKey(xnode, xname);
            if (NULL == innerNode) {
                // @todo
                continue;
            }
            size_t cArrSize = cmeta_getArraySize(xmeta->metaType, xfield->fieldName);
            int index = 0;
            for (xmlNode * xI = innerNode->children; xI && (index < cArrSize); xI = xI->next, index++) {
                void * itemObj = cmeta_getArrayItem(obj, xmeta->metaType, xfield->fieldName, (int)index);
                _xmeta_deserialize(itemObj, xfield->type, xI);
            }
        } else {
            const char * content = _xml_getContentByKey(xnode, xname);
            if (cmeta_type_eq(cfield->type, &CBOOLEAN)) {
                bool value = (strncmp(content, "true", 4) == 0) ? true : false;
                cmeta_setBoolean(obj, xmeta->metaType, xfield->fieldName, value);
            } else if (cmeta_type_eq(cfield->type, &CINTEGER)) {
                int value = atoi(content);
                cmeta_setInteger(obj, xmeta->metaType, xfield->fieldName, value);
            } else if (cmeta_type_eq(cfield->type, &CDOUBLE)) {
                double value = atof(content);
                cmeta_setDouble(obj, xmeta->metaType, xfield->fieldName, value);
            } else if (cmeta_type_eq(cfield->type, &CSTRING)) {
                const char * value = content;
                cmeta_setString(obj, xmeta->metaType, xfield->fieldName, value);
            }
            free((void *)content);
        }
        if (xfield->isAttribute) {
            //xmlNewProp(xnode, (unsigned char *)xname, (unsigned char *)xvalue);
        } else {
            //xmlNewChild(xnode, NULL, (unsigned char *)xname, (unsigned char *)xvalue);
        }
    }
}

void xmeta_deserialize(void * obj, const xmeta_struct_t * xtype, const char * xstr) {
    xmlDoc * xDoc = xmlReadMemory(xstr, strlen(xstr), NULL, NULL, 0);
    xmlNode * xRoot = xmlDocGetRootElement(xDoc);
    _xmeta_deserialize(obj, xtype, xRoot);
    xmlFreeDoc(xDoc);
}
