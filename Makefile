CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -Iinclude
LDFLAGS = 

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include/vyronix

LIB_OBJS = $(OBJ_DIR)/Lexer.o \
           $(OBJ_DIR)/Parser.o \
           $(OBJ_DIR)/SemanticAnalyzer.o \
           $(OBJ_DIR)/IRGenerator.o \
           $(OBJ_DIR)/IROptimizer.o \
           $(OBJ_DIR)/VM.o \
           $(OBJ_DIR)/NativeRegistry.o \
           $(OBJ_DIR)/Serialization.o

# Platform detection
ifeq ($(OS),Windows_NT)
    EXE = .exe
    RESOURCE_OBJ = $(OBJ_DIR)/resource.o
    RESOURCE_CMD = windres resource.rc -o $(RESOURCE_OBJ)
    MKDIR = if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
    RM = if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
    BUILD_RUNNER = vyronix_runner
else
    EXE =
    RESOURCE_OBJ =
    RESOURCE_CMD = 
    MKDIR = mkdir -p $(OBJ_DIR)
    RM = rm -rf $(OBJ_DIR)
    BUILD_RUNNER = 
endif

all: vyronixc vyronixvm vyronix-runtime.a vyronix vyx $(BUILD_RUNNER)

vyronix: $(OBJ_DIR)/main.o $(RESOURCE_OBJ) vyronix-runtime.a
	$(CXX) $(CXXFLAGS) -o $@$(EXE) $^ $(LDFLAGS)

vyx: src/vyx_main.cpp
	$(CXX) $(CXXFLAGS) -o $@$(EXE) $^ $(LDFLAGS)

vyronixc: $(OBJ_DIR)/compiler_main.o $(RESOURCE_OBJ) vyronix-runtime.a
	$(CXX) $(CXXFLAGS) -o $@$(EXE) $^ $(LDFLAGS)

vyronixvm: $(OBJ_DIR)/vm_main.o $(RESOURCE_OBJ) vyronix-runtime.a
	$(CXX) $(CXXFLAGS) -o $@$(EXE) $^ $(LDFLAGS)

vyronix_runner: vyronix_runner.cpp $(RESOURCE_OBJ)
	$(CXX) $(CXXFLAGS) -o $@.exe $^ -mwindows -lgdi32 -lcomctl32 -lcomdlg32 -lshell32 -lgdiplus -static

vyronix-runtime.a: $(LIB_OBJS)
	ar rcs $@ $^

$(OBJ_DIR)/resource.o: resource.rc
	$(MKDIR)
	$(RESOURCE_CMD)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(MKDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM)
	-del *.exe *.a out.vxb out.vyb 2>nul || rm -f *$(EXE) *.a out.vxb out.vyb

.PHONY: all clean
