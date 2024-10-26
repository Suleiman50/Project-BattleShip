#include <stdio.h>
#include <stdlib.h>

void DisplayRules()
{
    printf("Below are the rules of the game: \n");
    printf("\t1. Each player has a 10X10 grid for them to place ships on \n");
    printf("\t2. Each player has 4 ships in their fleet of increasing sizes (2 to 5) \n");
    printf("\t3. You can ships anywhere on the grid as long as they do not overlap\n");
    printf("\t4. During each turn the player should choose one of these moves: \n");
    printf("\t\ta. Fire: hits a certain coordinate on the enemy grid \n");
    printf("\t\tb. Smoke Screen: protects a 2x2 area from the Radar Sweep \n");
    printf("\t\tc. Radar Sweep: scans a 2x2 area for enemy ships (you are allowed ONLY 3 moves) \n");
    printf("\t\td. Artillery: attacks enemy ships on a 2x2 area (allowed ONLY AFTER you sink an enemy's ship) \n");
    printf("\t\te. Torpedo: attacks enemy ships on an entire row or collumn (allowed ONLY AFTER you sink the THIRD enemy ship) \n");
    printf("\t5. For Coordinate moves area moves always provide the X followed by the Y ( (0,0) is the top left corner of the grid) \n");
    printf("\t6. For 2x2 area moves always provide the top left corner Coordinate \n");
    printf("\t7. Please note that any invalid Coordinates will automatically skip your turn \n");
}
void InitializeGame()
{
    printf("Welcome To BattleShip (But the better one)\n");
    printf("\n");
    char choice;
    char secondchoice;
    printf("Main Menu:\nPlay Game (Enter 0)\nGame Rules (Enter 1) \nExit Game (Enter 2)\n");
    scanf("%c", &choice);
    getchar();
    if (choice == '0')
    {
        printf("Local Game (Enter 0)\n");
        printf("Vs CPU Game (Coming Soon...)\n"); // teaser for phase 2
        printf("Back (Enter b)\n");
        scanf("%c", &secondchoice);
        getchar();
        if (secondchoice == '0')
        {
            // StartLocalGame();
            // We stopped here
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
void StartLocalGame()
{
}

int main()
{
    InitializeGame();
    return 0;
}

/*
void StartLocalGame();

Main:

Get input
Setup Grid

while(true){
    *Display Grid*
    *Ask for a Move*
    *Do move (update the grid) (Phase 1.2)*
    if(PlayerOpponent.shipsRemaining == 0){
        return 1
    }
}

Struct Player:
    Ship fleet[4]:
        ship(1)
        ship(2)
        ship(3)
        ship(4)

    int[][] grid
    int shipsRemaining

Struct Coordinate:
    int x
    int y

Struct ship:
Coorinates positions[]
int hits

*/