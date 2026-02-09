#pragma once
#include <string>
#include <optional>
#include <vector>
#include <chrono>
#include <drogon/drogon.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

namespace services {
    class JwtService {
    public:
         /**
          * Generate a JWT for the given user.
          * Payload: { "sub": username, "id": userId, "exp": ... }
          */
         static std::string generateToken(int userId, const std::string& username);

         /**
          * Verify the token and return the username if valid.
          * Returns nullopt if invalid or expired.
          */
         static std::optional<std::string> verifyToken(const std::string& token);

    private:
         static std::string base64UrlEncode(const std::string& data);
         static std::string base64UrlDecode(const std::string& data);
         static std::string hmacSha256(const std::string& data, const std::string& key);
         static const std::string SECRET_KEY;
    };
}
