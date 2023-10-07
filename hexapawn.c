#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* whether to use pruning */
//#define USE_PRUNING

#define PLY_DEPTH 3

#define C_BLACK "\033[0;30m"
#define C_RED "\033[0;31m"
#define C_YELLOW "\033[0;33m"
#define C_WHITE "\033[0;37m"

#define C_BLACK_WHITE "\033[47;30m"
#define C_WHITE_BLACK "\033[40;37m"
#define C_WHITE_BBLUE "\033[104;37m"

#define BOARD_ROW 3
#define BOARD_COLUMN 3

#define PAWN_WHITE 0
#define PAWN_BLACK 1

#define TURN_PLAYER 0
#define TURN_AI 1

#define WIN_NONE 1
#define WIN_DRAW 0
#define WIN_WHITE -1
#define WIN_BLACK -2

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) > (b) ? (b) : (a))

#define COMPARE_POSITION(pos, a, b) (pos.row == a && pos.column == b)
#define SET_POSITION(pos, a, b) \
	do \
	{ \
		pos.row = a; \
		pos.column = b; \
	} \
	while (0)
#define COPY_POSITION(pos, new) \
	do \
	{ \
		pos.row = new.row; \
		pos.column = new.column; \
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
int hexapawn_alpha (int depth, int alpha, int beta);
int hexapawn_beta (int depth, int alpha, int beta);

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

coordinate_t selected_pawn = { -1, -1 };

/* debug */
/* function call count */
static int debug_alpha = 0;
static int debug_beta = 0;

#define DEBUG_CALL_INIT() (debug_alpha = debug_beta = 0)
#define DEBUG_ALPHA_CALL() (debug_alpha++);
#define DEBUG_BETA_CALL() (debug_beta++);

/*
 * AI-algorithm
 */
int winner (void)
{
	int white_win, black_win;
	int white, black;
	int i, j;

	white_win = 0;
	black_win = 0;
	for (i = 1; i < BOARD_COLUMN + 1; i++)
	{
		if (state[1][i] == 'B')
		{
			black_win = 1;
		}
	}
	for (i = 1; i < BOARD_COLUMN + 1; i++)
	{
		if (state[3][i] == 'W')
		{
			white_win = 1;
		}
	}
	if (white_win && black_win)
		return WIN_DRAW;
	else if (white_win)
		return WIN_WHITE;
	else if (black_win)
		return WIN_BLACK;

	white = 0;
	black = 0;
	for (i = 1; i < BOARD_ROW + 1; i++)
	{
		for (j = 1; j < BOARD_COLUMN + 1; j++)
		{
			if (state[i][j] == 'W')
				white++;
			if (state[i][j] == 'B')
				black++;
		}
	}
	if (white == 0 && black == 0)
		return WIN_DRAW;
	/* black win */
	if (white == 0)
		return WIN_BLACK;
	/* black win */
	if (black == 0)
		return WIN_WHITE;

	return WIN_NONE;
}

int evaluate_state (void)
{
	int w, b;
    int value;
    int i, j;
    
	w = 0;
	b = 0;
    value = 0;
    for (i = 1; i < BOARD_ROW + 1; i++)
    {
        for (j = 1; j < BOARD_COLUMN + 1; j++)
        {
            /* white */
            if (state[i][j] == 'W')
            {
				w++;
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
				b++;
                /* win */
                if (i == 1)
                    return -100;
                /* point */
                else
                    value -= (4 - i) * (4 - i);
            }
        }
    }
	if (w == 0 && b == 0)
		return 0;
	else if (w == 0)
		return -100;
	else if (b == 0)
		return 100;

    return value;
}

/* alpha beta pruning */
/* this function will envelop the logic ordering by decreasing evaluate value */
int hexapawn_alpha (int depth, int alpha, int beta)
{
    coordinate_t remove[2] = { { 1, -1 }, { 1, 1 } };
    coordinate_t move[4] = { { 1, 0 }, { 0, 1 }, { 0, -1 }, { -1, 0 } };
    int evaluation_beta;
    int evaluation = -10000;
    int value;
    int row, column;
    int i, j;
    int p;

	DEBUG_ALPHA_CALL ();

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

                        evaluation_beta = hexapawn_beta (depth - 1, alpha, beta);
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
#ifdef USE_PRUNING
						/* pruning */
						if (evaluation >= beta)
							return evaluation;
						alpha = max (alpha, evaluation);
#endif
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

                        evaluation_beta = hexapawn_beta (depth - 1, alpha, beta);
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

#ifdef USE_PRUNING
						/* pruning */
						if (evaluation >= beta)
							return evaluation;
						alpha = max (alpha, evaluation);
#endif
                    }
                } 
            }
        }
    }

    return evaluation;
}

