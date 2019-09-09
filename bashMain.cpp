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
#include <dirent.h>
//#include "profile.h"
using namespace std;

map<string,string> mapEnv;
int status=0;

struct trieDir {
	int val;
	struct trieDir *child[128];
};

struct trieDir *createNewNode()
{
	struct trieDir *node = (struct trieDir*)malloc(sizeof(struct trieDir));
	node->val=0;
	for(int p=0; p<=127; p++)
		node->child[p]=NULL;
	return node;
}

struct trieDir* rootHist=createNewNode();
void insertTrie(string dir, struct trieDir *root)
{	struct trieDir *p = root;
	int v;
	int i=0;
	//char arr[100];
	// char* ag;
	// ag=(char*)dir.c_str();
	// strcpy(arr,ag);
	// arr[dir.length()]='\0';
	while(dir[i] != '\0')
	{
		//cout<<"aaya mai"<<endl;
		v=dir[i];
	
	
		if(p->child[v] != NULL)
			p = p->child[v];
		else
		{
			p->child[v]=createNewNode();
			p=p->child[v];

		}	
		//p->val=0;
		i++;
	}
	p->val=1;
}

// void printtrie(struct trieDir *root)
// {
// 	int index;
// 	for(int i=0; i<128; i++)
// 	{	struct trieDir *head=root;
// 		index=i;
// 		if(head->child[index] == NULL)
// 		{
// 			index=i;
// 			char s=index;
// 			cout<<endl;
// 			head = head->child[index];
// 		}

// 	}

// }

string searchTrie(string dir, struct trieDir *root)
{	int v;
	string filename=dir;
	struct trieDir *p=root;
	for(int i=0; dir[i]!='\0'; i++)
	{	//cout<<"traversed"<<dir[i]<<endl;	
		v=dir[i];
		//cout<<"v "<<v<<endl;
		if(p->child[v] == NULL )
		return NULL;
		p=p->child[v];
	}
	int num=0;
	//char ch;
	// p=root;
	while(p->val != 1)
	{	int j;
		num=0;
		//cout<<"enterednext node"<<endl;
		for(j=0; j<=127; j++)
		{	
			if(p->child[j] != NULL)
				num++;
		}
				if(num>1)
					return " " ;
		for(j=0; j<=127; j++)
		{	
			if(p->child[j] != NULL){
				filename += (char)j;
				//cout<<"filename so far "<<filename<<endl;			
				break;
			}
			
		}
		// 		cout<<(char)j;
		// 		filename += (char)j;
		// 	}.
		// }
		p=p->child[j];
	}
	//cout<<"filename in search"<<filename<<endl;
	if(p->val == 1)
	return filename;
	else
		return " ";
}

void createTrie(struct trieDir* root, string dirlist[],int j)
{	
	for(int i=0; i<j; i++)
	{
		insertTrie(dirlist[i],root);
		//cout<<"inserted"<<dirlist[i]<<endl;
	}
	
}

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

string getPathname()
{	FILE* fileStream;
	//cout<<"reached";
	string pathname="";
	char buf[1000];
	int fd;
	char* token;
	fileStream = fopen("/etc/manpath.config","r");
	if(fileStream != NULL)
	{	while(fgets(buf, 500, fileStream))
		{	char val[20];	
			token = strtok(buf,"\n\t\r");
			strcpy(val,token);
			if(strcmp(val,"MANPATH_MAP")==0)
			{
				//while(token != NULL)
				//{	//cout<<"token"<<token<<endl;
					token=strtok(NULL,"\n\t\r");
					//token=strtok(NULL,"\n\t\r");
					strcpy(val,token);
					//cout<<"Token--"<<val;
					pathname += (string)val+":";
					
					//count++;
					//cout<<"token--"<<token<<endl;
				//}
			}
		}
		fclose(fileStream);
	}
	else
	{
		cout<<"Error in opening /etc/manpath.config"<<endl;
		exit(1);
	}
	mapEnv.insert({"PATH",pathname});
	return pathname;
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
	fprintf(fd, "PATH=%s\n", (char*)getPathname().c_str() );
	fprintf(fd, "PS1=%s\n", "$");
	mapEnv.insert({"PS1","$"});
	//fprintf(fd,"PATH=%s\n", "Path");
	fclose(fd);
}

