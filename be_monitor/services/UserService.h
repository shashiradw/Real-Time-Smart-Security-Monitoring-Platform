#pragma once
#include "../models/User.h"
#include <drogon/drogon.h>
#include <optional>

namespace services {
    class UserService {
    public:
        static std::optional<models::User> findUserByUsername(const std::string& username);
        static std::optional<models::User> createUser(const std::string& username, const std::string& email, const std::string& password);
        static bool validateCredentials(const std::string& username, const std::string& password);
    };
}
