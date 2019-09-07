#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <cstring>
#include <termios.h>
#include <signal.h>
#include <pwd.h>
#include <fcntl.h>
#include <map>
//#include "profile.h"
using namespace std;

map<string,string> mapEnv;

string getHomeDir()
{	
	char HOME[64];	
	struct passwd *pw = getpwuid(getuid());
	strcpy(HOME,pw->pw_dir);
	mapEnv.insert({"HOME",HOME});
	return HOME;
	
}

string getUserId()
{	char USER[64];
	strcpy(USER,getpwuid(getuid())->pw_name);
	mapEnv.insert({"USER",USER});
	return USER;
		
}

string getHostname()
{	char HOSTNAME[64];
	gethostname(HOSTNAME, 1024);
	//strcpy(HOST,HOSTNAME);
	mapEnv.insert({"HOSTNAME",HOSTNAME});
	return HOSTNAME;
}

void createBashrc()
{	FILE *fd;
	fd = fopen("bashrc1","w");
	if(fd == NULL)
	{
		perror("Error in opening");
		exit(1);
	}
	//string m=;
	// char *HOME;
	//  HOME=;
	fprintf(fd,"HOME=%s\n", (char*)getHomeDir().c_str());
	fprintf(fd,"USER=%s\n", (char*)getUserId().c_str());
	fprintf(fd,"HOSTNAME=%s\n", (char*)getHostname().c_str());
	fprintf(fd, "PS1=%s\n", "$");
	mapEnv.insert({"PS1","$"});
	fprintf(fd,"PATH=%s\n", "Path");
	fclose(fd);
}



void takeInput();
//#define MAX 2048
// void promptPs()
// {
// 	char cwd[2048];
// 	getcwd(cwd, sizeof(cwd));
// 	string sign;
// 	if(USER == "root")
// 		sign="#";
// 	else
// 		sign="$";
// 	cout<<PS1<<cwd<<sign;
// }

// void Print_prompt()
// {
// 	cout<<"shell-->";

//}
void enableRawInputMode()
{
	struct termios rw;
	tcgetattr(STDIN_FILENO, &rw);
	rw.c_lflag &= ~(ICANON);
	//rw.c_iflag &= IUTF8;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &rw);
}

void tstp_handler(int signo)
{	
	//cout<<"in handler\n";
	//signal(SIGTSTP, tstp_handler);
	//cout<<"stop handler";
	//fprintf(stderr,"stop interrupt");
	//cout<<"\n$";
	//Print_prompt();
	cout<<"Exiting";
	cout<<endl;
	cout<<"$";
	fflush(stdout);
	return;
	//takeInput();
	
	//takeInput();
	//exit(1);
}

// void int_handler(int signo)
// {
// 	//signal(SIGINT, int_handler);
// 	cout<<"sigint handler";
// 	//exit(1);
// }


void parseCommand(char *comm, char** argv)
{	//cout<<"in parse";
	string ch=" ";
	//int redirect=0;
	//string opeFile="";
	while(*comm != '\0')
	{	while(*comm == ' ' || *comm == '\t' || *comm =='\n')
		{	*comm++='\0';
			while(*comm == ' ' || *comm == '\t')
			{
				comm++;
			}
		}
		// if((*comm == '>' && *(comm+1) == '>') )
		// {	redirect=2;
		// 	openFile=*(comm+2);
		// }
		// else if(*comm == '>')
		// {	redirect=1;
		// 	openFile=*(comm+1);
		// }
		// 	ch=*comm + *(comm+1);
		// else if( *comm == '>' || *comm == '<')
		// 	ch=*comm;

		if(*comm != '\0')
		*argv++ = comm;

		while(*comm != ' ' && *comm !='\t' && *comm != '\n' && *comm != '\0')
			comm++;
	}

	*argv=NULL;
	//cout<<openFile<<"file"<<endl;
	//return;
	//return ch;
	// for(int i=0; argv[i]!=NULL; i++)
	// {
	// 	cout<<"in parse "<<argv[i]<<endl;
	// }

}



