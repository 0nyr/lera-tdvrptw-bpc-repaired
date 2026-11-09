#pragma once
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <goc/goc.h>
namespace ms56 {
inline long long now_ns() { return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
inline double elapsed() { const char* p = std::getenv("PROBE_START_MONOTONIC_NS"); return p ? (now_ns()-std::stoll(p))/1e9 : 0.0; }
inline double remaining() { const char* p = std::getenv("PROBE_DEADLINE_MONOTONIC_NS"); return p ? std::max(0.0,(std::stoll(p)-now_ns())/1e9) : 0.0; }
inline void emit(nlohmann::json event) { event["elapsed_s"]=elapsed(); std::cout << event << std::endl; }
inline void candidate(const goc::VRPSolution& solution, const std::string& origin) {
    nlohmann::json routes=nlohmann::json::array();
    const char* end_text=std::getenv("UPSTREAM_EXPECTED_END_DEPOT");
    int end=end_text ? std::stoi(end_text) : -1;
    for (const auto& route: solution.routes) {
        if(route.path.size()<3 || route.path.front()!=0 || route.path.back()!=end) {
            emit({{"event","integrity"},{"reason","invalid_native_route_depot_mapping"}}); return;
        }
        for(size_t k=1;k+1<route.path.size();++k) if(route.path[k]<=0 || route.path[k]>=end) {
            emit({{"event","integrity"},{"reason","invalid_native_customer_mapping"}}); return;
        }
        std::vector<int> customers;
        for (size_t k=1;k+1<route.path.size();++k) customers.push_back(route.path[k]);
        routes.push_back(customers);
    }
    emit({{"event","candidate"},{"phase","exact"},{"origin",origin},{"native_cost",solution.value},{"routes",routes},{"routes_format","mamut-customers"}});
}
}
