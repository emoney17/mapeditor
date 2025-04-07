all:
	cc -o editor editor.c -I raylib/include/ raylib/lib/libraylib.a -lm -Wall
clean:
	$(RM) editor