OBJS = Object/agentx1.o Object/control_lan.o Object/control_wan.o Object/handle_packet.o
LDFLAGS := $(LDFLAGS) -lpthread

agentx1: objdir $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o agentx1
objdir:
	mkdir -p Object
Object/agentx1.o: Source/agentx1.c Source/agentx1.h
	$(CC) $(CFLAGS) -c Source/agentx1.c -o Object/agentx1.o
Object/control_lan.o: Source/control_lan.c Source/agentx1.h
	$(CC) $(CFLAGS) -c Source/control_lan.c -o Object/control_lan.o
Object/control_wan.o: Source/control_wan.c Source/agentx1.h
	$(CC) $(CFLAGS) -c Source/control_wan.c -o Object/control_wan.o
Object/handle_packet.o: Source/handle_packet.c Source/agentx1.h
	$(CC) $(CFLAGS) -c Source/handle_packet.c -o Object/handle_packet.o
clean:
	rm -f Object/* agentx1

