#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h> 
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>

#define ERR_EXIT(msg) do{perror(msg);exit(EXIT_FAILURE);}while(0)

struct  normal_user{
  char name[100];
  char password[20];
};

struct  agent_user{
  char name[100];
  char password[20];
};

struct login_details 
{
     int userid;
     char password[20];
     int type;
};

struct train_data
{
	int train_no;
	char train_name[50];
	char boarding_point[50];
    char destination_point[50];
	int available_seats;
	int booked_seats;
	int ticket_price;
    int date;
};

struct booked_train_data
{
	int account_type;
	int userid;
	char passenger_name[100];
	int train_no;
	char train_name[50];
	char boarding_point[50];
    char destination_point[50];
	int seat_no;
	int ticket_price;
	int date,month,year;
};

void user_display(int, int, int);
void admin_display(int);

void signin(int sockfd)
{
    struct login_details det;
    short int verify;
    int type;

	printf("Enter account type to Signin:\n1. Normal user\n2. Agent account\n");
	scanf("%d", &type);
	type = htonl(type);
	if(write(sockfd, &type, sizeof(int)) == -1)
		ERR_EXIT("write()");
	
	printf("Enter Userid : ");
	scanf("%d", &det.userid);
	det.userid = htonl(det.userid);
		
	if(write(sockfd,&det.userid,sizeof(int)) == -1)
		ERR_EXIT("write()");			 	
	
	char* password;
	password = getpass("Enter password: ");
	
	if(write(sockfd, password, sizeof(det.password)) == -1) 
		ERR_EXIT("write()");
	if(read(sockfd, &verify, sizeof(int)) == -1)
		ERR_EXIT("read()");
	verify = ntohl(verify);
	
	while ((getchar()) != '\n');
	//clear input buffer
	if(verify == 0)
	{
		printf("Login successful.\n");
		printf("Press enter to show the menu.");
		getchar();
		user_display(det.userid, type, sockfd);
	}
	else if(verify == -1)
	{
		printf("Invalid login details.\n");
		printf("Press any key to retry...\n");
		getchar();
	}
	else if(verify == 2)
	{
		printf("Already Logged in.\n");
		printf("Press any key to retry...\n");
		getchar();
	}
}

void signup(int sockfd)
{
	struct normal_user user;
	struct agent_user agent; 
	int type;
	int userid;
	int login;
	printf("Enter account type to Signup:\n1. Normal user\n2. Agent account\n");
	scanf("%d",&type);
	type = htonl(type);
	if(write(sockfd, &type, sizeof(int)) == -1)
		ERR_EXIT("write()");
	type = ntohl(type);

	if(type == 1)  
	{
		printf("Enter name: ");
		scanf("%s",user.name);
		if(write(sockfd, user.name, sizeof(user.name)) == -1)
			ERR_EXIT("write()");

		char* password;
		password = getpass("Enter password: ");
		if(write(sockfd, password, sizeof(user.password)) == -1) ERR_EXIT("write()");
		
		if(read(sockfd, &userid, sizeof(int)) == -1)
			ERR_EXIT("read()");
		printf("Sign up successful.\n");
		printf("Your account number for further login: %d\n", ntohl(userid));
		printf("Press any key to continue...\n");
		while ((getchar()) != '\n');getchar();
	}
	else if(type == 2)
	{   
		printf("Enter name: ");
		scanf("%s", agent.name);
		if( write(sockfd, agent.name, sizeof(agent.name)) == -1)
			ERR_EXIT("write()");
		
		char* password;
		password = getpass("Enter password: ");
		if(write(sockfd, password, sizeof(agent.password)) == -1) ERR_EXIT("write()");
		
		if(read(sockfd,&userid,sizeof(int)) == -1)
			ERR_EXIT("read()");
		printf("Sign up successful.\n");
		printf("Your account number for further login: %d\n", ntohl(userid));
		printf("Press any key to continue...\n");
		while ((getchar()) != '\n');getchar();
	}
}

void admin(int sockfd)
{
	int verified = 0;
	printf("Username       :  admin\n");
	char* password;
	password = getpass("Admin password : ");
	if(write(sockfd, password, sizeof(password)) == -1) 
		ERR_EXIT("write()");
			
	if(read(sockfd, &verified, sizeof(int)) == -1)
		ERR_EXIT("read()");
	verified = ntohl(verified);
	
	if(verified == 1)
	{
		getchar();
		admin_display(sockfd);
	}
	else
	{
		printf("Invalid Admin Credentials\n");
		
		getchar();
	}
}


