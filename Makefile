PROGS = playinst armemu


OBJS_PLAYINST = add_s.o
OBJS_ARMEMU = add_s.o

CFLAGS = -g

%.o : %.s
	as -o $@ $<

%.o : %.c
	gcc -c ${CFLAGS} -o $@ $<

all : ${PROGS}

playinst : playinst.c ${OBJS_PLAYINST}
	gcc -o playinst playinst.c ${OBJS_PLAYINST}

armemu : armemu.c ${OBJS_ARMEMU}
	gcc -o armemu armemu.c ${OBJS_ARMEMU}

clean :
	rm -rf ${PROGS} ${OBJS_PLAYINST}
