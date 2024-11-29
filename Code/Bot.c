#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "Bot.h"
#include "BattleShip.h"

#define WATER 0
#define HIT -1
#define MISS -2
#define MAX 1000000
#define MAX_NAME_LENGTH 100
#define MAX_NAMES 500

char *generateBotName()
{
    FILE *file = fopen("names.txt", "r");
    if (file == NULL)
    {
        printf("Error: Could not open names.txt\n");
        return NULL;
    }

    char names[MAX_NAMES][MAX_NAME_LENGTH];
    // Array of Names
    int count = 0;

    while (fgets(names[count], MAX_NAME_LENGTH, file) != NULL && count < MAX_NAMES)
    {
        // Remove the newline character at the end if present
        size_t len = strlen(names[count]);
        if (len > 0 && names[count][len - 1] == '\n')
        {
            names[count][len - 1] = '\0';
        }
        count++;
    }

    fclose(file);

    if (count == 0)
    {
        printf("The file is empty.\n");
        return NULL;
    }

    // Seed the random number generator
    srand(time(NULL));

    // Generate a random index
    int randomIndex = rand() % count;

    // Allocate memory for the selected name
    char *selectedName = malloc(strlen(names[randomIndex]) + 1);
    if (selectedName == NULL)
    {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    // Copy the selected name into the allocated memory
    strcpy(selectedName, names[randomIndex]);

    return selectedName;
}
int **allocateMem()
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
    int **finalGrid = allocateMem();

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

void placeShips(struct player *botPlayer)
{
    int **probabilityGrid = generateProbabilityGrid(0, botPlayer->grid, botPlayer->ships); 

    for (int shipSize = 5; shipSize >= 2; shipSize--)  // Largest to smallest
    {
        int placed = 0;
        int attempts = 0;

        while (!placed && attempts < 1000)  // Limit placement attempts
        {
            attempts++;

            // Randomly pick an orientation ('H' for horizontal, 'V' for vertical)
            char orientation = (rand() % 2) ? 'H' : 'V';

            // Randomly select a starting point for the ship placement
            int x = rand() % 10; 
            int y = rand() % 10;

            int valid = 1;

            // Check if the placement is valid
            if (orientation == 'H') // Horizontal
            {
                if (x + shipSize <= 10) 
                {
                    for (int i = 0; i < shipSize; i++)
                    {
                        if (botPlayer->grid[y][x + i] != 0)
                        {
                            valid = 0;
                            break;
                        }
                    }

                    if (valid)
                    {
                        // Place the ship
                        for (int i = 0; i < shipSize; i++)
                        {
                            botPlayer->grid[y][x + i] = shipSize; 
                        }
                        placed = 1;
                    }
                }
            }
            else if (orientation == 'V') // Vertical
            {
                if (y + shipSize <= 10) 
                {
                    for (int i = 0; i < shipSize; i++)
                    {
                        if (botPlayer->grid[y + i][x] != 0)
                        {
                            valid = 0;
                            break;
                        }
                    }

                    if (valid)
                    {
                        // Place the ship
                        for (int i = 0; i < shipSize; i++)
                        {
                            botPlayer->grid[y + i][x] = shipSize; 
                        }
                        placed = 1;
                    }
                }
            }

            // Debugging information
            if (!valid)
            {
                printf("Failed to place ship size %d at (%d, %d) orientation %c\n",
                       shipSize, x, y, orientation);
            }
        }

        // If placement failed after max attempts
        if (!placed)
        {
            printf("Error: Failed to place ship of size %d after 1000 attempts.\n", shipSize);
            exit(1);  // Exit the program to avoid infinite loop
        }
    }

    for (int i = 0; i < 10; i++)
    {
        free(probabilityGrid[i]);
    }
    free(probabilityGrid);
}


void performBotMove(struct player *player, struct player *botPlayer)
{
    int **probabilityGrid = generateProbabilityGrid(1, botPlayer->grid, botPlayer->ships); // 1 for targeting mode

    // If torpedo is unlocked and available, use it on the row or column with the highest probability.
    if (botPlayer->torpedoAvailable)
    {
        float rowProbabilities[10] = {0}; 
        float colProbabilities[10] = {0}; 

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                rowProbabilities[i] += probabilityGrid[i][j];
                colProbabilities[j] += probabilityGrid[i][j];
            }
        }

        float maxRowProb = 0, maxColProb = 0;
        int targetRow = 0, targetCol = 0;

        for (int i = 0; i < 10; i++)
        {
            if (rowProbabilities[i] > maxRowProb)
            {
                maxRowProb = rowProbabilities[i];
                targetRow = i;
            }
            if (colProbabilities[i] > maxColProb)
            {
                maxColProb = colProbabilities[i];
                targetCol = i;
            }
        }

        if (maxRowProb >= maxColProb)
        {
            BotTorpedo(botPlayer->grid, 'R', targetRow, player, botPlayer);  // Fire bot torpedo on row
        }
        else
        {
            BotTorpedo(botPlayer->grid, 'C', targetCol, player, botPlayer);  // Fire bot torpedo on column
        }
        free(probabilityGrid); // Free the memory used by the probability grid
        return;
    }

    // If artillery is unlocked and available, use it on the most probable 2x2 area.
    if (botPlayer->artilleryAvailable)
    {
        int targetRow = 0, targetCol = 0;
        float maxProbability = 0;
        for (int i = 0; i < 9; i++)  
        {
            for (int j = 0; j < 9; j++)
            {
                float areaProbability = probabilityGrid[i][j] +
                                        probabilityGrid[i + 1][j] +
                                        probabilityGrid[i][j + 1] +
                                        probabilityGrid[i + 1][j + 1];

                if (areaProbability > maxProbability)
                {
                    maxProbability = areaProbability;
                    targetRow = i;
                    targetCol = j;
                }
            }
        }

        BotArtillery(botPlayer->grid, targetRow, targetCol, player, botPlayer); // Call bot-specific artillery
        free(probabilityGrid); // Free the memory used by the probability grid
        return;
    }

    // If radar sweeps are available, decide if using one is strategic.
    if (botPlayer->radarCount > 0)
    {
        if (shouldUseRadar(probabilityGrid))
        {
            int targetRow = 0, targetCol = 0;
            float maxProbability = 0;
            for (int i = 0; i < 9; i++)
            {
                for (int j = 0; j < 9; j++)
                {
                    float areaProbability = probabilityGrid[i][j] +
                                            probabilityGrid[i + 1][j] +
                                            probabilityGrid[i][j + 1] +
                                            probabilityGrid[i + 1][j + 1];

                    if (areaProbability > maxProbability)
                    {
                        maxProbability = areaProbability;
                        targetRow = i;
                        targetCol = j;
                    }
                }
            }

            BotRadarSweep(botPlayer->grid, targetRow, targetCol, player, botPlayer);
            botPlayer->radarCount--; // Decrease radar count after using it
            free(probabilityGrid); // Free the memory used by the probability grid
            return;
        }
    }

    // If smoke screens are available, decide if using one is strategic.
    if (botPlayer->availableScreens > 0)
    {
        int targetRow = 0, targetCol = 0;
        float maxProbability = 0;

        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                float areaProbability = probabilityGrid[i][j] +
                                        probabilityGrid[i + 1][j] +
                                        probabilityGrid[i][j + 1] +
                                        probabilityGrid[i + 1][j + 1];

                if (areaProbability > maxProbability)
                {
                    maxProbability = areaProbability;
                    targetRow = i;
                    targetCol = j;
                }
            }
        }

        BotSmokeScreen(botPlayer->grid, targetRow, targetCol, botPlayer);
        botPlayer->availableScreens--; // Decrease smoke screen count after using it
        free(probabilityGrid); // Free the memory used by the probability grid
        return;
    }

    // Default to firing at the cell with the highest probability.
    int targetRow = 0, targetCol = 0;
    float maxProbability = 0;

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (probabilityGrid[i][j] > maxProbability)
            {
                maxProbability = probabilityGrid[i][j];
                targetRow = i;
                targetCol = j;
            }
        }
    }

    BotFire(botPlayer->grid, botPlayer->ships, targetRow, targetCol, player, botPlayer);

    free(probabilityGrid); // Free the memory used by the probability grid
}

