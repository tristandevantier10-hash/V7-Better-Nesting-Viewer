#include "Application.h"
#include <iostream>
#include "InvoicePrinter.h"
#include "TestJobFactory.h"
#include "MaterialDatabase.h"
#include "PricingDatabase.h"
#include "ProductionPricingDatabase.h"
#include "HttpClient.h"
#include "Format.h"
#include <thread>
#include <chrono>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <atomic>
#include <mutex>
#include "ResultsViewModelBuilder.h"
#include "TextRenderer.h"
#include "NewJobScreen.h"
#include <windows.h>
#include "CustomerDatabase.h"
#include "FileSystem.h"
#include "DatabaseManager.h"

Application::Application()
{
}

Application::~Application()
{
    shutdown();
}

bool Application::initialise()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "SDL_Init Failed\n";
        return false;
    }

    if (!createWindow())
    {
        std::cout << "FAILED: createWindow()\n";
        return false;
    }

    if (!createRenderer())
    {
        std::cout << "FAILED: createRenderer()\n";
        return false;
    }

    SDL_StartTextInput();

    if (!DatabaseManager::initialise())
    {
        std::cout << "FAILED: DatabaseManager\n";
        return false;
    }

    //=================================================
    // UI Initialisation
    //=================================================

    fontManager.initialise();

    TextRenderer::initialise(&fontManager);

    uiRenderer =
        new Renderer(
            renderer,
            fontManager,
            DefaultTheme);

    mainMenu.refreshStatus();

    newJobScreen.initialiseMaterials();

    newJobScreen.setJob(&currentJob);

    customerSelectionScreen.refreshCustomers();

    //=================================================
    // Initial Screen
    //=================================================

    ui.setScreen(&mainMenu);

    //=================================================
    // Callbacks
    //=================================================

    newJobScreen.setCalculateCallback(
        [this]()
        {
            calculateQuote();
        });

    mainMenu.setNewJobCallback(
        [this]()
        {
            ui.setScreen(&customerSelectionScreen);
        });

    customerEditorScreen.setSaveCallback(
        [this]()
        {
            if (customerEditorScreen.isEditMode())
            {
                CustomerDatabase::update(
                    customerEditorScreen.getEditingIndex(),
                    customerEditorScreen.createCustomer());
            }
            else
            {
                CustomerDatabase::add(
                    customerEditorScreen.createCustomer());
            }

            customerEditorScreen.clearEditor();

            customerSelectionScreen.refreshCustomers();

            ui.setScreen(&customerSelectionScreen);
        });

    customerSelectionScreen.setNewCustomerCallback(
        [this]()
        {
            ui.setScreen(&customerEditorScreen);
        });

    customerSelectionScreen.setSelectCustomerCallback(
        [this](int index)
        {
            const auto& customers =
                CustomerDatabase::getAll();

            if (index >= 0 &&
                index < static_cast<int>(customers.size()))
            {
                currentJob.customer = customers[index];
            }

            ui.setScreen(&newJobScreen);
        });

    customerSelectionScreen.setEditCustomerCallback(
        [this](int index)
        {
            if (index < 0)
                return;

            Customer customer =
                CustomerDatabase::get(index);

            customerEditorScreen.editCustomer(
                index,
                customer);

            ui.setScreen(&customerEditorScreen);
        });

    customerSelectionScreen.setDeleteCustomerCallback(
        [this](int index)
        {
            if (index < 0)
                return;

            const Customer customer =
                CustomerDatabase::get(index);

            std::string message =
                "Delete customer:\n\n" +
                customer.company +
                "\n\nThis action cannot be undone.";

            const SDL_MessageBoxButtonData buttons[] =
            {
                { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Delete" },
                { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" }
            };

            const SDL_MessageBoxData data =
            {
                SDL_MESSAGEBOX_WARNING,
                nullptr,
                "Confirm Delete",
                message.c_str(),
                2,
                buttons,
                nullptr
            };

            int button = 0;

            SDL_ShowMessageBox(&data, &button);

            if (button != 1)
                return;

            CustomerDatabase::remove(index);

            customerSelectionScreen.refreshCustomers();
        });

    resultsScreen.getToolbar().setNewJobCallback(
        [this]()
        {
            beginNewJob();
            ui.setScreen(&newJobScreen);
        });

    resultsScreen.getToolbar().setCalculateCallback(
        [this]()
        {
            calculateQuote();
        });

    state = AppState::MainMenu;

    running = true;

    worker =
        std::thread(
            &Application::workerThread,
            this
        );

    return true;
}

bool Application::createWindow()
{
    window = SDL_CreateWindow(
        "Signage Costing System",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1200,
        800,
        SDL_WINDOW_SHOWN
    );

    if (!window)
    {
        std::cout << "Failed creating window\n";
        return false;
    }

    return true;
}

bool Application::createRenderer()
{
    renderer =
        SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
        );

    if (!renderer)
    {
        renderer =
            SDL_CreateRenderer(
                window,
                -1,
                SDL_RENDERER_SOFTWARE
            );
    }

    if (!renderer)
    {
        std::cout << "Failed creating renderer\n";
        return false;
    }

    return true;
}

