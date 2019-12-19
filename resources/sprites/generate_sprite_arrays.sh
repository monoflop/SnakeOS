#!/bin/bash

java -classpath ../tools/ BmpToArray snake_head_0.bmp 8 8 sprite_snake_head_0 >> output.c
java -classpath ../tools/ BmpToArray snake_head_1.bmp 8 8 sprite_snake_head_1 >> output.c
java -classpath ../tools/ BmpToArray snake_head_2.bmp 8 8 sprite_snake_head_2 >> output.c
java -classpath ../tools/ BmpToArray snake_head_3.bmp 8 8 sprite_snake_head_3 >> output.c

printf "\n" >> output.c

java -classpath ../tools/ BmpToArray snake_body_0.bmp 8 8 sprite_snake_body_0 >> output.c
java -classpath ../tools/ BmpToArray snake_body_1.bmp 8 8 sprite_snake_body_1 >> output.c
java -classpath ../tools/ BmpToArray snake_body_2.bmp 8 8 sprite_snake_body_2 >> output.c
java -classpath ../tools/ BmpToArray snake_body_3.bmp 8 8 sprite_snake_body_3 >> output.c

printf "\n" >> output.c

java -classpath ../tools/ BmpToArray apple.bmp 8 8 sprite_apple >> output.c
java -classpath ../tools/ BmpToArray snake_logo.bmp 32 32 sprite_snake_logo >> output.c
java -classpath ../tools/ BmpToArray credits.bmp 40 5 sprite_credits >> output.c
java -classpath ../tools/ BmpToArray highscore.bmp 34 34 sprite_highscore >> output.c

rm output
