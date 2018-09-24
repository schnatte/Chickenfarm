
CC = gcc

#CFLAGS  = -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marmv6z -Wall
CFLAGS  = -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm -O3 -Wall
LD = ld
LDFLAGS = -lm -lwiringPi -lwiringPiDev


OBJ = test.o i2c.o max44009.o 
BIN = test

gpio: $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(BIN) $(OBJ)
