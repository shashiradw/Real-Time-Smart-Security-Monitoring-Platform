#include "AuthController.h"
#include "../../services/UserService.h"
#include "../../services/JwtService.h"

namespace api {
    void AuthController::login(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) {
        auto json = req->getJsonObject();
        if(!json) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k400BadRequest);
            resp->setBody("Invalid JSON");
            callback(resp);
            return;
        }

        std::string username = (*json)["username"].asString();
        std::string password = (*json)["password"].asString();

        // In a real app we'd fetch the user ID too.
        // For now, we will just fetch the User object.
        auto user = services::UserService::findUserByUsername(username); 

        if (user && services::UserService::validateCredentials(username, password)) {
            // Generate JWT
            std::string token = services::JwtService::generateToken(user->id, username);
            auto resp = HttpResponse::newHttpJsonResponse(Json::Value());
            resp->setStatusCode(k200OK);
            Json::Value ret;
            ret["status"] = "ok";
            ret["message"] = "Logged in";
            ret["token"] = token;
            resp->setBody(ret.toStyledString());
            callback(resp);
        } else {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k401Unauthorized);
            resp->setBody("Invalid credentials");
            callback(resp);
        }
    }

    void AuthController::logout(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) {
        // Stateless logout (client should discard token)
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value());
        Json::Value ret;
        ret["status"] = "ok";
        ret["message"] = "Logged out (stateless)";
        resp->setBody(ret.toStyledString());
        callback(resp);
    }

    void AuthController::registerUser(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) {
        auto json = req->getJsonObject();
        if(!json) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }

        std::string username = (*json)["username"].asString();
        std::string password = (*json)["password"].asString();
        std::string email = (*json)["email"].asString();

        auto user = services::UserService::createUser(username, email, password);
        if (user) {
            Json::Value ret;
            ret["status"] = "ok";
            ret["id"] = user->id;
            ret["username"] = user->username;
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            callback(resp);
        } else {
             auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k500InternalServerError);
            resp->setBody("Failed to create user (might already exist)");
            callback(resp);
        }
    }
    
    void AuthController::me(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) {
        // Determine user from JWT Header manually (since we didn't inject it in filter yet perfectly)
        // OR rely on filter rejection. If we reach here, filter passed.
        
        std::string token;
        auto authHeader = req->getHeader("Authorization");
        if (!authHeader.empty()) {
            size_t pos = authHeader.find("Bearer ");
            if (pos != std::string::npos) {
                token = authHeader.substr(pos + 7);
            }
        }
        
        auto username = services::JwtService::verifyToken(token);
    
        if (username) {
            Json::Value ret;
            ret["username"] = *username;
            ret["status"] = "authenticated";
            ret["method"] = "jwt";
            
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            callback(resp);
        } else {
            // Should be caught by filter ideally
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k401Unauthorized);
            callback(resp);
        }
    }
}
