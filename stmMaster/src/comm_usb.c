
/* Includes ------------------------------------------------------------------*/
#include "comm_usb.h"
#include "usbd_cdc_core.h"
#include "usbd_req.h"
#include <string.h>
#include "main.h"

extern volatile uint32_t UsbControl;
extern volatile uint8_t running;

volatile uint8_t UsbTxNext = 0;
extern volatile uint8_t tx_data[FRAME_SIZE];			// send data. Fix format
//extern volatile uint8_t UsbState;
extern volatile uint8_t USB_buffer[USB_NUM_BUFFERS][USB_BUFFER_SIZE];								// Reception buffer

uint8_t UsbConfig = 0;

/**
  * @brief  usbd_cdc_Init
  *         Initilaize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t usbd_cdc_Init (void  *pdev,
                               uint8_t cfgidx)
{
  uint8_t *pbuf;

  /* Open EP IN */  
  DCD_EP_Open(pdev,
              CDC_IN_EP,
              CDC_DATA_IN_PACKET_SIZE,
              USB_OTG_EP_BULK);
   
  /* Open EP OUT  */  
  DCD_EP_Open(pdev,
              CDC_OUT_EP,
              CDC_DATA_OUT_PACKET_SIZE,
              USB_OTG_EP_BULK);

  /* Open Command IN EP */  
  DCD_EP_Open(pdev,
              CDC_CMD_EP,
              CDC_CMD_PACKET_SZE,
              USB_OTG_EP_INT);

  pbuf = (uint8_t *)USBD_DeviceDesc;
  pbuf[4] = DEVICE_CLASS_CDC;
  pbuf[5] = DEVICE_SUBCLASS_CDC;

  /* Initialize the Interface physical components */
  //DCD_Init(pdev, USB_OTG_FS_CORE_ID);
  //APP_FOPS.pIf_Init();

  /* Prepare Out endpoint to receive next packet */
  DCD_EP_PrepareRx(pdev,
                   CDC_OUT_EP,
                   (uint8_t*)(APP_Rx_Buffer),
                   CDC_DATA_OUT_PACKET_SIZE);

  return USBD_OK;
}

/**
  * @brief  usbd_cdc_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  usbd_cdc_DeInit (void  *pdev,
                                 uint8_t cfgidx)
{
  /* Open EP IN */
  DCD_EP_Close(pdev,
              CDC_IN_EP);

  /* Open EP OUT */
  DCD_EP_Close(pdev,
              CDC_OUT_EP);

  /* Open Command IN EP */
  DCD_EP_Close(pdev,
              CDC_CMD_EP);

  /* Restore default state of the Interface physical components */
  //APP_FOPS.pIf_DeInit();

  return USBD_OK;
}

/**
  * @brief  usbd_cdc_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  usbd_cdc_Setup (void  *pdev,
                                USB_SETUP_REQ *req)
{
  uint16_t len;
  uint8_t  *pbuf;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    /* CDC Class Requests -------------------------------*/
  case USB_REQ_TYPE_CLASS :
      /* Check if the request is a data setup packet */
      if (req->wLength)
      {
        /* Check if the request is Device-to-Host */
        if (req->bmRequest & 0x80)
        {
          /* Get the data to be sent to Host from interface layer */
        	//APP_FOPS.pIf_Ctrl(req->bRequest, CmdBuff, req->wLength);
          /*
           *
		  for(len=0;len<64;len++)
        	  CmdBuff[len] = len + 1;
          req->wLength = 64;
           */
          /* Send the data to the host */
          USBD_CtlSendData (pdev,
                            CmdBuff,
                            req->wLength);
        }
        else /* Host-to-Device requeset */
        {
          /* Set the value of the current command to be processed */
          cdcCmd = req->bRequest;
          cdcLen = req->wLength;

          /* Prepare the reception of the buffer over EP0
          Next step: the received data will be managed in usbd_cdc_EP0_TxSent()
          function. */
          USBD_CtlPrepareRx (pdev,
                             CmdBuff,
                             req->wLength);
        }
      }
      else /* No Data request */
      {
        /* Transfer the command to the interface layer */
        //APP_FOPS.pIf_Ctrl(req->bRequest, NULL, 0);
      }

      return USBD_OK;

    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL;



    /* Standard Requests -------------------------------*/
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
      if( (req->wValue >> 8) == CDC_DESCRIPTOR_TYPE)
      {
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        pbuf = usbd_cdc_Desc;
#else
        pbuf = usbd_cdc_CfgDesc + 9 + (9 * USBD_ITF_MAX_NUM);
#endif
        len = MIN(USB_CDC_DESC_SIZ , req->wLength);
      }

      USBD_CtlSendData (pdev,
                        pbuf,
                        len);
      break;

    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&usbd_cdc_AltSet,
                        1);
      break;

    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM)
      {
        usbd_cdc_AltSet = (uint8_t)(req->wValue);
      }
      else
      {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
      }
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  usbd_cdc_EP0_RxReady
  *         Data received on control endpoint
  * @param  pdev: device device instance
  * @retval status
  */
