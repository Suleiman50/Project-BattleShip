#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include "Bot.h"
#include "BattleShip.h"

#define WATER 0
#define HIT -1
#define MISS -2
#define MAX 1000000
#define MAX_NAME_LENGTH 100
#define MAX_NAMES 500

int **playerCopyGrid;

// Structure to represent a possible ship location
typedef struct
{
    int shipSize;           // Size of the ship
    int row;                // Starting row
    int col;                // Starting column
    int orientation;        // 0 for horizontal, 1 for vertical
    int shipIndex;          // Index of the ship (0 to numShips - 1)
    int locationIndex;      // Index in the possible locations array
    int occupiedSquares[5]; // List of squares occupied by the ship (max size 5)
} ShipLocation;

// Function to generate a random bot name from a file
char *generateBotName()
{
    FILE *file = fopen("names.txt", "r");
    if (file == NULL)
    {
        printf("Error: Could not open names.txt\n");
        return NULL;
    }

    char names[MAX_NAMES][MAX_NAME_LENGTH];
    int count = 0;

    while (fgets(names[count], MAX_NAME_LENGTH, file) != NULL && count < MAX_NAMES)
    {
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

    srand(time(NULL));

    int randomIndex = rand() % count;

    char *selectedName = malloc(strlen(names[randomIndex]) + 1);
    if (selectedName == NULL)
    {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    strcpy(selectedName, names[randomIndex]);

    return selectedName;
}

// Function to allocate memory for a 10x10 grid and initialize it to zeros
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

// less powerful version of the probability Grid generation
int **generateProbabilityGridSlidingWindow(int isTargeter, int **currentGrid, int ships[6])
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

// Helper function to generate all possible ship locations for the remaining ships
void generatePossibleShipLocations(int **grid, int shipSizes[], int numShips,
                                   ShipLocation *possibleShipLocations[], int numPossibleShipLocations[])
{
    for (int shipIndex = 0; shipIndex < numShips; shipIndex++)
    {
        int size = shipSizes[shipIndex];
        int capacity = 200; // Initial capacity for possible locations
        possibleShipLocations[shipIndex] = malloc(capacity * sizeof(ShipLocation));
        numPossibleShipLocations[shipIndex] = 0;

        // Iterate over all cells in the grid
        for (int row = 0; row < 10; row++)
        {
            for (int col = 0; col < 10; col++)
            {
                // Try both horizontal and vertical orientations
                for (int orientation = 0; orientation < 2; orientation++)
                {
                    // Check if the ship would overhang the board
                    if (orientation == 0 && col + size > 10)
                        continue;
                    if (orientation == 1 && row + size > 10)
                        continue;

                    int overlapMiss = 0; // Flag to check overlap with MISS squares
                    int occupiedSquares[5];
                    int coversAllHits = 1; // Flag to ensure ship covers all HIT squares

                    // Check each square the ship would occupy
                    for (int k = 0; k < size; k++)
                    {
                        int r = row + (orientation == 1 ? k : 0);
                        int c = col + (orientation == 0 ? k : 0);

                        if (grid[r][c] == MISS)
                        {
                            overlapMiss = 1; // Cannot place ship here
                            break;
                        }

                        occupiedSquares[k] = r * 10 + c; // Convert 2D index to 1D
                    }

                    if (overlapMiss)
                        continue; // Skip to next position

                    // Add this ship location to the list
                    if (numPossibleShipLocations[shipIndex] >= capacity)
                    {
                        capacity *= 2;
                        possibleShipLocations[shipIndex] = realloc(possibleShipLocations[shipIndex],
                                                                   capacity * sizeof(ShipLocation));
                    }

                    ShipLocation loc;
                    loc.shipSize = size;
                    loc.row = row;
                    loc.col = col;
                    loc.orientation = orientation;
                    loc.shipIndex = shipIndex;
                    loc.locationIndex = numPossibleShipLocations[shipIndex];

                    for (int k = 0; k < size; k++)
                    {
                        loc.occupiedSquares[k] = occupiedSquares[k];
                    }

                    possibleShipLocations[shipIndex][numPossibleShipLocations[shipIndex]] = loc;
                    numPossibleShipLocations[shipIndex]++;
                }
            }
        }
    }
}

// Most powerful probability grid generation & using parallel programming
int **generateProbabilityGrid(int **grid, int ships[6])
{
    int numShips = 0;
    int shipSizes[4]; // Array to hold sizes of remaining ships

    // Collect sizes of ships that have not been sunk yet
    for (int k = 2; k <= 5; k++)
    {
        if (ships[k] > 0)
        {
            shipSizes[numShips++] = k;
        }
    }

    ShipLocation *possibleShipLocations[4]; // Array to hold possible locations for each ship
    int numPossibleShipLocations[4];        // Number of possible locations for each ship

    // Generate all possible ship locations
    generatePossibleShipLocations(grid, shipSizes, numShips, possibleShipLocations, numPossibleShipLocations);

    // Initialize location frequencies for each ship
    int **locationFrequencies = malloc(numShips * sizeof(int *));
    for (int shipIndex = 0; shipIndex < numShips; shipIndex++)
    {
        locationFrequencies[shipIndex] = calloc(numPossibleShipLocations[shipIndex], sizeof(int));
    }

    int validConfigurationsCounted = 0; // Total number of valid configurations found
    int y = 10000;                      // Number of samples for Monte Carlo simulation

// Parallelization using OpenMP
#pragma omp parallel
    {
        // Thread-local copies of variables to avoid race conditions
        int **localLocationFrequencies = malloc(numShips * sizeof(int *));
        for (int shipIndex = 0; shipIndex < numShips; shipIndex++)
        {
            localLocationFrequencies[shipIndex] = calloc(numPossibleShipLocations[shipIndex], sizeof(int));
        }

        int localValidConfigurations = 0; // Thread-local count of valid configurations

        unsigned int seed = (unsigned int)time(NULL) ^ omp_get_thread_num(); // Unique seed for each thread

// Parallel for loop over the number of samples
#pragma omp for
        for (int sample = 0; sample < y; sample++)
        {
            ShipLocation selectedLocations[4]; // Selected locations for each ship
            int selectedIndices[4];            // Indices of the selected locations
            int occupiedSquares[100] = {0};    // Grid to mark occupied squares

            int overlap = 0; // Flag to check for overlaps between ships

            // Randomly select positions for each ship
            for (int shipIndex = 0; shipIndex < numShips; shipIndex++)
            {
                int n = numPossibleShipLocations[shipIndex];
                if (n == 0)
                {
                    overlap = 1; // No possible positions for this ship
                    break;
                }

                int randIndex = rand_r(&seed) % n; // Thread-safe random number

                ShipLocation loc = possibleShipLocations[shipIndex][randIndex];
                selectedLocations[shipIndex] = loc;
                selectedIndices[shipIndex] = randIndex;

                // Check for overlaps with other ships
                for (int k = 0; k < loc.shipSize; k++)
                {
                    int square = loc.occupiedSquares[k];
                    if (occupiedSquares[square])
                    {
                        overlap = 1;
                        break;
                    }
                    else
                    {
                        occupiedSquares[square] = 1;
                    }
                }
                if (overlap)
                    break;
            }

            if (overlap)
                continue; // Skip to next sample

            // Check for conflicts with the current board state
            int conflict = 0;
            for (int i = 0; i < 100; i++)
            {
                int row = i / 10;
                int col = i % 10;

                if (grid[row][col] == HIT)
                {
                    if (!occupiedSquares[i])
                    {
                        conflict = 1; // HIT cell not covered by any ship
                        break;
                    }
                }
                else if (grid[row][col] == MISS)
                {
                    if (occupiedSquares[i])
                    {
                        conflict = 1; // MISS cell occupied by a ship
                        break;
                    }
                }
            }

            if (conflict)
                continue; // Skip to next sample

            // Valid configuration found
            for (int shipIndex = 0; shipIndex < numShips; shipIndex++)
            {
                localLocationFrequencies[shipIndex][selectedIndices[shipIndex]]++;
            }
            localValidConfigurations++;
        } // End of parallel for loop

// Critical section to update shared variables
#pragma omp critical
        {
            for (int shipIndex = 0; shipIndex < numShips; shipIndex++)
            {
                for (int locIndex = 0; locIndex < numPossibleShipLocations[shipIndex]; locIndex++)
                {
                    locationFrequencies[shipIndex][locIndex] += localLocationFrequencies[shipIndex][locIndex];
                }
            }
            validConfigurationsCounted += localValidConfigurations;
        }

        // Free thread-local memory
        for (int shipIndex = 0; shipIndex < numShips; shipIndex++)
        {
            free(localLocationFrequencies[shipIndex]);
        }
        free(localLocationFrequencies);
    } // End of parallel region

    // Compute square frequencies based on location frequencies
    int squareFrequencies[100] = {0};

    for (int shipIndex = 0; shipIndex < numShips; shipIndex++)
    {
        for (int locIndex = 0; locIndex < numPossibleShipLocations[shipIndex]; locIndex++)
        {
            int freq = locationFrequencies[shipIndex][locIndex];
            if (freq > 0)
            {
                ShipLocation loc = possibleShipLocations[shipIndex][locIndex];
                for (int k = 0; k < loc.shipSize; k++)
                {
                    int square = loc.occupiedSquares[k];
                    squareFrequencies[square] += freq;
                }
            }
        }
    }

    // Allocate and initialize the probability grid
    int **probabilityGrid = allocateMem();

    // Exclude HIT and MISS cells from being targeted again
    for (int i = 0; i < 100; i++)
    {
        int row = i / 10;
        int col = i % 10;
        if (grid[row][col] == HIT || grid[row][col] == MISS)
        {
            probabilityGrid[row][col] = 0;
        }
        else
        {
            probabilityGrid[row][col] = squareFrequencies[i];
        }
    }

    // Free allocated memory
    for (int shipIndex = 0; shipIndex < numShips; shipIndex++)
    {
        free(possibleShipLocations[shipIndex]);
        free(locationFrequencies[shipIndex]);
    }
    free(locationFrequencies);

    return probabilityGrid; // Return the computed probability grid
}

void placeShips(struct player *botPlayer)
{
    playerCopyGrid = allocateMem();
    int **probabilityGrid = generateProbabilityGrid(botPlayer->grid, botPlayer->ships);

    for (int shipSize = 5; shipSize >= 2; shipSize--) // Largest to smallest
    {
        int placed = 0;
        int attempts = 0;

        while (!placed && attempts < 1000) // Limit placement attempts
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
            exit(1); // Exit the program to avoid infinite loop
        }
    }

    printGrid(botPlayer->grid, 1);
    getchar();
    printWithDelay("Press to start.", 25);
    getchar();
    waitForMilliseconds(3000);
    for (int i = 0; i < 10; i++)
    {
        free(probabilityGrid[i]);
    }
    free(probabilityGrid);
}

void performBotMove(struct player *player, struct player *botPlayer)
{
    int **probabilityGrid = generateProbabilityGrid(playerCopyGrid, player->ships);
    int isTargeter = 0;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (playerCopyGrid[i][j] == HIT)
            {
                isTargeter = 1;
                break;
            }
        }
    }
    // This if condition is to override the use of the smoke screen if we are in targeter mode
    if (isTargeter && !botPlayer->torpedoAvailable && !botPlayer->artilleryAvailable)
    {
        int targetRow = 0, targetCol = 0;
        int maxProbability = 0;

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                if (probabilityGrid[i][j] > maxProbability && probabilityGrid[i][j] < MAX)
                {
                    maxProbability = probabilityGrid[i][j];
                    targetRow = i;
                    targetCol = j;
                }
            }
        }

        BotFire(player->grid, player->ships, targetRow, targetCol, botPlayer, player);

        free(probabilityGrid);
        return;
    }
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

        // Bot fires the torpedo on the row or column with the highest probability
        if (maxRowProb >= maxColProb)
        {
            BotTorpedo(player->grid, 'R', targetRow, botPlayer, player); // Use player's grid
        }
        else
        {
            BotTorpedo(player->grid, 'C', targetCol, botPlayer, player); // Use player's grid
        }

        free(probabilityGrid); // Free the memory used by the probability grid
        return;
    }

    // If artillery is unlocked and available (after sinking a ship), use it on the most probable 2x2 area.
    if (botPlayer->artilleryNextTurn && !botPlayer->torpedoAvailable)
    {
        int targetRow = 0, targetCol = 0;
        int maxProbability = 0;
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                int areaProbability = probabilityGrid[i][j] +
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

        // Call bot-specific artillery (use player's grid)
        BotArtillery(player->grid, targetRow, targetCol, botPlayer, player);
        free(probabilityGrid); // Free the memory used by the probability grid
        return;
    }

    // If radar sweeps are available, decide if using one is strategic.
    if (botPlayer->radarCount > 1)
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

        BotRadarSweep(player->grid, targetRow, targetCol, botPlayer, player);
        free(probabilityGrid); // Free the memory used by the probability grid
        return;
    }

    // If smoke screens are available, decide if using one is strategic.
    if (botPlayer->availableScreens > 0)
    {
        int targetRow = 0, targetCol = 0;
        int maxShips = 0;

        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                int shipCount = 0;
                if (botPlayer->grid[i][j] > 0)
                {
                    shipCount++;
                }
                if (botPlayer->grid[i][j + 1] > 0)
                {
                    shipCount++;
                }
                if (botPlayer->grid[i + 1][j] > 0)
                {
                    shipCount++;
                }
                if (botPlayer->grid[i + 1][j + 1] > 0)
                {
                    shipCount++;
                }
                if (shipCount > maxShips)
                {
                    maxShips = shipCount;
                    targetRow = i;
                    targetCol = j;
                }
            }
        }

        BotSmokeScreen(botPlayer->grid, targetRow, targetCol, botPlayer);
        botPlayer->availableScreens--; // Decrease smoke screen count after using it
        free(probabilityGrid);         // Free the memory used by the probability grid
        return;
    }

    // Default to firing at the cell with the highest probability.
    int targetRow = 0, targetCol = 0;
    int maxProbability = 0;

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (probabilityGrid[i][j] > maxProbability && probabilityGrid[i][j] < MAX)
            {
                maxProbability = probabilityGrid[i][j];
                targetRow = i;
                targetCol = j;
            }
        }
    }

    BotFire(player->grid, player->ships, targetRow, targetCol, botPlayer, player);

    free(probabilityGrid); // Free the memory used by the probability grid
}

