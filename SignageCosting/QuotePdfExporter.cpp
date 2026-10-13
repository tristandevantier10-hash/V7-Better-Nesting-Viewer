#include "QuotePdfExporter.h"

#include <hpdf.h>

#include <algorithm>
#include <cstdio>
#include <string>

namespace
{
    constexpr float PAGE_WIDTH = 595.28f;
    constexpr float PAGE_HEIGHT = 841.89f;

    constexpr float MARGIN = 42.0f;
    constexpr float CONTENT_WIDTH = PAGE_WIDTH - (MARGIN * 2.0f);

    constexpr float DARK = 0.25f;
    constexpr float MUTED = 0.43f;
    constexpr float PALE = 0.93f;

    struct QuotePdf
    {
        HPDF_Doc pdf = nullptr;
        HPDF_Page page = nullptr;

        HPDF_Font regular = nullptr;
        HPDF_Font bold = nullptr;

        float y = 0.0f;
        int pageNumber = 0;
    };

    void setColour(
        HPDF_Page page,
        float r,
        float g,
        float b)
    {
        HPDF_Page_SetRGBFill(page, r, g, b);
    }

    void drawText(
        QuotePdf& doc,
        float x,
        float y,
        const std::string& value,
        HPDF_Font font,
        float size,
        float r = 0.20f,
        float g = 0.20f,
        float b = 0.21f)
    {
        if (value.empty())
            return;

        setColour(doc.page, r, g, b);

        HPDF_Page_BeginText(doc.page);
        HPDF_Page_SetFontAndSize(doc.page, font, size);
        HPDF_Page_TextOut(
            doc.page,
            x,
            y,
            value.c_str());
        HPDF_Page_EndText(doc.page);
    }

    void drawRightText(
        QuotePdf& doc,
        float rightX,
        float y,
        const std::string& value,
        HPDF_Font font,
        float size,
        float r = 0.20f,
        float g = 0.20f,
        float b = 0.21f)
    {
        if (value.empty())
            return;

        setColour(doc.page, r, g, b);

        HPDF_Page_BeginText(doc.page);
        HPDF_Page_SetFontAndSize(doc.page, font, size);

        const float textWidth =
            HPDF_Page_TextWidth(doc.page, value.c_str());

        HPDF_Page_TextOut(
            doc.page,
            rightX - textWidth,
            y,
            value.c_str());

        HPDF_Page_EndText(doc.page);
    }

    void drawLine(
        HPDF_Page page,
        float x1,
        float y1,
        float x2,
        float y2,
        float r = 0.84f,
        float g = 0.84f,
        float b = 0.85f)
    {
        HPDF_Page_SetRGBStroke(page, r, g, b);
        HPDF_Page_SetLineWidth(page, 0.7f);

        HPDF_Page_MoveTo(page, x1, y1);
        HPDF_Page_LineTo(page, x2, y2);
        HPDF_Page_Stroke(page);
    }

    void drawRectangle(
        HPDF_Page page,
        float x,
        float y,
        float width,
        float height,
        float r,
        float g,
        float b)
    {
        HPDF_Page_SetRGBFill(page, r, g, b);
        HPDF_Page_Rectangle(page, x, y, width, height);
        HPDF_Page_Fill(page);
    }

    void startPage(QuotePdf& doc)
    {
        doc.page = HPDF_AddPage(doc.pdf);

        HPDF_Page_SetSize(
            doc.page,
            HPDF_PAGE_SIZE_A4,
            HPDF_PAGE_PORTRAIT);

        doc.pageNumber++;

        // Leave room for the brand header and divider on every page.
        doc.y = PAGE_HEIGHT - 118.0f;

        drawRectangle(
            doc.page,
            0.0f,
            PAGE_HEIGHT - 5.0f,
            PAGE_WIDTH,
            5.0f,
            DARK,
            DARK,
            DARK);

        drawText(
            doc,
            MARGIN,
            PAGE_HEIGHT - 43.0f,
            "E & G SIGNS CC",
            doc.bold,
            19.0f,
            0.20f,
            0.20f,
            0.21f);

        drawLine(
            doc.page,
            MARGIN,
            PAGE_HEIGHT - 100.0f,
            PAGE_WIDTH - MARGIN,
            PAGE_HEIGHT - 100.0f);

        drawText(
            doc,
            MARGIN,
            22.0f,
            "E & G SIGNS CC  |  QUOTATION",
            doc.regular,
            7.5f,
            MUTED,
            MUTED,
            MUTED);

        drawRightText(
            doc,
            PAGE_WIDTH - MARGIN,
            22.0f,
            "Page " + std::to_string(doc.pageNumber),
            doc.regular,
            7.5f,
            MUTED,
            MUTED,
            MUTED);
    }

