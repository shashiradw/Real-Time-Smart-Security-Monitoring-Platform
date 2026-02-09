#include "SensorWebSocket.h"
#include <trantor/utils/Logger.h>
#include <json/json.h>

std::set<WebSocketConnectionPtr> SensorWebSocket::m_connections;
std::mutex SensorWebSocket::m_connectionsMutex;

void SensorWebSocket::handleNewMessage(const WebSocketConnectionPtr &wsConnPtr,
                                       std::string &&message,
                                       const WebSocketMessageType &type)
{
    LOG_DEBUG << "New WebSocket message: " << message;
}

void SensorWebSocket::handleNewConnection(const HttpRequestPtr &req,
                                          const WebSocketConnectionPtr &wsConnPtr)
{
    LOG_INFO << "New WebSocket connection from " << req->getPeerAddr().toIpPort();
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    m_connections.insert(wsConnPtr);
}

void SensorWebSocket::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr)
{
    LOG_INFO << "WebSocket connection closed";
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    m_connections.erase(wsConnPtr);
}

void SensorWebSocket::broadcast(const Json::Value &message)
{
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string messageStr = Json::writeString(builder, message);

    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    for (auto &conn : m_connections)
    {
        if (conn->connected())
        {
            conn->send(messageStr);
        }
    }
}
