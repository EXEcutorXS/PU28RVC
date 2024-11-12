#include "main.h"

class Backup
{
	public:
	 uint16_t* initFlag ;
	 uint16_t* heaterButtonFlag;
	 uint16_t* elementButtonFlag;
	 uint16_t* airOnFlag;
	 uint16_t* waterButtonFlag;
	 uint16_t* pumpManualButtonFlag;
	 uint16_t* fanAutoButtonFlag;
	 uint16_t* dayTimeSetpoint;
	 uint16_t* nightTimeSetpoint;
	 uint16_t* reloadCounter;
	 uint16_t* systemElapsedMinutes;
	 uint16_t* waterElapsedSeconds;
	 uint16_t* pumpElapsedSeconds; 
	 uint16_t* lastErrors1;
	 uint16_t* lastErrors2;
	 uint16_t* lastErrors3;
	 uint16_t* lastErrors4;
	 uint16_t* lastErrors5;
	 uint16_t* isBleAccept;
	
	void init();
	void handler();
	void restoreData();
	void addErrorToLog(uint16_t);
};

extern Backup backup;
