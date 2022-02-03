/**
* @file    main.h
* @author  Samuel Martel
* @date    2022/01/18
* @brief   Header containing the general definitions for the GD32F407R-START board.
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

#ifndef GD32F407_SIMPLEPROGRAM_MAIN_H
#define GD32F407_SIMPLEPROGRAM_MAIN_H

#define LED_PORT GPIOC
#define LED_PIN  BIT(6)

#define BUTTON_PORT GPIOA
#define BUTTON_PIN  BIT(0)

#endif /* GD32F407_SIMPLEPROGRAM_MAIN_H */
