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

    //=================================================
    // Initial Screen
    //=================================================



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

    customerSelectionScreen.setBackCallback(
        [this]()
        {
            ui.setScreen(&mainMenu);
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
                currentJob.customer =
                    customers[index];

                newJobScreen.setJob(
                    &currentJob);
            }

            ui.setScreen(
                &newJobScreen);
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

            Customer customer =
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

    resultsScreen.setNewJobCallback(
        [this]()
        {
            beginNewJob();
            ui.setScreen(&newJobScreen);
        });

    resultsScreen.setBackCallback(
        [this]()
        {
            ui.setScreen(&mainMenu);
        });

    resultsScreen.setOverviewCallback(
        [this]()
        {
            ui.setScreen(&resultsScreen);
        });

    resultsScreen.setPrintCallback(
        [this]()
        {
            std::cout << "PRINT QUOTE\n";

            // Existing print functionality can be moved here.
        });

    resultsScreen.setExportCallback(
        [this]()
        {
            std::cout << "EXPORT PDF\n";

            // PDF export will be wired here.
        });

    state = AppState::Splash;

    running = true;

    splashStartTime = SDL_GetTicks();

    splashAnimation = 0.0f;
    startupProgress = 0.0f;

    startupStageStartTime =
        splashStartTime;

    readyStartTime = 0;

    startupCompletedStage = -1;

    splashFadingOut = false;
    mainMenuFadingIn = false;

    splashFadeAlpha = 0.0f;

    startupComplete = false;
    startupFailed = false;

    startupWorker =
        std::thread(
            [this]()
            {
                if (!DatabaseManager::initialise(
                    [this](int stage)
                    {
                        startupCompletedStage = stage;
                    }))
                {
                    startupFailed = true;
                    return;
                }

                startupComplete = true;
            });

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

            pendingResults.viewModel = vm;

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

    ui.tick(1.0f / 60.0f);

    Uint32 now =
        SDL_GetTicks();

    if (state == AppState::Splash)
    {
        splashAnimation += 0.02f;

        if (splashAnimation > 1.0f)
            splashAnimation = 0.0f;
    }

    if (splashFadingOut)
    {
        splashFadeAlpha += 8.0f;

        if (splashFadeAlpha >= 255.0f)
        {
            splashFadeAlpha = 255.0f;

            newJobScreen.initialiseMaterials();

            newJobScreen.setJob(
                &currentJob);

            mainMenu.refreshStatus();

            customerSelectionScreen.refreshCustomers();

            startupStage =
                StartupStage::Complete;

            state =
                AppState::MainMenu;

            ui.setScreen(
                &mainMenu);

            // Switch from fading OUT the splash
            // to fading IN the main menu.
            splashFadingOut = false;
            mainMenuFadingIn = true;
        }
    }
    else if (mainMenuFadingIn)
    {
        splashFadeAlpha -= 8.0f;

        if (splashFadeAlpha <= 0.0f)
        {
            splashFadeAlpha = 0.0f;
            mainMenuFadingIn = false;
        }
    }

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
        {
            if (startupFailed)
            {
                std::cout
                    << "FAILED: DatabaseManager\n";

                running = false;
                break;
            }

            if (startupComplete)
            {
                const Uint32 minimumStageTime =
                    1500; // Hold time for Stages

                Uint32 stageElapsed =
                    now - startupStageStartTime;

                int completedStage =
                    startupCompletedStage.load();

                startupProgress = 0.0f;

                //=================================================
                // Advance through startup stages one at a time
                //=================================================

                if (stageElapsed >= minimumStageTime)
                {
                    switch (startupStage.load())
                    {
                    case StartupStage::Materials:

                        if (completedStage >= 1)
                        {
                            startupProgress = 0.15f;

                            startupStage =
                                StartupStage::Pricing;

                            startupStageStartTime =
                                now;
                        }

                        break;

                    case StartupStage::Pricing:

                        if (completedStage >= 2)
                        {
                            startupProgress = 0.35f;

                            startupStage =
                                StartupStage::Customers;

                            startupStageStartTime =
                                now;
                        }

                        break;

                    case StartupStage::Customers:

                        if (completedStage >= 3)
                        {
                            startupProgress = 0.55f;

                            startupStage =
                                StartupStage::ProductionPricing;

                            startupStageStartTime =
                                now;
                        }

                        break;

                    case StartupStage::ProductionPricing:

                        if (completedStage >= 4)
                        {
                            startupProgress = 0.75f;

                            startupStage =
                                StartupStage::Complete;

                            startupStageStartTime =
                                now;
                        }

                        break;

                    case StartupStage::Complete:

                        startupProgress = 0.90f;

                        startupStage =
                            StartupStage::Ready;

                        readyStartTime =
                            now;

                        startupStageStartTime =
                            now;

                        break;
                    }
                }

                //=================================================
                // READY hold
                //=================================================

                if (startupStage.load() ==
                    StartupStage::Ready)
                {
                    Uint32 readyElapsed =
                        now - readyStartTime;

                    const Uint32 readyHoldTime =
                        3000;  // Hold time for Ready only

                    if (readyElapsed >= readyHoldTime &&
                        !splashFadingOut)
                    {
                        splashFadingOut = true;
                        splashFadeAlpha = 0.0f;
                    }
                }
            }

            break;
        }

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

    //=================================================
    // SPLASH SCREEN
    //=================================================

    if (state == AppState::Splash)
    {
        uiRenderer->beginFrame();

        //=================================================
        // Background
        //=================================================

        SDL_Color background =
        {
            248,
            248,
            247,
            255
        };

        SDL_SetRenderDrawColor(
            renderer,
            background.r,
            background.g,
            background.b,
            background.a);

        SDL_RenderClear(renderer);

        //=================================================
        // Colours
        //=================================================

        SDL_Color dark =
        {
            32,
            32,
            32,
            255
        };

        SDL_Color secondary =
        {
            105,
            105,
            105,
            255
        };

        SDL_Color light =
        {
            175,
            175,
            175,
            255
        };

        SDL_Color accent =
        {
            35,
            105,
            170,
            255
        };

        SDL_Color panel =
        {
            255,
            255,
            255,
            255
        };

        SDL_Color readyGreen =
        {
            46,
            160,
            67,
            255
        };

        //=================================================
        // Main layout
        //=================================================

        const int leftMargin = 110;
        const int contentTop = 150;

        //=================================================
        // Accent vertical line
        //=================================================

        SDL_Rect accentBar =
        {
            leftMargin,
            contentTop,
            5,
            300
        };

        uiRenderer->fillRect(
            accentBar,
            accent);

        //=================================================
        // Company
        //=================================================

        const std::string company =
            "E & G SIGNS CC";

        uiRenderer->drawText(
            company,
            leftMargin + 30,
            contentTop + 4,
            LabelStyle::Small,
            secondary);

        //=================================================
        // Main title
        //=================================================

        const std::string title =
            "SIGNAGE";

        const std::string subtitle =
            "COSTING SYSTEM";

        uiRenderer->drawText(
            title,
            leftMargin + 30,
            contentTop + 48,
            LabelStyle::Heading,
            dark);

        uiRenderer->drawText(
            subtitle,
            leftMargin + 30,
            contentTop + 92,
            LabelStyle::Heading,
            accent);

        //=================================================
        // Supporting line
        //=================================================

        SDL_Rect divider =
        {
            leftMargin + 30,
            contentTop + 145,
            420,
            1
        };

        uiRenderer->fillRect(
            divider,
            light);

        //=================================================
        // Startup status
        //=================================================

        std::string status = "INITIALISING APPLICATION";

        switch (startupStage.load())
        {
        case StartupStage::Materials:
            status =
                "LOADING MATERIAL DATABASE";
            break;

        case StartupStage::Pricing:
            status =
                "LOADING PRICING DATABASE";
            break;

        case StartupStage::Customers:
            status =
                "LOADING CUSTOMER DATABASE";
            break;

        case StartupStage::ProductionPricing:
            status =
                "LOADING PRODUCTION PRICING";
            break;

        case StartupStage::Complete:
            status =
                "PREPARING WORKSPACE";
            break;

        case StartupStage::Ready:
            status =
                "READY";
            break;
        }

        SDL_Color statusColour = secondary;

        if (startupStage.load() ==
            StartupStage::Ready)
        {
            statusColour = readyGreen;
        }

        uiRenderer->drawText(
            status,
            leftMargin + 30,
            contentTop + 175,
            LabelStyle::Small,
            statusColour);

        //=================================================
        // Loading track
        //=================================================

        const int trackWidth = 420;

        SDL_Rect loadingTrack =
        {
            leftMargin + 30,
            contentTop + 205,
            trackWidth,
            4
        };

        SDL_Color trackColour =
        {
            225,
            225,
            225,
            255
        };

        uiRenderer->fillRoundedRect(
            loadingTrack,
            trackColour,
            2);

        //=================================================
        // Animated loading indicator
        //=================================================

        const int progressWidth =
            static_cast<int>(
                trackWidth * startupProgress);

        SDL_Rect loadingProgress =
        {
            leftMargin + 30,
            contentTop + 205,
            progressWidth,
            4
        };

        if (progressWidth > 0)
        {
            uiRenderer->fillRoundedRect(
                loadingProgress,
                accent,
                2);
        }

        //=================================================
        // Information block
        //=================================================

        SDL_Rect infoPanel =
        {
            leftMargin + 30,
            contentTop + 245,
            420,
            58
        };

        uiRenderer->fillRoundedRect(
            infoPanel,
            panel,
            6);

        const std::string info =
            "Preparing materials, pricing and customer data";

        uiRenderer->drawText(
            info,
            leftMargin + 48,
            contentTop + 265,
            LabelStyle::Small,
            secondary);

        //=================================================
        // Version
        //=================================================

        const std::string version =
            "VERSION 3.0";

        int versionWidth =
            uiRenderer->getTextWidth(version);

        uiRenderer->drawText(
            version,
            w - versionWidth - 45,
            h - 40,
            LabelStyle::Small,
            light);

        //=================================================
        // Bottom accent
        //=================================================

        SDL_Rect bottomAccent =
        {
            0,
            h - 4,
            w,
            4
        };

        uiRenderer->fillRect(
            bottomAccent,
            accent);

        //=================================================
        // Splash fade-out
        //=================================================

        if (splashFadingOut)
        {
            SDL_SetRenderDrawBlendMode(
                renderer,
                SDL_BLENDMODE_BLEND);

            SDL_SetRenderDrawColor(
                renderer,
                0,
                0,
                0,
                static_cast<Uint8>(
                    splashFadeAlpha));

            SDL_Rect fadeRect =
            {
                0,
                0,
                w,
                h
            };

            SDL_RenderFillRect(
                renderer,
                &fadeRect);

            SDL_SetRenderDrawBlendMode(
                renderer,
                SDL_BLENDMODE_NONE);
        }

        uiRenderer->endFrame();

        return;

    }

    //=================================================
    // Main application UI
    //=================================================

    if (uiRenderer == nullptr)
    {
        return;
    }

    uiRenderer->beginFrame();

    ui.render(*uiRenderer);

    drawSheets();

    //=================================================
    // Main menu fade-in
    //=================================================

    if (mainMenuFadingIn)
    {
        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND);

        SDL_SetRenderDrawColor(
            renderer,
            0,
            0,
            0,
            static_cast<Uint8>(
                splashFadeAlpha));

        SDL_Rect fadeRect =
        {
            0,
            0,
            w,
            h
        };

        SDL_RenderFillRect(
            renderer,
            &fadeRect);

        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_NONE);
    }

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

    if (startupWorker.joinable())
    {
        startupWorker.join();
    }

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