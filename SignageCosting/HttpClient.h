#pragma once
#include <string>

class HttpClient {
public:
    static std::string get(const std::string& url);

    // Optional: control logging globally (v2 expansion)
    static void setDebug(bool enabled);
    static bool isDebug();
};