#pragma once

#include <string>

class AuthenticationService {
public:
    // Core Authentication Methods
    bool login(const std::string& username, const std::string& password, std::string& errorMessage);
    void logout();
    bool isAuthenticated() const;

    // Getters
    const std::string& getAccessToken() const;
    int getEmployeeId() const;
    const std::string& getUsername() const;
    const std::string& getFullName() const;
    const std::string& getRole() const;

    // Role Checks
    bool isAdmin() const;
    bool isEmployee() const;

private:
    std::string accessToken;
    int employeeId = 0;
    std::string loggedInUsername;
    std::string fullName;
    std::string role;
};