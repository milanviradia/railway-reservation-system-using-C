#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/sem.h>

#define ERR_EXIT(msg) do{perror(msg);/*pthread_exit(NULL);*/}while(0)
#define _ERR_EXIT(msg) do{perror(msg);exit(EXIT_FAILURE);}while(0)

#define NORMAL "./db/NORMAL_acct_det.db"
#define AGENT "./db/AGENT_acct_det.db"
#define TRAIN "./db/TRAIN_det.db"
#define BOOKED_TRAIN "./db/BOOKED_TRAIN_det.db"

#define PASS_LENGTH 20

struct user_account
{
	int userid;
	char name[100];
	char password[PASS_LENGTH];
};

struct agent_account
{
	int userid;
	char name[100];
	char password[PASS_LENGTH];
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
	int date;
};

union senum
{
	int val;
	unsigned short int* array;
}arg;

int key,semid;
struct sembuf buf = {0, -1, SEM_UNDO}; /* (-1 + previous value) */

// authenticate the account number and password 
// return 0 for successfull
// for AGENT else return -1 for invalid
short authenticate(int userid, char *password, int account_type)
{
	int fd;
	int flags = O_RDWR | O_CREAT;
	int mode = S_IRUSR | S_IWUSR; // 0600
	if (account_type == 1)
	{
		struct user_account uad;
		if((fd = open(NORMAL, flags, mode)) == -1) ERR_EXIT("open()");
		if((lseek(fd, (userid - 1) * sizeof(uad), SEEK_SET)) == (off_t)-1)
			ERR_EXIT("authenticate() -> lseek()");
		if(read(fd, &uad, sizeof(uad)) == -1)
			ERR_EXIT("authenticate() -> read()");
		if(uad.userid == userid)
			{
			if(strcmp(uad.password, password) == 0)
				{
				// int key = ftok(".",userid);
				// int semid = semget(key,1,IPC_CREAT|IPC_EXCL|0744);
	            
				// if(semid!=-1)
				// {
				// 	arg.val = 1;
				// 	semctl(semid,0,SETVAL,arg);				
				// }
				// else
	            // 	semid = semget(key,1,0);
	           	
				// // int cnt = semctl(semid, 0,GETVAL,arg.val);
				// // printf("argg = %d\n\n", cnt);
				// semop(semid,&buf,1);
				
				close(fd);
				return 0;
				}
			}
	}
	else if(account_type = 2)
	{
		struct agent_account aad;
		if((fd = open(AGENT, flags, mode)) == -1) ERR_EXIT("open()");
		
		if((lseek(fd, (userid - 1) * sizeof(aad), SEEK_SET)) == (off_t)-1)
			ERR_EXIT("authenticate() -> lseek()");
		if(read(fd, &aad, sizeof(aad)) == -1)
			ERR_EXIT("authenticate() -> read()");
		// Unlock
		// lock.l_type = F_UNLCK;
		// if(fcntl(fd, F_SETLKW, &lock) == -1)
		// 	ERR_EXIT("authenticate() -> fcntl()");
		if(aad.userid == userid)
		{
			if(strcmp(aad.password, password) == 0)
			{
				close(fd);
				return 0;
			}
		}
	}
	return -1;
}

int create_user_account(int conn_fd)
{
	char user[100];
	char password[PASS_LENGTH];
	int errorcode = -1;
	ssize_t count_user, count_pass;

	if((count_user = read(conn_fd, user, sizeof(user))) == -1)
		ERR_EXIT("create_user_account() -> read()");
	user[count_user] = '\0';
	if((count_pass = read(conn_fd, password, sizeof(password))) == -1)
		ERR_EXIT("create_user_account() -> read()");
	if(count_pass < 5)
		if(write(conn_fd, &errorcode, sizeof(int)) == -1)
			ERR_EXIT("create_user_account() -> write()");
	password[count_pass] = '\0';
	struct user_account uad;
	for (int i = 0; i <= count_user; ++i)
		uad.name[i] = user[i];
	for (int i = 0; i <= count_pass; ++i)
		uad.password[i] = password[i];

	uad.userid = 1;
	int flags = O_RDWR | O_CREAT;
	int mode = S_IRUSR | S_IWUSR; // 0600
	int fd = open(NORMAL, flags, mode);
	if(fd == -1) ERR_EXIT("create_user_account() -> open()");
	else
	{
		off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) ERR_EXIT("lseek()");
		if (offset == 0) // file was empty
		{
			uad.userid = 1; // initial account
			if(write(fd, &uad, sizeof(uad)) == -1) ERR_EXIT("write()");
		}
		else
		{
			struct user_account temp;
			offset = -sizeof(uad);
			if(lseek(fd, offset, SEEK_END) == (off_t)-1 ) ERR_EXIT("lseek()");
			if(read(fd, &temp, sizeof(temp)) == -1) ERR_EXIT("read()");
			uad.userid = temp.userid + 1;
	    	if(lseek(fd, (off_t)0, SEEK_END) == (off_t)-1 ) ERR_EXIT("lseek()");
	    	if(write(fd, &uad, sizeof(uad)) == -1) ERR_EXIT("write()");
	    }
	}
	if(close(fd) == -1) 
		ERR_EXIT("close()");
	return uad.userid;
}

