#pragma once

//---------------------------------------------------------
// Application States
//---------------------------------------------------------
// Every screen in the program is represented by one state.
// The application will always be in exactly one of these
// states.
//
// Startup flow:
//
// Login
//    ↓
// Splash
//    ↓
// MainMenu
//    ↓
// InteractiveJob / TestJob
//    ↓
// Calculating
//    ↓
// Results
//    ↓
// MainMenu
//---------------------------------------------------------

enum class AppState
{
    // Authentication screen
    Login,

    // Startup splash screen
    Splash,

    // Main application menu
    MainMenu,

    // Normal production workflow
    InteractiveJob,

    // Debug / developer workflow
    TestJob,

    // CostEngine is calculating
    Calculating,

    // Display invoice + nesting
    Results,

    // Program shutting down
    Exit
};
