#include "QuoteReferenceGenerator.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

std::string QuoteReferenceGenerator::generate(
    const std::vector<QuoteRecord>& quotes)
{
    const std::time_t now = std::time(nullptr);

    std::tm timeInfo{};

    if (localtime_s(&timeInfo, &now) != 0)
        return "";

    const int year = timeInfo.tm_year + 1900;

    int highestNumber = 0;

    std::ostringstream prefixStream;
    prefixStream << "EG-Q-" << year << "-";

    const std::string prefix = prefixStream.str();

    for (const auto& quote : quotes)
    {
        const std::string& reference = quote.invoice.jobref;

        if (reference.rfind(prefix, 0) != 0)
            continue;

        const std::string numberText =
            reference.substr(prefix.length());

        if (numberText.empty())
            continue;

        if (!std::all_of(
            numberText.begin(),
            numberText.end(),
            [](unsigned char c)
            {
                return c >= '0' && c <= '9';
            }))
        {
            continue;
        }

        try
        {
            const int number = std::stoi(numberText);

            if (number > highestNumber)
                highestNumber = number;
        }
        catch (...)
        {
            // Ignore invalid references.
        }
    }

    std::ostringstream result;

    result << prefix
        << std::setw(4)
        << std::setfill('0')
        << highestNumber + 1;

    return result.str();
}