int create_agent_account(int conn_fd)
{
	char user[100];
	char password[PASS_LENGTH];
	int errorcode = -1;
	ssize_t count_user, count_pass;
	if((count_user = read(conn_fd, user, sizeof(user))) == -1)
		ERR_EXIT("read()");
	user[count_user] = '\0';
	
	if((count_pass = read(conn_fd, password, sizeof(password))) == -1)
		ERR_EXIT("read()");
	if(count_pass < 5)
		if(write(conn_fd, &errorcode, sizeof(int)) == -1)
			ERR_EXIT("write()");
	password[count_pass] = '\0';
	
	struct agent_account aad;
	for (int i = 0; i <= count_user; ++i)
		aad.name[i] = user[i];
	for (int i = 0; i <= count_pass; ++i)
		aad.password[i] = password[i];

	int flags = O_RDWR | O_CREAT;
	int mode = S_IRUSR | S_IWUSR; // 0600
	
	int fd = open(AGENT, flags, mode);
	if(fd == -1) ERR_EXIT("read()");
	else
	{
		off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) 
			ERR_EXIT("lseek()");
		
		if (offset == 0) // file was empty
		{
			aad.userid = 1; // initial account
			if(write(fd, &aad, sizeof(aad)) == -1) ERR_EXIT("write()");
		}
		else
		{
			struct user_account temp;
			offset = -sizeof(aad);
			if(lseek(fd, offset, SEEK_END) == (off_t)-1 ) ERR_EXIT("lseek()");
			if(read(fd, &temp, sizeof(temp)) == -1) ERR_EXIT("read()");
			aad.userid = temp.userid + 1;
	    	if(lseek(fd, (off_t)0, SEEK_END) == (off_t)-1 ) ERR_EXIT("lseek()");
	    	if(write(fd, &aad, sizeof(aad)) == -1) ERR_EXIT("write()");
	    }
	}
	if(close(fd) == -1) 
		ERR_EXIT("close()");
	return aad.userid;
}

