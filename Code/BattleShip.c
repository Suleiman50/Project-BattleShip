#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Bot.h"

#define WATER 0
#define HIT -1
#define MISS -2

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Define ANSI color codes
#define RESET_COLOR "\033[0m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"

// Wait function in milliseconds
void waitForMilliseconds(int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

// Function to print text with a typing effect
void printWithDelay(const char *text, int delayMilliseconds)
{
    while (*text)
    {
        putchar(*text++);
        fflush(stdout); // Ensure character is printed immediately
        waitForMilliseconds(delayMilliseconds);
    }
}

// Clearing Terminal Function
void clear_terminal()
{
#ifdef _WIN32
    system("cls"); // Windows
#else
    system("clear");
#endif
}

// Custom Loading Animation Function
void loadingAnimation(char *text)
{

    char message[100];
    snprintf(message, sizeof(message), "%s", text);
    printWithDelay(message, 45);
    waitForMilliseconds(50);
    fflush(stdout);
    for (int i = 0; i < 3; i++)
    {
        waitForMilliseconds(500);
        printf(".");
        fflush(stdout);
    }
    printf("\n");
}

// Memory Allocation
int **allocate()
{
    int **grid = (int **)malloc(10 * sizeof(int *));
    for (int i = 0; i < 10; i++)
    {
        grid[i] = (int *)malloc(10 * sizeof(int));
    }
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            grid[i][j] = 0;
        }
    }
    return grid;
}

void fire(int **grid, int ships[], int row, int col, struct player *attacker)
{
    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;
    waitForMilliseconds(500); // Simulate time for action

    if (row < 0 || row >= 10 || col < 0 || col >= 10)
    {
        printWithDelay("Invalid coordinates! You lose your turn :(\n", 25);
        return;
    }

    if (grid[row][col] == HIT || grid[row][col] == MISS)
    {
        printWithDelay("You already fired at this location!\n", 25);
        return;
    }

    if (grid[row][col] == WATER)
    {
        grid[row][col] = MISS;
        printWithDelay("Miss!\n", 25);
        return;
    }

    int shipID = grid[row][col];
    if (shipID > 10)
    {
        shipID /= 10;
    }
    // Validate shipID to avoid out-of-bounds access
    if (shipID < 2 || shipID > 5)
    {
        printWithDelay("Invalid ship ID! Something went wrong.\n", 25);
        return;
    }

    // Decrement ship health
    ships[shipID]--;
    grid[row][col] = HIT;

    printWithDelay("Hit!\n", 25);

    // Check if the ship is sunk
    if (ships[shipID] == 0)
    {
        char message[50];
        snprintf(message, sizeof(message), "You sunk the opponent's %s!\n",
                 shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                         : shipID == 4   ? "Battleship"
                                                         : "Carrier");
        printWithDelay(message, 25);

        attacker->shipsSunk++;           // Track the number of ships sunk
        attacker->availableScreens++;    // Gain one smoke screen per ship sunk
        attacker->artilleryNextTurn = 1; // Unlock artillery for the next turn

        // Unlock torpedo if 3 ships are sunk
        if (attacker->shipsSunk == 3)
        {
            attacker->torpedoAvailable = 1;
            printWithDelay("Torpedo unlocked!\n", 25);
        }
    }
}

void radar_sweep(int **grid, int row, int col, struct player *attacker)
{
    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;
    waitForMilliseconds(500);

    if (attacker->radarCount <= 0)
    {
        printWithDelay("No radar sweeps remaining! You lose your turn.\n", 25);
        return;
    }

    if (row < 0 || row >= 9 || col < 0 || col >= 9)
    {
        printWithDelay("Invalid coordinates! You lose your turn.\n", 25);
        return;
    }

    int found = 0;
    for (int i = row; i < row + 2; i++)
    {
        for (int j = col; j < col + 2; j++)
        {
            int cell = grid[i][j];

            // Check if the cell is under a smoke screen (values multiplied by 10)
            if (cell % 10 == 0)
                continue; // Skip smoke-screened cells

            if (cell >= 2 && cell <= 5) // A ship part is detected
            {
                found = 1;
            }
        }
    }

    if (found)
    {
        printWithDelay("Enemy ships found!\n", 25);
    }
    else
    {
        printWithDelay("No enemy ships found.\n", 25);
    }

    attacker->radarCount--; // Decrease radar count
}

