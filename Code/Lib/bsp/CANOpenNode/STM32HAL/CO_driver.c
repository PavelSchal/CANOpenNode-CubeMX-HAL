/*
 * \brief CAN module object for generic STM microcontroller.
 *
 * \details CanOpenNode driver implementation based on STMCubeMX HAL.
 *
 * @file        CO_driver.c
 * @ingroup     CO_driver
 * @author      Janez Paternoster
 * @author      Andrii Shylenko
 * @copyright   2004 - 2015 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * CANopenNode is free and open source software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Following clarification and special exception to the GNU General Public
 * License is included to the distribution terms of CANopenNode:
 *
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library. Thus, the terms and
 * conditions of the GNU General Public License cover the whole combination.
 *
 * As a special exception, the copyright holders of this library give
 * you permission to link this library with independent modules to
 * produce an executable, regardless of the license terms of these
 * independent modules, and to copy and distribute the resulting
 * executable under terms of your choice, provided that you also meet,
 * for each linked independent module, the terms and conditions of the
 * license of that module. An independent module is a module which is
 * not derived from or based on this library. If you modify this
 * library, you may extend this exception to your version of the
 * library, but you are not obliged to do so. If you do not wish
 * to do so, delete this exception statement from your version.
 */

/*-----------------------------------------------------------------------------
 * INCLUDE SECTION
 *----------------------------------------------------------------------------*/
#include "can.h" /* Include HAL interfaces generated by cube MX. */
#include "CO_driver.h"
#include "CO_Emergency.h"

/*-----------------------------------------------------------------------------
 * LOCAL (static) DEFINITIONS
 *----------------------------------------------------------------------------*/
/*\brief pointer to CO_CanModule used in CubeMX CAN Rx interrupt routine*/
static CO_CANmodule_t* RxFifo_Callback_CanModule_p = NULL;
/*-----------------------------------------------------------------------------
 * LOCAL FUNCTION PROTOTYPES
 *----------------------------------------------------------------------------*/
static void prepareTxHeader(CAN_TxHeaderTypeDef *TxHeader, CO_CANtx_t *buffer);

/*-----------------------------------------------------------------------------
 * LOCAL FUNCTIONS
 *----------------------------------------------------------------------------*/

/*!*****************************************************************************
 * \author
 * \date 	10.11.2018
 *
 * \brief prepares CAN Tx header based on the ID, RTR and data count.
 * \param [in]	TxHeader pointer to @CAN_TxHeaderTypeDef object
 * \param [in]	buffer ponyer to CO_CANtx_t with CANopen configuration data
 *
 * \ingroup CO_driver
 ******************************************************************************/
void prepareTxHeader(CAN_TxHeaderTypeDef *TxHeader, CO_CANtx_t *buffer)
{
    /* Map buffer data to the HAL CAN tx header data*/
    TxHeader->ExtId = 0u;
    TxHeader->DLC = buffer->DLC;
    TxHeader->StdId = ( buffer->ident >> 2 );
    TxHeader->RTR = ( buffer->ident & 0x2 );
}

/*-----------------------------------------------------------------------------
 * GLOBAL FUNCTIONS - see descriptions in header file
 *----------------------------------------------------------------------------*/

//TODO move callbacks to the CO_driver.c and implement callback init routine

/* \brief 	Cube MX callbacks for Fifo0 and Fifo1
 * \details It is assumed that only one CANmodule is (CO->CANmodule[0]) is used.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(RxFifo_Callback_CanModule_p != NULL)
	{
		CO_CANinterrupt_Rx(RxFifo_Callback_CanModule_p);
	}
	else
	{
		;/*TODO add assert, according to Cube MX driver we should not be here
		  *but for some reason interrupts get activated as soon as HAL_NVIC_EnableIRQ is called.
		  *According to Cube CAN docs HAL_CAN_ActivateNotification should be executed to
		  *activate callbacks.
		  */
	}
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(RxFifo_Callback_CanModule_p != NULL)
	{
		CO_CANinterrupt_Rx(RxFifo_Callback_CanModule_p);
	}
	else
	{
		;//TODO add assert here
	}
}

