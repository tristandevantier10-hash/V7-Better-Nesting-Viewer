#pragma once

//---------------------------------------------------------
// Application States
//---------------------------------------------------------
// Every screen in the program is represented by one state.
// The application will always be in exactly one of these
// states.
//
// The main loop will update and render based on this state.
//
// Current Flow:
//
// Splash
//    ?
// MainMenu
//    ?
// InteractiveJob  ?? TestJob
//    ?
// Calculating
//    ?
// Results
//    ?
// MainMenu
//---------------------------------------------------------

enum class AppState
{
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
