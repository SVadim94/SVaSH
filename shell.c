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

char *readCommand();
int doCommand(char *);
int coExCommand(char *,unsigned int,unsigned int,short int);
result command(char *,unsigned int, unsigned int,condition,result);

int main(int argc,char **argv)
{
	char *cStr;
	readlink("/proc/self/exe",path,PATH_MAX);
	if (argc!=1)
		return doCommand(argv[1]);
	while(1)
	{
		cStr=readCommand();
		doCommand(cStr);
		free(cStr);
	}
	return 0;
}

char *readCommand()
{
	unsigned int i=0,j=0,lCount=0;
	char *buf=NULL,c;
	short eFlag=1,wSpace=1;
	while(eFlag)
	{
		if (i>=L_Count*lCount)
		{
			++lCount;
			buf=realloc(buf,lCount*L_Count*sizeof(char));
		}
		c=getchar();
		switch (c)
		{
			case ' ':
				if (!wSpace)
					buf[i++]=' ';
				wSpace=1;
			break;
			case '\n':
				buf[i]='\0';
				buf=realloc(buf,(i+1)*sizeof(char));
				eFlag=0;
			break;
			default:
				wSpace=0;
				buf[i++]=c;
			break;
		}
	}
	return buf;
}

int doCommand(char *cStr)
{
	unsigned int pos=0,bPos,len=strlen(cStr);
	short bgFlag,pFlag,coExStatus;
	while (pos<len)
	{
		bPos=pos;
		pFlag=1;
		bgFlag=0;
		while(pFlag)
		{
			if (pos==len)
			{
				++pos;
				break;
			}
			switch(cStr[pos])
			{
				case '&':
					++pos
					if (cStr[pos]!=&)
					{
						bgFlag=1;
						pFlag=0;
					}
					else
						++pos;
				break;
				case ';':
					++pos;
					pFlag=0;
				break;
				default:
					++pos;
				break;
			}
		}
		coExStatus=coExCommand(cStr,bPos,pos-2,bgFlag);
		if (coExStatus)
			return -1;
	}
	return 0;
}

int coExCommand(char *cStr,unsigned int begin,unsigned int end,short bgFlag)
{
	unsigned int pos=begin,bPos;
	short cFlag;
	result prevRes;
	condition conCommand=CON_NONE,currCon=CON_NONE;
	if (begin>end)
	{
		puts("Syntax error!");
		return -1;
	}
	while(pos<ePos)
	{
		bPos=pos;
		cFlag=1;
		while(cFlag)
		{
			if (pos==end+1)
			{
				conCommand=CON_NONE;
				pos+=2;
				break;
			}
			switch(cStr[pos])
			{
				case '&':
					pos+=2;
					cFlag=1;
					conCommand=CON_AND;
				break;
				case '|':
					pos+=2;
					cFlag=1;
					conCommand=CON_OR;
				break;
				default:
					++pos;
				break;
			}
		}
		prevRes=command(cStr,bPos,pos-3,currCon,prevRes);
		currCon=conCommand;
		if (prevRes==RES_ERROR)
			return -1;
	}
}

result command(char *cStr,unsigned int begin, unsigned int end,condition currCon,result prevRes)
{
	unsigned int bPos=begin,pos=end;
	char *buf=NULL;
	int status;
	if (begin>end)
	{
		puts("Syntax error!");
		return RES_ERROR;
	}
	if (prevRes==0 && currCon==CON_AND)
		return 0;
	if (prevRes==1 && currCon==CON_OR)
		return 1;
	while(bPos<pos && cStr[bPos++]!='(');
	while(pos>bPos && cStr[pos--]!=')');
	if (pos>bPos)
	{
		buf=malloc((pos-bPos+2)*sizeof(char));
		strncpy(buf,cStr+bPos,pos-bPos+1);
		buf[pos-bPos+1]='\0';
		if ((pid=fork())==0)
		{
			execlp(path,path,buf,NULL);
			perror("Error");
			exit(-1);
		}
		else
		{
			if (wait(&status)==-1)
				return RES_ERROR;
			else
			if (WIFEXITED(status)==0 || WEXITSTATUS(status))
				return RES_BAD_LUCK;
			else
				return RES_SUCCES;
		}
	}
	else
	{
		//Конвейер или ошибка
	}
	return status;
}
