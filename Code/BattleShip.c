#include <stdio.h>
#include <stdlib.h>

#define WATER 0
#define HIT -1
#define MISS -2
void DisplayRules()

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

// Accurate wait function in milliseconds
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

void clear_terminal()
{
#ifdef _WIN32
    system("cls"); // Windows
#else
    printf("\033[2J\033[H");
#endif
}

void loadingAnimation()
{
    const char *loadingText = "Loading";
    printf("%s", loadingText);
    fflush(stdout);
    for (int i = 0; i < 3; i++)
    {
        waitForMilliseconds(500);
        printf(".");
        fflush(stdout);
    }
    printf("\n");
}

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

struct player
{
    char *name;
    int ships[6];
    int shipsRemaining;
    int **grid;
    int radarCount; //Starts at 3
    int availableScreens; //Increase by 1 per ship sunk
    int shipsSunk;
    int artilleryAvailable;
    int artilleryNextTurn;
    int torpedoAvailable;
};


void fire(int **grid, int ships[], int row, int col, struct player *attacker) 
{
    if (row < 0 || row >= 10 || col < 0 || col >= 10) {
        printf("Invalid coordinates! You lose your turn :(\n");
        return;
    }

    if (grid[row][col] == HIT || grid[row][col] == MISS) {
        printf("You already fired at this location!\n");
        return;
    }

    if (grid[row][col] == WATER) {
        grid[row][col] = MISS;  
        printf("Miss!\n");
        return;
    }

    int shipID = grid[row][col];  
    ships[shipID]--;  // Decrease health of the ship
    grid[row][col] = HIT;  // Mark as HIT

    printf("Hit!\n");

    if (ships[shipID] == 0) {
        printf("You sunk the opponent's %s!\n", 
               shipID == 2 ? "Submarine" : 
               shipID == 3 ? "Destroyer" : 
               shipID == 4 ? "Battleship" : "Carrier");

        attacker->shipsSunk++;  // Track ships sunk

        // Unlock Artillery for the next turn
        attacker->artilleryNextTurn = 1;

        // Unlock Torpedo if 3 ships are sunk
        if (attacker->shipsSunk == 3) {
            attacker->torpedoAvailable = 1;
            printf("Torpedo unlocked!\n");
        }
    }
}

void radar_sweep(int **grid, int row, int col, int *radarCount) 
{
    if (*radarCount <= 0) {
        printf("No radar sweeps remaining! You lose your turn.\n");
        return;
    }

    if (row < 0 || row >= 9 || col < 0 || col >= 9) {
        printf("Invalid coordinates! You lose your turn.\n");
        return;
    }

    int found = 0;
    for (int i = row; i < row + 2; i++) {
        for (int j = col; j < col + 2; j++) {
            int cell = grid[i][j];
            if (cell >= 2 && cell <= 5) {  // Check for a ship
                found = 1;
            }
        }
    }

    if (found) {
        printf("Enemy ships found!\n");
    } else {
        printf("No enemy ships found.\n");
    }
    (*radarCount)--;
}

void smoke_screen(int **grid, int row, int col, int *availableScreens) 
{
    if (*availableScreens <= 0) {
        printf("No smoke screens available! You lose your turn.\n");
        return;
    }

    if (row < 0 || row >= 9 || col < 0 || col >= 9) {
        printf("Invalid coordinates! You lose your turn.\n");
        return;
    }

    for (int i = row; i < row + 2; i++) {
        for (int j = col; j < col + 2; j++) {
            grid[i][j] *= 10;  // Mark as protected
        }
    }

    (*availableScreens)--;
    system("clear || cls"); //Clear screen
    printf("Smoke screen deployed! Your move is hidden.\n");
}

