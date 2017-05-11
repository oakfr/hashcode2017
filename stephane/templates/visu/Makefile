# executable :
EXE = exe
# sources a compiler :
SRC = main.cpp Visu.cpp
# SRC = $(wildcard *.cpp)

CFLAGS = --std=c++11 -W -Wall -s -pipe
LDFLAGS = -lGL -lglfw


# CIBLES DISPO :

# build normal (defaut)
release: all

# build avec symboles de debug
debug: all

# build pour profiling avec gprof
# $ exe [options/parametres...]
# $ gprof exe
prof: all

# build pour couverture
# $ exe [options/parametres...]
# $ gcov [fichier_source_a_analyser]
# -> resultat dispo dans [fichier_source_a_analyser].gcov
cov: all



OBJ = $(SRC:.cpp=.o)
CLEAN_FILES=$(wildcard $(EXE) *.o *core* gmon.out *.gcov *.gcda *.gcno)
ifeq ($(CLEAN_FILES),)
    CLEAN =
else
    CLEAN = rm $(CLEAN_FILES)
endif
clean:
	$(CLEAN)
all: $(EXE)
$(EXE): $(OBJ)
	g++ $(OBJ) $(LDFLAGS) -o $(EXE)
%.o : %.cpp
	g++ $(CFLAGS) -o $@ -c $<
release: CFLAGS += -O2
debug: CFLAGS += -g
prof: CFLAGS += -g -pg
prof: LDFLAGS += -g -pg
cov: CFLAGS += -g -fprofile-arcs -ftest-coverage
cov: LDFLAGS += -g -fprofile-arcs -ftest-coverage
.PHONY: all debug profile cov clean