void executeCommand(char **argv)
{	//cout<<"Exe";
	pid_t pid;
	int status;
	int redirect=0;
	int i=0;
	int fd;
	char *fileOpen;
	while(argv[i] != NULL)
	{	//cout<<argv[i]<<"buff"<<endl;
		if(strcmp(argv[i],">>") == 0 )
		{	redirect=2;
			argv[i]=NULL;
			fileOpen=argv[i+1];
			break;
		}
		else if(strcmp(argv[i],">")==0)
		{
			redirect=1;
			argv[i]=NULL;
			fileOpen=argv[i+1];
			break;
		}
		i++;
	}
	//cout<<fileOpen<<"open file"<<endl;
	if((pid = fork()) < 0 )
	{
		cout<<"error";
		exit(1);
	}
	else if(pid == 0 && redirect == 0)
	{	if( execvp(*argv,argv) < 0)
		{
			cout<<"Execution Failed"<<endl;
			exit(1);
		}
	}
	else if(pid == 0 && redirect != 0)
	{	
		if(redirect == 1)
		{	//cout<<"entered"<<endl;
			fd=open(fileOpen, O_WRONLY|O_CREAT|O_TRUNC , 0664);
			//cout<<"opened"<<endl;
		}
		else if(redirect == 2)
		{
			fd=open(fileOpen , O_WRONLY|O_CREAT|O_APPEND , 0664);
			// cout<<"opened"<<endl;
		}
		if(fd < 0)
		{
			cout<<"Error in opening file"<<endl;
			exit(1);
		}
		dup2(fd,1);
		if( execvp(*argv,argv) < 0)
		{
			cout<<"Execution Failed"<<endl;
			exit(1);
		}

	}

	else 
	{
		while(wait(&status) != pid);
	}
}

void executePipe(char* comm)
{	//cout<<"entered execute pipe"<<endl;
	char* listCommands[1025];
	char* token;
	int i=0;
	int inputfd=0;
	int fd[2];
	int count=0;
	int pid;
	
	char command[100];
	token = strtok(comm,"|");
	while(token != NULL)
	{	//cout<<"token"<<token<<endl;
		listCommands[i++]=token;
		token=strtok(NULL,"|");
		count++;
	}
	int processed=0;
	// for(int i=0; listCommands[i]!=NULL; i++)
	 	

	while(processed < count)
	{	pipe(fd);
		if((pid=fork()) < 0)
		{
			cout<<"Error in creating child process"<<endl;
			exit(1);
		}
		else if( pid == 0 )
		{
			dup2(inputfd,0);	//redirected input from output from previous
			// cout<<"processedbeforeif"<<processed<<endl;
			// cout<<"count"<<count<<endl;
			// if(processed+1 < count)
			// 	dup2(fd[1],1);
			// cout<<"hellofd\n";
			
			// cout<<"hifd\n";
			// for(int i=0; listCommands[i]!=NULL; i++)
			// 		cout<<"hello"<<listCommands[i]<<endl;
			char* argv[64];
			//cout<<"processed "<<processed<<endl;
			// cout<<"0yth"<<listCommands[processed]<<endl;
			strcpy(command,listCommands[processed]);
			//cout<<command<<"command"<<endl;
   //        	string a="mn";
			// argv[0]=(char*)a.c_str();
			parseCommand(command, argv);
			//cout<<"argv--"<<argv[0]<<endl;
			//

			if(processed+1 < count)
				dup2(fd[1],1);
			close(fd[0]);
			// cout<<argv[0];
			// for(int i=0; argv[i]!=NULL; i++)
			// 	cout<<argv[i]<<endl;
			// cout<<"args generated";
			if(execvp(*argv,argv) < 0)
			{
				cout<<"Execution failed"<<endl;
				exit(1);
			}
		}
		else
		{
			wait(NULL);
			close(fd[1]);
			inputfd=fd[0];
			processed++;
		}


	}
}


