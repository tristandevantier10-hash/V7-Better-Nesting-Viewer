#define SDL_MAIN_HANDLED
#include "../NewJobScreen.h"
#include "../TextBox.h"
#include <cassert>
#include <iostream>

int main()
{
    SDL_SetMainReady();
    SDL_Init(0);
    NewJobScreen screen;
    screen.setPosition(0, 0);
    screen.setSize(1280, 900);
    Job job;
    screen.setJob(&job);
    int calculations = 0;
    screen.setCalculateCallback([&]() { ++calculations; });
    auto clickCalculate = [&]()
    {
        SDL_Event event{};
        event.type = SDL_MOUSEMOTION;
        event.motion.x = 740;
        event.motion.y = 854;
        screen.update(event);
        event.type = SDL_MOUSEBUTTONDOWN;
        event.button.button = SDL_BUTTON_LEFT;
        event.button.x = 740;
        event.button.y = 854;
        screen.update(event);
        event.type = SDL_MOUSEBUTTONUP;
        screen.update(event);
    };

    // Reproduce pressing Calculate with the form blank and no added items.
    clickCalculate();
    clickCalculate();
    assert(calculations == 0 && job.items.empty());

    // Filling the editor is not the same as adding an item to the job.
    std::vector<TextBox*> fields;
    screen.collectTextBoxes(fields);
    assert(fields.size() == 3);
    fields[0]->setText("600");
    fields[1]->setText("400");
    fields[2]->setText("5");
    clickCalculate();
    assert(calculations == 0 && job.items.empty());

    // Confirm the same real button event still submits a job with an item.
    job.items.push_back(JobItem{});
    clickCalculate();
    assert(calculations == 1 && job.items.size() == 1);

    // Removing the last item must restore the guard.
    job.items.clear();
    screen.clearEditor();
    clickCalculate();
    assert(calculations == 1 && job.items.empty());
    screen.setJob(nullptr);
    clickCalculate();
    assert(calculations == 1);
    std::cout << "Calculate guards passed: empty form, repeated clicks, unadded input, existing item, removed last item, no active job.\n";
}
