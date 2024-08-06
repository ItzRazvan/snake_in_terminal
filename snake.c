#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SLEEP_TIME 400000

#define UP 1
#define RIGHT 2
#define DOWN 3
#define LEFT 4

#define NUMBER_OF_OBS 5

int screen_height;
int screen_width;

void get_screen_measurements(){
    struct winsize size;
    ioctl(0, TIOCGWINSZ, &size);
    screen_height = size.ws_row;
    screen_width = size.ws_col;
}

typedef struct{
    int x;
    int y;
} Coords;

typedef struct{
    bool alive;
    Coords head;
    int direction;
    int body_size;
    Coords body[300];
} Snake;

Snake snake;
void snake_init(){
    snake.alive = 1;
    snake.head.x = screen_width / 2;
    snake.head.y = screen_height / 2;
    snake.body_size = 0;
    snake.direction = RIGHT;
}


typedef struct{
    Coords obs_position;
} Obs;

Obs obs[NUMBER_OF_OBS];
void generate_obs_coords(int number){
    int x = rand() % screen_width + 1;
    int y = rand() % screen_height + 1;
    obs[number].obs_position.x = x;
    obs[number].obs_position.y = y;
}

void obs_init(){
    srand(time(NULL));
    for(int i = 0; i < NUMBER_OF_OBS; ++i)
        generate_obs_coords(i);
}

typedef struct{
    int score;
    int max_score;
    bool is_running;
} Game;

Game game;
void game_init(){
    game.score = 0;
    game.max_score = 0;
    game.is_running = 1;
}

void game_elements_init(){
    get_screen_measurements();
    //printf("%d, %d", screen_width, screen_height);

    game_init();
    snake_init();
    obs_init();
}


void clear_screen(){
    printf("\033[H\033[J");
}

void print_at(int y, int x, char c){
    printf("\033[%d;%dH%c", x, y, c);
}

void game_loop(){
    while(game.is_running){
        fflush(stdout);

        clear_screen();

        print_at(snake.head.x, snake.head.y, 'x');

        if(!snake.alive)
            game.is_running = 0;

        usleep(SLEEP_TIME);
    }
}

int main(){
    game_elements_init();
    game_loop();

    return 0;
}