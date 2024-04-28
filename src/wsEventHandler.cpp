#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "wsEventHandler.h"

AsyncWebSocketClient *clients[16];

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
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
          Serial.printf("%02x:", data[i]);
        }
        file.close();
      }
    }
  }
}

void wsEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
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