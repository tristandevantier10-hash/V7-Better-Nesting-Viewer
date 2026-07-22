#include "CustomerDatabase.h"
#include "FileSystem.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<Customer> CustomerDatabase::customers;

void CustomerDatabase::add(const Customer& customer)
{
    customers.push_back(customer);

    save();
}

const std::vector<Customer>& CustomerDatabase::getAll()
{
    return customers;
}

void CustomerDatabase::clear()
{
    customers.clear();
}

void CustomerDatabase::initialise()
{
    namespace fs = std::filesystem;

    std::string filename =
        FileSystem::getDataFolder() +
        "customers.json";

    if (!fs::exists(filename))
    {
        std::ofstream file(filename);

        file <<
            R"({
    "customers": []
})";

        file.close();
    }

    std::ifstream file(filename);

    std::stringstream buffer;
    buffer << file.rdbuf();

    load(buffer.str());
}

void CustomerDatabase::save()
{
    json j;

    j["customers"] = json::array();

    for (const auto& customer : customers)
    {
        j["customers"].push_back(
            {
                { "company", customer.company },
                { "contact", customer.contact },
                { "phone", customer.phone },
                { "email", customer.email }
            });
    }

    std::string filename =
        FileSystem::getDataFolder() +
        "customers.json";

    std::ofstream file(filename);

    if (!file.is_open())
        return;

    file << j.dump(4);
}

void CustomerDatabase::load(const std::string& jsonData)
{
    customers.clear();

    json j;

    try
    {
        j = json::parse(jsonData);
    }
    catch (const std::exception&)
    {
        return;
    }

    if (!j.contains("customers"))
        return;

    if (!j["customers"].is_array())
        return;

    for (const auto& item : j["customers"])
    {
        Customer customer;

        customer.company =
            item.value("company", "");

        customer.contact =
            item.value("contact", "");

        customer.phone =
            item.value("phone", "");

        customer.email =
            item.value("email", "");

        customers.push_back(customer);
    }
}

Customer CustomerDatabase::get(int index)
{
    if (index < 0 ||
        index >= static_cast<int>(customers.size()))
    {
        return {};
    }

    return customers[index];
}

void CustomerDatabase::update(
    int index,
    const Customer& customer)
{
    if (index < 0 ||
        index >= static_cast<int>(customers.size()))
    {
        return;
    }

    customers[index] = customer;

    save();
}

void CustomerDatabase::remove(int index)
{
    if (index < 0 ||
        index >= static_cast<int>(customers.size()))
    {
        return;
    }

    customers.erase(
        customers.begin() + index);

    save();
}

int CustomerDatabase::count()
{
    return static_cast<int>(customers.size());
}
