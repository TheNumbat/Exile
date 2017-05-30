@echo off

set Wildcard=*.h *.cpp *.inl *.c

echo STATICS FOUND:
findstr -s -n -i -l "static" %Wildcard%
