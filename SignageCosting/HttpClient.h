#pragma once

#include <string>

class HttpClient
{
public:
    // Existing GET request — retained.
    static std::string get(const std::string& url);

    // POST a JSON request body and return the response.
    static std::string postJson(
        const std::string& url,
        const std::string& jsonBody);

    // Optional: control logging globally.
    static void setDebug(bool enabled);
    static bool isDebug();

};