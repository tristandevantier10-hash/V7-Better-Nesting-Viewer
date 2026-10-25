#include "Application.h"
#include <iostream>
#include "TestJobFactory.h"
#include "MaterialDatabase.h"
#include "PricingDatabase.h"
#include "ProductionPricingDatabase.h"
#include "HttpClient.h"
#include "Format.h"
#include <thread>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <atomic>
#include "TextRenderer.h"
#include "NewJobScreen.h"
#include <windows.h>
#include "CustomerDatabase.h"
#include "FileSystem.h"
#include "DatabaseManager.h"
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <string>
#include <algorithm>
#include "QuoteDatabase.h"
#include "QuotePdfExporter.h"
#include "PrimaryNavigation.h"
#include <filesystem>
#include <system_error>
#include <cmath>

namespace
{
    std::string findOriginalQuotePdf(const std::string& reference)
    {
        if (reference.empty())
            return {};

        namespace fs = std::filesystem;
        std::error_code error;
        fs::path directory = fs::current_path(error);
        if (error)
            return {};

        const std::string filename = reference + ".pdf";
        for (int level = 0; level < 6 && !directory.empty(); ++level)
        {
            const fs::path candidates[] =
            {
                directory / filename,
                directory / "Quotes" / filename,
                directory / "SignageCosting" / filename
            };
            for (const auto& candidate : candidates)
            {
                error.clear();
                if (fs::is_regular_file(candidate, error) && !error)
                {
                    error.clear();
                    const fs::path absolutePath = fs::absolute(candidate, error);
                    return error ? candidate.string() : absolutePath.string();
                }
            }
            directory = directory.parent_path();
        }
        return {};
    }
}
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
        std::cerr
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
        std::cerr
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
        std::cerr
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
        std::cerr
            << "Could not create company logo texture: "
            << SDL_GetError()
            << "\n";

        return false;
    }

    //=================================================
    // SPLASH ANIMATED GIF
    //=================================================

    IMG_Animation* splashAnimation =
        IMG_LoadAnimation("Assets/Images/EstiMate_LaserReveal.gif");

    if (splashAnimation)
    {
        splashGifFrames.reserve(splashAnimation->count);
        splashGifFrameDelays.reserve(splashAnimation->count);

        for (int i = 0; i < splashAnimation->count; ++i)
        {
            SDL_Texture* frameTexture = SDL_CreateTextureFromSurface(
                renderer,
                splashAnimation->frames[i]);

            if (!frameTexture)
            {
                std::cerr << "Could not create splash GIF frame: "
                    << SDL_GetError() << "\n";
                break;
            }

            SDL_SetTextureBlendMode(frameTexture, SDL_BLENDMODE_BLEND);
            splashGifFrames.push_back(frameTexture);
            splashGifFrameDelays.push_back(
                (std::max)(20, splashAnimation->delays[i]));
        }

        IMG_FreeAnimation(splashAnimation);
    }

    if (splashGifFrames.empty())
    {
        std::cerr << "Could not load splash GIF: " << IMG_GetError() << "\n";
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

    return true;
}

