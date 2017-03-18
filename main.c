#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <cmeta.h>

typedef struct {
    int x;
    int y;
} point_t;

CMETA_STRUCT(CPOINT_T, point_t, {
    CMETA(point_t, x, CINTEGER),
    CMETA(point_t, y, CINTEGER)
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
    CMETA(sample_t, boolean, CBOOLEAN),
    CMETA(sample_t, integer, CINTEGER),
    CMETA(sample_t, _double, CDOUBLE),
    CMETA_STR(sample_t, stringBuf, 20),
    CMETA_STR_PTR(sample_t, stringPtr),
    CMETA_OBJ(sample_t, pointObj, CPOINT_T),
    CMETA_OBJ_PTR(sample_t, pointObjPtr, CPOINT_T),
    CMETA_ARR(sample_t, pointArr, CPOINT_T, 5),
});

void sample_cmeta_print(sample_t * self);

int main(void) {
    cmeta_struct_print(&CSAMPLE_T);
    point_t point = { -333, 777 };
    point_t point2 = { 10, 100 };
    point_t pointItem = { 9999, -7777 };
    sample_t obj = {
        .boolean = false,
        .integer = 667,
        ._double = 321.123,
        .stringBuf = "String buffer string",
        .pointObj = {8, 8},
        .pointObjPtr = &point,
        .pointArr = { point, point2, {0, 0}, {1, 1}, {2, 2}}
    };
    sample_cmeta_print(&obj);
    cmeta_set(&obj, &CSAMPLE_T, "boolean", true);
    cmeta_set(&obj, &CSAMPLE_T, "integer", 999);
    cmeta_set(&obj, &CSAMPLE_T, "_double", 19.93);
    cmeta_set(&obj, &CSAMPLE_T, "stringBuf", "I am string");  // overflow here
    cmeta_set(&obj, &CSAMPLE_T, "stringBuf", "I set a new string here");  // overflow here
    cmeta_set(&obj, &CSAMPLE_T, "pointObj", &point);
    cmeta_set(&obj, &CSAMPLE_T, "pointObjPtr", &point2);
    cmeta_setArrayItem(&obj, &CSAMPLE_T, "pointArr", 2, &pointItem);
    cmeta_setArrayItem(&obj, &CSAMPLE_T, "pointArr", 4, &pointItem);
    sample_cmeta_print(&obj);
    return 0;
}

void sample_cmeta_print(sample_t * self) {
    printf("\nsample_T obj;\n");
    printf("obj.boolean = %i;\n", cmeta_get(self, &CSAMPLE_T, "boolean", bool));
    printf("obj.integer = %i;\n", cmeta_get(self, &CSAMPLE_T, "integer", int));
    printf("obj._double = %f;\n", cmeta_get(self, &CSAMPLE_T, "_double", double));
    printf("obj.stringBuf = \"%s\";\n", cmeta_get(self, &CSAMPLE_T, "stringBuf", char *));
    point_t * pointObj = cmeta_get(self, &CSAMPLE_T, "pointObj", point_t *);
    printf("obj.pointObj = {%i, %i};\n", pointObj->x, pointObj->y);
    pointObj = cmeta_get(self, &CSAMPLE_T, "pointObjPtr", point_t *);
    printf("obj.pointObjPtr = {%i, %i};\n", pointObj->x, pointObj->y);
    int arrLen = cmeta_getArraySize(&CSAMPLE_T, "pointArr");
    printf("obj.pointArr[%i]:\n", arrLen);
    for (int i = 0; i < arrLen; i++) {
        point_t * p = (point_t *)cmeta_getArrayItem(self, &CSAMPLE_T, "pointArr", i);
        printf("\tobj.pointArr[%i] = {%i, %i};\n", i, p->x, p->y);
    }
    printf("\n");
}
