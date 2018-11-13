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
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  printf("unicast message received from %d.%d\n",
	 from->u8[0], from->u8[1]);
}

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

/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/

static int tempVal;

static int tempVals[4];
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
    printf("broadcast message received from %d.%d: '%s'\n",
           from->u8[0], from->u8[1], (char *)packetbuf_dataptr());

    //Sending the temperature to the sink

    linkaddr_t destination;
    char str[10];
    sprintf(str, "%d", tempVal);
    packetbuf_copyfrom(str, 5);
    destination.u8[0] = from->u8[0];
    destination.u8[1] = from->u8[1];
    if(!linkaddr_cmp(&destination, &linkaddr_node_addr)) {
        unicast_send(&uc, &destination);
    }

}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;


PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  unicast_open(&uc, 135, &unicast_callbacks);	

  while(1) {
    static struct etimer et;

    linkaddr_t addr;

    SENSORS_ACTIVATE(sht11_sensor);
    
    etimer_set(&et, CLOCK_SECOND);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    int val = sht11_sensor.value(SHT11_SENSOR_TEMP);
    
    tempVal = (-39.60 + 0.01*val);

    }
    
    etimer_reset(&et);

    SENSORS_DEACTIVATE(sht11_sensor);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
