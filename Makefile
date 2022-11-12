CC = gcc
OBJ = slash.o

slash: $(OBJ)
	$(CC) -o $@ $^ -lreadline

clean:
	@echo "Nettoyage ..."
	@rm -f $(OBJ) slash
	@echo "Nettoyage terminé."

.PHONY: clean