int shouldUseRadar(int **probabilityGrid)
{
    // Define a threshold that determines whether radar usage is "worth it"
    float radarThreshold = 5.0;  // Probabilities above this value are interesting for radar sweeps

    // Check for cells that have a high probability of containing ships
    // The bot will try to sweep areas with higher probability first
    float maxProbability = 0;
    int highProbRow = -1, highProbCol = -1;

    for (int i = 0; i < 9; i++)  // Iterate through the grid (excluding the last row and column)
    {
        for (int j = 0; j < 9; j++)
        {
            // Calculate the total probability of the 2x2 area
            float areaProbability = probabilityGrid[i][j] +
                                    probabilityGrid[i + 1][j] +
                                    probabilityGrid[i][j + 1] +
                                    probabilityGrid[i + 1][j + 1];

            // If the area has a higher probability than the max found so far, and it's above the threshold
            if (areaProbability > maxProbability && areaProbability > radarThreshold)
            {
                maxProbability = areaProbability;
                highProbRow = i;
                highProbCol = j;
            }
        }
    }

    // If we found a high-probability area, return 1 to indicate radar sweep should be used
    if (highProbRow != -1 && highProbCol != -1)
    {
        return 1; // Radar sweep should be used
    }

    // Otherwise, return 0 to indicate radar sweep is not worth it
    return 0;
}

