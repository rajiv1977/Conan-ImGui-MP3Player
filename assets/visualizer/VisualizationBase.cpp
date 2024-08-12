#include "VisualizationBase.h"

std::atomic<int32_t> VisualizationBase::mScreenWidth                 = 100;
std::atomic<int32_t> VisualizationBase::mScreenHeight                = 100;
std::atomic<int32_t> VisualizationBase::mWorldWindowWidth            = 0;
std::atomic<int32_t> VisualizationBase::mWorldWindowHeight           = 0;
UTILITY::Vector2f_t  VisualizationBase::mWorldDisplayPos             = UTILITY::Vector2f_t(NAN, NAN);
UTILITY::Vector2f_t  VisualizationBase::mWorldCursorRelativePos      = UTILITY::Vector2f_t(NAN, NAN);
bool                 VisualizationBase::mWorldCursorIsHoveringWindow = false;

VisualizationBase::VisualizationBase()  = default;
VisualizationBase::~VisualizationBase() = default;

void VisualizationBase::setScreenDimensions()
{
    int32_t xPosition;
    int32_t yPosition;
    int32_t width;
    int32_t height;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &xPosition, &yPosition, &width, &height);
    mScreenWidth  = width;
    mScreenHeight = height - mWindowsTaskBarSize;
}

void VisualizationBase::worldInitFcn()
{
}

void VisualizationBase::worldCleanupFcn()
{
}

void VisualizationBase::worldFramePreDisplayFcn(bool demoMode)
{
}

void VisualizationBase::localFrameDisplayFcn()
{
}

void VisualizationBase::worldReadIniSettings()
{
}

GLFWwindow* VisualizationBase::createWindow(int32_t            width,
                                            int32_t            height,
                                            int32_t            horizontalPosition,
                                            int32_t            verticalPosition,
                                            const std::string& title,
                                            GLFWwindowsizefun  windowSizeCallback,
                                            GLFWkeyfun         keyCallback,
                                            GLFWcursorposfun   cursorPositionCallback,
                                            GLFWmousebuttonfun mouseButtonCallback,
                                            GLFWscrollfun      scrollCallback)
{
    // Provide window hints
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_FALSE);

    // Enable these option when we are ready to require all modern OpenGL functions
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (window == nullptr)
    {
        // Print a warning
        printf("Unable to create window: %s\n", title.c_str());
    }
    else
    {
        // Set the context to the current window
        makeContextCurrent(window);

        glfwMakeContextCurrent(window);

        // Set the swap interval to be 1 to avoid half frames
        glfwSwapInterval(1);

        // Initialize GLEW
        GLenum glewState = glewInit();
        if (glewState != GLEW_OK)
        {
            // Initialization failed
            printf("Error: %s\n", glewGetErrorString(glewState));
            exit(EXIT_FAILURE);
        }

        // Initialize the display settings
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        // glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Register callback functions
        glfwSetWindowSizeCallback(window, windowSizeCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetScrollCallback(window, scrollCallback);

        // Set the window size and position
        glfwSetWindowSize(window, width, height);
        glfwSetWindowPos(window, horizontalPosition, verticalPosition);
        if (windowSizeCallback != nullptr)
        {
            (*windowSizeCallback)(window, width, height);
        }
    }
    // Return the window pointer
    return window;
}

void VisualizationBase::createImGuiContext(GLFWwindow* window, bool installCallbacks)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    io.ConfigViewportsNoAutoMerge        = true;          // Force all windows to use separate viewport
    io.ConfigWindowsMoveFromTitleBarOnly = true;          // Allows mouse functions in world visualization

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Set the default style parameters
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.FrameRounding               = 3.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, installCallbacks);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Setup Font
    ImGui::GetIO().Fonts->Clear();
    ImFontConfig font_cfg;
    strcpy(font_cfg.Name, "Roboto");
    font_cfg.PixelSnapH           = true;
    font_cfg.OversampleH          = 5;
    font_cfg.OversampleV          = 5;
    font_cfg.FontDataOwnedByAtlas = false;
    ImGui::GetIO().Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf,
                                               Roboto_Regular_ttf_len,
                                               std::round(16),
                                               &font_cfg,
                                               ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
}

void VisualizationBase::newFrameImGui()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void VisualizationBase::renderImGui()
{
    // Render the ImGui content
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this
    // code elsewhere. For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backupCurrentContext);
    }
}

void VisualizationBase::hideWindow(GLFWwindow* window)
{
    // Hide the input window
    glfwHideWindow(window);
}

void VisualizationBase::showWindow(GLFWwindow* window)
{
    // Show the input window
    glfwShowWindow(window);
}

void VisualizationBase::makeContextCurrent(GLFWwindow* window)
{
    // Set the context to the window input
    glfwMakeContextCurrent(window);
}

void VisualizationBase::setWindowTitle(GLFWwindow* window, const std::string& title)
{
    // Set the title
    glfwSetWindowTitle(window, title.c_str());
}

UTILITY::Vector2f_t VisualizationBase::getCursorPostionInWindow(GLFWwindow* window)
{
    // Get the current cursor position
    float64_t xCursorPostionInWindow;
    float64_t yCursorPostionInWindow;
    glfwGetCursorPos(window, &xCursorPostionInWindow, &yCursorPostionInWindow);
    return {xCursorPostionInWindow, yCursorPostionInWindow};
}

unsigned int VisualizationBase::handleGLErrors()
{
    int         errorStackCount = 0;
    GLenum      err;
    std::string error;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        // log the error.
        errorStackCount++;
        switch (err)
        {
            case GL_INVALID_ENUM:
                error = "AN UNACCEPTABLE VALUE IS SPECIFIED FOR AN ENUMERATED ARGUMENT";
                break;
            case GL_INVALID_VALUE:
                error = "A NUMERIC ARGUMENT IS OUT OF RANGE";
                break;
            case GL_INVALID_OPERATION:
                error = "THE SPECIFIED OPERATION IS NOT ALLOWED IN THE CURRENT STATE";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "THE FRAMEBUFFER OBJECT IS NOT COMPLETE";
                break;
            case GL_OUT_OF_MEMORY:
                error = "THERE IS NOT ENOUGH MEMORY LEFT TO EXECUTE THE COMMAND";
                break;
            case GL_STACK_UNDERFLOW:
                error = "AN INTERNAL STACK TO UNDERFLOW";
                break;
            case GL_STACK_OVERFLOW:
                error = "AN INTERNAL STACK TO OVERFLOW";
                break;
        };
    }

    if (errorStackCount > 0)
    {
        std::stringstream ss;
        ss << "VisualizationBase::handleGLErrors: " << error << std::endl;
        throw std::runtime_error(ss.str());
    }

    return errorStackCount;
}

GLint VisualizationBase::getWindowTextRows()
{
    // Get the current window dimensions
    int32_t width;
    int32_t height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    return height / mWindowTextLineSpacing;
}

GLint VisualizationBase::getWindowTextColumns()
{
    // Get the current window dimensions
    int32_t width;
    int32_t height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    return width / mWindowTextCharSpacing;
}
