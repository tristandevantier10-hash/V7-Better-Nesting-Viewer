#pragma once

#include <vector>
#include <string>
#include "Customer.h"

class CustomerDatabase
{
public:

    static void add(const Customer& customer);

    static const std::vector<Customer>& getAll();

    static void clear();

    static void initialise();

    static void load(const std::string& jsonData);

    static void save();

    static Customer get(int index);

    static void update(
        int index,
        const Customer& customer);

    static void remove(int index);

    static int count();

private:

    static std::vector<Customer> customers;
};
