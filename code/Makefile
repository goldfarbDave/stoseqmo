CORPORA=data/cantrbry data/enwik8 data/shakespeare data/calgary

all: $(CORPORA)


data/cantrbry: | data
	mkdir -p $@
	wget -qO- http://corpus.canterbury.ac.nz/resources/cantrbry.tar.gz | tar xz -C $@

data/enwik8: | data
	mkdir -p $@
	cd $@ && \
	wget -q https://data.deepai.org/enwik8.zip && \
	unzip enwik8.zip && \
	rm enwik8.zip

# Note, we trim out the gutenburg prologue+epilogue legalese
data/shakespeare: | data
	mkdir -p $@
	cd $@ && \
	wget -qO- https://www.gutenberg.org/files/100/100-0.txt |\
	sed -n '85,170287p;170288q' > shakespeare

data/calgary: | data
	mkdir -p $@
	wget -qO- http://corpus.canterbury.ac.nz/resources/calgary.tar.gz | tar xz -C $@

data:
	mkdir $@