void BotFire(int **grid, int ships[], int row, int col, struct player *attacker, struct player *defender)
{
    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;
    waitForMilliseconds(500); 

    if (row < 0 || row >= 10 || col < 0 || col >= 10)
    {
        printWithDelay("Invalid coordinates! Bot loses its turn :(\n", 25);
        return;
    }

    if (grid[row][col] == HIT || grid[row][col] == MISS)
    {
        printWithDelay("Bot already fired at this location!\n", 25);
        return;
    }

    if (grid[row][col] == WATER)
    {
        grid[row][col] = MISS;
        printWithDelay("The bot missed!\n", 25);
        return;
    }

    int shipID = grid[row][col];
    if (shipID > 10)
    {
        shipID /= 10;  
    }

    if (shipID < 2 || shipID > 5)
    {
        printWithDelay("Invalid ship ID! Something went wrong.\n", 25);
        return;
    }

    ships[shipID]--;
    grid[row][col] = HIT;

    printWithDelay("The bot hit your ship!\n", 25);

    if (ships[shipID] == 0)
    {
        char message[50];
        snprintf(message, sizeof(message), "The bot sunk your %s!\n",
                 shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                         : shipID == 4   ? "Battleship"
                                                         : "Carrier");
        printWithDelay(message, 25);

        // Mark the sunk ship's coordinates as MISS or 0 probability
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                if (grid[i][j] == shipID)
                {
                    grid[i][j] = MISS;  // Mark the ship's coordinates as missed
                }
            }
        }

        attacker->shipsSunk++;           
        attacker->availableScreens++;    
        attacker->artilleryNextTurn = 1; 

        if (attacker->shipsSunk == 3)
        {
            attacker->torpedoAvailable = 1;
            printWithDelay("Torpedo unlocked!\n", 25);
        }
    }
}

void BotRadarSweep(int **grid, int row, int col, struct player *attacker, struct player *defender)
{
    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;
    waitForMilliseconds(500);

    if (attacker->radarCount <= 0)
    {
        printWithDelay("The bot has no radar sweeps remaining! It loses its turn.\n", 25);
        return;
    }

    if (row < 0 || row >= 9 || col < 0 || col >= 9)
    {
        printWithDelay("Invalid coordinates! The bot loses its turn.\n", 25);
        return;
    }

    int found = 0;
    for (int i = row; i < row + 2; i++) 
    {
        for (int j = col; j < col + 2; j++)
        {
            int cell = grid[i][j];

            if (cell % 10 == 0)
                continue;

            if (cell >= 2 && cell <= 5) 
            {
                found = 1;
            }
        }
    }

    if (found)
    {
        printWithDelay("The bot found enemy ships!\n", 25);
    }
    else
    {
        printWithDelay("The bot found no enemy ships.\n", 25);
    }

    attacker->radarCount--;
}

void BotSmokeScreen(int **grid, int row, int col, struct player *attacker)
{
    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;
    waitForMilliseconds(500);  

    if (attacker->availableScreens <= 0)
    {
        printWithDelay("The bot has no smoke screens available! It loses its turn.\n", 25);
        return;
    }

    if (row < 0 || row >= 9 || col < 0 || col >= 9)
    {
        printWithDelay("Invalid coordinates! The bot loses its turn.\n", 25);
        return;
    }

    for (int i = row; i < row + 2; i++)
    {
        for (int j = col; j < col + 2; j++)
        {
            grid[i][j] *= 10; 
        }
    }

    attacker->availableScreens--;

    clear_terminal();

    printWithDelay("The bot deployed a smoke screen! The move is hidden.\n", 25);
}