// int shouldUseRadar(int **probabilityGrid)
// {
//     // Define a threshold that determines whether radar usage is "worth it"
//     float radarThreshold = 5.0; // Probabilities above this value are interesting for radar sweeps

//     // Check for cells that have a high probability of containing ships
//     // The bot will try to sweep areas with higher probability first
//     float maxProbability = 0;
//     int highProbRow = -1, highProbCol = -1;

//     for (int i = 0; i < 9; i++) // Iterate through the grid (excluding the last row and column)
//     {
//         for (int j = 0; j < 9; j++)
//         {
//             // Calculate the total probability of the 2x2 area
//             int areaProbability = probabilityGrid[i][j] +
//                                   probabilityGrid[i + 1][j] +
//                                   probabilityGrid[i][j + 1] +
//                                   probabilityGrid[i + 1][j + 1];

//             // If the area has a higher probability than the max found so far, and it's above the threshold
//             if (areaProbability > maxProbability && areaProbability > radarThreshold)
//             {
//                 maxProbability = areaProbability;
//                 highProbRow = i;
//                 highProbCol = j;
//             }
//         }
//     }

//     // If we found a high-probability area, return 1 to indicate radar sweep should be used
//     if (highProbRow != -1 && highProbCol != -1)
//     {
//         return 1; // Radar sweep should be used
//     }