static uint8_t  usbd_cdc_EP0_RxReady (void  *pdev)
{
  if (cdcCmd != NO_CMD)
  {
    /* Process the data */
    // APP_FOPS.pIf_Ctrl(cdcCmd, CmdBuff, cdcLen);


    /* Reset the command variable to default value */
    cdcCmd = NO_CMD;
  }

  return USBD_OK;
}

/**
  * @brief  usbd_audio_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_cdc_DataIn (void *pdev, uint8_t epnum)
{
	uint16_t USB_Tx_ptr;
	uint16_t USB_Tx_length;
	uint8_t i;

	if (USB_Tx_State == 1)
	{
		if (APP_Tx_length == 0)	{
			if (running) {
				UsbControl 			&= ~((uint32_t)0x01 << UsbTxNext);
				for(i=0; i<USB_NUM_BUFFERS; i++)
				{
					if(UsbControl & ((uint32_t)0x01 <<  i))
					{
						APP_Tx_length = USB_TRANSF_SIZE;
						APP_Tx_ptr_idx = 4;
						UsbTxNext = i;
						break;
					}
				}
			}
			else
			{
				if (UsbConfig == 1)
				{
					APP_Tx_Buffer[ 0] = 0xFF;
					APP_Tx_Buffer[ 1] = 0x03;
					APP_Tx_Buffer[ 2] = SLAVE_QTY;
					APP_Tx_Buffer[ 3] = CAPTURE_SCAN >> 8;
					APP_Tx_Buffer[ 4] = CAPTURE_SCAN & 0xFF;
					APP_Tx_Buffer[ 5] = 2;	// Not Used
					APP_Tx_Buffer[ 6] = CAPTURE_LEN >> 8;
					APP_Tx_Buffer[ 7] = CAPTURE_LEN & 0xFF;
					APP_Tx_Buffer[ 8] = RCV_TIMEOUT >> 8;
					APP_Tx_Buffer[ 9] = RCV_TIMEOUT & 0xFF;
					APP_Tx_Buffer[10] = 0xFF;
					APP_Tx_Buffer[11] = 0xFF;
					APP_Tx_Buffer[12] = 0xFF;
					APP_Tx_Buffer[13] = 0xFF;
					APP_Tx_Buffer[14] = 0xFF;
					APP_Tx_Buffer[15] = 0xFF;
					APP_Tx_Buffer[16] = 0xEE;
					APP_Tx_Buffer[17] = 0x55;
					APP_Tx_length = 18;
					APP_Tx_ptr_idx = 0;
					UsbConfig = 0;
				}
			}
		}
		if (APP_Tx_length == 0)	{
			USB_Tx_State = 0;
		}
		else {
			USB_Tx_ptr = APP_Tx_ptr_idx;
			if (APP_Tx_length > CDC_DATA_IN_PACKET_SIZE) {
				USB_Tx_length = CDC_DATA_IN_PACKET_SIZE;
			}
			else {
				USB_Tx_length = APP_Tx_length;
			}
			APP_Tx_ptr_idx += USB_Tx_length;
			APP_Tx_length -= USB_Tx_length;
			/* Prepare the available data buffer to be sent on IN endpoint */
			if(running)
				DCD_EP_Tx (pdev, CDC_IN_EP, (uint8_t*)&USB_buffer[UsbTxNext][USB_Tx_ptr], USB_Tx_length);
			else
				DCD_EP_Tx (pdev, CDC_IN_EP, (uint8_t*)&APP_Tx_Buffer[USB_Tx_ptr], USB_Tx_length);
		}
	}

	return USBD_OK;
}

/**
  * @brief  usbd_audio_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
// Data from Computer to STM32
static uint8_t  usbd_cdc_DataOut (void *pdev, uint8_t epnum)
{
	uint16_t USB_Rx_Cnt;

	/* Get the received data buffer and update the counter */
	USB_Rx_Cnt = ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].xfer_count;

	/* USB data will be immediately processed, this allow next USB traffic being
	 NAKed till the end of the application Xfer */

	// Treatment of received data
	if ((USB_Rx_Cnt >= 4) && (APP_Rx_Buffer[0]==0xF1))
	{
		switch(APP_Rx_Buffer[1])
		{
		case 0x01:		// Acquisition Start
			if ((USB_Rx_Cnt == 5) && !UsbConfig)
			{
				GPIOC->BSRRL = GPIO_Pin_8;
				if(!running)
				{

					if (APP_Rx_Buffer[2])
					{
						TIM1->PSC = (256*84) - 1;				// Calibration
						tx_data[4] = 0x01;
					}
					else
					{
						TIM1->PSC	= 84 - 1;				// Normal operation
						tx_data[4] 	= 0x00;
					}
					running = 1;
					//DMA2_Stream5->CR	|= (uint32_t)DMA_SxCR_EN;			// Enable Receiving DMA
				}
			}
			break;
		case 0x02:		// Acquisition Stop
			if ((USB_Rx_Cnt == 4) && (running & 1))
			{
				running = 2;
			}
			break;
		case 0x03:		// Configuration Read
			if (!running && (USB_Rx_Cnt == 4))
			{
				UsbConfig = 1;
			}

		}
	}

	/* Prepare Out endpoint to receive next packet */
	DCD_EP_PrepareRx(pdev, CDC_OUT_EP, (uint8_t*)(APP_Rx_Buffer), CDC_DATA_OUT_PACKET_SIZE);

	return USBD_OK;
}

