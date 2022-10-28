int init_nibbler(int width, int height, int cell_size, const char *window_name);
int get_pressed_keys(int **keys, int *size);
void clear_screen();
void set_square_color(int x, int y, int r, int g, int b);
void render();
void show_game_over();

typedef int (*init_nibbler_t)(int width, int height, int cell_size, const char *window_name);
typedef int (*get_pressed_keys_t)(int **keys, int *size);
typedef void (*clear_screen_t)(void);
typedef void (*set_square_color_t)(int x, int y, int r, int g, int b);
typedef void (*render_t)(void);
typedef void (*show_game_over_t)(void);