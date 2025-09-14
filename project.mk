PROJ.NAME=map

LIBS.SHARED:=

DIR.SRC:=src
DIR.INC:=inc
DIR.BIN:=bin

DIR.OUT:=.

FLAGS.PROJ:=-Wall -Wextra
FLAGS.EXEC:=$(foreach lib,-l$(lib),$(LIBS.SHARED)) -pthread
