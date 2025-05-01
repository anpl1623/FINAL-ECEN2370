/*
 * ApplicationCode.c
 *
 *  Created on: Dec 30, 2023 (updated 11/12/2024) Thanks Donavon! 
 *      Author: Xavion
 */

#include "ApplicationCode.h"
#include "connect_four.h"
#include "main.h"
#include <stdio.h>

/* Static variables */


extern void initialise_monitor_handles(void);

#if COMPILE_TOUCH_FUNCTIONS == 1
static STMPE811_TouchData StaticTouchData;
#endif // COMPILE_TOUCH_FUNCTIONS

static ConnectFourGame game;
extern RNG_HandleTypeDef hrng;


void ApplicationInit(void) 
{
    initialise_monitor_handles(); // Allows printf functionality
    LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0, LCD_COLOR_WHITE);

#if COMPILE_TOUCH_FUNCTIONS == 1
    InitializeLCDTouch();

    // This is the orientation for the board to be directly up where the buttons are vertically above the screen
    // Top left would be low x value, high y value. Bottom right would be low x value, low y value.
    StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;

#endif // COMPILE_TOUCH_FUNCTIONS

    ConnectFour_Init(&game);
}

void LCD_Visual_Demo(void) {
    GameLoop();
}

void GameLoop(void) {
    GPIO_PinState lastButtonState = GPIO_PIN_RESET;
    
    while (1) {
#if COMPILE_TOUCH_FUNCTIONS == 1
		/* If touch pressed */
        if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
			/* Touch valid */
            ConnectFour_ProcessTouch(&game, StaticTouchData.x, StaticTouchData.y, &hrng);
            
            while (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
                HAL_Delay(10);
            }
        }
#endif // COMPILE_TOUCH_FUNCTIONS
        GPIO_PinState buttonState = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
        
        // button press (falling edge detection)
        if (buttonState == GPIO_PIN_SET && lastButtonState == GPIO_PIN_RESET) {
            if ((game.state == STATE_GAME_1P || game.state == STATE_GAME_2P) && !game.gameOver) {
                ConnectFour_DropCoin(&game);
                if (game.state == STATE_GAME_1P && 
                    game.currentPlayer == PLAYER_2 && 
                    !game.gameOver) {
                    HAL_Delay(500);
                    ConnectFour_ComputerMove(&game, &hrng);
                }
            }
        }
        
        lastButtonState = buttonState;
        
        HAL_Delay(10);
    }
}

#if COMPILE_TOUCH_FUNCTIONS == 1
void LCD_Touch_Polling_Demo(void) {
    GameLoop();
}
#endif // COMPILE_TOUCH_FUNCTIONS
