#include "UserService.h"
#include <drogon/utils/Utilities.h>

namespace services {

    std::optional<models::User> UserService::findUserByUsername(const std::string& username) {
        auto client = drogon::app().getDbClient();
        try {
            auto f = client->execSqlAsyncFuture("SELECT * FROM users WHERE username = $1", username);
            auto result = f.get();
            if (result.size() > 0) {
                return models::User::fromRow(result[0]);
            }
        } catch (const std::exception& e) {
            LOG_ERROR << "Database error in findUserByUsername: " << e.what();
        }
        return std::nullopt;
    }

    std::optional<models::User> UserService::createUser(const std::string& username, const std::string& email, const std::string& password) {
        auto client = drogon::app().getDbClient();
        std::string password_hash = drogon::utils::getMd5(password);
        
        try {
            auto f = client->execSqlAsyncFuture(
                "INSERT INTO users (username, email, password_hash) VALUES ($1, $2, $3) RETURNING *",
                username, email, password_hash
            );
            auto result = f.get();
            if (result.size() > 0) {
                return models::User::fromRow(result[0]);
            }
        } catch (const std::exception& e) {
             LOG_ERROR << "Database error in createUser: " << e.what();
        }
        return std::nullopt;
    }

    bool UserService::validateCredentials(const std::string& username, const std::string& password) {
        auto user = findUserByUsername(username);
        if (!user) return false;
        
        std::string input_hash = drogon::utils::getMd5(password);
        return input_hash == user->password_hash;
    }
}
