#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wsEventHandler.h"

AsyncWebSocketClient *clients[16];

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  Serial.begin(115200);
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (LittleFS.begin())
    {
      File file = LittleFS.open("/data.json", "w");
      if (file)
      {
        file.write(data, len);
        for (int i = 0; i < len; i++)
        {
          Serial.printf("%c", data[i]);
        }
        file.close();
        LittleFS.end();
        File file = LittleFS.open("/data.json", "r");
        LittleFS.begin();
        if (file)
        {
          while (file.available())
          {
            Serial.write(file.read());
          }
          file.close();
        }
        ESP.restart();
      }
    }
  }
}

void wsEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  Serial.begin(115200);
  if (type == WS_EVT_DATA)
  {
    handleWebSocketMessage(arg, data, len);
  }
  else if (type == WS_EVT_CONNECT)
  {
    Serial.println("Websocket client connection received");
    // ACK with current state
    client->text("ACK");
    // store connected client
    for (int i = 0; i < 16; ++i)
      if (clients[i] == NULL)
      {
        clients[i] = client;
        break;
      }
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.println("Client disconnected");
    // remove client from storage
    for (int i = 0; i < 16; ++i)
      if (clients[i] == client)
        clients[i] = NULL;
  }
}