void CO_CANsetConfigurationMode(int32_t CANbaseAddress){
    /* Put CAN module in configuration mode */
	/* HAL is responsible for that */
}

/******************************************************************************/
CO_ReturnError_t CO_CANsetNormalMode(CO_CANmodule_t *CANmodule){
    /* Put CAN module in normal mode */

	CO_ReturnError_t Error = CO_ERROR_NO;
	if(HAL_CAN_Start(CANmodule->CANbaseAddress) != HAL_OK)
	{
	    /* Start Error */
		Error = CO_ERROR_HAL;
	}

    /* Enable CAN interrupts */
	if(HAL_CAN_ActivateNotification( CANmodule->CANbaseAddress,
									 CAN_IT_RX_FIFO0_MSG_PENDING |
									 CAN_IT_RX_FIFO1_MSG_PENDING |
									 CAN_IT_TX_MAILBOX_EMPTY)
									 != HAL_OK)
	{
		/* Notification Error */
		Error = CO_ERROR_HAL;
	}

    CANmodule->CANnormal = true;
    return Error;
}

/******************************************************************************/
CO_ReturnError_t CO_CANmodule_init(
        CO_CANmodule_t         *CANmodule,
		CAN_HandleTypeDef      *HALCanObject,
        CO_CANrx_t              rxArray[],
        uint16_t                rxSize,
        CO_CANtx_t              txArray[],
        uint16_t                txSize,
        uint16_t                CANbitRate)
{
    uint16_t i;

    /* verify arguments */
    if(CANmodule==NULL || rxArray==NULL || txArray==NULL)
    {
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }
    else
    {
    	;//do nothing
    }

    RxFifo_Callback_CanModule_p = CANmodule;

    /* Configure object variables */
    CANmodule->CANbaseAddress = (CAN_HandleTypeDef*)HALCanObject;
    CANmodule->rxArray = rxArray;
    CANmodule->rxSize = rxSize;
    CANmodule->txArray = txArray;
    CANmodule->txSize = txSize;
    CANmodule->CANnormal = false;
    CANmodule->useCANrxFilters = false;
    CANmodule->bufferInhibitFlag = false;
    CANmodule->firstCANtxMessage = true;
    CANmodule->CANtxCount = 0U;
    CANmodule->errOld = 0U;
    CANmodule->em = NULL;

    for(i=0U; i<rxSize; i++)
    {
        rxArray[i].ident = 0U;
        rxArray[i].pFunct = NULL;
    }

    for(i=0U; i<txSize; i++)
    {
        txArray[i].bufferFull = false;
    }

    /* Configure CAN module registers */
    /* Configuration is handled by CubeMX HAL*/
	HAL_CAN_Stop(CANmodule->CANbaseAddress);
    //HAL_CAN_MspInit(CANmodule->CANbaseAddress); /* NVIC and GPIO */

    CANmodule->CANbaseAddress->Instance = CAN1;
    CANmodule->CANbaseAddress->Init.Mode = CAN_MODE_NORMAL;
    CANmodule->CANbaseAddress->Init.SyncJumpWidth = CAN_SJW_1TQ;
    CANmodule->CANbaseAddress->Init.TimeTriggeredMode = DISABLE;
    CANmodule->CANbaseAddress->Init.AutoBusOff = DISABLE;
    CANmodule->CANbaseAddress->Init.AutoWakeUp = DISABLE;
    CANmodule->CANbaseAddress->Init.AutoRetransmission = ENABLE;
    CANmodule->CANbaseAddress->Init.ReceiveFifoLocked = DISABLE;
    CANmodule->CANbaseAddress->Init.TransmitFifoPriority = DISABLE;
    CANmodule->CANbaseAddress->Init.TimeSeg2 = CAN_BS2_2TQ;
    CANmodule->CANbaseAddress->Init.TimeSeg1 = CAN_BS1_13TQ;

    /* Can speed configuration. */
    /* Based on the values obtained from http://bittiming.can-wiki.info */
    /* Assuming CAN clock is 80 MHz */

    /*
    Bit	rate	Acurracy	Prescaler   nr. timequanta  Seg.1	Seg.2	Sample point  CAN_BUS_TIME
    1000		0.0000		5			16				13		2		87.5		  0x001c0004
    500		    0.0000		10			16				13		2		87.5		  0x001c0009
    250		 	0.0000		20			16				13		2		87.5		  0x001c0013
    125		    0.0000		40			16				13		2		87.5		  0x001c0027
    100		 	0.0000		50			16				13		2		87.5		  0x001c0031
    50 		 	0.0000		100			16				13		2		87.5		  0x001c0063
    20 		 	0.0000		250			16				13		2		87.5		  0x0007018f
    10 		 	0.0000		500			16				13		2		87.5		  0x001c01f3
    */

    uint32_t Prescaler = 500;

    switch(CANbitRate) {
       case 1000:
    	   Prescaler = 5;
          break;
       case 500:
    	   Prescaler = 10;
          break;
       case 250:
    	   Prescaler = 20;
          break;
       case 125:
    	   Prescaler = 40;
          break;
       case 100:
    	   Prescaler = 50;
          break;
       case 50:
    	   Prescaler = 100;
          break;
       case 20:
    	   Prescaler = 250;
          break;
       case 10:
    	   Prescaler = 500;
          break;

       default :
    	   return  CO_ERROR_ILLEGAL_BAUDRATE;
    }

    CANmodule->CANbaseAddress->Init.Prescaler = Prescaler;

    if (HAL_CAN_Init(CANmodule->CANbaseAddress) != HAL_OK)
    {
    	//_Error_Handler(__FILE__, __LINE__);
    	return CO_ERROR_HAL;
    }

    return CO_ERROR_NO;
}


