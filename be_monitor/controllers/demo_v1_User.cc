#include "demo_v1_User.h"

using namespace demo::v1;

// Add definition of your processing function here
void User::sayHello(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) const{
    auto resp = HttpResponse::newHttpResponse();
    resp->setBody(
        "Hello, this is a generic hello message from the SayHello "
        "controller");
    callback(resp);
}