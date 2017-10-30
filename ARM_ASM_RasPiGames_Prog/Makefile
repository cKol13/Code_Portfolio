
CC = gcc

FILES = main.S RasPi_IO.S Game_States.S ticker.c Simon.S
FILES += notes.c notes_helpers.S LEDpinControl.S init.S LEDLineup.S
OUT = simon

FLAGS = -Wall
LIBS = -lpthread

all:    $(FILE)
	$(CC) $(FLAGS) -o $(OUT) $(FILES) $(LIBS)
        
clean:
	rm -f *.o core $(OUT)
        
rebuild: clean all
