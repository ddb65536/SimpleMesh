TARGET = mesh_controller
OBJS = main.o mqtt_client.o wifi_monitor.o system_monitor.o

CFLAGS += -Wall -Wextra -std=c99
CFLAGS += -D_GNU_SOURCE
CFLAGS += -DMESH_CONTROLLER_VERSION=\"1.0.0\"

LIBS += -luv -lmosquitto

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 