    void drawSectionHeading(
        QuotePdf& doc,
        const std::string& heading,
        float x,
        float y)
    {
        drawText(
            doc,
            x,
            y,
            heading,
            doc.bold,
            9.0f,
            0.20f,
            0.20f,
            0.21f);
    }

    void drawTableHeader(QuotePdf& doc)
    {
        constexpr float QTY_RIGHT = 370.0f;
        constexpr float AREA_RIGHT = 446.0f;
        constexpr float PRICE_RIGHT = PAGE_WIDTH - MARGIN - 10.0f;

        const float tableTop = doc.y;
        const float tableHeight = 27.0f;
        const float tableBottom = tableTop - tableHeight;

        drawRectangle(
            doc.page,
            MARGIN,
            tableBottom,
            CONTENT_WIDTH,
            tableHeight,
            DARK,
            DARK,
            DARK);

        drawText(
            doc,
            MARGIN + 9.0f,
            tableBottom + 9.0f,
            "Material / Variant",
            doc.bold,
            8.0f,
            1.0f,
            1.0f,
            1.0f);

        drawRightText(
            doc,
            QTY_RIGHT,
            tableBottom + 9.0f,
            "Qty",
            doc.bold,
            8.0f,
            1.0f,
            1.0f,
            1.0f);

        drawRightText(
            doc,
            AREA_RIGHT,
            tableBottom + 9.0f,
            "Area",
            doc.bold,
            8.0f,
            1.0f,
            1.0f,
            1.0f);

        drawRightText(
            doc,
            PRICE_RIGHT,
            tableBottom + 9.0f,
            "Sell Price",
            doc.bold,
            8.0f,
            1.0f,
            1.0f,
            1.0f);

        doc.y = tableBottom;
    }

    void drawTableRow(
        QuotePdf& doc,
        const JobSummaryViewModel& job,
        std::size_t index)
    {
        constexpr float ROW_HEIGHT = 43.0f;
        constexpr float QTY_RIGHT = 370.0f;
        constexpr float AREA_RIGHT = 446.0f;
        constexpr float PRICE_RIGHT = PAGE_WIDTH - MARGIN - 10.0f;

        const float rowBottom = doc.y - ROW_HEIGHT;

        if (rowBottom < 65.0f)
        {
            startPage(doc);
            drawTableHeader(doc);
        }

        const float bottom = doc.y - ROW_HEIGHT;

        // The first row stays white; following rows alternate in pale gray.
        if (index % 2 == 1)
        {
            drawRectangle(
                doc.page,
                MARGIN,
                bottom,
                CONTENT_WIDTH,
                ROW_HEIGHT,
                PALE,
                PALE,
                PALE);
        }

        std::string description = job.material;

        if (!job.variant.empty())
            description += " - " + job.variant;

        drawText(
            doc,
            MARGIN + 9.0f,
            bottom + 25.0f,
            description,
            doc.bold,
            8.0f);

        std::string detail;

        if (job.isRoll)
        {
            if (!job.rollWidth.empty())
                detail = "Roll width: " + job.rollWidth;

            if (!job.lengthUsed.empty())
            {
                if (!detail.empty())
                    detail += "  |  ";

                detail += "Length: " + job.lengthUsed;
            }

            if (!job.efficiency.empty())
            {
                if (!detail.empty())
                    detail += "  |  ";

                detail += "Efficiency: " + job.efficiency;
            }
        }
        else
        {
            if (!job.sheetSize.empty())
                detail = "Sheet size: " + job.sheetSize;

            if (!job.sheetsUsed.empty())
            {
                if (!detail.empty())
                    detail += "  |  ";

                detail += "Sheets: " + job.sheetsUsed;
            }
        }

        drawText(
            doc,
            MARGIN + 9.0f,
            bottom + 11.0f,
            detail,
            doc.regular,
            6.8f,
            MUTED,
            MUTED,
            MUTED);

        drawRightText(
            doc,
            QTY_RIGHT,
            bottom + 18.0f,
            job.quantity,
            doc.regular,
            8.0f);

        drawRightText(
            doc,
            AREA_RIGHT,
            bottom + 18.0f,
            job.area,
            doc.regular,
            8.0f);

        drawRightText(
            doc,
            PRICE_RIGHT,
            bottom + 18.0f,
            job.sellPrice,
            doc.bold,
            8.0f);

        drawLine(
            doc.page,
            MARGIN,
            bottom,
            PAGE_WIDTH - MARGIN,
            bottom);

        doc.y = bottom;
    }

