#include "HttpClient.h"

#include <string>
#include <curl/curl.h>

namespace
{
    bool debugEnabled = false;

    size_t WriteCallback(
        void* contents,
        size_t size,
        size_t nmemb,
        std::string* output)
    {
        const size_t totalSize = size * nmemb;

        output->append(
            static_cast<char*>(contents),
            totalSize);

        return totalSize;
    }

    std::string performRequest(
        const std::string& url,
        const std::string* jsonBody)
    {
        CURL* curl = curl_easy_init();

        if (!curl)
            return "";

        std::string response;

        curl_easy_setopt(
            curl,
            CURLOPT_URL,
            url.c_str());

        curl_easy_setopt(
            curl,
            CURLOPT_WRITEFUNCTION,
            WriteCallback);

        curl_easy_setopt(
            curl,
            CURLOPT_WRITEDATA,
            &response);

        curl_easy_setopt(
            curl,
            CURLOPT_VERBOSE,
            0L);

        curl_easy_setopt(
            curl,
            CURLOPT_FOLLOWLOCATION,
            0L);

        curl_easy_setopt(
            curl,
            CURLOPT_CONNECTTIMEOUT,
            5L);

        curl_easy_setopt(
            curl,
            CURLOPT_TIMEOUT,
            10L);

        // This client currently targets the local development
        // HTTP server. Do not disable TLS verification for HTTPS.
        curl_easy_setopt(
            curl,
            CURLOPT_SSL_VERIFYPEER,
            1L);

        curl_easy_setopt(
            curl,
            CURLOPT_SSL_VERIFYHOST,
            2L);

        struct curl_slist* headers = nullptr;

        if (jsonBody != nullptr)
        {
            headers = curl_slist_append(
                headers,
                "Content-Type: application/json");

            curl_easy_setopt(
                curl,
                CURLOPT_HTTPHEADER,
                headers);

            curl_easy_setopt(
                curl,
                CURLOPT_POST,
                1L);

            curl_easy_setopt(
                curl,
                CURLOPT_POSTFIELDS,
                jsonBody->c_str());

            curl_easy_setopt(
                curl,
                CURLOPT_POSTFIELDSIZE,
                static_cast<long>(jsonBody->size()));
        }

        const CURLcode result =
            curl_easy_perform(curl);

        long httpCode = 0;

        curl_easy_getinfo(
            curl,
            CURLINFO_RESPONSE_CODE,
            &httpCode);

        if (headers != nullptr)
            curl_slist_free_all(headers);

        curl_easy_cleanup(curl);

        if (result != CURLE_OK)
            return "";

        if (httpCode < 200 || httpCode >= 300)
            return "";

        return response;
    }
}

std::string HttpClient::get(const std::string& url)
{
    return performRequest(url, nullptr);
}

std::string HttpClient::postJson(
    const std::string& url,
    const std::string& jsonBody)
{
    return performRequest(url, &jsonBody);
}

void HttpClient::setDebug(bool enabled)
{
    debugEnabled = enabled;
}

bool HttpClient::isDebug()
{
    return debugEnabled;
}