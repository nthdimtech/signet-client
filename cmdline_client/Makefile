all: client

CFLAGS = -std=c99 -I../common -I../host_interface
CFLAGS += -DUSE_RAW_HID

SOURCES = client.c ../host_interface/client_proto.c 
OBJECTS = $(SOURCES:.c=.o)
DEPFILES = $(SOURCES:.c=.o.d)

client: $(OBJECTS)
	gcc $(CFLAGS) $^ -o $@
		
%.o: %.c
	gcc  $(CFLAGS) $< -c -o $@
	gcc  $(CFLAGS) $< -M -MF $@.d

clean:
	rm -rf client $(OBJECTS) $(DEPFILES)

-include $(DEPFILES)