bool Application::initialise()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init Failed\n";
        return false;
    }

    if (!createWindow())
    {
        std::cerr << "FAILED: createWindow()\n";
        return false;
    }

    if (!createRenderer())
    {
        std::cerr << "FAILED: createRenderer()\n";
        return false;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        std::cerr
            << "SDL_image PNG initialisation failed: "
            << IMG_GetError()
            << "\n";
    }

    SDL_StartTextInput();

    //=================================================
    // UI Initialisation
    //=================================================

    fontManager.initialise();
    splashTitleFont = TTF_OpenFont(
        "Fonts/BasisGrotesqueArabicPro-Bold.ttf",
        32);
    splashBodyFont = TTF_OpenFont(
        "Fonts/BasisGrotesqueArabicPro-Regular.ttf",
        16);
    loginScreen.setWelcomeFont(splashTitleFont);

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

    loginScreen.setLoginCallback(
        [this](
            const std::string& username,
            const std::string& password)
        {
            // TEMPORARY DEVELOPMENT BYPASS
            // Only available in Debug builds.
    #ifdef _DEBUG
            char* devPassword = nullptr;
            size_t passwordLength = 0;

            if (_dupenv_s(
                &devPassword,
                &passwordLength,
                "ESTIMATE_DEV_PASSWORD") != 0)
            {
                devPassword = nullptr;
            }

            if (devPassword != nullptr &&
                passwordLength > 1 &&
                password == devPassword)
            {
                free(devPassword);

                loginScreen.setStatus(
                    "Developer access accepted. Preparing workspace...");

                loginScreen.clearPassword();

                startSplashStartup();
                return;
            }

            free(devPassword);
    #endif

            // NORMAL EMPLOYEE AUTHENTICATION
            std::string errorMessage;

            if (authenticationService.login(
                username,
                password,
                errorMessage))
            {
                loginScreen.setStatus(
                    "Login successful. Preparing workspace...");

                loginScreen.clearPassword();

                startSplashStartup();
            }
            else
            {
                loginScreen.setStatus(errorMessage);

                std::cerr
                    << "Authentication failed: "
                    << errorMessage
                    << "\n";
            }
        });

    newJobScreen.setCalculateCallback(
        [this]()
        {
            calculateQuote();
        });

    mainMenu.setNewJobCallback(
        [this]()
        {
            customerSelectionScreen.setSelectionMode(true);
            customerSelectionScreen.refreshCustomers();
            ui.setScreen(&customerSelectionScreen);
        });

    mainMenu.setQuotesCallback(
        [this]()
        {
            quoteHistoryScreen.refreshQuotes();
            ui.setScreen(&quoteHistoryScreen);
        });

    mainMenu.setCustomerDatabaseCallback(
        [this]()
        {
            customerSelectionScreen.setSelectionMode(false);
            customerSelectionScreen.refreshCustomers();
            ui.setScreen(&customerSelectionScreen);
        });

    mainMenu.setSettingsCallback(
        [this]()
        {
            ui.setScreen(&settingsScreen);
        });

    mainMenu.setLabourRatesCallback([this]()
    {
        if (!authenticationService.isAdmin() && mode != RunMode::Test)
            return;
        labourRatesScreen.refresh();
        ui.setScreen(&labourRatesScreen);
    });
    labourRatesScreen.setBackCallback([this]() { ui.setScreen(&mainMenu); });

    settingsScreen.setBackCallback(
        [this]()
        {
            ui.setScreen(&mainMenu);
        });

    auto navigateFromPage = [this](PrimaryNavigationRoute route)
    {
        switch (route)
        {
        case PrimaryNavigationRoute::NewJob:
            customerSelectionScreen.setSelectionMode(true);
            customerSelectionScreen.refreshCustomers();
            ui.setScreen(&customerSelectionScreen);
            break;
        case PrimaryNavigationRoute::Quotes:
            quoteHistoryScreen.refreshQuotes();
            ui.setScreen(&quoteHistoryScreen);
            break;
        case PrimaryNavigationRoute::CustomerDatabase:
            customerSelectionScreen.setSelectionMode(false);
            customerSelectionScreen.refreshCustomers();
            ui.setScreen(&customerSelectionScreen);
            break;
        case PrimaryNavigationRoute::Settings:
            ui.setScreen(&settingsScreen);
            break;
        case PrimaryNavigationRoute::LabourRates:
            if (authenticationService.isAdmin() || mode == RunMode::Test)
            {
                labourRatesScreen.refresh();
                ui.setScreen(&labourRatesScreen);
            }
            break;
        case PrimaryNavigationRoute::Exit:
            state = AppState::Exit;
            break;
        case PrimaryNavigationRoute::MaterialDatabase:
            break;
        }
    };
    quoteHistoryScreen.setNavigationCallback(navigateFromPage);
    settingsScreen.setNavigationCallback(navigateFromPage);
    labourRatesScreen.setNavigationCallback(navigateFromPage);
    newJobScreen.setNavigationCallback(navigateFromPage);

    mainMenu.setExitCallback(
        [this]()
        {
            state = AppState::Exit;
        });

    quoteHistoryScreen.setBackCallback([this]()
    {
        ui.setScreen(&mainMenu);
    });
    quoteHistoryScreen.setQuoteSelectedCallback(
        [this](const QuoteRecord& quote)
        {
            std::string pdfPath = findOriginalQuotePdf(quote.invoice.jobref);
            if (pdfPath.empty())
            {
                std::filesystem::path generatedPath =
                    std::filesystem::path(FileSystem::getQuotesFolder()) /
                    ((quote.invoice.jobref.empty() ? std::string("Quote") : quote.invoice.jobref) + ".pdf");
                std::string errorMessage;
                if (!QuotePdfExporter::exportToPdf(
                    quote.invoice, generatedPath.string(), errorMessage))
                {
                    std::cerr << "Could not create quote PDF: " << errorMessage << std::endl;
                    return std::string{};
                }
                std::error_code error;
                pdfPath = std::filesystem::absolute(generatedPath, error).string();
                if (error)
                    pdfPath = generatedPath.string();
            }

            return pdfPath;
        });
    quoteHistoryScreen.setViewOverviewCallback(
        [this](const QuoteRecord& quote)
        {
            ResultsViewModel viewModel;
            viewModel.invoice = quote.invoice;
            viewModel.sheetsUsed = 0;
            resultsScreen.setViewModel(viewModel);
            resultsScreen.setSheets({});
            ui.setScreen(&resultsScreen);
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

            // Existing print functionality can be moved here.
        });

    resultsScreen.setSaveAndExportQuoteCallback(
        [this](InvoiceViewModel& invoice, std::string& errorMessage) -> bool
        {
            if (invoice.jobs.empty())
            {
                errorMessage = "There are no quotation items to save or export.";
                SDL_ShowSimpleMessageBox(
                    SDL_MESSAGEBOX_ERROR,
                    "Quote Has No Items",
                    errorMessage.c_str(),
                    window);
                return false;
            }

            const bool quoteNeedsSaving = invoice.jobref.empty();

            if (quoteNeedsSaving && !QuoteDatabase::saveQuote(invoice))
            {
                errorMessage = "The quote could not be saved.";
                SDL_ShowSimpleMessageBox(
                    SDL_MESSAGEBOX_ERROR,
                    "Quote Save Failed",
                    errorMessage.c_str(),
                    window);
                return false;
            }

            if (quoteNeedsSaving)
            {
                std::cout << "QUOTE SAVED: " << invoice.jobref << '\n';
            }

            const std::string outputPath =
                FileSystem::getQuotesFolder() + invoice.jobref + ".pdf";
            if (!QuotePdfExporter::exportToPdf(invoice, outputPath, errorMessage))
            {
                std::cerr << "PDF export failed: " << errorMessage << '\n';
                const std::string message =
                    "The quote was saved as " + invoice.jobref +
                    ", but its PDF could not be created. " + errorMessage +
                    "\n\nClick the button again to retry the PDF export.";
                SDL_ShowSimpleMessageBox(
                    SDL_MESSAGEBOX_ERROR,
                    "PDF Export Failed",
                    message.c_str(),
                    window);
                return false;
            }

            return true;
        });

    state = AppState::Login;
    ui.setScreen(&loginScreen);

    running = true;

    jobProcessingService.start();

    return true;

}

