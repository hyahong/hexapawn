#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOARD_ROW 3
#define BOARD_COLUMN 3

#define PAWN_WHITE 0
#define PAWN_BLACK 1

#define PLAYER_FIRST 0
#define AI_FIRST 1

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) > (b) ? (b) : (a))

/*
 * struct
 */
typedef struct coordinate coordinate_t;
struct coordinate
{
    int row;
    int column;
};

/*
 * prototype
 */
int hexapawn_alpha (int depth);
int hexapawn_beta (int depth);

/*
 * global
 */
/* current state */
char state[BOARD_ROW + 2][BOARD_COLUMN + 2];
/* AI-saved state */
char next_state[BOARD_ROW + 2][BOARD_COLUMN + 2];
/* pawn color */
int player_pawn;
/* first player */
int first;

/*
 * interaction
 */
void print_state (void)
{
    int i, j;

    for (i = 0; i < BOARD_ROW + 2; i++)
    {
        for (j = 0; j < BOARD_COLUMN + 2; j++)
        {
            printf ("%c", state[i][j] ? state[i][j] : '*');
        }
        printf ("\n");
    }
}

void input_setting (void)
{
    char answer[128] = { 0, };

    /* input a color which player uses */
    do
    {
        printf ("\n");
        if (answer[0])
            printf ("wrong input. ");
        printf ("select your color. (white or black)\n$> ");
        scanf ("%s", answer);
    }
    while (strcmp (answer, "white") && strcmp (answer, "black"));
    player_pawn = strcmp (answer, "white") ? PAWN_BLACK : PAWN_WHITE;

    /* input first */
    answer[0] = 0;
    do
    {
        printf ("\n");
        if (answer[0])
            printf ("wrong input. ");
        printf ("who plays first? (player or AI)\n(%s) $> ", player_pawn == PAWN_WHITE ? "white" : "black");
        scanf ("%s", answer);
    }
    while (strcmp (answer, "player") && strcmp (answer, "AI"));
    first = strcmp (answer, "player") ? AI_FIRST : PLAYER_FIRST;
}

/*
 * AI-algorithm
 */
int evaluate_state (void)
{
    int value;
    int i, j;
    
    value = 0;
    for (i = 1; i < BOARD_ROW + 1; i++)
    {
        for (j = 1; j < BOARD_COLUMN + 1; j++)
        {
            /* white */
            if (state[i][j] == 'W')
            {
                /* win */
                if (i == BOARD_ROW)
                    return 100;
                /* point */
                else
                    value += i * i;
            }
            /* black */
            else if (state[i][j] == 'B')
            {
                /* win */
                if (i == 1)
                    return -100;
                /* point */
                else
                    value -= (4 - i) * (4 - i);
            }
        }
    }

    return value;
}

