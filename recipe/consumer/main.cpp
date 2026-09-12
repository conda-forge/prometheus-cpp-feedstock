#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/gateway.h>
#include <prometheus/registry.h>
#include <curl/curl.h>
#include <iostream>
#include <memory>
#include <string>
static size_t receive(char* data, size_t size, size_t count, void* context) {
    static_cast<std::string*>(context)->append(data, size * count);
    return size * count;
}
int main() {
    auto registry = std::make_shared<prometheus::Registry>();
    auto& counter = prometheus::BuildCounter().Name("native_requests_total")
        .Help("Native consumer requests").Register(*registry).Add({});
    counter.Increment(3);
    prometheus::Exposer exposer{"127.0.0.1:0"};
    exposer.RegisterCollectable(registry);
    const auto ports = exposer.GetListeningPorts();
    if (ports.size() != 1 || ports[0] <= 0) return 1;
    prometheus::Gateway gateway{"127.0.0.1", "9", "native_consumer"};
    gateway.RegisterCollectable(registry);
    auto* curl = curl_easy_init();
    if (!curl) return 2;
    std::string body;
    const auto url = "http://127.0.0.1:" + std::to_string(ports[0]) + "/metrics";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_PROXY, "");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    const auto result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    if (result != CURLE_OK || status != 200 ||
        body.find("native_requests_total 3") == std::string::npos) return 3;
    std::cout << "Installed core, pull HTTP metrics, and push registration passed\n";
}
