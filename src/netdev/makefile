OBJS = ether.o netdev.o driver.o packet.o tap.o loop.o
SUBDIR = netdev

all:netdev_obj.o
netdev_obj.o:$(OBJS)
	@echo " [LD] $(SUBDIR)/$@"
	$(Q)$(LD) -r -o $@ $^

%.o:%.c
	@echo " [CC] $(SUBDIR)/$@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o
