TARGET = simple_mesh
OBJS = main.o mqtt_client.o wifi_monitor.o system_monitor.o
HEADERS = mqtt_client.h mqtt_topics.h wifi_monitor.h system_monitor.h

CFLAGS += -Wall -Wextra -std=c99
CFLAGS += -D_GNU_SOURCE
CFLAGS += -DSIMPLE_MESH_VERSION=\"1.0.0\"

LIBS += -luv -lmosquitto

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 