/******************************************************************************/
void CO_CANmodule_disable(CO_CANmodule_t *CANmodule){
    /* turn off the module */
	/* handled by CubeMX HAL*/
	HAL_CAN_Stop(CANmodule->CANbaseAddress);
}


/******************************************************************************/
uint16_t CO_CANrxMsg_readIdent(const CO_CANrxMsg_t *rxMsg){
    return (uint16_t) rxMsg->RxHeader.StdId;
}


/******************************************************************************/
CO_ReturnError_t CO_CANrxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        uint16_t                mask,
        bool_t                  rtr,
        void                   *object,
        void                  (*pFunct)(void *object, const CO_CANrxMsg_t *message))
{
    CO_ReturnError_t ret = CO_ERROR_NO;

    if((CANmodule!=NULL) && (object!=NULL) && (pFunct!=NULL) && (index < CANmodule->rxSize)){
        /* buffer, which will be configured */
        CO_CANrx_t *buffer = &CANmodule->rxArray[index];

        /* Configure object variables */
        buffer->object = object;
        buffer->pFunct = pFunct;

        /* CAN identifier and CAN mask, bit aligned with CAN module. Different on different microcontrollers. */
        buffer->ident = (ident & 0x07FF) << 2;
        if (rtr)
        {
        	buffer->ident |= 0x02;
        }
        buffer->mask = (mask & 0x07FF) << 2;
        buffer->mask |= 0x02;

        /* Set CAN hardware module filter and mask. */
        if(CANmodule->useCANrxFilters)
        	{
            /* TODO Configure CAN module hardware filters */
        	}
        else
        	{
        	/*no hardware filters*/
        		CAN_FilterTypeDef FilterConfig;

        		FilterConfig.FilterBank = 0;
        		FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
        		FilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
        		FilterConfig.FilterIdHigh = 0x0;
        		FilterConfig.FilterIdLow = 0x0;
        		FilterConfig.FilterMaskIdHigh = 0x0;
        		FilterConfig.FilterMaskIdLow = 0x0;
        		FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
        		FilterConfig.FilterActivation = ENABLE;
        		FilterConfig.SlaveStartFilterBank = 14;

				if(HAL_CAN_ConfigFilter(CANmodule->CANbaseAddress, &FilterConfig)!=HAL_OK)
				{
					return CO_ERROR_HAL;
				}
				else
				{
					;//do nothing
				}
        	}
        }
    else
    {
        ret = CO_ERROR_ILLEGAL_ARGUMENT;
    }
    return ret;
}


