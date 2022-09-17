NAME = create-note

CC := clang

SRC = $(NAME).c
BIN = $(NAME)

.PHONY:
.DEFAULT: $(BIN)

$(BIN): $(SRC)
	$(CC) -O3 -D 'PROGRAM_NAME="$(NAME)"' $< -o $@
	strip create-note
