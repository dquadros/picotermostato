/**
 * @file sensor.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <cstdio>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico-onewire/api/one_wire.h"

#include "picotermostato.h"

static One_wire one_wire(PIN_SENSOR);

#define MAX_SENSORES 3
static int nSensores;
static rom_address_t sensor[MAX_SENSORES];

// Iniciação dos sensores
void sensorInit () {
	one_wire.init();

	int count = one_wire.find_and_count_devices_on_bus();
	nSensores = 0;
	for (int i = 0; i < count; i++) {
		auto address = One_wire::get_address(i);
		printf("Address: %02x%02x%02x%02x%02x%02x%02x%02x\r\n", address.rom[0], address.rom[1], address.rom[2],
				address.rom[3], address.rom[4], address.rom[5], address.rom[6], address.rom[7]);
		if ((address.rom[0] == FAMILY_CODE_DS18B20) && (nSensores < MAX_SENSORES)) {
			sensor[nSensores] = address;
			one_wire.convert_temperature(address, true, false);
			printf("Temperature: %3.1foC\n", one_wire.temperature(address));
			nSensores++;
		}
	}
}

// Retorna a temperatura atual
int sensorLe() {
	// Trata o caso de nenhum sensor detectado
	if (nSensores == 0) {
		return 0;
	}

	// Dispara a leitura dos sensores
	for (int i = 0; i < nSensores; i++) {
		one_wire.convert_temperature(sensor[i], i == (nSensores-1), false);
	}
	
	// Le os resultados e retorna a média
	float soma = 0.0f;
	for (int i = 0; i < nSensores; i++) {
		float leitura = one_wire.temperature(sensor[i]);
		soma += leitura;
		printf("Temperature: %3.1foC\n", leitura);
	}
	return (int) roundf(soma/nSensores);
}





