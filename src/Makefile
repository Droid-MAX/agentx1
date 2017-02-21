OBJS = agentx1.o control_lan.o control_wan.o handle_packet.o
LDFLAGS := $(LDFLAGS) -lpthread

agentx1: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o agentx1
agentx1.o: agentx1.c agentx1.h
	$(CC) $(CFLAGS) -c agentx1.c -o agentx1.o
control_lan.o: control_lan.c agentx1.h
	$(CC) $(CFLAGS) -c control_lan.c -o control_lan.o
control_wan.o: control_wan.c agentx1.h
	$(CC) $(CFLAGS) -c control_wan.c -o control_wan.o
handle_packet.o: handle_packet.c agentx1.h
	$(CC) $(CFLAGS) -c handle_packet.c -o handle_packet.o
clean:
	rm -rf *.o
