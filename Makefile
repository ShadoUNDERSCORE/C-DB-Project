TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -f ./mynewdb.db -n
	./$(TARGET) -f ./mynewdb.db -a "Ken B.,42 Barbie Ln.,64"
	./$(TARGET) -f ./mynewdb.db -a "Jim B.,418 State St.,128" -l
	./$(TARGET) -f ./mynewdb.db -d "Jim B." -l

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $? 

obj/%.o : src/%.c
	gcc -c $< -o $@ -I include