#pragma once
#include <drogon/WebSocketController.h>
#include <drogon/PubSubService.h>
#include <set>
#include <mutex>

using namespace drogon;

class SensorWebSocket : public drogon::WebSocketController<SensorWebSocket>
{
  public:
    void handleNewMessage(const WebSocketConnectionPtr &,
                          std::string &&,
                          const WebSocketMessageType &) override;
    void handleNewConnection(const HttpRequestPtr &,
                             const WebSocketConnectionPtr &) override;
    void handleConnectionClosed(const WebSocketConnectionPtr &) override;

    static void broadcast(const Json::Value &message);

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/ws/sensors");
    WS_PATH_LIST_END
    
  private:
    static std::set<WebSocketConnectionPtr> m_connections;
    static std::mutex m_connectionsMutex;
};
