.PHONY: clean

srcF = $(wildcard ./*.F)
objF = $(patsubst %.F, %.o, $(srcF))

srcC = $(wildcard ./*.c tmp/*.c)
objC = $(patsubst %.c, %.o, $(srcC))

srcCPP = $(wildcard ./*.cpp)
objCPP = $(patsubst %.cpp, %.o, $(srcCPP))


all: main.exe

main.exe: $(objF) $(objC) $(objCPP)
	/usr/bin/gfortran -rdynamic \
	-o main.exe \
	 \
	$^ \
	-lstdc++ -L//usr/lib64 -lgfortran -lgcc_s -lm -ldl -lpthread


$(objF):%.o:%.F
	/usr/bin/gfortran -c -m64 -fimplicit-none -fsecond-underscore -Wunused -Wuninitialized -fcray-pointer -O3 -fomit-frame-pointer -fautomatic -fdefault-real-8 -fdefault-double-8 $< -o $@

$(objC):%.o:%.c
	/usr/bin/gcc -c -m64 -fPIC -O3 -fomit-frame-pointer -funroll-loops $< -o $@

$(objCPP):%.o:%.cpp
	/usr/bin/g++ -c -std=c++0x -m64 -fPIC -O3 -fomit-frame-pointer -funroll-loops $< -o $@


clean:
	rm -f *.o main.exe