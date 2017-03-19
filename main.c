#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <cmeta.h>
#include <xmeta.h>

typedef struct {
    int x;
    int y;
} point_t;

CMETA_STRUCT(CPOINT_T, point_t, {
    CMETA(point_t, x),
    CMETA(point_t, y)
});

XMETA_STRUCT(XPOINT_T, CPOINT_T, {
    XMETA_ATTR(x, AUTO),
    XMETA_ATTR(y, AUTO),
});

typedef struct {
    bool boolean;
    int integer;
    double _double;
    char stringBuf[20];
    char * stringPtr;
    point_t pointObj;
    point_t * pointObjPtr;
    point_t pointArr[5];
} sample_t;

CMETA_STRUCT(CSAMPLE_T, sample_t, {
    CMETA(sample_t, boolean),
    CMETA(sample_t, integer),
    CMETA(sample_t, _double),
    CMETA_STR(sample_t, stringBuf),
    CMETA_STR_PTR(sample_t, stringPtr),
    CMETA_OBJ(sample_t, pointObj, CPOINT_T),
    CMETA_OBJ_PTR(sample_t, pointObjPtr, CPOINT_T),
    CMETA_ARR(sample_t, pointArr, CPOINT_T),
});

XMETA_STRUCT(XSAMPLE_T, CSAMPLE_T, {
    XMETA(boolean, AUTO),
    XMETA(integer, AUTO),
    XMETA(_double, AUTO),
    XMETA(stringBuf, AUTO),
    XMETA(stringPtr, AUTO),
    XMETA_OBJ(pointObj, XPOINT_T, AUTO),
    XMETA_OBJ(pointObjPtr, XPOINT_T, AUTO),
    XMETA_ARR(pointArr, XPOINT_T, "pt", AUTO)
});

void sample_cmeta_print(sample_t * self);
void sample_clear_fields(sample_t * self);

int main(void) {
    cmeta_struct_print(&CSAMPLE_T);

    point_t point = { -333, 777 };
    point_t point2 = { 10, 100 };

    sample_t sample = {
        .boolean = false,
        .integer = 667,
        ._double = 321.123,
        .stringBuf = "String buffer string",
        .stringPtr = "String pointer",
        .pointObj = {8, 8},
        .pointObjPtr = &point,
        .pointArr = { point, point2, {0, 0}, {1, 1}, {2, 2}}
    };
    sample_cmeta_print(&sample);

    cmeta_object_t * obj = cmeta_cast_object(&sample, &CSAMPLE_T);
    cmeta_set(obj, "boolean", true);
    cmeta_set(obj, "integer", 999);
    cmeta_set(obj, "_double", 19.93);
    cmeta_set(obj, "stringBuf", "I am string");  // overflow here
    cmeta_set(obj, "stringBuf", "I set a new string here");  // overflow here
    cmeta_set(obj, "pointObj", &point);
    cmeta_set(obj, "pointObjPtr", &point2);
    cmeta_setArrayItem(obj, "pointArr", 2, &((point_t){ 9999, -7777 }));
    cmeta_setArrayItem(obj, "pointArr", 4, &((point_t){ 1111, -7777 }));
    sample_cmeta_print(&sample);

    // XML

    const char * xmlString = xmeta_serialize_root((void *)obj->ptr, &XSAMPLE_T, "sample");
    puts(xmlString);

    sample_t * newObj = (sample_t *)xmeta_deserialize_new(&XSAMPLE_T, xmlString);
    sample_cmeta_print(newObj);
    free(newObj);

    free((void *)xmlString);

    return 0;
}

void sample_clear_fields(sample_t * self) {
    self->boolean = false;
    self->integer = 0;
    self->_double = 0.0;
    strcpy(self->stringBuf, "");
    self->stringPtr = NULL;
    self->pointObj = (point_t){0, 0};
    self->pointObjPtr = NULL;
    for (int i = 0; i < 5; i++) {
        self->pointArr[i] = (point_t){0, 0};
    }
}

void sample_cmeta_print(sample_t * sample) {
    printf("\nsample_T obj;\n");
    cmeta_object_t * self = cmeta_cast_object(sample, &CSAMPLE_T);
    printf("obj.boolean = %i;\n", cmeta_get(self, "boolean", bool));
    printf("obj.integer = %i;\n", cmeta_get(self, "integer", int));
    printf("obj._double = %f;\n", cmeta_get(self, "_double", double));
    printf("obj.stringBuf = \"%s\";\n", cmeta_get(self, "stringBuf", char *));
    const char * stringPtr = cmeta_get(self, "stringPtr", char *);
    if (stringPtr != NULL)
        printf("obj.stringPtr = \"%s\";\n", stringPtr);
    else
        printf("obj.stringPtr = NULL;\n");
    point_t * pointObj = cmeta_get(self, "pointObj", point_t *);
    printf("obj.pointObj = {%i, %i};\n", pointObj->x, pointObj->y);
    pointObj = cmeta_get(self, "pointObjPtr", point_t *);
    if (pointObj != NULL)
        printf("obj.pointObjPtr = {%i, %i};\n", pointObj->x, pointObj->y);
    else 
        printf("obj.pointObjPtr = NULL;\n");
    int arrLen = cmeta_getArraySize(self, "pointArr");
    printf("obj.pointArr[%i]:\n", arrLen);
    for (int i = 0; i < arrLen; i++) {
        point_t * p = (point_t *)cmeta_getArrayItem(self, "pointArr", i);
        printf("\tobj.pointArr[%i] = {%i, %i};\n", i, p->x, p->y);
    }
    printf("\n");
}
