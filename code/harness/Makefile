ROOT=..
OPTFLAGS=-O3 -march=native
WARNFLAGS=-Wall -Wextra -Wconversion -Wdisabled-optimization -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wundef -Werror
#PTHREADFLAGS=-pthread -lpthread
INC_DIRS= corpus utils coding/ac models harness
HEADER_FILES=\
	harness/models.hpp \
	corpus/corpus.hpp \
	utils/utils.hpp \
	coding/ac/ac.hpp \
	models/model_ctx.hpp \
	models/model_mem.hpp \
	models/model_utils.hpp \
	models/model_sequence.hpp \
	models/hashing.hpp \
	models/hashsm.hpp \
	models/sequencememoizer.hpp \
	models/volfctw.hpp




# Wrap in quotes so that cpp treats variable as a string
ROOT_IN_QUOTES=$(addprefix '",$(addsuffix "',$(ROOT)))
INC=$(addprefix -I$(ROOT)/,$(INC_DIRS))
HEADERS=$(addprefix $(ROOT)/,$(HEADER_FILES))

CPPFLAGS=-std=c++17 $(OPTFLAGS) $(WARNFLAGS) $(PTHREADFLAGS) -g $(INC) -DROOT_PATH=$(ROOT_IN_QUOTES)


all: correctness
	./correctness
# all: calgary
# 	./calgary | time tee calgary_rest.csv
# all: shakespeare
# 	./shakespeare | time tee shakespeare.csv


shakespeare: shakespeare.cpp $(HEADERS)
	g++ $(CPPFLAGS) $< -o $@
correctness: correctness.cpp $(HEADERS)
	g++ $(CPPFLAGS) $< -o $@
calgary: calgary.cpp $(HEADERS)
	g++ $(CPPFLAGS) $< -o $@
