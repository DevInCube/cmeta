#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <cmeta.h>

const char * cmeta_toString(const cmeta_struct_t * type) {
    return type->name;
}

int cmeta_type_eq(const cmeta_struct_t * a, const cmeta_struct_t * b) {
    return strcmp(a->name, b->name) == 0;
}

int cmeta_isBasicType(const cmeta_field_t * field) {
    return (NULL == field->type->fields);
}

int cmeta_isObject(const cmeta_field_t * field) {
    return !cmeta_isBasicType(field) && (0 == field->arrSize);
}

int cmeta_isArray(const cmeta_field_t * field) {
    return !cmeta_isBasicType(field) && (0 != field->arrSize);
}

static void cmeta_struct_print_lvl(const cmeta_struct_t * metaType, const cmeta_field_t * field, int lvl) {
	const int spaces = 4;
	char * pad = malloc(sizeof(char) * lvl * spaces + 1);
	for (int i = 0; i < lvl * spaces; i++) { pad[i] = ' '; };
	pad[lvl * spaces] = '\0';
	printf("%s", pad);
	printf("%-10s", metaType->name);
	if (NULL != field) {
        char * ptrstr = (field->isPointer) ? " *" : "";
		printf("%s %-10s __(%zu)", ptrstr, field->name, field->offset);
	}
	printf("\n");
	for (int i = 0; i < metaType->fieldsSize; i++) {
		const cmeta_field_t * meta = metaType->fields + i;
		if (cmeta_isObject(meta)) {
			cmeta_struct_print_lvl(meta->type, meta, lvl + 1);
		} else if (cmeta_isArray(meta)) {
			printf("%s", pad);
			printf("    %-10s<%s> %-10s[%zu] __(%zu):\n",
				cmeta_toString(meta->type), meta->type->name, meta->name, meta->arrSize, meta->offset);
		} else {
			printf("%s", pad);
			char arrSizeStr[10] = "";
			if (meta->arrSize > 0) {
                snprintf(arrSizeStr, 10, " [%zu]", meta->arrSize);
			}
			const char * ptrStr = (meta->isPointer) ? " *" : "";
			printf("    %-10s%s %-10s%s __(%zu)\n",
				cmeta_toString(meta->type), ptrStr, meta->name, arrSizeStr, meta->offset);
		}
	}
	free(pad);
}

void cmeta_struct_print(const cmeta_struct_t * metaType) {
	cmeta_struct_print_lvl(metaType, NULL, 0);
}

const cmeta_field_t * cmeta_struct_getField(const cmeta_struct_t * meta, const char * fieldName) {
    const cmeta_field_t * field = NULL;
    for (int i = 0; i < meta->fieldsSize; i++) {
        field = meta->fields + i;
        if (strcmp(field->name, fieldName) == 0) {
            return field;
        }
    }
    return NULL;
}

const char * cmeta_getString(cmeta_object_t * self, const char * fieldName) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_type_eq(field->type, &CSTRING)) {
        return NULL;
    }
    if (field->isPointer) {
        const char * str = (const char *)VALUE_POINTER(CMETA_OFFSET(obj, field->offset));
        return str;
    } else {
        const char * str = (const char *)CMETA_OFFSET(obj, field->offset);
        return str;
    }
}

void cmeta_setString(cmeta_object_t * self, const char * fieldName, const char * value) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_type_eq(field->type, &CSTRING)) {
        return;
    }
    if (field->isPointer) {
        const char ** str = (const char **)VALUE_POINTER(CMETA_OFFSET(obj, field->offset));
        *str = value;
    } else {
        char * str = (char *)CMETA_OFFSET(obj, field->offset);
        size_t maxSize = field->arrSize;
        if (strlen(value) >= maxSize) {
            // @todo error
            return;
        }
        strncpy(str, value, maxSize);
    }
}

int cmeta_getInteger(cmeta_object_t * self, const char * fieldName) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_type_eq(field->type, &CINTEGER)) {
        return 0;
    }
    return *(int *)CMETA_OFFSET(obj, field->offset);
}

