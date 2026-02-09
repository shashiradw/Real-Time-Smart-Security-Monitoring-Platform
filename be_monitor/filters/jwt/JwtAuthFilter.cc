#include "JwtAuthFilter.h"
#include <drogon/drogon.h>

namespace filters {

    void JwtAuthFilter::doFilter(const drogon::HttpRequestPtr &req,
                                 drogon::FilterCallback &&fcb,
                                 drogon::FilterChainCallback &&fccb) {
        
        std::string token;
        auto authHeader = req->getHeader("Authorization");
        if (!authHeader.empty()) {
            size_t pos = authHeader.find("Bearer ");
            if (pos != std::string::npos) {
                token = authHeader.substr(pos + 7);
            }
        }

        if (token.empty()) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k401Unauthorized);
            resp->setBody("Missing or invalid Authorization header");
            fcb(resp);
            return;
        }

        auto username = services::JwtService::verifyToken(token);
        if (username) {
            fccb();
            return;
        }

        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setBody("Invalid or expired token");
        fcb(resp);
    }
}
