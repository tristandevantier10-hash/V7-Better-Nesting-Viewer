#include "ResultsViewModelBuilder.h"
#include "InvoiceViewModelBuilder.h"

ResultsViewModel
ResultsViewModelBuilder::build(
    const CostResult& result)
{
    ResultsViewModel vm;

    vm.invoice =
        InvoiceViewModelBuilder::build(result);

    vm.sheetsUsed =
        static_cast<int>(result.nestingSheets.size());

    return vm;
}