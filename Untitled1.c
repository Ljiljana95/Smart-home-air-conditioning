#include <iostream>
#include <stdio.h>
#include <mosquitto.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <string.h>

//gcc main.cpp -L /lib/libmoquitto.so -lmosquitto -lstdc++ -lwiringPi

using namespace std;

double buffer;
int i = 0;
int j = 0;
double const temp= 31;
int prekidac;
int mag;
int stanje;

#define PIN 22

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message){
	if(message->payloadlen){
		if(strcmp(message->topic,"SensorValue--EnO_01834685")==0){
				buffer = atoi((char*)message->payload);
		}
		if(strcmp(message->topic,"SensorValue--EnO_0029E357")==0){
				prekidac = strcmp((char*)message->payload,"BI");
		}
		if(strcmp(message->topic,"SensorValue--EnO_01810977")==0){
				mag = strcmp((char*)message->payload,"closed");
		}
		if(buffer>=temp){
			if(stanje==1){
				if(mag==0){
					cout<<endl<<"Temperatura je visoka, prozor je zatvoren i klima vec radi."<<endl;
				}else{
					stanje=0;
					digitalWrite(PIN,0);
					cout<<endl<<"Temperatura je visoka! Zatvori prozor, gasi se klima!"<<endl;
				}
			}else{
				if(mag==0){
					stanje=1;
					digitalWrite(PIN,1);
					cout<<endl<<"Temperatura je visoka, prozor je zatvoren i klima nije radila, pa se pali!"<<endl;
				}else{
					cout<<endl<<"Temperatura je visoka, prozor je otvoren i klima ne radi.
											"Zatvori prozor da bi upalio klimu!"<<endl;
				}
			}
		}else{
			if(stanje==1){
				if(prekidac==0){
					if(mag==0){
						cout<<endl<<"Temperatura je niska, prozor je zatvoren
												"i klima vec radi jer korisnik zahteva."<<endl;
					}else{
						stanje=0;
						digitalWrite(PIN,0);
						cout<<endl<<"Temperatura je niska. Otvoren je prozor, gasi se klima!"<<endl;
					}
				}else{
					stanje=0;
					digitalWrite(PIN,0);
					delay(5000);
					cout<<endl<<"Temperatura je niska! Klima je radila ali je ugasena jer korisniku ne treba!"<<endl;
				}
			}else{
				if(prekidac==0){
					if(mag==0){
						stanje=1;
						digitalWrite(PIN,1);
						cout<<endl<<"Temperatura je niska, prozor je zatvoren i klima nije radila pa se pali!"<<endl;
					}else{
						cout<<endl<<"Temperatura je niska, prozor je otvoren i klima ne radi.
												"Zatvori prozor da bi upalio klimu!"<<endl;
					}
				}else{
					cout<<endl<<"Temperatura je niska, korisnik ne zahteva klimu i ona ne radi."<<endl;
				}
			}
		}
	}
	fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	int i;
	if(!result){
		/* Subscribe to broker information topics on successful connect. */
		mosquitto_subscribe(mosq, NULL, "SensorValue--EnO_0029E357", 2);
		mosquitto_subscribe(mosq, NULL, "SensorValue--EnO_01834685", 2);
		mosquitto_subscribe(mosq, NULL, "SensorValue--EnO_01810977", 2);
	}else{
		fprintf(stderr, "Connect failed\n");
	}
}

void my_subscribe_callback(struct mosquitto *mosq,void *userdata,int mid,int qos_count,const int *granted_qos)
{
	int i;
	//printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		//printf(", %d", granted_qos[i]);
	}
	printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	/* Pring all log messages regardless of level. */
	//printf("%s\n", str);
}

int main(int argc, char *argv[]){
 	cout << endl << "Program za regulaciju temperature u  prostoriji:" << endl;
	int i;
	char *host = "localhost";
	int port = 1883;
	int keepalive = 60;
	bool clean_session = true;
	struct mosquitto *mosq = NULL;

	if (wiringPiSetup () == -1)
		return 1 ;
	pinMode( PIN , OUTPUT);
	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, clean_session, NULL);
	if(!mosq){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	mosquitto_log_callback_set(mosq, my_log_callback);
	mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_message_callback_set(mosq, my_message_callback);
	mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);

	if(mosquitto_connect(mosq, host, port, keepalive)){
		fprintf(stderr, "Unable to connect.\n");
		return 1;
	}
	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return 0;
}
