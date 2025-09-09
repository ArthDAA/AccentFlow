#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <linux/input.h>
#include <string.h>
#include <stdlib.h>

int main() {
    const char *keyboard_device = "/dev/input/event3"; // à adapter
    int fd = open(keyboard_device, O_RDONLY);
    if(fd < 0) { perror("open"); return 1; }

    // Création d'un device uinput pour simuler des touches
    int ufd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(ufd < 0) { perror("uinput"); return 1; }

    ioctl(ufd, UI_SET_EVBIT, EV_KEY);
    ioctl(ufd, UI_SET_KEYBIT, KEY_E);    // Lettre e
    ioctl(ufd, UI_SET_KEYBIT, KEY_RIGHTALT);

    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "virtual-accent-keyboard");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1234;
    uidev.id.product = 0x5678;
    uidev.id.version = 1;

    write(ufd, &uidev, sizeof(uidev));
    ioctl(ufd, UI_DEV_CREATE);

    struct input_event ev;
    int accent_mode = 0;

    while(read(fd, &ev, sizeof(ev)) > 0) {
        if(ev.type == EV_KEY) {
            if(ev.code == KEY_RIGHTALT) {
                accent_mode = ev.value; // 1=pressed, 0=released
            }

            // Si mode accent et touche 'e'
            if(accent_mode && ev.code == KEY_E && ev.value == 1) {
                // Injecter 'é'
                struct input_event kev;
                memset(&kev, 0, sizeof(kev));
                kev.type = EV_KEY;
                kev.code = KEY_E;  // on simplifie ici, on pourrait mapper é correctement via compose
                kev.value = 1;
                write(ufd, &kev, sizeof(kev));
                kev.value = 0;
                write(ufd, &kev, sizeof(kev));

                // Émettre EV_SYN
                memset(&kev, 0, sizeof(kev));
                kev.type = EV_SYN;
                kev.code = SYN_REPORT;
                kev.value = 0;
                write(ufd, &kev, sizeof(kev));
            }
        }
    }

    ioctl(ufd, UI_DEV_DESTROY);
    close(ufd);
    close(fd);
    return 0;
}

