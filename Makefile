.PHONY: clean

srcF = $(wildcard *.F)
objF = $(patsubst %.F, obj/%.o, $(srcF))

srcOpenmpF = $(wildcard openmp/*.F)
objOpenmpF = $(patsubst %.F, obj/%.o, $(srcOpenmpF))

srcC = $(wildcard *.c)
objC = $(patsubst %.c, obj/%.o, $(srcC))

srcO0C = $(wildcard O0/*.c)
objO0C = $(patsubst %.c, obj/%.o, $(srcO0C))

srcCPP = $(wildcard *.cpp)
objCPP = $(patsubst %.cpp, obj/%.o, $(srcCPP))

objs = $(objF) $(objC) $(objCPP) $(objOpenmpF) $(objO0C)
dep = $(patsubst %.o, %.d, $(objF) $(objC) $(objCPP))

all: main.exe

main.exe: obj $(objs) 
	@echo "hello" $(objOpenmpF) 
	/usr/bin/gfortran -rdynamic -fopenmp -o main.exe $(objs) -lstdc++ -L//usr/lib64 -lgfortran -lgcc_s -lm -ldl -lpthread

$(objOpenmpF):obj/%.o:%.F
	/usr/bin/gfortran -c -I. -m64 -fimplicit-none -fsecond-underscore -Wunused -Wuninitialized -fcray-pointer -fopenmp -fomit-frame-pointer -fautomatic -fdefault-double-8 $< -o $@ -MMD -MF obj/$*.d -MP

$(objF):obj/%.o:%.F
	/usr/bin/gfortran -c -m64 -fimplicit-none -fsecond-underscore -Wunused -Wuninitialized -fcray-pointer -O3 -fomit-frame-pointer -fautomatic -fdefault-double-8 $< -o $@ -MMD -MF obj/$*.d -MP

$(objO0C):obj/%.o:%.c
	/usr/bin/gcc -c -I. -DTAG=O0 -m64 -fPIC -fomit-frame-pointer -funroll-loops -o $@ -MMD -MF obj/$*.d -MP $< 

$(objC):obj/%.o:%.c
	/usr/bin/gcc -c -m64 -fPIC -O3 -fomit-frame-pointer -funroll-loops -o $@ -MMD -MF obj/$*.d -MP $< 

$(objCPP):obj/%.o:%.cpp
	/usr/bin/g++ -c -std=c++0x -m64 -fPIC -O3 -fomit-frame-pointer -funroll-loops $< -o $@ -MMD -MF obj/$*.d -MP

-include $(dep)

obj:
	mkdir -p $@
	mkdir -p $@/openmp
	mkdir -p $@/O0

clean:
	rm -f *.o *.d main.exe obj -r