void takeInput()
{	signal(SIGTSTP, tstp_handler);
	if(signal(SIGTSTP, tstp_handler) == 0)
		 	return;
	char comm[64];
	char* argv[64];
	//cout<<"entered input"<<endl;
	// while(1)
	// {	//cout<<"$";
		// getline(cin,comm);
		// cout<<"\n";
		// char line[comm.length()+1];
		// comm.copy(line,comm.length()+1);
		// line[comm.length()]='\0';
		
		//ut<<"$";
		//cout<<"$";
		// signal(SIGTSTP, tstp_handler);
		//  while(signal(SIGTSTP, tstp_handler) == 0)
		//  	continue;
		char c=0,k=0;
		int i=0;
		int checkPipe=0;
		//fflush(stdout);
		//t_prompt();
		while( (c = getchar()) !='\n')
		//while( (c=getchar()) != '\0')
		{	//cout<<c<<endl;
			// if(c == 127)
			// 	printf("%c\n",  c);
			//cout<<c;
			//fflush(stdout);
			// printf("%d",c);
			// int val=c;
			// fflush(stdout);
			if(c == 127)
			{	//cout<<"backspace";
				//printf("buff %s",comm);
				printf("\r                                     ");
				//printf("%c[2K", 100);
				//printf("removed %c\n",comm[i-1]);
				comm[i-1]='\0';
				i--;
				//printf("after removal %c\n",comm[i]);
				printf("\r%s",comm);
				//printf("buff after %s",comm);
				// comm[i-1]='\0';
				// printf("Hii");
				//printf("%s",comm);
			}
			
			 // if (c == 27) {
    //         // "throw away" next two characters which specify escape sequence
    //         c = getchar();
    //         c = getchar();
    //         continue;
    //     	}
			 // if (c == 0x7f) {
    //         // go one char left
    //         printf("\b");
    //         // overwrite the char with whitespace
    //         printf(" ");
    //         // go back to "now removed char position"
    //         printf("\b");
    //         continue;
    //     	}
			// if (c == 127) {
			// 	 	cout<<"backspace";
			if(c == 27)                  // if ch is the escape sequence with num code 27, k turns 1 to signal the next
    		{	k = 1;
    			cout<<"reached"<<endl;
				read(STDIN_FILENO, &c, 1);
				 // if (c == 63 && k==1) {
				 // 	cout<<"backspace";
     //        	// // go one char left
     //       		//  printf("\b");
     //        	// // overwrite the char with whitespace
     //        	// printf(" ");
     //        	// // go back to "now removed char position"
     //        	// printf("\b");
     //        	// continue;
     //        	}
  				if(c == 91 && k == 1)       // if the previous char was 27, and the current 91, k turns 2 for further use
		   		k = 2;
				read(STDIN_FILENO, &c, 1);
  				if(c == 65 && k == 2)       // finally, if the last char of the sequence matches, you've got a key !
    			printf("You pressed the up arrow key !!\n");
  				if(c == 66 && k == 2)                             
    			printf("You pressed the down arrow key !!\n");
				break;
				 if (c == 63) {
            	// go one char left
           		 printf("\b");
            	// overwrite the char with whitespace
            	printf(" ");
            	// go back to "now removed char position"
            	printf("\b");
            	continue;
            	}
        	
			}
			else
			{	if( c == '|')
				checkPipe=1;
				
				comm[i++]=c;
				//cout<<comm[i]<<"added to buffer\n";
			}
		}
		comm[i]='\0';
		// for(int j=0; j<=i; j++)
		// 	cout<<"val "<<comm[j]<<endl;
			//printf("%s in buff",comm);												
		//cout<<comm;
		//cout<<comm;
		//if( parseCommand(line,argv) == " ")
		if(checkPipe==1)
		{	cout<<"Pipe present"<<endl;
			executePipe(comm);		
		}	

		else
		{	parseCommand(comm,argv);
			if(strcmp(argv[0],"exit")==0)
				exit(0);
			
			executeCommand(argv);
		}
			//cout<<endl;
		//cout<<*argv<<"vales"<<endl;
		//cout<<*(argv+1)<<"vales"<<endl;	
		// cout<<*(argv[2])<<"vales"<<endl;
		// else
		// {	

		// }

	//}

}

int main()
{	createBashrc();
	//cout<<geteuid()<<"EUID"<<endl;
	// getHomeDir();
	// getUserId();
	enableRawInputMode();
	
	// //signal(SIGINT, int_handler);
	while(1)
	{	cout<<"$";
		fflush(stdout);
		signal(SIGTSTP, tstp_handler);
		if(signal(SIGTSTP, tstp_handler) == 0)
		 	continue;
		takeInput();
	}
	
}