string createPrompt()
{
	char currDir[100];
	getcwd(currDir,sizeof(currDir));
	string dir=currDir;
	char home[100];
	string dd="";
	int len = mapEnv["HOME"].length();
	if(dir.length() < len)
		dd="~";
	else
	{
		string parsed = dir.substr(len,dir.length());
		dd="~"+parsed;
	}
	string ps="";
	if(strcmp(mapEnv["USER"].c_str(),"root") == 0 )
	{
		mapEnv["PS1"]="#";
		ps="#";
	}
	else
	{
		mapEnv["PS1"]="$";
		ps="$";
	}
	string prom=mapEnv["USER"]+"@"+mapEnv["HOSTNAME"]+":"+dd+ps;
	return prom;
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
	int st=0;
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
		//while(wait(&status) != pid);
		waitpid(pid, &st, 0);
		if(WIFEXITED(st))
		{
			status=WEXITSTATUS(st);
		}
	}
}

string checkRedirectFile(char** argv)
{
	//int pos=-1;
	string filename="";
	for(int i=0; argv[i]!=NULL; i++)
	{	
		if(strcmp(argv[i],">")==0)
			{	//cout<<"mil gya"<<endl;
				//checkredirect=1;
				//redirectOp=1;
				//cout<<"aa";
					//string m=argv[i+1];
					//cout<<"file is "<<m<<"/n";
				//pos=i;
				filename=argv[i+1];
				argv[i]=NULL;
				return filename;
					//cout<<"pos-- "<<pos;
					//break;
			}
			if(strcmp(argv[i],">>")==0)
			{	//checkredirect=1;
				//redirectOp=2;
				//pos=i;
				filename=argv[i+1];
				argv[i]=NULL;
				return filename;
			}
		}
		return filename;
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
	int st=0;
	char command[100];
	token = strtok(comm,"|");
	//cout<<token<<endl;
	while(token != NULL)
	{	//cout<<"token"<<token<<endl;
		listCommands[i++]=token;
		token=strtok(NULL,"|");
		count++;
		//cout<<"token--"<<token<<endl;
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
			//cout<<"processed--"<<processed;
			for(int i =0 ; argv[i]!=NULL;i++)
			{
			//cout<<"argv--"<<i<<"---"<<argv[i]<<endl;
			//cout<<"argv--"<<argv[1]<<endl;
			}
			int redirectOp=-1;
			for(int i =0 ; argv[i]!=NULL;i++)
			{	if(strcmp(argv[i],">") == 0)
				{	redirectOp=1;
					break;
				}
				if(strcmp(argv[i],">>") == 0)
				{
					redirectOp=2;
					break;
				}
			}


			
			//cout<<"returned from pipe check";
			
			if(processed+1 == count && redirectOp != -1)
			{	//cout<<"entered";
				string fileOpen = checkRedirectFile(argv);
				int fdd;
				//for(int i=0; argv[])
				//char* fileOpen=argv[checkredirect+1];
				//cout<<"filename"<<
				if(redirectOp == 1)
					fdd=open(fileOpen.c_str(), O_WRONLY|O_CREAT|O_TRUNC , 0664);
				else
					fdd=open(fileOpen.c_str(), O_WRONLY|O_CREAT|O_APPEND , 0664);
				if(fdd < 0)
				{	cout<<"Error in opening file"<<endl;
					exit(1);
				}
				dup2(fdd,1);
				if( execvp(*argv,argv) < 0)
				{
					cout<<"Execution Failed"<<endl;
					exit(1);
				}
				close(fd[0]);

			}
			else{
					if(processed+1 < count)
						dup2(fd[1],1);
					close(fd[0]);
			
					if(execvp(*argv,argv) < 0)
					{
						cout<<"Execution failed"<<endl;
						exit(1);
					}
				}
		}
		else
		{
			//wait(NULL);
			waitpid(pid, &st, 0);
			if(WIFEXITED(st))
			{
				status=WEXITSTATUS(st);
			}
			close(fd[1]);
			inputfd=fd[0];
			processed++;
		}


	}
}

