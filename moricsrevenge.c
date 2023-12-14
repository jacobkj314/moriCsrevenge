#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    // Initialize ncurses
    initscr();
    // Don't display input characters on the screen
    cbreak();
    // Don't display input until Enter is pressed
    noecho();

    // Create a window
    WINDOW *win = newwin(0, 0, 0, 0);

    // Get the dimensions of the window
    int height, width;
    getmaxyx(win, height, width);

    // Print the dimensions to the screen
    printw("Window dimensions: %d x %d", height, width);
    printw("\n%d", rand());
    refresh();
    
    while(true){
	int newY = rand() % height;
	int newX = rand() % width;
        mvaddstr(newY, newX, "*");
        refresh();
	usleep(1000);
    }
    // Wait for a key press
    getch();

    // Clean up and exit
    endwin();

    return 0;
}

