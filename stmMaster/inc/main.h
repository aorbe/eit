/*
 * main.h
 *
 *  Created on: 20/03/2014
 *      Author: asantos
 */

#ifndef MAIN_H_
#define MAIN_H_

#undef _SIMULATION

#define DMA_RECEPTION
#undef DMA_CAPTURE


#define CAPTURE_LEN			1020

#define SERIAL_BAUD			5250000
#define SLAVE_QTY			8

#define SIZE_RCV_BUFFER		32
#define SIZE_SND_BUFFER		12

// 500 => 161.200 Kbytes/s
// 400 => 201.500 Kbytes/s
// 200 => 403.000 Kbytes/s
#define SCAN_PERIOD			625			// 2.5e6 / 1024
#define	RCV_TIMEOUT			180			// SCAN_PERIOD / 2

#define CAPTURE_SCAN		625			// 625us -> 50 image/s @ 32 electrode

#define MAX_FRAMES			32			// 256 / SLAVE_QTY
#define FRAME_SIZE			90			// 10 * SLAVE_QTY + 10

#define MODE				tx_data[4]

/*
#define USB_NUM_BUFFERS		6
#define USB_FULL_BUFFER		0x3F
#define USB_BUFFER_SIZE		20492	// 2^N + 12
#define USB_BUFFER_INIT		20320	// USB_BUFFER_SIZE - FRAME_SIZE
#define USB_TRANSF_SIZE		20488
#define USB_DATA_SIZE		80
*/

#define USB_NUM_BUFFERS		6
#define USB_FULL_BUFFER		0x3F
#define USB_BUFFER_SIZE		10252	// 2^N + 12
#define USB_BUFFER_INIT		10160	// USB_BUFFER_SIZE - FRAME_SIZE
#define USB_TRANSF_SIZE		10248
#define USB_DATA_SIZE		80

#endif /* MAIN_H_ */
