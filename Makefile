CC = gcc
DEPS = cmd_interne.h cmd_externe.h
OBJ = cmd_externe.o cmd_interne.o slash.o

%.o: %.c $(DEPS)
	$(CC) -Wall -c -o $@ $< $(CFLAGS)

slash: $(OBJ)
	$(CC) -Wall -o $@ $^ -lreadline $(CFLAGS)

clean:
	@echo "Nettoyage ..."
	@rm -f $(OBJ) slash
	@echo "Nettoyage terminé."

.PHONY: clean