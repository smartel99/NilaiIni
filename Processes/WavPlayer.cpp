/**
 * @file    WavPlayer.cpp
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
#include "WavPlayer.h"

#define WAV_DEBUG(msg, ...) LOG_DEBUG("[WavPlayer]: " msg, ##__VA_ARGS__)
#define WAV_INFO(msg, ...)  LOG_INFO("[WavPlayer]: " msg, ##__VA_ARGS__)
#define WAV_ERROR(msg, ...) LOG_ERROR("[WavPlayer]: " msg, ##__VA_ARGS__)

#define FOUR_CHARS(a) a[0], a[1], a[2], a[3]

WavPlayer::WavPlayer(Tas5707Module* handler) : m_handler(handler), m_label("WavPlayer")
{
    CEP_ASSERT(handler != nullptr, "Handler is null!");

    // Hook the callbacks up.
    m_handler->SetTxHalfCpltCb(
      [this]()
      {
          LED_GPIO_Port->BSRR = LED_Pin;    // Set the led.
          m_bufferState       = BufferState::FirstHalfEmpty;
      });
    m_handler->SetTxCpltCb(
      [this]()
      {
          LED_GPIO_Port->BSRR = (uint32_t)LED_Pin << 16U;    // Reset the led.
          m_bufferState       = BufferState::SecondHalfEmpty;
          m_handler->Restream(reinterpret_cast<const uint16_t*>(m_buff), s_buffSize >> 1);
      });

    WAV_INFO("Initialized");
}

bool WavPlayer::DoPost()
{
    WAV_INFO("POST OK");

    return true;
}

void WavPlayer::Run()
{
    if (m_bufferState == BufferState::Handled)
    {
        return;
    }

    if (m_bytesLeft == 0)
    {
        // No more data to be read.
        Stop();
        return;
    }

    size_t br = 0;

    // Re-fill half of the buffer.
    size_t offset        = (m_bufferState == BufferState::FirstHalfEmpty) ? 0 : s_buffSize >> 1;
    LED2_GPIO_Port->BSRR = LED2_Pin;    // Set the led.
    m_currentFile.Read(&m_buff[offset], s_buffSize >> 1, &br);
    LED2_GPIO_Port->BSRR = (uint32_t)LED2_Pin << 16U;    // Reset the led.


    m_bufferState = BufferState::Handled;

    if (m_bytesLeft > (s_buffSize >> 1))
    {
        m_bytesLeft -= br;
    }
    else
    {
        m_bytesLeft = 0;
    }
}

bool WavPlayer::PlayFile(std::string f)
{
    if (m_isPlaying)
    {
        // Already a song playing.
        return false;
    }

    m_currentFilePath = std::move(f);
    m_currentFile     = cep::Filesystem::File(m_currentFilePath, cep::Filesystem::FileModes::Read);
    if (!m_currentFile.IsOpen())
    {
        WAV_ERROR("Unable to open '%s'.", m_currentFilePath.c_str());
        return false;
    }

    // Read the wav header.
    size_t br = 0;
    m_currentFile.Read(&m_fmt, sizeof(WavFmt), &br);
    if (br != sizeof(WavFmt))
    {
        WAV_ERROR("Didn't read as many bytes as were expected: %d/%d", br, sizeof(WavFmt));
        return false;
    }


    // Print header.
    WAV_DEBUG(
      "Header of %s:"
      "\n\r\tRiff: %c%c%c%c"
      "\n\r\tFileSize: %d"
      "\n\r\tFileFormat: %c%c%c%c"
      "\n\r\tFmtHeader: %c%c%c%c"
      "\n\r\tFmtSize: %d"
      "\n\r\tAudioFormat: %d"
      "\n\r\tNbrChannels: %d"
      "\n\r\tSampleRate: %d"
      "\n\r\tByteRate: %d"
      "\n\r\tBlockAlign: %d"
      "\n\r\tBitPerSample: %d"
      "\n\r\tDataHeader: %c%c%c%c"
      "\n\r\tDataSize: %d",
      m_currentFilePath.c_str(),
      FOUR_CHARS(m_fmt.Riff),
      m_fmt.FileSize,
      FOUR_CHARS(m_fmt.FileFormat),
      FOUR_CHARS(m_fmt.FmtHeader),
      m_fmt.FmtSize,
      m_fmt.AudioFormat,
      m_fmt.NbrChannels,
      m_fmt.SampleRate,
      m_fmt.ByteRate,
      m_fmt.BlockAlign,
      m_fmt.BitPerSample,
      FOUR_CHARS(m_fmt.DataHeader),
      m_fmt.DataSize);

    m_bytesLeft = m_fmt.FileSize;
    m_bytesLeft -= sizeof(WavFmt);    // Remove header from total bytes left.

    // Start to read the file.
    br = 0;
    m_currentFile.Read(m_buff, s_buffSize, &br);
    m_bytesLeft -= br;

    // Needs the number of samples, not the number of bytes.
    if (m_handler->Stream(reinterpret_cast<const uint16_t*>(m_buff), s_buffSize / 2))
    {
        WAV_DEBUG("Started playing!");
        m_isPlaying = true;
    }

    return m_isPlaying;
}

bool WavPlayer::Pause()
{
    return false;
}

bool WavPlayer::Resume()
{
    return false;
}

bool WavPlayer::Stop()
{
    if (!m_isPlaying)
    {
        return false;
    }
    m_handler->StopStream();
    m_isPlaying = false;
    WAV_INFO("Finished playing %s", m_currentFilePath.c_str());
    m_currentFile.Close();

    return true;
}

float WavPlayer::GetCurrentTime() const
{
    // CurrentTime = (TotalBytes - BytesLeft) / BytesPerSeconds
    return (float)(m_fmt.FileSize - m_bytesLeft) / (float)m_fmt.ByteRate;
}

float WavPlayer::GetTotTime() const
{
    // Time = TotalBytes / BytesPerSeconds
    return (float)m_fmt.FileSize / (float)m_fmt.ByteRate;
}
