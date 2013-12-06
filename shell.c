#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <limits.h>
#include <sys/wait.h>
#include <signal.h>

#define L_Count 20

typedef enum {CON_NONE,CON_AND,CON_OR} condition; //Тип результата предыдущей команды в условном выполнении
typedef enum {RES_ERROR=-1,RES_SUCCES,RES_BAD_LUCK} result;

char path[PATH_MAX],login[LOGIN_NAME_MAX],CURDIR[PATH_MAX],hostname[HOST_NAME_MAX];
//int fNull;

//Считывание команды и нарезка на "лексемы"
char **readCommand(int *);

//Функция простой проверки введенного выражения
//Поддерживает баланс скобок, "тройные" управляющие символы и др.
int simpleCheck(char **);

//Функция копирования массива "лексем"
void cStrCpy(char **,char ***,int count);

//Очистка памяти
void strFree(char **,int count);

//Обработчик комманды первого уровня.
//Нарезает массив для других функций
result doCommand(char **);

//Обработчик команд с условным выполнением
result coExCommand(char **,int,int);

//Обработчик комманд shell'а
result command(char **,int,int,condition,result);

int main(int argc,char **argv)
{
	char **cStr;
	int count=0;
	signal(SIGINT,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	//В программе реализован простейший cd.
	//Поэтому полное имя программы получается в начале
	readlink("/proc/self/exe",path,PATH_MAX);
	getlogin_r(login,LOGIN_NAME_MAX);
	getcwd(CURDIR,PATH_MAX);
	gethostname(hostname,HOST_NAME_MAX);
/*
	fNull=open("/dev/null",O_RDWR);
	if (fNull==-1)
		puts("Can't open /dev/null! Background-mode not guaranteed!");
*/
	if (argc!=1) //Shell запущен из shell'a с параметрами?
	{
		cStrCpy(argv,&cStr,argc);
		return doCommand(cStr);
	}
	while(1)
	{
//		waitpid(-1,NULL,WNOHANG);
		printf("%s@%s %s:>",login,hostname,CURDIR);
		cStr=readCommand(&count);
		if (simpleCheck(cStr)==0)
		{
			if (strcmp(cStr[0],"exit")==0)
				return 0;
			else
			if (strcmp(cStr[0],"cd")==0)
			{
				if (cStr[1]==NULL)
					chdir(getenv("HOME"));
				else
					if (chdir(cStr[1])==-1)
						perror("SVaSH");
				getcwd(CURDIR,PATH_MAX);
			}
			else
				doCommand(cStr);
		}
		strFree(cStr,count);
	}
	return 0;
}

//Формируется массив "лексем". При чтении считанный
//символ "решает", как поступить  с предыдущим
char **readCommand(int *count)
{
	char **buf=NULL,prev,c;
	int cw=0,cl=0;
	short eFlag=0;
	while ((prev=getchar())==' ');
	if (prev=='\n')
	{
		*count=0;
		return NULL;
	}
	buf=malloc(sizeof(char *));
	buf[0]=NULL;
	if (prev=='"')
	{
		while(getchar()!='\n');
		free(buf);
		*count=0;
		return NULL;
	}
	while (!eFlag)
	{
		c=getchar();
		switch(c)
		{
		case ' ':
			if (prev!=' ' && prev!='\\')
			{
				buf[cw]=realloc(buf[cw],cl+2);
				buf[cw][cl++]=prev;
				buf[cw][cl]='\0';
			}
			if (prev=='\\')
			{
				c=-1;
			}
		break;
		case '\n':
			if (prev!=' ')
			{
				buf[cw]=realloc(buf[cw],cl+2);
				buf[cw][cl++]=prev==-1?' ':prev;
				buf[cw][cl]='\0';
			}
			buf=realloc(buf,(cw+2)*sizeof(char *));
			buf[++cw]=NULL;
			eFlag=1;
		break;
		case '"':
			if (prev!=' ')
			{
				buf[cw]=realloc(buf[cw],cl+2);
				buf[cw][cl++]=prev==-1?' ':prev;
				buf[cw][cl]='\0';
			}
			buf=realloc(buf,(cw+2)*sizeof(char *));
			buf[++cw]=NULL;
			cl=0;
			while(c!='\n')
			{
				c=getchar();
				switch(c)
				{
				case '\n':
					buf[cw]=realloc(buf[cw],cl+1);
					buf[cw][cl++]='\0';
					eFlag=1;
				break;
				case '"':
					buf[cw]=realloc(buf[cw],cl+1);
					buf[cw][cl++]='\0';
					c='\n';
				break;
				case '\\':
					c=getchar();
					if (c!='\n')
					{
						buf[cw]=realloc(buf[cw],cl+1);
						buf[cw][cl++]=c;
					}
				break;
				default:
					buf[cw]=realloc(buf[cw],cl+1);
					buf[cw][cl++]=c;
				break;
				}
			}
			c=' ';
		break;
		case '(':
		case ')':
		case '<':
		case ';':
			if (prev!=' ')
			{
				buf[cw]=realloc(buf[cw],cl+2);
				buf[cw][cl++]=prev==-1?' ':prev;
				buf[cw][cl]='\0';
			}
			buf=realloc(buf,(cw+2)*sizeof(char *));
			buf[++cw]=NULL;
			cl=0;
		break;
		case '&':
		case '|':
		case '>':
			if (prev!=' ')
			{
				if (prev!=c)
				{
					buf[cw]=realloc(buf[cw],cl+2);
					buf[cw][cl++]=prev==-1?' ':prev;
					buf[cw][cl]='\0';
					buf=realloc(buf,(cw+2)*sizeof(char *));
					buf[++cw]=NULL;
					cl=0;
				}
				else
				{
					buf[cw]=realloc(buf[cw],cl+1);
					buf[cw][cl++]=c;
				}
			}
			else
			{
				buf=realloc(buf,(cw+2)*sizeof(char *));
				buf[++cw]=NULL;
				cl=0;
			}
		break;
		default:
			switch(prev)
			{
			case '|':
			case '&':
			case '>':
			case '<':
			case ';':
			case '(':
			case ')':
				buf[cw]=realloc(buf[cw],cl+2);
				buf[cw][cl++]=prev==-1?' ':prev;
				buf[cw][cl]='\0';
				buf=realloc(buf,(cw+2)*sizeof(char *));
				buf[++cw]=NULL;
				cl=0;
			break;
			case ' ':
				buf=realloc(buf,(cw+2)*sizeof(char *));
				buf[++cw]=NULL;
				cl=0;
			break;
			default:
				buf[cw]=realloc(buf[cw],cl+1);
				buf[cw][cl++]=prev==-1?' ':prev;
			break;
			}
		break;
		}
		prev=c;
	}
	*count=cw;
	return buf;
}

int simpleCheck(char **cStr)
{
	int brackets=0;
	if (cStr==NULL)
		return -1;
	while (cStr[0]!=NULL)
	{
		switch(cStr[0][0])
		{
		case '(':
			++brackets;
		break;
		case ')':
			if (--brackets<0)
			{
				puts("SVaSH: Brackets disbalance");
				return -1;
			}
		break;
		case '|':
		case '&':
		case '>':
			if (strlen(cStr[0])>2)
			{
				puts("SVaSH: Wrong usage of special symbols");
				return -1;
			}
		break;
		}
		++cStr;
	}
	if (brackets)
		{
			puts("SVaSH: Brackets disbalance");
			return -1;
		}
	else
		return 0;
}

void cStrCpy(char **source,char ***dest,int count)
{
	int i;
	*dest=malloc(count*sizeof(char *));
	for (i=0;i<count-1;++i)
	{
		(*dest)[i]=malloc(strlen(source[i+1])+1);
		strcpy((*dest)[i],source[i+1]);
	}
	(*dest)[count-1]=NULL;
}

void strFree(char **cStr,int count)
{
	int i;
	for (i=0;i<count;++i)
		if(cStr[i]!=NULL)
			free(cStr[i]);
	free(cStr);
}

result doCommand(char **cStr)
{
	int pos=0,bPos=0;
	int pid,bCount;
	result coExStatus;
	while (1)
	{
		if (cStr[pos]==NULL)
		{
			if (pos==bPos)
				break;
			coExStatus=coExCommand(cStr,bPos,pos);
//			if (coExStatus==RES_ERROR)
//				return RES_ERROR;
			break;
		}
		else
		if (strcmp(cStr[pos],"(")==0)
		{
			bCount=1;
			while (bCount>0)
			{
				++pos;
				if (strcmp(cStr[pos],"(")==0)
					++bCount;
				else
				if (strcmp(cStr[pos],")")==0)
					--bCount;
			}
		}
		else
		if (strcmp(cStr[pos],";")==0)
		{
			free(cStr[pos]);
			cStr[pos]=NULL;
			coExStatus=coExCommand(cStr,bPos,pos);
//			if (coExStatus==RES_ERROR)
//				return RES_ERROR;
			bPos=pos+1;
		}
		else
		if (strcmp(cStr[pos],"&")==0)
		{
			free(cStr[pos]);
			cStr[pos]=NULL;
			if ((pid=fork())==0)
			{
				if ((pid=fork())==0)
				{
					setpgid(0,0);
/*					dup2(fNull,0);
//					dup2(fNull,1);
//					signal(SIGTSTP,SIG_DFL);
//					signal(SIGINT,SIG_DFL);
//					signal(SIGTTIN,SIG_DFL);
//					signal(SIGTTOU,SIG_DFL);
*/					coExStatus=coExCommand(cStr,bPos,pos);
					exit(coExStatus);
				}
				else
				if (pid==-1)
					exit(-1);
				usleep(100000);
				exit(0);
			}
			else
			if (pid==-1)
				perror("Error");
			else
				waitpid(pid,NULL,0);
			bPos=pos+1;
		}
			++pos;
	}
	return coExStatus;
}

result coExCommand(char **cStr,int begin,int end)
{
	int pos=begin,bPos,bCount;
	result prevRes=1;
	condition conCommand=CON_NONE,currCon=CON_NONE;
	while(pos<end)
	{
		bPos=pos;
		while(1)
		{
			if (cStr[pos]==NULL)
			{
				conCommand=CON_NONE;
				break;
			}
			if (strcmp(cStr[pos],"(")==0)
			{
				bCount=1;
				while (bCount>0)
				{
					++pos;
					if (strcmp(cStr[pos],"(")==0)
						++bCount;
					else
					if (strcmp(cStr[pos],")")==0)
						--bCount;
				}
			}
			if (strcmp(cStr[pos],"&&")==0)
			{
				conCommand=CON_AND;
				break;
			}
			if (strcmp(cStr[pos],"||")==0)
			{
				conCommand=CON_OR;
				break;
			}
			++pos;
		}
		free(cStr[pos]);
		cStr[pos]=NULL;
		prevRes=command(cStr,bPos,pos,currCon,prevRes);
		currCon=conCommand;
		++pos;
		if (prevRes==RES_ERROR)
			return RES_ERROR;
	}
	return prevRes;
}

result command(char **cStr,int begin, int end,condition currCon,result prevRes)
{
	int bPos=begin,pos=end;
	int status=-1,pid,fin=-1,fout=-1;
	if (prevRes==RES_BAD_LUCK && currCon==CON_AND)
		return RES_BAD_LUCK;
	if (prevRes==RES_SUCCES && currCon==CON_OR)
		return RES_SUCCES;
	//"Скобки", т.е. Нужен запуск отдельной копии shell'a
	if (strcmp(cStr[bPos],"(")==0 && strcmp(cStr[pos-1],")")==0)
	{
		if ((pid=fork())==0)
		{
			signal(SIGINT,SIG_DFL);
			signal(SIGTSTP,SIG_DFL);
			free(cStr[pos-1]);
			cStr[pos-1]=NULL;
			cStr[bPos]=realloc(cStr[bPos],PATH_MAX);
			strcpy(cStr[bPos],path);
			execvp(path,cStr+bPos);
			perror("SVaSH");
			exit(-1);
		}
		else
		{
			if (wait(&status)==-1)
				return RES_ERROR;
			else
			if (status)
				return RES_BAD_LUCK;
			else
				return RES_SUCCES;
		}
	}
	else
	{
		//Анализ перенаправления ввода-вывода
		if (pos-4>=bPos)
		{
			if (strcmp(cStr[pos-4],"<")==0)
			{
				fin=open(cStr[pos-3],O_RDONLY);
				if (fin==-1)
				{
					perror("SVaSH");
					return RES_ERROR;
				}
				free(cStr[pos-4]);
				cStr[pos-4]=NULL;
			}
			else
			if (strcmp(cStr[pos-4],">")==0)
			{
				fout=open(cStr[pos-3],O_WRONLY|O_CREAT|O_TRUNC,0660);
				if (fout==-1)
				{
					perror("SVaSH");
					return RES_ERROR;
				}
				free(cStr[pos-4]);
				cStr[pos-4]=NULL;
			}
			else
			if (strcmp(cStr[pos-4],">>")==0)
			{
				fout=open(cStr[pos-3],O_WRONLY|O_CREAT|O_APPEND,0660);
				if (fout==-1)
				{
					perror("SVaSH");
					return RES_ERROR;
				}
				free(cStr[pos-4]);
				cStr[pos-4]=NULL;
			}
		}
		if (pos-2>=bPos)
		{
			if (strcmp(cStr[pos-2],"<")==0)
			{
				fin=open(cStr[pos-1],O_RDONLY);
				if (fin==-1)
				{
					perror("SVaSH");
					return RES_ERROR;
				}
				free(cStr[pos-2]);
				cStr[pos-2]=NULL;
			}
			else
			if (strcmp(cStr[pos-2],">")==0)
			{
				fout=open(cStr[pos-1],O_WRONLY|O_CREAT|O_TRUNC,0660);
				if (fout==-1)
				{
					perror("SVaSH");
					return RES_ERROR;
				}
				free(cStr[pos-2]);
				cStr[pos-2]=NULL;
			}
			else
			if (strcmp(cStr[pos-2],">>")==0)
			{
				fout=open(cStr[pos-1],O_WRONLY|O_CREAT|O_APPEND,0660);
				if (fout==-1)
				{
					perror("SVaSH");
					return RES_ERROR;
				}
				free(cStr[pos-2]);
				cStr[pos-2]=NULL;
			}
		}
		if ((pid=fork())==0)
		{
			int cPos=bPos;
			int p1[2]={-1,-1},p2[2]={-1,-1};
			signal(SIGINT,SIG_DFL);
			signal(SIGTSTP,SIG_DFL);
			while(cStr[cPos]!=NULL)
			{
				memcpy(p1,p2,2*sizeof(int));
				pipe(p2);
				bPos=cPos;
				while(cStr[cPos]!=NULL && strcmp(cStr[cPos],"|"))
					++cPos;
				if (!pid)
				{
					if ((pid=fork())==0)
					{
						if (fin!=-1)
							dup2(fin,0);
						if (cStr[cPos]==NULL)
						{
							if (fout!=-1)
								dup2(fout,1);
						}
						else
						{
							dup2(p2[1],1);
							close(p2[0]);
							free(cStr[cPos]);
							cStr[cPos]=NULL;
						}
						execvp(cStr[bPos],cStr+bPos);
						perror("SVaSH");
						exit(-1);
					}
					else
					if (pid==-1)
					{
						perror("SVaSH");
						exit(-1);
					}
				}
				else
				{
					if ((pid=fork())==0)
					{
						dup2(p1[0],0);
						close(p1[1]);
						if (cStr[cPos]==NULL)
						{
							if (fout!=-1)
								dup2(fout,1);
						}
						else
						{
							dup2(p2[1],1);
							close(p2[0]);
							free(cStr[cPos]);
							cStr[cPos]=NULL;
						}
						execvp(cStr[bPos],cStr+bPos);
						perror("SVaSH");
						exit(-1);
					}
					else
					if (pid==-1)
					{
						perror("SVaSH");
						exit(-1);
					}
				}
				close(p1[0]);
				close(p1[1]);
				if (cStr[cPos]!=NULL)
					++cPos;
			}
			close(p2[0]);
			close(p2[1]);
			waitpid(pid,&status,0);
			if (status==0)
				exit(0);
			else
				exit(-1);
		}
		else
		if (pid==-1)
		{
			perror("SVaSH");
			close(fin);
			close(fout);
			return RES_ERROR;
		}
		else
			if (waitpid(pid,&status,0)==-1)
				return RES_ERROR;
		if (status==0)
			return RES_SUCCES;
		else
			return RES_BAD_LUCK;
	}
}

