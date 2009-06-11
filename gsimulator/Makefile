include ../Makefile.ocaml

STUBS= icfprandom state parser gsimulator
LIB_STUBS= str
CMI=icfprandom.cmi state.cmi parser.cmi 
COMP_OBJ= $(addsuffix .$(OBJSUFFIX), $(STUBS))
COMP_LIBS= $(addsuffix .$(LIBSUFFIX), $(LIB_STUBS))

TARGET=gsimulator

all: $(TARGET)

%.mli: %.ml
	-$(OCAMLC) -c -i $< > $@

gsimulator.$(OBJSUFFIX): gsimulator.ml
	ocamlfind $(OCAMLC) -c -package lablgtk2 gsimulator.ml

%.$(OBJSUFFIX): %.ml
	$(OCAMLC) -c $<

%.cmi: %.mli
	$(OCAMLC) -c $<

$(TARGET): $(CMI) $(COMP_OBJ)
	ocamlfind $(OCAMLC) -package lablgtk2 $(COMP_LIBS) $(COMP_OBJ) -linkpkg -o $(TARGET)

clean: 
	rm -rf *~ *.o *.cmi *.cmo *.cmx $(TARGET) gmon.out ocamlprof.dump
