CC = gcc
DEPS = cmd_interne.h
OBJ = cmd_interne.o slash.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

slash: $(OBJ)
	$(CC) -o $@ $^ -lreadline $(CFLAGS)

clean:
	@echo "Nettoyage ..."
	@rm -f $(OBJ) slash
	@echo "Nettoyage terminé."

.PHONY: clean