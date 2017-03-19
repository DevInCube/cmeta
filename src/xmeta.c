/**
    Current RESTRICTIONS:
    - cannot deserialize to pointer fields
*/

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
        const char * cname = cfield->name;
        const char * xvalue = NULL;
        cmeta_object_t * metaObj = cmeta_cast_object(obj, xmeta->metaType);
        if (cmeta_isObject(cfield)) {
            void * innerObj = cmeta_get(metaObj, cname, void *);
            _xmeta_serialize(innerObj, xfield->type, xnode, xname);
            continue;
        } else if (cmeta_type_eq(cfield->type, &CBOOLEAN)) {
            bool value = cmeta_get(metaObj, cname, bool);
            xvalue = (value) ? "true" : "false";
        } else if (cmeta_type_eq(cfield->type, &CINTEGER)) {
            int value = cmeta_get(metaObj, cname, int);
            char buf[20] = "";
            snprintf(buf, 20, "%i", value);
            xvalue = buf;
        } else if (cmeta_type_eq(cfield->type, &CDOUBLE)) {
            double value = cmeta_get(metaObj, cname, double);
            char buf[20] = "";
            snprintf(buf, 20, "%f", value);
            xvalue = buf;
        } else if (cmeta_type_eq(cfield->type, &CSTRING)) {
            const char * value = cmeta_get(metaObj, cname, const char *);
            xvalue = value;
        }
        if (xfield->isAttribute) {
            xmlNewProp(xnode, (unsigned char *)xname, (unsigned char *)xvalue);
        } else {
            xmlNode * newNode = xmlNewChild(xnode, NULL, (unsigned char *)xname, (unsigned char *)xvalue);
            if (cmeta_isArray(cfield)) {
                size_t arrSize = cmeta_getArraySize(metaObj, cname);
                for (int itemIndex = 0; itemIndex < arrSize; itemIndex++) {
                    void * arrItem = cmeta_getArrayItem(metaObj, cname, itemIndex);
                    _xmeta_serialize(arrItem, xfield->type, newNode, xfield->arrItemName);
                }
            }
        }
    }
}

const char * xmeta_serialize(void * obj, const xmeta_struct_t * xmeta) {
    xmlDoc * xdoc = xmlNewDoc((unsigned char *)"1.0");
    _xmeta_serialize(obj, xmeta, (xmlNode *)xdoc, "root");
    xmlBuffer * bufferPtr = xmlBufferCreate();
	xmlNodeDump(bufferPtr, NULL, (xmlNode *)xdoc, 0, 1);
	const char * content = (const char *)bufferPtr->content;
	char * strdup = malloc(sizeof(char) * (strlen(content) + 1));
	strcpy(strdup, content);
	xmlBufferFree(bufferPtr);
    return (const char *)strdup;
}

static xmlNode * _xml_getNodeByKey(xmlNode * xnode, const char * key) {
    for (xmlNode * xI = xnode->children; xI; xI = xI->next) {
        if(xmlStrcmp(xI->name, (unsigned char *)key) == 0) {
            return xI;
        }
    }
    return NULL;
}

void _xmeta_deserialize(void * obj, const xmeta_struct_t * xmeta, xmlNode * xnode) {
    if (obj == NULL) {
        // @todo error deserialising into NULL pointer
        return;
    }
    const cmeta_struct_t * cmetatype = xmeta->metaType;
    cmeta_object_t * metaObj = cmeta_cast_object(obj, cmetatype);
    for (int i = 0; i < xmeta->fieldsSize; i++) {
        const xmeta_field_t * xfield = xmeta->fields + i;
        const cmeta_field_t * cfield = cmeta_struct_getField(cmetatype, xfield->fieldName);
        if (NULL == cfield) {
            // @todo error
            continue;
        }
        const char * xname = xfield->name;
        const char * cname = cfield->name;
        xmlNode * innerNode = _xml_getNodeByKey(xnode, xname);
        if (cmeta_isObject(cfield)) {
            if (NULL == innerNode) {
                // @todo xml node is null
                continue;
            }
            void * innerObj = cmeta_get(metaObj, cname, void *);
            _xmeta_deserialize(innerObj, xfield->type, innerNode);
        } else if (cmeta_isArray(cfield)) {
            if (NULL == innerNode) {
                // @todo xml node is null
                continue;
            }
            size_t cArrSize = cmeta_getArraySize(metaObj, cname);
            int index = 0;
            for (xmlNode * xI = innerNode->children; xI; xI = xI->next) {
                if (xI->type == XML_ELEMENT_NODE) {
                    if (index >= cArrSize) {
                        // @todo overflow error here
                        break;
                    }
                    void * itemObj = cmeta_getArrayItem(metaObj, cname, index);
                    _xmeta_deserialize(itemObj, xfield->type, xI);
                    index++;
                }
            }
        } else {
            char * value = (char *)(xfield->isAttribute 
                ? xmlGetProp(xnode, (unsigned char *)xname)
                : xmlNodeGetContent(innerNode));
            size_t valueLength = strlen(value) + 1;
            char * content = malloc(sizeof(char) * valueLength);
            strncpy(content, value, valueLength);
            if (cmeta_type_eq(cfield->type, &CBOOLEAN)) {
                cmeta_set(metaObj, cname, (bool)(strncmp(content, "true", 4) == 0));
            } else if (cmeta_type_eq(cfield->type, &CINTEGER)) {
                cmeta_set(metaObj, cname, (int)atoi(content));
            } else if (cmeta_type_eq(cfield->type, &CDOUBLE)) {
                cmeta_set(metaObj, cname, (double)atof(content));
            } else if (cmeta_type_eq(cfield->type, &CSTRING)) {
                cmeta_set(metaObj, cname, (const char *)content);
            }
            free((void *)content);
        }
    }
}

void xmeta_deserialize(void * obj, const xmeta_struct_t * xtype, const char * xstr) {
    xmlDoc * xDoc = xmlReadMemory(xstr, strlen(xstr), NULL, NULL, 0);
    xmlNode * xRoot = xmlDocGetRootElement(xDoc);
    _xmeta_deserialize(obj, xtype, xRoot);
    xmlFreeDoc(xDoc);
}
