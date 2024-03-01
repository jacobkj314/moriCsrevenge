#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

//VARIABLES
int MS_PER_TICK = 50;

int screenHeight;
int screenWidth;

int viewWidth;
int consoleWidth;

int** terrain;
int** objects;

char** messages;

int pressed;

int playerX, playerY;

int colors[] = {COLOR_BLUE, COLOR_CYAN, COLOR_YELLOW, COLOR_GREEN, COLOR_BLACK};
int num_colors = 4;
int player_color;

//HELPER FUNCTIONS
int min(int a, int b, int c, int d)
{
    int m = a;
    m = (m < b) ? m : b;
    m = (m < c) ? m : c;
    m = (m < d) ? m : d;
}
int relu(int x)
{
    return (x < 0) ? 0 : x;
}

//INIT METHODS
void init_terrain(int height, int width)
{
    srand(time(NULL));
    int y;
    int x;

    //allocation
    terrain = (int**) malloc(height*sizeof(int*));
    for(y = 0 ; y < height; y++)
    {
        terrain[y] = (int*) malloc(width*sizeof(int));
    }

    //building base
    for(y = 0 ; y < height ; y++)
    {
        for(x = 0 ; x < width ; x++)
        {
            terrain[y][x] = min(y, height-y, x, width-x);
        }
    }

    //eroding
    for(int i = 0 ; i < height*width*3 ; i++ )
    {
        y = rand() % height;
        x = rand() % width;
        terrain[y][x] = relu(terrain[y][x]-1);
    }

    //scaling
    int m = ((height < width) ? height : width)/2;
    int v;
    for(y = 0 ; y < height ; y++)
    {
        for(x = 0 ; x < width ; x++)
        {
            v = terrain[y][x];
            terrain[y][x] = (2*v*m - v*v)*4 / (m*m) ;
            terrain[y][x] = 3-relu(3-terrain[y][x]);
        }
    }
}
void init_console(){
    messages = (char**) malloc(screenHeight*sizeof(char*));
    for(int i = 0 ; i < screenHeight; i++)
    {
        messages[i] = (char*) malloc(consoleWidth*sizeof(char));
        for(int j = 0 ; j < consoleWidth; j++){
            messages[i][j] = ' ';
        }
    }
}

bool walkable(int y, int x){
    return (y < screenHeight) && (x < viewWidth) && terrain[y][x] > 0;
}


//PLAYER METHODS
void init_player(){
    while(!(walkable(playerY, playerX) && walkable(playerY+1, playerX) && walkable(playerY, playerX+1) && walkable(playerY+1, playerX+1))){
        playerY++;
        playerX++;
    }
}


//CONTROL METHODS
int get_key(WINDOW *win){
    int last = ERR;
    int curr = wgetch(win);
    int ms = 1;
    while (curr != ERR && ms < MS_PER_TICK){
        last = curr;
	curr = wgetch(win);
        ms++;
    }
    return last;
}

//RENDERING METHODS
void render_terrain(){
    for(int y = 0 ; y < screenHeight ; y++)
    {
        for(int x = 0 ; x < viewWidth ; x++)
        {
            attron(COLOR_PAIR(terrain[y][x]+1));
            mvaddch(y, x, ' ');
            attroff(COLOR_PAIR(terrain[y][x]+1));
        }
    }
}

void render_console(){
    for(int i = 0; i < screenHeight; i++){
        //mvaddch(i, viewWidth, '|');
        char* message = messages[i];
        for(int j = 0; j < consoleWidth; j++){
            char c = message[j];
            if(c == '\0'){
                break;
            }
            mvaddch(i, viewWidth+j, c);
        }
    }
}


void render_player(){
    attron(COLOR_PAIR(terrain[playerY][playerX]+5));
	mvaddstr(playerY, playerX, "&"); // draw player sprite
    attroff(COLOR_PAIR(terrain[playerY][playerX]+5));
	mvaddstr(playerY, playerX, "");   // move cursor to player position
}



void render(){
	erase();
	render_terrain();
    render_console();
	render_player();
}

void wrconsole(char* message){
    int num_new_lines = strlen(message) / consoleWidth + 1;
    for(int i = screenHeight-1; i >= num_new_lines; i--){
        messages[i] = messages[i-num_new_lines];
    }
    for(int i = 0; i < num_new_lines; i++){
        messages[i] = &message[i*consoleWidth];
    }
}

//MAIN METHODS
void move_player(int pressed){
    int new_playerY = playerY;
    int new_playerX = playerX;

    if(pressed == 119){
        new_playerY--;
    }
    else if(pressed == 100){
        new_playerX++;
    }
    else if(pressed == 115){
        new_playerY++;
    }
    else if(pressed == 97){
        new_playerX--;
    }
    else if(pressed == 98){
        erase();
    }

    //revert changes if player is trying to walk to a new space
    if(!walkable(new_playerY, new_playerX)){
        wrconsole("You cannot go there!");
        new_playerY = playerY;
        new_playerX = playerX;
    }


    playerY = new_playerY;
    playerX = new_playerX;
}

bool game_loop(int pressed){
    move_player(pressed);
    render();
    return true;
}



int main() {
    // Initialize ncurses
    WINDOW* win = initscr();
    cbreak();
    noecho();
    keypad(win, true);
    nodelay(win, true);
    timeout(0);
    if(has_colors())
    {
        start_color();

        int color_number;
        for(color_number=0; color_number < num_colors; color_number++)
        {
            init_pair(color_number+1, COLOR_BLACK, colors[color_number]);
        }
        for( ; color_number < num_colors*2; color_number++)
        {
            init_pair(color_number+1, COLOR_MAGENTA, colors[color_number-num_colors]);
        }
    }

    getmaxyx(win, screenHeight, screenWidth);
    viewWidth = 3 * (screenWidth >> 2);
    consoleWidth = screenWidth - viewWidth;


    init_terrain(screenHeight, viewWidth);
    init_console();
    init_player();



    wrconsole("Welcome to MoriCs Revenge!");
    do {
        usleep(MS_PER_TICK * 1000);
        pressed = get_key(win);
    } while(game_loop(pressed));
    



    endwin();
    return 0;
}

