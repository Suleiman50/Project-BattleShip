#include <stdlib.h>
#include <stdio.h>
#include "Bot.h"
#define WATER 0
#define HIT -1
#define MISS -2
#define MAX 1000000

int **allocate()
{
    int **grid = (int **)malloc(10 * sizeof(int *));
    for (int i = 0; i < 10; i++)
    {
        grid[i] = (int *)malloc(10 * sizeof(int));
        for (int j = 0; j < 10; j++)
        {
            grid[i][j] = 0;
        }
    }
    return grid;
}

int **generateProbabilityGrid(int isTargeter, int **currentGrid, int ships[6])
{
    int **finalGrid = allocate();

    // Horizontal Ships
    for (int k = 2; k < 6; k++)
    {
        if (ships[k] == 0)
            continue;

        for (int row = 0; row < 10; row++)
        {
            int left = 0, right = 0;
            int targeterValid = 0;

            while (right < 10)
            {
                if (currentGrid[row][right] == MISS)
                {
                    // Reset Window After Finding Miss
                    left = right + 1;
                    targeterValid = 0;
                    right = left;
                    continue;
                }
                if (currentGrid[row][right] == HIT)
                {
                    finalGrid[row][right] = MAX;
                    targeterValid = 1;
                }

                if (right - left + 1 == k)
                {
                    // Valid Window Condition
                    if (!isTargeter || targeterValid)
                    {
                        // Targeter Condition
                        for (int i = left; i <= right; i++)
                        {
                            if (currentGrid[row][i] != MISS && finalGrid[row][i] < MAX)
                                finalGrid[row][i] += 1;
                        }
                    }
                    left++;
                }
                right++;
            }
        }
    }

    // Vertical Ships
    for (int k = 2; k < 6; k++)
    {
        if (ships[k] == 0)
            continue;

        for (int col = 0; col < 10; col++)
        {
            int top = 0, bottom = 0;
            int targeterValid = 0;

            while (bottom < 10)
            {
                if (currentGrid[bottom][col] == MISS)
                {
                    // Reset Window After Miss
                    top = bottom + 1;
                    targeterValid = 0;
                    bottom++;
                    continue;
                }
                if (currentGrid[bottom][col] == HIT)
                {
                    finalGrid[bottom][col] = MAX;
                    targeterValid = 1;
                }

                if (bottom - top + 1 == k)
                {
                    // Valid Window Condition
                    if (!isTargeter || targeterValid)
                    {
                        // Targeter Condition
                        for (int i = top; i <= bottom; i++)
                        {
                            if (currentGrid[i][col] != MISS && finalGrid[i][col] < MAX)
                                finalGrid[i][col] += 1;
                        }
                    }
                    top++;
                }
                bottom++;
            }
        }
    }

    return finalGrid;
}

void printGrid(int **grid)
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (grid[i][j] >= MAX)
            {
                printf("X ");
                continue;
            }
            printf("%d ", grid[i][j]);
        }
        printf("\n");
    }
}

int main()
{
    // int ships[6] = {0, 0, 2, 3, 4, 5};
    // int **grid = allocate();
    // for (int j = 0; j < 5; j++)
    //     grid[0][j] = 5;

    // for (int i = 1; i <= 4; i++)
    //     grid[i][4] = 4;

    // for (int j = 6; j < 9; j++)
    //     grid[6][j] = 3;

    // for (int j = 7; j < 9; j++)
    //     grid[9][j] = 2;

    // grid[0][2] = -2; // Miss on size-5 ship
    // grid[6][7] = -2;
    // grid[8][7] = -2;
    // grid[5][5] = -2; // Miss
    // grid[4][4] = -2; // Miss

    // int **probaGrid = generateProbabilityGrid(0, grid, ships);
    // printGrid(probaGrid);

    // // Free allocated memory
    // for (int i = 0; i < 10; i++)
    // {
    //     free(grid[i]);
    //     free(probaGrid[i]);
    // }
    // free(grid);
    // free(probaGrid);

    return 0;
}