void artillery(int **grid, int row, int col, struct player *opponent, struct player *attacker) 
{
    // Check if Artillery is available only for the next turn
    if (!attacker->artilleryNextTurn) {
        printf("Artillery not available! It was only available for the next turn after sinking a ship.\n");
        return;
    }

    if (row < 0 || row >= 9 || col < 0 || col >= 9) {
        printf("Invalid coordinates! You lose your turn.\n");
        return;
    }

    int hit = 0; 
    int *ships = opponent->ships;  

    for (int i = row; i < row + 2; i++) {
        for (int j = col; j < col + 2; j++) {
            if (grid[i][j] > 0) {  // A ship part is found
                int shipID = grid[i][j];  
                ships[shipID]--;  
                grid[i][j] = HIT;  
                hit = 1;

                if (ships[shipID] == 0) {
                    printf("You sunk the opponent's %s!\n", 
                           shipID == 2 ? "Submarine" : 
                           shipID == 3 ? "Destroyer" : 
                           shipID == 4 ? "Battleship" : "Carrier");

                    opponent->shipsRemaining--;  // Decrease ships 
                    attacker->shipsSunk++;  // Increase  attacker's sunk ships

                    // Unlock Torpedo if this was the 3rd ship sunk
                    if (attacker->shipsSunk == 3) {
                        attacker->torpedoAvailable = 1;
                        printf("Torpedo unlocked!\n");
                    }
                }
            } else if (grid[i][j] == WATER) {
                grid[i][j] = MISS;  
            }
        }
    }

    if (hit) {
        printf("Artillery hit!\n");
    } else {
        printf("Artillery missed!\n");
    }
    // Reset the Artillery availability
    attacker->artilleryNextTurn = 0;
}


