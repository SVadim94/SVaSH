#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/wait.h>
#define L_Count 20

enum condition{CON_NONE,CON_AND,CON_OR;};
enum result{RES_ERROR=-1,RES_BAD_LUCK,RES_SUCCES;};
char path[PATH_MAX];

char **readCommand();
int simpleCheck(char **);
void strFree(char **);
int doCommand(char *);
int coExCommand(char *,unsigned int,unsigned int);
result command(char *,unsigned int, unsigned int,condition,result);


int main(int argc,char **argv)
{
	char **cStr;
	readlink("/proc/self/exe",path,PATH_MAX);
	if (argc!=1)
		return doCommand(argv+1);
	while(1)
	{
		cStr=readCommand();
		doCommand(cStr);
		free(cStr);
	}
	return 0;
}

char **readCommand()
{
	char **buf=NULL,prev,c;
	unsigned int cw=0,cl=0;
	short eFlag=0,qFlag=0;
	while ((prev=getchar())==' ');
	if (prev=='\n')
		return NULL;
	buf=malloc(sizeof(char *));
	if (prev=='"')
	{
		qFlag=1;
		c='"';
		prev=' ';
		cw=-1;
	}
	while (!eFlag)
	{
		if (!qFlag)
			c=getchar();
		qFlag=0;
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
				return -1;
		break;
		case '|':
		case '&':
		case '>':
			if (strlen(cStr[0])>2)
				return -1;
		break;
		}
		++cStr;
	}
	if (brackets)
		return -1;
	else
		return 0;
}

void strFree(char **cStr)
{
	char **tmp=cStr;
	while(tmp!=NULL)
	{
		free(*tmp);
		++tmp;
	}
	free(cStr);
}

int doCommand(char *cStr)
{
	unsigned int pos=0,bPos;
	short bgFlag,coExStatus;
	while (cStr[pos]!=NULL)
	{
		bPos=pos;
		bgFlag=0;
		while(1)
		{
			if (cStr[pos]==NULL || strcmp(cStr[pos],';'))
				break;
			if (strcmp(cStr[pos],'&')==0)
			{
				bgFlag=1;
				break;
			}
		}
		free(cStr[pos]);
		cStr[pos]=NULL;
		if (!bgFlag)
		{
			coExStatus=coExCommand(cStr,bPos,pos);
			if (coExStatus)
				return -1;
		}
		else
		{
			//Background Shell
		}
	}
	return 0;
}

int coExCommand(char *cStr,unsigned int begin,unsigned int end)
{
	unsigned int pos=begin,bPos;
	result prevRes;
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
		if (prevRes==RES_ERROR)
			return -1;
	}
}

result command(char *cStr,unsigned int begin, unsigned int end,condition currCon,result prevRes)
{
	unsigned int bPos=begin,pos=end;
	int status,pid,fin=-1,fout=-1;
	if (prevRes==0 && currCon==CON_AND)
		return 0;
	if (prevRes==1 && currCon==CON_OR)
		return 1;
	if (strcmp(cStr[bPos],'(')==0 && strcmp(cStr[pos],')')==0)
	{
		if ((pid=fork())==0)
		{
			fin=open("/dev/null",O_RDONLY);
			free(cStr[pos]);
			cStr[pos]=NULL;
			cStr[bPos]=realloc(cStr[bPos],PATH_MAX);
			strcpy(cStr[bPos],path);
			dup2(fin,0);
			execvp(path,cStr+bPos);
			perror("Error");
			close(fin);
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
		if (pos-4>=bPos)
		{
			if (strcmp(cStr[pos-4],"<")==0)
			{
				fin=open(cStr[pos-3],O_RDONLY);
				if (fin==-1)
				{
					perror("Error:");
					return RES_ERROR;
				}
				free(cStr[pos-4]);
				cStr[pos-4]=NULL;
			}
			else
			if (strcmp(cStr[pos-4],">")==0)
			{
				fout=open(cStr[pos-3],O_WRONLY|O_CREAT);
				if (fout==-1)
				{
					perror("Error:");
					return RES_ERROR;
				}
				free(cStr[pos-4]);
				cStr[pos-4]=NULL;
			}
			else
			if (strcmp(cStr[pos-4],">>")==0)
			{
				fout=open(cStr[pos-3],O_WRONLY|O_CREAT|O_APPEND);
				if (fout==-1)
				{
					perror("Error:");
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
					perror("Error:");
					return RES_ERROR;
				}
				free(cStr[pos-2]);
				cStr[pos-2]=NULL;
			}
			else
			if (strcmp(cStr[pos-2],">")==0)
			{
				fout=open(cStr[pos-1],O_WRONLY|O_CREAT);
				if (fout==-1)
				{
					perror("Error:");
					return RES_ERROR;
				}
				free(cStr[pos-2]);
				cStr[pos-2]=NULL;
			}
			else
			if (strcmp(cStr[pos-2],">>")==0)
			{
				fout=open(cStr[pos-1],O_WRONLY|O_CREAT|O_APPEND);
				if (fout==-1)
				{
					perror("Error:");
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
			while(cStr[pos]!=NULL)
			{
				memcpy(p1,p2,2*sizeof(int));
				pipe(p2);
				if (!pid)
				{
					if ((pid=fork())==0)
					{
						if (fin!=-1)
							dup2(fin,0);
						while(cStr[cPos]!=NULL || strcmp(cStr[cPos],"|"))
							++cPos;
						if (cStr[cPos]==NULL)
						{
							if (fout!=-1)
								dup2(fout,1);
						}
						else
							dup2(p2[1],1);
						execvp(cStr[bPos],cStr+bPos);
						exit(-1);
					}
					else
					if (pid==-1)
					{
						perror("Error:");
						exit(-1);
					}
				}
				else
				{
					if ((pid=fork())==0)
					{
						dup2(p1[0],0);
						while(cStr[cPos]!=NULL || strcmp(cStr[cPos],"|"))
							++cPos;
						if (cStr[cPos]==NULL)
						{
							if (fout!=-1)
								dup2(fout,1);
						}
						else
							dup2(p2[1],1);
						execvp(cStr[bPos],cStr+bPos);
						exit(-1);
					}
					else
					if (pid==-1)
					{
						perror("Error:");
						exit(-1);
					}
				}
				close(p1[0]);
				close(p1[1]);
			}
			close(p2[0]);
			close(p2[1]);
			waitpid(pid,&status,0);
			exit(status);
		}
		else
		if (pid==-1)
		{
			perror("Error:");
			close(fin);
			close(fout);
			return RES_ERROR;
		}
		else
			wait(&status);
		if (status==0)
			return RES_SUCCES;
		else
			return RES_BAD_LUCK;
	}
}