/******************************************************************************/
CO_CANtx_t *CO_CANtxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        bool_t                  rtr,
        uint8_t                 noOfBytes,
        bool_t                  syncFlag)
{
    CO_CANtx_t *buffer = NULL;

    if((CANmodule != NULL) && (index < CANmodule->txSize)){
        /* get specific buffer */
        buffer = &CANmodule->txArray[index];

        /* CAN identifier, DLC and rtr, bit aligned with CAN module transmit buffer.*/

        buffer->ident &= 0x7FF;
        buffer->ident = ident << 2;
        if (rtr) buffer->ident |= 0x02;

        buffer->DLC = noOfBytes;
        buffer->bufferFull = false;
        buffer->syncFlag = syncFlag;
    }

    return buffer;
}

/******************************************************************************/
CO_ReturnError_t CO_CANsend(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer)
{
	CO_ReturnError_t err = CO_ERROR_NO;
    static CAN_TxHeaderTypeDef TxHeader;

    /* Verify overflow */
    if(buffer->bufferFull){
        if(!CANmodule->firstCANtxMessage){
            /* don't set error, if bootup message is still on buffers */
            CO_errorReport((CO_EM_t*)CANmodule->em, CO_EM_CAN_TX_OVERFLOW, CO_EMC_CAN_OVERRUN, buffer->ident);
        }
        err = CO_ERROR_TX_OVERFLOW;
    }

    prepareTxHeader(&TxHeader, buffer);

    CO_LOCK_CAN_SEND();
    /* if CAN TX buffer is free, send message */

    uint32_t TxMailboxNum;

    if ((CANmodule->CANtxCount == 0) &&
    	(HAL_CAN_GetTxMailboxesFreeLevel(CANmodule->CANbaseAddress) > 0 )) {
        CANmodule->bufferInhibitFlag = buffer->syncFlag;

        if( HAL_CAN_AddTxMessage(CANmodule->CANbaseAddress,
        						 &TxHeader,
								 &buffer->data[0],
								 &TxMailboxNum)
        						 != HAL_OK)
        {
        	err = CO_ERROR_HAL;
        }
        else
        {
        	;/*do nothing*/
        }
    }
    /* if no buffer is free, message will be sent in the task */
    else
    {
        buffer->bufferFull = true;
        CANmodule->CANtxCount++;
    }
    CO_UNLOCK_CAN_SEND();

    return err;
}


/******************************************************************************/
void CO_CANclearPendingSyncPDOs(CO_CANmodule_t *CANmodule)
{
    uint32_t tpdoDeleted = 0U;

    CO_LOCK_CAN_SEND();
    /* Abort message from CAN module, if there is synchronous TPDO.
     * Take special care with this functionality. */
    /* TODO */

    /*
      if ((state = HAL_CAN_IsTxMessagePending(CANmodule->CANbaseAddress) && (CANmodule->bufferInhibitFlag))
      {
    	HAL_CAN_AbortTxRequest(CANmodule->);
      }
    */

    if(/*messageIsOnCanBuffer && */CANmodule->bufferInhibitFlag){
        /* clear TXREQ */
        CANmodule->bufferInhibitFlag = false;
        tpdoDeleted = 1U;
    }
    /* delete also pending synchronous TPDOs in TX buffers */
    if(CANmodule->CANtxCount != 0U){
        uint16_t i;
        CO_CANtx_t *buffer = &CANmodule->txArray[0];
        for(i = CANmodule->txSize; i > 0U; i--){
            if(buffer->bufferFull){
                if(buffer->syncFlag){
                    buffer->bufferFull = false;
                    CANmodule->CANtxCount--;
                    tpdoDeleted = 2U;
                }
            }
            buffer++;
        }
    }
    CO_UNLOCK_CAN_SEND();


    if(tpdoDeleted != 0U){
        CO_errorReport((CO_EM_t*)CANmodule->em, CO_EM_TPDO_OUTSIDE_WINDOW, CO_EMC_COMMUNICATION, tpdoDeleted);
    }
}


