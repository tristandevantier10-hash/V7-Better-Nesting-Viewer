#include "ResultsViewModelBuilder.h"
#include "InvoiceViewModelBuilder.h"

ResultsViewModel
ResultsViewModelBuilder::build(
    const CostResult& result)
{
    ResultsViewModel vm;

    vm.invoice =
        InvoiceViewModelBuilder::build(result);

    return vm;
}