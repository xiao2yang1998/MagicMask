#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h> 
#include <stdlib.h>
#include <ctype.h>
#define MAXSIZE 10

typedef struct
{
	char key[20];
	struct {
		int			intValue;
		double		doubleValue;
		char		stringValue[20];
		int		arrayValue[3];
		struct KeyValue* childList;
	} value;
	
} KeyValue;
extern KeyValue keyValueList[MAXSIZE];
extern KeyValue keyValueList_1[MAXSIZE];
extern int length;
extern char str[200];
//typedef struct
//{
//	KeyValue keyValues[MAXSIZE];
//	int length;
//} KeyValueList;

//void setInt(KeyValueList* list,char* key,int value);
//void setDouble(KeyValueList* list,char* key,double value);
//void setString(KeyValueList* list,char* key,char* value);
//void setArray(KeyValueList* list,char* key,int value[3]);
//void setJSON(KeyValueList* list,char* key,KeyValue* childList);
//char* toString(KeyValueList* list);
//char* toJSONString(value_t* value,char* str);
void initKeyValueList(KeyValue keyValueList[MAXSIZE]);
void setInt(KeyValue* list,char* key,int value);
void setDouble(KeyValue* list,char* key,double value);
void setString(KeyValue* list,char* key,char* value);
void setArray(KeyValue* list,char* key,int value[3]);
void setJSON(KeyValue* list,char* key,KeyValue* childList);
char* toString(KeyValue* list);
char* toJSONString(KeyValue keyValue,char* str);
