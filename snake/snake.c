#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define WIDTH 24
#define HEIGHT 10

#define move_to(x, y) printf("\x1b[%d;%dH", y, x); // starts with 1
#define clear() printf("\x1b[2J");
#define hide_cursor() printf("\x1b[?25l");
#define show_cursor() printf("\x1b[?25h");

#define WHITE "\xb1[39m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

void colorize(char *color, char *text) {
    printf("%s%s%s", color, text, ANSI_COLOR_RESET);
    fflush(stdout);
}

enum Keys {
    UP,
    DOWN,
    RIGHT,
    LEFT,
};

const int opposite[4] = {DOWN, UP, LEFT, RIGHT};

int is_opposite(int self, int other) { return self == opposite[other]; }

struct Cell {
    int x;
    int y;
};

int check_collision(struct Cell *self, struct Cell *other) {
    return self->x == other->x && self->y == other->y;
}

void render_apple(struct Cell *apple) {
    move_to(apple->x, apple->y);
    colorize(RED, "o");
}

struct Snake {
    int length;
    int direction;
    struct Cell body[WIDTH * HEIGHT];
};

void move_snake(struct Snake *snake) {
    struct Cell *head = &snake->body[0];

    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }

    switch (snake->direction) {
    case UP:
        head->y -= 1;
        break;
    case DOWN:
        head->y += 1;
        break;
    case RIGHT:
        head->x += 1;
        break;
    case LEFT:
        head->x -= 1;
    }

    // with border offsets
    if (head->x < 2) {
        head->x = WIDTH + 1;
    } else if (head->x > WIDTH + 1) {
        head->x = 2;
    } else if (head->y < 2) {
        head->y = HEIGHT + 1;
    } else if (head->y > HEIGHT + 1) {
        head->y = 2;
    }
}

void render_snake(struct Snake *snake) {
    for (int i = 0; i < snake->length; i++) {
        move_to(snake->body[i].x, snake->body[i].y);
        colorize(GREEN, "0");
    }
}

struct winsize get_winsize() {
    struct winsize size;

    ioctl(STDIN_FILENO, TIOCGWINSZ, &size);

    return size;
}

void disable_raw_mode() {
    struct termios term;

    tcgetattr(STDIN_FILENO, &term);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void enable_raw_mode() {
    struct termios raw;

    tcgetattr(STDIN_FILENO, &raw);

    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void draw_bounds() {
    // add border offset
    for (int i = 1; i < WIDTH + 2; i++) {
        move_to(i, 1);
        putchar('#');
        move_to(i, HEIGHT + 2);
        putchar('#');
    }

    for (int i = 1; i <= HEIGHT + 2; i++) {
        move_to(1, i);
        putchar('#');
        move_to(WIDTH + 2, i);
        putchar('#');
    }
}

void init_screen() {
    clear();
    hide_cursor();
    enable_raw_mode();

    move_to(10, 5);
    printf("\x1b[1;30;42m SNAKE \x1b[0m\n");
    move_to(4, 7);
    printf("Press ENTER to start\n");
}

void cleanup() {
    move_to(1, 1);
    show_cursor();
    disable_raw_mode();
}

int poll(int timeout) {
    fd_set fds;

    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = timeout;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
}

int get_arrow_key(char c) {
    if (c == '\e') {
        char bytes[2];

        if (read(STDIN_FILENO, &bytes, 2) && bytes[0] == '[') {
            switch (bytes[1]) {
            case 'A':
                return UP;
            case 'B':
                return DOWN;
            case 'C':
                return RIGHT;
            case 'D':
                return LEFT;
            }
        }
    }

    return -1;
}

void game_over() {
    move_to(10, 6);
    colorize(RED, "GAME OVER");
    move_to(10, 7);
    printf("q - Quit\n");
    move_to(9, 8);
    colorize(GREEN, "r - Restart");
}

int game_loop() {
    char c;
    int timeout, new_direction;
    int dead = 0;

    // in microsecond
    const int vspeed = 200000;
    const int hspeed = 150000;
    const int boost = 2500;

    struct Cell apple;
    struct Snake snake;

    apple.x = WIDTH / 2;
    apple.y = (HEIGHT / 2) + 3;

    snake.length = 1;
    snake.direction = DOWN;
    snake.body[0].x = WIDTH / 2;
    snake.body[0].y = HEIGHT / 2;

    srandom(time(NULL));

    while (!dead) {
        clear();

        if (snake.direction == UP || snake.direction == DOWN) {
            timeout = vspeed;
        } else {
            timeout = hspeed;
        }

        if (poll(timeout - snake.length * boost)) {
            read(STDIN_FILENO, &c, 1);

            new_direction = get_arrow_key(c);

            if (is_opposite(snake.direction, new_direction) ||
                new_direction == -1) {
                continue;
            }

            snake.direction = new_direction;
        }

        draw_bounds();
        render_apple(&apple);
        move_snake(&snake);
        render_snake(&snake);

        for (int i = 1; i < snake.length; i++) {
            if (check_collision(&snake.body[0], &snake.body[i])) {
                game_over();
                dead = 1;
            }
        }

        if (check_collision(&snake.body[0], &apple)) {
            snake.length += 1;

            for (int i = 0; i < snake.length; i++) {
                while (check_collision(&snake.body[i], &apple)) {
                    apple.x = (rand() % (WIDTH - 2)) + 2;
                    apple.y = (rand() % (HEIGHT - 2)) + 2;
                }
            }
        }
    }

    while (read(STDIN_FILENO, &c, 1)) {
        if (c == 'r') {
            return 1;
        } else if (c == 'q') {
            break;
        }
    }

    return 0;
}

int main() {
    char c;

    struct winsize size = get_winsize();

    if (size.ws_col < WIDTH || size.ws_row < HEIGHT) {
        printf("The required screen size is %dx%d", WIDTH, HEIGHT);
        return 0;
    }

    atexit(cleanup);

    init_screen();

    while (read(STDIN_FILENO, &c, 1)) {
        if (c == '\n') {
            break;
        }
    }

    while (game_loop())
        ;

    return 0;
}