void executeChangeDir(char** argv, struct trieDir* root)
{
	if(argv[1] == NULL || strcmp(argv[1],"~")==0) 
	{
		if(chdir(getHomeDir().c_str()) != 0 )
		{
			cout<<"Error in changing Directory"<<endl;
			status=127;
			return;
		}
		else
		{	char currentDir[100];
			struct dirent* dir;
			string dirlist[2048];
			int m=0;
			getcwd(currentDir,sizeof(currentDir));
			//cout<<currentDir;
			DIR* dd=opendir(currentDir);
			if(dd)
			{
				while((dir=readdir(dd)) != NULL)
				{	//cout<<"dirname "<<dir->d_name;
					dirlist[m++]=dir->d_name;
				}
			}
			closedir(dd);

			struct trieDir* root=createNewNode();
			createTrie(root,dirlist,m);
		}
	}
	else
	{	string directoryName=argv[1];
		if(directoryName[0] == '~')
		{	
			// if(strcmp(directoryName.c_str(),NULL) == 0)
			// {	if(chdir(getHomeDir().c_str()) != 0 )
			// 	{
			// 		cout<<"Error in changing Directory"<<endl;
			// 		return;
			// 	}
			// 	return;
			// }
			string directName=getHomeDir()+"/"+directoryName.substr(2,directoryName.length());
			cout<<directName<<"directory";
			if(chdir(directName.c_str()) != 0 )
			{
				cout<<"Error in changing Directory"<<endl;
				status=127;
				return;
			}
			else
			{	char currentDir[100];
				struct dirent* dir;
				string dirlist[2048];
				int m=0;
				getcwd(currentDir,sizeof(currentDir));
			//cout<<currentDir;
				DIR* dd=opendir(currentDir);
				if(dd)
				{
					while((dir=readdir(dd)) != NULL)
					{	//cout<<"dirname "<<dir->d_name;
						dirlist[m++]=dir->d_name;
					}
				}
				closedir(dd);

				struct trieDir* root=createNewNode();
				createTrie(root,dirlist,m);

			}

		}
		else
		{	if(chdir(directoryName.c_str()) != 0 )
			{
				cout<<"Error in changing Directory"<<endl;
				status=127;
				return;
			}
			else
			{
				char currentDir[100];
				struct dirent* dir;
				string dirlist[2048];
				int m=0;
				getcwd(currentDir,sizeof(currentDir));
			//cout<<currentDir;
				DIR* dd=opendir(currentDir);
				if(dd)
				{
					while((dir=readdir(dd)) != NULL)
					{	//cout<<"dirname "<<dir->d_name;
						dirlist[m++]=dir->d_name;
					}
				}
				closedir(dd);

				struct trieDir* root=createNewNode();
				createTrie(root,dirlist,m);
			}

		}
	}
		
}

void executeEcho(char* comm)
{	char* token;
	char* token2;
	char com[100];
	char* argv[100];
	strcpy(com,comm);
	// cout<<"comm "<<comm<<endl;
	// cout<<"com "<<com<<endl;
	token = strtok(com," ");
	// cout<<"token--"<<token<<endl;
	// cout<<"com1 "<<com<<endl;
	token2 = strtok(NULL,"\n");
	//cout<<"token--"<<token2;
	parseCommand(token2,argv);
	if(strcmp(argv[0],"$$") == 0)
	{
		cout<<getpid()<<endl;
		status=1;
		return;
	}
	if(strcmp(argv[0],"$?") == 0)
	{
		cout<<status<<endl;
		status=1;
		return;
	}
	for(int i=0; argv[i]!=NULL; i++)
	{ 	//cout<<"argv--"<<argv[i]<<endl;
		if(argv[i][0]=='$')
		{	char gg[100];
			strcpy(gg,argv[i]);
			string val=gg;
			string v=val.substr(1,val.length());
			//gg = v.c_str();
			//cout<<mapEnv[v];
			argv[i]=(char*)mapEnv[v].c_str();
		}
	}
	for(int i=0; argv[i]!=NULL; i++)
		cout<<argv[i]<<" ";
	status=1;
	cout<<endl;
	return;
}

