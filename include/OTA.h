#ifndef __OTA_H__
#define __OTA_H__

#include <HTTPClient.h>
#include <Update.h>

#define CURRENT_VERSION "1.0.1"
#define VARIANT "esp32"
#define CLOUD_FUNCTION_URL "https://us-central1-batteryswapstation.cloudfunctions.net/getDownloadUrl"

String getDownloadUrl();
bool downloadUpdate(String url);

#endif