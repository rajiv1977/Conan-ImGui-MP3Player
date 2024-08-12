#pragma once

#include <algorithm>
#include <atomic>
#include <array>
#include <mutex>
#include <queue>
#include <string>

#include "UTILITYColors.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <chrono>
#include <imfonts.h>
#include "implot.h"

class VisualizationBase
{
  public:
    static const int32_t        mWindowsTaskBarSize = 30;
    static const int32_t        mWindowNameBarSize  = 30;
    static std::atomic<int32_t> mScreenWidth;
    static std::atomic<int32_t> mScreenHeight;
    static void                 setScreenDimensions();

    // Common world visualization parameters
    static std::atomic<int32_t> mWorldWindowWidth;
    static std::atomic<int32_t> mWorldWindowHeight;

    // Required functions for the world visualization
    virtual void worldInitFcn();
    virtual void worldCleanupFcn();
    virtual void worldFramePreDisplayFcn(bool demoMode);
    virtual void localFrameDisplayFcn();
    virtual void worldReadIniSettings();

    // protected:
    VisualizationBase();
    virtual ~VisualizationBase();

    // Common functions/parameters
    mutable std::mutex  mLock;
    GLFWwindow*         createWindow(int32_t            width,
                                     int32_t            height,
                                     int32_t            horizontalPosition,
                                     int32_t            verticalPosition,
                                     const std::string& title,
                                     GLFWwindowsizefun  windowSizeCallback     = nullptr,
                                     GLFWkeyfun         keyCallback            = nullptr,
                                     GLFWcursorposfun   cursorPositionCallback = nullptr,
                                     GLFWmousebuttonfun mouseButtonCallback    = nullptr,
                                     GLFWscrollfun      scrollCallback         = nullptr);
    void                createImGuiContext(GLFWwindow* window, bool installCallbacks = true);
    void                newFrameImGui();
    void                renderImGui();
    void                hideWindow(GLFWwindow* window);
    void                showWindow(GLFWwindow* window);
    void                makeContextCurrent(GLFWwindow* window);
    void                setWindowTitle(GLFWwindow* window, const std::string& title);
    UTILITY::Vector2f_t getCursorPostionInWindow(GLFWwindow* window);

    // Utility functions
    static unsigned int handleGLErrors(); // To be removed

    // Window text functions/parameters
    static UTILITY::Vector2f_t mWorldDisplayPos;             // Screen coordinates
    static UTILITY::Vector2f_t mWorldCursorRelativePos;      // Screen coordinates relative to the world window origin
    static bool                mWorldCursorIsHoveringWindow; // True when cursor is over hovering over the ImGui window
    static const GLint         mWindowTextLineSpacing = 14;  // Only valid for Arial 12 pt.
    static const GLint         mWindowTextCharSpacing = 8;   // Approximation is only valid for for Arial 12 pt.
    static GLint               getWindowTextRows();
    static GLint               getWindowTextColumns();
};
