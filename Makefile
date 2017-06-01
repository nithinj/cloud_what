CC      = gcc
CFLAGS  = -g
RM      = rm -f


default: all

all: cloud_what

cloud_what: clouddetect.c
	$(CC) $(CFLAGS) -o cloud_what clouddetect.c

clean veryclean:
	$(RM) cloud_what
