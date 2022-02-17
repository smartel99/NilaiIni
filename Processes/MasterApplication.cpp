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
#include "NilaiTFO/drivers/uartModule.hpp"
#include "NilaiTFO/interfaces/heartbeatModule.h"
#include "NilaiTFO/services/filesystem.h"
#include "NilaiTFO/services/logger.hpp"

#include "NilaiTFO/services/IniParser.h"


#define HAS_SECTION(section)         (ini.HasSection(section) ? "true" : "false")
#define HAS_VALUE_STR(section, name) section, name, ini.HasValue(section, name) ? "true" : "false"

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

    CheckParser();
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

    // --- Interfaces ---
    cep::Filesystem::Init();
    cep::Filesystem::Mount("", true);    // Mount the SD card, if one is found.

    // --- Processes ---
    AddModule(new HeartbeatModule({LED_GPIO_Port, LED_Pin}, "heartbeat"));


    LOG_INFO("Application Initialized!");
}
void MasterApplication::CheckParser()
{
    cep::IniParser ini("cfg.ini");

    if (ini.GetError() != 0)
    {
        LOG_ERROR("ini failed to be parsed: %i", ini.GetError());
        return;
    }

    LOG_DEBUG("HasSection:");
    LOG_DEBUG("Has section 1: %s", HAS_SECTION("section 1"));
    LOG_DEBUG("Has section 2: %s", HAS_SECTION("section 2"));
    LOG_DEBUG("Has section 3: %s", HAS_SECTION("section 3"));
    LOG_DEBUG("Has section 4: %s", HAS_SECTION("section 4"));

    LOG_DEBUG("\n\rHasValue:");
    LOG_DEBUG("Has %s - %s: %s", HAS_VALUE_STR("section 1", "s1"));
    LOG_DEBUG("Has %s - %s: %s", HAS_VALUE_STR("section 1", "s2"));
    LOG_DEBUG("Has %s - %s: %s", HAS_VALUE_STR("section 2", "i1"));
    LOG_DEBUG("Has %s - %s: %s", HAS_VALUE_STR("section 2", "i2"));
    LOG_DEBUG("Has %s - %s: %s", HAS_VALUE_STR("section 2", "i3"));
    LOG_DEBUG("Has %s - %s: %s", HAS_VALUE_STR("section 3", "f1"));
    LOG_DEBUG("Has %s - %s: %s", HAS_VALUE_STR("section 3", "f2"));
    LOG_DEBUG("Has %s - %s: %s", HAS_VALUE_STR("section 4", "b1"));
    LOG_DEBUG("Has %s - %s: %s", HAS_VALUE_STR("section 4", "b2"));

    LOG_DEBUG("\n\rGetString:");
    LOG_DEBUG("s1: %s", ini.GetString("section 1", "s1").c_str());
    LOG_DEBUG("s2: %s", ini.GetString("section 1", "s2").c_str());

    LOG_DEBUG("\n\rGetInteger:");
    LOG_DEBUG("i1: %d", ini.GetInteger("section 2", "i1"));
    LOG_DEBUG("i2: %d", ini.GetInteger("section 2", "i2"));
    LOG_DEBUG("i3: %d", ini.GetInteger("section 2", "i3"));

    LOG_DEBUG("\n\rGetDecimal:");
    LOG_DEBUG("f1: %0.4f", ini.GetDecimal("section 3", "f1"));
    LOG_DEBUG("f2: %0.4f", ini.GetDecimal("section 3", "f2"));

    LOG_DEBUG("\n\rGetBoolean:");
    LOG_DEBUG("b1: %s", ini.GetBoolean("section 4", "b1") ? "true" : "false");
    LOG_DEBUG("b2: %s", ini.GetBoolean("section 4", "b2") ? "true" : "false");

    LOG_DEBUG("\n\rIterators:");
    for (auto& [k, v] : ini)
    {
        LOG_DEBUG("%s = %s", k.c_str(), v.c_str());
    }
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == AUDIO_BKND_ERROR_Pin)
    {
        LOG_CRITICAL("[TAS5707]: Backend error detected!");
    }
}