/******************************************************************************/
void CO_CANverifyErrors(CO_CANmodule_t *CANmodule){
    CO_EM_t* em = (CO_EM_t*)CANmodule->em;
    uint32_t HalCanErrorCode = CANmodule->CANbaseAddress->ErrorCode;

    if(CANmodule->errOld != HalCanErrorCode)
    {
        CANmodule->errOld = HalCanErrorCode;
        if(HalCanErrorCode & HAL_CAN_ERROR_BOF)
        {                               /* bus off */
            CO_errorReport(em, CO_EM_CAN_TX_BUS_OFF, CO_EMC_BUS_OFF_RECOVERED, HalCanErrorCode);
        }
        else{                                               /* not bus off */
            CO_errorReset(em, CO_EM_CAN_TX_BUS_OFF, HalCanErrorCode);

            if(HalCanErrorCode & HAL_CAN_ERROR_EWG)
            {     											/* bus warning */
                CO_errorReport(em, CO_EM_CAN_BUS_WARNING, CO_EMC_NO_ERROR, HalCanErrorCode);
            }
            else
            {
            	//do nothing
            }
            if(HalCanErrorCode & HAL_CAN_ERROR_EPV)
            {      											/* TX/RX bus passive */
                if(!CANmodule->firstCANtxMessage)
                {
                    CO_errorReport(em, CO_EM_CAN_TX_BUS_PASSIVE, CO_EMC_CAN_PASSIVE, HalCanErrorCode);
                }
                else
                {
                	//do nothing
                }
            }
            else{
                bool_t isError = CO_isError(em, CO_EM_CAN_TX_BUS_PASSIVE);
                if(isError)
                {
                    CO_errorReset(em, CO_EM_CAN_TX_BUS_PASSIVE, HalCanErrorCode);
                    CO_errorReset(em, CO_EM_CAN_TX_OVERFLOW, HalCanErrorCode);
                }
                else
                {
                	//do nothing
                }
            }

            if(HalCanErrorCode & HAL_CAN_ERROR_NONE)
            {      											 /* no error */
                CO_errorReset(em, CO_EM_CAN_BUS_WARNING, HalCanErrorCode);
            }
            else
            {
            	//do nothing
            }

        }

        if((HalCanErrorCode & HAL_CAN_ERROR_RX_FOV0) || (HalCanErrorCode & HAL_CAN_ERROR_RX_FOV1))
        {                                 					/* CAN RX bus overflow */
            CO_errorReport(em, CO_EM_CAN_RXB_OVERFLOW, CO_EMC_CAN_OVERRUN, HalCanErrorCode);
        }
        else
        {
        	//do nothing
        }
    }
}

