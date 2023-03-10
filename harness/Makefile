ROOT=..
OPTFLAGS=-O3 -march=native
WARNFLAGS=-Wall -Wextra -Wdisabled-optimization -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wundef -Werror -Weffc++ -Wfatal-errors
# Extra flags that break on either clang or non-home systems
GCCONLYFLAGS=-Wlogical-op
# EXTRAWARNFLAGS=-Wconversion
# ASANFLAGS=-fno-omit-frame-pointer -fsanitize=address
PTHREADFLAGS=-pthread -lpthread
INC_DIRS= corpus utils coding/ac models harness
HEADER_FILES=\
	coding/ac/ac.hpp \
	corpus/corpus.hpp \
	harness/file_utils.hpp \
	harness/models.hpp \
	models/hash_methods.hpp \
	models/hashing.hpp \
	models/model_backings.hpp \
	models/model_ctx.hpp \
	models/model_mem.hpp \
	models/model_sequence.hpp \
	models/model_traversals.hpp \
	models/model_utils.hpp \
	models/ppmdp.hpp \
	models/ppmdp_params.hpp \
	models/sequencememoizer.hpp \
	models/volfctw.hpp
	utils/data_types.hpp \
	utils/utils.hpp \




# Wrap in quotes so that cpp treats variable as a string
ROOT_IN_QUOTES=$(addprefix '",$(addsuffix "',$(ROOT)))
INC=$(addprefix -I$(ROOT)/,$(INC_DIRS))
HEADERS=$(addprefix $(ROOT)/,$(HEADER_FILES))

CPPFLAGS=-std=c++17 $(OPTFLAGS) $(WARNFLAGS) $(GCCONLYFLAGS) $(EXTRAWARNFLAGS) $(PTHREADFLAGS) $(ASANFLAGS) -g $(INC) -DROOT_PATH=$(ROOT_IN_QUOTES)


all:
runcorrectness: correctness
	./correctness
# all: shakespeare.csv
# all: calgary_.csv
# all: incrementalshakespeare
# 	./incrementalshakespeare > incrementalshakespeare_other.csv

%.csv: filebench
	time ./filebench $(*F) | tee $(*F)_other.csv

incrementalshakespeare: incrementalshakespeare.cpp $(HEADERS) $(MAKEFILE_LIST)
	g++ $(CPPFLAGS) $< -o $@
correctness: correctness.cpp $(HEADERS) $(MAKEFILE_LIST)
	g++ $(CPPFLAGS) $< -o $@
filebench: filebench.cpp $(HEADERS) $(MAKEFILE_LIST)
	g++ $(CPPFLAGS) $< -o $@
