#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>

DADissenterRef BlockMount(DADiskRef disk, void *context)
{
        DADissenterRef dissenter = DADissenterCreate(kCFAllocatorDefault, kDAReturnNotPermitted, CFSTR("forbidden!"));
        return dissenter;
}

int main (int argc, const char * argv[])
{
    DAApprovalSessionRef session = DAApprovalSessionCreate (kCFAllocatorDefault);
    if (!session)
    {
        fprintf(stderr, "failed to create Disk Arbitration session");
    }
        else
        {
        DARegisterDiskMountApprovalCallback(session, NULL, BlockMount, NULL);
        DAApprovalSessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

        while (true) {
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 60 /* seconds */, false);
        }

        DAApprovalSessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        DAUnregisterApprovalCallback(session, BlockMount, NULL);
        CFRelease(session);
    }
    return 0;
}