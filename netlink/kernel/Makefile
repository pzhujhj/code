
MAKE_OPTS := -C $(LINUX_DIR)
obj-m += netlink_test.o

#netlink_test-objs := netlink_main.o


clean:
	rm -rf *.o *.ko .*.cmd *mod.c .tmp_versions

compile:
	+$(MAKE) $(MAKE_OPTS) M=$(CURDIR) modules
