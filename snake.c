#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#define UP 1
#define RIGHT 2
#define DOWN 3
#define LEFT 4

#define NUMBER_OF_OBS 5

struct termios orig_termios;
void disable_raw_mode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode(){

    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

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
    int x = (rand() % screen_width - 2) + 2;
    int y = (rand() % screen_height - 4) + 4;
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
    int sleep_time;
} Game;

Game game;
void game_init(){
    game.score = 0;
    game.max_score = 0;
    game.is_running = 1;
    game.sleep_time = 10000000 / (screen_width + screen_width);
}


char pressed_key;
void game_elements_init(){
    get_screen_measurements();
    //printf("%d, %d", screen_width, screen_height);

    game_init();
    snake_init();
    obs_init();

    pressed_key = '\0';
}


void clear_screen(){
    printf("\033[H\033[J");
}

void print_at(int y, int x, char c){
    printf("\033[%d;%dH%c", x, y, c);
}

void move_cursor_to_end(){
    printf("\033[%d;%dH", screen_height + 1, 1);
}

void set_dir(){
    switch (pressed_key){
        case 'w':
            snake.direction = UP;
            break;
        case 'd':
            snake.direction = RIGHT;
            break;
        case 's':
            snake.direction = DOWN;
            break;
        case 'a':
            snake.direction = LEFT;
            break;
        default:
            break;
    }
}

void listen_for_key(){

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(STDIN_FILENO, &fd);

    struct timeval timeint;
    timeint.tv_sec = 0;
    timeint.tv_usec = game.sleep_time;

    int res = select(STDIN_FILENO + 1, &fd, NULL, NULL, &timeint);

    if(res > 0){
        read(STDIN_FILENO, &pressed_key, 1);

        set_dir();
    }
}

void move_snake(){
    
        for(int i = snake.body_size; i >= 2; --i)
            snake.body[i-1] = snake.body[i-2];
        if(snake.body_size >= 1){
            snake.body[0] = snake.head;
        }
     switch (snake.direction){
        case UP:
            snake.head.y--;
            break;
        case RIGHT:
            snake.head.x++;
            break;
        case DOWN:
            snake.head.y++;
            break;
        case LEFT:
            snake.head.x--;
            break;
        default:
            exit(EXIT_FAILURE);
    }
}

void print_obs(){
    for(int i = 0; i < NUMBER_OF_OBS; ++i)
        print_at(obs[i].obs_position.x, obs[i].obs_position.y, 'o');
}

void print_snake(){
    print_at(snake.head.x, snake.head.y, 'X');
    for(int i = 0; i < snake.body_size; ++i)
        print_at(snake.body[i].x, snake.body[i].y, 'x');
}

void print_elements(){
    print_obs();
    print_snake();
}

void print_score(){
    printf("\033[1;1H%s %d", "Your score is:", game.score);

}

void add_body_part(){
    switch (snake.direction){
    case UP:
        if(snake.body_size != 0){
            snake.body[snake.body_size].x = snake.body[snake.body_size - 1].x;
            snake.body[snake.body_size].y = snake.body[snake.body_size - 1].y + 1;
        }else{
            snake.body[snake.body_size].x = snake.head.x;
            snake.body[snake.body_size].y = snake.head.y + 1;
        }
        break;
    case RIGHT:
        if(snake.body_size != 0){
            snake.body[snake.body_size].x = snake.body[snake.body_size - 1].x - 1;
            snake.body[snake.body_size].y = snake.body[snake.body_size - 1].y;
        }else{
            snake.body[snake.body_size].x = snake.head.x - 1;
            snake.body[snake.body_size].y = snake.head.y;
        }
        break;
    case DOWN:
        if(snake.body_size != 0){
            snake.body[snake.body_size].x = snake.body[snake.body_size - 1].x;
            snake.body[snake.body_size].y = snake.body[snake.body_size - 1].y - 1;
        }else{
            snake.body[snake.body_size].x = snake.head.x;
            snake.body[snake.body_size].y = snake.head.y - 1;
        }
        break;
    case LEFT:
        if(snake.body_size != 0){
            snake.body[snake.body_size].x = snake.body[snake.body_size - 1].x + 1;
            snake.body[snake.body_size].y = snake.body[snake.body_size - 1].y;
        }else{
            snake.body[snake.body_size].x = snake.head.x + 1;
            snake.body[snake.body_size].y = snake.head.y;
        }
        break;
    default:
        exit(EXIT_FAILURE);
    }
    snake.body_size++;
}

void check_colosions(){
    for(int i = 0; i < NUMBER_OF_OBS; ++i){
        if(snake.head.x == obs[i].obs_position.x && snake.head.y == obs[i].obs_position.y){
            game.score++;
            add_body_part();
            generate_obs_coords(i);
        }
    }

    if(snake.head.x <= 0 || snake.head.x >= screen_width || snake.head.y <= 0 || snake.head.y >= screen_height)
        snake.alive = 0;

    for(int i = 0; i < snake.body_size; ++i)
        if(snake.head.x == snake.body[i].x && snake.head.y == snake.body[i].y)
            snake.alive = 0;
    
}

void game_loop(){
    while(game.is_running){
        fflush(stdout);
        clear_screen();

        print_score();

        move_snake();

        listen_for_key();

        print_elements();
        
        check_colosions();

        if(!snake.alive)
            game.is_running = 0;

        usleep(game.sleep_time);
    }

    if(!snake.alive)
        move_cursor_to_end();
}

int main(){
    game_elements_init();

    enable_raw_mode();

    game_loop();

    return 0;
}