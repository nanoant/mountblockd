// Mount Block Daemon
//
// (c) 2012 Adam Strzelecki nanoant.com
//
// Based on:
// http://superuser.com/questions/336455/mac-lion-fstab-is-deprecated-so-what-replaces-it-to-prevent-a-partition-from-m/336474#336474

#include <syslog.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>

CFStringRef *names = NULL;
int nameCount = 0;
bool run = true;
bool console = true;

void printlog(int level, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsyslog(level, format, args);
	va_end(args);

	if (console) {
		const char *str_level = "";
		time_t now = time(NULL);
		char *str_time = ctime(&now);

		str_time[strlen(str_time) - 1] = 0;
		switch(level) {
			case LOG_ERR:     str_level = "Error: ";   break;
			case LOG_WARNING: str_level = "Warning: "; break;
			case LOG_NOTICE:  str_level = "Notice: ";  break;
			case LOG_INFO:    str_level = "Info: ";    break;
			case LOG_DEBUG:   str_level = "Debug: ";   break;
		}
		fprintf(stderr, "[%s] %s", str_time, str_level);
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		fprintf(stderr, "\n");
	}
}

DADissenterRef BlockMount(DADiskRef disk, void *context)
{
	CFDictionaryRef description = DADiskCopyDescription(disk);
	CFStringRef name = description ? CFDictionaryGetValue(description, kDADiskDescriptionVolumeNameKey) : NULL;
	DADissenterRef dissenter = NULL;
	bool block = false;
	char *str = NULL;
	if (name) {
		int i;
		CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(name), kCFStringEncodingUTF8);
		str = alloca(length + 1);
		CFStringGetCString(name, str, length, kCFStringEncodingUTF8);
		for (i = 0; i < nameCount; i++) {
			if (CFEqual(name, names[i])) {
				block = true;
				break;
			}
		}
	}
	if (block) {
		dissenter = DADissenterCreate(kCFAllocatorDefault, kDAReturnNotPermitted, NULL);
		printlog(LOG_INFO, "BLOCKED volume `%s'", str);
	} else {
		printlog(LOG_INFO, "allowed volume `%s'", str);
	}
	if (description) {
		CFRelease(description);
	}
	return dissenter;
}

void signal_handler(int sig) {
	switch(sig) {
	case SIGHUP:
		printlog(LOG_INFO, "received SIGHUP signal, not supported");
		CFRunLoopStop(CFRunLoopGetCurrent());
		break;
	case SIGTERM:
		printlog(LOG_INFO, "received SIGTERM signal, terminating");
		run = false; CFRunLoopStop(CFRunLoopGetCurrent());
		break;
	case SIGINT:
		printlog(LOG_INFO, "received SIGINT signal, terminating");
		run = false; CFRunLoopStop(CFRunLoopGetCurrent());
		break;
	case SIGQUIT:
		printlog(LOG_INFO, "received SIGQUIT signal, terminating");
		run = false; CFRunLoopStop(CFRunLoopGetCurrent());
		break;
	default:
		printlog(LOG_WARNING, "uhandled signal (%d) %s", sig, strsignal(sig));
		break;
	}
}

int main(int argc, const char *argv[])
{
	int argi;
	bool useConsole;

	openlog("mountblockd", LOG_NDELAY | LOG_PID | LOG_CONS, LOG_DAEMON);

	for (argi = 1; argi < argc && argv[argi][0] == '-'; argi++) {
		if (!strcmp(argv[argi], "-console")) {
			useConsole = true;
		} else {
			printlog(LOG_ERR, "unknown parameter `%s'", argv[argi]);
		}
	}
	console = useConsole;

	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

	DAApprovalSessionRef session = DAApprovalSessionCreate(kCFAllocatorDefault);
	if (!session) {
		printlog(LOG_ERR, "failed to create Disk Arbitration session");
	} else if (argc - argi <= 0) {
		console = true;
		printlog(LOG_ERR, "usage: %s [-console] <name> ...", argv[0]);
	} else {
		CFStringRef cfStringNames[argc - argi];
		printlog(LOG_INFO, "blocking:");
		for(nameCount = 0; nameCount < argc - argi; nameCount++) {
			printlog(LOG_INFO, "(%d) `%s'", nameCount + 1, argv[nameCount + argi]);
			cfStringNames[nameCount] = CFStringCreateWithCStringNoCopy(NULL, 
						argv[nameCount + 1],
						kCFStringEncodingUTF8,
						kCFAllocatorNull);
		}
		names = cfStringNames;

		DARegisterDiskMountApprovalCallback(session, NULL, BlockMount, NULL);
		DAApprovalSessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

		while (run) {
			CFRunLoopRun();
		}

		DAApprovalSessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		DAUnregisterApprovalCallback(session, BlockMount, NULL);
		CFRelease(session);
	}
	return 0;
}
