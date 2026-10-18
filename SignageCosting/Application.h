#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vector>
#include <thread>
#include <atomic>
#include "AppState.h"
#include "RunMode.h"
#include "Job.h"
#include "JobProcessingService.h"
#include "NestingRender.h"
#include "UIManager.h"
#include "MainMenuScreen.h"
#include "ResultsScreen.h"
#include "Renderer.h"
#include "FontManager.h"
#include "Theme.h"
#include "NewJobScreen.h"
#include "CustomerSelectionScreen.h"
#include "CustomerEditorScreen.h"
#include "ApplicationShell.h"
#include "LoginScreen.h"
#include "AuthenticationService.h"
#include "QuoteHistoryScreen.h"
#include "SettingsScreen.h"
#include "LabourRatesScreen.h"

class Application
{
public:

    Application();
    ~Application();

    bool initialise();
    void run();
    void shutdown();

    void startSplashStartup();

    bool isRunning() const;
    SDL_Renderer* getRenderer() const;

private:

    //=================================================
    // Main Engine
    //=================================================

    void processEvents();
    void update();
    void render();

    //=================================================
    // SDL Initialisation
    //=================================================

    bool createWindow();
    bool createRenderer();

    void setSplashWindowSize();
    void restoreMainWindowSize();

    bool splashWindowResized = false;

    void destroyRenderer();
    void destroyWindow();

    //=================================================
    // Application State
    //=================================================

    bool running = true;

    AppState state = AppState::Login;

    RunMode mode = RunMode::Test;

    float splashAnimation = 0.0f;

    float startupProgress = 0.0f;

    float displayedProgress = 0.0f;

    Uint32 splashStartTime = 0;

    Uint32 startupStageStartTime = 0;

    Uint32 readyStartTime = 0;

    bool splashFadingOut = false;

    bool mainMenuFadingIn = false;

    bool loginToSplashFadingOut = false;

    bool loginToSplashFadingIn = false;

    float loginToSplashAlpha = 0.0f;

    float splashFadeAlpha = 0.0f;

    //=================================================
    // Startup Loading
    //=================================================

    enum class StartupStage
    {
        Materials,
        Pricing,
        Customers,
        ProductionPricing,
        Complete,
        Ready
    };

    std::atomic<StartupStage> startupStage =
        StartupStage::Materials;

    std::atomic<int> startupCompletedStage{ -1 };

    std::thread startupWorker;

    std::atomic<bool> startupComplete{ false };

    std::atomic<bool> startupFailed{ false };

    void toggleMode();

    //=================================================
    // Job Workflow
    //=================================================

    void beginNewJob();

    void buildJob();

    void createTestJob();

    void createInteractiveJob();

    void addInteractiveItem();

    void calculateQuote();

    //=================================================
    // SDL Objects
    //=================================================

    SDL_Window* window = nullptr;

    SDL_Renderer* renderer = nullptr;

    SDL_Texture* estimateLogoTexture = nullptr;

    SDL_Texture* companyLogoTexture = nullptr;

    std::vector<SDL_Texture*> splashGifFrames;
    std::vector<int> splashGifFrameDelays;
    std::size_t splashGifFrame = 0;
    Uint32 splashGifFrameStartedAt = 0;

    bool loadSplashLogos();
    bool splashAssetsLoaded = false;

    //=================================================
    // UI
    //=================================================

    UIManager ui;

    // Authentication
    LoginScreen loginScreen;
    AuthenticationService authenticationService;

    MainMenuScreen mainMenu;

    NewJobScreen newJobScreen;

    ResultsScreen resultsScreen;

    CustomerSelectionScreen customerSelectionScreen;

    CustomerEditorScreen customerEditorScreen;

    QuoteHistoryScreen quoteHistoryScreen;

    SettingsScreen settingsScreen;

    LabourRatesScreen labourRatesScreen;

    ApplicationShell shell;

    //=================================================
    // Business Objects
    //=================================================

    JobProcessingService jobProcessingService;

    Job currentJob;

    //=================================================
    // Rendering Objects
    //=================================================

    FontManager fontManager;

    TTF_Font* splashTitleFont = nullptr;
    TTF_Font* splashBodyFont = nullptr;

    Renderer* uiRenderer = nullptr;

    NestingRenderer nestingRenderer;

    std::vector<Sheet> sheets;
};