/* alpha beta puruning */
/* this function will envelop the logic ordering by decreasing evaluate value */
int hexapawn_alpha (int depth)
{
    coordinate_t remove[4] = { { 1, -1 }, { 1, 1 } };
    coordinate_t move[4] = { { 1, 0 }, { 0, 1 }, { 0, -1 }, { -1, 0 } };
    int evaluation_beta;
    int evaluation = -10000;
    int value;
    int row, column;
    int i, j;
    int p;

    value = evaluate_state ();
    /* the evaluation value is returned below situation */
    /* 1. depth is zero */
    /* 2. game is end in this state */
    if (!depth || value == -100 || value == 100)
        return value;

    /* search all possibilities for W to catch the enemy pawn */
    /* this action make evaluation value better than just moving the pawn */
    for (i = 1; i < BOARD_ROW + 1; i++)
    {
        for (j = 1; j < BOARD_COLUMN + 1; j++)
        {
            if (state[i][j] == 'W')
            {
                for (p = 0; p < 2; p++)
                {
                    row = i + remove[p].row;
                    column = j + remove[p].column;

                    if (state[row][column] == 'B')
                    {
                        //printf ("%d %d k\n", row, column);
                        state[i][j] = '.';
                        state[row][column] = 'W';

                        evaluation_beta = hexapawn_beta (depth - 1);
                        if (evaluation_beta > evaluation)
                        {
                            /* save current state as next state */
                            memcpy (next_state, state, (BOARD_ROW + 2) * (BOARD_COLUMN + 2));
                            evaluation = evaluation_beta;
                        }

                        state[i][j] = 'W';
                        state[row][column] = 'B';
                    }
                }
            }
        }
    }

    /* search all possibilities for W to move */
    for (i = 1; i < BOARD_ROW + 1; i++)
    {
        for (j = 1; j < BOARD_COLUMN + 1; j++)
        {
            if (state[i][j] == 'W')
            {
                for (p = 0; p < 4; p++)
                {
                    row = i + move[p].row;
                    column = j + move[p].column;
                    
                    if (state[row][column] == '.')
                    {
                        //printf ("%d %d\n", row, column);
                        state[i][j] = '.';
                        state[row][column] = 'W';

                        evaluation_beta = hexapawn_beta (depth - 1);
                        if (evaluation_beta > evaluation)
                        {
                            /* save current state as next state */
                            memcpy (next_state, state, (BOARD_ROW + 2) * (BOARD_COLUMN + 2));
                            evaluation = evaluation_beta;
                        }

                        state[i][j] = 'W';
                        state[row][column] = '.';
                    }
                } 
            }
        }
    }

    return evaluation;
}

int hexapawn_beta (int depth)
{
    coordinate_t remove[4] = { { -1, -1 }, { -1, 1 } };
    coordinate_t move[4] = { { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 0 } };
    int evaluation = 10000;
    int value;
    int row, column;
    int i, j;
    int p;

    value = evaluate_state ();
    /* the evaluation value is returned below situation */
    /* 1. depth is zero */
    /* 2. game is end in this state */
    if (!depth || value == -100 || value == 100)
        return value;

    /* search all possibilities for W to catch the enemy pawn */
    /* this action make evaluation value better than just moving the pawn */
    for (i = 1; i < BOARD_ROW + 1; i++)
    {
        for (j = 1; j < BOARD_COLUMN + 1; j++)
        {
            if (state[i][j] == 'B')
            {
                for (p = 0; p < 2; p++)
                {
                    row = i + remove[p].row;
                    column = j + remove[p].column;

                    if (state[row][column] == 'W')
                    {
                        //printf ("%d %d k\n", row, column);
                        state[i][j] = '.';
                        state[row][column] = 'B';

                        evaluation = min (evaluation, hexapawn_beta (depth - 1));

                        state[i][j] = 'B';
                        state[row][column] = 'W';
                    }
                }
            }
        }
    }

    /* search all possibilities for W to move */
    for (i = 1; i < BOARD_ROW + 1; i++)
    {
        for (j = 1; j < BOARD_COLUMN + 1; j++)
        {
            if (state[i][j] == 'B')
            {
                for (p = 0; p < 4; p++)
                {
                    row = i + move[p].row;
                    column = j + move[p].column;
                    
                    if (state[row][column] == '.')
                    {
                        //printf ("%d %d\n", row, column);
                        state[i][j] = '.';
                        state[row][column] = 'B';

                        evaluation = min (evaluation, hexapawn_beta (depth - 1));

                        state[i][j] = 'B';
                        state[row][column] = '.';
                    }
                } 
            }
        }
    }

    return evaluation;
}

/*
 * entry
 */
int main (void)
{
    /* TODO: read the initial state of board */
    memset (state, 0, BOARD_ROW * BOARD_COLUMN);
    strcpy (&state[1][1], "WWW");
    strcpy (&state[2][1], "...");
    strcpy (&state[3][1], "BBB");

    /* display the initial state */
    print_state ();
    /* get a information about player */
    input_setting ();

    /* in game */
    //while (1)
    {
        printf ("%d\n", hexapawn_alpha (3));
        memcpy (state, next_state, (BOARD_ROW + 2) * (BOARD_COLUMN + 2));
        print_state ();
    }
    
}