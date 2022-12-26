javafiles=$(shell ls *.java)
all: ABCTest.class
	java ABCTest

ABCTest.class: $(javafiles)
	javac -g ABCTest.java

clean:
	rm *.class
	rm output.bin
