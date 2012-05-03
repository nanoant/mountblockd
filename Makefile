ORG  := com.nanoant
NAME := mountblockd
PREFIX := /usr/local

all: $(NAME)

$(NAME): $(NAME).c
	$(CC) $(NAME).c -g -o $(NAME) -framework Foundation -framework DiskArbitration

install: $(NAME) $(ORG).$(NAME).plist
	install -d $(PREFIX)/sbin
	install -p $(NAME) $(PREFIX)/sbin
	install -d $(PREFIX)/etc/LaunchDaemons
	install -p $(ORG).$(NAME).plist $(PREFIX)/etc/LaunchDaemons

start:
	launchctl load -F $(PREFIX)/etc/LaunchDaemons/$(ORG).$(NAME).plist
	launchctl start $(ORG).$(NAME)

stop:
	launchctl stop $(ORG).$(NAME)
	launchctl unload -F $(PREFIX)/etc/LaunchDaemons/$(ORG).$(NAME).plist
