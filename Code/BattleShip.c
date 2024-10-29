#include <stdio.h>
#include <stdlib.h>

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
};

int fire(int grid[10][10], int row, int col)
{
    return 0;
}

int radar_sweep(int grid[10][10], int row, int col)
{
    return 0;
}

void smoke_screen(int grid[10][10], int row, int col)
{
}

int artillery(int grid[10][10], int row, int col)
{
    return 0;
}

int torpedo(int grid[10][10], char type, int index)
{
    return 0;
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
        for (int j = -1; j < 10; j++)
        {
            if (j == -1)
            {
                if (i < 9)
                {
                    printf("%d  ", i + 1);
                }
                else
                {
                    printf("%d ", i + 1);
                }
                continue;
            }
            if (grid[i][j] == 0)
            {
                printf("~ ");
            }
            else if (grid[i][j] > 0)
            {
                if (isOwner)
                {
                    printf("# ");
                }
                else
                {
                    printf("~ ");
                }
            }
            else if (grid[i][j] == -1)
            {
                printf("O ");
            }
            else if (grid[i][j] < -1)
            {
                printf("X ");
            }
        }
        printf("\n");
    }
}

void DisplayRules()
{
    printf("Below are the rules of the game: \n");
    printf("\t1. Each player has a 10X10 grid for them to place ships on \n");
    printGrid(allocate(), 1);
    printf("\t2. Each player has 4 ships in their fleet of increasing sizes (2 to 5) \n");
    printf("\t3. You can place ships anywhere on the grid as long as they do not overlap\n");
    printf("\t4. During each turn the player should choose one of these moves: \n");
    printf("\t\ta. Fire: hits a certain coordinate on the enemy grid \n");
    printf("\t\tb. Smoke Screen: protects a 2x2 area from the Radar Sweep \n");
    printf("\t\tc. Radar Sweep: scans a 2x2 area for enemy ships (you are allowed ONLY 3 moves) \n");
    printf("\t\td. Artillery: attacks enemy ships on a 2x2 area (allowed ONLY AFTER you sink an enemy's ship) \n");
    printf("\t\te. Torpedo: attacks enemy ships on an entire row or column (allowed ONLY AFTER you sink the THIRD enemy ship) \n");
    printf("\t5. For coordinate moves, always provide the X followed by the Y ( (0,0) is the top left corner of the grid) \n");
    printf("\t6. For 2x2 area moves, always provide the top left corner coordinate \n");
    printf("\t7. Please note that any invalid coordinates will automatically skip your turn \n");
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

    char X;
    int Y;
    char HV;

    // PLAYER 1 SHIP PLACEMENT
    printf("Welcome %s. \nNow you have to place your ships on this 10X10 grid using this coordinate system:\n", ptrname1);
    printGrid(player1.grid, 1);
    printf("For placing ships, choose X Y H/V (e.g., B 3 H for cell B3 horizontally)\n");
    while (1)
    {
        printf("Where do you want to place the carrier (size 5)?\n");
        scanf(" %c %d %c", &X, &Y, &HV);
        if (addToGrid(player1.grid, X, Y, HV, 5))
        {
            break;
        }
    }
    printGrid(player1.grid, 1);

    while (1)
    {
        printf("Where do you want to place the battleship (size 4)?\n");
        scanf(" %c %d %c", &X, &Y, &HV);
        if (addToGrid(player1.grid, X, Y, HV, 4))
        {
            break;
        }
    }
    printGrid(player1.grid, 1);

    while (1)
    {
        printf("Where do you want to place the destroyer (size 3)?\n");
        scanf(" %c %d %c", &X, &Y, &HV);
        if (addToGrid(player1.grid, X, Y, HV, 3))
        {
            break;
        }
    }
    printGrid(player1.grid, 1);

    while (1)
    {
        printf("Where do you want to place the submarine (size 2)?\n");
        scanf(" %c %d %c", &X, &Y, &HV);
        if (addToGrid(player1.grid, X, Y, HV, 2))
        {
            break;
        }
    }
    printGrid(player1.grid, 1);
    printf("\n");

    // PLAYER 2 SHIP PLACEMENT
    printf("Welcome %s. \nNow you have to place your ships on this 10X10 grid using this coordinate system:\n", ptrname2);
    printGrid(player2.grid, 1);
    printf("For placing ships, choose X Y H/V (e.g., B 3 H for cell B3 horizontally)\n");
    while (1)
    {
        printf("Where do you want to place the carrier (size 5)?\n");
        scanf(" %c %d %c", &X, &Y, &HV);
        if (addToGrid(player2.grid, X, Y, HV, 5))
        {
            break;
        }
    }
    printGrid(player2.grid, 1);

    while (1)
    {
        printf("Where do you want to place the battleship (size 4)?\n");
        scanf(" %c %d %c", &X, &Y, &HV);
        if (addToGrid(player2.grid, X, Y, HV, 4))
        {
            break;
        }
    }
    printGrid(player2.grid, 1);

    while (1)
    {
        printf("Where do you want to place the destroyer (size 3)?\n");
        scanf(" %c %d %c", &X, &Y, &HV);
        if (addToGrid(player2.grid, X, Y, HV, 3))
        {
            break;
        }
    }
    printGrid(player2.grid, 1);

    while (1)
    {
        printf("Where do you want to place the submarine (size 2)?\n");
        scanf(" %c %d %c", &X, &Y, &HV);
        if (addToGrid(player2.grid, X, Y, HV, 2))
        {
            break;
        }
    }
    printGrid(player2.grid, 1);
    printf("\n");
}

void InitializeGame()
{
    printf("Welcome To BattleShip (But the better one)\n");
    printf("\n");
    char choice;
    char secondchoice;
    printf("Main Menu:\nPlay Game (Enter 0)\nGame Rules (Enter 1)\nExit Game (Enter 2)\n");
    scanf(" %c", &choice); // Added space before %c
    if (choice == '0')
    {
        printf("Local Game (Enter 0)\n");
        printf("Vs CPU Game (Coming Soon...)\n"); // Teaser for phase 2
        printf("Back (Enter b)\n");
        scanf(" %c", &secondchoice); // Added space before %c
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

    // Still have the game loop to finish and fix a bug in the coordinates
}

/*

while(true){
    *Display Grid*
    *Ask for a Move*
    *Do move (update the grid) (Phase 1.2)*
    if(PlayerOpponent.shipsRemaining == 0){
        return 1
    }
}

*/
