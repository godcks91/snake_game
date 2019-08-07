#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define  HEIGHT         30 //Y coordinate
#define  WIDTH          60 //X coordinate
#define  MAX_SNAKE_BODY 50
#define  OCCUPIED       1
#define  FREE           0

#define WIN_PAIR      1

typedef struct snake
{
   int serial_number;
   bool is_initialized;
   int current_x;
   int current_y;
   int past_x;
   int past_y;
}s_snake;

typedef enum result
{
   game_over = 0,
   game_continue = 1
}e_result;

int startx, starty, width, height;
int max_x = 0, max_y = 0, score = 0, main_max_x = 0, main_max_y = 0;

//To check which coordinates are occupied and which are free
int coordinate[WIDTH][HEIGHT] = {0};
int current_food_coordinate_x = 0;
int current_food_coordinate_y = 0;

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

void init_snake(s_snake *snake)
{
   int i;

   for (i = 0; i < MAX_SNAKE_BODY; i++)
   {
      memset(snake, 0x00, sizeof(snake));
      snake[i].serial_number = i;
      snake[i].is_initialized = FALSE;
   }
}

e_result move_snake(WINDOW *my_win, s_snake *snake, int arrow_key, int max_y, int max_x)
{
   int i, j, k = 0;
   starty = (LINES - HEIGHT) / 2;   /* Calculating for a center placement */
   startx = (COLS - WIDTH) / 2;  /* of the window     */

   e_result status = game_continue;

   for (i = 0; i < MAX_SNAKE_BODY; i++)
   {
      if (snake[i].is_initialized)
      {
         if (snake[i].serial_number == 0)
         {
            switch(arrow_key)
            {
               case KEY_LEFT:
                  snake[i].past_x = snake[i].current_x;
                  snake[i].current_x--;
                  snake[i].past_y = snake[i].current_y;//
                  break;
               case KEY_RIGHT:
                  snake[i].past_x = snake[i].current_x;
                  snake[i].current_x++;
                  snake[i].past_y = snake[i].current_y;//
                  break;
               case KEY_UP:
                  snake[i].past_y = snake[i].current_y;
                  snake[i].current_y--;
                  snake[i].past_x = snake[i].current_x;//
                  break;
               case KEY_DOWN:
                  snake[i].past_y = snake[i].current_y;
                  snake[i].current_y++;
                  snake[i].past_x = snake[i].current_x;//
                  break;
            }

            /* Wall collision detection */
            if (snake[i].current_x == max_x-1 ||
                  snake[i].current_x == 0 ||
                  snake[i].current_y == max_y-1 ||
                  snake[i].current_y == 0)
            {
               status = game_over;
               // Prints GAME OVER in window
               int size = strlen("GAME OVER...");
               mvwprintw(my_win, max_y/2, (max_x-size)/2, "GAME OVER..."); //
               wrefresh(my_win);
               break;
            }

            mvwprintw(my_win, snake[i].current_y, snake[i].current_x, "0");
         }
         else
         {
            snake[i].past_x = snake[i].current_x;
            snake[i].past_y = snake[i].current_y;

            snake[i].current_x = snake[i-1].past_x;
            snake[i].current_y = snake[i-1].past_y;
            mvwprintw(my_win, snake[i].current_y, snake[i].current_x, "o");
         }

         //Update coordinates array
         coordinate[snake[i].current_x][snake[i].current_y] = OCCUPIED;   //Mark current snake body coordinate
         coordinate[snake[i].past_x][snake[i].past_y] = FREE;             //Unmar previous snake body coordinate

         //mvwprintw(my_win, y, x, "%s", snake);
         //mvwprintw(my_win, snake[i].current_y, snake[i].current_x, "o");
         //mvwprintw(my_win, snake[i].current_y, snake[i].current_x, "(%d,%d)", snake[i].current_x, snake[i].current_y);
      }
      else
      {
         //If current item is uninitialized then subsequent items would be uninitialized too!
         break;
      }
   }

   /* Self collision detection */
   for (j = 1; j < MAX_SNAKE_BODY; j++ )
   {
      if (snake[0].current_x == snake[j].current_x &&
            snake[0].current_y == snake[j].current_y)
      {
         wrefresh(my_win);
         usleep(500000);

         while (k < 3)
         {
            wclear(my_win);

            my_win = create_newwin(HEIGHT, WIDTH, starty, startx);
            wrefresh(my_win);
            usleep(250000);

            for (i = 0; i < MAX_SNAKE_BODY; i++)
            {
               if (snake[i].is_initialized)
               {
                  if (i == 0)
                  {
                     mvwprintw(my_win, snake[i].current_y, snake[i].current_x, "0");
                  }
                  else
                  {
                     mvwprintw(my_win, snake[i].current_y, snake[i].current_x, "o");
                  }
               }
               else
               {
                  break;
               }
            }

            wrefresh(my_win);
            usleep(250000);
            k++;
         }

         status = game_over;
         // Prints GAME OVER in window
         int size = strlen("GAME OVER...");
         mvwprintw(my_win, max_y/2, (max_x-size)/2, "GAME OVER..."); //
         wrefresh(my_win);
         break;
      }
   }

   return status;
}

