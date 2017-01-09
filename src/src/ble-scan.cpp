/*
 * Copyright (c) 2015 - 2016 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * @ingroup howtocode
 * @brief BLE scan bracelet in C++
 *
 * This alarm-clock application is part of a series of how-to Intel IoT code
 * sample exercises using the Intel® IoT Developer Kit, Intel® Edison board,
 * cloud platforms, APIs, and other technologies.
 *
 * @hardware Sensors used:\n
 * Xadow* - OLED display\n
 *
 * @cc
 * @cxx -std=c++1y
 * @ld -lupm-i2clcd -lpaho-mqtt3cs -lcurl -lbluetooth
 *
 * Additional source files required to build this example:
 * @req datastore.cpp
 * @req mqtt.cpp
 *
 * @date 04/04/2016
 */

#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <ctime>
#include <chrono>
#include <signal.h>
#include <types.h>

using namespace std;

#include "mqtt.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

// Log the event to the mqtt broker
void log(char *from, char *to, uint8_t *rssi) {

	time_t now = time(NULL);
	char mbstr[sizeof "2011-10-08T07:07:09Z"];
	strftime(mbstr, sizeof(mbstr), "%FT%TZ", localtime(&now));

	char text[200];

	log_mqtt(text);
}

struct BLE {
	int hciSocket;
	struct hci_filter newFilter, originalFilter;
	char macaddr[18];

	void open() {
	}

	void close() {
	}

	void get_device() {
	}
};

BLE ble;

// Exit handler for program
void exit_handler(int param) {
	printf("Closing...\n");
	ble.close();
	disconnect_mqtt();
	exit(1);
}

// The main function for the example program
int main() {

	// handles ctrl-c or other orderly exits
	signal(SIGINT, exit_handler);

	// create and initialize BLE
	ble.open();

	// Open MQTT connection
	connect_mqtt();

	for (;;) {
		ble.get_device();
		sleep(1);

	}

	return MRAA_SUCCESS;
}