    void drawTotals(QuotePdf& doc, const InvoiceViewModel& invoice)
    {
        constexpr float LABEL_X = 350.0f;
        constexpr float VALUE_RIGHT = PAGE_WIDTH - MARGIN - 10.0f;
        constexpr float ROW_GAP = 19.0f;

        if (doc.y < 200.0f)
            startPage(doc);

        doc.y -= 25.0f;

        drawText(
            doc, LABEL_X, doc.y,
            "Materials", doc.regular, 9.0f);

        drawRightText(
            doc, VALUE_RIGHT, doc.y,
            invoice.materialCost, doc.regular, 9.0f);

        doc.y -= ROW_GAP;

        drawText(
            doc, LABEL_X, doc.y,
            "Labour", doc.regular, 9.0f);

        drawRightText(
            doc, VALUE_RIGHT, doc.y,
            invoice.labourCost, doc.regular, 9.0f);

        doc.y -= ROW_GAP;

        drawText(
            doc, LABEL_X, doc.y,
            "Production", doc.regular, 9.0f);

        drawRightText(
            doc, VALUE_RIGHT, doc.y,
            invoice.productionCost, doc.regular, 9.0f);

        doc.y -= ROW_GAP;

        // Place the note alongside the subtotal, VAT, and total block.
        drawSectionHeading(
            doc,
            "TERMS & ACCEPTANCE",
            MARGIN,
            doc.y);

        drawText(
            doc,
            MARGIN,
            doc.y - 17.0f,
            "Please review the quoted items and amounts",
            doc.regular,
            8.0f,
            MUTED,
            MUTED,
            MUTED);

        drawText(
            doc,
            MARGIN,
            doc.y - 29.0f,
            "above before accepting this quotation.",
            doc.regular,
            8.0f,
            MUTED,
            MUTED,
            MUTED);

        drawLine(
            doc.page,
            LABEL_X,
            doc.y + 9.0f,
            PAGE_WIDTH - MARGIN,
            doc.y + 9.0f);

        drawText(
            doc, LABEL_X, doc.y,
            "Subtotal", doc.regular, 9.0f);

        drawRightText(
            doc, VALUE_RIGHT, doc.y,
            invoice.subtotal, doc.regular, 9.0f);

        doc.y -= ROW_GAP;

        drawText(
            doc, LABEL_X, doc.y,
            "Markup", doc.regular, 9.0f);

        drawRightText(
            doc, VALUE_RIGHT, doc.y,
            invoice.markup, doc.regular, 9.0f);

        doc.y -= ROW_GAP;

        drawText(
            doc, LABEL_X, doc.y,
            "VAT", doc.regular, 9.0f);

        drawRightText(
            doc, VALUE_RIGHT, doc.y,
            invoice.vat, doc.regular, 9.0f);

        doc.y -= 29.0f;

        drawRectangle(
            doc.page,
            LABEL_X - 10.0f,
            doc.y - 9.0f,
            PAGE_WIDTH - MARGIN - LABEL_X + 10.0f,
            32.0f,
            DARK,
            DARK,
            DARK);

        drawText(
            doc, LABEL_X, doc.y + 3.0f,
            "TOTAL", doc.bold, 11.0f,
            1.0f, 1.0f, 1.0f);

        drawRightText(
            doc, VALUE_RIGHT, doc.y + 3.0f,
            invoice.total, doc.bold, 11.0f,
            1.0f, 1.0f, 1.0f);

        doc.y -= 30.0f;
    }
}

