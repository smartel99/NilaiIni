/**
 * @file    DebugUI.h
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
#ifndef DERISKING_SD_DAC_I2S_DEBUGUI_H
#define DERISKING_SD_DAC_I2S_DEBUGUI_H

#include "NilaiTFO/defines/module.hpp"
#include "NilaiTFO/drivers/uartModule.hpp"

#include <map>

enum class UiMenu
{
    EnteringHome = 0,
    Home,
    EnteringFE,
    FileExplorer,
    EnteringPlay,
    Playback
};

class DebugUI : public cep::Module
{
public:
    DebugUI(UartModule* uart);
    ~DebugUI() override = default;

    bool                             DoPost() override;
    void                             Run() override;
    [[nodiscard]] const std::string& GetLabel() const override { return m_label; }

private:
    void EnterHome();
    void HandleHome();

    void EnterFE();
    void HandleFE();

    void EnterPlayback();
    void HandlePlayback();

    void HandleVolume();

    void OpenDir(const std::string& dir);
    void DisplayCurrentDir();
    void DisplayCurrentlyPlaying();

    UartModule* m_uart = nullptr;
    std::string m_label;

    UiMenu m_currentMenu = UiMenu::EnteringHome;

    bool    m_newChar = true;
    uint8_t m_char    = 0;

    std::string m_currentDir;
    std::string m_currentlyPlaying;
    uint8_t     m_cursorPos = 0;

    static constexpr uint8_t s_maxVol    = 0xCE;
    static constexpr uint8_t s_minVol    = 0x00;
    static constexpr uint8_t s_defVol    = 0x9E;
    uint8_t                  m_volume    = s_defVol;
    bool                     m_isPlaying = false;

    std::map<UiMenu, std::function<void()>> m_handlers;

    struct File
    {
        std::string Name;
        bool        IsDir = false;

        File(std::string name, bool isDir) : Name(std::move(name)), IsDir(isDir) {}
    };
    std::vector<File> m_files;
};



#endif    // DERISKING_SD_DAC_I2S_DEBUGUI_H
