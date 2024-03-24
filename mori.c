#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

//VARIABLES
long tick = 0;
int MS_PER_TICK = 50;

int screenHeight;
int screenWidth;

int viewWidth;
int consoleWidth;

int** terrain;
char** objects;

char** messages;

int pressed;

bool debug = false;

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

bool walkable(int y, int x){
    return (y < screenHeight) && (x < viewWidth) && (terrain[y][x] > 0) && objects[y][x] == ' ';
}
bool land(int y, int x){
    return terrain[y][x] > 1;
}
bool sea(int y, int x){
    return terrain[y][x] < 2;
}
bool interactable(int y, int x){
    return (objects[y][x] == '!' || objects[y][x] == 'a' || objects[y][x] == '?'); //TODO: expand
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

    //init objects array
    objects = (char**) malloc(height*sizeof(char*));
    for(y = 0 ; y < height; y++)
    {
        objects[y] = (char*) malloc(width*sizeof(char));
    }
    for(y = 0; y < height ; y++){
        for(x = 0 ; x < width ; x ++){
            objects[y][x] = ' ';
        }
    }
    for(y = 0; y < height ; y++){
        for(x = 0 ; x < width ; x ++){
            bool generate_nature = (((rand() % 50) == 1));
            if(generate_nature){
                bool on_land = land(y, x);
                objects[y][x] = (on_land ? '!' : '?'); //trees and corals
                if(on_land){
                    if((rand() % 20) == 1){
                        int dy = (rand() % 2) * 2 - 1;
                        int dx = (rand() % 2) * 2 - 1;
                        objects[y+dy][x+dx] = 'a';
                    }
                }
            }
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




//PLAYER METHODS
void init_player(){
    while(!(land(playerY, playerX) && walkable(playerY, playerX) && land(playerY+1, playerX) && land(playerY, playerX+1) && land(playerY+1, playerX+1))){
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
            mvaddch(y, x, objects[y][x]);
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

/*
void wrconsole(char* message){
    int message_length = strlen(message);
    int num_new_lines = message_length / consoleWidth;
    if(num_new_lines * consoleWidth < message_length){
        num_new_lines++;
    }
    for(int i = screenHeight-1; i >= num_new_lines; i--){
        messages[i] = messages[i-num_new_lines];
    }
    for(int i = 0; i < num_new_lines; i++){
        messages[i] = &message[i*consoleWidth];
    }
}
*/
void wrconsole(char* message) {
    int message_length = strlen(message);
    int num_new_lines = message_length / consoleWidth;
    if (num_new_lines * consoleWidth < message_length) {
        num_new_lines++;
    }
    for (int i = screenHeight - 1; i >= num_new_lines; i--) {
        strcpy(messages[i], messages[i - num_new_lines]);
    }
    for (int i = 0; i < num_new_lines; i++) {
        strncpy(messages[i], &message[i * consoleWidth], consoleWidth);
    }
}

void interact(int y, int x){
  if(debug){
    char dm[32];
    sprintf(dm, "interaction with %c at (%d,%d)", objects[y][x], y, x);
    wrconsole(dm);
  }

  char*  m;

  char o = objects[y][x];
  if(o == '!'){
    m = "Tree";
  }
  else if(o == '?'){
    m = "Coral";
  }
  else if(o == 'a'){
    m = "Apple";
  }
  else if(o = ' '){
    return;
  }
  else{
    m = "Unexpected Object!";
  }

  wrconsole(m);
}

//MAIN METHODS
void move_player(int pressed){
    int new_playerY = playerY;
    int new_playerX = playerX;
    bool tried_to_move = false;
    if(pressed == 119){ //W
        new_playerY--;
        tried_to_move = true;
    }
    else if(pressed == 100){ //A
        new_playerX++;
        tried_to_move = true;
    }
    else if(pressed == 115){ //S
        new_playerY++;
        tried_to_move = true;
    }
    else if(pressed == 97){ //D
        new_playerX--;
        tried_to_move = true;
    }
    else if(pressed == 102){ //F
        interact(playerY, playerX);
    }
    else if(pressed == 108){ //L
      debug = !debug;

      //char dm[12];
      //sprintf(dm, "debug=%s;", debug ? "true" : "false");
      //wrconsole(dm);
    }
    else if(pressed == 98){ //B -- I don't remember why I wrote this
        erase();
    }

    if(tried_to_move && walkable(new_playerY, new_playerX)){
        playerX = new_playerX;
        playerY = new_playerY;
    }
    else if(tried_to_move && interactable(new_playerY, new_playerX)){
        interact(new_playerY, new_playerX); //TODO
        //wrconsole("You found a tree!");
    }
    else if(tried_to_move){
        wrconsole("You cannot go there!");
    }


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
            init_pair(color_number+1, COLOR_RED, colors[color_number]);
        }
        for( ; color_number < num_colors*2; color_number++)
        {
            init_pair(color_number+1, COLOR_MAGENTA, colors[color_number-num_colors]);
        }
        for( ; color_number < num_colors*3; color_number++)
        {
            init_pair(color_number+1, COLOR_RED, colors[color_number-num_colors]);
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
        tick++;
        if(debug){
          char dm[16];
          sprintf(dm, "%ld", tick);
          //wrconsole(dm);
          mvaddstr(0,0,dm);
        }
        usleep(MS_PER_TICK * 1000);
        pressed = get_key(win);
    } while(game_loop(pressed));
    



    endwin();
    return 0;
}

