#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

#define vendorID 0x1111
#define productID 0x2222

#define N_ENDPOINT 3
#define TIMEOUTREAD 500
#define SIZE2RECIEVE 2

libusb_context *context;
libusb_device_handle *handle;
uint8_t endpointInt[N_ENDPOINT];
int cptInt =0;

void init_usb(void)
{
int status=libusb_init(&context);
if(status!=0) {perror("libusb_init"); exit(-1);}
}

//Enumération des périphériques USB
void enum_usb(void)
{
libusb_device **list;
ssize_t count=libusb_get_device_list(context,&list);
if(count<0) {perror("libusb_get_device_list"); exit(-1);}
ssize_t i=0;
for(i=0;i<count;i++){
	libusb_device *device=list[i];
	struct libusb_device_descriptor desc;
  	int status=libusb_get_device_descriptor(device,&desc);
	if(status!=0) continue;
	uint8_t bus=libusb_get_bus_number(device);
	uint8_t address=libusb_get_device_address(device);

	//Affichage des périphériques USB trouvés
	printf("Device Found @ (Bus:Address) %d:%d\n",bus,address);
	printf("Vendor ID 0x0%x\n",desc.idVendor);
	printf("Product ID 0x0%x\n",desc.idProduct);

	//On récupère notre manette
	if(desc.idVendor == vendorID && desc.idProduct == productID){
			int status=libusb_open(device,&handle);
			if(status!=0){ perror("libusb_open"); exit(-1); }
	}
}
libusb_free_device_list(list,1);
}

//On libère toutes les interfaces de notre périphérique
void release(void)
{
struct libusb_config_descriptor *config;
libusb_device *deviceToRelease = libusb_get_device(handle);
libusb_get_active_config_descriptor(deviceToRelease,&config);
for(int i=0;i<config->bNumInterfaces;i++){
	uint8_t interface = config->interface[i].altsetting[0].bInterfaceNumber;
	if(libusb_kernel_driver_active(handle,interface)){
		int status=libusb_detach_kernel_driver(handle,interface);
		if(status!=0){ perror("libusb_detach_kernel_driver"); exit(-1); }
	}
}
}

//Claim de toutes les interfaces et stockage des endpoints
void config(void)
{
libusb_device *deviceToGet = libusb_get_device(handle);
struct libusb_config_descriptor *config;
libusb_get_config_descriptor(deviceToGet,0,&config);
printf("Valeur de la config : %d\n",config->bConfigurationValue);
int configuration=config->bConfigurationValue;
int status=libusb_set_configuration(handle,configuration);
if(status!=0){ perror("libusb_set_configuration"); exit(-1); }
for(int i=0;i<config->bNumInterfaces;i++){ //Sur toutes les interfaces
	uint8_t interface = config->interface[i].altsetting[0].bInterfaceNumber;
	printf("Indice : %d ,Interface detectée et réclamée : %d\n",i,interface);
	status=libusb_claim_interface(handle,interface);
	if(status!=0){ perror("libusb_claim_kernel_driver"); exit(-1);}
	for(int j=0;j<config->interface[i].altsetting[0].bNumEndpoints;j++){
		if((config->interface[i].altsetting[0].endpoint[j].bmAttributes & 0b00000011)  == LIBUSB_TRANSFER_TYPE_INTERRUPT)
		{
		int addEP =config->interface[i].altsetting[0].endpoint[j].bEndpointAddress;
		printf(" Indice : %d ,Endpoint trouvé : %d",j,addEP);
		if(cptInt==N_ENDPOINT){printf("\n");continue;}
		printf(" saved !\n");
		endpointInt[cptInt]=addEP;
		cptInt++;		
		}
	}
}
}

//Envoi de données sur le port USB
void Send(char c, int endpoint_out)
{
unsigned char data[8]={c};            // data to send or to receive 
int size=sizeof(data);           //size to send or maximum size to receive 
int timeout=0;        // timeout infini
        
//OUT interrupt, from host to device 
int bytes_out;
int status = libusb_interrupt_transfer(handle,endpoint_out,data,size,&bytes_out,timeout);

if(status!=0){ perror("libusb_interrupt_transfer_S"); exit(-1); }
}

//Récupération des données sur le port USB
int Read(int endpoint_in,unsigned char* data)
{
int size=sizeof(data);;           /* size to send or maximum size to receive */
int timeout=TIMEOUTREAD;        /* timeout in ms */

/* IN interrupt, host polling device */
int bytes_in;
int status=libusb_interrupt_transfer(handle,endpoint_in,data,size,&bytes_in,timeout);
if(status!=0){ perror("libusb_interrupt_transfer_R"); exit(-1); }
   
return data[0];
}

//Libération des interfaces
void release_interface(void)
{	
struct libusb_config_descriptor *config;
libusb_device *deviceToRelease = libusb_get_device(handle);
libusb_get_active_config_descriptor(deviceToRelease,&config);
for(int i=0;i<config->bNumInterfaces;i++){
    uint8_t interface = config->interface[i].altsetting[0].bInterfaceNumber;
    int status=libusb_release_interface(handle,interface);
	printf("Indice: %d ,interface free %d\n",i,interface);
    if(status!=0){ perror("libusb_release_interface"); exit(-1);}
}
}

//Programme principale
int main(void)
{
init_usb();
enum_usb();
if(handle != NULL) printf("Device Found\n");
else{printf("Device not Found\n");exit(-1);}
release();
config();
	
//Traitement
char c=0;
while (c != 'x')
{
	printf("Entrez un caractère (a-f pour allumer/A-F pour eteindre), x pour quitter\n");
	c = getchar();
	Send(c, endpointInt[2]);
	unsigned char data[SIZE2RECIEVE];
	printf("Data boutons :%c\n",Read(endpointInt[0],data));
	Read(endpointInt[1],data);
	printf("Data JoyA :%c\nData JoyB :%c\n",data[0],data[1]);
}

//release
release_interface();
libusb_close(handle);
libusb_exit(context);

return 0;
}
