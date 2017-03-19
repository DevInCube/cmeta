#include <stdio.h>
#include <stdlib.h>

#include <cmeta.h>
#include "jmeta.h"

typedef struct {
    int x;
    int y;
} point_t;

CMETA_STRUCT(CPOINT_T, point_t, {
    CMETA(point_t, x, CINTEGER),
    CMETA(point_t, y, CINTEGER)
});

JMETA_STRUCT(JPOINT_T, CPOINT_T, {
    JMETA(x, AUTO),
    JMETA(y, AUTO)
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

JMETA_STRUCT(JSAMPLE_T, CSAMPLE_T, {
    JMETA(boolean, AUTO),
    JMETA(integer, AUTO),
    JMETA(_double, AUTO),
    JMETA(stringBuf, AUTO),
    JMETA(stringPtr, AUTO),
    JMETA_OBJ(pointObj, JPOINT_T, AUTO),
    JMETA_OBJ(pointObjPtr, JPOINT_T, AUTO),
    JMETA_ARR(pointArr, JPOINT_T, AUTO),
});

int main(void) {
    point_t point = { 7, 10 };
    sample_t sample = {
        .boolean = true,
        .integer = 777,
        ._double = 45.56,
        .stringBuf = "Buffer of chars",
        .stringPtr = "Pointer to string",
        .pointObj = { 6, 5 },
        .pointObjPtr = & point,
        .pointArr = {{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}},
    };
    const char * jStr = jmeta_serialize(&sample, &JSAMPLE_T);
    printf("JSON: %s\n", jStr);
    free((void *)jStr);
    const char * newJsonStr = "{"
        "\"boolean\" : false,"
        "\"integer\" : 12345,"
        "\"_double\" : 1.123456,"
        "\"stringBuf\" : \"json string\","
        "\"stringPtr\" : \"json string pointed\","
        "\"pointArr\" : [ {\"x\":9, \"y\":9}, {\"x\":8, \"y\":8} ]"
    "}";
    jmeta_deserialize(&sample, &JSAMPLE_T, newJsonStr);
    jStr = jmeta_serialize(&sample, &JSAMPLE_T);
    printf("JSON-2: %s\n", jStr);
    free((void *)jStr);
    return 0;
}
