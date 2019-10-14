/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "crc8.h"
#include "string.h"

static uint8_t crc8_small_table[16] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
    0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d
};

/*!
 * @brief  CRC8 Init
 *
 * @param[in] void
 *
 * @return  uint8_t
 */


uint8_t crc8_init(void)
{
    return 0xff;
}

/*!
 * @brief  Calcula el CRC8 de un string
 *
 * @param[in] val
 * @param[in] *buf
 * @param[in] *cnt
 *
 * @return  uint8_t
 */


uint8_t crc8_calc(uint8_t val, void *buf, int cnt)
{
	int i;
	uint8_t *p = buf;

	for (i = 0; i < cnt; i++) {
		val ^= p[i];
		val = (val << 4) ^ crc8_small_table[val >> 4];
		val = (val << 4) ^ crc8_small_table[val >> 4];
	}
	return val;
}

/*!
 * @brief  Calcula el CRC8 del string y lo suma en la ultima posicion
 *
 * @param[in] char*
 *
 * @return  char*
 */


char* CalculaCRC8(char *str){

	uint8_t indice;

	uint8_t crc = crc8_calc(0 , str , strlen(str)); // realizo el calculo del CRC8

	indice = strlen(str);						    // calcula el largo del str

	str[indice] = crc;   // le agrega el CRC8
	str[indice+1] = '\0';		    // le agrega el caracter fin de linea

	return str;
}

