/******************************************************************************
* ООО Теплостар
* Самара
* 
* Программисты: Клюев А.А.
* 
* 08.08.2018
* Описание:
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "core.h"
#include "unix_time.h"

/* Defines ------------------------------------------------------------------*/
#define SEC_A_DAY 86400

UnixTime unixTime;
//-----------------------------------------------------
void UnixTime::timerToCal (unsigned long timer)
{
	unsigned long a;
	char b;
	char c;
	char d;
	unsigned long time;

	time = timer%SEC_A_DAY;
	a = ((timer+43200)/(86400>>1)) + (2440587<<1) + 1;
	a>>=1;
	this->wday = a%7;
	a+=32044;
	b=(4*a+3)/146097;
	a=a-(146097*b)/4;
	c=(4*a+3)/1461;
	a=a-(1461*c)/4;
	d=(5*a+2)/153;
	this->mday=a-(153*d+2)/5+1;
	this->mon=d+3-12*(d/10);
	this->year=100*b+c-4800+(d/10);
	this->hour=time/3600;
	this->min=(time%3600)/60;
	this->sec=(time%3600)%60;
}
//-----------------------------------------------------
unsigned long UnixTime::calToTimer (void)
{
	char a;
	int y;
	char m;
	unsigned long Uday;
	unsigned long time;

	a=((14-this->mon)/12);
	y=this->year+4800-a;
	m=this->mon+(12*a)-3;
	Uday=(this->mday+((153*m+2)/5)+365*y+(y/4)-(y/100)+(y/400)-32045)-2440588;
	time=Uday*86400;
	time+=this->sec+this->min*60+this->hour*3600;
	return time;
}
//-----------------------------------------------------
