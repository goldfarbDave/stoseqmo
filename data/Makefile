CORPORA=cantrbry enwik8 shakespeare calgary abc1bit

all: $(CORPORA)


cantrbry:
	mkdir -p $@
	wget -qO- http://corpus.canterbury.ac.nz/resources/cantrbry.tar.gz | tar xz -C $@

enwik8:
	mkdir -p $@
	cd $@ && \
	wget -q https://data.deepai.org/enwik8.zip && \
	unzip enwik8.zip && \
	rm enwik8.zip

# Note, we trim out the gutenburg prologue+epilogue legalese
shakespeare:
	mkdir -p $@
	cd $@ && \
	wget -qO- https://www.gutenberg.org/files/100/100-0.txt |\
	sed -n '85,170287p;170288q' > shakespeare

calgary:
	mkdir -p $@
	wget -qO- http://corpus.canterbury.ac.nz/resources/calgary.tar.gz | tar xz -C $@

abc1bit:
	mkdir -p $@
	gzip -kd abc1bit.txt.gz
	mv abc1bit.txt $@