int add_train(int conn_fd)
{
	int train_no;
	char train_name[50];
	char boarding_point[50];
    char destination_point[50];
	int available_seats;
	int booked_seats;
	int ticket_price;
    int date;
	
	struct train_data td;
	ssize_t tno,tname,bp,dp,as,bs,tp,d;
	if((tno = read(conn_fd, &train_no, sizeof(train_no))) == -1)
		ERR_EXIT("read()");
	td.train_no = train_no;

	if((tname = read(conn_fd, train_name, sizeof(train_name))) == -1)
		ERR_EXIT("read()");
	train_name[tname] = '\0';
	
	if((bp = read(conn_fd, boarding_point, sizeof(boarding_point))) == -1)
		ERR_EXIT("read()");
	boarding_point[bp] = '\0';
	
	if((dp = read(conn_fd, destination_point, sizeof(destination_point))) == -1)
		ERR_EXIT("read()");
	destination_point[dp] = '\0';
	
	if((as = read(conn_fd, &available_seats, sizeof(available_seats))) == -1)
		ERR_EXIT("read()");
	td.available_seats = available_seats;

	if((bs = read(conn_fd, &booked_seats, sizeof(booked_seats))) == -1)
		ERR_EXIT("read()");
	td.booked_seats = booked_seats;

	if((tp = read(conn_fd, &ticket_price, sizeof(ticket_price))) == -1)
		ERR_EXIT("read()");
	td.ticket_price = ticket_price;

	if((d = read(conn_fd, &date, sizeof(date))) == -1)
		ERR_EXIT("read()");
	td.date = date;

	for (int i = 0; i <= tname; ++i)
		td.train_name[i] = train_name[i];

	for (int i = 0; i <= bp; ++i)
		td.boarding_point[i] = boarding_point[i];
	
	for (int i = 0; i <= dp; ++i)
		td.destination_point[i] = destination_point[i];
	
	int flags = O_RDWR | O_CREAT;
	int mode = S_IRUSR | S_IWUSR; // 0600
	// write locking
	int fd = open(TRAIN, flags, mode);
	if(fd == -1) ERR_EXIT("read()");
	else
	{
		off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) ERR_EXIT("lseek()");
		if (offset == 0) // file was empty
		{
			if(write(fd, &td, sizeof(td)) == -1) ERR_EXIT("write()");
		}
		else
		{
			/*struct train_data temp;
			offset = -sizeof(td);
			if(lseek(fd, offset, SEEK_END) == (off_t)-1 ) ERR_EXIT("lseek()");
			if(read(fd, &temp, sizeof(temp)) == -1) ERR_EXIT("read()");
			*/
			if(lseek(fd, (off_t)0, SEEK_END) == (off_t)-1 ) ERR_EXIT("lseek()");
	    	if(write(fd, &td, sizeof(td)) == -1) ERR_EXIT("write()");
	    }
	}
	if(close(fd) == -1) ERR_EXIT("close()");
	return td.train_no;
}

void display_trains(int conn_fd)
{
	// search for detail
	// send details after searching to connection fd in buffer
	int fd;
	int flags = O_RDONLY;
	int mode = 0; //ignore mode
	struct train_data td;
	if((fd = open(TRAIN, flags, mode)) == -1)
		ERR_EXIT("display_trains() -> open()");

	char details[1024];

	off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) ERR_EXIT("lseek()");
		if (offset == 0) // file was empty
			return;
	lseek(fd, 0, SEEK_SET);
	while(read(fd,&td,sizeof(td)))
	{
		if(td.train_no!=-1)
		{
		sprintf(details, "Train No: %d\nTrain Name: %s\nBoarding Point: %s\nDestination Point: %s\nAvailable Seats: %d\nBooked Seats: %d\nTicket Price: %d\nDate: %d\n"
		,td.train_no,td.train_name,td.boarding_point,td.destination_point,td.available_seats,td.booked_seats,td.ticket_price,td.date);
		
		if(write(conn_fd, details, sizeof(details)) == -1)
			ERR_EXIT("display_trains() -> write()");
		}
	}
				memset(details,0,sizeof(details));
				sprintf(details,"end");
				if(write(conn_fd, details, sizeof(details)) == -1)
					ERR_EXIT("previous_booking() -> write()");
		close(fd);
		return;
}

void deleteTrain(struct train_data td2,int train_no)
{
	struct train_data td,td1;

	td1.train_no = -1;
	strcpy(td1.train_name,td2.train_name);
	strcpy(td1.boarding_point,td2.boarding_point);
	strcpy(td1.destination_point,td2.destination_point);
	td1.available_seats = td2.available_seats;
	td1.booked_seats = td2.booked_seats;
	td1.ticket_price = td2.ticket_price;
	td1.date = td2.date;

	int fd=open(TRAIN, O_RDWR);
		off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) 
			ERR_EXIT("lseek()");
		
		if (offset == 0) // file was empty
			return;

		lseek(fd, 0, SEEK_SET);
		while(read(fd,&td,sizeof(td)))
			{
				lseek(fd,-sizeof(td),SEEK_CUR);
				if(td.train_no == train_no)
				{
					write(fd,&td1,sizeof(td1));
					break;
				}
				else
				{
					lseek(fd,sizeof(td),SEEK_CUR);
				}
			}
	return;
}

