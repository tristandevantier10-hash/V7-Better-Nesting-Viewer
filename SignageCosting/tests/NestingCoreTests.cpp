#include "../NestingCore.h"
#include "../NestingLayoutSearch.h"
#include "../NestingManager.h"
#include <cassert>
#include <iostream>

int main()
{
    NestingCore core;
    const auto strategy = PlacementStrategy::BestShortSideFit;
    auto unlimited = core.pack({ { 500, 500, 10 } }, 1000, 1000, strategy);
    assert(unlimited.isValid() && unlimited.sheets.size() == 3);
    size_t total = 0;
    for (const auto& sheet : unlimited.sheets) total += sheet.placed.size();
    assert(total == 10);

    auto rotated = core.pack({ { 600, 400, 1 } }, 400, 600, strategy);
    assert(rotated.isValid() && rotated.sheets.front().placed.front().rotated);

    auto oversized = core.pack({ { 1100, 1100, 5 } }, 1000, 1000, strategy);
    assert(!oversized.isValid() && oversized.unplaced.front().quantity == 5);
    assert(oversized.sheets.front().placed.empty());

    // Complementary sizes must share sheets, rather than being packed separately.
    auto mixed = core.pack({ { 600, 1000, 2 }, { 400, 1000, 2 } },
        1000, 1000, strategy);
    assert(mixed.isValid() && mixed.sheets.size() == 2);
    size_t largeCount = 0, smallCount = 0;
    for (const auto& sheet : mixed.sheets)
    {
        assert(sheet.placed.size() == 2);
        for (const auto& part : sheet.placed)
        {
            const double area = part.width * part.height;
            if (area == 600000) ++largeCount;
            if (area == 400000) ++smallCount;
        }
    }
    assert(largeCount == 2 && smallCount == 2);
    auto invalid = core.pack({ { 500, 500, 1 } }, 0, 1000, strategy);
    assert(!invalid.isValid());
    const std::vector<Rect> regressionItems{ {600,400,50}, {300,100,34} };
    auto baseline = core.pack(regressionItems, 2440, 1220, strategy);
    auto improved = NestingLayoutSearch::findBest(regressionItems, 2440, 1220);
    assert(improved.isValid() && improved.sheets.size() == 5);
    assert(NestingLayoutSearch::containsAll(improved, regressionItems));
    assert(NestingLayoutSearch::envelopeArea(improved.sheets.back()) <
        NestingLayoutSearch::envelopeArea(baseline.sheets.back()));
    assert(NestingLayoutSearch::score(improved) < NestingLayoutSearch::score(baseline));
    // A rotated part in the bottom-right corner used to block an extra small
    // part. Prefer the equally compact arrangement with a usable free slot.
    const std::vector<Rect> cornerItems{{500,500,8},{300,100,28}};
    auto corner = NestingLayoutSearch::findBest(cornerItems,2440,1220);
    assert(corner.isValid() && corner.sheets.size() == 1);
    assert(NestingLayoutSearch::containsAll(corner,cornerItems));
    assert(corner.sheets.front().placed.size() == 36);
    assert(core.maxAdditionalFit(corner,{{300,100,1}}) >= 1);
    auto extraPiece = NestingLayoutSearch::findBest({{500,500,8},{300,100,29}},2440,1220);
    assert(extraPiece.isValid() && extraPiece.sheets.size() == 1);
    assert(extraPiece.sheets.front().placed.size() == 37);
    std::cout << "Corner regression passed: 36 requested pieces leave room for another; 37 requested pieces fit.\n";
    const std::vector<Rect> multiCornerItems{{500,500,50},{300,100,34}};
    auto multiCorner = NestingLayoutSearch::findBest(multiCornerItems,2440,1220);
    assert(multiCorner.isValid() && multiCorner.sheets.size() == 7);
    assert(NestingLayoutSearch::containsAll(multiCorner,multiCornerItems));
    size_t firstLarge = 0, firstSmall = 0;
    for (const auto& part : multiCorner.sheets.front().placed)
    {
        if (part.width * part.height == 250000) ++firstLarge;
        if (part.width * part.height == 30000) ++firstSmall;
    }
    assert(firstLarge == 8 && firstSmall >= 29);
    assert(multiCorner.sheets.front().placed.size() >= 37);
    // The donor sheet must be tidied after transferring its first small part.
    // Its old arrangement occupied a 2000 x 1200 envelope with a blank slot.
    assert(multiCorner.sheets[1].placed.size() == 13);
    assert(NestingLayoutSearch::envelopeArea(multiCorner.sheets[1]) < 2000.0 * 1200.0);
    std::set<double> donorLargeRows;
    for (const auto& part : multiCorner.sheets[1].placed)
        if (part.width == 500 && part.height == 500) donorLargeRows.insert(part.y);
    assert(donorLargeRows.size() == 2);

    // Check the shared pipeline on varied demands, not only the reported job.
    for (const auto& demand : std::vector<std::vector<Rect>>{
        {{700,400,7},{300,200,17},{150,150,12}},
        {{500,500,8},{300,100,34}},
        {{100,300,12},{400,200,9},{250,150,14},{50,50,20}} })
    {
        auto before = core.pack(demand,2440,1220,strategy);
        auto after = NestingLayoutSearch::findBest(demand,2440,1220);
        assert(after.isValid() && NestingLayoutSearch::containsAll(after,demand));
        assert(after.sheets.size() <= before.sheets.size());
        assert(NestingCore::validateLayout(after.sheets,2440,1220).empty());
    }
    std::cout << "Full-job corner regression passed: first sheet holds " << firstLarge
        << " large + " << firstSmall << " small; all 84 pieces preserved.\n";
    std::cout << "Final sheet occupied envelope: "
        << NestingLayoutSearch::envelopeArea(baseline.sheets.back()) << " -> "
        << NestingLayoutSearch::envelopeArea(improved.sheets.back()) << " mm2\n";
    auto exact = NestingLayoutSearch::findBest({ {500,500,4} }, 1000, 1000);
    assert(exact.isValid() && exact.sheets.size() == 1 && exact.sheets.front().placed.size() == 4);
    auto impossible = NestingLayoutSearch::findBest({ {1100,1100,1} }, 1000, 1000);
    assert(!impossible.isValid() && impossible.unplaced.size() == 1);

    // Quote groups must produce exactly the same placements as the workspace.
    auto quoteJob = [](const std::vector<Rect>& items)
    {
        Job job;
        for (const auto& rect : items)
        {
            JobItem item{};
            item.material.id = "TEST_SHEET";
            item.material.cost_model = "SHEET_AREA";
            item.material.variants.push_back(MaterialVariant{});
            item.material.variants.front().label = "Standard";
            item.selectedSheetFormat = {2440,1220};
            item.width = rect.width;
            item.height = rect.height;
            item.quantity = rect.quantity;
            job.items.push_back(item);
        }
        return job;
    };
    NestingManager manager;
    auto fullCornerQuote = manager.calculate(quoteJob(multiCornerItems));
    assert(fullCornerQuote.valid && fullCornerQuote.sheets.size() == 7);
    assert(fullCornerQuote.sheets.front().placed.size() == 37);
    assert(fullCornerQuote.sheets[1].placed.size() == 13);
    auto quote = manager.calculate(quoteJob(regressionItems));
    assert(quote.valid && quote.groups.size() == 1);
    assert(quote.groups.front().jobItemIndices.size() == 2);
    assert(quote.sheets.size() == improved.sheets.size());
    for (size_t sheetIndex = 0; sheetIndex < quote.sheets.size(); ++sheetIndex)
    {
        const auto& actual = quote.sheets[sheetIndex];
        const auto& expected = improved.sheets[sheetIndex];
        assert(actual.placed.size() == expected.placed.size());
        for (size_t partIndex = 0; partIndex < actual.placed.size(); ++partIndex)
        {
            const auto& a = actual.placed[partIndex];
            const auto& e = expected.placed[partIndex];
            assert(a.x == e.x && a.y == e.y && a.width == e.width &&
                a.height == e.height && a.rotated == e.rotated);
        }
    }
    const auto& group = quote.groups.front();
    assert(group.sheetCount == 5);
    assert(std::abs(group.wasteAreaM2 - (5 * 2.440 * 1.220 - 13.02)) < 1e-7);
    auto invalidQuote = manager.calculate(quoteJob({ {3000,3000,1} }));
    assert(!invalidQuote.valid && !invalidQuote.errors.empty());
    assert(invalidQuote.groups.front().unplaced.front().quantity == 1);
    auto invalidSheetJob = quoteJob({ {600,400,1} });
    invalidSheetJob.items.front().selectedSheetFormat = {0,1220};
    assert(!manager.calculate(invalidSheetJob).valid);
    auto separateJob = quoteJob({ {600,400,1}, {300,100,1} });
    separateJob.items.back().material.id = "ANOTHER_SHEET";
    auto separate = manager.calculate(separateJob);
    assert(separate.valid && separate.groups.size() == 2);
    std::cout << "Quote/workspace parity, material grouping, statistics and failure handling passed.\n";
    std::cout << "Nesting checks passed: quantities, rotation, oversized parts, mixed sizes, invalid sheet.\n";
}