/*Interrupt handlers*/
/******************************************************************************/
void CO_CANinterrupt_Rx(const CO_CANmodule_t *CANmodule)
{
    /* receive interrupt */

	static CO_CANrxMsg_t CANmessage;
	bool_t msgMatched = false;
	CO_CANrx_t *MsgBuff = CANmodule->rxArray; /* receive message buffer from CO_CANmodule_t object. */
	HAL_CAN_GetRxMessage(CANmodule->CANbaseAddress, CAN_RX_FIFO0, &CANmessage.RxHeader, &CANmessage.data[0]);

	/*dirty hack, consider change to a pointer here*/
	CANmessage.DLC = (uint8_t)CANmessage.RxHeader.DLC;
	CANmessage.ident = CANmessage.RxHeader.StdId;

    uint32_t index;
    /* Search rxArray form CANmodule for the same CAN-ID. */
    for (index = 0; index < CANmodule->rxSize; index++)
    	{
			uint16_t msg = (((uint16_t)(CANmessage.RxHeader.StdId << 2)) | (uint16_t)(CANmessage.RxHeader.RTR));
			if (((msg ^ MsgBuff->ident) & MsgBuff->mask) == 0)
			{
				msgMatched = true;
				break;
			}
			MsgBuff++;
   	    }

	/* Call specific function, which will process the message */
	if(msgMatched && (MsgBuff != NULL) && (MsgBuff->pFunct != NULL))
	{
		MsgBuff->pFunct(MsgBuff->object, &CANmessage);
	}

//TODO filters handing

//        if(CANmodule->useCANrxFilters){
//            /* CAN module filters are used. Message with known 11-bit identifier has */
//           /* been received */
//            index = rcvMsg;  /* get index of the received message here. Or something similar */
//            if(index < CANmodule->rxSize){
//                buffer = &CANmodule->rxArray[index];
//                /* verify also RTR */
//                if(( RxHeader.RTR ) == CAN_RTR_DATA){
//                    msgMatched = true;
//                }
//                if((( RxHeader. rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U){
//                    msgMatched = true;
//                }
//            }
//        }
//        else{
//            /* CAN module filters are not used, message with any standard 11-bit identifier */
//            /* has been received. Search rxArray form CANmodule for the same CAN-ID. */
//            buffer = &CANmodule->rxArray[0];
//            for(index = CANmodule->rxSize; index > 0U; index--){
//                if(((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U){
//                    msgMatched = true;
//                    break;
//                }
//                buffer++;
//            }
//        }

        /*CubeMx HAL is responsible for clearing interrupt flags and all the dirty work. */
}


void CO_CANpolling_Tx(CO_CANmodule_t *CANmodule)
{
	if (HAL_CAN_GetTxMailboxesFreeLevel((CAN_HandleTypeDef*)CANmodule->CANbaseAddress) > 0)
	{


		/* First CAN message (bootup) was sent successfully */
		CANmodule->firstCANtxMessage = false;
		/* Clear flag from previous message */
		CANmodule->bufferInhibitFlag = false;
		/* Are there any new messages waiting to be send */
		if(CANmodule->CANtxCount > 0U)
		{
			uint16_t i;             /* index of transmitting message */
			CO_LOCK_CAN_SEND();

			/* first buffer */
			CO_CANtx_t *buffer = &CANmodule->txArray[0];
			/* search through whole array of pointers to transmit message buffers. */
			for(i = CANmodule->txSize; i > 0U; i--)
			{
				/* if message buffer is full, send it. */
				if(buffer->bufferFull)
				{

					/* Copy message to CAN buffer */
					CANmodule->bufferInhibitFlag = buffer->syncFlag;
					CAN_TxHeaderTypeDef TxHeader;

					prepareTxHeader(&TxHeader, buffer);

					uint32_t TxMailboxNum;
					CO_LOCK_CAN_SEND();
					if( HAL_CAN_AddTxMessage(CANmodule->CANbaseAddress, &TxHeader, &buffer->data[0], &TxMailboxNum) != HAL_OK)
					{
						;//do nothing
					}
					else
					{
						buffer->bufferFull = false;
						CANmodule->CANtxCount--;
					}
				    CO_UNLOCK_CAN_SEND();
					break;                      /* exit for loop */
				}
				else
				{
					/*do nothing*/;
				}
				buffer++;
			 }/* end of for loop */

		    /* Clear counter if no more messages */
			 if(i == 0U)
			 {
				 CANmodule->CANtxCount = 0U;
			 }
			 else
			 {
				 /*do nothing*/;
			 }
		}
	}
}

