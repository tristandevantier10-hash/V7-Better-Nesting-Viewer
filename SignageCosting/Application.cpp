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
#include <SDL2/SDL_image.h>

Application::Application()
{
}

Application::~Application()
{
    shutdown();
}

bool Application::loadSplashLogos()
{
    //=================================================
    // ESTIMATE PROGRAM LOGO
    //=================================================

    SDL_Surface* estimateSurface =
        IMG_Load("Assets/Images/EstiMate.png");

    if (!estimateSurface)
    {
        std::cout
            << "Could not load EstiMate.png: "
            << IMG_GetError()
            << "\n";

        return false;
    }

    estimateLogoTexture =
        SDL_CreateTextureFromSurface(
            renderer,
            estimateSurface);

    SDL_FreeSurface(estimateSurface);

    if (!estimateLogoTexture)
    {
        std::cout
            << "Could not create Estimate logo texture: "
            << SDL_GetError()
            << "\n";

        return false;
    }

    //=================================================
    // E&G SIGNS COMPANY LOGO
    //=================================================

    SDL_Surface* companySurface =
        IMG_Load("Assets/Images/E&G Signs cc.png");

    if (!companySurface)
    {
        std::cout
            << "Could not load E&G Signs cc.png: "
            << IMG_GetError()
            << "\n";

        return false;
    }

    companyLogoTexture =
        SDL_CreateTextureFromSurface(
            renderer,
            companySurface);

    SDL_FreeSurface(companySurface);

    if (!companyLogoTexture)
    {
        std::cout
            << "Could not create company logo texture: "
            << SDL_GetError()
            << "\n";

        return false;
    }

    //=================================================
    // SPLASH ARTWORK PNG
    //=================================================

    SDL_Surface* artworkSurface =
        IMG_Load("Assets/Images/SplashArtwork.png");

    if (artworkSurface)
    {
        splashArtworkTexture =
            SDL_CreateTextureFromSurface(
                renderer,
                artworkSurface);

        SDL_FreeSurface(artworkSurface);

        if (!splashArtworkTexture)
        {
            std::cout
                << "Could not create splash artwork texture: "
                << SDL_GetError()
                << "\n";

            // Artwork is optional while building the template.
        }
        else
        {
            SDL_SetTextureBlendMode(
                splashArtworkTexture,
                SDL_BLENDMODE_BLEND);
        }
    }
    else
    {
        // Optional until your artwork PNG is ready.
        std::cout
            << "Splash artwork not loaded yet: "
            << IMG_GetError()
            << "\n";
    }

    //=================================================
    // LOGO TRANSPARENCY
    //=================================================

    SDL_SetTextureBlendMode(
        estimateLogoTexture,
        SDL_BLENDMODE_BLEND);

    SDL_SetTextureBlendMode(
        companyLogoTexture,
        SDL_BLENDMODE_BLEND);

    std::cout << "Splash logos loaded successfully.\n";

    return true;
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

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        std::cout
            << "SDL_image PNG initialisation failed: "
            << IMG_GetError()
            << "\n";
    }
    else
    {
        if (!loadSplashLogos())
        {
            std::cout
                << "Warning: One or more splash logos failed to load.\n";
        }
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

    setSplashWindowSize();

    running = true;

    splashStartTime = SDL_GetTicks();

    splashAnimation = 0.0f;
    startupProgress = 0.0f;
    displayedProgress = 0.0f;

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

void Application::setSplashWindowSize()
{
    if (!window || splashWindowResized)
        return;

    // Compact popup dimensions for the startup screen.
    const int splashWidth = 1040;
    const int splashHeight = 460;

    SDL_SetWindowSize(
        window,
        splashWidth,
        splashHeight);

    SDL_SetWindowPosition(
        window,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED);

    splashWindowResized = true;
}

void Application::restoreMainWindowSize()
{
    if (!window || !splashWindowResized)
        return;

    // Restore the normal application window.
    SDL_SetWindowSize(
        window,
        1200,
        800);

    SDL_SetWindowPosition(
        window,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED);

    splashWindowResized = false;
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

    displayedProgress +=
        (startupProgress - displayedProgress) * 0.10f;

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

            restoreMainWindowSize();

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

                float stageFraction =
                    static_cast<float>(stageElapsed) /
                    static_cast<float>(minimumStageTime);

                if (stageFraction > 1.0f)
                    stageFraction = 1.0f;

                switch (startupStage.load())
                {
                case StartupStage::Materials:
                    startupProgress = 0.15f * stageFraction;
                    break;

                case StartupStage::Pricing:
                    startupProgress =
                        0.15f + 0.20f * stageFraction;
                    break;

                case StartupStage::Customers:
                    startupProgress =
                        0.35f + 0.20f * stageFraction;
                    break;

                case StartupStage::ProductionPricing:
                    startupProgress =
                        0.55f + 0.20f * stageFraction;
                    break;

                case StartupStage::Complete:
                    startupProgress =
                        0.75f + 0.15f * stageFraction;
                    break;

                case StartupStage::Ready:
                {
                    Uint32 readyElapsed =
                        now - readyStartTime;

                    startupProgress =
                        0.90f + 0.10f *
                        (static_cast<float>(readyElapsed) / 3000.0f);

                    if (startupProgress > 1.0f)
                        startupProgress = 1.0f;

                    break;
                }
                }

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
            242,
            245,
            248,
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
        // COMPACT SPLASH LAYOUT
        //=================================================

        const int contentWidth = 1000;
        const int contentHeight = 410;

        const int leftMargin = (w - contentWidth) / 2;
        const int contentTop = (h - contentHeight) / 2;

        const int contentPadding = 20;

        const int leftColumnX = leftMargin + 20;
        const int leftColumnWidth = 420;
        const int columnGap = 50;

        //=================================================
        // ESTIMATE BRANDING
        //=================================================

        if (estimateLogoTexture != nullptr)
        {
            int textureWidth = 0;
            int textureHeight = 0;

            SDL_QueryTexture(
                estimateLogoTexture,
                nullptr,
                nullptr,
                &textureWidth,
                &textureHeight);

            if (textureWidth > 0 && textureHeight > 0)
            {
                SDL_Rect logoBox =
                {
                    leftColumnX,
                    contentTop,
                    260,
                    72
                };

                float scaleX =
                    static_cast<float>(logoBox.w) / textureWidth;

                float scaleY =
                    static_cast<float>(logoBox.h) / textureHeight;

                float scale = scaleX < scaleY ? scaleX : scaleY;

                SDL_Rect logoRect =
                {
                    logoBox.x,

                    logoBox.y + (logoBox.h -
                        static_cast<int>(textureHeight * scale)) / 2,

                    static_cast<int>(textureWidth * scale),
                    static_cast<int>(textureHeight * scale)
                };

                SDL_RenderCopy(
                    renderer,
                    estimateLogoTexture,
                    nullptr,
                    &logoRect);
            }
        }
        else
        {
            uiRenderer->drawText(
                "EstiMate",
                leftColumnX,
                contentTop + 15,
                LabelStyle::Heading,
                dark);
        }

        // Welcome heading
        uiRenderer->drawText(
            "Welcome to EstiMate.",
            leftColumnX,
            contentTop + 100,
            LabelStyle::Heading,
            dark);

        // Supporting text
        uiRenderer->drawText(
            "Your workspace for smarter signage estimating.",
            leftColumnX,
            contentTop + 150,
            LabelStyle::Small,
            secondary);

        //=================================================
        // RIGHT-HAND ARTWORK PANEL
        //=================================================

        const int artX = leftMargin + 490;
        const int artY = contentTop;
        const int artW = 490;

        const int companyY = contentTop + 330;
        const int companyLogoHeight = 44;

        // Make the artwork panel end exactly at the bottom
        // of the E&G company logo box.
        const int artH =
            (companyY + companyLogoHeight) - artY;

        SDL_Color artBackground =
        {
            220, 239, 241, 255
        };

        SDL_Rect artPanel =
        {
            artX,
            artY,
            artW,
            artH
        };

        // Panel background — visible until the PNG is loaded.
        uiRenderer->fillRoundedRect(
            artPanel,
            artBackground,
            12
        );

        // Draw the supplied PNG, preserving its aspect ratio.
        if (splashArtworkTexture != nullptr)
        {
            int textureWidth = 0;
            int textureHeight = 0;

            SDL_QueryTexture(
                splashArtworkTexture,
                nullptr,
                nullptr,
                &textureWidth,
                &textureHeight
            );

            if (textureWidth > 0 && textureHeight > 0)
            {
                float scaleX =
                    static_cast<float>(artW) / textureWidth;

                float scaleY =
                    static_cast<float>(artH) / textureHeight;

                float scale =
                    scaleX < scaleY ? scaleX : scaleY;

                int drawW =
                    static_cast<int>(textureWidth * scale);

                int drawH =
                    static_cast<int>(textureHeight * scale);

                SDL_Rect imageRect =
                {
                    artX,
                    artY,
                    artW,
                    artH
                };

                SDL_RenderCopy(
                    renderer,
                    splashArtworkTexture,
                    nullptr,
                    &imageRect
                );
            }
        }

        //=================================================
        // Supporting line
        //=================================================

        SDL_Rect divider =
        {
            leftColumnX,
            contentTop + 200,
            leftColumnWidth,
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
            status = "LOADING MATERIAL DATABASE";
            break;

        case StartupStage::Pricing:
            status = "LOADING PRICING DATABASE";
            break;

        case StartupStage::Customers:
            status = "LOADING CUSTOMER DATABASE";
            break;

        case StartupStage::ProductionPricing:
            status = "LOADING PRODUCTION PRICING";
            break;

        case StartupStage::Complete:
            status = "PREPARING WORKSPACE";
            break;

        case StartupStage::Ready:
            status = "READY";
            break;
        }

        SDL_Color statusColour = secondary;

        if (startupStage.load() == StartupStage::Ready)
        {
            statusColour = readyGreen;
        }

        uiRenderer->drawText(
            status,
            leftColumnX,
            contentTop + 250,
            LabelStyle::Small,
            statusColour);

        //=================================================
        // Loading track
        //=================================================

        const int trackWidth = leftColumnWidth;

        SDL_Rect loadingTrack =
        {
            leftColumnX,
            contentTop + 280,
            trackWidth,
            4
        };

        SDL_Color trackColour =
        {
            225, 225, 225, 255
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
                trackWidth * displayedProgress);

        SDL_Rect loadingProgress =
        {
            leftColumnX,
            contentTop + 280,
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
        // COMPANY BRANDING FOOTER
        //=================================================

        if (companyLogoTexture != nullptr)
        {
            int textureWidth = 0;
            int textureHeight = 0;

            SDL_QueryTexture(
                companyLogoTexture,
                nullptr,
                nullptr,
                &textureWidth,
                &textureHeight);

            if (textureWidth > 0 && textureHeight > 0)
            {
                SDL_Rect logoBox =
                {
                    leftColumnX,
                    companyY,
                    90,
                    44
                };

                float scaleX =
                    static_cast<float>(logoBox.w) / textureWidth;

                float scaleY =
                    static_cast<float>(logoBox.h) / textureHeight;

                float scale = scaleX < scaleY ? scaleX : scaleY;

                SDL_Rect logoRect =
                {
                    logoBox.x + (logoBox.w -
                        static_cast<int>(textureWidth * scale)) / 2,

                    logoBox.y + (logoBox.h -
                        static_cast<int>(textureHeight * scale)) / 2,

                    static_cast<int>(textureWidth * scale),
                    static_cast<int>(textureHeight * scale)
                };

                SDL_RenderCopy(
                    renderer,
                    companyLogoTexture,
                    nullptr,
                    &logoRect);
            }
        }

        uiRenderer->drawText(
            "E & G SIGNS CC",
            leftColumnX + 110,
            companyY + 3,
            LabelStyle::Small,
            dark);

        uiRenderer->drawText(
            "SIGNAGE THAT GUIDES SOUTH AFRICA",
            leftColumnX + 110,
            companyY + 24,
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

    if (estimateLogoTexture)
    {
        SDL_DestroyTexture(estimateLogoTexture);
        estimateLogoTexture = nullptr;
    }

    if (companyLogoTexture)
    {
        SDL_DestroyTexture(companyLogoTexture);
        companyLogoTexture = nullptr;
    }

    IMG_Quit();

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