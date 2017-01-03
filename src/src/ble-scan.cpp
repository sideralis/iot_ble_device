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

// Log the event to the remote datastore
void log(char *from, char *to, uint8_t *rssi) {

	time_t now = time(NULL);
	char mbstr[sizeof "2011-10-08T07:07:09Z"];
	strftime(mbstr, sizeof(mbstr), "%FT%TZ", localtime(&now));

	char text[200];
	sprintf(text, "{\"from\": \"%s\", \"to\": \"%s\", \"rssi\": %i, \"date\": \"%s\"}", from, to, *rssi, mbstr);

	//printf("%s\n", text);
	log_mqtt(text);
}

struct BLE {
	int hciSocket;
	struct hci_filter newFilter, originalFilter;
	char macaddr[18];

	void open() {
		int status, ret;
		bdaddr_t btaddr;

		hciSocket = hci_open_dev(0);

		if (hciSocket < 0) {
			printf("Failed to open BLE device!\n");
			return;
		}
		// =======================
		// === Get MAC address ===
		// =======================
		if (hci_devba(0, &btaddr) < 0) {
			printf("Failed to get MAC address!\n");
			return;
		}
		ba2str(&btaddr, macaddr);
		printf("MAC address = %s\n", macaddr);

		// =======================
		// === Set advertising ===
		// =======================
		le_set_advertising_parameters_cp adv_params_cp;
		le_set_advertising_data_cp adv_data_cp;
		struct hci_request adv_params_rq;
		struct hci_request adv_data_rq;
		struct hci_request enable_adv_rq;
		le_set_advertise_enable_cp advertise_cp;

		memset(&adv_params_cp, 0, sizeof(adv_params_cp));
		adv_params_cp.min_interval = htobs(0x0800); /* 0x800 * 0.625ms = 1,280s */
		adv_params_cp.max_interval = htobs(0x0800); /* 0x800 * 0.625ms = 1,280s */
		adv_params_cp.chan_map = 7;

		memset(&adv_params_rq, 0, sizeof(adv_params_rq));
		adv_params_rq.ogf = OGF_LE_CTL;
		adv_params_rq.ocf = OCF_LE_SET_ADVERTISING_PARAMETERS;
		adv_params_rq.cparam = &adv_params_cp;
		adv_params_rq.clen = LE_SET_ADVERTISING_PARAMETERS_CP_SIZE;
		adv_params_rq.rparam = &status;
		adv_params_rq.rlen = 1;

		ret = hci_send_req(hciSocket, &adv_params_rq, 1000);
		if (ret < 0) {
			hci_close_dev(hciSocket);
			perror("Failed to set advertisement parameters data.");
			return;
		}

		char name[] = "Edison";
		int name_len = strlen(name);

		memset(&adv_data_cp, 0, sizeof(adv_data_cp));

		// Build simple advertisement data bundle according to:
		// - ​"Core Specification Supplement (CSS) v5"
		// ( https://www.bluetooth.org/en-us/specification/adopted-specifications )

		adv_data_cp.data[0] = 0x02; // Length.
		adv_data_cp.data[1] = 0x01; // Flags field.
		adv_data_cp.data[2] = 0x01; // LE Limited Discoverable Flag set

		adv_data_cp.data[3] = name_len + 1; // Length.
		adv_data_cp.data[4] = 0x09; // Name field.
		memcpy(adv_data_cp.data + 5, name, name_len);

		adv_data_cp.length = strlen((const char *) adv_data_cp.data);

		memset(&adv_data_rq, 0, sizeof(adv_data_rq));
		adv_data_rq.ogf = OGF_LE_CTL;
		adv_data_rq.ocf = OCF_LE_SET_ADVERTISING_DATA;
		adv_data_rq.cparam = &adv_data_cp;
		adv_data_rq.clen = LE_SET_ADVERTISING_DATA_CP_SIZE;
		adv_data_rq.rparam = &status;
		adv_data_rq.rlen = 1;

		ret = hci_send_req(hciSocket, &adv_data_rq, 1000);
		if (ret < 0) {
			hci_close_dev(hciSocket);
			perror("Failed to set advertising data.");
			return;
		}

		memset(&advertise_cp, 0, sizeof(advertise_cp));
		advertise_cp.enable = 0x01;

		memset(&enable_adv_rq, 0, sizeof(enable_adv_rq));
		enable_adv_rq.ogf = OGF_LE_CTL;
		enable_adv_rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
		enable_adv_rq.cparam = &advertise_cp;
		enable_adv_rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
		enable_adv_rq.rparam = &status;
		enable_adv_rq.rlen = 1;

		ret = hci_send_req(hciSocket, &enable_adv_rq, 1000);
		if (ret < 0) {
			hci_close_dev(hciSocket);
			perror("Failed to enable advertising.");
		}

		// ===================
		// == Set scanning ===
		// ===================

		// save original filter
		socklen_t originalFilterLen = sizeof(originalFilter);
		getsockopt(hciSocket, SOL_HCI, HCI_FILTER, &originalFilter, &originalFilterLen);

		// setup new filter
		hci_filter_clear(&newFilter);
		hci_filter_set_ptype(HCI_EVENT_PKT, &newFilter);
		hci_filter_set_event(EVT_LE_META_EVENT, &newFilter);
		setsockopt(hciSocket, SOL_HCI, HCI_FILTER, &newFilter, sizeof(newFilter));

		// disable scanning just in case scanning was already happening,
		// otherwise hci_le_set_scan_parameters call will fail
		hci_le_set_scan_enable(hciSocket, 0x00, 0, 1000);

		// set scan params
		hci_le_set_scan_parameters(hciSocket, 0x01, htobs(0x0010), htobs(0x0010), 0x00, 0, 1000);

		// start scanning
		hci_le_set_scan_enable(hciSocket, 0x00, 0, 1000);
		hci_le_set_scan_enable(hciSocket, 0x01, 0, 1000);

		printf("Scanning ...\n");
	}

