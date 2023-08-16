CFLAGS=-g -Wall -Werror
SOURCES=hash.c  main.c  crypto.c
BINARY=signtest
IMAGE=device_app.bin
TARGET=sig_device_app.bin

.FORCE: sign
all:
	make sign

sign:
	$(CC) $(CFLAGS) -o $(BINARY) $(SOURCES)
	./$(BINARY)	  $(IMAGE)
	cat $(IMAGE).sig $(IMAGE) > $(TARGET)
	-rm $(IMAGE).sig

key:
	$(CC) $(CFLAGS) -o $(BINARY) $(SOURCES)
	./$(BINARY) k

clean:
	rm -rf *.o $(BINARY)