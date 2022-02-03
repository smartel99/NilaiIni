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
#include "Core/Inc/main.h"
#include "Core/Inc/usart.h"

#include "NilaiTFO/defines/macros.hpp"
#include "NilaiTFO/defines/pin.h"

#include "NilaiTFO/drivers/uartModule.hpp"
#include "NilaiTFO/interfaces/heartbeatModule.h"
#include "NilaiTFO/services/logger.hpp"
#include "NilaiTFO/services/SystemModule.h"
#include "NilaiTFO/services/umoModule.h"



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
      "================================================================================");
    Logger::Get()->Log("Application started.");

    AddModule(new HeartbeatModule({LED_GPIO_Port, LED_Pin}, "heartbeat"));

    // --- Processes ---

    LOG_INFO("Application Initialized!");
}