void cancel_train(int conn_fd)
{
	int train_no;
	if(read(conn_fd, &train_no, sizeof(train_no)) == -1)
		ERR_EXIT("Train no : Read()");

	printf("train no = %d",train_no);

	train_no = ntohs(train_no);

	struct train_data td2;
	int fd=open(TRAIN, O_RDWR);
		off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) 
			ERR_EXIT("lseek()");
		if (offset == 0) //file was empty
			return;

			lseek(fd, 0, SEEK_SET);
			while(read(fd,&td2,sizeof(td2)))
			{
				lseek(fd,-sizeof(td2),SEEK_CUR);
				if(td2.train_no == train_no)
				{
					deleteTrain(td2,train_no);
					break;
				}
				else
				{
					lseek(fd,sizeof(td2),SEEK_CUR);
				}
			}	
	char buf[1024];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"Train no %d Cancelled !!!",train_no);
	if(write(conn_fd, &buf, sizeof(buf)) == -1)
		ERR_EXIT("Train no : Read()");	
}

void updateTrainSeats(struct train_data td,int train_no)
{
		struct train_data td1,td2;
		
		td1.train_no = train_no;
		strcpy(td1.train_name,td.train_name);
		strcpy(td1.boarding_point,td.boarding_point);
		strcpy(td1.destination_point,td.destination_point);
		td1.available_seats = td.available_seats - 1;
		td1.booked_seats = td.booked_seats + 1;
		td1.ticket_price = td.ticket_price;
		td1.date = td.date;
		
		int fd=open(TRAIN, O_RDWR);
		off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) ERR_EXIT("lseek()");
		if (offset == 0) // file was empty
			return;

			lseek(fd, 0, SEEK_SET);
			while(read(fd,&td2,sizeof(td2)))
			{
				lseek(fd,-sizeof(td2),SEEK_CUR);
				if(td2.train_no == train_no)
				{
					write(fd,&td1,sizeof(td1));
					break;
				}
				else
				{
					lseek(fd,sizeof(td1),SEEK_CUR);
				}
			}
	return;			
}

void search_and_book_train(int account_type,int userid,int conn_fd)
{
	int train_no;

	// search for detail
	// send details after searching to connection fd in buffer
	if(read(conn_fd, &train_no, sizeof(int)) == -1) ERR_EXIT("read()");
		train_no = ntohl(train_no);
	
	int fd;
	int ret_val=-1,n;
	int flags = O_RDONLY;
	int mode = 0; //ignore mode
	struct train_data td,td1;
	struct booked_train_data btd;

	if((fd = open(TRAIN, flags, mode)) == -1)
		ERR_EXIT("display_trains() -> open()");

	char details[1024];
	struct flock lock;
	off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) ERR_EXIT("lseek()");
		if (offset == 0) // file was empty
			return;
	lseek(fd, 0, SEEK_SET);
	while(read(fd,&td,sizeof(td)))
	{
		lseek(fd,-sizeof(td),SEEK_CUR);
		if(td.train_no == train_no)
		{
					btd.account_type = account_type;
					btd.userid = userid;
					strcpy(btd.passenger_name,"");
					btd.train_no = td.train_no;
					strcpy(btd.train_name, td.train_name);
					strcpy(btd.boarding_point, td.boarding_point);
					strcpy(btd.destination_point, td.destination_point);
					btd.seat_no = td.booked_seats+1;
					btd.ticket_price = td.ticket_price;
					btd.date = td.date;
					
					updateTrainSeats(td,train_no);

						sprintf(details, "Train No: %d\nTrain Name: %s\nBoarding Point: %s\nDestination Point: %s\nTicket Price: %d\nDate: %d\n\n"
		,td.train_no,td.train_name,td.boarding_point,td.destination_point,td.ticket_price,td.date);
			ret_val = 1;
						// lock.l_type = F_WRLCK;
						// lock.l_whence = SEEK_CUR;
						// lock.l_start = 0;
						// lock.l_len = sizeof(td);
						// lock.l_pid = getpid();
					break;
		}
		else
				{
					lseek(fd,sizeof(td),SEEK_CUR);
				}
	}

	ret_val = htonl(ret_val);
		write(conn_fd, &ret_val, sizeof(ret_val));
	
	int flags1 = O_RDWR | O_CREAT;
	int mode1 = S_IRUSR | S_IWUSR; // 0600
	char passenger_name[100];

	if(ntohl(ret_val)==1)
		{
			write(conn_fd, &details, sizeof(details));
			if(read(conn_fd, &passenger_name, sizeof(passenger_name)) == -1) ERR_EXIT("read()");
				for (int i = 0; i <=sizeof(passenger_name) ; ++i)
					btd.passenger_name[i] = passenger_name[i];
					btd.train_no = train_no;
				int fd1 = open(BOOKED_TRAIN,flags1,mode1);
				if(fd1 == -1)
						ERR_EXIT("search_and_book_train() -> open()");
				else
				{
				off_t offset = 0;
					if((offset = lseek(fd1, (off_t)0, SEEK_END)) == (off_t)-1 ) 
						ERR_EXIT("lseek()");
				if (offset == 0) // file was empty
				{
					if(write(fd1, &btd, sizeof(btd)) == -1) 
						ERR_EXIT("write()");
				}
				else
				{
					if(lseek(fd1, offset, SEEK_END) == (off_t)-1 ) ERR_EXIT("lseek()");
					
					if(write(fd1, &btd, sizeof(btd)) == -1) ERR_EXIT("write()");
	    		}

				close(fd1);
				memset(details, 0, 1024);
				sprintf(details,"Train Booked Successfully !!!\n");
					// lock.l_type = F_UNLCK;
					// if(fcntl(fd, F_SETLKW, &lock) == -1)
					// 	ERR_EXIT("authenticate() -> fcntl()");
				write(conn_fd, &details, sizeof(details));
				}	
		}
	
		close(fd);
}

