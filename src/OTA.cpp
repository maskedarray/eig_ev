#include "OTA.h"

WiFiClient client;

String getDownloadUrl()
{
    HTTPClient http;
    String downloadUrl;
    log_i("[HTTP] begin...\n");

    String url = CLOUD_FUNCTION_URL;
    url += String("?version=") + CURRENT_VERSION;
    url += String("&variant=") + VARIANT;
    http.begin(url);

    log_i("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        log_i("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            log_i("%s", payload.c_str());
            downloadUrl = payload;
        }
        else
        {
            log_i("Device is up to date!");
        }
    }
    else
    {
        log_i("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();

    return downloadUrl;
}

bool downloadUpdate(String url)
{
    HTTPClient http;
    log_i("[HTTP] Download begin...\n");

    http.begin(url);

    log_i("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // HTTP header has been send and Server response header has been handled
        log_i("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {

            int contentLength = http.getSize();
            log_i("contentLength : %s\n", String(contentLength));

            if (contentLength > 0)
            {
                bool canBegin = Update.begin(contentLength);
                if (canBegin)
                {
                    WiFiClient &stream = http.getStream();
                    log_i("\nBegin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!\n");
                    size_t written = Update.writeStream(stream);

                    if (written == contentLength)
                    {
                        log_i("Written : %s successfully\n", String(written));
                    }
                    else
                    {
                        log_i("Written only : %s/%s. Retry?", String(written), String(contentLength));
                    }

                    if (Update.end())
                    {
                        log_i("OTA done!\n");
                        if (Update.isFinished())
                        {
                            log_i("Update successfully completed. Rebooting.\n");
                            ESP.restart();
                            return true;
                        }
                        else
                        {
                            log_i("Update not finished? Something went wrong!\n");
                            return false;
                        }
                    }
                    else
                    {
                        log_i("Error Occurred. Error #: %s\n", String(Update.getError()));
                        return false;
                    }
                }
                else
                {
                    log_i("Not enough space to begin OTA\n");
                    client.flush();
                    return false;
                }
            }
            else
            {
                log_i("There was no content in the response\n");
                client.flush();
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}