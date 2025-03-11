#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include "includes.h"

// 当升级开始时，打印日志
void update_started()
{
    LOG_PRINTLN("CALLBACK:  HTTP update process started");
}

// 当升级结束时，打印日志
void update_finished()
{
    LOG_PRINTLN("CALLBACK:  HTTP update process finished");
}

// 当升级中，打印日志
void update_progress(int cur, int total)
{
    LOG_PRINTF("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

// 当升级失败时，打印日志
void update_error(int err)
{
    LOG_PRINTF("CALLBACK:  HTTP update fatal error code %d\n", err);
}

/**
 * 固件升级函数
 * 通过http请求获取远程固件，实现升级
 */
void updateBin(String url)
{
    LOG_PRINTLN("start update");
    WiFiClient UpdateClient;

    ESPhttpUpdate.onStart(update_started);     // 当升级开始时
    ESPhttpUpdate.onEnd(update_finished);      // 当升级结束时
    ESPhttpUpdate.onProgress(update_progress); // 当升级中
    ESPhttpUpdate.onError(update_error);       // 当升级失败时

    t_httpUpdate_return ret = ESPhttpUpdate.update(UpdateClient, url);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED: // 当升级失败
        LOG_PRINTLN("[update] Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES: // 当无升级
        LOG_PRINTLN("[update] Update no Update.");
        break;
    case HTTP_UPDATE_OK: // 当升级成功
        LOG_PRINTLN("[update] Update ok.");
        delay(1000);
        ESP.restart();
        break;
    }
}