int hexapawn_beta (int depth, int alpha, int beta)
{
    coordinate_t remove[2] = { { -1, -1 }, { -1, 1 } };
    coordinate_t move[4] = { { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 0 } };
    int evaluation_alpha;
    int evaluation = 10000;
    int value;
    int row, column;
    int i, j;
    int p;

	DEBUG_BETA_CALL ();

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

                        evaluation_alpha = hexapawn_alpha (depth - 1, alpha, beta);
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

#ifdef USE_PRUNING
						/* pruning */
						if (evaluation <= alpha)
							return evaluation;
						beta = min (beta, evaluation);
#endif
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

                        evaluation_alpha = hexapawn_alpha (depth - 1, alpha, beta);
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

#ifdef USE_PRUNING
						/* pruning */
						if (evaluation <= alpha)
							return evaluation;
						beta = min (beta, evaluation);
#endif
                    }
                } 
            }
        }
    }

    return evaluation;
}

/*
 * interaction
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

void print_state (void)
{
	char *color;
    int i, j;

    for (i = 1; i < BOARD_ROW + 1; i++)
    {
        printf ("  ");
        for (j = 1; j < BOARD_COLUMN + 1; j++)
        {
			color = C_WHITE;
			if (COMPARE_POSITION (selected_pawn, i, j))
				color = C_RED;
			if (COMPARE_POSITION (moved_position, i, j) || COMPARE_POSITION (origin_position, i, j))
				color = C_WHITE_BBLUE;
			printf ("%s", color);
			printf ("%c" C_WHITE, state[i][j] ? state[i][j] : '*');
        }
        printf ("\n");
    }
}

void input_state (void)
{
#define NUMERIC_STATE_COLOR(x) ((x) ? ((x) == 1 ? 'W' : 'B') : '.')

    char answer[128] = { 0, };
	FILE *stream;
	int c[3];
	int i;

    /* input a state file */
    do
    {
        printf ("\n");
        if (answer[0])
            printf ("wrong input. ");
        printf ("input the file name. (default: 0)\n$> ");
        scanf ("%s", answer);

		/* default */
		if (!strcmp (answer, "0"))
			return ;

		stream = fopen (answer, "r");
		if (stream)
			break;
    }
    while (1);
	
	for (i = 1; i <= 3; i++)
	{
		fscanf (stream, "%d %d %d", &c[0], &c[1], &c[2]);
		state[i][1] = NUMERIC_STATE_COLOR (c[0]);
		state[i][2] = NUMERIC_STATE_COLOR (c[1]);
		state[i][3] = NUMERIC_STATE_COLOR (c[2]);
	}

	fclose (stream);

#undef NUMERIC_STATE_COLOR
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

void print_evalbar (int *grid)
{
	int i;

	printf (C_BLACK_WHITE);
	for (i = 0; i < grid[0]; i++)
		printf ("%c", i == 0 ? 'W' : ' ');
	printf (C_WHITE_BLACK);
	for (i = 0; i < grid[1]; i++)
		printf ("%c", i == grid[1] - 1 ? 'B' : ' ');
	printf ("\n" C_WHITE);
}

