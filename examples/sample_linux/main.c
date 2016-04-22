#include <stdio.h>
#include <stdlib.h>

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

void sample_cmetaprint(sample_t * self) {
    printf("\nsample_T obj;\n");
    printf("obj.boolean = %i;\n", cmeta_getBoolean(self, &CSAMPLE_T, "boolean"));
    printf("obj.integer = %i;\n", cmeta_getInteger(self, &CSAMPLE_T, "integer"));
    printf("obj._double = %f;\n", cmeta_getDouble(self, &CSAMPLE_T, "_double"));
    printf("obj.stringBuf = \"%s\";\n", cmeta_getString(self, &CSAMPLE_T, "stringBuf"));
    point_t * pointObj = (point_t *)cmeta_getObject(self, &CSAMPLE_T, "pointObj");
    printf("obj.pointObj = {%i, %i};\n", pointObj->x, pointObj->y);
    pointObj = (point_t *)cmeta_getObject(self, &CSAMPLE_T, "pointObjPtr");
    printf("obj.pointObjPtr = {%i, %i};\n", pointObj->x, pointObj->y);
    int arrLen = cmeta_getArraySize(&CSAMPLE_T, "pointArr");
    printf("obj.pointArr[%i]:\n", arrLen);
    for (int i = 0; i < arrLen; i++) {
        point_t * p = (point_t *)cmeta_getArrayItem(self, &CSAMPLE_T, "pointArr", i);
        printf("\tobj.pointArr[%i] = {%i, %i};\n", i, p->x, p->y);
    }
    printf("\n");
}

int main(void) {
    cmeta_struct_print(&CSAMPLE_T);
    point_t point = {
        .x = -333,
        .y = 777
    };
    point_t point2 = { 10, 100 };
    point_t pointItem = { 9999, -7777 };
    sample_t obj = {
        .boolean = false,
        .integer = 667,
        ._double = 321.123,
        .stringBuf = "String buffer string",
        .pointObj = {8, 8},
        .pointObjPtr = & point,
        .pointArr = { point, point2, {0, 0}, {1, 1}, {2, 2}}
    };
    sample_cmetaprint(&obj);
    cmeta_setBoolean(&obj, &CSAMPLE_T, "boolean", true);
    cmeta_setInteger(&obj, &CSAMPLE_T, "integer", 999);
    cmeta_setDouble(&obj, &CSAMPLE_T, "_double", 19.93);
    cmeta_setString(&obj, &CSAMPLE_T, "stringBuf", "I set a new string here");
    cmeta_setObject(&obj, &CSAMPLE_T, "pointObj", &point);
    cmeta_setObject(&obj, &CSAMPLE_T, "pointObjPtr", &point2);
    cmeta_setArrayItem(&obj, &CSAMPLE_T, "pointArr", 2, &pointItem);
    sample_cmetaprint(&obj);
    return 0;
}