void torpedo(int **grid, char type, int index, struct player *attacker) 
{
    if (!attacker->torpedoAvailable) {
        printf("Torpedo not available yet! You must sink three ships first.\n");
        return;
    }

    if (index < 0 || index >= 10) {
        printf("Invalid index! You lose your turn.\n");
        return;
    }

    int hit = 0;

    if (type == 'R' || type == 'r') {
        for (int j = 0; j < 10; j++) {
            if (grid[index][j] > 0) {
                grid[index][j] = HIT;
                hit = 1;
            } else if (grid[index][j] == WATER) {
                grid[index][j] = MISS;
            }
        }
    } else if (type == 'C' || type == 'c') {
        for (int i = 0; i < 10; i++) {
            if (grid[i][index] > 0) {
                grid[i][index] = HIT;
                hit = 1;
            } else if (grid[i][index] == WATER) {
                grid[i][index] = MISS;
            }
        }
    } else {
        printf("Invalid type! Use 'R' for row or 'C' for column.\n");
        return;
    }

    if (hit) {
        printf("Torpedo hit!\n");
    } else {
        printf("Torpedo missed!\n");
    }

    attacker->torpedoAvailable = 0;  // Reset after use
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
            if (grid[i][j] == 0)
            {
                printf(BLUE "~ " RESET_COLOR);
            }
            else if (grid[i][j] > 0)
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
            else if (grid[i][j] == -1)
            {
                printf(YELLOW "O " RESET_COLOR);
            }
            else if (grid[i][j] < -1)
            {
                printf(RED "X " RESET_COLOR);
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
    printWithDelay("\t\tb. Smoke Screen: protects a 2x2 area from the Radar Sweep\n", 25);
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
        printf("Input Error \n");
        return 0;
    }

    int col = x - 'A';
    int row = y - 1;

    if (orientation == 'V')
    {
        if (row + size > 10)
        {
            printf("Out of bounds Error \n");
            return 0;
        }
        for (int i = 0; i < size; i++)
        {
            if (grid[row + i][col] != 0)
            {
                printf("Overlap Error \n");
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
            printf("Out of bounds Error \n");
            return 0;
        }
        for (int i = 0; i < size; i++)
        {
            if (grid[row][col + i] != 0)
            {
                printf("Overlap Error \n");
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
    char *names[6] = {"", "", "submarine", "destroyer", "battleship", "carrier"};
    for (int i = 5; i >= 2; i--)
    {
        char X;
        int Y;
        char HV;
        printGrid(p->grid, 1);
        printf("Where do you want to place the %s (size %d)?\n", names[i], i);
        if (scanf(" %c %d %c", &X, &Y, &HV) != 3)
        {
            while (getchar() != '\n')
                ;
            clear_terminal();
            printf("Invalid Input \n ");
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

void getAndPerformMove(struct player player1, struct player player2, int turn)
{
    while (1)
    {
        int choice;
        char X;
        int Y;
        char HV;
        printf("%s pick a move (1: Fire/ 2: SmokeScreen/ 3: RadarSweep/ 4: Artillery/ 5: Torpedo) \n", turn ? player1.name : player2.name);
        if (scanf(" %d", &choice) != 1)
        {
            printf("Invalid input \n");
            // Clear the input buffer
            while (getchar() != '\n')
                ; // Discards the remaining characters until newline

            continue;
        }
        if (choice == 5)
        {
            printGrid(turn ? player2.grid : player1.grid, 0);
            printf("Pick your Coordinates and Orientation: \n");
            scanf(" %c %d %c", &X, &Y, &HV);
            // torpedo(player2.grid, y - 1, (int) (X - 'A'), HV);
        }
        else if (choice == 4)
        {
            printGrid(turn ? player2.grid : player1.grid, 0);
            printf("Pick your Coordinates: \n");
            scanf(" %c %d", &X, &Y);
            // artillery(player2.grid, y - 1, (int) (X - 'A'));
        }
        else if (choice == 3)
        {
            printGrid(turn ? player2.grid : player1.grid, 0);
            printf("Pick your Coordinates: \n");
            scanf(" %c %d", &X, &Y);
            // radar_sweep(player2.grid, y -1, (int) (X - 'A'));
        }
        else if (choice == 2)
        {
            printGrid(turn ? player1.grid : player2.grid, 1);
            printf("Pick your Coordinates: \n");
            scanf(" %c %d", &X, &Y);
            // smoke_screen(player1.grid, y -1, (int) (X - 'A'));
        }
        else if (choice == 1)
        {
            printGrid(turn ? player2.grid : player1.grid, 0);
            printf("Pick your Coordinates: \n");
            scanf(" %c %d", &X, &Y);
            // fire(player2.grid, y -1, (int)(X - 'A'));
        }
        else
        {
            printf("Invalid Input \n");
            continue;
        }
        break;
    }
}

void StartLocalGame()
{
    char *ptrname1 = (char *)malloc(sizeof(char) * 20);
    printf("Please enter your name (0 - 20 characters)\n");
    scanf("%19s", ptrname1);
    printf("Welcome to the game %s!\n", ptrname1);
    char *ptrname2 = (char *)malloc(sizeof(char) * 20);
    printf("Please enter your name (0 - 20 characters)\n");
    scanf("%19s", ptrname2);
    printf("Welcome to the game %s!\n", ptrname2);

    struct player player1 = {ptrname1, {0, 0, 2, 3, 4, 5}, 4, allocate()};
    struct player player2 = {ptrname2, {0, 0, 2, 3, 4, 5}, 4, allocate()};

    // PLAYER 1 SHIP PLACEMENT
    printf("Welcome %s.\nNow you have to place your ships on this 10X10 grid using this coordinate system:\n", ptrname1);
    printf("For placing ships, choose X Y H/V (e.g., B 3 H for cell B3 horizontally)\n");
    gridsetup(&player1);

    clear_terminal();
    // PLAYER 2 SHIP PLACEMENT
    printf("Welcome %s.\nNow you have to place your ships on this 10X10 grid using this coordinate system:\n", ptrname2);
    printf("For placing ships, choose X Y H/V (e.g., B 3 H for cell B3 horizontally)\n");
    gridsetup(&player2);
    clear_terminal();

    printf("Now let the games begin \n");
    int turn = 1;
    while (player1.shipsRemaining > 0 && player2.shipsRemaining > 0)
    {
        getAndPerformMove(player1, player2, turn);
        clear_terminal();
        turn = ((turn + 1) % 2);
    }
    printf("Congrats %s You Won! \n", player1.shipsRemaining == 0 ? player2.name : player1.name);
    printf("Thank You For Playing\n");
}

void InitializeGame()
{
    printf("Welcome To BattleShip (But the better one)\n");
    printf("\n");
    char choice;
    char secondchoice;
    loadingAnimation();
    printWithDelay("Main Menu:\nPlay Game (Enter 0)\nGame Rules (Enter 1)\nExit Game (Enter 2)\n", 25);
    scanf(" %c", &choice); // Added space before %c
    if (choice == '0')
    {
        waitForMilliseconds(300);
        printWithDelay("Local Game (Enter 0)\n", 25);
        waitForMilliseconds(200);
        printWithDelay("Vs CPU Game (Coming Soon...)\n", 25); // Teaser for phase 2
        waitForMilliseconds(200);
        printWithDelay("Back (Enter b)\n", 25);
        scanf(" %c", &secondchoice);
        if (secondchoice == '0')
        {
            StartLocalGame();
            return;
        }
        else if (secondchoice == 'b')
        {
            InitializeGame();
            return;
        }
        else
        {
            printf("Invalid Choice, choose either 0 or b\n");
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
        return;
    }
    else
    {
        printf("Invalid Choice, choose either 0 or 1 or 2 \n");
        InitializeGame();
        return;
    }
}

int main()
{
    InitializeGame();
    return 0;
}
