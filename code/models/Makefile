ROOT=..
INC_DIRS= corpus utils coding/ac
HEADERS=\
	$(ROOT)/corpus/corpus.hpp \
	$(ROOT)/utils/utils.hpp \
	$(ROOT)/coding/ac/ac.hpp

# Wrap in quotes so that cpp treats variable as a string
ROOT_IN_QUOTES=$(addprefix '",$(addsuffix "',$(ROOT)))
INC=$(addprefix -I$(ROOT)/,$(INC_DIRS))

CPPFLAGS=-std=c++17 -Og -g $(INC) -DROOT_PATH=$(ROOT_IN_QUOTES)

#all: ctw order0 hashing
all: hashing ctw
	./hashing
	./ctw

# all: ctw
# 	./ctw
ctw: volfctw.cpp $(HEADERS)
	g++ $(CPPFLAGS) $< -o $@
order0: order0.cpp $(HEADERS)
	g++ $(CPPFLAGS) $< -o $@
hashing: hashing.cpp $(HEADERS)
	g++ $(CPPFLAGS) $< -o $@
