#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOARD_ROW 3
#define BOARD_COLUMN 3

#define PAWN_WHITE 0
#define PAWN_BLACK 1

#define TURN_PLAYER 0
#define TURN_AI 1

#define PLY_DEPTH 3

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) > (b) ? (b) : (a))

#define SET_POSITION(pos, a, b) \
	do \
	{ \
		pos.row = a; \
		pos.column = b; \
	} \
	while (0)

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
/* player */
int turn;

/* trace */
coordinate_t origin_position;
coordinate_t moved_position;

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
    turn = strcmp (answer, "player") ? TURN_AI : TURN_PLAYER;
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
    coordinate_t remove[2] = { { 1, -1 }, { 1, 1 } };
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
							if (depth == PLY_DEPTH)
							{
								SET_POSITION (origin_position, i, j);
								SET_POSITION (moved_position, row, column);
								memcpy (next_state, state, (BOARD_ROW + 2) * (BOARD_COLUMN + 2));
							}
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
							if (depth == PLY_DEPTH)
							{
								SET_POSITION (origin_position, i, j);
								SET_POSITION (moved_position, row, column);
								memcpy (next_state, state, (BOARD_ROW + 2) * (BOARD_COLUMN + 2));
							}
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
    coordinate_t remove[2] = { { -1, -1 }, { -1, 1 } };
    coordinate_t move[4] = { { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 0 } };
    int evaluation_alpha;
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

                        evaluation_alpha = hexapawn_alpha (depth - 1);
                        if (evaluation_alpha < evaluation)
                        {
                            /* save current state as next state */
							if (depth == PLY_DEPTH)
							{
								SET_POSITION (origin_position, i, j);
								SET_POSITION (moved_position, row, column);
								memcpy (next_state, state, (BOARD_ROW + 2) * (BOARD_COLUMN + 2));
							}
                            evaluation = evaluation_alpha;
                        }

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

                        evaluation_alpha = hexapawn_alpha (depth - 1);
                        if (evaluation_alpha < evaluation)
                        {
                            /* save current state as next state */
							if (depth == PLY_DEPTH)
							{
								SET_POSITION (origin_position, i, j);
								SET_POSITION (moved_position, row, column);
								memcpy (next_state, state, (BOARD_ROW + 2) * (BOARD_COLUMN + 2));
							}
                            evaluation = evaluation_alpha;
                        }

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
int coordinate_validation (coordinate_t *pawn, coordinate_t *new)
{
	coordinate_t white_remove[2] = { { 1, -1 }, { 1, 1 } };
	coordinate_t black_remove[2] = { { -1, -1 }, { -1, 1 } };
	coordinate_t (*remove)[2];
    coordinate_t move[4] = { { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 0 } };
	int row, column;
	int p;

	for (p = 0; p < 4; p++)
	{
		row = pawn->row + move[p].row;
		column = pawn->column + move[p].column;
		
		if (row == new->row && column == new->column && state[row][column] == '.')
			return 1;
	}
	remove = (player_pawn == PAWN_WHITE ? &white_remove : &black_remove);
	for (p = 0; p < 2; p++)
	{
		row = pawn->row + (*remove)[p].row;
		column = pawn->column + (*remove)[p].column;
		
		if (row == new->row && column == new->column &&
				state[row][column] == (player_pawn == PAWN_WHITE ? 'B' : 'W'))
			return 1;
	}

	return 0;
}

void handle_player (void)
{
	coordinate_t pawn;
	coordinate_t new;
	int wrong;

	/* pawn selection */
	wrong = 0;
	do
    {
        printf ("\n");
        if (wrong)
            printf ("wrong input. ");
        printf ("select the pawn you want to move. (row column)\n(%s) $> ",
				player_pawn == PAWN_WHITE ? "white" : "black");
        scanf ("%d %d", &pawn.row, &pawn.column);
		wrong = 1;
    }
    while (!((pawn.row >= 1 && pawn.row <= BOARD_ROW) &&
			(pawn.column >= 1 && pawn.column <= BOARD_COLUMN) &&
			state[pawn.row][pawn.column] == (player_pawn == PAWN_WHITE ? 'W' : 'B')));

	/* input coordinates to move */
	wrong = 0;
	do
    {
        printf ("\n");
        if (wrong)
            printf ("wrong input. ");
        printf ("enter the coordinates where you want to move the pawn. (row column)\n(%s) $> ",
				player_pawn == PAWN_WHITE ? "white" : "black");
        scanf ("%d %d", &new.row, &new.column);
		wrong = 1;
    }
    while (!((new.row >= 1 && new.row <= BOARD_ROW) &&
			(new.column >= 1 && new.column <= BOARD_COLUMN) &&
			coordinate_validation (&pawn, &new)));
				
	/* move */
	printf ("(%s) $> move (%d, %d) -> (%d, %d)\n", player_pawn == PAWN_WHITE ? "white" : "black",
			pawn.row, pawn.column, new.row, new.column);

	state[pawn.row][pawn.column] = '.';
	state[new.row][new.column] = (player_pawn == PAWN_WHITE ? 'W' : 'B');
}

void handle_AI (void)
{
	/* AI */
	/* search the next move */
	if (player_pawn == PAWN_WHITE)
		hexapawn_beta (PLY_DEPTH);
	else
		hexapawn_alpha (PLY_DEPTH);

	/* reflect search results */
	memcpy (state, next_state, (BOARD_ROW + 2) * (BOARD_COLUMN + 2));
}

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
    while (1)
    {
		printf ("\n");
		print_state ();
		printf ("%d\n", evaluate_state ());
		/* player */
		if (turn == TURN_PLAYER)
		{
			handle_player ();
			turn = TURN_AI;
		}
		/* AI */
		else
		{
			handle_AI ();
			turn = TURN_PLAYER;
		}
    }
    
}