void previous_booking(int account_type,int userid, int conn_fd)
{
	// send details after searching to connection fd in buffer
	// search for detail
	int fd;
	int flags = O_RDONLY;
	int mode = 0; //ignore mode
	struct booked_train_data btd;
	if((fd = open(BOOKED_TRAIN, flags, mode)) == -1)
		ERR_EXIT("previous_booking() -> open()");

	char details[1024];
	off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) ERR_EXIT("lseek()");
		if (offset == 0) // file was empty
			return;
	lseek(fd, 0, SEEK_SET);
	
	while(read(fd,&btd,sizeof(btd)))
	{
		memset(details,0,sizeof(details));
		if(btd.account_type == account_type && btd.userid == userid)
		{
		sprintf(details, "Account Type: %d\nUserid: %d\nPassenger Name: %s\nTrain No: %d\nTrain Name: %s\nBoarding Point: %s\nDestination Point: %s\nSeat No: %d\nTicket Price: %d\nDate: %d\n"
		, btd.account_type, btd.userid,btd.passenger_name, btd.train_no,btd.train_name,btd.boarding_point,btd.destination_point,btd.seat_no,
	btd.ticket_price,btd.date);

			if(write(conn_fd, details, sizeof(details)) == -1)
				ERR_EXIT("previous_booking() -> write()");
		}
	}
			memset(details,0,sizeof(details));
			sprintf(details,"end");
			if(write(conn_fd, details, sizeof(details)) == -1)
				ERR_EXIT("previous_booking() -> write()");
	
	close(fd);
	return;
	
}

