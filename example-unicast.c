//Franz Blancaflor
//R00153623

#include "contiki.h"
#include "net/rime/rime.h"

#include "dev/button-sensor.h"

#include "dev/sht11/sht11-sensor.h"

#include "dev/leds.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
//When the node receives a unicast message
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
 
}
/*
//The node sends a unicast message to the broadcast
static void
sent_uc(struct unicast_conn *c, int status, int num_tx)
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(linkaddr_cmp(dest, &linkaddr_null)) {
    return;
  }
  printf("unicast message sent to %d.%d: status %d num_tx %d\n",
    dest->u8[0], dest->u8[1], status, num_tx);
}

//The struct that tells the node what methods to invoke
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {recv_uc};//, sent_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/

//Rolling Average that will be sent to collector
static int temp;

//Holds the averages of the nodes
static int readings[6];

//When the nodes receive a broadcast message
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
    printf("Broadcast message received from %d.%d: '%s'\n",
           from->u8[0], from->u8[1], (char *)packetbuf_dataptr());

    //Converting temp to string
    linkaddr_t destination;

    char str[5];

    sprintf(str, "%d", temp);

    //Copying temp to packet buffer and sending it to the collector
    packetbuf_copyfrom(str, 5);

    destination.u8[0] = from->u8[0];

    destination.u8[1] = from->u8[1];

    if(!linkaddr_cmp(&destination, &linkaddr_node_addr)) 
    {
        unicast_send(&uc, &destination);
    }

}

//The struct that tells the node what methods to invoke
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();

  //Open broadcast connection in band 129 and unicast connection in band 135
  broadcast_open(&broadcast, 129, &broadcast_call);

  unicast_open(&uc, 135, &unicast_callbacks);	

  //Counter for the loop
  int i = 0;

  //Populate arrays with 0's
  for(i = 0; i < 6; i++)
  {
	  readings[i] = 0;
  }

  while(1) 
  {
   static struct etimer et;

   linkaddr_t addr;

   //Activate sensors
   SENSORS_ACTIVATE(sht11_sensor);

   etimer_set(&et, CLOCK_SECOND);

   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

   //Gets the temperature
   int val = sht11_sensor.value(SHT11_SENSOR_TEMP);
 
   //Makes it into Celcius
   temp = (-39.60 + 0.01*val);

   //Push out the first value and makes the last index free to put new value
   for( i = 0; i < 6; i++)
    {
      if((i+1) < 6){
      readings[i] = readings[i+1];
    }
 
    }
      readings[5] = temp;

   val = 0;

   //Gets the sum
   for( i = 0; i < 6; i++)
    {
      val = val + readings[i];
    }

   //Gets the average
   temp = val/6;

   etimer_reset(&et);
  
   SENSORS_DEACTIVATE(sht11_sensor);
}

PROCESS_END();
}
/*---------------------------------------------------------------------------*/
