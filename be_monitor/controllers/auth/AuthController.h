#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

namespace api {
    class AuthController : public drogon::HttpController<AuthController> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(AuthController::login, "/login", Post);
        METHOD_ADD(AuthController::logout, "/logout", Post);
        METHOD_ADD(AuthController::registerUser, "/register", Post);
        METHOD_ADD(AuthController::me, "/me", Get);
        METHOD_LIST_END

        void login(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
        void logout(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
        void registerUser(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
        void me(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
    };
}