void Application::startSplashStartup() {
    // Splash visuals are not needed to sign in. Defer their synchronous image
    // and GIF loading until after authentication so the login appears quickly.
    if (!splashAssetsLoaded)
    {
        if (!loadSplashLogos())
            std::cerr << "Warning: One or more splash logos failed to load.\n";
        splashAssetsLoaded = true;
    }

    // 1. Reset state flags
    running = true;

    // 2. Initialize timing & animation properties
    splashStartTime = 0;
    startupStageStartTime = 0;
    readyStartTime = 0;
    splashAnimation = 0.0f;
    splashGifFrame = 0;
    splashGifFrameStartedAt = SDL_GetTicks();
    startupProgress = 0.0f;
    displayedProgress = 0.0f;

    // 3. Reset state flow control
    splashFadingOut = false;
    mainMenuFadingIn = false;
    splashFadeAlpha = 0.0f;
    loginToSplashFadingOut = true;
    loginToSplashFadingIn = false;
    loginToSplashAlpha = 0.0f;

    // Use atomic stores to guarantee thread safety
    startupComplete.store(false);
    startupFailed.store(false);
    startupErrorMessage.clear();
    startupCompletedStage.store(-1);

    // 4. Clean up any existing worker thread before creating a new one
    if (startupWorker.joinable()) {
        startupWorker.join();
    }

    // 5. Launch the asynchronous initialization thread
    startupWorker = std::thread([this]() {
        bool initSuccess = DatabaseManager::initialise([this](int stage) {
            startupCompletedStage.store(stage);
            }, &startupErrorMessage);

        if (!initSuccess) {
            startupFailed.store(true);
            return;
        }

        startupComplete.store(true);
        });
}

void Application::setSplashWindowSize()
{
    if (!window || splashWindowResized)
        return;

    // Compact popup dimensions for the startup screen.
    const int splashWidth = 1000;
    const int splashHeight = 400;

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
        600,
        600,
        SDL_WINDOW_SHOWN
    );

    if (!window)
    {
            std::cerr << "Failed creating window.\n";
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
            std::cerr << "Failed creating renderer.\n";
        return false;
    }

    return true;
}

