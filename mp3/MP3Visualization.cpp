#include "MP3Visualization.h"

#undef e
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

Player::MP3Visualization::MP3Visualization()
    : VisualizationBase()
    , mAudioPlayer()
    , mMP3FileName{}
    , mPlayStatus(false)
    , mPauseStatus(false)
    , mVolumeLevel(255)
    , mVisualFrameStatus(true)
{
    // Read in the INI settings
    worldReadIniSettings();
}

Player::MP3Visualization::~MP3Visualization()
{
}

Player::MP3Visualization& Player::MP3Visualization::getInstance()
{
    static MP3Visualization instance;
    return instance;
}

void Player::MP3Visualization::initialize()
{
}

void Player::MP3Visualization::setMP3FileName(std::string& str)
{
    mMP3FileName = str;
}

std::string Player::MP3Visualization::getMP3FileName() 
{
    return mMP3FileName;
}

void Player::MP3Visualization::worldReadIniSettings()
{

}

void Player::MP3Visualization::worldInitFcn()
{   
    auto source = getMP3FileName();
    TCHAR* tFilename = new TCHAR[source.size() + 1];
    tFilename[source.size()] = 0;
    std::copy(source.begin(), source.end(), tFilename);
    mAudioPlayer.openFromFile(tFilename);
}

// World frame drawing functions
void Player::MP3Visualization::worldFramePreDisplayFcn(bool demoMode)
{
    // Create a new window for MP3 player
    ImGui::Begin("MP3 Player", &mVisualFrameStatus, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Do stuff */ }
            if (ImGui::MenuItem("Close", "Ctrl+W")) { mVisualFrameStatus = false; }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();

        ImGui::TextColored(ImVec4(UTILITYColors::White.r, UTILITYColors::White.g, UTILITYColors::White.b, 1.0F), "Day:");
        ImGui::SameLine(100);
        ImGui::TextColored(ImVec4{ UTILITYColors::Green.r, UTILITYColors::Green.g, UTILITYColors::Green.b, 1.0f }, getDayAndTime().c_str());

        if (mWorldFrameSettings.displayFile)
        {
            ImGui::Checkbox("play", &mWorldFrameSettings.play);
            if (mWorldFrameSettings.play && !mPlayStatus)
            {
                mAudioPlayer.play();
                mPlayStatus = true;
            }
            ImGui::SameLine(100);
            ImGui::Checkbox("pause", &mWorldFrameSettings.pause);
            ImGui::SameLine(200);
            ImGui::Checkbox("stop", &mWorldFrameSettings.stop);

            ImGui::SliderFloat("<< >>", &mWorldFrameSettings.fwdValue, 0.0f, mAudioPlayer.getDuration());
            ImGui::SliderFloat("volume", &mWorldFrameSettings.volume, 255.0F, 65535.0F);


            //AddCircle(const ImVec2 & center, float radius, ImU32 col, int num_segments = 0, float thickness = 1.0f);
            ImGui::GetWindowDrawList()->AddCircle(ImVec2{1.0f,3.5f}, 5.5F, 2, 25);

            // Need some wave form
            float samples[100];
            for (int n = 0; n < 100; n++)
                samples[n] = sinf(n * 0.2f + ImGui::GetTime() * 1.5f);
            ImGui::PlotLines("Samples", samples, 100); 

            // list all the .mp3 in the folder
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
            ImGui::BeginChild("Scrolling");
            for (int n = 0; n < 50; n++)
                ImGui::Text("%04d: Some text", n);
            ImGui::EndChild();

        }
    }

    ImGui::End();
}

void Player::MP3Visualization::localFrameDisplayFcn()
{
    if (mWorldFrameSettings.displayFile)
    {
        mWorldFrameSettings.fwdValue = static_cast<float>(mAudioPlayer.getPosition());

        mAudioPlayer.setVolume(static_cast<unsigned long>(mWorldFrameSettings.volume));

        if (mWorldFrameSettings.play && mWorldFrameSettings.pause && !mPauseStatus)
        {
            mAudioPlayer.setPause();
            mPauseStatus = true;
        }
        if (!mWorldFrameSettings.pause && mPauseStatus)
        {
            mAudioPlayer.unSetPause();
            mWorldFrameSettings.pause = false;
            mPauseStatus = false;
        }
        if (mWorldFrameSettings.play && mWorldFrameSettings.stop)
        {
            mAudioPlayer.close();
            mWorldFrameSettings.stop = false;
            mWorldFrameSettings.play = false;
            mWorldFrameSettings.pause = false;
        }
    }
}

std::string Player::MP3Visualization::getTime()
{
    auto        today = std::chrono::system_clock::now();
    auto        ptr   = std::chrono::system_clock::to_time_t(today);
    auto        tm    = localtime(&ptr);
    std::string strLocalTime =
        std::to_string(tm->tm_hour) + ":" + std::to_string(tm->tm_min) + ":" + std::to_string(tm->tm_sec);
    return strLocalTime;
}

std::string Player::MP3Visualization::getDayAndTime()
{
    auto        today = std::chrono::system_clock::now();
    auto        ptr   = std::chrono::system_clock::to_time_t(today);
    std::string strDayAndTime(ctime(&ptr));
    return strDayAndTime;
}
