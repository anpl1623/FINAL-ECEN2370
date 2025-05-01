#include "connect_four.h"
#include "LCD_Driver.h"
void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color);

#include <stdio.h>
#include <string.h>

#define COLOR_BOARD      LCD_COLOR_BLUE
#define COLOR_PLAYER1    LCD_COLOR_RED
#define COLOR_PLAYER2    LCD_COLOR_YELLOW
#define COLOR_EMPTY      LCD_COLOR_BLACK
#define COLOR_BACKGROUND LCD_COLOR_WHITE
#define COLOR_TEXT       LCD_COLOR_BLACK

extern FONT_t Font16x24;
extern FONT_t Font12x12;

void ConnectFour_Init(ConnectFourGame *game) {
    memset(game->cells, PLAYER_NONE, sizeof(game->cells));
    
    game->state = STATE_MENU;
    game->currentPlayer = PLAYER_1;
    game->selectedColumn = 3;
    game->gameOver = false;
    game->winner = PLAYER_NONE;
    game->player1Score = 0;
    game->player2Score = 0;
    game->gameStartTime = 0;
    game->gameDuration = 0;

    ConnectFour_DrawMenu(game);
}

void ConnectFour_DrawMenu(ConnectFourGame *game) {
    LCD_Clear(0, COLOR_BACKGROUND);
    
    LCD_SetFont(&Font16x24);
    LCD_SetTextColor(COLOR_TEXT);
    
    char title[] = "Connect Four";
    uint16_t titleX = (LCD_PIXEL_WIDTH - strlen(title) * 16) / 2;
    uint16_t y = 50;
    
    for (int i = 0; i < strlen(title); i++) {
        LCD_DisplayChar(titleX + i * 16, y, title[i]);
    }
    
    LCD_Draw_Circle_Fill(LCD_PIXEL_WIDTH / 2, 120, 40, COLOR_BOARD);
    LCD_SetTextColor(COLOR_BACKGROUND);
    
    char onePlayer[] = "1v1";
    uint16_t onePlayerX = LCD_PIXEL_WIDTH / 2 - 16;
    
    for (int i = 0; i < strlen(onePlayer); i++) {
        LCD_DisplayChar(onePlayerX + i * 16, 115, onePlayer[i]);
    }
    
    LCD_Draw_Circle_Fill(LCD_PIXEL_WIDTH / 2, 220, 40, COLOR_BOARD);
    
    char twoPlayer[] = "CPU";
    uint16_t twoPlayerX = LCD_PIXEL_WIDTH / 2 - 16;
    
    for (int i = 0; i < strlen(twoPlayer); i++) {
        LCD_DisplayChar(twoPlayerX + i * 16, 215, twoPlayer[i]);
    }
}

void ConnectFour_DrawBoard(ConnectFourGame *game) {
    LCD_Clear(0, COLOR_BACKGROUND);
    
    uint16_t boardWidth = BOARD_COLS * CELL_SIZE;
    uint16_t boardHeight = BOARD_ROWS * CELL_SIZE;
    
    for (uint16_t y = BOARD_OFFSET_Y; y < BOARD_OFFSET_Y + boardHeight; y++) {
        for (uint16_t x = BOARD_OFFSET_X; x < BOARD_OFFSET_X + boardWidth; x++) {
            LCD_Draw_Pixel(x, y, COLOR_BOARD);
        }
    }
    
    for (uint8_t row = 0; row < BOARD_ROWS; row++) {
        for (uint8_t col = 0; col < BOARD_COLS; col++) {
            uint16_t centerX = BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
            uint16_t centerY = BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;
            LCD_Draw_Circle_Fill(centerX, centerY, COIN_RADIUS, COLOR_EMPTY);
        }
    }
    
    for (uint8_t row = 0; row < BOARD_ROWS; row++) {
        for (uint8_t col = 0; col < BOARD_COLS; col++) {
            if (game->cells[row][col] != PLAYER_NONE) {
                uint16_t centerX = BOARD_OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
                uint16_t centerY = BOARD_OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;
                ConnectFour_DrawCoin(centerX, centerY, game->cells[row][col]);
            }
        }
    }
    
    uint16_t coinX = BOARD_OFFSET_X + game->selectedColumn * CELL_SIZE + CELL_SIZE / 2;
    uint16_t coinY = BOARD_OFFSET_Y - CELL_SIZE / 2;
    ConnectFour_DrawCoin(coinX, coinY, game->currentPlayer);
}

