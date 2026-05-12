@echo off
set CXX=g++
set CXXFLAGS=-std=c++17 -Wall -Wextra -Werror -Iinclude

if not exist obj mkdir obj

echo Compiling library objects...
%CXX% %CXXFLAGS% -c src/Lexer.cpp -o obj/Lexer.o
%CXX% %CXXFLAGS% -c src/Parser.cpp -o obj/Parser.o
%CXX% %CXXFLAGS% -c src/SemanticAnalyzer.cpp -o obj/SemanticAnalyzer.o
%CXX% %CXXFLAGS% -c src/IRGenerator.cpp -o obj/IRGenerator.o
%CXX% %CXXFLAGS% -c src/IROptimizer.cpp -o obj/IROptimizer.o
%CXX% %CXXFLAGS% -c src/VM.cpp -o obj/VM.o
%CXX% %CXXFLAGS% -c src/NativeRegistry.cpp -o obj/NativeRegistry.o
%CXX% %CXXFLAGS% -c src/Serialization.cpp -o obj/Serialization.o

echo Creating vyronix-runtime.a...
ar rcs vyronix-runtime.a obj/Lexer.o obj/Parser.o obj/SemanticAnalyzer.o obj/IRGenerator.o obj/IROptimizer.o obj/VM.o obj/NativeRegistry.o obj/Serialization.o

echo Compiling resource...
windres resource.rc -o obj/resource.o

echo Compiling vyronixc...
%CXX% %CXXFLAGS% src/compiler_main.cpp obj/resource.o vyronix-runtime.a -o vyronixc.exe

echo Compiling vyronixvm...
%CXX% %CXXFLAGS% src/vm_main.cpp obj/resource.o vyronix-runtime.a -o vyronixvm.exe

echo Compiling vyronix runner...
%CXX% %CXXFLAGS% vyronix_runner.cpp obj/resource.o -o vyronix_runner.exe -mwindows -lgdi32 -lcomctl32 -lcomdlg32 -lshell32 -lgdiplus -static

echo Compiling vyronix...
%CXX% %CXXFLAGS% src/main.cpp obj/resource.o vyronix-runtime.a -o vyronix.exe

echo Compiling vyx...
%CXX% %CXXFLAGS% src/vyx_main.cpp -o vyx.exe

echo Compiling vyronix_tests...
%CXX% %CXXFLAGS% tests/test_lexer.cpp vyronix-runtime.a -o test_lexer.exe
%CXX% %CXXFLAGS% tests/test_parser.cpp vyronix-runtime.a -o test_parser.exe
%CXX% %CXXFLAGS% tests/test_semantic.cpp vyronix-runtime.a -o test_semantic.exe
%CXX% %CXXFLAGS% tests/test_functions.cpp vyronix-runtime.a -o test_functions.exe
%CXX% %CXXFLAGS% tests/test_structs.cpp vyronix-runtime.a -o test_structs.exe
%CXX% %CXXFLAGS% tests/test_native_std.cpp vyronix-runtime.a -o test_native_std.exe

echo Build complete.
