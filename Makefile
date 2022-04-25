TARGET := kbdump
OBJDIR := obj
CFLAGS := -std=gnu99 -Wall -Wextra -O2

$(TARGET): $(OBJDIR)/dump.o $(OBJDIR)/main.o
	@echo [$(CC)] $@
	@$(CC) -o $@ $^

$(OBJDIR):
	@mkdir $(OBJDIR)

$(OBJDIR)/main.o: main.c dump.h $(OBJDIR)
	@echo [$(CC)] $@
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/dump.o: dump.c dump.h $(OBJDIR)
	@echo [$(CC)] $@
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo [rm] $(TARGET) $(OBJDIR)
	@rm -rf $(TARGET) $(OBJDIR)

.PHONY: clean