//     // Otherwise, return 0 to indicate radar sweep is not worth it
//     return 0;
// }

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
        playerCopyGrid[row][col] = MISS;
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
    playerCopyGrid[row][col] = HIT;

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
                if (playerCopyGrid[i][j] == HIT)
                {
                    playerCopyGrid[i][j] = MISS; // Mark the ship's coordinates as missed
                }
            }
        }

        attacker->shipsSunk++;
        attacker->availableScreens++;
        attacker->artilleryNextTurn = 1;

        if (attacker->shipsSunk == 3)
        {
            attacker->torpedoAvailable = 1;
            // printWithDelay("Torpedo unlocked!\n", 25);
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

    if (!found)
    {
        // printWithDelay("The bot found enemy ships!\n", 25);
        for (int i = row; i < row + 2; i++)
        {
            for (int j = col; j < col + 2; j++)
            {
                playerCopyGrid[i][j] = MISS;
            }
        }
    }
    else
    {
        attacker->radarCount = 0;
        return;
    }
    printWithDelay("The bot used a radar Sweep!", 25);
    waitForMilliseconds(300);
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
            if (grid[i][j] != HIT && grid[i][j] != MISS)
            {
                grid[i][j] *= 10;
            }
        }
    }

    attacker->availableScreens--;

    clear_terminal();

    printWithDelay("The bot deployed a smoke screen! The move is hidden.\n", 25);
    waitForMilliseconds(300);
}

