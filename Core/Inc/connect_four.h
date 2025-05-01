#ifndef CONNECT_FOUR_H
#define CONNECT_FOUR_H

#include "stm32f4xx_hal.h"
#include "LCD_Driver.h"
#include <stdbool.h>

#define BOARD_ROWS 6
#define BOARD_COLS 7
#define CELL_SIZE 30
#define BOARD_OFFSET_X 30
#define BOARD_OFFSET_Y 50
#define COIN_RADIUS 12

typedef enum {
    STATE_MENU,
    STATE_GAME_1P,
    STATE_GAME_2P,
    STATE_GAME_OVER
} GameState;

typedef enum {
    PLAYER_NONE = 0,
    PLAYER_1 = 1,
    PLAYER_2 = 2
} Player;

typedef struct {
    uint8_t cells[BOARD_ROWS][BOARD_COLS]; // 0: empty, 1: player 1, 2: player 2
    Player currentPlayer;
    GameState state;
    uint8_t selectedColumn;
    bool gameOver;
    Player winner;
    uint32_t player1Score;
    uint32_t player2Score;
    uint32_t gameStartTime;
    uint32_t gameDuration;
} ConnectFourGame;

void ConnectFour_Init(ConnectFourGame *game);
void ConnectFour_DrawMenu(ConnectFourGame *game);
void ConnectFour_DrawBoard(ConnectFourGame *game);
void ConnectFour_DrawCoin(uint16_t x, uint16_t y, Player player);
void ConnectFour_UpdateSelectedColumn(ConnectFourGame *game, uint16_t touchX);
void ConnectFour_DropCoin(ConnectFourGame *game);
bool ConnectFour_CheckWin(ConnectFourGame *game, uint8_t row, uint8_t col);
bool ConnectFour_IsBoardFull(ConnectFourGame *game);
void ConnectFour_ComputerMove(ConnectFourGame *game, RNG_HandleTypeDef *hrng);
void ConnectFour_DrawFinalScreen(ConnectFourGame *game);
void ConnectFour_StartNewGame(ConnectFourGame *game, GameState newState);
void ConnectFour_ProcessTouch(ConnectFourGame *game, uint16_t touchX, uint16_t touchY, RNG_HandleTypeDef *hrng);

#endif