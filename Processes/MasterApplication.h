/**
 ******************************************************************************
 * @addtogroup MasterApplication
 * @{
 * @file    MasterApplication
 * @author  Samuel Martel
 * @author  Lou-Gabriel Gaucher
 * @brief   Header for the MasterApplication module.
 *
 * @date 1/4/2021 17:00:00
 *
 ******************************************************************************
 */
#ifndef SWIDGET_MOTHERBOARD_MASTERAPPLICATION_H
#    define SWIDGET_MOTHERBOARD_MASTERAPPLICATION_H

/*****************************************************************************/
/* Includes */
#    include "NilaiTFO/defines/module.hpp"
#    include "NilaiTFO//processes/application.hpp"

#    include "NilaiTFO/drivers/uartModule.hpp"

#    include "NilaiTFO/services/logger.hpp"
#    include "NilaiTFO/services/umoModule.h"


#    include <map>

/*****************************************************************************/
/* Exported defines */

/*****************************************************************************/
/* Exported macro */
// Explanation of static_cast: https://stackoverflow.com/a/1255015/11443498

// DRIVERS
#    define UART2_MODULE (static_cast<UartModule*>(MasterApplication::GetModule("uart2")))

// SERVICES

// INTERFACES

// PROCESSES

/*****************************************************************************/
/* Exported types */
class MasterApplication : public cep::Application
{
public:
    MasterApplication();
    ~MasterApplication() override = default;

    bool              DoPost() override;
    void              Init() override;
    [[noreturn]] void Run() override;

    void AddModule(cep::Module* newModule) { m_modules[newModule->GetLabel()] = newModule; }

    static cep::Module* GetModule(const std::string& moduleName);

    static MasterApplication* Get() { return s_instance; }

private:
    std::map<std::string, cep::Module*> m_modules;
    Logger*                             m_logger = nullptr;

private:
    static MasterApplication* s_instance;

private:
    void InitializeHal();
    void InitializeModules();
    void CheckParser();
};
/*****************************************************************************/
/* Exported functions */

/* Have a wonderful day :) */
#endif /* SWIDGET_MOTHERBOARD_MASTERAPPLICATION_H */
       /**
        * @}
        */
       /****** END OF FILE ******/
