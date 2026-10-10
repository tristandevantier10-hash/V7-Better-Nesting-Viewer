#pragma once

#include <string>

class AuthenticationService
{
public:
    bool login(
        const std::string& username,
        const std::string& password,
        std::string& errorMessage);

    void logout();

    bool isAuthenticated() const;

    const std::string& getAccessToken() const;

private:
    std::string accessToken;
};
