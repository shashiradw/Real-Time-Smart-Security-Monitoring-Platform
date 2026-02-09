#include "DeviceController.h"
#include "models/Device.h"
#include <drogon/orm/Mapper.h>
#include <trantor/utils/Logger.h>

using namespace api::v1;
using namespace drogon_model::ddmdb;

void DeviceController::getDevices(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    auto client = app().getDbClient();
    drogon::orm::Mapper<Device> mapper(client);

    mapper.findAll(
        [callback](const std::vector<Device> &devices) {
            Json::Value ret;
            ret["devices"] = Json::arrayValue;
            for (const auto &device : devices)
            {
                ret["devices"].append(device.toJson());
            }
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->addHeader("Access-Control-Allow-Origin", "*");
            callback(resp);
        },
        [callback](const drogon::orm::DrogonDbException &e) {
            LOG_ERROR << "Error finding devices: " << e.base().what();
            Json::Value ret;
            ret["error"] = "Internal Server Error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            resp->addHeader("Access-Control-Allow-Origin", "*");
            callback(resp);
        }
    );
}

void DeviceController::createDevice(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr)
    {
        Json::Value ret;
        ret["error"] = "Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    std::string name = jsonPtr->get("name", "").asString();
    std::string id = jsonPtr->get("device_id", "").asString();

    if (name.empty() || id.empty())
    {
        Json::Value ret;
        ret["error"] = "Name and device_id serve required";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        callback(resp);
        return;
    }

    Device device;
    device.setName(name);
    device.setDeviceId(id);

    // Create current timestamp
    auto now = trantor::Date::now();
    device.setCreatedAt(now);
    device.setUpdatedAt(now);

    auto client = app().getDbClient();
    drogon::orm::Mapper<Device> mapper(client);

    mapper.insert(
        device,
        [callback](const Device &d) {
            Json::Value ret;
            ret["status"] = "ok";
            ret["device"] = d.toJson();
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k201Created);
            resp->addHeader("Access-Control-Allow-Origin", "*");
            callback(resp);
        },
        [callback](const drogon::orm::DrogonDbException &e) {
            LOG_ERROR << "Error creating device: " << e.base().what();
            Json::Value ret;
            ret["error"] = "Failed to create device (duplicate ID?)";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            resp->addHeader("Access-Control-Allow-Origin", "*");
            callback(resp);
        }
    );
}
