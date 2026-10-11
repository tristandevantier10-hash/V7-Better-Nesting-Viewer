#include "AuthenticationService.h"
#include "HttpClient.h"
#include <nlohmann/json.hpp>

bool AuthenticationService::login(
    const std::string& username,
    const std::string& password,
    std::string& errorMessage)
{
    logout();
    errorMessage.clear();

    try
    {
        nlohmann::json request;
        request["username"] = username;
        request["password"] = password;

        const std::string response = HttpClient::postJson(
            "http://localhost:5135/api/auth/login",
            request.dump()
        );

        if (response.empty())
        {
            errorMessage =
                "Login failed. Check your credentials and ensure "
                "the authentication server is running.";
            return false;
        }

        const nlohmann::json result =
            nlohmann::json::parse(response);

        if (!result.contains("accessToken") ||
            !result["accessToken"].is_string() ||
            !result.contains("employee") ||
            !result["employee"].is_object())
        {
            errorMessage =
                "The server returned an invalid login response.";
            return false;
        }

        const auto& employee = result["employee"];

        // Accept both camelCase and PascalCase JSON property names.
        const char* idKey =
            employee.contains("id") ? "id" : "Id";

        const char* usernameKey =
            employee.contains("username") ? "username" : "Username";

        const char* fullNameKey =
            employee.contains("fullName") ? "fullName" : "FullName";

        const char* roleKey =
            employee.contains("role") ? "role" : "Role";

        if ((!employee.contains("id") && !employee.contains("Id")) ||
            (!employee.contains("username") &&
                !employee.contains("Username")) ||
            (!employee.contains("fullName") &&
                !employee.contains("FullName")) ||
            (!employee.contains("role") &&
                !employee.contains("Role")))
        {
            errorMessage =
                "The server returned incomplete employee details.";
            return false;
        }

        if (!employee.at(idKey).is_number_integer() ||
            !employee.at(usernameKey).is_string() ||
            !employee.at(fullNameKey).is_string() ||
            !employee.at(roleKey).is_string())
        {
            errorMessage =
                "The server returned employee details in an invalid format.";
            return false;
        }

        const std::string receivedToken =
            result["accessToken"].get<std::string>();

        const int receivedId =
            employee.at(idKey).get<int>();

        const std::string receivedUsername =
            employee.at(usernameKey).get<std::string>();

        const std::string receivedFullName =
            employee.at(fullNameKey).get<std::string>();

        const std::string receivedRole =
            employee.at(roleKey).get<std::string>();

        if (receivedToken.empty() ||
            receivedId <= 0 ||
            receivedUsername.empty() ||
            receivedRole.empty())
        {
            errorMessage =
                "The server returned invalid employee details.";
            return false;
        }

        // Store the session only after all fields pass validation.
        accessToken = receivedToken;
        employeeId = receivedId;
        loggedInUsername = receivedUsername;
        fullName = receivedFullName;
        role = receivedRole;

        return true;
    }
    catch (const nlohmann::json::exception&)
    {
        logout();
        errorMessage =
            "Unable to read the authentication server response.";
        return false;
    }
    catch (...)
    {
        logout();
        errorMessage =
            "An unexpected error occurred during login.";
        return false;
    }
}

void AuthenticationService::logout()
{
    accessToken.clear();
    employeeId = 0;
    loggedInUsername.clear();
    fullName.clear();
    role.clear();
}

bool AuthenticationService::isAuthenticated() const
{
    return !accessToken.empty();
}

const std::string& AuthenticationService::getAccessToken() const
{
    return accessToken;
}

int AuthenticationService::getEmployeeId() const
{
    return employeeId;
}

const std::string& AuthenticationService::getUsername() const
{
    return loggedInUsername;
}

const std::string& AuthenticationService::getFullName() const
{
    return fullName;
}

const std::string& AuthenticationService::getRole() const
{
    return role;
}

bool AuthenticationService::isAdmin() const
{
    return role == "Admin";
}

bool AuthenticationService::isEmployee() const
{
    return role == "Employee";
}