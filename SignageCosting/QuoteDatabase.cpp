#include "QuoteDatabase.h"

#include "FileSystem.h"
#include "QuoteReferenceGenerator.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

std::vector<QuoteRecord> QuoteDatabase::quotes;

namespace
{
    std::string currentDateTime()
    {
        const std::time_t now = std::time(nullptr);

        std::tm timeInfo{};

        if (localtime_s(&timeInfo, &now) != 0)
            return "";

        std::ostringstream result;

        result << std::put_time(
            &timeInfo,
            "%Y-%m-%d %H:%M:%S");

        return result.str();
    }

    json invoiceToJson(const InvoiceViewModel& invoice)
    {
        json j;

        j["invoiceDate"] = invoice.invoiceDate;
        j["jobref"] = invoice.jobref;

        j["customer"] = invoice.customer;
        j["customerContact"] = invoice.customerContact;
        j["customerPhone"] = invoice.customerPhone;
        j["customerEmail"] = invoice.customerEmail;

        j["materialCost"] = invoice.materialCost;
        j["labourCost"] = invoice.labourCost;
        j["productionCost"] = invoice.productionCost;

        j["subtotal"] = invoice.subtotal;
        j["markup"] = invoice.markup;
        j["sellPrice"] = invoice.sellPrice;
        j["vat"] = invoice.vat;
        j["total"] = invoice.total;

        j["jobs"] = json::array();

        for (const auto& job : invoice.jobs)
        {
            j["jobs"].push_back({
                {"material", job.material},
                {"variant", job.variant},
                {"quantity", job.quantity},
                {"area", job.area},
                {"sellPrice", job.sellPrice},
                {"unitSize", job.unitSize},
                {"labourCost", job.labourCost},
                {"labourBreakdown", job.labourBreakdown},
                {"isRoll", job.isRoll},

                {"rollWidth", job.rollWidth},
                {"lengthUsed", job.lengthUsed},
                {"efficiency", job.efficiency},

                {"sheetSize", job.sheetSize},
                {"sheetsUsed", job.sheetsUsed}
                });
        }

        return j;
    }

    InvoiceViewModel invoiceFromJson(const json& j)
    {
        InvoiceViewModel invoice;

        invoice.invoiceDate =
            j.value("invoiceDate", std::string{});

        invoice.jobref =
            j.value("jobref", std::string{});

        invoice.customer =
            j.value("customer", std::string{});

        invoice.customerContact =
            j.value("customerContact", std::string{});

        invoice.customerPhone =
            j.value("customerPhone", std::string{});

        invoice.customerEmail =
            j.value("customerEmail", std::string{});

        invoice.materialCost =
            j.value("materialCost", std::string{});

        invoice.labourCost =
            j.value("labourCost", std::string{});

        invoice.productionCost =
            j.value("productionCost", std::string{});

        invoice.subtotal =
            j.value("subtotal", std::string{});

        invoice.markup =
            j.value("markup", std::string{});

        invoice.sellPrice =
            j.value("sellPrice", std::string{});

        invoice.vat =
            j.value("vat", std::string{});

        invoice.total =
            j.value("total", std::string{});

        if (j.contains("jobs") && j["jobs"].is_array())
        {
            for (const auto& item : j["jobs"])
            {
                JobSummaryViewModel job;

                job.material =
                    item.value("material", std::string{});

                job.variant =
                    item.value("variant", std::string{});

                job.quantity =
                    item.value("quantity", std::string{});

                job.area =
                    item.value("area", std::string{});

                job.sellPrice =
                    item.value("sellPrice", std::string{});
                job.unitSize = item.value("unitSize", std::string{});
                job.labourCost = item.value("labourCost", std::string{});
                if (item.contains("labourBreakdown") && item["labourBreakdown"].is_array())
                    job.labourBreakdown = item["labourBreakdown"].get<std::vector<std::string>>();

                job.isRoll =
                    item.value("isRoll", true);

                job.rollWidth =
                    item.value("rollWidth", std::string{});

                job.lengthUsed =
                    item.value("lengthUsed", std::string{});

                job.efficiency =
                    item.value("efficiency", std::string{});

                job.sheetSize =
                    item.value("sheetSize", std::string{});

                job.sheetsUsed =
                    item.value("sheetsUsed", std::string{});

                invoice.jobs.push_back(job);
            }
        }

        return invoice;
    }
}