void ConnectFour_DrawCoin(uint16_t x, uint16_t y, Player player) {
    uint16_t color = (player == PLAYER_1) ? COLOR_PLAYER1 : COLOR_PLAYER2;
    LCD_Draw_Circle_Fill(x, y, COIN_RADIUS, color);
}

void ConnectFour_UpdateSelectedColumn(ConnectFourGame *game, uint16_t touchX) {
    int16_t column = (touchX - BOARD_OFFSET_X) / CELL_SIZE;
    
    if (column >= 0 && column < BOARD_COLS) {
        game->selectedColumn = column;
    }
    
    ConnectFour_DrawBoard(game);
}

void ConnectFour_DropCoin(ConnectFourGame *game) {
    if (game->cells[0][game->selectedColumn] != PLAYER_NONE) {
        return;
    }
    
    uint8_t row;
    for (row = BOARD_ROWS - 1; row > 0; row--) {
        if (game->cells[row][game->selectedColumn] == PLAYER_NONE) {
            break;
        }
    }
    
    game->cells[row][game->selectedColumn] = game->currentPlayer;
    
    ConnectFour_DrawBoard(game);
    
    if (ConnectFour_CheckWin(game, row, game->selectedColumn)) {
        game->gameOver = true;
        game->winner = game->currentPlayer;
        
        if (game->currentPlayer == PLAYER_1) {
            game->player1Score++;
        } else {
            game->player2Score++;
        }
        
        game->gameDuration = HAL_GetTick() - game->gameStartTime;
        
        game->state = STATE_GAME_OVER;
        
        ConnectFour_DrawFinalScreen(game);
        return;
    }
    
    if (ConnectFour_IsBoardFull(game)) {
        game->gameOver = true;
        game->winner = PLAYER_NONE;
        
        game->gameDuration = HAL_GetTick() - game->gameStartTime;
        
        game->state = STATE_GAME_OVER;
        
        ConnectFour_DrawFinalScreen(game);
        return;
    }
    
    game->currentPlayer = (game->currentPlayer == PLAYER_1) ? PLAYER_2 : PLAYER_1;

    ConnectFour_DrawBoard(game);
}

bool ConnectFour_CheckWin(ConnectFourGame *game, uint8_t row, uint8_t col) {
    Player player = game->cells[row][col];
    int count;
    // horizontal
    count = 0;
    for (int c = 0; c < BOARD_COLS; c++) {
        if (game->cells[row][c] == player) {
            count++;
            if (count >= 4) return true;
        } else {
            count = 0;
        }
    }
    // vertical
    count = 0;
    for (int r = 0; r < BOARD_ROWS; r++) {
        if (game->cells[r][col] == player) {
            count++;
            if (count >= 4) return true;
        } else {
            count = 0;
        }
    }
    // diagonal top left -> bottom right
    for (int r = 0; r <= BOARD_ROWS - 4; r++) {
        for (int c = 0; c <= BOARD_COLS - 4; c++) {
            if (game->cells[r][c] == player && 
                game->cells[r+1][c+1] == player && 
                game->cells[r+2][c+2] == player && 
                game->cells[r+3][c+3] == player) {
                return true;
            }
        }
    }
    // diagonal bottom left to rop right
    for (int r = 3; r < BOARD_ROWS; r++) {
        for (int c = 0; c <= BOARD_COLS - 4; c++) {
            if (game->cells[r][c] == player && 
                game->cells[r-1][c+1] == player && 
                game->cells[r-2][c+2] == player && 
                game->cells[r-3][c+3] == player) {
                return true;
            }
        }
    }
    return false;
}

bool ConnectFour_IsBoardFull(ConnectFourGame *game) {
    for (uint8_t col = 0; col < BOARD_COLS; col++) {
        if (game->cells[0][col] == PLAYER_NONE) {
            return false;
        }
    }
    return true;
}