void smoke_screen(int **grid, int row, int col, struct player *attacker)
{
    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;
    waitForMilliseconds(500);

    // Check if smoke screens are available
    if (attacker->availableScreens <= 0)
    {
        printWithDelay("No smoke screens available! You lose your turn.\n", 25);
        return;
    }

    if (row < 0 || row >= 9 || col < 0 || col >= 9)
    {
        printWithDelay("Invalid coordinates! You lose your turn.\n", 25);
        return;
    }

    // Deploy smoke screen over a 2x2 area
    for (int i = row; i < row + 2; i++)
    {
        for (int j = col; j < col + 2; j++)
        {
            if (grid[i][j] != HIT && grid[i][j] != MISS)
            {
                grid[i][j] *= 10; // Mark the cell as protected by smoke
            }
        }
    }

    // Decrease the available smoke screens count
    attacker->availableScreens--;

    clear_terminal(); // Clear the screen to hide the move
    printWithDelay("Smoke screen deployed! Your move is hidden.\n", 25);
}

void artillery(int **grid, int row, int col, struct player *opponent, struct player *attacker)
{
    waitForMilliseconds(500); // Simulate artillery firing
    // Check if Artillery is available only for the next turn
    if (!attacker->artilleryNextTurn)
    {
        printWithDelay("Artillery not available! It was only available for the next turn after sinking a ship.\n", 25);
        return;
    }

    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;
    if (row < 0 || row >= 9 || col < 0 || col >= 9)
    {
        printWithDelay("Invalid coordinates! You lose your turn.\n", 25);
        return;
    }

    int hit = 0;
    int *ships = opponent->ships;

    for (int i = row; i < row + 2; i++)
    {
        for (int j = col; j < col + 2; j++)
        {
            if (grid[i][j] > 0)
            { // A ship part is found
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
                    snprintf(message, sizeof(message), "You sunk the opponent's %s!\n",
                             shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                                     : shipID == 4   ? "Battleship"
                                                                     : "Carrier");
                    printWithDelay(message, 25);

                    opponent->shipsRemaining--; // Decrease ships
                    attacker->shipsSunk++;      // Increase attacker's sunk ships
                    attacker->artilleryNextTurn = 1;
                    // Gain one smoke screen per ship sunk
                    attacker->availableScreens++;

                    // Unlock Torpedo if this was the 3rd ship sunk
                    if (attacker->shipsSunk == 3)
                    {
                        attacker->torpedoAvailable = 1;
                        printWithDelay("Torpedo unlocked!\n", 25);
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
        printWithDelay("Artillery hit!\n", 25);
    }
    else
    {
        printWithDelay("Artillery missed!\n", 25);
    }
}

void torpedo(int **grid, char type, int index, struct player *attacker, struct player *opponent)
{
    waitForMilliseconds(500);

    if (!attacker->torpedoAvailable)
    {
        printWithDelay("Torpedo not available! You can use torpedo right after sinking 3 ships \n", 25);
        return;
    }
    attacker->torpedoAvailable = 0;
    attacker->artilleryNextTurn = 0;

    if (index < 0 || index >= 10)
    {
        printWithDelay("Invalid index! You lose your turn.\n", 25);
        return;
    }

    int hit = 0;

    if (type == 'R' || type == 'r') // Row attack
    {
        for (int j = 0; j < 10; j++)
        {
            if (grid[index][j] > 0) // Ship part found
            {
                int shipID = grid[index][j];
                if (shipID > 10)
                {
                    shipID /= 10;
                }
                opponent->ships[shipID]--;
                grid[index][j] = HIT;
                hit = 1;

                if (opponent->ships[shipID] == 0) // Ship is sunk
                {
                    char message[50];
                    snprintf(message, sizeof(message), "You sunk the opponent's %s!\n",
                             shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                                     : shipID == 4   ? "Battleship"
                                                                     : "Carrier");
                    printWithDelay(message, 25);
                    opponent->shipsRemaining--;
                    attacker->shipsSunk++;
                    attacker->availableScreens++;
                }
            }
            else if (grid[index][j] == WATER)
            {
                grid[index][j] = MISS;
            }
        }
    }
    else if (type == 'C' || type == 'c') // Column attack
    {
        for (int i = 0; i < 10; i++)
        {
            if (grid[i][index] > 0) // Ship part found
            {
                int shipID = grid[i][index];
                if (shipID > 10)
                {
                    shipID /= 10;
                }
                opponent->ships[shipID]--;
                grid[i][index] = HIT;
                hit = 1;

                if (opponent->ships[shipID] == 0) // Ship is sunk
                {
                    char message[50];
                    snprintf(message, sizeof(message), "You sunk the opponent's %s!\n",
                             shipID == 2 ? "Submarine" : shipID == 3 ? "Destroyer"
                                                     : shipID == 4   ? "Battleship"
                                                                     : "Carrier");
                    printWithDelay(message, 25);
                    opponent->shipsRemaining--;
                    attacker->shipsSunk++;
                    attacker->availableScreens++;
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
        printWithDelay("Torpedo hit!\n", 25);
    }
    else
    {
        printWithDelay("Torpedo missed!\n", 25);
    }
}

void printGrid(int **grid, int isOwner)
{
    printf("   ");
    for (int i = 0; i < 10; i++)
    {
        printf("%c ", 'A' + i);
    }
    printf("\n");
    for (int i = 0; i < 10; i++)
    {
        waitForMilliseconds(50); // Subtle delay between rows
        if (i < 9)
        {
            printf("%d  ", i + 1);
        }
        else
        {
            printf("%d ", i + 1);
        }
        for (int j = 0; j < 10; j++)
        {
            int cellValue = grid[i][j];
            if (cellValue == 0)
            {
                printf(BLUE "~ " RESET_COLOR);
            }
            else if (cellValue > 0)
            {
                if (isOwner)
                {
                    printf(GREEN "# " RESET_COLOR);
                }
                else
                {
                    printf(BLUE "~ " RESET_COLOR);
                }
            }
            else if (cellValue == MISS)
            {
                printf(YELLOW "O " RESET_COLOR);
            }
            else if (cellValue == HIT)
            {
                printf(RED "X " RESET_COLOR);
            }
            else
            {
                // For smoke screens or other unknown values
                printf(BLUE "~ " RESET_COLOR);
            }
        }
        printf("\n");
    }
}

void DisplayRules()
{
    printWithDelay("Below are the rules of the game:\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t1. Each player has a 10X10 grid for them to place ships on\n", 25);
    waitForMilliseconds(300);
    printGrid(allocate(), 1);
    waitForMilliseconds(300);
    printWithDelay("\t2. Each player has 4 ships in their fleet of increasing sizes (2 to 5)\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t3. You can place ships anywhere on the grid as long as they do not overlap\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t4. During each turn the player should choose one of these moves:\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t\ta. Fire: hits a certain coordinate on the enemy grid\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t\tb. Smoke Screen: protects a 2x2 area from the Radar Sweep (you earn one per ship sunk)\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t\tc. Radar Sweep: scans a 2x2 area for enemy ships (you are allowed ONLY 3 moves)\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t\td. Artillery: attacks enemy ships on a 2x2 area (allowed ONLY AFTER you sink an enemy's ship)\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t\te. Torpedo: attacks enemy ships on an entire row or column (allowed ONLY AFTER you sink the THIRD enemy ship)\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t5. For coordinate moves, always provide the X followed by the Y ( (0,0) is the top left corner of the grid)\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t6. For 2x2 area moves, always provide the top left corner coordinate\n", 25);
    waitForMilliseconds(300);
    printWithDelay("\t7. Please note that any invalid coordinates will automatically skip your turn\n", 25);
}

int addToGrid(int **grid, char x, int y, char orientation, int size)
{
    if (x < 'A' || x > 'J' || y < 1 || y > 10 || !(orientation == 'V' || orientation == 'H'))
    {
        printWithDelay("Input Error\n", 25);
        return 0;
    }

    int col = x - 'A';
    int row = y - 1;

    if (orientation == 'V')
    {
        if (row + size > 10)
        {
            printWithDelay("Out of bounds Error\n", 25);
            return 0;
        }
        for (int i = 0; i < size; i++)
        {
            if (grid[row + i][col] != 0)
            {
                printWithDelay("Overlap Error\n", 25);
                return 0;
            }
        }
        for (int i = 0; i < size; i++)
        {
            grid[row + i][col] = size;
        }
    }
    else if (orientation == 'H')
    {
        if (col + size > 10)
        {
            printWithDelay("Out of bounds Error\n", 25);
            return 0;
        }
        for (int i = 0; i < size; i++)
        {
            if (grid[row][col + i] != 0)
            {
                printWithDelay("Overlap Error\n", 25);
                return 0;
            }
        }
        for (int i = 0; i < size; i++)
        {
            grid[row][col + i] = size;
        }
    }
    return 1;
}

void gridsetup(struct player *p)
{
    char *names[6] = {"", "", "Submarine", "Destroyer", "Battleship", "Carrier"};
    for (int i = 5; i >= 2; i--)
    {
        char X;
        int Y;
        char HV;
        printGrid(p->grid, 1);
        char prompt[100];
        snprintf(prompt, sizeof(prompt), "Where do you want to place the %s (size %d)?\n", names[i], i);
        printWithDelay(prompt, 25);
        if (scanf(" %c %d %c", &X, &Y, &HV) != 3)
        {
            while (getchar() != '\n')
                ;
            clear_terminal();
            printWithDelay("Invalid Input\n", 25);
            i++;
            continue;
        }
        clear_terminal();
        if (!addToGrid(p->grid, X, Y, HV, i))
        {
            i++;
        }
    }
}

int didLose(struct player *plr)
{
    for (int i = 2; i <= 5; i++)
    {
        if (plr->ships[i] > 0)
        {
            return 0;
        }
    }
    return 1;
}

void getAndPerformMove(struct player *player1, struct player *player2, int turn)
{
    while (1)
    {
        int choice;
        char X;
        int Y;
        char HV;
        struct player *currentPlayer = turn ? player1 : player2;
        struct player *opponentPlayer = turn ? player2 : player1;
        char prompt[100];
        snprintf(prompt, sizeof(prompt), "%s, pick a move (1: Fire/ 2: Smoke Screen/ 3: Radar Sweep/ 4: Artillery/ 5: Torpedo)\n", currentPlayer->name);
        printWithDelay(prompt, 25);
        if (scanf(" %d", &choice) != 1)
        {
            printWithDelay("Invalid input\n", 25);
            // Clear the input buffer
            while (getchar() != '\n')
                ; // Discards the remaining characters until newline

            continue;
        }
        clear_terminal();
        if (choice == 5)
        {
            printGrid(opponentPlayer->grid, 0);
            printWithDelay("Pick your Coordinates and Type (R for row/C for column): \n", 25);
            if (scanf(" %c %d %c", &X, &Y, &HV) != 3)
            {
                printWithDelay("Invalid input\n", 25);
                while (getchar() != '\n')
                    ;
                continue;
            }
            int index = (HV == 'R' || HV == 'r') ? Y - 1 : X - 'A';
            while (getchar() != '\n')
                ;
            torpedo(opponentPlayer->grid, HV, index, currentPlayer, opponentPlayer);
        }
        else if (choice == 4)
        {
            printGrid(opponentPlayer->grid, 0);
            printWithDelay("Pick your Coordinates (e.g., B 3): \n", 25);
            if (scanf(" %c %d", &X, &Y) != 2)
            {
                printWithDelay("Invalid input\n", 25);
                while (getchar() != '\n')
                    ;
                continue;
            }
            while (getchar() != '\n')
                ;
            artillery(opponentPlayer->grid, Y - 1, X - 'A', opponentPlayer, currentPlayer);
        }
        else if (choice == 3)
        {
            printGrid(opponentPlayer->grid, 0);
            printWithDelay("Pick your Coordinates (e.g., B 3): \n", 25);
            if (scanf(" %c %d", &X, &Y) != 2)
            {
                printWithDelay("Invalid input\n", 25);
                while (getchar() != '\n')
                    ;
                continue;
            }
            while (getchar() != '\n')
                ;
            radar_sweep(opponentPlayer->grid, Y - 1, X - 'A', currentPlayer);
        }
        else if (choice == 2)
        {
            printGrid(currentPlayer->grid, 1);
            printWithDelay("Pick your Coordinates (e.g., B 3): \n", 25);
            if (scanf(" %c %d", &X, &Y) != 2)
            {
                printWithDelay("Invalid input\n", 25);
                while (getchar() != '\n')
                    ;
                continue;
            }
            while (getchar() != '\n')
                ;
            smoke_screen(currentPlayer->grid, Y - 1, X - 'A', currentPlayer);
        }
        else if (choice == 1)
        {
            printGrid(opponentPlayer->grid, 0);
            printWithDelay("Pick your Coordinates (e.g., B 3): \n", 25);
            if (scanf(" %c %d", &X, &Y) != 2)
            {
                printWithDelay("Invalid input\n", 25);
                while (getchar() != '\n')
                    ;
                continue;
            }
            while (getchar() != '\n')
                ;

            fire(opponentPlayer->grid, opponentPlayer->ships, Y - 1, X - 'A', currentPlayer);
        }
        else
        {
            printWithDelay("Invalid Input\n", 25);
            continue;
        }
        break;
    }
}

void StartLocalGame()
{
    char *ptrname1 = (char *)malloc(sizeof(char) * 21);
    printWithDelay("Player 1, please enter your name (0 - 20 characters)\n", 25);
    scanf("%20s", ptrname1);
    char welcomeMsg[200];
    snprintf(welcomeMsg, sizeof(welcomeMsg), "Welcome to the game, %s!\n", ptrname1);
    printWithDelay(welcomeMsg, 25);
    waitForMilliseconds(500);

    char *ptrname2 = (char *)malloc(sizeof(char) * 21);
    printWithDelay("Player 2, please enter your name (0 - 20 characters)\n", 25);
    scanf("%20s", ptrname2);
    snprintf(welcomeMsg, sizeof(welcomeMsg), "Welcome to the game, %s!\n", ptrname2);
    printWithDelay(welcomeMsg, 25);
    waitForMilliseconds(500);

    struct player player1 = {ptrname1, {0, 0, 2, 3, 4, 5}, 4, allocate(), 3, 0, 0, 0, 0};
    struct player player2 = {ptrname2, {0, 0, 2, 3, 4, 5}, 4, allocate(), 3, 0, 0, 0, 0};

    // PLAYER 1 SHIP PLACEMENT
    clear_terminal();
    snprintf(welcomeMsg, sizeof(welcomeMsg), "Welcome %s.\nNow you have to place your ships on this 10X10 grid using this coordinate system:\n", ptrname1);
    printWithDelay(welcomeMsg, 25);
    printWithDelay("For placing ships, choose X Y H/V (e.g., B 3 H for cell B3 horizontally)\n", 25);
    waitForMilliseconds(500);
    gridsetup(&player1);

    clear_terminal();
    // PLAYER 2 SHIP PLACEMENT
    snprintf(welcomeMsg, sizeof(welcomeMsg), "Welcome %s.\nNow you have to place your ships on this 10X10 grid using this coordinate system:\n", ptrname2);
    printWithDelay(welcomeMsg, 25);
    printWithDelay("For placing ships, choose X Y H/V (e.g., B 3 H for cell B3 horizontally)\n", 25);
    waitForMilliseconds(500);
    gridsetup(&player2);
    clear_terminal();

    printWithDelay("Now let the games begin!\n", 25);
    waitForMilliseconds(500);

    // Randomly select the starting player
    int turn = rand() % 2; // turn will be 0 or 1

    if (turn)
    {
        snprintf(welcomeMsg, sizeof(welcomeMsg), "%s will start the game.\n", player1.name);
    }
    else
    {
        snprintf(welcomeMsg, sizeof(welcomeMsg), "%s will start the game.\n", player2.name);
    }
    printWithDelay(welcomeMsg, 25);
    waitForMilliseconds(500);

    while (!didLose(&player1) && !didLose(&player2))
    {
        getAndPerformMove(&player1, &player2, turn);
        printWithDelay("Press Enter to end your turn...", 25);
        getchar();
        clear_terminal();
        turn = ((turn + 1) % 2);
    }
    char congratsMsg[50];
    snprintf(congratsMsg, sizeof(congratsMsg), "Congrats %s! You won!\n", didLose(&player1) ? player2.name : player1.name);
    printWithDelay(congratsMsg, 25);
    printWithDelay("Thank you for playing!\n", 25);

    free(ptrname1);
    free(ptrname2);
    for (int i = 0; i < 10; i++)
    {
        free(player1.grid[i]);
        free(player2.grid[i]);
    }
    free(player1.grid);
    free(player2.grid);
}

void StartBotGame()
{
    // Step 1: Get the player's name
    char *ptrname1 = (char *)malloc(sizeof(char) * 21);
    printWithDelay("Player 1, please enter your name (0 - 20 characters): ", 25);
    scanf("%20s", ptrname1);
    char welcomeMsg[200];
    snprintf(welcomeMsg, sizeof(welcomeMsg), "Welcome to the game, %s!\n", ptrname1);
    printWithDelay(welcomeMsg, 25);
    waitForMilliseconds(500);

    // Step 2: Generate a name for the bot
    char *botName = generateBotName();
    loadingAnimation("Matchmaking");
    waitForMilliseconds(500);
    snprintf(welcomeMsg, sizeof(welcomeMsg), "%s will be your opponent!\n", botName);
    printWithDelay(welcomeMsg, 25);

    // Step 3: Initialize player and bot
    struct player player1 = {ptrname1, {0, 0, 2, 3, 4, 5}, 4, allocate(), 3, 0, 0, 0, 0};
    struct player player2 = {botName, {0, 0, 2, 3, 4, 5}, 4, allocate(), 3, 0, 0, 0, 0};
    getchar();
    printWithDelay("Press any key to start the game...", 25);
    getchar();
    // Step 4: Player places their ships
    clear_terminal();
    snprintf(welcomeMsg, sizeof(welcomeMsg), "Welcome %s.\nNow you have to place your ships on this 10x10 grid.\n", ptrname1);
    printWithDelay(welcomeMsg, 25);
    printWithDelay("For placing ships, choose X Y H/V (e.g., B 3 H for cell B3 horizontally)\n", 25);
    waitForMilliseconds(500);
    gridsetup(&player1); // Player ship placement

    // Step 5: Bot places its ships
    clear_terminal();
    waitForMilliseconds(500);
    snprintf(welcomeMsg, sizeof(welcomeMsg), "%s is placing the ships.\n", botName);
    printWithDelay(welcomeMsg, 25);
    waitForMilliseconds(750);
    loadingAnimation("Placing Ships");
    placeShips(&player2); // Bot ship placement
    clear_terminal();

    // Step 6: Announce game start
    printWithDelay("Now let the games begin!\n", 25);
    waitForMilliseconds(500);

    // Step 7: Randomly choose who starts the game
    int turn = rand() % 2; // Randomly determine who goes first

    if (turn)
    {
        snprintf(welcomeMsg, sizeof(welcomeMsg), "%s will start the game.\n", player1.name);
    }
    else
    {
        snprintf(welcomeMsg, sizeof(welcomeMsg), "%s will start the game.\n", player2.name);
    }
    printWithDelay(welcomeMsg, 25);
    waitForMilliseconds(500);

    // Step 8: Main game loop
    while (!didLose(&player1) && !didLose(&player2))
    {
        if (turn == 1)
        {
            // Player's move
            printGrid(player1.grid, 1);
            getAndPerformMove(&player1, &player2, turn);
            printWithDelay("Press Enter to end your turn...", 25);
            getchar();
        }
        else
        {
            // Bot's move
            performBotMove(&player1, &player2);
        }
        clear_terminal();        // Clear screen for next turn
        turn = ((turn + 1) % 2); // Alternate turns
    }

    // Step 9: End the game and announce the winner
    char congratsMsg[50];
    if (didLose(&player1))
    {
        snprintf(congratsMsg, sizeof(congratsMsg), "You Lost!\n");
    }
    else
    {
        snprintf(congratsMsg, sizeof(congratsMsg), "Congrats! You won!\n");
    }
    printWithDelay(congratsMsg, 25);
    printWithDelay("Thank you for playing!\n", 25);

    // Step 10: Clean up dynamically allocated memory
    free(botName);
    free(ptrname1);
    for (int i = 0; i < 10; i++)
    {
        free(player1.grid[i]);
        free(player2.grid[i]);
    }
    free(player1.grid);
    free(player2.grid);
}

void InitializeGame()
{
    printWithDelay("Welcome To BattleShip \n\n", 25);
    char choice;
    char secondchoice;
    loadingAnimation("Loading");
    printWithDelay("Main Menu:\nPlay Game (Enter 0)\nGame Rules (Enter 1)\nExit Game (Enter 2)\n", 25);
    scanf(" %c", &choice);
    if (choice == '0')
    {
        waitForMilliseconds(300);
        printWithDelay("Local Game (Enter 0)\n", 25);
        waitForMilliseconds(200);
        printWithDelay("Vs CPU Game (Enter 1)\n", 25);
        waitForMilliseconds(200);
        printWithDelay("Back (Enter b)\n", 25);
        scanf(" %c", &secondchoice);
        if (secondchoice == '0')
        {
            StartLocalGame();
            return;
        }
        else if (secondchoice == '1')
        {
            StartBotGame();
            return;
        }
        else if (secondchoice == 'b')
        {
            InitializeGame();
            return;
        }
        else
        {
            printWithDelay("Invalid Choice, choose either 0 or b\n", 25);
            InitializeGame();
            return;
        }
    }
    else if (choice == '1')
    {
        DisplayRules();
        printf("\n");
        InitializeGame();
        return;
    }
    else if (choice == '2')
    {
        printWithDelay("Exiting the game. Goodbye!\n", 25);
        return;
    }
    else
    {
        printWithDelay("Invalid Choice, choose either 0 or 1 or 2\n", 25);
        InitializeGame();
        return;
    }
}

int main()
{
    srand(time(NULL));
    InitializeGame();
    return 0;
}
