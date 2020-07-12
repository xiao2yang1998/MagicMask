#include "json.h"
KeyValue keyValueList[MAXSIZE];
KeyValue keyValueList_1[MAXSIZE];
int length = 0;
char str[200];

static char tmp_str[10];

KeyValue* keyValueList_tmp;

//char* toJSONString(value_t* value,char* str);

//KeyValue keyValues_init[10];
void initKeyValueList(KeyValue keyValueList[MAXSIZE]){
	memset(keyValueList,0,sizeof(KeyValue)*MAXSIZE);
	
//	KeyValue keyValue;
//	memset(keyValue.key,0,sizeof(keyValue.key));
//	keyValue.value.intValue = 0;
//	keyValue.value.doubleValue = 0.0;
//	memset(keyValue.value.stringValue,0,sizeof(keyValue.value.stringValue));
//	memset(keyValue.value.arrayValue,0,sizeof(keyValue.value.arrayValue));
//	
//	for(int i = 0;i < MAXSIZE; i++){
//		keyValues_init[i] = keyValue;
//	}
//	keyValue.value.childList = (struct KeyValue*)keyValues_init;
}
	
//add element   
int addKeyValue(KeyValue* list,KeyValue keyValue){
	
	printf("======the length of list is %d\n",length);
	if(length == MAXSIZE) printf("the list is full");
	list[length++] = keyValue;
    printf("now===%s  %f\n",keyValue.key,keyValue.value.doubleValue);
	if(list[0].value.childList){
		//	initKeyValueList(keyValueList_tmp);
		keyValueList_tmp = (KeyValue*)list[0].value.childList;
		printf("childList====%s  %s  %s\n",list[0].key,keyValueList_tmp[0].key,keyValueList_tmp[0].value.stringValue);
//		printf("log-2====%s  %s\n",list[0].key,list[0].value.stringValue);
	}
	
	
}
KeyValue keyValue;
void setInt(KeyValue* list,char* key,int value){

	memset(&keyValue,0,sizeof(keyValue));
	strcpy(keyValue.key,key);
//	keyValue.key = key;
	keyValue.value.intValue = value;
	addKeyValue(list,keyValue);
}

void setDouble(KeyValue* list,char* key,double value){

	strcpy(keyValue.key,key);
//	keyValue.key = key;
	keyValue.value.doubleValue = value;
	addKeyValue(list,keyValue);
}

void setString(KeyValue* list,char* key,char* value){

	memset(&keyValue,0,sizeof(keyValue));
	strcpy(keyValue.key,key);
//	printf("log-2====%s  %d\n",list[0].key,list[0].value.intValue);
//	keyValue.key = key;
	strcpy(keyValue.value.stringValue,value);
//	printf("log-2====%s  %d\n",list[0].key,list[0].value.intValue);
//	keyValue.value.stringValue = value;
//	printf("log-2====%s  %s\n",keyValue.key,keyValue.value.stringValue);
	addKeyValue(list,keyValue);

}

void setArray(KeyValue* list,char* key,int value[3]){

	memset(&keyValue,0,sizeof(keyValue));
	strcpy(keyValue.key,key);
//	keyValue.key = key;
	for(int i = 0; i < 3; i++){
		keyValue.value.arrayValue[i] = value[i];
	}
	addKeyValue(list,keyValue);
}

void setJSON(KeyValue* list,char* key,KeyValue* childList){

	memset(&keyValue,0,sizeof(keyValue));
	strcpy(keyValue.key,key);
//	keyValue.key = key;
	keyValue.value.childList = (struct KeyValue* )childList;
//	printf("1====%s  %s  %s\n",keyValue.key,childList[0].key,childList[0].value.stringValue);
	
//	initKeyValueList(keyValueList_tmp);
//	keyValueList_tmp = (KeyValue*)keyValue.value.childList;
//	printf("1====%s  %s  %s\n",keyValue.key,keyValueList_tmp[0].key,keyValueList_tmp[0].value.stringValue);
	addKeyValue(list,keyValue);
}

bool child_flag = false;
//char* quotaMark = "\"";
char* toString(KeyValue* list){

//	printf("======the length of length is %d\n",length);
	strcat(str,"{");
	if(length==0){
		strcat(str,"}");
	}else{
		for(int i = 0;i<length;i++){
			if(child_flag){
				strcat(str,"\\");
			}
			strcat(str,"\"");
            printf("=====%s\n",list[i].key);
			strcat(str,list[i].key);
            
			if(child_flag){
				strcat(str,"\\");
			}
			strcat(str,"\":");
			printf("=======The str is %s\n",str);
//			printf("======log0%s\n",list->keyValues[i].value.stringValue);
			strcat(str,toJSONString(list[i],str));
			printf("======log0%s\n",str);
			if(i!=length-1){
				strcat(str,",");
			}
		}
		strcat(str,"}");
	}
	
	return str;
}


char* toJSONString(KeyValue keyValue,char* string){
//	keyValue.value->arrayValue[0] && value->arrayValue[1] && value->arrayValue[2]
	if(keyValue.value.arrayValue[0] && keyValue.value.arrayValue[1] && keyValue.value.arrayValue[2]){
		printf("======log1\n");
		strcat(string,"[");
		for(int i = 0;i < 3;i++){
			memset(string,0,sizeof(tmp_str));
			sprintf(tmp_str,"%d",keyValue.value.arrayValue[i]);
			if(i!=2){			
				strcat(strcat(string,tmp_str),",");
			}
		}
		strcat(string,"]");
        printf("======OK1\n");
	}else if(keyValue.value.intValue){
		printf("======log2\n");
		sprintf(tmp_str,"%d",keyValue.value.intValue);
		strcat(string,tmp_str);
        printf("======OK2\n");
	}else if(keyValue.value.doubleValue){
		printf("======log3\n");
		sprintf(tmp_str,"%f",keyValue.value.doubleValue);
		strcat(string,tmp_str);
        printf("======OK3\n");
	}else if(keyValue.value.stringValue[0]){	//?
		printf("======log4 %s \n",keyValue.value.stringValue);
		if(child_flag){
			strcat(str,"\\");
		}
		strcat(string,"\"");
		strcat(string,keyValue.value.stringValue);
		if(child_flag){
			strcat(str,"\\");
		}
		strcat(string,"\"");
//		strcat(strcat(strcat(string,"\""),keyValue.value.stringValue),"\"");
		printf("======OK4\n");
	}else if(keyValue.value.childList){
		child_flag = true;
		printf("======log5 \n");
		strcat(string,"\"");
		toString((KeyValue* )keyValue.value.childList);
		child_flag = false;			//reset is right？
		strcat(string,"\"");
		printf("======OK5\n");
//		char* str_child = toString((KeyValue* )keyValue.value.childList);
//		printf("======log6 %s\n",str_child);
//		strcat(string,str_child);
//		printf("======log5_1\n");
	}
}

