#pragma once

#include <string>

enum class AccountType
{
    Cash,
    Credit
};

struct Customer
{
    AccountType accountType = AccountType::Cash;

    std::string company;

    std::string contact;

    std::string phone;

    std::string email;
};
