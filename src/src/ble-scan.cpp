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
	/* TP: Formatez la variable text au format JSON */

	log_mqtt(text);
}

struct BLE {
	int hciSocket;
	struct hci_filter newFilter, originalFilter;
	char macaddr[18];

	void open() {
		/* TP: Recuperation de l'adresse MAC de notre device Bluetooth */


		/* TP: Activer le mode advertising de notre device Bluetooth */


		socklen_t originalFilterLen = sizeof(originalFilter);
		getsockopt(hciSocket, SOL_HCI, HCI_FILTER, &originalFilter, &originalFilterLen); // save original filter
		hci_filter_clear(&newFilter);
		hci_filter_set_ptype(HCI_EVENT_PKT, &newFilter);
		hci_filter_set_event(EVT_LE_META_EVENT, &newFilter);
		setsockopt(hciSocket, SOL_HCI, HCI_FILTER, &newFilter, sizeof(newFilter)); // setup new filter
		/* TP: Activer le scanning de notre device Bluetooth */


	}

	void close() {
		// put back original filter
		setsockopt(hciSocket, SOL_HCI, HCI_FILTER, &originalFilter, sizeof(originalFilter));

		// stop scanning
		hci_le_set_scan_enable(hciSocket, 0x00, 0, 1000);

		// close BLE adaptor
		hci_close_dev(hciSocket);
	}

	int get_device() {

		unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
		int len;

		evt_le_meta_event *meta;

		fd_set rfds;
		struct timeval tv;
		int selectRetval;

		FD_ZERO(&rfds);
		FD_SET(hciSocket, &rfds);

		tv.tv_sec = 2;
		tv.tv_usec = 0;

		// wait to see if we have data on the socket
		selectRetval = select(hciSocket + 1, &rfds, NULL, NULL, &tv);

		if (-1 == selectRetval) {
			return -1;
		}

		len = read(hciSocket, buf, sizeof(buf));
		if (len == 0) {
			return -1;
		}

		ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
		len -= (1 + HCI_EVENT_HDR_SIZE);

		meta = (evt_le_meta_event *) ptr;

		if (meta->subevent != 0x02)
			return -1;

		/* TP: Recuperez les infos adresse mac et valeur rssi du device detecte a partir du contenu pointe par meta->data */
		//int num_reports = meta->data[0];


		/* TP: Appelez log() avec les bons parametres */



		return 0;
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
