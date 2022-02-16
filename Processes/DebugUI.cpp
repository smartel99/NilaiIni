/**
 * @file    DebugUI.cpp
 * @author  Samuel Martel
 * @date    2022-02-14
 * @brief
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <a href=https://www.gnu.org/licenses/>https://www.gnu.org/licenses/<a/>.
 */
#include "DebugUI.h"

#include "MasterApplication.h"
#include "NilaiTFO/services/filesystem.h"

#define DEB_INFO(msg, ...) LOG_INFO("[DebugUI]: " msg, ##__VA_ARGS__)
#define PRINT(msg, ...)    Logger::Get()->Log(msg, ##__VA_ARGS__)
#define CLEAR              "\033[2J"

DebugUI::DebugUI(UartModule* uart)
: m_uart(uart),
  m_label("DebugUI"),
  m_handlers({{UiMenu::EnteringHome, [this] { EnterHome(); }},
              {UiMenu::Home, [this] { HandleHome(); }},
              {UiMenu::EnteringFE, [this] { EnterFE(); }},
              {UiMenu::FileExplorer, [this] { HandleFE(); }},
              {UiMenu::EnteringPlay, [this] { EnterPlayback(); }},
              {UiMenu::Playback, [this] { HandlePlayback(); }}})
{
    CEP_ASSERT(uart != nullptr, "Handle is null!");

    m_uart->SetExpectedRxLen(1);
    m_uart->ClearStartOfFrameSequence();
    m_uart->ClearEndOfFrameSequence();
    m_uart->SetFrameReceiveCpltCallback(
      [this]()
      {
          m_char    = m_uart->Receive().data[0];
          m_newChar = true;
      });

    DEB_INFO("Initialized");
}

bool DebugUI::DoPost()
{
    DEB_INFO("POST OK");
    return true;
}

void DebugUI::Run()
{
    //        if (m_newChar)
    //        {
    //            DEB_INFO("Received new char: (%#02x) %c", m_char, m_char);
    //            //            m_newChar = false;
    //        }
    m_handlers[m_currentMenu]();
}

void DebugUI::EnterHome()
{
    PRINT(
      "%s"
      "================================================================================\n\r"
      "| Neurospa V5 Debug interface\n\r"
      "| Use W (up) and S (down) to navigate, enter to select, escape to return.\n\r"
      "| Volume: %#02x (+/- to control)\n\r"
      " -> Play file\n\r",
      CLEAR,
      m_volume);

    m_currentMenu = UiMenu::Home;
}

void DebugUI::HandleHome()
{
    // The arrow keys don't do anything, just handle ENTER.
    if (m_newChar)
    {
        m_newChar = false;
        // Enter key pressed.
        if (m_char == 0x0D)
        {
            m_currentMenu = UiMenu::EnteringFE;
        }
        else
        {
            HandleVolume();
        }
    }
}

void DebugUI::EnterFE()
{
    m_cursorPos  = 0;
    m_currentDir = "/";

    m_currentMenu = UiMenu::FileExplorer;

    OpenDir(m_currentDir);

    DisplayCurrentDir();
}

void DebugUI::HandleFE()
{
    if (m_newChar)
    {
        m_newChar = false;
        if (m_char == 0x0D)
        {
            // Enter key pressed -> Play track/enter dir.
            if (m_files[m_cursorPos].IsDir)
            {
                m_currentDir += m_files[m_cursorPos].Name + "/";
                OpenDir(m_currentDir);
            }
            else
            {
                m_currentMenu = UiMenu::EnteringPlay;
                return;
            }
        }
        else if (m_char == 'W' || m_char == 'w')
        {
            // Up.
            if (m_cursorPos == 0)
            {
                m_cursorPos = m_files.size() - 1;
            }
            else
            {
                --m_cursorPos;
            }
        }
        else if (m_char == 'S' || m_char == 's')
        {
            // Down.
            if (m_cursorPos == m_files.size() - 1)
            {
                m_cursorPos = 0;
            }
            else
            {
                ++m_cursorPos;
            }
        }
        else if (m_char == 0x1B)
        {
            // Escape key, go up one dir or go back to the main menu.
            if (m_currentDir == "/")
            {
                m_currentMenu = UiMenu::EnteringHome;
            }
            else
            {
                m_currentDir = m_currentDir.substr(
                  0, m_currentDir.find_last_of('/', m_currentDir.size() - 2) + 1);
                OpenDir(m_currentDir);
            }
        }
        else
        {
            HandleVolume();
        }

        DisplayCurrentDir();
    }
}
void DebugUI::DisplayCurrentDir()
{
    PRINT(
      "%s"
      "================================================================================\n\r"
      "| Volume: %#02x (+/- to control)\n\r"
      "| Select the file to play:\n\r"
      "| Current directory: %s\n\r",
      CLEAR,
      m_volume,
      m_currentDir.c_str());

    for (size_t i = 0; i < m_files.size(); i++)
    {
        PRINT("%s%s\t%s\n\r",
              (m_cursorPos == i) ? " ->" : "   ",
              m_files[i].Name.c_str(),
              (m_files[i].IsDir ? "(DIR)" : ""));
    }
}