void Application::run()
{

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

    if (loginToSplashFadingOut)
    {
        loginToSplashAlpha += 20.0f;
        if (loginToSplashAlpha >= 255.0f)
        {
            loginToSplashAlpha = 255.0f;
            // Keep the login at full size until its fade is complete.
            setSplashWindowSize();
            loginToSplashFadingOut = false;
            loginToSplashFadingIn = true;
            state = AppState::Splash;
            splashStartTime = now;
            splashGifFrame = 0;
            splashGifFrameStartedAt = now;
            startupStageStartTime = now;
        }
    }
    else if (loginToSplashFadingIn)
    {
        loginToSplashAlpha -= 20.0f;
        if (loginToSplashAlpha <= 0.0f)
        {
            loginToSplashAlpha = 0.0f;
            loginToSplashFadingIn = false;
        }
    }

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
            mainMenu.setLabourRatesAvailable(
                authenticationService.isAdmin() || mode == RunMode::Test);

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

        PendingResults completedResults;
        if (jobProcessingService.tryTakeResult(completedResults))
        {
            resultsScreen.setViewModel(completedResults.viewModel);

            if (!completedResults.result.nestingSheets.empty())
            {
                resultsScreen.setSheets(completedResults.result.nestingSheets);
            }

            ui.setScreen(&resultsScreen);
        }

        switch (state)
        {

        case AppState::Splash:
        {
            if (startupFailed)
            {
                std::cerr
                    << "Startup failed: " << startupErrorMessage << '\n';

                SDL_ShowSimpleMessageBox(
                    SDL_MESSAGEBOX_ERROR,
                    "Startup Failed",
                    startupErrorMessage.empty()
                        ? "The application could not load required data. Please restart and try again."
                        : startupErrorMessage.c_str(),
                    window);

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
            250,
            251,
            253,
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
            215,
            221,
            228,
            255
        };

        SDL_Color accent =
        {
            0,
            170,
            90,
            255
        };

        const SDL_Color versionText = { 145, 153, 165, 255 };

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

        const int contentWidth = 960;
        const int contentHeight = 360;

        const int leftMargin = (w - contentWidth) / 2;
        const int contentTop = (h - contentHeight) / 2;

        const int leftColumnX = leftMargin + 20;
        const int leftColumnWidth = 360;

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
                    280,
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

        uiRenderer->fillRoundedRect(
            { leftColumnX, contentTop + 92, 46, 4 },
            accent,
            2);

        // Compact two-line welcome heading, using the app's bold face at splash scale.
        TTF_Font* splashHeadlineFont = splashTitleFont
            ? splashTitleFont
            : fontManager.getHeadingFont();

        uiRenderer->drawText(
            "Your signage estimates,",
            leftColumnX,
            contentTop + 118,
            splashHeadlineFont,
            dark);

        uiRenderer->drawText(
            "made simple.",
            leftColumnX,
            contentTop + 153,
            splashHeadlineFont,
            dark);

        // Supporting text
        if (splashBodyFont)
        {
            uiRenderer->drawText(
                "A smarter workspace for signage estimating.",
                leftColumnX,
                contentTop + 197,
                splashBodyFont,
                secondary);
        }
        else
        {
            uiRenderer->drawText(
                "A smarter workspace for signage estimating.",
                leftColumnX,
                contentTop + 197,
                LabelStyle::Small,
                secondary);
        }

        //=================================================
        // RIGHT-HAND ARTWORK PANEL
        //=================================================

        const int artX = leftColumnX + leftColumnWidth + 30;
        const int artY = contentTop - 8;
        const int artW = 550;

        const int companyY = contentTop + 310;
        const int companyLogoHeight = 38;

        // Make the artwork panel end exactly at the bottom
        // of the E&G company logo box.
        const int artH =
            (companyY + companyLogoHeight) - artY;

        const SDL_Color artBackground = background;

        SDL_Rect artPanel =
        {
            artX,
            artY,
            artW,
            artH
        };

        // Panel background - visible until the PNG is loaded.
        uiRenderer->fillRoundedRect(
            artPanel,
            artBackground,
            12
        );

        // Draw the supplied GIF frame, preserving its aspect ratio.
        if (!splashGifFrames.empty())
        {
            int textureWidth = 0;
            int textureHeight = 0;

            const Uint32 now = SDL_GetTicks();
            if (splashGifFrame >= splashGifFrames.size())
                splashGifFrame = 0;

            while (now - splashGifFrameStartedAt >= static_cast<Uint32>(
                splashGifFrameDelays[splashGifFrame]))
            {
                splashGifFrameStartedAt += static_cast<Uint32>(
                    splashGifFrameDelays[splashGifFrame]);
                splashGifFrame = (splashGifFrame + 1) % splashGifFrames.size();
            }

            SDL_Texture* currentFrame = splashGifFrames[splashGifFrame];

            SDL_QueryTexture(
                currentFrame,
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
                    artX + (artW - drawW) / 2,
                    artY + (artH - drawH) / 2,
                    drawW,
                    drawH
                };

                SDL_RenderCopy(
                    renderer,
                    currentFrame,
                    nullptr,
                    &imageRect
                );

                constexpr int cornerRadius = 10;
                for (int row = 0; row < cornerRadius; ++row)
                {
                    const float dy = cornerRadius - (row + 0.5f);
                    const int inset = static_cast<int>(std::ceil(
                        cornerRadius - std::sqrt(
                            cornerRadius * cornerRadius - dy * dy)));
                    const int maskWidth = (std::min)(inset, imageRect.w / 2);

                    uiRenderer->fillRect(
                        { imageRect.x, imageRect.y + row, maskWidth, 1 },
                        background);
                    uiRenderer->fillRect(
                        { imageRect.x + imageRect.w - maskWidth,
                          imageRect.y + row, maskWidth, 1 },
                        background);
                    uiRenderer->fillRect(
                        { imageRect.x, imageRect.y + imageRect.h - 1 - row,
                          maskWidth, 1 },
                        background);
                    uiRenderer->fillRect(
                        { imageRect.x + imageRect.w - maskWidth,
                          imageRect.y + imageRect.h - 1 - row, maskWidth, 1 },
                        background);
                }

                uiRenderer->drawRoundedRect(
                    imageRect,
                    { 222, 228, 235, 255 },
                    cornerRadius);
            }
        }

        //=================================================
        // Supporting line
        //=================================================

        SDL_Rect divider =
        {
            leftColumnX,
            contentTop + 230,
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
            contentTop + 256,
            LabelStyle::Small,
            statusColour);

        //=================================================
        // Loading track
        //=================================================

        const int trackWidth = leftColumnWidth;

        SDL_Rect loadingTrack =
        {
            leftColumnX,
            contentTop + 274,
            trackWidth,
            12
        };

        SDL_Color trackColour =
        {
            222, 228, 235, 255
        };

        uiRenderer->fillRoundedRect(
            loadingTrack,
            trackColour,
            6);

        //=================================================
        // Animated loading indicator
        //=================================================

        const int progressWidth =
            static_cast<int>(
                trackWidth * displayedProgress);

        SDL_Rect loadingProgress =
        {
            leftColumnX,
            contentTop + 274,
            progressWidth,
            12
        };

        if (progressWidth > 0)
        {
            uiRenderer->fillRoundedRect(
                loadingProgress,
                accent,
                6);
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
                    86,
                    38
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
            leftColumnX + 104,
            companyY + 3,
            LabelStyle::Small,
            dark);

        uiRenderer->drawText(
            "SIGNAGE THAT GUIDES SOUTH AFRICA",
            leftColumnX + 104,
            companyY + 21,
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
            w - versionWidth - 28,
            h - 30,
            LabelStyle::Small,
            versionText);

        uiRenderer->drawRoundedRect(
            { 1, 1, w - 2, h - 2 },
            { 205, 214, 224, 255 },
            9);

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

        if (loginToSplashFadingIn)
        {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<Uint8>(loginToSplashAlpha));
            SDL_Rect transitionRect = { 0, 0, w, h };
            SDL_RenderFillRect(renderer, &transitionRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
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

    if (loginToSplashFadingOut)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<Uint8>(loginToSplashAlpha));
        SDL_Rect transitionRect = { 0, 0, w, h };
        SDL_RenderFillRect(renderer, &transitionRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

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

void Application::beginNewJob()
{
    currentJob = Job();

    newJobScreen.clearEditor();

    jobProcessingService.clearPendingResults();

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

    if (startupWorker.joinable())
    {
        startupWorker.join();
    }

    jobProcessingService.stop();

    delete uiRenderer;
    uiRenderer = nullptr;

    if (splashTitleFont)
    {
        TTF_CloseFont(splashTitleFont);
        splashTitleFont = nullptr;
    }

    if (splashBodyFont)
    {
        TTF_CloseFont(splashBodyFont);
        splashBodyFont = nullptr;
    }

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

    for (SDL_Texture* frameTexture : splashGifFrames)
    {
        if (frameTexture)
            SDL_DestroyTexture(frameTexture);
    }
    splashGifFrames.clear();
    splashGifFrameDelays.clear();

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
    if (!jobProcessingService.submit(currentJob))
    {
        std::cerr << "Job processing service is not running.\n";
    }

    state = AppState::Calculating;
}
