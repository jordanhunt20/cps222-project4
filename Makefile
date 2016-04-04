# Makefile for CPS222 Project 4
# There are four targets that can be made
# project4: (the default) compiles the student version of the program with
#   executable placed in file project4
# instructor: compiles the instructor version of the program into directory
#   instructor, with executable placed in file instructor_project4.
# distribution: creates the distribution to be given to the students in \
#   folder project4
# results: creates a directory named results with output resulting from using
#   the instructor program on each of the standard test data files

project4: 	BTree.o BTreeBlock.o BTreeFile.o project4.o
	g++ -o $@ $^

BTree.o:	BTree.h BTreeFile.h BTreeBlock.h
BTreeBlock.o:	BTreeFile.h BTreeBlock.h
BTreeFile.o:	BTreeFile.h BTreeBlock.h
project4.o:	BTree.h

%.o:	%.cc
	g++ -c $<

instructor: BTree.h BTree.cc BTreeBlock.h BTreeBlock.cc BTreeFile.h \
		BTreeFile.cc project4prof.cc project4.cc
	rm -rf ../instructor
	mkdir ../instructor
	g++ -o ../instructor/instructor_project4 \
		-D PROFESSOR_VERSION=project4prof.cc \
		BTree.cc BTreeBlock.cc BTreeFile.cc project4.cc

DEST = "../put-on-server"

distribution: BTree.h BTree.cc BTreeBlock.h BTreeBlock.cc BTreeFile.h \
		BTreeFile.cc project4.cc instructor
	echo 'Be sure this is being done on the architecture students will use in the lab'
	rm -rf $(DEST)
	mkdir $(DEST)
	cp BTree.h $(DEST)
	cp BTree.cc $(DEST)
	cp BTreeBlock.h $(DEST)
	cp BTreeBlock.cc $(DEST)
	cp BTreeFile.h $(DEST)
	cp BTreeFile.cc $(DEST)
	cp project4.cc $(DEST)
	rm -rf test.tree
	../instructor/instructor_project4 < maketest.tree.in
	cp test.tree $(DEST)
	cp onelevel.in $(DEST)
	cp twolevel.in $(DEST)
	cp threelevel.in $(DEST)
	cp Makefile $(DEST)
	chmod -R a+rX $(DEST)

results: ../instructor/instructor_project4
	rm -rf ../results
	mkdir ../results
	cd ../testing; ../instructor/instructor_project4 < test.in \
		> ../results/test.out
	cd ../testing; ../instructor/instructor_project4 < torture.in \
		> ../results/torture.out
	cd ../testing; ../instructor/instructor_project4 < remove.in \
		> ../results/remove.out

../instructor/instructor_project4 : instructor
