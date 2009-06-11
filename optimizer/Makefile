OCAMLC_OPS = -I ../simulator -cclib "-L ../simulator"

include ../Makefile.ocaml

STUBS = optimizer
LIB_STUBS = str
EXTRA_OBJS = icfprandom state parser
CMI =
COMP_OBJ = $(addsuffix .$(OBJSUFFIX), $(STUBS))
COMP_EOBJ = $(addsuffix .$(OBJSUFFIX), $(EXTRA_OBJS))
COMP_LIBS = $(addsuffix .$(LIBSUFFIX), $(LIB_STUBS))

TARGET = optimizer

all: $(TARGET)

simulator:
	make -C ../simulator all

.PHONY:
	simulator

%.mli: %.ml
	-$(OCAMLC) -c -i $< > $@

%.$(OBJSUFFIX): %.ml
	$(OCAMLC) -c $<

%.cmi: %.mli
	$(OCAMLC) -c $<

$(TARGET): simulator $(CMI) $(COMP_OBJ)
	$(OCAMLC) $(COMP_LIBS) $(COMP_EOBJ) $(COMP_OBJ) -o $(TARGET)

clean: 
	make -C ../simulator clean
	rm -rf *~ *.o *.cmi *.cmo *.cmx $(TARGET) gmon.out ocamlprof.dump
