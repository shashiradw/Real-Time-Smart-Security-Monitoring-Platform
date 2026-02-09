#include "MqttPlugin.h"
#include "services/SensorDataService.h"
#include <drogon/drogon.h>
#include <json/json.h>

void MqttPlugin::initAndStart(const Json::Value &config) {
    m_broker_host = config.get("broker_host", "localhost").asString();
    m_broker_port = config.get("broker_port", 1883).asInt();
    m_topic = config.get("topic", "devices/#").asString();

    LOG_INFO << "Initializing MQTT Plugin, connecting to " << broker_host << ":" << broker_port;

    mosquitto_lib_init();
    m_mosq = mosquitto_new("drogon_backend", true, this);
    
    if (!m_mosq) {
        LOG_ERROR << "Failed to create mosquitto instance";
        return;
    }

    mosquitto_connect_callback_set(m_mosq, on_connect);
    mosquitto_message_callback_set(m_mosq, on_message);

    int rc = mosquitto_connect_bind(m_mosq, m_broker_host.c_str(), m_broker_port, 60, NULL);
    if (rc != MOSQ_ERR_SUCCESS) {
        LOG_ERROR << "Failed to connect to MQTT broker: " << mosquitto_strerror(rc);
        return;
    }

    m_running = true;
    m_loop_thread = std::thread([this]() {
        while (m_running) {
            int rc = mosquitto_loop(m_mosq, 1000, 1);
            if (rc != MOSQ_ERR_SUCCESS) {
                LOG_WARN << "MQTT loop error: " << mosquitto_strerror(rc);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                mosquitto_reconnect(m_mosq);
            }
        }
    });
}

void MqttPlugin::shutdown() {
    LOG_INFO << "Shutting down MQTT Plugin";
    m_running = false;
    if (m_loop_thread.joinable()) {
        m_loop_thread.join();
    }
    if (m_mosq) {
        mosquitto_destroy(m_mosq);
    }
    mosquitto_lib_cleanup();
}

void MqttPlugin::on_connect(struct mosquitto *mosq, void *obj, int rc) {
    MqttPlugin *self = static_cast<MqttPlugin *>(obj);
    if (rc == 0) {
        LOG_INFO << "Connected to MQTT broker, subscribing to " << self->m_topic;
        mosquitto_subscribe(mosq, NULL, self->m_topic.c_str(), 0);
    } else {
        LOG_ERROR << "Failed to connect to MQTT broker, code: " << rc;
    }
}

void MqttPlugin::on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    if (!msg->payload) return;

    std::string payload(static_cast<char *>(msg->payload), msg->payloadlen);
    LOG_DEBUG << "Received MQTT message: " << payload;

    Json::Value root;
    Json::Reader reader;
    if (reader.parse(payload, root)) {
        try {
            std::string deviceId = root.get("device_id", "unknown").asString();
            std::string sensorType = root.get("type", "unknown").asString();
            double value = root.get("current", 0.0).asDouble(); 
            std::string unit = root.get("unit", "N/A").asString();
            std::string timestamp = root.get("timestamp", "").asString();

            services::SensorDataService service;
            service.saveReading(deviceId, sensorType, value, unit, timestamp);
        } catch (const std::exception &e) {
            LOG_ERROR << "Failed to process MQTT payload: " << e.what();
        }
    } else {
        LOG_WARN << "Failed to parse MQTT JSON payload";
    }
}
