#include "AuthenticationService.h"
#include "HttpClient.h"

#include <nlohmann/json.hpp>

bool AuthenticationService::login(
    const std::string& username,
    const std::string& password,
    std::string& errorMessage)
{
    accessToken.clear();
    errorMessage.clear();

    try
    {
        nlohmann::json request;
        request["username"] = username;
        request["password"] = password;

        const std::string response =
            HttpClient::postJson(
                "http://localhost:5135/api/auth/login",
                request.dump());

        if (response.empty())
        {
            errorMessage =
                "Login failed. Check your credentials "
                "and ensure the authentication server is running.";

            return false;
        }

        const nlohmann::json result =
            nlohmann::json::parse(response);

        if (!result.contains("accessToken") ||
            !result["accessToken"].is_string())
        {
            errorMessage =
                "The server returned an invalid login response.";

            return false;
        }

        accessToken = result["accessToken"].get<std::string>();

        if (accessToken.empty())
        {
            errorMessage =
                "The server returned an empty access token.";

            return false;
        }

        return true;
    }
    catch (const nlohmann::json::exception&)
    {
        errorMessage =
            "Unable to read the authentication server response.";

        return false;
    }
    catch (...)
    {
        errorMessage =
            "An unexpected error occurred during login.";

        return false;
    }
}

void AuthenticationService::logout()
{
    accessToken.clear();
}

bool AuthenticationService::isAuthenticated() const
{
    return !accessToken.empty();
}

const std::string& AuthenticationService::getAccessToken() const
{
    return accessToken;
}