void executeOpen(char** argv)
{	FILE *fd;
	char line[100];
	int pid;
	int st;
	fd = fopen("bashrc1","a+");
	if(fd == NULL)
	{
		cout<<"Error in opening";
		exit(1);
	}
	
	fprintf(fd,"mp4=%s\n", "/usr/bin/vlc");
	fprintf(fd,"mp3=%s\n", "/usr/bin/vlc");
	fprintf(fd,"pdf=%s\n", "/usr/bin/okular");
	fprintf(fd, "txt=%s\n", "/usr/bin/subl");
	fprintf(fd, "cpp=%s\n", "/usr/bin/subl");
	fclose(fd);
	char filename[100];
	strcpy(filename,argv[1]);
	string location="/home/jyoti/Desktop/os/"+(string)filename;
	char* filen = strtok(filename,".");
	char* ext = strtok(NULL, ".");

	fd= fopen("bashrc1","r");
	if(fd==NULL)
	{
		cout<<"error in opening file";
		exit(1);
	}
	char* token;
	char* token2;
	char* path;
	while(fgets(line,sizeof(line),fd))
	{
		token = strtok(line,"=");
		token2 = strtok(NULL,"\n");
		if(strcmp(token,ext) == 0)
		{
			path=token2;
			break;
		}
	}
	char execPath[100];
	strcpy(execPath,path);
	fclose(fd);
	if((pid=fork()) < 0)
	{
		cout<<"Error while creating child process";
		exit(1);
	}
	else if(pid == 0)
	{
		if(execl(execPath,"xdg-open",location.c_str(),(char*)0) < 0)
		{
			cout<<"Exec Failed";
			exit(1);
		}
	}
	else
	{
		waitpid(pid, &st, 0);
		if(WIFEXITED(st))
		{
			status=WEXITSTATUS(st);
		}

	}
}

