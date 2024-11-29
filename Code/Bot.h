
#ifndef BOT_H
#define BOT_H

int **generateProbabilityGrid(int isTargeter, int **currentGrid, int ships[6]);
char *generateBotName();
// void placeShips(*player) --> Takes the botPlayer class & places ships for the bot
// void PerformBotMove(*player, *botPlayer) --> Performs the move by following the below steps
/*
    1- pick between the 5 moves based on the decider
    2- update the grid by using the generateProbabilityGrid() function (unless move is SmokeScreen)
    3- perform selected move based on the ProbabilityGrid
    4- call correct Modified Move Function depending on the move (modify move functions from BattleShip.c)
*/

#endif