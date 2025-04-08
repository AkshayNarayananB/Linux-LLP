#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define GPIO_GREEN 17
#define GPIO_YELLOW 4
#define GPIO_RED 13
int chip_fd;

void toggle_led(int line_num) {
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    memset(&req, 0, sizeof(req));
    req.lineoffsets[0] = line_numl
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = 1;
    data.values[0] = 1;

    // Request control of the GPIO line
    if (ioctl(chip_fd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0) {
        perror("Failed to get line handle");
        return;
    }

    // Turn ON
    if (ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) {
        perror("Failed to set line value ON");
        close(req.fd);
        return;
    }
    sleep(1);

    // Turn OFF
    data.values[0] = 0;
    if (ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0) {
        perror("Failed to set line value OFF");
        close(req.fd);
        return;
    }
    sleep(1);
}

void *toggle_thread(void *args) {
    while (1) {
        toggle_led(GPIO_GREEN);
        toggle_led(GPIO_YELLOW);
    }
    return NULL;
}

int main() {
    chip_fd = open("/dev/gpiochip0", O_RDONLY);
    if (chip_fd < 0) {
        perror("Failed to open /dev/gpiochip0");
        return 1;
    }

    pthread_t thread;
    pthread_create(&thread, NULL, toggle_thread,NULL);

    struct gpioevent_request button_req;
    memset(&button_req, 0, sizeof(button_req));
    button_req.lineoffset = GPIO_BUTTON;
    button_req.eventflags = GPIOEVENT_REQUEST_FALLING_EDGE; // Detect button press
    button_req.consumer_label = "pibrella_button";

    if (ioctl(chip_fd, GPIO_GET_LINEEVENT_IOCTL, &button_req) < 0) {
        perror("Failed to get button event handle");
        return 1;
    }

    struct gpioevent_data event;
    while (1) {
        if (read(button_req.fd, &event, sizeof(event)) == sizeof(event)) {
            // Button pressed, blink Red LED once
            toggle_led(GPIO_RED);
        }
    }

    close(chip_fd);
    return 0;
}
