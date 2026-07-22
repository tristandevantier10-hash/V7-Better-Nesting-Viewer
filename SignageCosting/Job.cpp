#include "Job.h"

//DEFINITION//
// ----------------- //
//Even simpler:
//It:
//takes a JobItem
//stores it inside the Job
//doesn’t calculate anything
//doesn’t validate anything
//doesn’t care what it is

void Job::addItem(JobItem item) {
    items.push_back(item);
}