#pragma once

#include <drogon/HttpController.h>
#include <drogon/orm/Mapper.h>
#include <json/json.h>

using namespace drogon;

namespace api
{
    namespace v1
    {
        class DeviceController : public drogon::HttpController<DeviceController>
        {
            public:
                METHOD_LIST_BEGIN
                ADD_METHOD_TO(DeviceController::getDevices, "/api/devices", Get, Options);
                ADD_METHOD_TO(DeviceController::createDevice, "/api/devices", Post, Options);
                METHOD_LIST_END

                void getDevices(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
                void createDevice(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
        };
    }
}
