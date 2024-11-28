#ifndef BOT_H
#define BOT_H

// int** Generate probability grid (int isTargeter, int** currentGridState) --> Returns the main proba grid
int **generateProbabilityGrid(int isTargeter, int **currentGrid);
// int[] Pick and Attack (int** probabilityGrid) --> Returns an array first element being the move second element being the cooredinate
// int** Place Ships() --> Returns A grid with ships

#endif