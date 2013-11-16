#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char **readString();
int simpleCheck();

int main()
{
	char **buf;
	buf=readString();
	if (simpleCheck(buf)!=-1)
	while (buf[0]!=NULL)
	{
		puts(buf[0]);
		++buf;
	}
	return 0;
}

char **readString()
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
 