e_result add_snake_body(s_snake *snake)
{
   int i;
   e_result status = game_over;

   for (i = 0; i < MAX_SNAKE_BODY; i++)
   {
      if (snake[i].is_initialized == FALSE)
      {
         snake[i].is_initialized = TRUE;

         if (snake[i].serial_number == 0)
         {
            snake[i].current_x = 1;
            snake[i].current_y = 1;
         }
         else
         {
            snake[i].current_x = snake[i-1].past_x;
            snake[i].current_y = snake[i-1].past_y;
         }

         snake[i].past_x = snake[i].current_x;
         snake[i].past_y = snake[i].current_y;

         status = game_continue;
         break;
      }
   }

   return status;
}

e_result generate_snake_food_coordinates(void)
{
   int random_number = 0;
   int lower_limit = 0;
   int upper_limit = (WIDTH * HEIGHT) - 1;
   e_result status = game_over;

  while (1)
  {
     //Generate random number
     srand(time(0));//Add time based seed
     random_number = ( rand() % (upper_limit - lower_limit + 1) ) + lower_limit;//This is random number in range [0, WIDTH*HEIGHT]

     /*
      * Choosing next random coordinate from the window for the snake food
      *
      *              width = x ------>
      *              --------------
      *              |            |   ^
      *              |            |   |
      *              | Coordinate |   |
      *              |   system   |   |
      *              |            |   |
      *              |            |height = y
      *              --------------
      *
      */
     current_food_coordinate_x = random_number % WIDTH;
     current_food_coordinate_y = random_number / WIDTH;

     if (current_food_coordinate_x == 0 ||
           current_food_coordinate_x == WIDTH-1 ||
           current_food_coordinate_y == 0 ||
           current_food_coordinate_y == HEIGHT-1 )
     {
        continue;
     }

     if (coordinate[current_food_coordinate_x][current_food_coordinate_y] == FREE)
     {
        coordinate[current_food_coordinate_x][current_food_coordinate_y] = OCCUPIED;
        status = game_continue;
        break;
     }
  }

  return status;
}

e_result update_food_status(WINDOW *my_win, s_snake *snake)
{
   e_result status = game_over;

   if ( (snake[0].current_x == current_food_coordinate_x) && (snake[0].current_y == current_food_coordinate_y) )
   {
      //Snake has eaten a food - Add body part
      status = add_snake_body(snake);
      // Add the score by 1 everytime the snake eats the food
      score++;

      if (status == game_continue)
      {
         //Body part added - generate new food
         status = generate_snake_food_coordinates();
         if (status == game_over)
         {
            //Return error in food coordinates generation
         }
      }
      else
      {
         //Snake has reached its max length - stop the game
         //Nothing to be done
      }
   }
   else
   {
      //Food is yet to be eaten - continue game
      status = game_continue;
   }

   //Refresh food coordinates
   //mvwprintw(my_win, current_food_coordinate_y, current_food_coordinate_x, "%d,%d", current_food_coordinate_x, current_food_coordinate_y);
   mvwprintw(my_win, current_food_coordinate_y, current_food_coordinate_x, "o");

   return status;
}

