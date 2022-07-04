#include <curses.h>

#include <cstdlib>
#include <ctime>

using namespace std;


enum { delay_duration = 300 };
enum { KEY_ESCAPE = 27 };

static int __setpare(int fg, int bg)
{
    int n = 8 * bg + fg + 1;
    init_pair(n, fg, bg);
    return n;
}
static bool __cell_in_field(int rows, int cols, int _i, int _j)
{
    return (0 <= _i && _i < rows) && (0 <= _j && _j < cols);
}
static int __cnt_leave_cell(unsigned char ** cell_field, int rows, int cols, int _i, int _j)
{
    int cnt = 0;
    for(int i = _i - 1; i <= _i + 1; i++)
    {
        for(int j = _j - 1; j <= _j + 1; j++)
        {
            if(i == _i && j == _j)
                continue;
            if(cell_field[(i >= 0) ? (i%rows) : (rows-1)][(j >= 0) ? (j%cols) : (cols-1)])
                cnt++;
        }
    }
    return cnt;
}
static void __swap_fields(unsigned char ** first, unsigned char const * const * second, int rows, int cols)
{
    for(register int i = 0; i < rows; i++)
        for(register int j = 0; j < cols; j++)
            first[i][j] = second[i][j];
}
static void __clear_filed(unsigned char ** field, int rows, int cols)
{
    for(register int i = 0; i < rows; i++)
        for(register int j = 0; j < cols; j++)
            field[i][j] = 0;
}
static void __apply_attributes(unsigned char ** field, int rows, int cols, int clr_fg_pare, int clt_bg_pare)
{
    for(register int i = 0; i < rows; i++)
    {
        for(register int j = 0; j < cols; j++)
        {
            move(i, j);
            if(field[i][j] == 1)
                attrset(COLOR_PAIR(clr_fg_pare));
            else
                attrset(COLOR_PAIR(clt_bg_pare));
            addch(' ');
        }
    }
}


int main()
{
    /* declare all variables */
    int i;
    int j;
    int k;
    int cnt_cells;
    int clr_fg_pare;
    int clt_bg_pare;
    int key;
    int cnt_cell;
    int rows, cols;
    unsigned char ** cell_field_first;
    unsigned char ** cell_field_second;

    /* initialization ncurses */
    initscr();
    getmaxyx(stdscr, rows, cols);
    cbreak();
    noecho();
    keypad(stdscr, 1);
    curs_set(0);
    start_color();
    timeout(delay_duration);

    /* allocate virtual address space */
    cell_field_first = new unsigned char * [rows];
    cell_field_second = new unsigned char * [rows];
    for(i = 0; i < cols; i++)
    {
        cell_field_first[i] = new unsigned char[cols];
        cell_field_second[i] = new unsigned char[cols];
    }

    /* fill field by zero */
    __clear_filed(cell_field_first, rows, cols);
    __clear_filed(cell_field_second, rows, cols);

    /* seed for pseudo sequence number and calc count of cells */
    srand(time(NULL));
    cnt_cells = rows * cols / 2;

    /* generate cells */
    for(k = 0; k < cnt_cells; k++)
    {
        i = rand() % (rows - 1) + 1;
        j = rand() % (cols - 1) + 1;
        if(cell_field_first[i][j] == 0)
            cell_field_first[i][j] = 1;
        else
            k--;
    }

    /* initialize color pare number in ncurses */
    clr_fg_pare = __setpare(COLOR_RED, COLOR_RED);
    clt_bg_pare = __setpare(COLOR_BLACK, COLOR_BLACK);

    /* game loop */
    while((key = getch()) != KEY_ESCAPE)
    {
        /* set attributes */
        __apply_attributes(cell_field_first, rows, cols, clr_fg_pare, clt_bg_pare);

        /* clear second buffer */
        __clear_filed(cell_field_second, rows, cols);

        /* calculate next step of cellular automaton (based on rules GOL) */
        for(i = 0; i < rows; i++)
        {
            for(j = 0; j < cols; j++)
            {
                cnt_cell = __cnt_leave_cell(cell_field_first, rows, cols, i, j);

                /* rule for die cell */
                if(cell_field_first[i][j]==0)
                {
                    if(cnt_cell == 3)
                        cell_field_second[i][j] = 1;
                }

                if(cell_field_first[i][j]==1)
                {
                    if(cnt_cell == 2 || cnt_cell == 3)
                        cell_field_second[i][j] = 1;
                    else
                        cell_field_second[i][j] = 0;
                }
            }
        }

        /* swap buffers */
        __swap_fields(cell_field_first, cell_field_second, rows, cols);
    }


    /* release allocated virtual memory */
    for(i = 0; i < cols; i++)
    {
        delete [] cell_field_first[i];
        delete [] cell_field_second[i];
    }
    delete [] cell_field_first;
    delete [] cell_field_second;

    /* restore state of terminal */
    endwin();
    return 0;
}
