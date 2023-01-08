CC = gcc
DEPS = cmd/cmd_interne/cmd_interne.h cmd/cmd_externe/cmd_externe.h
OBJ = cmd/cmd_externe/cmd_externe.o cmd/cmd_interne/cmd_interne.o slash.o

%.o: %.c $(DEPS)
	$(CC) -Wall -c -o $@ $< $(CFLAGS)

slash: $(OBJ)
	$(CC) -Wall -o $@ $^ -lreadline $(CFLAGS)

clean:
	@echo "Nettoyage ..."
	@rm -f $(OBJ) slash
	@echo "Nettoyage terminé."

.PHONY: clean