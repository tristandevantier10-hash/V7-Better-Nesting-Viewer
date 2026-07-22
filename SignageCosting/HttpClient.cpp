#include "HttpClient.h"
#include <string>
#include <curl/curl.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output)
{
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

std::string HttpClient::get(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        return "";

    std::string response;
    char errorBuffer[CURL_ERROR_SIZE] = { 0 };

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Silent mode (NO CURL DEBUG OUTPUT)
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    // NOTE: insecure settings kept as-is (dev behaviour preserved)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_easy_cleanup(curl);

    // Silent failure handling (no console spam)
    if (res != CURLE_OK)
        return "";

    if (httpCode != 200)
        return "";

    if (response.empty())
        return "";

    return response;
}