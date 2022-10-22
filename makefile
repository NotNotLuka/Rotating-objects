run: compile
	.\main.exe

compile:
	gcc -O2 -ISFML/include -LSFML/lib -DSFML_STATIC -c main.cpp
	gcc -O2 -ISFML/include main.o -o main.exe -LSFML/lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lwinmm -lgdi32 -lstdc++ -static