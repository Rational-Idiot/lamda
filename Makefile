CC = gcc
CFLAGS = -Wall -Wextra
TARGET = lamda
SRC = lamda.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lm

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