void user_display(int userid,int type,int sockfd)
{
	int choice=0;
	int ack=-1;
	
	while(1)
	{
		// clear screen
		system("clear");

		// printf("******************************************\n");
		printf("                USER MODE                \n\n");
		// printf("******************************************\n\n");
		printf("Enter you choice\n1. Book Ticket\n2. View Previous Booking\n");
		printf("3. Update Booking\n4. Cancel Booking\n");
		printf("5. Available Trains\n6. Exit\n");
		scanf("%d", &choice);
		choice = htonl(choice);
		if(write(sockfd, &choice, sizeof(choice)) == -1) ERR_EXIT("write()");
		choice = ntohl(choice);
		int ack = 0;
		int n = 0;	
		switch(choice)
		{
			case 1:
			{
				char buf[1024];
				char passenger_name[100];
				memset(buf,0,1024);
				int train_no;
				printf("Enter the train no you want to book = ");
					scanf("%d",&train_no);
				train_no = htonl(train_no);
				if(write(sockfd, &train_no, sizeof(train_no)) == -1) ERR_EXIT("write()");
				
				if(read(sockfd, &ack, sizeof(ack)) == -1) ERR_EXIT("read()");
				
				if(ntohl(ack) == 1)
				{
					printf("Train Found !!!\n\n");
					if(read(sockfd, buf, sizeof(buf)) == -1) ERR_EXIT("read()");
					printf("%s\n", buf);
					printf("Enter Passenger name to Book : \n");
					scanf("%s",passenger_name);
					
					if(write(sockfd, &passenger_name, sizeof(passenger_name)) == -1) ERR_EXIT("write()");
					memset(buf,0,1024);
					if(read(sockfd, buf, sizeof(buf)) == -1) ERR_EXIT("read()");
					printf("%s", buf);		
				}
				else if(ntohl(ack) == -1) 
					printf("No train available...please try again.\n\n");
				
				break;  
			}

			case 2:
				while(1)
				{
					char buf[1024];
					memset(buf, 0, 1024);
					if(read(sockfd, buf, sizeof(buf)) == -1) 
						ERR_EXIT("read()");
	
							if(strcmp(buf,"end")==0)
								break;
							else
							{
								printf("%s\n", buf);
							}
				}
				break;
		
			case 3:
			{ 
				char buf[1024];
				char passenger_name[100];
				memset(buf,0,1024);
				int train_no;
				printf("Enter the train no you want to update ticket = ");
					scanf("%d",&train_no);
				train_no = htonl(train_no);
				if(write(sockfd, &train_no, sizeof(train_no)) == -1) ERR_EXIT("write()");
					
				if(read(sockfd, &ack, sizeof(ack)) == -1) ERR_EXIT("read()");
				
				if(ntohl(ack) == 1)
				{
					memset(buf,0,sizeof(buf));
					printf("Booking Found !!!\n\n");
					if(read(sockfd, buf, sizeof(buf)) == -1) ERR_EXIT("read()");
						printf("%s\n", buf);
			
					printf("Enter Updated Passenger Name : \n");
					scanf("%s",passenger_name);
					
					if(write(sockfd, &passenger_name, sizeof(passenger_name)) == -1) 
						ERR_EXIT("write()");
					
					memset(buf,0,sizeof(buf));
						printf("%s", buf);		
					if(read(sockfd, buf, sizeof(buf)) == -1) ERR_EXIT("read()");
				}
				else if(ntohl(ack) == -1) 
				{	
					memset(buf,0,sizeof(buf));
					printf("No Booking Found.\n\n");
				}	
				break;  

			}
			case 4:
			{
				int train_no;
				printf("Enter Train no to Cancel: ");
				scanf("%d",&train_no);
				train_no = htons(train_no);
				if(write(sockfd, &train_no, sizeof(train_no)) == -1)
					ERR_EXIT("write()");

				char buf[1024];
				memset(buf,0,sizeof(buf));
					if(read(sockfd, &buf, sizeof(buf)) == -1)
							ERR_EXIT("Train no : Read()");
				printf("%s\n",buf);
				break;
			}
			case 5:
			{
				char buf[1024];
				
					while(1)
					{
					memset(buf, 0, 1024);
					if(read(sockfd, buf, sizeof(buf)) == -1) ERR_EXIT("read()");
						{
							if(strcmp(buf,"end")==0)
								break;
							else
							{
									printf("%s\n", buf);
				
							}
							
						}
					}

				break;
			}
			default:
			{
				if(close(sockfd) == -1) ERR_EXIT("close()");
				exit(EXIT_SUCCESS);
			}
		}
		// clear the buffer
		while ((getchar()) != '\n');
		printf("Press any key to continue...\n");
		getchar();
	}
}

