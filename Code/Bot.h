#ifndef BOT_H
#define BOT_H

#include "BattleShip.h" // Include BattleShip.h for struct player and shared function declarations

// Function to generate the probability grid for the bot's targeting
int **generateProbabilityGrid(int **currentGrid, int ships[6]);

// Function to generate a random name for the bot
char *generateBotName();

// Function to place ships for the bot on the grid
void placeShips(struct player *botPlayer);

// Function to perform a bot move (decides and performs the best move)
void selectMoveDifficulty(struct player *player, struct player *botPlayer, int difficulty);
void performBotMoveHard(struct player *player, struct player *botPlayer);
void performBotMoveMedium(struct player *player, struct player *botPlayer);
void performBotMoveEasy(struct player *player, struct player *botPlayer);

// Bot-specific move functions
void BotFire(int **grid, int ships[], int row, int col, struct player *attacker, struct player *defender);
void BotRadarSweep(int **grid, int row, int col, struct player *attacker, struct player *defender);
void BotSmokeScreen(int **grid, int row, int col, struct player *attacker);
void BotArtillery(int **grid, int row, int col, struct player *attacker, struct player *defender);
void BotTorpedo(int **grid, char type, int index, struct player *attacker, struct player *defender);

#endif
