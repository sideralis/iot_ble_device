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

#include "mqtt.h"
#include <string.h>
/*
 char MQTT_SERVER[] = "ssl://c00jve.messaging.internetofthings.ibmcloud.com:8883";
 char MQTT_CLIENTID[] = "d:c00jve:edison:58A839004685";
 char MQTT_TOPIC[] = "iot/ble/rssi";
 char MQTT_USERNAME[] = "use-token-auth";
 char MQTT_PASSWORD[] = "pLK!CTeh)me84dMfSU";
 */
/*
 char MQTT_SERVER[]="ssl://lkf111.messaging.internetofthings.ibmcloud.com:8883";
 char MQTT_CLIENTID[]="d:lkf111:edison:edison_intel_up";
 char MQTT_USERNAME[]="use-token-auth";
 char MQTT_PASSWORD[]="QJX)H(jtKbtL@hnbHJ";
 char MQTT_TOPIC[]="iot-2/evt/status/fmt/json";
 */
char MQTT_SERVER[] = "tcp://iot.eclipse.org:1883";
char MQTT_CLIENTID[] = "edison:edison_biot_up";
char MQTT_TOPIC[] = "campusid/edison/rssi";

MQTTClient client;
MQTTClient_deliveryToken token;

void connect_mqtt() {
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_SSLOptions sslOptions = MQTTClient_SSLOptions_initializer;
	int rc;

	rc = MQTTClient_create(&client, MQTT_SERVER, MQTT_CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if (rc != MQTTCLIENT_SUCCESS) {
		std::cerr << "Failed to create MQTT client, exiting" << std::endl;
		exit(rc);
	}

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
//	Only for secure connection
//  conn_opts.username = MQTT_USERNAME;
//  conn_opts.password = MQTT_PASSWORD;

	sslOptions.enableServerCertAuth = false;
	conn_opts.ssl = &sslOptions;

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
		std::cout << "Failed to connect to MQTT server, return code:" << rc << std::endl;
		return;
	}
}

void disconnect_mqtt() {
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
}

void log_mqtt(char *payload) {
	int rc;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;

	char payload2[200];
	sprintf(payload2, "%s", payload);

	int payloadlen = strlen(payload);

	pubmsg.payload = &payload2;
	pubmsg.payloadlen = payloadlen;
	pubmsg.qos = QOS;
	pubmsg.retained = 0;

	rc = MQTTClient_publishMessage(client, MQTT_TOPIC, &pubmsg, &token);
	if (rc != MQTTCLIENT_SUCCESS)
		std::cout << "Error publishing!" << std::endl;
	std::cout << payload << std::endl;

	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
	if (rc != MQTTCLIENT_SUCCESS)
		std::cout << "Error wait for completion!" << std::endl;
}