void DebugUI::DisplayCurrentlyPlaying()
{
    static WavPlayer* player = WAV_PLAYER;
    PRINT(
      "%s"
      "\r================================================================================\n\r"
      "| Volume: %#02x (+/- to control)\n\r"
      "| Currently playing: %s\n\r"
      "| %s %s\n\r"
      "| %s Stop\n\r"
      "| %0.3f/%0.3f\n\r",
      CLEAR,
      m_volume,
      m_currentlyPlaying.c_str(),
      (m_cursorPos == 0 ? " ->" : "   "),
      (m_isPlaying ? "Pause" : "Play"),
      (m_cursorPos == 1 ? " ->" : "   "),
      player->GetCurrentTime(),
      player->GetTotTime());
}

void DebugUI::EnterPlayback()
{
    m_currentlyPlaying = m_currentDir + m_files[m_cursorPos].Name;

    if (WAV_PLAYER->PlayFile(m_currentlyPlaying))
    {
        m_currentMenu = UiMenu::Playback;
        DisplayCurrentlyPlaying();
    }
    else
    {
        // Failed to start playback, go back.
        m_currentMenu = UiMenu::EnteringFE;
    }
}

void DebugUI::HandlePlayback()
{
    static WavPlayer* player = WAV_PLAYER;
    if (m_newChar)
    {
        m_newChar = false;
        if (m_char == 0x0D)
        {
            // Enter key pressed -> Activate selection.
            // Pos 0 -> Play/pause
            // Pos 1 -> Stop
            if (m_cursorPos == 0)
            {
                if (player->IsPaused())
                {
                    player->Resume();
                }
                else
                {
                    player->Pause();
                }
            }
            else
            {
                player->Stop();
            }
        }
        else if (m_char == 'W' || m_char == 'w' || m_char == 'S' || m_char == 's')
        {
            // Up.
            m_cursorPos = (uint8_t)(!(bool)m_cursorPos);
        }
        else if (m_char == 0x1B)
        {
            // Escape key, stop and go back.
            player->Stop();
            m_currentMenu = UiMenu::EnteringFE;
        }
        else
        {
            HandleVolume();
        }

        DisplayCurrentlyPlaying();
    }

    // Update once every 100ms.
    if (HAL_GetTick() % 1000 == 0)
    {
        DisplayCurrentlyPlaying();
        if (!player->IsPlaying())
        {
            m_currentMenu = UiMenu::EnteringFE;
        }
    }
}

void DebugUI::OpenDir(const std::string& dir)
{
    using namespace cep;
    Filesystem::Result     r;
    Filesystem::dir_t      dirHandle;
    Filesystem::fileInfo_t fInfo;

    m_files.clear();
    r = Filesystem::OpenDir(dir, &dirHandle);
    if (r == Filesystem::Result::Ok)
    {
        do
        {
            r = Filesystem::ReadDir(&dirHandle, &fInfo);
            if (r == Filesystem::Result::Ok && fInfo.fname[0] != 0)
            {
                m_files.emplace_back(std::string {fInfo.fname}, bool(fInfo.fattrib & AM_DIR));
            }
        } while (r == Filesystem::Result::Ok && fInfo.fname[0] != 0);

        Filesystem::CloseDir(&dirHandle);
    }
}

void DebugUI::HandleVolume()
{
    static Tas5707Module* amp = TAS_MODULE;

    if (m_char == '+')
    {
        if (m_volume < s_maxVol)
        {
            ++m_volume;
            amp->SetMasterVolume(m_volume);
        }
    }
    else if (m_char == '-')
    {
        if (m_volume > s_minVol)
        {
            --m_volume;
            amp->SetMasterVolume(m_volume);
        }
    }
    else if ((m_char >= '0') && (m_char <= '9'))
    {
        uint8_t num = m_char - '0';
        // Scale the 0-9 range into 0-s_maxVol range.
        m_volume = (num * s_maxVol) / 10;
        amp->SetMasterVolume(m_volume);
    }
}