void admin_display(int sockfd)
{
	int choice=0;
	
	while(1)
	{
		// clear screen
		system("clear");
		// printf("******************************************\n");
		printf("                ADMIN MODE                \n\n");
		// printf("******************************************\n\n");
		printf("Enter you choice\n1. Add New Train\n2. View Trains\n");
		printf("3. Update Train\n4. Cancel Train\n");
		printf("5. Exit\n");
		scanf("%d", &choice);
		choice = htonl(choice);
		if(write(sockfd, &choice, sizeof(choice)) == -1) ERR_EXIT("write()");
		choice = ntohl(choice);
		struct train_data dt;
		switch(choice)
		{
			case 1:
			printf("Enter Train no : ");
			scanf("%d",&(dt.train_no));
			if(write(sockfd, &(dt.train_no), sizeof(dt.train_no)) == -1)
				ERR_EXIT("write()");
			
			printf("Enter Train name : ");
			scanf("%s",dt.train_name);
			if(write(sockfd, dt.train_name, sizeof(dt.train_name)) == -1)
				ERR_EXIT("write()");
			
			printf("Enter Boarding Point : ");
			scanf("%s",dt.boarding_point);
			if(write(sockfd, dt.boarding_point, sizeof(dt.boarding_point)) == -1)
				ERR_EXIT("write()");
			
			printf("Enter Destination Point : ");
			scanf("%s",dt.destination_point);
			if(write(sockfd, dt.destination_point, sizeof(dt.destination_point)) == -1)
				ERR_EXIT("write()");
			
			printf("Enter Total Available Seats : ");
			scanf("%d",&(dt.available_seats));
			if(write(sockfd, &(dt.available_seats), sizeof(dt.available_seats)) == -1)
				ERR_EXIT("write()");

			dt.booked_seats = 0;
			if(write(sockfd, &(dt.booked_seats), sizeof(dt.booked_seats)) == -1)
				ERR_EXIT("write()");

			printf("Enter Ticket Price : ");
			scanf("%d",&(dt.ticket_price));
			if(write(sockfd, &(dt.ticket_price), sizeof(dt.ticket_price)) == -1)
				ERR_EXIT("write()");
			
			printf("Enter Date : ");
			scanf("%d \n",&(dt.date)); 
			if(write(sockfd, &(dt.date), sizeof(dt.date)) == -1)
				ERR_EXIT("write()");
			break;  
		
			case 2:
				while(1)
				{
					char buf[1024];
					memset(buf, 0, 1024);
					if(read(sockfd, buf, sizeof(buf)) == -1) 
						ERR_EXIT("read()");
			
							//if(strncmp(buf,"end",3)==0)
							if(strcmp(buf,"end")==0)
								break;
							else
							{
								printf("\n%s", buf);
							}
				}
				break;
			case 3:
			{ 
				break;
			}
			
			case 4:
			{
				int train_no;
				printf("Enter Train no to Cancel: ");
				scanf("%d",&train_no);
				train_no = htons(train_no);
				if(write(sockfd, &train_no, sizeof(train_no)) == -1)
					ERR_EXIT("write()");

				char buf[1024];
				memset(buf,0,sizeof(buf));
					if(read(sockfd, &buf, sizeof(buf)) == -1)
							ERR_EXIT("Train no : Read()");
				printf("%s\n",buf);
				break;
			}
			
			case 5:
			{
				if(close(sockfd) == -1) ERR_EXIT("close()");
				exit(EXIT_SUCCESS);
				break; 
			}
			default:
			{
				if(close(sockfd) == -1) ERR_EXIT("close()");
				exit(EXIT_SUCCESS);
			}
		}
		// clear the buffer
		while ((getchar()) != '\n');
		printf("Press any key to continue...\n");
		getchar();
	}
}

int main(int argc,char* argv[])
{
	if(argc != 3)
	{
        	fprintf(stderr ,"Usage: %s <ip_addr_of_server> <port_number>\n", argv[0]);
        	exit(EXIT_FAILURE);
    } 
	
	int sockfd;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		ERR_EXIT("socket()");

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	
	server_addr.sin_family=AF_INET;
	
	server_addr.sin_port=htons(atoi(argv[2]));
	
	if(inet_pton(AF_INET, argv[1], &server_addr.sin_addr) != 1)
		ERR_EXIT("inet_pton()");

    	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    		ERR_EXIT("connect()");

	while(1)
	{
		// clear the screen
		system("clear");
		printf("--------------------------------------------------------\n");
		printf("***** WELCOME TO RAILWAY TICKET RESERVATION SYSTEM *****\n");
		printf("--------------------------------------------------------\n");
		printf("1. Sign in\n2. Sign up\n3. Admin Mode\n4. Exit\n");
		// printf("*****************************\n");
		
		int choice;
		scanf("%d",&choice);
		
		switch(choice)
		{
			case 1:
				choice = htonl(choice);
				if(write(sockfd, &choice, sizeof(int)) == -1)
					ERR_EXIT("write()");
				signin(sockfd);
			break;
			case 2:
				choice = htonl(choice);
				if(write(sockfd, &choice, sizeof(int)) == -1)
					ERR_EXIT("write()");
				signup(sockfd);
			break;
			case 3:
				choice = htonl(choice);
				if(write(sockfd, &choice, sizeof(int)) == -1)
					ERR_EXIT("write()");
				admin(sockfd);
				break;
			
			case 4:
				choice = htonl(choice);
				if(write(sockfd, &choice, sizeof(int)) == -1)
				if(close(sockfd) == -1) ERR_EXIT("close()");
					exit(EXIT_SUCCESS);
			break;
			default:
				printf("Please enter valid choice.\n");
			break;
		}
	}

	return 0;
}

