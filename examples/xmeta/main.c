#include <stdio.h>
#include <stdlib.h>

#include "xmeta.h"

typedef struct {
    int x;
    int attr;
} o_t;

CMETA_STRUCT(CO_T, o_t, {
    CMETA(o_t, x, CINTEGER),
    CMETA(o_t, attr, CINTEGER),
});

XMETA_STRUCT(XO_T, CO_T, {
    XMETA(x, AUTO),
    XMETA(attr, AUTO),
});

typedef struct {
    bool b;
    int x;
    double y;
    char str[20];
    o_t o;
    o_t arr[3];
} point_t;

CMETA_STRUCT(CPOINT_T, point_t, {
    CMETA(point_t, b, CBOOLEAN),
    CMETA(point_t, x, CINTEGER),
    CMETA(point_t, y, CDOUBLE),
    CMETA_STR(point_t, str, 20),
    CMETA_OBJ(point_t, o, CO_T),
    CMETA_ARR(point_t, arr, CO_T, 3),
});

XMETA_STRUCT(XPOINT_T, CPOINT_T, {
    XMETA(b, AUTO),
    XMETA_ATTR(x, AUTO),
    XMETA(y, AUTO),
    XMETA(str, AUTO),
    XMETA_OBJ(o, XO_T, AUTO),
    XMETA_ARR(arr, XO_T, "item", AUTO),
});

int main(void) {
    point_t point = { false, 5, 45.56, "Some string", {6, 6}};
    const char * xml = ""
        "<root x=\"23\">"
            "<b>true</b>"
            //"<x>23</x>"
            "<y>100.001</y>"
            "<str>String from XML</str>"
            "<o><x>1</x><attr>2</attr></o>"
            "<arr>"
                "<item><x>3</x><attr>7</attr></item>"
                "<item><x>5</x><attr>6</attr></item>"
            "</arr>"
        "</root>"
    "";
    xmeta_deserialize(&point, &XPOINT_T, xml);
    const char * str = xmeta_serialize(&point, &XPOINT_T);
    printf("%s\n", str);
    free((void *)str);
    return 0;
}
