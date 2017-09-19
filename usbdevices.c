/*
 * Copyright (c) 2017 ladders
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dev/usb/usb.h>

#ifndef NITEMS
    #define NITEMS(_a) (sizeof((_a)) / sizeof((_a)[0]))
#endif

void    usbdev(int, int);
int     main(int, char *[]);

char    seen[USB_MAX_DEVICES];

int
main(int argc, char *argv[]) {
    int cont_no;

    for (cont_no = 0; cont_no < 10; cont_no++) {
        char buf[10];

        snprintf(buf, sizeof buf, "/dev/usb%d", cont_no);

        int fd = open(buf, O_RDONLY);

        if (fd < 0) {
            if (errno == ENOENT || errno == ENXIO)
                continue;

            warn("%s", buf);
        }

        memset(seen, 0, sizeof seen);

        int addr;

        for (addr = 1; addr < USB_MAX_DEVICES; addr++) {
            if (!seen[addr])
                usbdev(fd, addr);
        }

        close(fd);
    }

    return 0;
}

void
usbdev(int fd, int addr) {
    struct usb_device_info di;

    di.udi_addr = addr;

    if (ioctl(fd, USB_DEVICEINFO, &di)) {
        if (errno != ENXIO)
            warn("addr %d: I/O error\n", addr);

        return;
    }

    seen[addr] = 1;         /* now seen this device */

    int i;

    for (i = 0; i < USB_MAX_DEVNAMES; i++) {
        if (di.udi_devnames[i][0])
            printf("%s\t%s (%s)\n", di.udi_devnames[i], di.udi_product, di.udi_vendor);
    }

    /* recurse */

    int p;

    for (p = 0; p < di.udi_nports && p < NITEMS(di.udi_ports); p++) {
        int saddr = di.udi_ports[p];

        if (saddr < USB_MAX_DEVICES)
            usbdev(fd, saddr);
    }
}