int main(int argc, char *argv[])
{
   WINDOW *my_win;
   int ch;
   int ch_old;//CS
   int pause_count = 0;
   e_result status;
   s_snake snake[MAX_SNAKE_BODY];

#if 1//Snake co-ordinates
//   int max_x = 0, max_y = 0;
   int next_x = 0, next_y = 0;
   int direction = 1;
#endif//Snake co-ordinates

   initscr();        /* Start curses mode       */
   //halfdelay(2);//CS
   /* Colour background */
   start_color();
   noecho();
   init_pair(1, COLOR_CYAN, COLOR_BLACK); //
   cbreak();
   noecho();//CS
   curs_set(FALSE);
   keypad(stdscr, TRUE);      /* I need that nifty F1    */

   height = HEIGHT;
   width = WIDTH;

   starty = (LINES - height) / 2;   /* Calculating for a center placement */
   startx = (COLS - width) / 2;  /* of the window     */

   /* current window */
   getmaxyx(stdscr, main_max_y, main_max_x);
   int size = strlen("SNAKE GAME");
   mvprintw(2, (main_max_x-size)/2, "SNAKE GAME");
   int size1 = strlen("Press SPACE for Pause & Resume");
   mvprintw(4, (main_max_x-size1)/2, "Press SPACE for Pause & Resume");

   refresh();

   my_win = create_newwin(height, width, starty, startx);
   init_pair(WIN_PAIR, COLOR_GREEN, COLOR_BLACK);
   wattron(my_win, COLOR_PAIR(WIN_PAIR));
   wrefresh(my_win);
   wattroff(my_win, COLOR_PAIR(WIN_PAIR));
   
   keypad(my_win, TRUE);      /* I need that nifty F1    */
   nodelay(my_win, TRUE);

   ch_old = KEY_RIGHT;//CS

   init_snake(snake);

   int count = 1;
   add_snake_body(snake);

   generate_snake_food_coordinates();

   //while((ch = getch()) != KEY_F(1))
   while(ch = wgetch(my_win))
      //while(ch = getch())
      //while(1)
   {
#if 1//Play pause
   if (ch == ' ')
   {
      if (pause_count == 0)
      {
         pause_count = 1;
      }
      else if (pause_count == 1)
      {
         pause_count = 0;
      }
   }

   //mvwprintw(stdscr, 15, 30, "ch:%c", ch);
   //refresh();

   if (pause_count == 1)
   {
      continue;
   }
#endif//Play pause

#if 1//In case of no user input select old arrow key
      if (ch == ERR)
         ch = ch_old;
#endif//In case of no user input select old arrow key

#if 1//Stop 180 degree rotation in snake's direction
      if ( (ch == KEY_LEFT && ch_old == KEY_RIGHT) ||
            (ch == KEY_RIGHT && ch_old == KEY_LEFT) ||
            (ch == KEY_UP && ch_old == KEY_DOWN) ||
            (ch == KEY_DOWN && ch_old == KEY_UP) )
      {
         ch = ch_old;
      }
#endif//Stop 180 degree rotation in snake's direction

#if 1//Stop unwanted keys detection
   if (ch != KEY_LEFT &&
         ch != KEY_RIGHT &&
         ch != KEY_UP &&
         ch != KEY_DOWN )
   {
      ch = ch_old;
   }
#endif//Stop unwanted keys detection

#if 1//Decide co-ordinates
      getmaxyx(my_win, max_y, max_x);
#endif//Decide co-ordinates

#if 0//Snake body test
      if (count == 10)
      {
         //wclear(my_win);
         //mvwprintw(my_win, 10, 15, "c6");
         //wrefresh(my_win);
         //sleep(3);
         status = add_snake_body(snake);
         if (status = game_over)
         {
            //more than 5 body parts are added to snake body - you won!
            sleep(2);
            break;
         }
         count = 0;
      }
      count++;
#endif//Snake body test

#if 1//Move the snake
      wclear(my_win);
      my_win = create_newwin(height, width, starty, startx);
      //mvwprintw(my_win, y, x, "%s", snake);
      status = move_snake(my_win, snake, ch, max_y, max_x);
      if (status == game_over)
      {
         //snake touched the border, game over!
         sleep(2);
         break;
      }
      status = update_food_status(my_win, snake);

      /* Update Score */
   int size2 = strlen("SCORE     ");
   mvprintw(6, (main_max_x-size2)/2, "SCORE : %d", score);
   refresh();

      wrefresh(my_win);
#endif//Move the snake

#if 0//Decide direction based on arrow keys
      switch(ch)
      {  case KEY_LEFT:
         --x;
         break;
         case KEY_RIGHT:
         ++x;
         break;
         case KEY_UP:
         --y;
         break;
         case KEY_DOWN:
         ++y;
         break;
      }
#endif//Decide direction based on arrow keys

      ch_old = ch;
      usleep(105000);//CS
      //sleep(5);//CS

      keypad(my_win, TRUE);      /* I need that nifty F1    */
      nodelay(my_win, TRUE);
   }

   endwin();         /* End curses mode        */
   return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{  WINDOW *local_win;

   local_win = newwin(height, width, starty, startx);
   box(local_win, 0 , 0);     /* 0, 0 gives default characters
                               * for the vertical and horizontal
                               * lines       */
   wrefresh(local_win);    /* Show that box     */

   return local_win;
}

void destroy_win(WINDOW *local_win)
{
   /* box(local_win, ' ', ' '); : This won't produce the desired
    * result of erasing the window. It will leave it's four corners
    * and so an ugly remnant of window.
    */
   wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
   /* The parameters taken are
    * 1. win: the window on which to operate
    * 2. ls: character to be used for the left side of the window
    * 3. rs: character to be used for the right side of the window
    * 4. ts: character to be used for the top side of the window
    * 5. bs: character to be used for the bottom side of the window
    * 6. tl: character to be used for the top left corner of the window
    * 7. tr: character to be used for the top right corner of the window
    * 8. bl: character to be used for the bottom left corner of the window
    * 9. br: character to be used for the bottom right corner of the window
    */
   wrefresh(local_win);
   delwin(local_win);
}
