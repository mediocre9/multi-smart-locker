#ifndef CUSTOM_ASYNC_RATE_LIMIT_MIDDLEWARE_H
#define CUSTOM_ASYNC_RATE_LIMIT_MIDDLEWARE_H

#include <ESPAsyncWebServer.h>
#include <cstdint>
#include "../includes/Config.hpp"

class CustomAsyncRateLimitMiddleware final : public AsyncRateLimitMiddleware {
public:
    CustomAsyncRateLimitMiddleware() = default;

    void run(AsyncWebServerRequest* request, ArMiddlewareNext next) override {
        std::uint32_t retryAfterSeconds;
        if (!isRequestAllowed(retryAfterSeconds)) {
            request->send(StatusCode::TOO_MANY_REQUESTS, ContentType::PLAIN, ResponseMessage::TOO_MANY_REQUESTS);
            return;
        }
        next();
    }
};

#endif