void BotArtillery(int **grid, int row, int col, struct player *attacker, struct player *opponent)
{
    waitForMilliseconds(500); // Simulate a small delay

    if (!attacker->artilleryNextTurn)
    {
        printWithDelay("Artillery not available! It was only available for the next turn after sinking a ship.\n", 25);
        return;
    }

    attacker->artilleryNextTurn = 0; // Reset artillery for next turn

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
            if (grid[i][j] > 0) // There's a ship at this position
            {
                int shipID = grid[i][j];
                if (shipID > 10)
                {
                    shipID /= 10;
                } // Normalize shipID if needed

                ships[shipID]--;            // Decrease ship health
                grid[i][j] = HIT;           // Mark hit on the player's grid
                playerCopyGrid[i][j] = HIT; // Update bot's internal grid
                hit = 1;

                if (ships[shipID] == 0) // If the ship is sunk
                {
                    // Notify about the ship being sunk
                    char message[50];
                    snprintf(message, sizeof(message), "The bot sunk your %s!\n",
                             shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                                     : shipID == 4   ? "Battleship"
                                                                     : "Carrier");
                    printWithDelay(message, 25);

                    opponent->shipsRemaining--;   // Decrease remaining ships
                    attacker->shipsSunk++;        // Increase bot's sunk ships
                    attacker->availableScreens++; // Give an extra screen (if relevant)

                    // Mark the sunk ship's coordinates as MISS on both grids
                    for (int i = 0; i < 10; i++)
                    {
                        for (int j = 0; j < 10; j++)
                        {
                            if (playerCopyGrid[i][j] == HIT)
                            {
                                playerCopyGrid[i][j] = MISS; // Mark the ship's coordinates as missed
                                grid[i][j] = MISS;           // Mark the ship's coordinates as missed
                            }
                        }
                    }
                }
            }
            else if (grid[i][j] == WATER) // Mark water as miss
            {
                grid[i][j] = MISS;
                playerCopyGrid[i][j] = MISS;
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
    waitForMilliseconds(500); // Simulate a small delay

    // If torpedo has already been used, show a message and exit
    if (!attacker->torpedoAvailable)
    {
        printWithDelay("The bot's torpedo has already been used!\n", 25);
        return;
    }

    attacker->torpedoAvailable = 0;  // Mark torpedo as used (it can be used only once)
    attacker->artilleryNextTurn = 0; // Reset artillery next turn (if relevant)

    // Ensure the index is within valid range
    if (index < 0 || index >= 10)
    {
        printWithDelay("Invalid index! The bot loses its turn.\n", 25);
        return;
    }

    int hit = 0;

    // Handling the row-based torpedo attack
    if (type == 'R' || type == 'r')
    {
        for (int j = 0; j < 10; j++)
        {
            if (grid[index][j] > 0) // There's a ship at this position
            {
                int shipID = grid[index][j];
                if (shipID > 10)
                {
                    shipID /= 10;
                } // Normalize shipID if needed

                opponent->ships[shipID]--;      // Decrease ship health
                grid[index][j] = HIT;           // Mark hit on the player's grid
                playerCopyGrid[index][j] = HIT; // Update bot's internal grid
                hit = 1;

                if (opponent->ships[shipID] == 0) // If the ship is sunk
                {
                    // Notify about the ship being sunk
                    char message[50];
                    snprintf(message, sizeof(message), "The bot sunk your %s!\n",
                             shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                                     : shipID == 4   ? "Battleship"
                                                                     : "Carrier");
                    printWithDelay(message, 25);
                    opponent->shipsRemaining--;   // Decrease remaining ships
                    attacker->shipsSunk++;        // Increase bot's sunk ships
                    attacker->availableScreens++; // Give an extra screen (if relevant)

                    // Mark the sunk ship's coordinates as MISS on both grids
                    for (int i = 0; i < 10; i++)
                    {
                        for (int j = 0; j < 10; j++)
                        {
                            if (playerCopyGrid[i][j] == HIT)
                            {
                                playerCopyGrid[i][j] = MISS; // Mark the ship's coordinates as missed
                                grid[i][j] = MISS;           // Mark the ship's coordinates as missed
                            }
                        }
                    }
                }
            }
            else if (grid[index][j] == WATER) // Mark water as miss
            {
                grid[index][j] = MISS;
                playerCopyGrid[index][j] = MISS;
            }
        }
    }
    // Handling the column-based torpedo attack
    else if (type == 'C' || type == 'c')
    {
        for (int i = 0; i < 10; i++)
        {
            if (grid[i][index] > 0) // There's a ship at this position
            {
                int shipID = grid[i][index];
                if (shipID > 10)
                {
                    shipID /= 10;
                } // Normalize shipID if needed

                opponent->ships[shipID]--;      // Decrease ship health
                grid[i][index] = HIT;           // Mark hit on the player's grid
                playerCopyGrid[i][index] = HIT; // Update bot's internal grid
                hit = 1;

                if (opponent->ships[shipID] == 0) // If the ship is sunk
                {
                    // Notify about the ship being sunk
                    char message[50];
                    snprintf(message, sizeof(message), "The bot sunk your %s!\n",
                             shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                                     : shipID == 4   ? "Battleship"
                                                                     : "Carrier");
                    printWithDelay(message, 25);
                    opponent->shipsRemaining--;   // Decrease remaining ships
                    attacker->shipsSunk++;        // Increase bot's sunk ships
                    attacker->availableScreens++; // Give an extra screen (if relevant)

                    // Mark the sunk ship's coordinates as MISS on both grids
                    for (int i = 0; i < 10; i++)
                    {
                        for (int j = 0; j < 10; j++)
                        {
                            if (playerCopyGrid[i][j] == HIT)
                            {
                                playerCopyGrid[i][j] = MISS; // Mark the ship's coordinates as missed
                                grid[i][j] = MISS;           // Mark the ship's coordinates as missed
                            }
                        }
                    }
                }
            }
            else if (grid[i][index] == WATER) // Mark water as miss
            {
                grid[i][index] = MISS;
                playerCopyGrid[i][index] = MISS;
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
// void printGrids(int **grid)
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
//     int ships[6] = {0, 0, 2, 3, 4, 5};
//     int **grid = allocateMem();
//     // for (int j = 0; j < 5; j++)
//     //     grid[0][j] = 5;

//     // for (int i = 1; i <= 4; i++)
//     //     grid[i][4] = 4;

//     // for (int j = 6; j < 9; j++)
//     //     grid[6][j] = 3;

//     // for (int j = 7; j < 9; j++)
//     //     grid[9][j] = 2;

//     grid[3][3] = -2; // Miss on size-5 ship
//     grid[4][4] = -2;
//     grid[5][5] = -2;
//     grid[6][6] = -2; // Miss
//     grid[2][7] = -2;
//     grid[2][6] = -1;
//     grid[3][7] = -2;
//     grid[3][6] = -1;
//     int **probaGrid = generateProbabilityGrid(grid, ships);
//     printGrids(probaGrid);

//     // Free allocated memory
//     for (int i = 0; i < 10; i++)
//     {
//         free(grid[i]);
//         free(probaGrid[i]);
//     }
//     free(grid);
//     free(probaGrid);

//     return 0;
// }
