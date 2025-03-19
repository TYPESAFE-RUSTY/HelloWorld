all : compile run

compile:
	@echo using clang to compile change compiler settings in makefile if clang not installed 
	cmake -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DWEBGPU_BACKEND=DAWN -B build/ -S . & cd build & ninja
	@echo everything compiled sucessfully
	@echo type make run to run the project 

run:
	./build/main.exe