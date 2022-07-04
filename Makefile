CC=g++
FLAGS_2D = -lncurses
FLAGS_3D = -lglfw -lGL -lpthread

run_2d: GOL_2D.cpp
	$(CC) GOL_2D.cpp $(FLAGS_2D) && ./a.out

run_3d: GOL_3D.cpp
	$(CC) GOL_3D.cpp $(FLAGS_3D) && ./a.out

clear: a.out
	-rm a.out 2>/dev/null
