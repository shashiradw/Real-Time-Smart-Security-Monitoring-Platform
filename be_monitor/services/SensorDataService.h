#pragma once
#include <drogon/drogon.h>
#include <string>
#include <optional>

namespace services {
    class SensorDataService {
    public:
        void saveReading(const std::string& deviceId, 
                         const std::string& sensorType, 
                         double value, 
                         const std::string& unit, 
                         const std::string& timestamp);
    };
}
