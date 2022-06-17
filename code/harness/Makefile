ROOT=..
OPTFLAGS=-O3 -march=native
INC_DIRS= corpus utils coding/ac models
HEADER_FILES=\
	corpus/corpus.hpp \
	utils/utils.hpp \
	coding/ac/ac.hpp \
	models/model_ctx.hpp \
	models/model_mem.hpp \
	models/model_utils.hpp \
	models/hashing.hpp \
	models/volfctw.hpp




# Wrap in quotes so that cpp treats variable as a string
ROOT_IN_QUOTES=$(addprefix '",$(addsuffix "',$(ROOT)))
INC=$(addprefix -I$(ROOT)/,$(INC_DIRS))
HEADERS=$(addprefix $(ROOT)/,$(HEADER_FILES))

CPPFLAGS=-std=c++17 $(OPTFLAGS) -pthread -lpthread -g $(INC) -DROOT_PATH=$(ROOT_IN_QUOTES)


all: correctness
	./correctness
# all: calgary
# 	./calgary | time tee calgary.csv
# all: shakespeare
# 	./shakespeare | time tee shakespeare.csv


shakespeare: shakespeare.cpp
	g++ $(CPPFLAGS) $< -o $@
correctness: correctness.cpp
	g++ $(CPPFLAGS) $< -o $@
calgary: calgary.cpp
	g++ $(CPPFLAGS) $< -o $@
