/**
  * Kevin Gilboy and Nick Petro
  *	ECE 1150 - Intro to Networks
  *	Lab 1
 
  ***   PLEASE NOTE: All multi-line comments before code sections were
  ***	written to help the developers with organizing the program
  ***	before implementing. They were not meant in any way to insult
  ***	the intelligence of any readers of this program :^)

 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT "80"
#define API_KEY "cf497a3ca5237e42"
#define URL "api.wunderground.com"

int main(int argc, char *argv[]){
	int success;
	/*
	 * Check input args for proper zipcode
	 */
	if(argc!=2){
		printf("Invalid. Usage: ./%s <us_zipcode>\n",argv[0]);
		exit(-1);
	}
	int zipcode = atoi(argv[1]);
	if(zipcode<=0 || zipcode>99999){
		printf("The entered zipcode (%s) is invalid.\n",argv[1]);
		exit(-1);
	}

	/*
	 * Get and Display the weather
	 */
	success = getWeather(argv[1]);
	return success;
}

int getWeather(char *zipcode){
	int success = 0;
	int recv_bytes = 0;
	int content_length = 0;
	int header_length = 0;
	int peek_size = 800;
	char peek_message[800];
    struct addrinfo hints;
	struct addrinfo *result;
	char get_command[100];
	char *curr_ptr;
	
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	
	/*
	 * Open a socket and connect to the addr
	 */
	int server_status = getaddrinfo(URL, PORT, &hints, &result);
	int client_socket = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	int connection_status = connect(client_socket, result->ai_addr, result->ai_addrlen);

	if(connection_status<0){
		printf("Connection failed\n");
		return -1;
	}
	
	/*
	 * Build a GET request and send it
	 */
	strcpy(get_command,"GET /api/"API_KEY"/conditions/q/");
	strcat(get_command,zipcode);
	strcat(get_command,".xml HTTP/1.1\r\nHost: "URL"\r\n\r\n");
	send(client_socket,get_command,strlen(get_command),0);
	
	/*
	 * Peek at response to get content length and size of header
	 */
	curr_ptr = peek_message;
	do{
		recv_bytes = recv(client_socket, curr_ptr, sizeof(peek_message)-1,MSG_PEEK);
		if(recv_bytes<=0){
			printf("No message received from host\n");
			return -1;
		}
		curr_ptr+=recv_bytes*sizeof(char);
	}while((curr_ptr-peek_message)/sizeof(char)<peek_size-1);
	peek_message[recv_bytes] = 0;

	// Get content length
	char *content_id = "Content-Length: ";
	char *next_number_character = strstr(peek_message,content_id)+((strlen(content_id))*sizeof(char));
	
	char *connection_id = "Connection:";
	char *end_number = strstr(peek_message,connection_id)-(2*sizeof(char)); //Remove two chars since the previous line ends in \r\n
	
	while(next_number_character<end_number){
		content_length = (content_length*10)+(next_number_character[0]-'0');
		next_number_character+=sizeof(char);
	}
	
	// Get size of header
	char *str3 = "<response>";
	char *response_start = strstr(peek_message,str3);
	header_length = (response_start-peek_message) / sizeof(char);

	/*
	 * Recv header and discard it
	 */
	char *header = malloc(sizeof(char) * (header_length+1));
	curr_ptr = header;
	do{
		recv_bytes = recv(client_socket, curr_ptr, header_length,0);
		if(recv_bytes<=0){
			printf("No message received from host\n");
			return -1;
		}
		curr_ptr+=recv_bytes*sizeof(char);
	}while((curr_ptr-header)/sizeof(char)<header_length-1);
	header[recv_bytes] = 0;
	free(header);
	
	/*
	 * Recv content
	 */
	char *content = malloc(sizeof(char) * (content_length+1));
	curr_ptr = content;
	// We must keep recv'ing if the recv gives less bytes than we ask for
	do{
		recv_bytes = recv(client_socket, curr_ptr, content_length,0);
		if(recv_bytes<=0){
			printf("No message received from host\n");
			return -1;
		}
		curr_ptr+=recv_bytes*sizeof(char);
	}while((curr_ptr-content)/sizeof(char)<content_length-1);
	content[content_length] = 0;
	close(client_socket);
	
	/*
	 * Display Weather Nicely
	 */
	success = displayWeather(content);
	
	free(content);
	return success;
}

int displayWeather(char *content){
	int success = 0;
	/*
	 * Parse, prettify, and print the content
	 */
	printLine();
	
	//If the xml response has an <error> tag, the zipcode must be invalid
	char *error_start_str = "<error>";
	if(!strstr(content,error_start_str)){
		//Parse and print the time
		char *time_start_str = "<observation_time>";
		char *time_start = strstr(content,time_start_str)+((strlen(time_start_str))*sizeof(char));
		char *time_end_str = "</observation_time>";
		char *time_end = strstr(content,time_end_str);
		printf("%.*s\n",(time_end-time_start)/sizeof(char),time_start);

		//Parse and print the city name
		char *full_start_str = "<full>";
		char *full_start = strstr(content,full_start_str)+((strlen(full_start_str))*sizeof(char));
		char *full_end_str = "</full>";
		char *full_end = strstr(content,full_end_str);
		printf("%.*s ",(full_end-full_start)/sizeof(char),full_start);

		//Parse and print the zip
		char *zip_start_str = "<zip>";
		char *zip_start = strstr(content,zip_start_str)+((strlen(zip_start_str))*sizeof(char));
		char *zip_end_str = "</zip>";
		char *zip_end = strstr(content,zip_end_str);
		printf("%.*s \n",(zip_end-zip_start)/sizeof(char),zip_start);
		
		//Parse and print the weather
		char *weather_start_str = "<weather>";
		char *weather_start = strstr(content,weather_start_str)+((strlen(weather_start_str))*sizeof(char));
		char *weather_end_str = "</weather>";
		char *weather_end = strstr(content,weather_end_str);
		printf("> Weather: %.*s\n",(weather_end-weather_start)/sizeof(char),weather_start);

		//Parse and print the temp
		char *temp_start_str = "<temperature_string>";
		char *temp_start = strstr(content,temp_start_str)+((strlen(temp_start_str))*sizeof(char));
		char *temp_end_str = "</temperature_string>";
		char *temp_end = strstr(content,temp_end_str);
		printf("> Temperature: %.*s\n",(temp_end-temp_start)/sizeof(char),temp_start);

		//Parse and print the wind
		char *wind_start_str = "<wind_string>";
		char *wind_start = strstr(content,wind_start_str)+((strlen(wind_start_str))*sizeof(char));
		char *wind_end_str = "</wind_string>";
		char *wind_end = strstr(content,wind_end_str);
		printf("> Winds: %.*s\n",(wind_end-wind_start)/sizeof(char),wind_start);
		
		success = 0;
	}
	else{
		printf("That zip code is not found on wunderground!\n");
		success = -1;
	}

	printLine();
	printf("\n");
	
}

int printLine(){
	printf("-----------------------------------------------\n");
	return 0;
}