void update_booking(int account_type,int userid,int conn_fd)
{
	int train_no;

	// search for detail
	// send details after searching to connection fd in buffer
	if(read(conn_fd, &train_no, sizeof(int)) == -1) ERR_EXIT("read()");
		train_no = ntohl(train_no);
	
	int fd;
	int ret_val=-1;
	int flags = O_RDONLY;
	int mode = 0; //ignore mode
	struct train_data td;
	struct booked_train_data btd,btd1;

	if((fd = open(BOOKED_TRAIN, flags, mode)) == -1)
		ERR_EXIT("display_trains() -> open()");

	char details[1024];
	off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) ERR_EXIT("lseek()");
		if (offset == 0) // file was empty
			return;
	
	lseek(fd, 0, SEEK_SET);
	while(read(fd,&btd,sizeof(btd)))
	{
		if(btd.account_type == account_type && btd.userid == userid && btd.train_no == train_no)
		{
		sprintf(details, "Account Type: %d\nUserid: %d\nPassenger Name: %s\nTrain No: %d\nTrain Name: %s\nBoarding Point: %s\nDestination Point: %s\nSeat No: %d\nTicket Price: %d\nDate: %d\n"
		, btd.account_type, btd.userid,btd.passenger_name, btd.train_no,btd.train_name,btd.boarding_point,btd.destination_point,btd.seat_no,
	btd.ticket_price,btd.date);
		ret_val = 1;
		break;
		}
	}
	close(fd);

	ret_val = htonl(ret_val);
		write(conn_fd, &ret_val, sizeof(ret_val));
	
	int flags1 = O_RDWR | O_CREAT;
	int mode1 = S_IRUSR | S_IWUSR; // 0600
	int fd1;
	char passenger_name[100];

	if(ntohl(ret_val)==1)
		{
			write(conn_fd, &details, sizeof(details));
	
			if((fd1 = open(BOOKED_TRAIN, flags1, mode1)) == -1)
					ERR_EXIT("display_trains() -> open()");

			if(read(conn_fd, &passenger_name, sizeof(passenger_name)) == -1) ERR_EXIT("read()");
					
			off_t offset = 0;
				if((offset = lseek(fd1, (off_t)0, SEEK_END)) == (off_t)-1 ) 
					ERR_EXIT("lseek()");
		
				if (offset == 0) // file was empty
					return;
			
			lseek(fd1, 0, SEEK_SET);
			while(read(fd1,&btd1,sizeof(btd1)))
			{
				lseek(fd1,-sizeof(btd1),SEEK_CUR);
				if(btd1.account_type == account_type && btd1.userid == userid && btd1.train_no == train_no)
				{
					strcpy(btd1.passenger_name, passenger_name);
					if(write(fd1, &btd1, sizeof(btd1)) == -1)
						ERR_EXIT("Error in Passengername update() -> write()");
					break;
				}
				else
				{
					lseek(fd1,sizeof(btd1),SEEK_CUR);
				}
			}
				close(fd1);
				memset(details, 0, sizeof(details));
				sprintf(details,"Ticket Updated Successfully !!!\n");
				write(conn_fd, &details, sizeof(details));
					
		}
	
}

void deleteBooking(struct booked_train_data td2,int account_type,int userid,int train_no)
{
	struct booked_train_data td,td1;

	td1.account_type = -1;
	td1.userid = -1;
	td1.train_no = td2.train_no;
	strcpy(td1.train_name,td2.train_name);
	strcpy(td1.boarding_point,td2.boarding_point);
	strcpy(td1.destination_point,td2.destination_point);
	td1.seat_no = td2.seat_no;
	td1.ticket_price = td2.ticket_price;
	td1.date = td2.date;

	int fd=open(BOOKED_TRAIN, O_RDWR);
		off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) 
			ERR_EXIT("lseek()");
		
		if (offset == 0) // file was empty
			return;

		lseek(fd, 0, SEEK_SET);
		while(read(fd,&td,sizeof(td)))
			{
				lseek(fd,-sizeof(td),SEEK_CUR);
				if(td.train_no == train_no)
				{
					write(fd,&td1,sizeof(td1));
					break;
				}
				else
				{
					lseek(fd,sizeof(td),SEEK_CUR);
				}
			}
	return;
}

void cancel_booking(int account_type,int userid,int conn_fd)
{
	int train_no;
	if(read(conn_fd, &train_no, sizeof(train_no)) == -1)
		ERR_EXIT("Train no : Read()");

	printf("train no = %d",train_no);

	train_no = ntohs(train_no);

	struct booked_train_data td2;
	int fd=open(BOOKED_TRAIN, O_RDWR);
		off_t offset = 0;
		if((offset = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1 ) 
			ERR_EXIT("lseek()");
		if (offset == 0) //file was empty
			return;

			lseek(fd, 0, SEEK_SET);
			while(read(fd,&td2,sizeof(td2)))
			{
				lseek(fd,-sizeof(td2),SEEK_CUR);
				if(td2.account_type == account_type && td2.userid == userid& td2.train_no == train_no)
				{
					deleteBooking(td2,account_type,userid,train_no);
					break;
				}
				else
				{
					lseek(fd,sizeof(td2),SEEK_CUR);
				}
			}	
	char buf[1024];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"Booking Cancelled !!!");
	if(write(conn_fd, &buf, sizeof(buf)) == -1)
		ERR_EXIT("Train no : Read()");	
}