bool QuotePdfExporter::exportToPdf(
    const InvoiceViewModel& invoice,
    const std::string& outputPath,
    std::string& errorMessage)
{
    errorMessage.clear();

    if (outputPath.empty())
    {
        errorMessage = "The PDF output path is empty.";
        return false;
    }

    QuotePdf doc;

    doc.pdf = HPDF_New(nullptr, nullptr);

    if (!doc.pdf)
    {
        errorMessage = "Could not initialise the PDF document.";
        return false;
    }

    doc.regular = HPDF_GetFont(
        doc.pdf,
        "Helvetica",
        nullptr);

    doc.bold = HPDF_GetFont(
        doc.pdf,
        "Helvetica-Bold",
        nullptr);

    if (!doc.regular || !doc.bold)
    {
        errorMessage = "Could not initialise PDF fonts.";
        HPDF_Free(doc.pdf);
        return false;
    }

    HPDF_SetInfoAttr(
        doc.pdf,
        HPDF_INFO_TITLE,
        "EstiMate Quotation");

    HPDF_SetInfoAttr(
        doc.pdf,
        HPDF_INFO_AUTHOR,
        "E & G SIGNS CC");

    startPage(doc);

    // Keep the title and quote details in the top-right header area,
    // above the divider drawn by startPage().
    drawText(
        doc,
        390.0f,
        PAGE_HEIGHT - 43.0f,
        "QUOTATION",
        doc.bold,
        20.0f,
        0.20f,
        0.20f,
        0.21f);

    drawText(
        doc,
        390.0f,
        PAGE_HEIGHT - 62.0f,
        "QUOTE REFERENCE",
        doc.bold,
        7.0f,
        MUTED,
        MUTED,
        MUTED);

    drawText(
        doc,
        390.0f,
        PAGE_HEIGHT - 76.0f,
        invoice.jobref.empty()
        ? "UNSAVED QUOTE"
        : invoice.jobref,
        doc.bold,
        10.0f);

    drawText(
        doc,
        390.0f,
        PAGE_HEIGHT - 90.0f,
        "DATE: " + invoice.invoiceDate,
        doc.regular,
        8.0f);

    // Start the customer section below the header divider.
    doc.y = PAGE_HEIGHT - 135.0f;

    drawSectionHeading(
        doc, "BILL TO", MARGIN, doc.y);

    drawSectionHeading(
        doc, "CONTACT DETAILS", 310.0f, doc.y);

    doc.y -= 19.0f;

    drawText(
        doc,
        MARGIN,
        doc.y,
        invoice.customer.empty()
        ? "Customer not specified"
        : invoice.customer,
        doc.bold,
        9.0f);

    drawText(
        doc,
        310.0f,
        doc.y,
        invoice.customerContact,
        doc.regular,
        8.5f);

    doc.y -= 16.0f;

    drawText(
        doc,
        MARGIN,
        doc.y,
        "Quote: " + (
            invoice.jobref.empty()
            ? std::string("Unsaved")
            : invoice.jobref),
        doc.regular,
        8.0f);

    drawText(
        doc,
        310.0f,
        doc.y,
        invoice.customerPhone,
        doc.regular,
        8.5f);

    doc.y -= 16.0f;

    drawText(
        doc,
        310.0f,
        doc.y,
        invoice.customerEmail,
        doc.regular,
        8.5f);

    doc.y -= 26.0f;

    drawSectionHeading(
        doc, "QUOTED ITEMS", MARGIN, doc.y);

    doc.y -= 14.0f;

    drawTableHeader(doc);

    for (std::size_t i = 0; i < invoice.jobs.size(); ++i)
    {
        drawTableRow(doc, invoice.jobs[i], i);
    }

    if (invoice.jobs.empty())
    {
        doc.y -= 10.0f;

        drawText(
            doc,
            MARGIN + 9.0f,
            doc.y,
            "No job items were supplied for this quotation.",
            doc.regular,
            8.0f);

        doc.y -= 25.0f;
    }

    drawTotals(doc, invoice);

    // Keep the acceptance lines above the footer. For a long quote,
    // move this block onto a fresh page.
    if (doc.y < 105.0f)
        startPage(doc);

    if (doc.y > 160.0f)
        doc.y = 160.0f;

    drawSectionHeading(
        doc,
        "ACCEPTANCE",
        MARGIN,
        doc.y);

    doc.y -= 20.0f;

    drawText(
        doc,
        MARGIN,
        doc.y,
        "Sign below to accept this quotation.",
        doc.regular,
        8.5f,
        MUTED,
        MUTED,
        MUTED);

    doc.y -= 28.0f;

    drawLine(
        doc.page,
        MARGIN,
        doc.y,
        MARGIN + 205.0f,
        doc.y);

    drawLine(
        doc.page,
        MARGIN + 245.0f,
        doc.y,
        PAGE_WIDTH - MARGIN,
        doc.y);

    doc.y -= 12.0f;

    drawText(
        doc,
        MARGIN,
        doc.y,
        "Authorised signature",
        doc.regular,
        7.5f,
        MUTED,
        MUTED,
        MUTED);

    drawText(
        doc,
        MARGIN + 245.0f,
        doc.y,
        "Date",
        doc.regular,
        7.5f,
        MUTED,
        MUTED,
        MUTED);

    const HPDF_STATUS status = HPDF_SaveToFile(
        doc.pdf,
        outputPath.c_str());

    if (status != HPDF_OK)
    {
        errorMessage =
            "libharu could not save the PDF file. "
            "Check the output path and file permissions.";

        HPDF_Free(doc.pdf);
        return false;
    }

    HPDF_Free(doc.pdf);
    return true;
}