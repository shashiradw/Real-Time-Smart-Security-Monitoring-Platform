#pragma once
#include <drogon/HttpFilter.h>
#include "../services/JwtService.h"

namespace filters {
    class JwtAuthFilter : public drogon::HttpFilter<JwtAuthFilter> {
    public:
        void doFilter(const drogon::HttpRequestPtr &req,
                      drogon::FilterCallback &&fcb,
                      drogon::FilterChainCallback &&fccb) override;
    };
}
