/**
 ******************************************************************************
 * @addtogroup MasterApplication
 * @{
 * @file    MasterApplication
 * @author  Samuel Martel
 * @author  Lou-Gabriel Gaucher
 * @brief   Source for the MasterApplication module.
 *
 * @date 1/4/2021 17:00:00
 *
 ******************************************************************************
 */
#include "MasterApplication.h"

#include "Core/Inc/dma.h"
#include "Core/Inc/i2c.h"
#include "Core/Inc/i2s.h"
#include "Core/Inc/main.h"
#include "Core/Inc/spi.h"
#include "Core/Inc/usart.h"

#include "NilaiTFO/defines/macros.hpp"
#include "NilaiTFO/defines/pin.h"

#include "NilaiTFO/drivers/i2cModule.hpp"
#include "NilaiTFO/drivers/i2sModule.h"
#include "NilaiTFO/drivers/uartModule.hpp"
#include "NilaiTFO/interfaces/heartbeatModule.h"
#include "NilaiTFO/interfaces/TAS5707/Tas5707Module.h"
#include "NilaiTFO/services/filesystem.h"
#include "NilaiTFO/services/logger.hpp"



MasterApplication* MasterApplication::s_instance = nullptr;

MasterApplication::MasterApplication()
{
    CEP_ASSERT(s_instance == nullptr, "Cannot have multiple instances of Application!");
    s_instance = this;
}

void MasterApplication::Init()
{
    InitializeHal();
    InitializeModules();
}

bool MasterApplication::DoPost()
{
    if (m_logger == nullptr)
    {
        return false;
    }
    m_logger->Log("\n\r----- Started POST...\n\r");

    bool   allModulesPassedPost = true;
    size_t start                = HAL_GetTick();

    if (!cep::Filesystem::IsMounted())
    {
        m_logger->Log("POST Error: File system is not mounted!\n\r");
        allModulesPassedPost = false;
    }

    for (auto module = s_instance->m_modules.rbegin(); module != s_instance->m_modules.rend();
         module++)
    {
        if (!module->second->DoPost())
        {
            LOG_ERROR("%s POST failed!", module->second->GetLabel().c_str());
            allModulesPassedPost = false;
        }
    }

    uint32_t timeTaken = (HAL_GetTick() - start);

    if (allModulesPassedPost)
    {
        LOG_INFO("----- POST OK! %0.3f seconds.\n\r", (float)(timeTaken) / 1000.0f);
    }
    else
    {
        LOG_ERROR("----- POST ERROR! %0.3f seconds.\n\r", (float)(timeTaken) / 1000.0f);
    }

    return allModulesPassedPost;
}

[[noreturn]] void MasterApplication::Run()
{
    while (true)
    {
        for (auto& module : s_instance->m_modules)
        {
            module.second->Run();
        }
    }
}

cep::Module* MasterApplication::GetModule(const std::string& moduleName)
{
    CEP_ASSERT(s_instance->m_modules.find(moduleName) != s_instance->m_modules.end(),
               "Module does not exist!");
    return s_instance->m_modules.at(moduleName);
}

/*****************************************************************************/
/* Private Method Definitions                                                */
/*****************************************************************************/
void MasterApplication::InitializeHal()
{
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART2_UART_Init();
    MX_SPI1_Init();
    MX_I2C1_Init();
    MX_I2S3_Init();
}


void MasterApplication::InitializeModules()
{
    // --- Connectivity ---
    // UART CONFIG
    UartModule* uart2 = new UartModule(&huart2, "uart2");
    AddModule(uart2);
    m_logger = new Logger(uart2);
    Logger::Get()->Log("\n\n\r");
    Logger::Get()->Log(
      "================================================================================\n\r");
    Logger::Get()->Log("Application started.\n\r");

    // --- Drivers ---

    I2cModule* i2c = new I2cModule(&hi2c1, "i2c1");
    I2sModule* i2s = new I2sModule(&hi2s3, "i2s3");

    // --- Interfaces ---
    cep::Filesystem::Init();
    cep::Filesystem::Mount("", false);    // Mount the SD card, if one is found.

    cep::Tas5707::SoftwareConfig tasSwCfg {};
    tasSwCfg.Ch2Source = cep::Tas5707::Channel2InputSources::SDIN_L;
    cep::Tas5707::HardwareConfig tasHwCfg {};
    tasHwCfg.I2cHandle = i2c;
    tasHwCfg.I2sHandle = i2s;
    tasHwCfg.Reset     = {AUDIO_RESET_GPIO_Port, AUDIO_RESET_Pin};
    tasHwCfg.PwrDwn    = {AUDIO_PDN_GPIO_Port, AUDIO_PDN_Pin};
    tasHwCfg.Fault     = {AUDIO_BKND_ERROR_GPIO_Port, AUDIO_BKND_ERROR_Pin};
    tasHwCfg.PVddEn    = {AUDIO_PVDDEn_GPIO_Port, AUDIO_PVDDEn_Pin};

    AddModule(new Tas5707Module(tasHwCfg, tasSwCfg, "TAS5707"));

    // --- Processes ---
    AddModule(new HeartbeatModule({LED_GPIO_Port, LED_Pin}, "heartbeat"));

    LOG_INFO("Application Initialized!");
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == AUDIO_BKND_ERROR_Pin)
    {
        LOG_CRITICAL("[TAS5707]: Backend error detected!");
    }
}