void Application::workerThread()
{
    std::cout << "WORKER STARTED\n";

    while (appRunning)
    {
        Job job;
        bool hasJob = false;

        {
            std::lock_guard<std::mutex> lock(jobMutex);

            if (!jobQueue.empty())
            {
                job = jobQueue.front();
                jobQueue.pop();
                hasJob = true;
            }
        }

        if (!hasJob)
        {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(50));
            continue;
        }

        std::cout << "WORKER: Processing job...\n";

        // Calculate
        CostResult result =
            engine.calculate(job);

        // Build UI data
        ResultsViewModel vm =
            ResultsViewModelBuilder::build(result);

        // Store sheets
        {
            std::lock_guard<std::mutex> lock(resultMutex);

            pendingResults.result = result;

            pendingResults.viewModel =
                ResultsViewModelBuilder::build(result);

            pendingResults.ready = true;
        }

        InvoicePrinter::print(result);

    }

    std::cout << "WORKER EXITED\n";
}

void Application::run()
{
    std::cout << "RUN STARTED\n";

    while (running)
    {
        processEvents();

        update();

        render();

        SDL_Delay(16);
    }
}

void Application::processEvents()
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {

        if (e.type == SDL_MOUSEWHEEL)
            std::cout << "Application received wheel\n";

        ui.update(e);

        if (e.type == SDL_QUIT)
        {
            running = false;
        }
    }
}

void Application::update()
    {

        {
            std::lock_guard<std::mutex> lock(resultMutex);

            if (pendingResults.ready)
            {

                resultsScreen.setViewModel(
                    pendingResults.viewModel);

                if (!pendingResults.result.nestingSheets.empty())
                {
                    resultsScreen.setSheets(
                        pendingResults.result.nestingSheets);

                }

                ui.setScreen(&resultsScreen);

                pendingResults.ready = false;
            }
        }

        switch (state)
        {
        case AppState::Splash:
            break;

        case AppState::MainMenu:
            break;

        case AppState::InteractiveJob:
            break;

        case AppState::TestJob:
            break;

        case AppState::Results:
            break;

        case AppState::Exit:

            running = false;

            break;
        }
    }

void Application::render()
{
    int w, h;

    SDL_GetRendererOutputSize(
        renderer,
        &w,
        &h);

    ui.setSize(w, h);

    if (uiRenderer == nullptr)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "DEBUG",
            "uiRenderer is nullptr",
            nullptr);

        return;
    }

    uiRenderer->beginFrame();

    ui.render(*uiRenderer);

    drawSheets();

    uiRenderer->endFrame();
}

void Application::toggleMode()
{
    mode = (mode == RunMode::Interactive)
        ? RunMode::Test
        : RunMode::Interactive;
}

/*
void Application::drawSheets()
{
    static bool printed = false;

    if (!printed)
    {
        std::cout << "DRAW SHEET CALLED\n";
        printed = true;
    }

    std::vector<Sheet> localCopy;

    {
        std::lock_guard<std::mutex> lock(sheetMutex);
        localCopy = allSheets;
    }

    static bool printedCount = false;

    if (!printedCount)
    {
        std::cout << "Sheets: " << localCopy.size() << "\n";
        printedCount = true;
    }

    int y = 100;

    // ---------------- FIX START ----------------
    static std::vector<size_t> lastPlacedCount;
    static std::vector<double> lastW;
    static std::vector<double> lastH;

    if (lastPlacedCount.size() != localCopy.size())
    {
        lastPlacedCount.assign(localCopy.size(), (size_t)-1);
        lastW.assign(localCopy.size(), -1.0);
        lastH.assign(localCopy.size(), -1.0);
    }
    // ---------------- FIX END ----------------

    for (size_t i = 0; i < localCopy.size(); i++)
    {
        const Sheet& sheet = localCopy[i];

        // print ONLY if changed
        if (sheet.placed.size() != lastPlacedCount[i] ||
            sheet.width != lastW[i] ||
            sheet.height != lastH[i])
        {
            std::cout << "Rendering sheet with placed: "
                << sheet.placed.size()
                << " size: "
                << sheet.width << "x" << sheet.height
                << "\n";

            lastPlacedCount[i] = sheet.placed.size();
            lastW[i] = sheet.width;
            lastH[i] = sheet.height;
        }

        nestingRenderer.drawSheet(
            renderer,
            sheet,
            100,
            y,
            800,
            500
        );

        y += 520;
    }
}*/

void Application::drawSheets()
{}

void Application::beginNewJob()
{
    currentJob = Job();

    newJobScreen.clearEditor();

    pendingResults = PendingResults();

    sheets.clear();
}

void Application::destroyRenderer()
{
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
}

void Application::destroyWindow()
{
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void Application::shutdown()
{
    running = false;
    appRunning = false;

    if (worker.joinable())
    {
        worker.join();
    }

    delete uiRenderer;
    uiRenderer = nullptr;

    fontManager.shutdown();

    destroyRenderer();

    destroyWindow();

    SDL_Quit();
}

SDL_Renderer* Application::getRenderer() const
{
    return renderer;
}

bool Application::isRunning() const
{
    return running;
}

void Application::calculateQuote()
{
    std::cout
        << "JOB ITEMS CREATED: "
        << currentJob.items.size()
        << "\n";

    for (const auto& item : currentJob.items)
    {
        std::cout
            << "Material : " << item.material.id << "\n"
            << "Width    : " << item.width << "\n"
            << "Height   : " << item.height << "\n"
            << "Quantity : " << item.quantity << "\n"
            << "Variant  : " << item.variantIndex << "\n\n";
    }

    {
        std::lock_guard<std::mutex> lock(jobMutex);
        jobQueue.push(currentJob);
    }

    state = AppState::Calculating;
}