/**
  * @brief  usbd_audio_SOF
  *         Start Of Frame event management
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_cdc_SOF (void *pdev)
{
	static uint32_t FrameCount = 0;


	if (FrameCount++ >= CDC_IN_FRAME_INTERVAL)
	{

		/* Reset the frame counter */
		FrameCount = 0;

		/* Check the data to be sent through IN pipe */
		Handle_USBAsynchXfer(pdev);
	}

	return USBD_OK;
}

/**
  * @brief  Handle_USBAsynchXfer
  *         Send data to USB
  * @param  pdev: instance
  * @retval None
  */
static void Handle_USBAsynchXfer (void *pdev)
{
	uint16_t USB_Tx_ptr;
	uint16_t USB_Tx_length;
	uint8_t i;

	if(USB_Tx_State != 1) {
		if(running)
		{
			if (!APP_Tx_length) {
				for(i=0; i<USB_NUM_BUFFERS; i++)
				{
					if(UsbControl & ((uint32_t)0x01 <<  i))
					{
						APP_Tx_length = USB_TRANSF_SIZE;
						APP_Tx_ptr_idx = 4;
						UsbTxNext = i;
						break;
					}
				}

			}
		}
		else
		{
			if (UsbConfig == 1)
			{
				APP_Tx_Buffer[ 0] = 0xFF;
				APP_Tx_Buffer[ 1] = 0x03;
				APP_Tx_Buffer[ 2] = SLAVE_QTY;
				APP_Tx_Buffer[ 3] = CAPTURE_SCAN & 0xFF;
				APP_Tx_Buffer[ 4] = CAPTURE_SCAN >> 8;
				APP_Tx_Buffer[ 5] = 2;	// Not Used
				APP_Tx_Buffer[ 6] = CAPTURE_LEN & 0xFF;
				APP_Tx_Buffer[ 7] = CAPTURE_LEN >> 8;
				APP_Tx_Buffer[ 8] = RCV_TIMEOUT & 0xFF;
				APP_Tx_Buffer[ 9] = RCV_TIMEOUT >> 8;
				APP_Tx_Buffer[10] = 0xFF;
				APP_Tx_Buffer[11] = 0xFF;
				APP_Tx_Buffer[12] = 0xFF;
				APP_Tx_Buffer[13] = 0xFF;
				APP_Tx_Buffer[14] = 0xFF;
				APP_Tx_Buffer[15] = 0xFF;
				APP_Tx_Buffer[16] = 0xEE;
				APP_Tx_Buffer[17] = 0x55;
				APP_Tx_ptr_idx = 0;
				APP_Tx_length = 18;
				UsbConfig = 0;
			}
		}

		if (!APP_Tx_length)
			return;


		USB_Tx_ptr = APP_Tx_ptr_idx;
		if (APP_Tx_length > CDC_DATA_IN_PACKET_SIZE)
		{
			USB_Tx_length = CDC_DATA_IN_PACKET_SIZE;
		}
		else
		{
			USB_Tx_length = APP_Tx_length;
		}
		APP_Tx_ptr_idx += USB_Tx_length;
		APP_Tx_length -= USB_Tx_length;

		USB_Tx_State = 1;

		if(running)
			DCD_EP_Tx (pdev, CDC_IN_EP, (uint8_t*)&USB_buffer[UsbTxNext][USB_Tx_ptr], USB_Tx_length);
		else
			DCD_EP_Tx (pdev, CDC_IN_EP, (uint8_t*)&APP_Tx_Buffer[USB_Tx_ptr], USB_Tx_length);
	}

}

/**
  * @brief  USBD_cdc_GetCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_cdc_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (usbd_cdc_CfgDesc);
  return usbd_cdc_CfgDesc;
}

/**
  * @brief  USBD_cdc_GetCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
#ifdef USE_USB_OTG_HS
static uint8_t  *USBD_cdc_GetOtherCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (usbd_cdc_OtherCfgDesc);
  return usbd_cdc_OtherCfgDesc;
}
#endif

