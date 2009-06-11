include ../Makefile.ocaml

STUBS= icfprandom state parser simulator
LIB_STUBS= str
CMI=icfprandom.cmi state.cmi parser.cmi 
COMP_OBJ= $(addsuffix .$(OBJSUFFIX), $(STUBS))
COMP_LIBS= $(addsuffix .$(LIBSUFFIX), $(LIB_STUBS))

TARGET=simulator

%.mli: %.ml
	-$(OCAMLC) -c -i $< > $@

%.$(OBJSUFFIX): %.ml
	$(OCAMLC) -c $<

%.cmi: %.mli
	$(OCAMLC) -c $<

all: $(TARGET)

$(TARGET): $(CMI) $(COMP_OBJ)
	$(OCAMLC) $(COMP_LIBS) $(COMP_OBJ) -o $(TARGET)

clean: 
	rm -rf *.o *.cmi *.cmo *.cmx $(TARGET) gmon.out ocamlprof.dump