void menu(int conn_fd, int account_type, int userid)
{

	int option;
	int train_no=0;
	while(1)
	{
		if(read(conn_fd, &option, sizeof(int)) == -1) ERR_EXIT("read()");
		option = ntohl(option);

		switch(option)
		{
			case 1:
				search_and_book_train(account_type,userid,conn_fd);
				break;
			case 2:
				previous_booking(account_type,userid, conn_fd);
				break;
			case 3:
				update_booking(account_type,userid,conn_fd);
				break;
			case 4:
				cancel_booking(account_type,userid,conn_fd);
				break;
			case 5:
				display_trains(conn_fd);
				break;
			default:
			{
				int key = ftok(".",userid);
				int semid = semget(key,1,0);
				struct sembuf buf = {0, -1, SEM_UNDO}; 
					
					buf.sem_op = 1;
					semop(semid,&buf,1);
					semctl(semid,IPC_RMID,SETVAL,arg);

				// menu(conn_fd, account_type, userid);
				pthread_exit(NULL);
			}
		}
	}
}

void admin_menu(int conn_fd)
{
	int option;
	while(1)
	{
		if(read(conn_fd, &option, sizeof(int)) == -1) ERR_EXIT("read()");
		option = ntohl(option);
		switch(option)
		{
			case 1:
				add_train(conn_fd);
				break;
			case 2:
				display_trains(conn_fd);
				break;
			case 3:
				break;
			case 4:
				cancel_train(conn_fd);
				break;
			case 5:
				break;
			default:
			{
				// admin_menu(conn_fd, account_type, userid);
				pthread_exit(NULL);
			}
		}
	}
}

void service_request(int *connection)
{
// print_all_details();
	int conn_fd = *connection;
	
	while(1)
	{
		int option;
		if(read(conn_fd, &option, sizeof(option)) == -1) ERR_EXIT("read()");
		option = ntohl(option);
		if(option == 1)
		{//sign in
			int userid;
			char password[PASS_LENGTH];
			int account_type=0;
			ssize_t count;
			if(read(conn_fd, &account_type, sizeof(account_type)) == -1)
				ERR_EXIT("read()");
			account_type = ntohl(account_type);
			if(read(conn_fd, &userid, sizeof(userid)) == -1)
				ERR_EXIT("read()");
			userid = ntohl(userid);
			if((count = read(conn_fd, password, sizeof(password))) == -1)
				ERR_EXIT("read()");
			int ret = authenticate(userid, password, account_type);
			ret = htonl(ret);
			if(write(conn_fd, &ret, sizeof(ret)) == -1) ERR_EXIT("write()");

		if(ret == 0)
			{
				menu(conn_fd, account_type, userid);
			}
		}
		else if(option == 2)
		{
			int account_type;
			if(read(conn_fd, &account_type, sizeof(account_type)) == -1)
				ERR_EXIT("read()");
			account_type = ntohl(account_type);
			int userid;
			if (account_type == 1) 
			{
				userid = create_user_account(conn_fd);
				userid = htonl(userid);
				if(write(conn_fd, &userid, sizeof(int)) == -1)
					ERR_EXIT("write()");
			}
			else
			{
				userid = create_agent_account(conn_fd);
				userid = htonl(userid);
				if(write(conn_fd, &userid, sizeof(int)) == -1)
					ERR_EXIT("write()");
			}
		}
		else if(option == 3)
		{
			char password[PASS_LENGTH];
			int verified = 0;
			if(read(conn_fd, &password, sizeof(password)) == -1)
					ERR_EXIT("write()");
			if(strcmp(password,"admin")==0)
			{
				verified = 1;
				verified = htonl(verified);
				if(write(conn_fd, &verified, sizeof(int)) == -1)
					ERR_EXIT("write()");	
				admin_menu(conn_fd);
			}
			else
			{
				verified = htonl(verified);
				if(write(conn_fd, &verified, sizeof(int)) == -1)
					ERR_EXIT("write()");
			}
		}
		else
		{
			if(close(conn_fd) == -1) ERR_EXIT("close()");
			pthread_exit(NULL);
		}
	}
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <port_number>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int socket_fd, conn_fd;
	struct sockaddr_in serv_addr;
	
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		_ERR_EXIT("socket()");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		_ERR_EXIT("bind()");
	
	if(listen(socket_fd, 2) == -1)
		_ERR_EXIT("listen()");

	while(1)
	{
		if((conn_fd = accept(socket_fd, (struct sockaddr*)NULL, NULL)) == -1)
			_ERR_EXIT("accept()");
		pthread_t th;
		if(pthread_create(&th, NULL, (void*)service_request, (void*)&conn_fd) != 0)
			_ERR_EXIT("pthread_create()");
	}
	close(socket_fd);
}
