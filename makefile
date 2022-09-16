NAME = create-note

CC := clang

SRC = $(NAME).c
BIN = $(NAME)

.PHONY:
.DEFAULT: $(BIN)

$(BIN): $(SRC)
	$(CC) -O3 $< -o $@
	strip create-note
