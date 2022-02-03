/**
 * @file    main.cpp
 * @author  Samuel Martel
 * @date    2022/01/18
 * @brief   Example program to show how to use GPIOs as inputs and outputs using the GD32F407R-START
 * starter kit.
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

#include "main.h"
#include "gd32f4xx.h"
#include "gd32f4xx_gpio.h"
#include "gd32f4xx_rcu.h"
#include "sys/systick.h"


static void InitClocks();
static void InitGpio();

int main()
{
    systick_config();

    InitClocks();
    InitGpio();

    volatile bool t = true;
    while (t)
    {
        FlagStatus s = gpio_input_bit_get(BUTTON_PORT, BUTTON_PIN);
        gpio_bit_write(LED_PORT, LED_PIN, s);
    }
}

void InitClocks()
{
    // LED is on port C.
    rcu_periph_clock_enable(RCU_GPIOC);

    // Button is on port A.
    rcu_periph_clock_enable(RCU_GPIOA);
}

void InitGpio()
{
    gpio_deinit(LED_PORT);
    gpio_deinit(BUTTON_PORT);

    gpio_mode_set(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
    gpio_output_options_set(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED_PIN);

    gpio_mode_set(BUTTON_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, BUTTON_PIN);
}
