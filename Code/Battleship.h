#ifndef BATTLESHIP_H
#define BATTLESHIP_H
// struct player definition
struct player
{
    char *name;
    int ships[6];
    int shipsRemaining;
    int **grid;
    int radarCount;
    int availableScreens;
    int shipsSunk;
    int artilleryAvailable;
    int artilleryNextTurn;
    int torpedoAvailable;
};

// General function declarations (for both player and bot)
void fire(int **grid, int ships[], int row, int col, struct player *attacker);
void radar_sweep(int **grid, int row, int col, struct player *attacker);
void smoke_screen(int **grid, int row, int col, struct player *attacker);
void artillery(int **grid, int row, int col, struct player *opponent, struct player *attacker);
void torpedo(int **grid, char type, int index, struct player *attacker, struct player *opponent);
void printGrid(int **grid, int isOwner);
void DisplayRules();
int addToGrid(int **grid, char x, int y, char orientation, int size);
int **allocate();
int didLose(struct player *plr);
void gridsetup(struct player *p);
void StartLocalGame();
void InitializeGame();
void StartBotGame();
void printWithDelay(const char *text, int delayMilliseconds);
void waitForMilliseconds(int milliseconds);
void clear_terminal();
void loadingAnimation(char *text);

// Bot-specific function declarations (these are declared in Bot.h)
void placeShips(struct player *botPlayer);
void performBotMove(struct player *player, struct player *botPlayer);

#endif