void takeInput(struct trieDir* root)
{	signal(SIGTSTP, tstp_handler);
	if(signal(SIGTSTP, tstp_handler) == 0)
		 	return;
	char comm[64];
	char* argv[64];
	char* token;
	char* token2;
	string dirName; 
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
				printf("\r                                                                            ");
				//printf("%c[2K", 100);
				//printf("removed %c\n",comm[i-1]);
				comm[i-1]='\0';
				if(i>0)
				i--;
				//printf("after removal %c\n",comm[i]);
				string st=createPrompt()+(char*)comm;
				printf("\r%s",st.c_str());
				continue;
				//printf("buff after %s",comm);
				// comm[i-1]='\0';
				// printf("Hii");
				//printf("%s",comm);
			}
			if(c==9)
			{	//cout<<"detected";		
				comm[i]='\0';
				token= strtok(comm," ");
				token2= strtok(NULL,"\0");
				//cout<<"token"<<token;
				//cout<<"dir"<<token2;
				char compart[20];
				char dirpart[30];
				strcpy(dirpart,token2);
				strcpy(compart,token);
				dirName=searchTrie((string)dirpart, root);
				//change buffer
				//cout<<"dir name"<<dirName;
				string finalbuff = (string)compart+" "+dirName;
				i=finalbuff.length();
				char* hh=(char*)finalbuff.c_str();
				strcpy(comm,hh);
				printf("\r                                                                                         ");
				//cout<<"buffer is "<<comm<<endl;
				string st=createPrompt()+(char*)comm;
				printf("\r%s",st.c_str());
				continue;
			}
   //         // "throw away" next two characters which specify escape sequence
    //         c = getchar();
    //         c = getchar();
    //         continue;
    //     	}
			// if (c== 18)
			// {

			// 	comm[i]='\0';
				
			// 	//cout<<"token"<<token;
			// 	//cout<<"dir"<<token2;
			// 	// char compart[20];
			// 	// char dirpart[30];
			// 	// strcpy(dirpart,token2);
			// 	// strcpy(compart,token);
			// 	//cout<<"comm"<<comm<<endl;
			// 	string commandName=searchTrie((string)comm, rootHist);
			// 	//change buffer
			// 	//cout<<"dir name"<<dirName;
			// 	//string finalbuff = (string)compart+" "+dirName;
			// 	//i=finalbuff.length();
			// 	//char* hh=(char*)finalbuff.c_str();
			// 	//strcpy(comm,hh);
			// 	//printf("\r                                                                                         ");
			// 	//cout<<"buffer is "<<comm<<endl;
			// 	/*string st=createPrompt()+commandName;*/
			// 	/*cout<<commandName<<"command";*/
			// 	//printf("\r%s",st.c_str());
			// 	/*strcpy(comm,commandName.c_str());
			// 	i=commandName.length();*/
			// 	continue;

			// }
   //  //         // go one char left
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
		string comcopy = comm;
		FILE *fdx;
		fdx = fopen("history","a+");
		if(fdx == NULL)
		{
			perror("Error in opening");
			exit(1);
		}
	
		fprintf(fdx,"%s\n", comm);
		insertTrie(comcopy,rootHist);
		fclose(fdx);

		

		// for(int j=0; j<=i; j++)
		// 	cout<<"val "<<comm[j]<<endl;
			//printf("%s in buff",comm);												
		//cout<<comm;
		//cout<<comm;
		//if( parseCommand(line,argv) == " ")
		if(checkPipe==1)
		{	//cout<<"Pipe present"<<endl;
			executePipe(comm);		
		}	

		else
		{	parseCommand(comm,argv);
			if(strcmp(argv[0],"exit")==0)
				exit(0);
			else if(strcmp(argv[0],"cd") == 0)
				executeChangeDir(argv,root);
			else if (strcmp(argv[0],"echo") == 0)
				executeEcho((char*)comcopy.c_str());
			else if(strcmp(argv[0],"open") == 0)
				executeOpen(argv);
			else if(strcmp(argv[0],"history")==0)
			{	FILE* fileStream;
				char buf[1000];
				//int fd;
	
				fileStream = fopen("history","r");
				if(fileStream != NULL)
				{	while(fgets(buf, 500, fileStream))
					{
						cout<<buf;
					}
				}
				//fclose(fd);
			}
			else 
				executeCommand(argv);
		}
		

}



int main()
{ 	//cout<<getPathname();
	createBashrc();
	//cout<<geteuid()<<"EUID"<<endl;
	// getHomeDir();
	// getUserId();

	enableRawInputMode();
	char currentDir[100];
	struct dirent* dir;
	string dirlist[2048];
	int m=0;
	getcwd(currentDir,sizeof(currentDir));
	//cout<<currentDir;
	DIR* dd=opendir(currentDir);
	if(dd)
	{
		while((dir=readdir(dd)) != NULL)
		{	//cout<<"dirname "<<dir->d_name;
			dirlist[m++]=dir->d_name;
		}
	}
	closedir(dd);

	struct trieDir* root=createNewNode();
	createTrie(root,dirlist,m);
	//searchTrie("process_fi",root);
	//string word = searchTrie("process_fi",root);
	// cout<<endl;
	// cout<<"word = "<<word<<endl;
	//printtrie(root);
	// //signal(SIGINT, int_handler);
	while(1)
	{	cout<<createPrompt();
		fflush(stdout);
		signal(SIGTSTP, tstp_handler);
		if(signal(SIGTSTP, tstp_handler) == 0)
		 	continue;
		takeInput(root);
 	}
	
}