void ConnectFour_ComputerMove(ConnectFourGame *game, RNG_HandleTypeDef *hrng) {
    if (game->gameOver) {
        return;
    }
    
    uint8_t validColumns[BOARD_COLS];
    uint8_t numValidColumns = 0;
    
    for (uint8_t col = 0; col < BOARD_COLS; col++) {
        if (game->cells[0][col] == PLAYER_NONE) {
            validColumns[numValidColumns++] = col;
        }
    }
    
    if (numValidColumns == 0) {
        return;
    }
    
    uint32_t randomValue;
    HAL_RNG_GenerateRandomNumber(hrng, &randomValue);
    uint8_t randomIndex = randomValue % numValidColumns;
    
    game->selectedColumn = validColumns[randomIndex];

    ConnectFour_DrawBoard(game);
    HAL_Delay(250);
    ConnectFour_DropCoin(game);
}

void ConnectFour_DrawFinalScreen(ConnectFourGame *game) {
    LCD_Clear(0, COLOR_BACKGROUND);
    
    LCD_SetFont(&Font16x24);
    LCD_SetTextColor(COLOR_TEXT);
    
    char title[] = "GAME OVER";
    uint16_t titleX = (LCD_PIXEL_WIDTH - strlen(title) * 16) / 2;
    uint16_t y = 40;
    
    for (int i = 0; i < strlen(title); i++) {
        LCD_DisplayChar(titleX + i * 16, y, title[i]);
    }
    
    y += 40;
    char *winnerMsg;
    if (game->winner == PLAYER_NONE) {
        winnerMsg = "DRAW";
    } else {
        winnerMsg = (game->winner == PLAYER_1) ? "PLAYER 1 WINS!" : "PLAYER 2 WINS!";
    }
    
    uint16_t winnerX = (LCD_PIXEL_WIDTH - strlen(winnerMsg) * 16) / 2;
    for (int i = 0; i < strlen(winnerMsg); i++) {
        LCD_DisplayChar(winnerX + i * 16, y, winnerMsg[i]);
    }

    y+= 40;

    char *timeMsg = "Duration: ";
    char timeStr[16];
    sprintf(timeStr, "%ld s", game->gameDuration/1000);
    uint16_t timeX = (LCD_PIXEL_WIDTH - strlen(timeMsg) * 12) / 2;
    
    for (int i = 0; i < strlen(timeMsg); i++) {
        LCD_DisplayChar(timeX + i * 12, y, timeMsg[i]);
    }

    for (int i = 0; i < strlen(timeStr); i++) {
        LCD_DisplayChar(timeX + strlen(timeMsg) * 12 + i * 12, y, timeStr[i]);
    }
}

void ConnectFour_StartNewGame(ConnectFourGame *game, GameState newState) {
    memset(game->cells, PLAYER_NONE, sizeof(game->cells));
    
    game->state = newState;
    game->currentPlayer = PLAYER_1;
    game->selectedColumn = 3;
    game->gameOver = false;
    game->winner = PLAYER_NONE;
    game->gameStartTime = HAL_GetTick();

    ConnectFour_DrawBoard(game);
}

void ConnectFour_ProcessTouch(ConnectFourGame *game, uint16_t touchX, uint16_t touchY, RNG_HandleTypeDef *hrng) {
    switch (game->state) {
        case STATE_MENU:
            if (touchY >= 80 && touchY <= 160) {
                ConnectFour_StartNewGame(game, STATE_GAME_1P);
            }

            else if (touchY >= 180 && touchY <= 260) {
                ConnectFour_StartNewGame(game, STATE_GAME_2P);
            }
            break;
            
        case STATE_GAME_1P:
        case STATE_GAME_2P:
            if (!game->gameOver) {
                if (touchX < LCD_PIXEL_WIDTH / 2) {
                    if (game->selectedColumn > 0) {
                        game->selectedColumn--;
                        ConnectFour_DrawBoard(game);
                    }
                } 

                else {
                    if (game->selectedColumn < BOARD_COLS - 1) {
                        game->selectedColumn++;
                        ConnectFour_DrawBoard(game);
                    }
                }
            }
            break;
            
        case STATE_GAME_OVER:
            ConnectFour_DrawMenu(game);
            game->state = STATE_MENU;
            break;
    }
}