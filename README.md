# tetris

CNIT 315 Class Project

Benjamin Griggs - bsgriggs@purdue.edu
Nicole Griffin - griff125@purdue.edu
Zayyad Atekoja - zatekoja@purdue.edu
Allison Babilonia - ababilo@purdue.edu
David Szymanski - szymansk@purdue.edu

ToDo:
-Use a queue of 2-3 blocks to show upcoming blocks, pull from the queue when adding to the game
-always print the queue on top of the game area, next block to the left
-add the player's name to the top of the game display
-menu needs to display the game controls
-give menu a fancy looking title

known bugs:
-termios doesnt reset after the game has ended (requires to close and reopen terminal)
-reading from file creates an empty end node

Sample Game Area ():
{Current Score: 0 | Current Level: 1}
======================
|                    |
|                    |
|                    |
|                    |
|        # #         |
|        # #         |
|                    |
|                    |
|                    |
|                    |
|                    |
|                    |
|                    |
|                    |
|                    |
|                    |
|@               @   |
|@               @   |
|@   X Z Z   # # @   |
|@ X X X Z Z # # @   |
======================

