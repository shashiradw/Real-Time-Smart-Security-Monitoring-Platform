#pragma once
#include <drogon/plugins/Plugin.h>
#include <mosquitto.h>
#include <memory>
#include <thread>
#include <atomic>

class MqttPlugin : public drogon::Plugin<MqttPlugin> {
public:
    MqttPlugin() {}
    void initAndStart(const Json::Value &config) override;
    void shutdown() override;

private:
    struct mosquitto *m_mosq = nullptr;
    std::string m_broker_host = "localhost";
    int m_broker_port = 1883;
    std::string m_topic = "devices/#";
    
    std::thread m_loop_thread;
    std::atomic<bool> m_running{false};

    static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
    static void on_connect(struct mosquitto *mosq, void *obj, int rc);
};
