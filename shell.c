#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#define L_Count 20

enum condition{CON_NONE,CON_AND,CON_OR};

char *readCommand();
void doCommand(char *);
void coExCommand(char *,unsigned int,unsigned int,short int);

int main(int argc,char **argv)
{
	char *cStr;
	if (argc==1)
	{
		doCommand(argv[1]);
		return 0;
	}
	else
	{
		while(1)
		{
			cStr=readCommand();
			doCommand(cStr);
			free(cStr);
		}
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

void doCommand(char *cStr)
{
	unsigned int pos=0,bPos,len=strlen(cStr);
	short bgFlag,pFlag;
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
		coExCommand(cStr,bPos,pos-2,bgFlag);
	}
}

void coExCommand(char *cStr,unsigned int begin,unsigned int end,short bgFlag)
{
	unsigned int pos=begin,bPos;
	short cFlag,prevRes=1;
	condition conCommand=CON_NONE,prevCon=CON_NONE;
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
		prevRes=command(cStr,bPos,pos-3,prevCon,prevRes,bgFlag);
		prevCon=conCommand;
	}
}

