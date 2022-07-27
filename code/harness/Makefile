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
	harness/models.hpp \
	harness/file_utils.hpp \
	corpus/corpus.hpp \
	utils/utils.hpp \
	utils/data_types.hpp \
	coding/ac/ac.hpp \
	models/model_ctx.hpp \
	models/model_mem.hpp \
	models/model_utils.hpp \
	models/model_sequence.hpp \
	models/hashing.hpp \
	models/hash_methods.hpp \
	models/ppmdp_params.hpp \
	models/ppmdp.hpp \
	models/sequencememoizer.hpp \
	models/volfctw.hpp




# Wrap in quotes so that cpp treats variable as a string
ROOT_IN_QUOTES=$(addprefix '",$(addsuffix "',$(ROOT)))
INC=$(addprefix -I$(ROOT)/,$(INC_DIRS))
HEADERS=$(addprefix $(ROOT)/,$(HEADER_FILES))

CPPFLAGS=-std=c++17 $(OPTFLAGS) $(WARNFLAGS) $(GCCONLYFLAGS) $(EXTRAWARNFLAGS) $(PTHREADFLAGS) $(ASANFLAGS) -g $(INC) -DROOT_PATH=$(ROOT_IN_QUOTES)


# all: correctness
# 	./correctness
# all: chunked_shakespeare.csv
all: incrementalshakespeare
	./incrementalshakespeare > incrementalshakespeare_other.csv

%.csv: filebench
	./filebench $(*F) | tee $(*F)_other.csv

incrementalshakespeare: incrementalshakespeare.cpp $(HEADERS) $(MAKEFILE_LIST)
	g++ $(CPPFLAGS) $< -o $@
correctness: correctness.cpp $(HEADERS) $(MAKEFILE_LIST)
	g++ $(CPPFLAGS) $< -o $@
filebench: filebench.cpp $(HEADERS) $(MAKEFILE_LIST)
	g++ $(CPPFLAGS) $< -o $@
