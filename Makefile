CC = gcc
CFLAGS = -Wall -Wextra
TARGET = lamda
SRC = lamda.c
TYPED_TARGET = lamdaty
TYPED_SRC = lamdaty.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lm

typed: $(TYPED_SRC)
	$(CC) $(CFLAGS) $(TYPED_SRC) -o $(TYPED_TARGET) -lm

run: $(TARGET)
	./$(TARGET)

run-typed: $(TYPED_TARGET)
	./$(TYPED_TARGET)

clean:
	rm -f $(TARGET) $(TYPED_TARGET)
