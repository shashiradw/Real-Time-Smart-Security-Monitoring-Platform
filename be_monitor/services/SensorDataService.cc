#include "SensorDataService.h"
#include "models/SensorData.h"
#include "controllers/SensorWebSocket.h"
#include <algorithm>

namespace services {
    void SensorDataService::saveReading(const std::string& deviceId, 
                                      const std::string& sensorType, 
                                      double value, 
                                      const std::string& unit, 
                                      const std::string& timestamp) {
        
        drogon_model::ddmdb::SensorData reading;
        reading.setDeviceId(deviceId);
        reading.setSensorType(sensorType);
        reading.setValue(value);
        reading.setUnit(unit);

        if (!timestamp.empty()) {
             std::string ts = timestamp;
             std::replace(ts.begin(), ts.end(), 'T', ' ');
             reading.setTimestamp(trantor::Date::fromDbStringLocal(ts));
        } else {
             reading.setTimestamp(trantor::Date::now());
        }

        auto client = drogon::app().getDbClient();
        drogon::orm::Mapper<drogon_model::ddmdb::SensorData> mapper(client);

        mapper.insert(
            reading,
            [deviceId, sensorType](const drogon_model::ddmdb::SensorData& r) {
                LOG_DEBUG << "Saved reading for " << deviceId << " (" << sensorType << ")";
                // If saved successfully, broadcast the reading to all connected WebSocket clients
                SensorWebSocket::broadcast(r.toJson());
            },
            [](const drogon::orm::DrogonDbException& e) {
                LOG_ERROR << "Failed to save sensor reading: " << e.base().what();
            }
        );
    }
}
