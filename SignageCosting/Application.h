#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include "AppState.h"
#include "RunMode.h"
#include "Job.h"
#include "CostEngine.h"
#include "NestingRender.h"
#include "UIManager.h"
#include "MainMenuScreen.h"
#include "ResultsScreen.h"
#include "PendingResults.h"
#include "Renderer.h"
#include "FontManager.h"
#include "Theme.h"
#include "NewJobScreen.h"
#include "CustomerSelectionScreen.h"
#include "CustomerEditorScreen.h"
#include "ApplicationShell.h"

class Application
{
public:

    Application();
    ~Application();

    bool initialise();
    void run();
    void shutdown();

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

    void destroyRenderer();
    void destroyWindow();

    //=================================================
    // Application State
    //=================================================

    bool running = true;

    AppState state = AppState::Splash;

    RunMode mode = RunMode::Test;

    float splashAnimation = 0.0f;

    float startupProgress = 0.0f;

    Uint32 splashStartTime = 0;

    Uint32 startupStageStartTime = 0;

    Uint32 readyStartTime = 0;

    bool splashFadingOut = false;

    bool mainMenuFadingIn = false;

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

    void workerThread();

    void calculateQuote();

    //=================================================
    // Rendering
    //=================================================

    void drawSheets();

    //=================================================
    // SDL Objects
    //=================================================

    SDL_Window* window = nullptr;

    SDL_Renderer* renderer = nullptr;

    //=================================================
    // UI
    //=================================================

    UIManager ui;

    MainMenuScreen mainMenu;

    NewJobScreen newJobScreen;

    ResultsScreen resultsScreen;

    CustomerSelectionScreen customerSelectionScreen;

    CustomerEditorScreen customerEditorScreen;

    ApplicationShell shell;

    //=================================================
    // Business Objects
    //=================================================

    CostEngine engine;

    Job currentJob;

    PendingResults pendingResults;

    std::queue<Job> jobQueue;

    std::mutex jobMutex;

    std::thread worker;

    std::atomic<bool> appRunning{ true };

    std::mutex resultMutex;

    //=================================================
    // Rendering Objects
    //=================================================

    FontManager fontManager;

    Renderer* uiRenderer = nullptr;

    NestingRenderer nestingRenderer;

    std::vector<Sheet> sheets;
};