	void close() {
		// put back original filter
		setsockopt(hciSocket, SOL_HCI, HCI_FILTER, &originalFilter, sizeof(originalFilter));

		// stop scanning
		hci_le_set_scan_enable(hciSocket, 0x00, 0, 1000);

		// close BLE adaptor
		hci_close_dev(hciSocket);
	}

	string get_device() {
		unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
		int len;

		evt_le_meta_event *meta;
		char addr[18];
		uint8_t *rssi;

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
			return "error";
		}

		len = read(hciSocket, buf, sizeof(buf));
		if (len == 0) {
			return "";
		}

		ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
		len -= (1 + HCI_EVENT_HDR_SIZE);

		meta = (evt_le_meta_event *) ptr;

		if (meta->subevent != 0x02)
			return "error";

		int num_reports = meta->data[0];
		int i, j;
		uint8_t *data;
		uint8_t k;
		data = (uint8_t *) (meta->data + 1);
		//ba2str((const bdaddr_t*)(data+2), addr);
		for (i = 0; i < num_reports; i++) {
			//printf("\n===== %i =====\n", i);
			//printf(" Event type = %i\n", *data);
			data++;
			//printf(" Address type = %i\n", *data);
			data++;
			//printf(" Address =");
			ba2str((const bdaddr_t*) data, addr);
			for (j = 0; j < 6; j++) {
				//printf("%02x:",*data);
				data++;
			}
			k = *data++;
			if (k) {
				//printf("\n Data nb = %i\n", k);
				//printf(" Data = ");
				for (j = 0; j < k; j++) {
					//printf("%c",*data);
					data++;
				}
			}
			rssi = data;
			//printf("\n RSSI = %i\n", *data);
			data++;
			log(macaddr, addr, rssi);

		}
		return addr;
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
	string address;

	// handles ctrl-c or other orderly exits
	signal(SIGINT, exit_handler);

	// create and initialize UPM devices
	ble.open();

	// Open MQTT connection
	connect_mqtt();

	for (;;) {
		address = ble.get_device();
		sleep(1);

	}

	return MRAA_SUCCESS;
}
