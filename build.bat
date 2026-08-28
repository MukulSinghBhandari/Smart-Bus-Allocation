@echo off
cd /d "%~dp0"

if not exist sqlite3.o (
    echo Compiling SQLite first time. This can take a minute...
    gcc -std=c11 -c ..\C\sqlite3.c -o sqlite3.o
    if errorlevel 1 exit /b 1
)

gcc -std=c11 -Wall -Wextra -I../C main.c graph.c minheap.c dijkstra.c trie.c bus.c loader.c output.c database.c sqlite3.o -o c_engine.exe
if errorlevel 1 exit /b 1

echo Build OK: c_engine.exe