void BotArtillery(int **grid, int row, int col, struct player *attacker, struct player *opponent)
{
    waitForMilliseconds(500); 

    if (!attacker->artilleryNextTurn)
    {
        printWithDelay("Artillery not available! It was only available for the next turn after sinking a ship.\n", 25);
        return;
    }

    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;

    if (row < 0 || row >= 9 || col < 0 || col >= 9)
    {
        printWithDelay("Invalid coordinates! The bot loses its turn.\n", 25);
        return;
    }

    int hit = 0;
    int *ships = opponent->ships;

    for (int i = row; i < row + 2; i++)
    {
        for (int j = col; j < col + 2; j++)
        {
            if (grid[i][j] > 0)  
            {
                int shipID = grid[i][j];
                if (shipID > 10)
                {
                    shipID /= 10; 
                }
                ships[shipID]--;  
                grid[i][j] = HIT;  
                hit = 1;

                if (ships[shipID] == 0)
                {
                    char message[50];
                    snprintf(message, sizeof(message), "The bot sunk your %s!\n",
                             shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                                     : shipID == 4   ? "Battleship"
                                                                     : "Carrier");
                    printWithDelay(message, 25);

                    opponent->shipsRemaining--;  
                    attacker->shipsSunk++;  
                    attacker->artilleryNextTurn = 1;  
                    attacker->availableScreens++;   

                    if (attacker->shipsSunk == 3)
                    {
                        attacker->torpedoAvailable = 1;
                        printWithDelay("Torpedo unlocked!\n", 25);
                    }
                    for (int i = 0; i < 10; i++)
                    {
                        for (int j = 0; j < 10; j++)
                        {
                            if (grid[i][j] == shipID)
                            {
                                grid[i][j] = MISS;  
                            }
                        }
                    }
                }
            }
            else if (grid[i][j] == WATER) 
            {
                grid[i][j] = MISS;  
            }
        }
    }

    if (hit)
    {
        printWithDelay("The bot's artillery hit!\n", 25);
    }
    else
    {
        printWithDelay("The bot's artillery missed!\n", 25);
    }
}

void BotTorpedo(int **grid, char type, int index, struct player *attacker, struct player *opponent)
{
    waitForMilliseconds(500);

    if (!attacker->torpedoAvailable)
    {
        printWithDelay("The bot's torpedo is not available! It can only be used after sinking 3 ships.\n", 25);
        return;
    }

    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;

    if (index < 0 || index >= 10)
    {
        printWithDelay("Invalid index! The bot loses its turn.\n", 25);
        return;
    }

    int hit = 0;

    if (type == 'R' || type == 'r')
    {
        for (int j = 0; j < 10; j++)
        {
            if (grid[index][j] > 0)
            {
                int shipID = grid[index][j];
                if (shipID > 10)
                {
                    shipID /= 10;
                }
                opponent->ships[shipID]--;
                grid[index][j] = HIT;
                hit = 1;

                if (opponent->ships[shipID] == 0)
                {
                    char message[50];
                    snprintf(message, sizeof(message), "The bot sunk your %s!\n",
                             shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                                     : shipID == 4   ? "Battleship"
                                                                     : "Carrier");
                    printWithDelay(message, 25);
                    opponent->shipsRemaining--;
                    attacker->shipsSunk++;
                    attacker->availableScreens++;

                    for (int i = 0; i < 10; i++)
                    {
                        for (int j = 0; j < 10; j++)
                        {
                            if (grid[i][j] == shipID)
                            {
                                grid[i][j] = MISS;
                            }
                        }
                    }
                }
            }
            else if (grid[index][j] == WATER)
            {
                grid[index][j] = MISS;
            }
        }
    }
    else if (type == 'C' || type == 'c')
    {
        for (int i = 0; i < 10; i++)
        {
            if (grid[i][index] > 0)
            {
                int shipID = grid[i][index];
                if (shipID > 10)
                {
                    shipID /= 10;
                }
                opponent->ships[shipID]--;
                grid[i][index] = HIT;
                hit = 1;

                if (opponent->ships[shipID] == 0)
                {
                    char message[50];
                    snprintf(message, sizeof(message), "The bot sunk your %s!\n",
                             shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                                     : shipID == 4   ? "Battleship"
                                                                     : "Carrier");
                    printWithDelay(message, 25);
                    opponent->shipsRemaining--;
                    attacker->shipsSunk++;
                    attacker->availableScreens++;

                    for (int i = 0; i < 10; i++)
                    {
                        for (int j = 0; j < 10; j++)
                        {
                            if (grid[i][j] == shipID)
                            {
                                grid[i][j] = MISS;
                            }
                        }
                    }
                }
            }
            else if (grid[i][index] == WATER)
            {
                grid[i][index] = MISS;
            }
        }
    }
    else
    {
        printWithDelay("Invalid type! Use 'R' for row or 'C' for column.\n", 25);
        return;
    }

    if (hit)
    {
        printWithDelay("The bot's torpedo hit!\n", 25);
    }
    else
    {
        printWithDelay("The bot's torpedo missed!\n", 25);
    }
}










// void printGrid(int **grid)
// {
//     for (int i = 0; i < 10; i++)
//     {
//         for (int j = 0; j < 10; j++)
//         {
//             if (grid[i][j] >= MAX)
//             {
//                 printf("X ");
//                 continue;
//             }
//             printf("%d ", grid[i][j]);
//         }
//         printf("\n");
//     }
// }

// int main()
// {
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

//     return 0;
// }
