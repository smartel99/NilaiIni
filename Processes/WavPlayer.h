/**
 * @file    WavPlayer.h
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
#ifndef DERISKING_SD_DAC_I2S_WAVPLAYER_H
#define DERISKING_SD_DAC_I2S_WAVPLAYER_H

#include "NilaiTFO/defines/module.hpp"
#include "NilaiTFO/interfaces/TAS5707/Tas5707Module.h"
#include "NilaiTFO/services/filesystem.h"

struct WavFmt
{
    char     Riff[4]       = {0};           /* 0 */
    uint32_t FileSize      = 0;             /* 4 */
    char     FileFormat[4] = {0};           /* 8 */
    char     FmtHeader[4]  = {0};           /* 12 */
    uint32_t FmtSize       = 0;             /* 16*/
    uint16_t AudioFormat   = 0; /* 20 */    // 1 -> PCM.
    uint16_t NbrChannels   = 0;             /* 22 */
    uint32_t SampleRate    = 0;             /* 24 */

    uint32_t ByteRate      = 0;   /* 28 */
    uint16_t BlockAlign    = 0;   /* 32 */
    uint16_t BitPerSample  = 0;   /* 34 */
    char     DataHeader[4] = {0}; /* 36 */
    uint32_t DataSize      = 0;   /* 40 */
};

enum class BufferState
{
    Handled = 0,
    FirstHalfEmpty,
    SecondHalfEmpty
};

class WavPlayer : public cep::Module
{
public:
    WavPlayer(Tas5707Module* handler);
    ~WavPlayer() override = default;

    bool                             DoPost() override;
    void                             Run() override;
    [[nodiscard]] const std::string& GetLabel() const override { return m_label; };

    [[nodiscard]] bool  IsPlaying() const { return m_isPlaying; }
    [[nodiscard]] bool  IsPaused() const { return m_isPaused; }
    [[nodiscard]] float GetCurrentTime() const;
    [[nodiscard]] float GetTotTime() const;

    bool PlayFile(std::string f);
    bool Pause();
    bool Resume();
    bool Stop();

    [[nodiscard]] const WavFmt& GetWavFileHeader() const { return m_fmt; }

private:
    I2sModule*  m_handler = nullptr;
    std::string m_label;

    bool                  m_isPlaying = false;
    bool                  m_isPaused  = false;
    std::string           m_currentFilePath;
    cep::Filesystem::File m_currentFile;
    WavFmt                m_fmt = {};

    BufferState             m_bufferState = BufferState::Handled;
    size_t                  m_bytesLeft   = 0;
    static constexpr size_t s_buffSize    = 8192;
    uint8_t                 m_buff[s_buffSize];
};



#endif    // DERISKING_SD_DAC_I2S_WAVPLAYER_H
