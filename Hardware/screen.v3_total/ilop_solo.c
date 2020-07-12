//#include "ilop.h"
//#include "mx_common.h"
//#include "mx_debug.h"
#include <stdio.h>
#include "cJSON.h"

//void user_ilop_status_cb(ilop_status_t status)
//{
//    switch (status) {
//    case ILOP_STATUS_WLAN_CONFIG_STARTED:
//        app_log("config mode...");
//        break;
//    case ILOP_STATUS_WLAN_CONNECTED:
//        app_log("wifi connected");
//        break;
//    case ILOP_STATUS_WLAN_DISCONNECTED:
//        app_log("wifi disconnect");
//        break;
//    case ILOP_STATUS_CLOUD_CONNECTED:
//        app_log("cloud connected");
//        break;
//    case ILOP_STATUS_CLOUD_DISCONNECTED:
//        app_log("cloud disconnect");
//        break;
//    default:
//        break;
//    }
//}

//void user_ilop_service_set_cb(char *data, uint32_t len)
//{
//    app_log("service set:%s", data);
//}

//void user_ilop_rawdata_set_cb(uint8_t data, uint32_t len)
//{
//    app_log("rawdata set");
//}

void user_ilop_property_get_cb(void)
{
    char* response_string = NULL;
    printf("property get");

    /* Prepare Response */
    cJSON *response_root = cJSON_CreateObject();
    if (response_root == NULL) {
        printf("No Enough Memory");
        return;
    }

    cJSON_AddNumberToObject(response_root, "PowerSwitch", 1);
    cJSON_AddNumberToObject(response_root, "Brightness", 100);
    cJSON* item_hsvcolor = cJSON_CreateObject();
    cJSON_AddNumberToObject(item_hsvcolor, "Hue", 0);
    cJSON_AddNumberToObject(item_hsvcolor, "Saturation", 100);
    cJSON_AddNumberToObject(item_hsvcolor, "Value", 100);
    cJSON_AddItemToObject(response_root, "HSVColor", item_hsvcolor);

    response_string = cJSON_PrintUnformatted(response_root);
    if (response_string == NULL) {
        app_log("No Enough Memory");
        cJSON_Delete(response_root);
    }
    cJSON_Delete(response_root);

    app_log("%s", response_string);
    ilop_report_property(response_string, strlen(response_string));

    if (response_string) {
        free(response_string);
    }
}

void user_ilop_property_set_cb(char* data, uint32_t len)
{
    cJSON *request_root = NULL, *LightSwitch = NULL, *Brightness = NULL;
    cJSON *HSVColor = NULL, *Saturation = NULL, *Value = NULL, *Hue = NULL;
    app_log("property set:%s", data);

    /* Parse Request */
    request_root = cJSON_Parse(data);
    if (request_root == NULL || !cJSON_IsObject(request_root)) {
        app_log("JSON Parse Error");
        return;
    }

    /* Try To Find LocalTimer Property */
    LightSwitch = cJSON_GetObjectItem(request_root, "LightSwitch");
    if (LightSwitch != NULL) {
        app_log("LightSwitch:%d", LightSwitch->valueint);
    }

    Brightness = cJSON_GetObjectItem(request_root, "Brightness");
    if (Brightness != NULL) {
        app_log("Brightness:%d", Brightness->valueint);
    }

    HSVColor = cJSON_GetObjectItem(request_root, "HSVColor");
    if (HSVColor != NULL) {
        Saturation = cJSON_GetObjectItem(HSVColor, "Saturation");
        Value = cJSON_GetObjectItem(HSVColor, "Value");
        Hue = cJSON_GetObjectItem(HSVColor, "Hue");

        app_log("Hue:%d Saturation:%d Value:%d", Hue->valueint, Saturation->valueint, Value->valueint);
    }

    cJSON_Delete(request_root);

    ilop_report_property(data, len);
}

void user_ilop_loop(void)
{
    return;
}