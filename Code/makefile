
CC = gcc

#CFLAGS  = -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marmv6z -Wall
CFLAGS  = -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm -O3 -Wall
LD = ld
LDFLAGS = -lm -lwiringPi -lcurl#-lwiringPiDev


OBJ = chickenfarm.o i2c.o max44009.o lcd.o bme280.o
BIN = chickenfarm

gpio: $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(BIN) $(OBJ)