void cmeta_setInteger(cmeta_object_t * self, const char * fieldName, int value) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) 
        || !(cmeta_type_eq(field->type, &CINTEGER) || cmeta_type_eq(field->type, &CBOOLEAN))) {
        return;
    }
    if (cmeta_type_eq(field->type, &CBOOLEAN)) {
        value = value ? 1 : 0;
    }
    *(int *)CMETA_OFFSET(obj, field->offset) = value;
}

bool cmeta_getBoolean(cmeta_object_t * self, const char * fieldName) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_type_eq(field->type, &CBOOLEAN)) {
        return false;
    }
    return *(bool *)CMETA_OFFSET(obj, field->offset);
}

void cmeta_setBoolean(cmeta_object_t * self, const char * fieldName, bool value) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_type_eq(field->type, &CBOOLEAN)) {
        return;
    }
    *(bool *)CMETA_OFFSET(obj, field->offset) = value;
}

double cmeta_getDouble(cmeta_object_t * self, const char * fieldName) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_type_eq(field->type, &CDOUBLE)) {
        return 0.0;
    }
    return *(double *)CMETA_OFFSET(obj, field->offset);
}

void cmeta_setDouble(cmeta_object_t * self, const char * fieldName, double value) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_type_eq(field->type, &CDOUBLE)) {
        return;
    }
    *(double *)CMETA_OFFSET(obj, field->offset) = value;
}

void * cmeta_getObject(cmeta_object_t * self, const char * fieldName) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_isObject(field)) {
        return NULL;
    }
    if (field->isPointer) {
        void * pointedObjAddr = VALUE_POINTER(CMETA_OFFSET(obj, field->offset));
        return pointedObjAddr;
    } else {
        return CMETA_OFFSET(obj, field->offset);
    }
}

void cmeta_setObject(cmeta_object_t * self, const char * fieldName, void * value) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_isObject(field)) {
        return;
    }
    if (field->isPointer) {
        void ** dest = (void **)VALUE_POINTER(CMETA_OFFSET(obj, field->offset));
        *dest = VALUE_POINTER(value);
    } else {
        void * destAddr = CMETA_OFFSET(obj, field->offset);
        memcpy(destAddr, value, field->type->size);
    }
}

int cmeta_getArraySize(cmeta_object_t * self, const char * fieldName) {
    const cmeta_struct_t * meta = self->type;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_isArray(field)) {
        return 0;
    }
    return field->arrSize;
}

void * cmeta_getArrayItem(cmeta_object_t * self, const char * fieldName, int index) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_isArray(field)) {
        return NULL;
    }
    if (field->isPointer) {
        // @todo
        printf("Not implemented!\n");
        return NULL;
    } else {
        size_t arrSize = field->arrSize;
        if (index < 0 || index >= arrSize) {
            // @todo error
            return NULL;
        }
        void * arrStartAddr = CMETA_OFFSET(obj, field->offset);
        size_t itemSize = field->type->size;
        void * itemAddr = CMETA_OFFSET(arrStartAddr, index * itemSize);
        return itemAddr;
    }
}

void cmeta_setArrayItem(cmeta_object_t * self, const char * fieldName, int index, void * value) {
    const cmeta_struct_t * meta = self->type;
    const void * obj = self->ptr;
    const cmeta_field_t * field = cmeta_struct_getField(meta, fieldName);
    if ((NULL == field) || !cmeta_isArray(field)) {
        return;
    }
     if (field->isPointer) {
        // @todo
        printf("Not implemented!\n");
        return;
    } else {
        size_t arrSize = field->arrSize;
        if (index < 0 || index >= arrSize) {
            // @todo error
            return;
        }
        void * arrStartAddr = CMETA_OFFSET(obj, field->offset);
        size_t itemSize = field->type->size;
        void * itemAddr =  CMETA_OFFSET(arrStartAddr, index * itemSize);
        memcpy(itemAddr, value, itemSize);
    }
}
