TARGET = simple_mesh
OBJS = main.o  wifi_monitor.o system_monitor.o mqtt_controller.o mqtt_agent.o
HEADERS =  mqtt_topics.h wifi_monitor.h system_monitor.h mqtt_controller.h mqtt_agent.h

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