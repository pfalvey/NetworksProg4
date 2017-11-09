CC = g++ -std=c++11 -lpthread
MAIN = ./chatclient
EXEC = ./chatclient

all: $(EXEC)

$(EXEC): $(MAIN).o
	$(CC) $(MAIN).o -o $(EXEC)

$(MAIN).o: $(MAIN).cpp
	$(CC) -c $(MAIN).cpp -o $(MAIN).o

clean:
	@echo "Cleaning..."
	@rm -f *.o
	@rm -f $(EXEC)