bool QuoteDatabase::initialise()
{
    try
    {
        FileSystem::initialise();

        const std::string filename =
            FileSystem::getQuotesFolder() + "quotes.json";

        const std::string legacyFilename =
            FileSystem::getDataFolder() + "quotes.json";

        // Keep existing quote history when moving the database into Quotes/.
        if (!fs::exists(filename) && fs::exists(legacyFilename))
        {
            std::error_code copyError;
            fs::copy_file(
                legacyFilename,
                filename,
                fs::copy_options::skip_existing,
                copyError);

            if (copyError)
                return false;
        }

        if (!fs::exists(filename))
        {
            std::ofstream file(filename);

            if (!file.is_open())
                return false;

            file << "{\n    \"quotes\": []\n}\n";

            if (!file.good())
                return false;
        }

        return load();
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool QuoteDatabase::load()
{
    const std::string filename =
        FileSystem::getQuotesFolder() + "quotes.json";

    std::ifstream file(filename);

    if (!file.is_open())
        return false;

    json root;

    try
    {
        file >> root;

        if (!root.contains("quotes") ||
            !root["quotes"].is_array())
        {
            return false;
        }

        std::vector<QuoteRecord> loadedQuotes;

        for (const auto& item : root["quotes"])
        {
            if (!item.contains("invoice") ||
                !item["invoice"].is_object())
            {
                continue;
            }

            QuoteRecord record;

            record.invoice =
                invoiceFromJson(item["invoice"]);

            record.status =
                item.value("status", std::string("Draft"));

            record.savedAt =
                item.value("savedAt", std::string{});

            record.statusUpdatedAt =
                item.value("statusUpdatedAt", std::string{});

            loadedQuotes.push_back(record);
        }

        quotes = std::move(loadedQuotes);

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool QuoteDatabase::save()
{
    try
    {
        FileSystem::initialise();

        const std::string filename =
            FileSystem::getQuotesFolder() + "quotes.json";

        const std::string temporaryFilename =
            filename + ".tmp";

        const std::string backupFilename =
            filename + ".bak";

        json root;
        root["quotes"] = json::array();

        for (const auto& quote : quotes)
        {
            root["quotes"].push_back({
                {"invoice", invoiceToJson(quote.invoice)},
                {"status", quote.status},
                {"savedAt", quote.savedAt},
                {"statusUpdatedAt", quote.statusUpdatedAt}
                });
        }

        // Write the new data without touching the
        // existing database.
        {
            std::ofstream file(
                temporaryFilename,
                std::ios::out | std::ios::trunc);

            if (!file.is_open())
                return false;

            file << root.dump(4);
            file.flush();

            if (!file.good())
                return false;
        }

        std::error_code error;

        const bool originalExists = fs::exists(filename);

        // If there is no existing database, install the
        // new file directly.
        if (!originalExists)
        {
            fs::rename(
                temporaryFilename,
                filename,
                error);

            return !error;
        }

        // Preserve the original before replacing it.
        if (fs::exists(backupFilename))
        {
            fs::remove(backupFilename, error);

            if (error)
                return false;
        }

        fs::rename(filename, backupFilename, error);

        if (error)
            return false;

        // Install the newly written database.
        error.clear();

        fs::rename(
            temporaryFilename,
            filename,
            error);

        if (error)
        {
            // Restore the original if installation fails.
            std::error_code restoreError;

            fs::rename(
                backupFilename,
                filename,
                restoreError);

            return false;
        }

        // The new database is in place. Remove the backup.
        error.clear();

        fs::remove(backupFilename, error);

        // The save succeeded even if backup cleanup failed.
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool QuoteDatabase::saveQuote(
    InvoiceViewModel& invoice,
    const std::string& status)
{
    // Do not allow accidental duplicate references.
    if (!invoice.jobref.empty())
    {
        for (const auto& quote : quotes)
        {
            if (quote.invoice.jobref == invoice.jobref)
                return false;
        }
    }

    const std::string originalReference = invoice.jobref;
    const std::string originalDate = invoice.invoiceDate;

    if (invoice.jobref.empty())
    {
        invoice.jobref =
            QuoteReferenceGenerator::generate(quotes);

        if (invoice.jobref.empty())
            return false;
    }

    if (invoice.invoiceDate.empty())
    {
        invoice.invoiceDate = currentDateTime();

        // Keep the displayed date concise.
        if (invoice.invoiceDate.size() >= 10)
            invoice.invoiceDate.resize(10);
    }

    QuoteRecord record;

    record.invoice = invoice;
    record.status = status;
    record.savedAt = currentDateTime();
    record.statusUpdatedAt = record.savedAt;

    quotes.push_back(record);

    if (!save())
    {
        quotes.pop_back();

        invoice.jobref = originalReference;
        invoice.invoiceDate = originalDate;

        return false;
    }

    return true;
}

const std::vector<QuoteRecord>& QuoteDatabase::getAll()
{
    return quotes;
}

bool QuoteDatabase::findByReference(
    const std::string& reference,
    QuoteRecord& result)
{
    for (const auto& quote : quotes)
    {
        if (quote.invoice.jobref == reference)
        {
            result = quote;
            return true;
        }
    }

    return false;
}

bool QuoteDatabase::updateStatus(
    const std::string& reference,
    const std::string& status)
{
    for (auto& quote : quotes)
    {
        if (quote.invoice.jobref == reference)
        {
            const std::string previousStatus = quote.status;
            const std::string previousStatusUpdatedAt = quote.statusUpdatedAt;

            quote.status = status;
            quote.statusUpdatedAt = currentDateTime();

            if (!save())
            {
                quote.status = previousStatus;
                quote.statusUpdatedAt = previousStatusUpdatedAt;
                return false;
            }

            return true;
        }
    }

    return false;
}

int QuoteDatabase::count()
{
    return static_cast<int>(quotes.size());
}