void print_header (int turn)
{
	/* 0: white, 1: black */
	int grid[2];
	int eval;

	/* -27 ~ 27 */
	/* -100, 100 */
	eval = evaluate_state ();

	/* draw eval */
	if (eval == 100)
	{
		grid[0] = 28 * 2;
		grid[1] = 0;
	}
	else if (eval == -100)
	{
		grid[0] = 0;
		grid[1] = 28 * 2;
	}
	else
	{
		grid[0] = eval + 28;
		grid[1] = 28 - eval;
	}
	printf ("EVALUATION\n");
	print_evalbar (grid);

	if (turn)
		printf ("%s TURN\n\n", turn == TURN_PLAYER ? "PLAYER" : "AI");
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
		scanf("%*[^\n]");
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
	
	COPY_POSITION (selected_pawn, pawn);

	system ("clear");
	print_header (1);
	print_state ();

	/* input coordinates to move */
	wrong = 0;
	do
    {
		scanf("%*[^\n]");
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
				
	SET_POSITION (selected_pawn, -1, -1);
	COPY_POSITION (origin_position, pawn);
	COPY_POSITION (moved_position, new);

	/* move */
	printf ("(%s) $> move (%d, %d) -> (%d, %d)\n", player_pawn == PAWN_WHITE ? "white" : "black",
			pawn.row, pawn.column, new.row, new.column);

	state[pawn.row][pawn.column] = '.';
	state[new.row][new.column] = (player_pawn == PAWN_WHITE ? 'W' : 'B');
}

void handle_AI (void)
{
	int evaluation;

	DEBUG_CALL_INIT ();

	/* AI */
	/* search the next move */
	if (player_pawn == PAWN_WHITE)
		evaluation = hexapawn_beta (PLY_DEPTH, -10000, 10000);
	else
		evaluation = hexapawn_alpha (PLY_DEPTH, -10000, 10000);

	printf ("(%s) $> move (%d, %d) -> (%d, %d)\n", player_pawn == PAWN_WHITE ? "black" : "white",
			origin_position.row, origin_position.column, moved_position.row, moved_position.column);
	printf ("alpha: %d beta: %d total: %d\nsearch evaluation: %d\n",
			debug_alpha, debug_beta, debug_alpha + debug_beta, evaluation);

	/* reflect search results */
	memcpy (state, next_state, (BOARD_ROW + 2) * (BOARD_COLUMN + 2));
}

/*
 * entry
 */
int main (void)
{
	int win;

	/* initial state */
    memset (state, 0, BOARD_ROW * BOARD_COLUMN);
    strcpy (&state[1][1], "WWW");
    strcpy (&state[2][1], "...");
    strcpy (&state[3][1], "BBB");

	/* input state */
	input_state ();

	system ("clear");
	printf ("\n");
	print_header (0);
    /* display the initial state */
    print_state ();
    /* get a information about player */
    input_setting ();

	system ("clear");
    /* in game */
    while ((win = winner ()) == WIN_NONE)
    {
		print_header (1);
		print_state ();

		/* player */
		if (turn == TURN_PLAYER)
		{
			handle_player ();
			turn = TURN_AI;

			system ("clear");
		}
		/* AI */
		else
		{
			printf ("\n");
			handle_AI ();
			turn = TURN_PLAYER;
			printf ("\n\n");
		}
    }
	
	print_header (0);
	if (win == WIN_DRAW)
		printf ("DRAW! ");
	else
		printf ("%s (%s) WIN!\n\n",
				(win == WIN_WHITE ?
				 (player_pawn == PAWN_WHITE ? "player" : "AI") :
				 (player_pawn == PAWN_WHITE ? "AI" : "player")),
				win == WIN_WHITE ? "white" : "black");
	print_state ();
	printf ("\n");
}
