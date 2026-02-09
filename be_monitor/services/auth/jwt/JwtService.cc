#include "JwtService.h"
#include <json/json.h>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/buffer.h>
#include <drogon/utils/Utilities.h>

namespace services {

    const std::string JwtService::SECRET_KEY = "XYZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";

    std::string JwtService::base64UrlEncode(const std::string& data) {
        std::string b64 = drogon::utils::base64Encode(data);
        std::string ret;
        ret.reserve(b64.size());
        for (char c : b64) {
            if (c == '+') ret.push_back('-');
            else if (c == '/') ret.push_back('_');
            else if (c == '=') continue;
            else if (c == '\n') continue;
            else ret.push_back(c);
        }
        return ret;
    }

    std::string JwtService::base64UrlDecode(const std::string& input) {
        std::string data = input;
        std::replace(data.begin(), data.end(), '-', '+');
        std::replace(data.begin(), data.end(), '_', '/');
        // Add padding
        switch (data.size() % 4) {
            case 2: data += "=="; break;
            case 3: data += "="; break;
        }
        return drogon::utils::base64Decode(data);
    }

    std::string JwtService::hmacSha256(const std::string& data, const std::string& key) {
        unsigned char* result;
        unsigned int len = EVP_MAX_MD_SIZE;
        unsigned char digest[EVP_MAX_MD_SIZE];

        HMAC(EVP_sha256(), key.c_str(), key.length(), 
             (unsigned char*)data.c_str(), data.length(), digest, &len);

        return std::string((char*)digest, len);
    }

    std::string JwtService::generateToken(int userId, const std::string& username) {
        // Header
        Json::Value header;
        header["alg"] = "HS256";
        header["typ"] = "JWT";
        
        // Payload
        Json::Value payload;
        payload["sub"] = username;
        payload["id"] = userId;

        // Expire in 1 hours
        auto now = std::chrono::system_clock::now();
        payload["exp"] = (Json::UInt64)std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() + 3600;
        
        Json::FastWriter writer;
        std::string headerStr = base64UrlEncode(writer.write(header));
        // Remove FastWriter added new line
        if (!headerStr.empty() && headerStr.back() == '\n') headerStr.pop_back(); 

        std::string rawPayload = writer.write(payload);
        // FastWriter appends newline
        if (!rawPayload.empty() && rawPayload.back() == '\n') rawPayload.pop_back();

        std::string payloadStr = base64UrlEncode(rawPayload);
        
        std::string rawHeader = writer.write(header);
        if (!rawHeader.empty() && rawHeader.back() == '\n') rawHeader.pop_back();
        headerStr = base64UrlEncode(rawHeader);

        std::string signature = hmacSha256(headerStr + "." + payloadStr, SECRET_KEY);
        std::string signatureStr = base64UrlEncode(signature);

        return headerStr + "." + payloadStr + "." + signatureStr;
    }

    std::optional<std::string> JwtService::verifyToken(const std::string& token) {
        size_t firstDot = token.find('.');
        size_t secondDot = token.find('.', firstDot + 1);

        if (firstDot == std::string::npos || secondDot == std::string::npos) {
            return std::nullopt;
        }

        std::string headerStr = token.substr(0, firstDot);
        std::string payloadStr = token.substr(firstDot + 1, secondDot - firstDot - 1);
        std::string signatureStr = token.substr(secondDot + 1);

        std::string signature = base64UrlDecode(signatureStr);
        std::string expectedSignature = hmacSha256(headerStr + "." + payloadStr, SECRET_KEY);

        if (signature != expectedSignature) {
            return std::nullopt;
        }

        std::string decodedPayload = base64UrlDecode(payloadStr);
        Json::Value payload;
        Json::Reader reader;
        if (!reader.parse(decodedPayload, payload)) {
             return std::nullopt;
        }

        if (payload.isMember("exp")) {
             auto exp = payload["exp"].asUInt64();
             auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
             if (now > exp) {
                 return std::nullopt;
             }
        }

        if (payload.isMember("sub")) {
            return payload["sub"].asString();
        }

        return std::nullopt;
    }
}
