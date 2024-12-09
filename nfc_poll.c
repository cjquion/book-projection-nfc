/**
 * Using RS300 Felica NFC device to read tags and project custom videos 
 */

#include <stdlib.h>
#include <nfc/nfc.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

// gcc -o nfc_poll nfc_poll.c -lnfc

static nfc_device *pnd = NULL;
static nfc_context *context;

static const uint8_t dragon_uid[10] = {0x04, 0x71, 0x8c, 0x94, 0x24, 0x02, 0x89};
static const uint8_t tiger_uid[10] =   {0x04, 0xd1, 0x4a, 0x7e, 0x24, 0x02, 0x89};
static const uint8_t stag_uid[10] =  {0x04, 0xe1, 0x43, 0x95, 0x24, 0x02, 0x89};

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

static void
print_hex(const uint8_t *pbtData, const size_t szBytes)
{
    size_t szPos;

    for (szPos = 0; szPos < szBytes; szPos++)
    {
        printf("%02x  ", pbtData[szPos]);
    }
    printf("\n");
}

void poll(nfc_target *targets)
{
    int res;
    const nfc_modulation nmMifare = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };
    if ((res = nfc_initiator_list_passive_targets(pnd, nmMifare, targets, 16)) >= 0)
    {
        if (res > 0)
        {
            for (int n = 0; n < res; n++)
            {
                nfc_target target = targets[n];
                if (target.nti.nai.szUidLen > 0)
                {
                    print_hex(target.nti.nai.abtUid, target.nti.nai.szUidLen);

                    if (memcmp(&dragon_uid, &target.nti.nai.abtUid, target.nti.nai.szUidLen) == 0)
                    {
                        printf("DRAGON!\n");
                        system("/Users/chrisquion/BookProjection/dragon.py");
                    }
                     if (memcmp(&stag_uid, &target.nti.nai.abtUid, target.nti.nai.szUidLen) == 0)
                    {
                        printf("STAG!\n");
                        system("/Users/chrisquion/BookProjection/stag.py");
                    }
                    if (memcmp(&tiger_uid, &target.nti.nai.abtUid, target.nti.nai.szUidLen) == 0)
                    {
                        printf("TIGER!\n");
                        system("/Users/chrisquion/BookProjection/tiger.py");
                    }
                }
            }
        }
    }
}

static void stop_polling(int sig)
{
    printf("Exiting\n");
    (void)sig;
    if (pnd != NULL) {
        nfc_abort_command(pnd);
        nfc_exit(context);
        exit(0);
    }
    else
    {
        nfc_exit(context);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char *argv[])
{
    signal(SIGINT, stop_polling);
    printf("Beginning poll\n");
    int i = 0;
    int time = 600;

    // the RS300 doesnt work with libnfc poll function 
    //-- it polls for a microsecond and powers off
    // so workaround is to open device and context
    // and use nfc_initiator_list_passive_targets,
    // then free context and close device, in time-based loop
    // RS300 with libnfc nfc_initiator_list_passive_targets
    // with my tags seem to only pick up UID on a 2nd poll, so 
    // wait for UID to show up 
    while (i < time)
    {
        // Allocate only a pointer to nfc_context
        nfc_context *context;

        // Initialize libnfc and set the nfc_context
        nfc_init(&context);
        if (context == NULL)
        {
            printf("Unable to init libnfc (malloc)\n");
            exit(EXIT_FAILURE);
        }

        // Display libnfc version
        const char *acLibnfcVersion = nfc_version();
        (void)argc;
        //printf("%s uses libnfc %s\n", argv[0], acLibnfcVersion);

        // Open, using the first available NFC device which can be in order of selection:
        //   - default device specified using environment variable or
        //   - first specified device in libnfc.conf (/etc/nfc) or
        //   - first specified device in device-configuration directory (/etc/nfc/devices.d) or
        //   - first auto-detected (if feature is not disabled in libnfc.conf) device
        pnd = nfc_open(context, NULL);

        if (pnd == NULL)
        {
            printf("ERROR: %s\n", "Unable to open NFC device.");
            exit(EXIT_FAILURE);
        }
        // Set opened NFC device to initiator mode
        if (nfc_initiator_init(pnd) < 0)
        {
            nfc_perror(pnd, "nfc_initiator_init");
            exit(EXIT_FAILURE);
        }

        //printf("NFC reader: %s opened\n", nfc_device_get_name(pnd));
        
        nfc_target targets[16];
        //printf("Beginning poll\n");
        poll(targets);

        // Close NFC device
        nfc_close(pnd);
        //printf("NFC device closed.");
        // Release the context
        nfc_exit(context);

        i++;
        msleep(200);
    }

    exit(EXIT_SUCCESS);
}