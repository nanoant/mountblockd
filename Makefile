ORG  := com.nanoant
NAME := mountblockd
PREFIX := /usr/local

all: $(NAME)

$(NAME): $(NAME).c
	$(CC) $(NAME).c -g -o $(NAME) -framework Foundation -framework DiskArbitration

install: $(NAME) $(ORG).$(NAME).plist
	install -d $(PREFIX)/sbin
	install -p $(NAME) $(PREFIX)/sbin
	cp $(ORG).$(NAME).plist /Library/LaunchDaemons

uninstall:
	rm $(PREFIX)/sbin/$(NAME)
	rm -f /Library/LaunchDaemons/$(ORG).$(NAME).plist

start:
	launchctl load -wF /Library/LaunchDaemons/$(ORG).$(NAME).plist
	launchctl start $(ORG).$(NAME)

stop:
	launchctl stop $(ORG).$(NAME)
	launchctl unload -F /Library/LaunchDaemons/$(ORG).$(NAME).plist
